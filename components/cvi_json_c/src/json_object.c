/*
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 * Copyright (c) 2009 Hewlett-Packard Development Company, L.P.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "json_define.h"

#include "strerror_override.h"

#include <assert.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arraylist.h"
#include "json_debug.h"
#include "json_inttypes.h"
#include "json_object.h"
#include "json_object_private.h"
#include "json_util.h"
#include "linkhash.h"
#include "math_compat.h"
#include "printbuf.h"
#include "snprintf_compat.h"
#include "strdup_compat.h"

/* Avoid ctype.h and locale overhead */
#define is_plain_digit(c) ((c) >= '0' && (c) <= '9')

#if SIZEOF_LONG_LONG != SIZEOF_INT64_T
#error "The long long type isn't 64-bits"
#endif

#ifndef SSIZE_T_MAX
#if SIZEOF_SSIZE_T == SIZEOF_INT
#define SSIZE_T_MAX INT_MAX
#elif SIZEOF_SSIZE_T == SIZEOF_LONG
#define SSIZE_T_MAX LONG_MAX
#elif SIZEOF_SSIZE_T == SIZEOF_LONG_LONG
#define SSIZE_T_MAX LLONG_MAX
#else
#error Unable to determine size of ssize_t
#endif
#endif

// Don't define this.  It's not thread-safe.
/* #define REFCOUNT_DEBUG 1 */

const char *cvi_json_hex_chars = "0123456789abcdefABCDEF";

static void cvi_json_object_generic_delete(struct cvi_json_object *jso);

#if defined(_MSC_VER) && (_MSC_VER <= 1800)
/* VS2013 doesn't know about "inline" */
#define inline __inline
#elif defined(AIX_CC)
#define inline
#endif

/*
 * Helper functions to more safely cast to a particular type of cvi_json_object
 */
static inline struct cvi_json_object_object *JC_OBJECT(struct cvi_json_object *jso)
{
	return (void *)jso;
}
static inline const struct cvi_json_object_object *JC_OBJECT_C(const struct cvi_json_object *jso)
{
	return (const void *)jso;
}
static inline struct cvi_json_object_array *JC_ARRAY(struct cvi_json_object *jso)
{
	return (void *)jso;
}
static inline const struct cvi_json_object_array *JC_ARRAY_C(const struct cvi_json_object *jso)
{
	return (const void *)jso;
}
static inline struct cvi_json_object_boolean *JC_BOOL(struct cvi_json_object *jso)
{
	return (void *)jso;
}
static inline const struct cvi_json_object_boolean *JC_BOOL_C(const struct cvi_json_object *jso)
{
	return (const void *)jso;
}
static inline struct cvi_json_object_double *JC_DOUBLE(struct cvi_json_object *jso)
{
	return (void *)jso;
}
static inline const struct cvi_json_object_double *JC_DOUBLE_C(const struct cvi_json_object *jso)
{
	return (const void *)jso;
}
static inline struct cvi_json_object_int *JC_INT(struct cvi_json_object *jso)
{
	return (void *)jso;
}
static inline const struct cvi_json_object_int *JC_INT_C(const struct cvi_json_object *jso)
{
	return (const void *)jso;
}
static inline struct cvi_json_object_string *JC_STRING(struct cvi_json_object *jso)
{
	return (void *)jso;
}
static inline const struct cvi_json_object_string *JC_STRING_C(const struct cvi_json_object *jso)
{
	return (const void *)jso;
}

#define JC_CONCAT(a, b) a##b
#define JC_CONCAT3(a, b, c) a##b##c

#define JSON_OBJECT_NEW(jtype)                                                           \
	(struct JC_CONCAT(cvi_json_object_, jtype) *)cvi_json_object_new(                        \
	    JC_CONCAT(cvi_json_type_, jtype), sizeof(struct JC_CONCAT(cvi_json_object_, jtype)), \
	    &JC_CONCAT3(cvi_json_object_, jtype, _to_cvi_json_string))

static inline struct cvi_json_object *cvi_json_object_new(enum cvi_json_type o_type, size_t alloc_size,
                                                  cvi_json_object_to_cvi_json_string_fn *to_cvi_json_string);

static void cvi_json_object_object_delete(struct cvi_json_object *jso_base);
static void cvi_json_object_string_delete(struct cvi_json_object *jso);
static void cvi_json_object_array_delete(struct cvi_json_object *jso);

static cvi_json_object_to_cvi_json_string_fn cvi_json_object_object_to_cvi_json_string;
static cvi_json_object_to_cvi_json_string_fn cvi_json_object_boolean_to_cvi_json_string;
static cvi_json_object_to_cvi_json_string_fn cvi_json_object_double_to_cvi_json_string_default;
static cvi_json_object_to_cvi_json_string_fn cvi_json_object_int_to_cvi_json_string;
static cvi_json_object_to_cvi_json_string_fn cvi_json_object_string_to_cvi_json_string;
static cvi_json_object_to_cvi_json_string_fn cvi_json_object_array_to_cvi_json_string;
static cvi_json_object_to_cvi_json_string_fn _cvi_json_object_userdata_to_cvi_json_string;

#ifndef JSON_NORETURN
#if defined(_MSC_VER)
#define JSON_NORETURN __declspec(noreturn)
#elif defined(__OS400__)
#define JSON_NORETURN
#else
/* 'cold' attribute is for optimization, telling the computer this code
 * path is unlikely.
 */
#define JSON_NORETURN __attribute__((noreturn, cold))
#endif
#endif
/**
 * Abort and optionally print a message on standard error.
 * This should be used rather than assert() for unconditional abortion
 * (in particular for code paths which are never supposed to be run).
 * */
JSON_NORETURN static void cvi_json_abort(const char *message);

/* ref count debugging */

#ifdef REFCOUNT_DEBUG

static struct lh_table *cvi_json_object_table;

static void cvi_json_object_init(void) __attribute__((constructor));
static void cvi_json_object_init(void)
{
	MC_DEBUG("cvi_json_object_init: creating object table\n");
	cvi_json_object_table = lh_kptr_table_new(128, NULL);
}

static void cvi_json_object_fini(void) __attribute__((destructor));
static void cvi_json_object_fini(void)
{
	struct lh_entry *ent;
	if (MC_GET_DEBUG())
	{
		if (cvi_json_object_table->count)
		{
			MC_DEBUG("cvi_json_object_fini: %d referenced objects at exit\n",
			         cvi_json_object_table->count);
			lh_foreach(cvi_json_object_table, ent)
			{
				struct cvi_json_object *obj = (struct cvi_json_object *)lh_entry_v(ent);
				MC_DEBUG("\t%s:%p\n", cvi_json_type_to_name(obj->o_type), obj);
			}
		}
	}
	MC_DEBUG("cvi_json_object_fini: freeing object table\n");
	cvi_lh_table_free(cvi_json_object_table);
}
#endif /* REFCOUNT_DEBUG */

/* helper for accessing the optimized string data component in cvi_json_object
 */
static inline char *get_string_component_mutable(struct cvi_json_object *jso)
{
	if (JC_STRING_C(jso)->len < 0)
	{
		/* Due to cvi_json_object_set_string(), we might have a pointer */
		return JC_STRING(jso)->c_string.pdata;
	}
	return JC_STRING(jso)->c_string.idata;
}
static inline const char *get_string_component(const struct cvi_json_object *jso)
{
	return get_string_component_mutable((void *)(uintptr_t)(const void *)jso);
}

/* string escaping */

static int cvi_json_escape_str(struct printbuf *pb, const char *str, size_t len, int flags)
{
	size_t pos = 0, start_offset = 0;
	unsigned char c;
	while (len--)
	{
		c = str[pos];
		switch (c)
		{
		case '\b':
		case '\n':
		case '\r':
		case '\t':
		case '\f':
		case '"':
		case '\\':
		case '/':
			if ((flags & JSON_C_TO_STRING_NOSLASHESCAPE) && c == '/')
			{
				pos++;
				break;
			}

			if (pos > start_offset)
				cvi_printbuf_memappend(pb, str + start_offset, pos - start_offset);

			if (c == '\b')
				cvi_printbuf_memappend(pb, "\\b", 2);
			else if (c == '\n')
				cvi_printbuf_memappend(pb, "\\n", 2);
			else if (c == '\r')
				cvi_printbuf_memappend(pb, "\\r", 2);
			else if (c == '\t')
				cvi_printbuf_memappend(pb, "\\t", 2);
			else if (c == '\f')
				cvi_printbuf_memappend(pb, "\\f", 2);
			else if (c == '"')
				cvi_printbuf_memappend(pb, "\\\"", 2);
			else if (c == '\\')
				cvi_printbuf_memappend(pb, "\\\\", 2);
			else if (c == '/')
				cvi_printbuf_memappend(pb, "\\/", 2);

			start_offset = ++pos;
			break;
		default:
			if (c < ' ')
			{
				char sbuf[7];
				if (pos > start_offset)
					cvi_printbuf_memappend(pb, str + start_offset,
					                   pos - start_offset);
				snprintf(sbuf, sizeof(sbuf), "\\u00%c%c", cvi_json_hex_chars[c >> 4],
				         cvi_json_hex_chars[c & 0xf]);
				printbuf_memappend_fast(pb, sbuf, (int)sizeof(sbuf) - 1);
				start_offset = ++pos;
			}
			else
				pos++;
		}
	}
	if (pos > start_offset)
		cvi_printbuf_memappend(pb, str + start_offset, pos - start_offset);
	return 0;
}

/* reference counting */

struct cvi_json_object *cvi_json_object_get(struct cvi_json_object *jso)
{
	if (!jso)
		return jso;

	// Don't overflow the refcounter.
	assert(jso->_ref_count < UINT32_MAX);

#if defined(HAVE_ATOMIC_BUILTINS) && defined(ENABLE_THREADING)
	__sync_add_and_fetch(&jso->_ref_count, 1);
#else
	++jso->_ref_count;
#endif

	return jso;
}

int cvi_json_object_put(struct cvi_json_object *jso)
{
	if (!jso)
		return 0;

	/* Avoid invalid free and crash explicitly instead of (silently)
	 * segfaulting.
	 */
	assert(jso->_ref_count > 0);

#if defined(HAVE_ATOMIC_BUILTINS) && defined(ENABLE_THREADING)
	/* Note: this only allow the refcount to remain correct
	 * when multiple threads are adjusting it.  It is still an error
	 * for a thread to decrement the refcount if it doesn't "own" it,
	 * as that can result in the thread that loses the race to 0
	 * operating on an already-freed object.
	 */
	if (__sync_sub_and_fetch(&jso->_ref_count, 1) > 0)
		return 0;
#else
	if (--jso->_ref_count > 0)
		return 0;
#endif

	if (jso->_user_delete)
		jso->_user_delete(jso, jso->_userdata);
	switch (jso->o_type)
	{
	case cvi_json_type_object: cvi_json_object_object_delete(jso); break;
	case cvi_json_type_array: cvi_json_object_array_delete(jso); break;
	case cvi_json_type_string: cvi_json_object_string_delete(jso); break;
	default: cvi_json_object_generic_delete(jso); break;
	}
	return 1;
}

/* generic object construction and destruction parts */

static void cvi_json_object_generic_delete(struct cvi_json_object *jso)
{
#ifdef REFCOUNT_DEBUG
	MC_DEBUG("cvi_json_object_delete_%s: %p\n", cvi_json_type_to_name(jso->o_type), jso);
	lh_table_delete(cvi_json_object_table, jso);
#endif /* REFCOUNT_DEBUG */
	cvi_printbuf_free(jso->_pb);
	free(jso);
}

static inline struct cvi_json_object *cvi_json_object_new(enum cvi_json_type o_type, size_t alloc_size,
                                                  cvi_json_object_to_cvi_json_string_fn *to_cvi_json_string)
{
	struct cvi_json_object *jso;

	jso = (struct cvi_json_object *)malloc(alloc_size);
	if (!jso)
		return NULL;

	jso->o_type = o_type;
	jso->_ref_count = 1;
	jso->_to_cvi_json_string = to_cvi_json_string;
	jso->_pb = NULL;
	jso->_user_delete = NULL;
	jso->_userdata = NULL;
	//jso->...   // Type-specific fields must be set by caller

#ifdef REFCOUNT_DEBUG
	lh_table_insert(cvi_json_object_table, jso, jso);
	MC_DEBUG("cvi_json_object_new_%s: %p\n", cvi_json_type_to_name(jso->o_type), jso);
#endif /* REFCOUNT_DEBUG */
	return jso;
}

/* type checking functions */

int cvi_json_object_is_type(const struct cvi_json_object *jso, enum cvi_json_type type)
{
	if (!jso)
		return (type == cvi_json_type_null);
	return (jso->o_type == type);
}

enum cvi_json_type cvi_json_object_get_type(const struct cvi_json_object *jso)
{
	if (!jso)
		return cvi_json_type_null;
	return jso->o_type;
}

void *cvi_json_object_get_userdata(cvi_json_object *jso)
{
	return jso ? jso->_userdata : NULL;
}

void cvi_json_object_set_userdata(cvi_json_object *jso, void *userdata, cvi_json_object_delete_fn *user_delete)
{
	// Can't return failure, so abort if we can't perform the operation.
	assert(jso != NULL);

	// First, clean up any previously existing user info
	if (jso->_user_delete)
		jso->_user_delete(jso, jso->_userdata);

	jso->_userdata = userdata;
	jso->_user_delete = user_delete;
}

/* set a custom conversion to string */

void cvi_json_object_set_serializer(cvi_json_object *jso, cvi_json_object_to_cvi_json_string_fn *to_string_func,
                                void *userdata, cvi_json_object_delete_fn *user_delete)
{
	cvi_json_object_set_userdata(jso, userdata, user_delete);

	if (to_string_func == NULL)
	{
		// Reset to the standard serialization function
		switch (jso->o_type)
		{
		case cvi_json_type_null: jso->_to_cvi_json_string = NULL; break;
		case cvi_json_type_boolean:
			jso->_to_cvi_json_string = &cvi_json_object_boolean_to_cvi_json_string;
			break;
		case cvi_json_type_double:
			jso->_to_cvi_json_string = &cvi_json_object_double_to_cvi_json_string_default;
			break;
		case cvi_json_type_int: jso->_to_cvi_json_string = &cvi_json_object_int_to_cvi_json_string; break;
		case cvi_json_type_object:
			jso->_to_cvi_json_string = &cvi_json_object_object_to_cvi_json_string;
			break;
		case cvi_json_type_array:
			jso->_to_cvi_json_string = &cvi_json_object_array_to_cvi_json_string;
			break;
		case cvi_json_type_string:
			jso->_to_cvi_json_string = &cvi_json_object_string_to_cvi_json_string;
			break;
		}
		return;
	}

	jso->_to_cvi_json_string = to_string_func;
}

/* extended conversion to string */

const char *cvi_json_object_to_cvi_json_string_length(struct cvi_json_object *jso, int flags, size_t *length)
{
	const char *r = NULL;
	size_t s = 0;

	if (!jso)
	{
		s = 4;
		r = "null";
	}
	else if ((jso->_pb) || (jso->_pb = cvi_printbuf_new()))
	{
		cvi_printbuf_reset(jso->_pb);

		if (jso->_to_cvi_json_string(jso, jso->_pb, 0, flags) >= 0)
		{
			s = (size_t)jso->_pb->bpos;
			r = jso->_pb->buf;
		}
	}

	if (length)
		*length = s;
	return r;
}

const char *cvi_json_object_to_cvi_json_string_ext(struct cvi_json_object *jso, int flags)
{
	return cvi_json_object_to_cvi_json_string_length(jso, flags, NULL);
}

/* backwards-compatible conversion to string */

const char *cvi_json_object_to_cvi_json_string(struct cvi_json_object *jso)
{
	return cvi_json_object_to_cvi_json_string_ext(jso, JSON_C_TO_STRING_SPACED);
}

static void indent(struct printbuf *pb, int level, int flags)
{
	if (flags & JSON_C_TO_STRING_PRETTY)
	{
		if (flags & JSON_C_TO_STRING_PRETTY_TAB)
		{
			cvi_printbuf_memset(pb, -1, '\t', level);
		}
		else
		{
			cvi_printbuf_memset(pb, -1, ' ', level * 2);
		}
	}
}

/* cvi_json_object_object */

static int cvi_json_object_object_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb,
                                             int level, int flags)
{
	int had_children = 0;
	struct cvi_json_object_iter iter;

	printbuf_strappend(pb, "{" /*}*/);
	if (flags & JSON_C_TO_STRING_PRETTY)
		printbuf_strappend(pb, "\n");
	cvi_json_object_object_foreachC(jso, iter)
	{
		if (had_children)
		{
			printbuf_strappend(pb, ",");
			if (flags & JSON_C_TO_STRING_PRETTY)
				printbuf_strappend(pb, "\n");
		}
		had_children = 1;
		if (flags & JSON_C_TO_STRING_SPACED && !(flags & JSON_C_TO_STRING_PRETTY))
			printbuf_strappend(pb, " ");
		indent(pb, level + 1, flags);
		printbuf_strappend(pb, "\"");
		cvi_json_escape_str(pb, iter.key, strlen(iter.key), flags);
		if (flags & JSON_C_TO_STRING_SPACED)
			printbuf_strappend(pb, "\": ");
		else
			printbuf_strappend(pb, "\":");
		if (iter.val == NULL)
			printbuf_strappend(pb, "null");
		else if (iter.val->_to_cvi_json_string(iter.val, pb, level + 1, flags) < 0)
			return -1;
	}
	if (flags & JSON_C_TO_STRING_PRETTY)
	{
		if (had_children)
			printbuf_strappend(pb, "\n");
		indent(pb, level, flags);
	}
	if (flags & JSON_C_TO_STRING_SPACED && !(flags & JSON_C_TO_STRING_PRETTY))
		return printbuf_strappend(pb, /*{*/ " }");
	else
		return printbuf_strappend(pb, /*{*/ "}");
}

static void cvi_json_object_lh_entry_free(struct lh_entry *ent)
{
	if (!ent->k_is_constant)
		free(lh_entry_k(ent));
	cvi_json_object_put((struct cvi_json_object *)lh_entry_v(ent));
}

static void cvi_json_object_object_delete(struct cvi_json_object *jso_base)
{
	cvi_lh_table_free(JC_OBJECT(jso_base)->c_object);
	cvi_json_object_generic_delete(jso_base);
}

struct cvi_json_object *cvi_json_object_new_object(void)
{
	struct cvi_json_object_object *jso = JSON_OBJECT_NEW(object);
	if (!jso)
		return NULL;
	jso->c_object =
	    cvi_lh_kchar_table_new(JSON_OBJECT_DEF_HASH_ENTRIES, &cvi_json_object_lh_entry_free);
	if (!jso->c_object)
	{
		cvi_json_object_generic_delete(&jso->base);
		errno = ENOMEM;
		return NULL;
	}
	return &jso->base;
}

struct lh_table *cvi_json_object_get_object(const struct cvi_json_object *jso)
{
	if (!jso)
		return NULL;
	switch (jso->o_type)
	{
	case cvi_json_type_object: return JC_OBJECT_C(jso)->c_object;
	default: return NULL;
	}
}

int cvi_json_object_object_add_ex(struct cvi_json_object *jso, const char *const key,
                              struct cvi_json_object *const val, const unsigned opts)
{
	struct cvi_json_object *existing_value = NULL;
	struct lh_entry *existing_entry;
	unsigned long hash;

	assert(cvi_json_object_get_type(jso) == cvi_json_type_object);

	// We lookup the entry and replace the value, rather than just deleting
	// and re-adding it, so the existing key remains valid.
	hash = lh_get_hash(JC_OBJECT(jso)->c_object, (const void *)key);
	existing_entry =
	    (opts & JSON_C_OBJECT_ADD_KEY_IS_NEW)
	        ? NULL
	        : cvi_lh_table_lookup_entry_w_hash(JC_OBJECT(jso)->c_object, (const void *)key, hash);

	// The caller must avoid creating loops in the object tree, but do a
	// quick check anyway to make sure we're not creating a trivial loop.
	if (jso == val)
		return -1;

	if (!existing_entry)
	{
		const void *const k =
		    (opts & JSON_C_OBJECT_KEY_IS_CONSTANT) ? (const void *)key : strdup(key);
		if (k == NULL)
			return -1;
		return cvi_lh_table_insert_w_hash(JC_OBJECT(jso)->c_object, k, val, hash, opts);
	}
	existing_value = (cvi_json_object *)lh_entry_v(existing_entry);
	if (existing_value)
		cvi_json_object_put(existing_value);
	existing_entry->v = val;
	return 0;
}

int cvi_json_object_object_add(struct cvi_json_object *jso, const char *key, struct cvi_json_object *val)
{
	return cvi_json_object_object_add_ex(jso, key, val, 0);
}

int cvi_json_object_object_length(const struct cvi_json_object *jso)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_object);
	return lh_table_length(JC_OBJECT_C(jso)->c_object);
}

size_t cvi_json_c_object_sizeof(void)
{
	return sizeof(struct cvi_json_object);
}

struct cvi_json_object *cvi_json_object_object_get(const struct cvi_json_object *jso, const char *key)
{
	struct cvi_json_object *result = NULL;
	cvi_json_object_object_get_ex(jso, key, &result);
	return result;
}

cvi_json_bool cvi_json_object_object_get_ex(const struct cvi_json_object *jso, const char *key,
                                    struct cvi_json_object **value)
{
	if (value != NULL)
		*value = NULL;

	if (NULL == jso)
		return 0;

	switch (jso->o_type)
	{
	case cvi_json_type_object:
		return cvi_lh_table_lookup_ex(JC_OBJECT_C(jso)->c_object, (const void *)key,
		                          (void **)value);
	default:
		if (value != NULL)
			*value = NULL;
		return 0;
	}
}

void cvi_json_object_object_del(struct cvi_json_object *jso, const char *key)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_object);
	lh_table_delete(JC_OBJECT(jso)->c_object, key);
}

/* cvi_json_object_boolean */

static int cvi_json_object_boolean_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb,
                                              int level, int flags)
{
	if (JC_BOOL(jso)->c_boolean)
		return printbuf_strappend(pb, "true");
	return printbuf_strappend(pb, "false");
}

struct cvi_json_object *cvi_json_object_new_boolean(cvi_json_bool b)
{
	struct cvi_json_object_boolean *jso = JSON_OBJECT_NEW(boolean);
	if (!jso)
		return NULL;
	jso->c_boolean = b;
	return &jso->base;
}

cvi_json_bool cvi_json_object_get_boolean(const struct cvi_json_object *jso)
{
	if (!jso)
		return 0;
	switch (jso->o_type)
	{
	case cvi_json_type_boolean: return JC_BOOL_C(jso)->c_boolean;
	case cvi_json_type_int:
		switch (JC_INT_C(jso)->cint_type)
		{
		case cvi_json_object_int_type_int64: return (JC_INT_C(jso)->cint.c_int64 != 0);
		case cvi_json_object_int_type_uint64: return (JC_INT_C(jso)->cint.c_uint64 != 0);
		default: cvi_json_abort("invalid cint_type");
		}
	case cvi_json_type_double: return (JC_DOUBLE_C(jso)->c_double != 0);
	case cvi_json_type_string: return (JC_STRING_C(jso)->len != 0);
	default: return 0;
	}
}

int cvi_json_object_set_boolean(struct cvi_json_object *jso, cvi_json_bool new_value)
{
	if (!jso || jso->o_type != cvi_json_type_boolean)
		return 0;
	JC_BOOL(jso)->c_boolean = new_value;
	return 1;
}

/* cvi_json_object_int */

static int cvi_json_object_int_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb, int level,
                                          int flags)
{
	/* room for 19 digits, the sign char, and a null term */
	char sbuf[21];
	if (JC_INT(jso)->cint_type == cvi_json_object_int_type_int64)
		snprintf(sbuf, sizeof(sbuf), "%" PRId64, JC_INT(jso)->cint.c_int64);
	else
		snprintf(sbuf, sizeof(sbuf), "%" PRIu64, JC_INT(jso)->cint.c_uint64);
	return cvi_printbuf_memappend(pb, sbuf, strlen(sbuf));
}

struct cvi_json_object *cvi_json_object_new_int(int32_t i)
{
	return cvi_json_object_new_int64(i);
}

int32_t cvi_json_object_get_int(const struct cvi_json_object *jso)
{
	int64_t cint64 = 0;
	double cdouble;
	enum cvi_json_type o_type;

	if (!jso)
		return 0;

	o_type = jso->o_type;
	if (o_type == cvi_json_type_int)
	{
		const struct cvi_json_object_int *jsoint = JC_INT_C(jso);
		if (jsoint->cint_type == cvi_json_object_int_type_int64)
		{
			cint64 = jsoint->cint.c_int64;
		}
		else
		{
			if (jsoint->cint.c_uint64 >= INT64_MAX)
				cint64 = INT64_MAX;
			else
				cint64 = (int64_t)jsoint->cint.c_uint64;
		}
	}
	else if (o_type == cvi_json_type_string)
	{
		/*
		 * Parse strings into 64-bit numbers, then use the
		 * 64-to-32-bit number handling below.
		 */
		if (cvi_json_parse_int64(get_string_component(jso), &cint64) != 0)
			return 0; /* whoops, it didn't work. */
		o_type = cvi_json_type_int;
	}

	switch (o_type)
	{
	case cvi_json_type_int:
		/* Make sure we return the correct values for out of range numbers. */
		if (cint64 <= INT32_MIN)
			return INT32_MIN;
		if (cint64 >= INT32_MAX)
			return INT32_MAX;
		return (int32_t)cint64;
	case cvi_json_type_double:
		cdouble = JC_DOUBLE_C(jso)->c_double;
		if (cdouble <= INT32_MIN)
			return INT32_MIN;
		if (cdouble >= INT32_MAX)
			return INT32_MAX;
		return (int32_t)cdouble;
	case cvi_json_type_boolean: return JC_BOOL_C(jso)->c_boolean;
	default: return 0;
	}
}

int cvi_json_object_set_int(struct cvi_json_object *jso, int new_value)
{
	return cvi_json_object_set_int64(jso, (int64_t)new_value);
}

struct cvi_json_object *cvi_json_object_new_int64(int64_t i)
{
	struct cvi_json_object_int *jso = JSON_OBJECT_NEW(int);
	if (!jso)
		return NULL;
	jso->cint.c_int64 = i;
	jso->cint_type = cvi_json_object_int_type_int64;
	return &jso->base;
}

struct cvi_json_object *cvi_json_object_new_uint64(uint64_t i)
{
	struct cvi_json_object_int *jso = JSON_OBJECT_NEW(int);
	if (!jso)
		return NULL;
	jso->cint.c_uint64 = i;
	jso->cint_type = cvi_json_object_int_type_uint64;
	return &jso->base;
}

int64_t cvi_json_object_get_int64(const struct cvi_json_object *jso)
{
	int64_t cint;

	if (!jso)
		return 0;
	switch (jso->o_type)
	{
	case cvi_json_type_int:
	{
		const struct cvi_json_object_int *jsoint = JC_INT_C(jso);
		switch (jsoint->cint_type)
		{
		case cvi_json_object_int_type_int64: return jsoint->cint.c_int64;
		case cvi_json_object_int_type_uint64:
			if (jsoint->cint.c_uint64 >= INT64_MAX)
				return INT64_MAX;
			return (int64_t)jsoint->cint.c_uint64;
		default: cvi_json_abort("invalid cint_type");
		}
	}
	case cvi_json_type_double:
		// INT64_MAX can't be exactly represented as a double
		// so cast to tell the compiler it's ok to round up.
		if (JC_DOUBLE_C(jso)->c_double >= (double)INT64_MAX)
			return INT64_MAX;
		if (JC_DOUBLE_C(jso)->c_double <= INT64_MIN)
			return INT64_MIN;
		return (int64_t)JC_DOUBLE_C(jso)->c_double;
	case cvi_json_type_boolean: return JC_BOOL_C(jso)->c_boolean;
	case cvi_json_type_string:
		if (cvi_json_parse_int64(get_string_component(jso), &cint) == 0)
			return cint;
		/* FALLTHRU */
	default: return 0;
	}
}

uint64_t cvi_json_object_get_uint64(const struct cvi_json_object *jso)
{
	uint64_t cuint;

	if (!jso)
		return 0;
	switch (jso->o_type)
	{
	case cvi_json_type_int:
	{
		const struct cvi_json_object_int *jsoint = JC_INT_C(jso);
		switch (jsoint->cint_type)
		{
		case cvi_json_object_int_type_int64:
			if (jsoint->cint.c_int64 < 0)
				return 0;
			return (uint64_t)jsoint->cint.c_int64;
		case cvi_json_object_int_type_uint64: return jsoint->cint.c_uint64;
		default: cvi_json_abort("invalid cint_type");
		}
	}
	case cvi_json_type_double:
		// UINT64_MAX can't be exactly represented as a double
		// so cast to tell the compiler it's ok to round up.
		if (JC_DOUBLE_C(jso)->c_double >= (double)UINT64_MAX)
			return UINT64_MAX;
		if (JC_DOUBLE_C(jso)->c_double < 0)
			return 0;
		return (uint64_t)JC_DOUBLE_C(jso)->c_double;
	case cvi_json_type_boolean: return JC_BOOL_C(jso)->c_boolean;
	case cvi_json_type_string:
		if (cvi_json_parse_uint64(get_string_component(jso), &cuint) == 0)
			return cuint;
		/* FALLTHRU */
	default: return 0;
	}
}

int cvi_json_object_set_int64(struct cvi_json_object *jso, int64_t new_value)
{
	if (!jso || jso->o_type != cvi_json_type_int)
		return 0;
	JC_INT(jso)->cint.c_int64 = new_value;
	JC_INT(jso)->cint_type = cvi_json_object_int_type_int64;
	return 1;
}

int cvi_json_object_set_uint64(struct cvi_json_object *jso, uint64_t new_value)
{
	if (!jso || jso->o_type != cvi_json_type_int)
		return 0;
	JC_INT(jso)->cint.c_uint64 = new_value;
	JC_INT(jso)->cint_type = cvi_json_object_int_type_uint64;
	return 1;
}

int cvi_json_object_int_inc(struct cvi_json_object *jso, int64_t val)
{
	struct cvi_json_object_int *jsoint;
	if (!jso || jso->o_type != cvi_json_type_int)
		return 0;
	jsoint = JC_INT(jso);
	switch (jsoint->cint_type)
	{
	case cvi_json_object_int_type_int64:
		if (val > 0 && jsoint->cint.c_int64 > INT64_MAX - val)
		{
			jsoint->cint.c_uint64 = (uint64_t)jsoint->cint.c_int64 + (uint64_t)val;
			jsoint->cint_type = cvi_json_object_int_type_uint64;
		}
		else if (val < 0 && jsoint->cint.c_int64 < INT64_MIN - val)
		{
			jsoint->cint.c_int64 = INT64_MIN;
		}
		else
		{
			jsoint->cint.c_int64 += val;
		}
		return 1;
	case cvi_json_object_int_type_uint64:
		if (val > 0 && jsoint->cint.c_uint64 > UINT64_MAX - (uint64_t)val)
		{
			jsoint->cint.c_uint64 = UINT64_MAX;
		}
		else if (val < 0 && jsoint->cint.c_uint64 < (uint64_t)(-val))
		{
			jsoint->cint.c_int64 = (int64_t)jsoint->cint.c_uint64 + val;
			jsoint->cint_type = cvi_json_object_int_type_int64;
		}
		else if (val < 0 && jsoint->cint.c_uint64 >= (uint64_t)(-val))
		{
			jsoint->cint.c_uint64 -= (uint64_t)(-val);
		}
		else
		{
			jsoint->cint.c_uint64 += val;
		}
		return 1;
	default: cvi_json_abort("invalid cint_type");
	}
}

/* json_object_double */

#if defined(HAVE___THREAD)
// i.e. __thread or __declspec(thread)
static SPEC___THREAD char *tls_serialization_float_format = NULL;
#endif
static char *global_serialization_float_format = NULL;

int cvi_json_c_set_serialization_double_format(const char *double_format, int global_or_thread)
{
	if (global_or_thread == JSON_C_OPTION_GLOBAL)
	{
#if defined(HAVE___THREAD)
		if (tls_serialization_float_format)
		{
			free(tls_serialization_float_format);
			tls_serialization_float_format = NULL;
		}
#endif
		if (global_serialization_float_format)
			free(global_serialization_float_format);
		global_serialization_float_format = double_format ? strdup(double_format) : NULL;
	}
	else if (global_or_thread == JSON_C_OPTION_THREAD)
	{
#if defined(HAVE___THREAD)
		if (tls_serialization_float_format)
		{
			free(tls_serialization_float_format);
			tls_serialization_float_format = NULL;
		}
		tls_serialization_float_format = double_format ? strdup(double_format) : NULL;
#else
		_cvi_json_c_set_last_err("cvi_json_c_set_option: not compiled with __thread support\n");
		return -1;
#endif
	}
	else
	{
		_cvi_json_c_set_last_err("cvi_json_c_set_option: invalid global_or_thread value: %d\n",
		                     global_or_thread);
		return -1;
	}
	return 0;
}

static int cvi_json_object_double_to_cvi_json_string_format(struct cvi_json_object *jso, struct printbuf *pb,
                                                    int level, int flags, const char *format)
{
	struct cvi_json_object_double *jsodbl = JC_DOUBLE(jso);
	char buf[128], *p, *q;
	int size;
	/* Although JSON RFC does not support
	 * NaN or Infinity as numeric values
	 * ECMA 262 section 9.8.1 defines
	 * how to handle these cases as strings
	 */
	if (isnan(jsodbl->c_double))
	{
		size = snprintf(buf, sizeof(buf), "NaN");
	}
	else if (isinf(jsodbl->c_double))
	{
		if (jsodbl->c_double > 0)
			size = snprintf(buf, sizeof(buf), "Infinity");
		else
			size = snprintf(buf, sizeof(buf), "-Infinity");
	}
	else
	{
		const char *std_format = "%.17g";
		int format_drops_decimals = 0;
		int looks_numeric = 0;

		if (!format)
		{
#if defined(HAVE___THREAD)
			if (tls_serialization_float_format)
				format = tls_serialization_float_format;
			else
#endif
			    if (global_serialization_float_format)
				format = global_serialization_float_format;
			else
				format = std_format;
		}
		size = snprintf(buf, sizeof(buf), format, jsodbl->c_double);

		if (size < 0)
			return -1;

		p = strchr(buf, ',');
		if (p)
			*p = '.';
		else
			p = strchr(buf, '.');

		if (format == std_format || strstr(format, ".0f") == NULL)
			format_drops_decimals = 1;

		looks_numeric = /* Looks like *some* kind of number */
		    is_plain_digit(buf[0]) ||
		    (size > 1 && buf[0] == '-' && is_plain_digit(buf[1]));

		if (size < (int)sizeof(buf) - 2 && looks_numeric && !p && /* Has no decimal point */
		    strchr(buf, 'e') == NULL && /* Not scientific notation */
		    format_drops_decimals)
		{
			// Ensure it looks like a float, even if snprintf didn't,
			//  unless a custom format is set to omit the decimal.
			strcat(buf, ".0");
			size += 2;
		}
		if (p && (flags & JSON_C_TO_STRING_NOZERO))
		{
			/* last useful digit, always keep 1 zero */
			p++;
			for (q = p; *q; q++)
			{
				if (*q != '0')
					p = q;
			}
			/* drop trailing zeroes */
			if (*p != 0)
				*(++p) = 0;
			size = p - buf;
		}
	}
	// although unlikely, snprintf can fail
	if (size < 0)
		return -1;

	if (size >= (int)sizeof(buf))
		// The standard formats are guaranteed not to overrun the buffer,
		// but if a custom one happens to do so, just silently truncate.
		size = sizeof(buf) - 1;
	cvi_printbuf_memappend(pb, buf, size);
	return size;
}

static int cvi_json_object_double_to_cvi_json_string_default(struct cvi_json_object *jso, struct printbuf *pb,
                                                     int level, int flags)
{
	return cvi_json_object_double_to_cvi_json_string_format(jso, pb, level, flags, NULL);
}

int cvi_json_object_double_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb, int level,
                                      int flags)
{
	return cvi_json_object_double_to_cvi_json_string_format(jso, pb, level, flags,
	                                                (const char *)jso->_userdata);
}

struct cvi_json_object *cvi_json_object_new_double(double d)
{
	struct cvi_json_object_double *jso = JSON_OBJECT_NEW(double);
	if (!jso)
		return NULL;
	jso->base._to_cvi_json_string = &cvi_json_object_double_to_cvi_json_string_default;
	jso->c_double = d;
	return &jso->base;
}

struct cvi_json_object *cvi_json_object_new_double_s(double d, const char *ds)
{
	char *new_ds;
	struct cvi_json_object *jso = cvi_json_object_new_double(d);
	if (!jso)
		return NULL;

	new_ds = strdup(ds);
	if (!new_ds)
	{
		cvi_json_object_generic_delete(jso);
		errno = ENOMEM;
		return NULL;
	}
	cvi_json_object_set_serializer(jso, _cvi_json_object_userdata_to_cvi_json_string, new_ds,
	                           cvi_json_object_free_userdata);
	return jso;
}

/*
 * A wrapper around cvi_json_object_userdata_to_cvi_json_string() used only
 * by cvi_json_object_new_double_s() just so cvi_json_object_set_double() can
 * detect when it needs to reset the serializer to the default.
 */
static int _cvi_json_object_userdata_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb,
                                                int level, int flags)
{
	return cvi_json_object_userdata_to_cvi_json_string(jso, pb, level, flags);
}

int cvi_json_object_userdata_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb, int level,
                                        int flags)
{
	int userdata_len = strlen((const char *)jso->_userdata);
	cvi_printbuf_memappend(pb, (const char *)jso->_userdata, userdata_len);
	return userdata_len;
}

void cvi_json_object_free_userdata(struct cvi_json_object *jso, void *userdata)
{
	free(userdata);
}

double cvi_json_object_get_double(const struct cvi_json_object *jso)
{
	double cdouble;
	char *errPtr = NULL;

	if (!jso)
		return 0.0;
	switch (jso->o_type)
	{
	case cvi_json_type_double: return JC_DOUBLE_C(jso)->c_double;
	case cvi_json_type_int:
		switch (JC_INT_C(jso)->cint_type)
		{
		case cvi_json_object_int_type_int64: return JC_INT_C(jso)->cint.c_int64;
		case cvi_json_object_int_type_uint64: return JC_INT_C(jso)->cint.c_uint64;
		default: cvi_json_abort("invalid cint_type");
		}
	case cvi_json_type_boolean: return JC_BOOL_C(jso)->c_boolean;
	case cvi_json_type_string:
		errno = 0;
		cdouble = strtod(get_string_component(jso), &errPtr);

		/* if conversion stopped at the first character, return 0.0 */
		if (errPtr == get_string_component(jso))
		{
			errno = EINVAL;
			return 0.0;
		}

		/*
		 * Check that the conversion terminated on something sensible
		 *
		 * For example, { "pay" : 123AB } would parse as 123.
		 */
		if (*errPtr != '\0')
		{
			errno = EINVAL;
			return 0.0;
		}

		/*
		 * If strtod encounters a string which would exceed the
		 * capacity of a double, it returns +/- HUGE_VAL and sets
		 * errno to ERANGE. But +/- HUGE_VAL is also a valid result
		 * from a conversion, so we need to check errno.
		 *
		 * Underflow also sets errno to ERANGE, but it returns 0 in
		 * that case, which is what we will return anyway.
		 *
		 * See CERT guideline ERR30-C
		 */
		if ((HUGE_VAL == cdouble || -HUGE_VAL == cdouble) && (ERANGE == errno))
			cdouble = 0.0;
		return cdouble;
	default: errno = EINVAL; return 0.0;
	}
}

int cvi_json_object_set_double(struct cvi_json_object *jso, double new_value)
{
	if (!jso || jso->o_type != cvi_json_type_double)
		return 0;
	JC_DOUBLE(jso)->c_double = new_value;
	if (jso->_to_cvi_json_string == &_cvi_json_object_userdata_to_cvi_json_string)
		cvi_json_object_set_serializer(jso, NULL, NULL, NULL);
	return 1;
}

/* cvi_json_object_string */

static int cvi_json_object_string_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb,
                                             int level, int flags)
{
	ssize_t len = JC_STRING(jso)->len;
	printbuf_strappend(pb, "\"");
	cvi_json_escape_str(pb, get_string_component(jso), len < 0 ? -(ssize_t)len : len, flags);
	printbuf_strappend(pb, "\"");
	return 0;
}

static void cvi_json_object_string_delete(struct cvi_json_object *jso)
{
	if (JC_STRING(jso)->len < 0)
		free(JC_STRING(jso)->c_string.pdata);
	cvi_json_object_generic_delete(jso);
}

static struct cvi_json_object *_cvi_json_object_new_string(const char *s, const size_t len)
{
	size_t objsize;
	struct cvi_json_object_string *jso;

	/*
	 * Structures           Actual memory layout
	 * -------------------  --------------------
	 * [cvi_json_object_string  [cvi_json_object_string
	 *  [cvi_json_object]        [cvi_json_object]
	 *  ...other fields...   ...other fields...
	 *  c_string]            len
	 *                       bytes
	 *                       of
	 *                       string
	 *                       data
	 *                       \0]
	 */
	if (len > (SSIZE_T_MAX - (sizeof(*jso) - sizeof(jso->c_string)) - 1))
		return NULL;
	objsize = (sizeof(*jso) - sizeof(jso->c_string)) + len + 1;
	if (len < sizeof(void *))
		// We need a minimum size to support cvi_json_object_set_string() mutability
		// so we can stuff a pointer into pdata :(
		objsize += sizeof(void *) - len;

	jso = (struct cvi_json_object_string *)cvi_json_object_new(cvi_json_type_string, objsize,
	                                                   &cvi_json_object_string_to_cvi_json_string);

	if (!jso)
		return NULL;
	jso->len = len;
	memcpy(jso->c_string.idata, s, len);
	jso->c_string.idata[len] = '\0';
	return &jso->base;
}

struct cvi_json_object *cvi_json_object_new_string(const char *s)
{
	return _cvi_json_object_new_string(s, strlen(s));
}

struct cvi_json_object *cvi_json_object_new_string_len(const char *s, const int len)
{
	return _cvi_json_object_new_string(s, len);
}

const char *cvi_json_object_get_string(struct cvi_json_object *jso)
{
	if (!jso)
		return NULL;
	switch (jso->o_type)
	{
	case cvi_json_type_string: return get_string_component(jso);
	default: return cvi_json_object_to_cvi_json_string(jso);
	}
}
static inline ssize_t _cvi_json_object_get_string_len(const struct cvi_json_object_string *jso)
{
	ssize_t len;
	len = jso->len;
	return (len < 0) ? -(ssize_t)len : len;
}

int cvi_json_object_get_string_len(const struct cvi_json_object *jso)
{
	if (!jso)
		return 0;
	switch (jso->o_type)
	{
	case cvi_json_type_string: return _cvi_json_object_get_string_len(JC_STRING_C(jso));
	default: return 0;
	}
}

static int _cvi_json_object_set_string_len(cvi_json_object *jso, const char *s, size_t len)
{
	char *dstbuf;
	ssize_t curlen;
	ssize_t newlen;
	if (jso == NULL || jso->o_type != cvi_json_type_string)
		return 0;

	if (len >= INT_MAX - 1)
		// jso->len is a signed ssize_t, so it can't hold the
		// full size_t range. cvi_json_object_get_string_len returns
		// length as int, cap length at INT_MAX.
		return 0;

	dstbuf = get_string_component_mutable(jso);
	curlen = JC_STRING(jso)->len;
	if (curlen < 0)
		curlen = -curlen;
	newlen = len;

	if ((ssize_t)len > curlen)
	{
		// We have no way to return the new ptr from realloc(jso, newlen)
		// and we have no way of knowing whether there's extra room available
		// so we need to stuff a pointer in to pdata :(
		dstbuf = (char *)malloc(len + 1);
		if (dstbuf == NULL)
			return 0;
		if (JC_STRING(jso)->len < 0)
			free(JC_STRING(jso)->c_string.pdata);
		JC_STRING(jso)->c_string.pdata = dstbuf;
		newlen = -(ssize_t)len;
	}
	else if (JC_STRING(jso)->len < 0)
	{
		// We've got enough room in the separate allocated buffer,
		// so use it as-is and continue to indicate that pdata is used.
		newlen = -(ssize_t)len;
	}

	memcpy(dstbuf, (const void *)s, len);
	dstbuf[len] = '\0';
	JC_STRING(jso)->len = newlen;
	return 1;
}

int cvi_json_object_set_string(cvi_json_object *jso, const char *s)
{
	return _cvi_json_object_set_string_len(jso, s, strlen(s));
}

int cvi_json_object_set_string_len(cvi_json_object *jso, const char *s, int len)
{
	return _cvi_json_object_set_string_len(jso, s, len);
}

/* cvi_json_object_array */

static int cvi_json_object_array_to_cvi_json_string(struct cvi_json_object *jso, struct printbuf *pb, int level,
                                            int flags)
{
	int had_children = 0;
	size_t ii;

	printbuf_strappend(pb, "[");
	if (flags & JSON_C_TO_STRING_PRETTY)
		printbuf_strappend(pb, "\n");
	for (ii = 0; ii < cvi_json_object_array_length(jso); ii++)
	{
		struct cvi_json_object *val;
		if (had_children)
		{
			printbuf_strappend(pb, ",");
			if (flags & JSON_C_TO_STRING_PRETTY)
				printbuf_strappend(pb, "\n");
		}
		had_children = 1;
		if (flags & JSON_C_TO_STRING_SPACED && !(flags & JSON_C_TO_STRING_PRETTY))
			printbuf_strappend(pb, " ");
		indent(pb, level + 1, flags);
		val = cvi_json_object_array_get_idx(jso, ii);
		if (val == NULL)
			printbuf_strappend(pb, "null");
		else if (val->_to_cvi_json_string(val, pb, level + 1, flags) < 0)
			return -1;
	}
	if (flags & JSON_C_TO_STRING_PRETTY)
	{
		if (had_children)
			printbuf_strappend(pb, "\n");
		indent(pb, level, flags);
	}

	if (flags & JSON_C_TO_STRING_SPACED && !(flags & JSON_C_TO_STRING_PRETTY))
		return printbuf_strappend(pb, " ]");
	return printbuf_strappend(pb, "]");
}

static void cvi_json_object_array_entry_free(void *data)
{
	cvi_json_object_put((struct cvi_json_object *)data);
}

static void cvi_json_object_array_delete(struct cvi_json_object *jso)
{
	cvi_array_list_free(JC_ARRAY(jso)->c_array);
	cvi_json_object_generic_delete(jso);
}

struct cvi_json_object *cvi_json_object_new_array(void)
{
	return cvi_json_object_new_array_ext(ARRAY_LIST_DEFAULT_SIZE);
}
struct cvi_json_object *cvi_json_object_new_array_ext(int initial_size)
{
	struct cvi_json_object_array *jso = JSON_OBJECT_NEW(array);
	if (!jso)
		return NULL;
	jso->c_array = cvi_array_list_new2(&cvi_json_object_array_entry_free, initial_size);
	if (jso->c_array == NULL)
	{
		free(jso);
		return NULL;
	}
	return &jso->base;
}

struct array_list *cvi_json_object_get_array(const struct cvi_json_object *jso)
{
	if (!jso)
		return NULL;
	switch (jso->o_type)
	{
	case cvi_json_type_array: return JC_ARRAY_C(jso)->c_array;
	default: return NULL;
	}
}

void cvi_json_object_array_sort(struct cvi_json_object *jso, int (*sort_fn)(const void *, const void *))
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	array_list_sort(JC_ARRAY(jso)->c_array, sort_fn);
}

struct cvi_json_object *cvi_json_object_array_bsearch(const struct cvi_json_object *key,
                                              const struct cvi_json_object *jso,
                                              int (*sort_fn)(const void *, const void *))
{
	struct cvi_json_object **result;

	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	result = (struct cvi_json_object **)array_list_bsearch((const void **)(void *)&key,
	                                                   JC_ARRAY_C(jso)->c_array, sort_fn);

	if (!result)
		return NULL;
	return *result;
}

size_t cvi_json_object_array_length(const struct cvi_json_object *jso)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	return cvi_array_list_length(JC_ARRAY_C(jso)->c_array);
}

int cvi_json_object_array_add(struct cvi_json_object *jso, struct cvi_json_object *val)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	return cvi_array_list_add(JC_ARRAY(jso)->c_array, val);
}

int cvi_json_object_array_put_idx(struct cvi_json_object *jso, size_t idx, struct cvi_json_object *val)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	return array_list_put_idx(JC_ARRAY(jso)->c_array, idx, val);
}

int cvi_json_object_array_del_idx(struct cvi_json_object *jso, size_t idx, size_t count)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	return array_list_del_idx(JC_ARRAY(jso)->c_array, idx, count);
}

struct cvi_json_object *cvi_json_object_array_get_idx(const struct cvi_json_object *jso, size_t idx)
{
	assert(cvi_json_object_get_type(jso) == cvi_json_type_array);
	return (struct cvi_json_object *)cvi_array_list_get_idx(JC_ARRAY_C(jso)->c_array, idx);
}

static int cvi_json_array_equal(struct cvi_json_object *jso1, struct cvi_json_object *jso2)
{
	size_t len, i;

	len = cvi_json_object_array_length(jso1);
	if (len != cvi_json_object_array_length(jso2))
		return 0;

	for (i = 0; i < len; i++)
	{
		if (!cvi_json_object_equal(cvi_json_object_array_get_idx(jso1, i),
		                       cvi_json_object_array_get_idx(jso2, i)))
			return 0;
	}
	return 1;
}

int cvi_json_object_array_shrink(struct cvi_json_object *jso, int empty_slots)
{
	if (empty_slots < 0)
		cvi_json_abort("cvi_json_object_array_shrink called with negative empty_slots");
	return cvi_array_list_shrink(JC_ARRAY(jso)->c_array, empty_slots);
}

struct cvi_json_object *cvi_json_object_new_null(void)
{
	return NULL;
}

static int cvi_json_object_all_values_equal(struct cvi_json_object *jso1, struct cvi_json_object *jso2)
{
	struct cvi_json_object_iter iter;
	struct cvi_json_object *sub;

	assert(cvi_json_object_get_type(jso1) == cvi_json_type_object);
	assert(cvi_json_object_get_type(jso2) == cvi_json_type_object);
	/* Iterate over jso1 keys and see if they exist and are equal in jso2 */
	cvi_json_object_object_foreachC(jso1, iter)
	{
		if (!cvi_lh_table_lookup_ex(JC_OBJECT(jso2)->c_object, (void *)iter.key,
		                        (void **)(void *)&sub))
			return 0;
		if (!cvi_json_object_equal(iter.val, sub))
			return 0;
	}

	/* Iterate over jso2 keys to see if any exist that are not in jso1 */
	cvi_json_object_object_foreachC(jso2, iter)
	{
		if (!cvi_lh_table_lookup_ex(JC_OBJECT(jso1)->c_object, (void *)iter.key,
		                        (void **)(void *)&sub))
			return 0;
	}

	return 1;
}

int cvi_json_object_equal(struct cvi_json_object *jso1, struct cvi_json_object *jso2)
{
	if (jso1 == jso2)
		return 1;

	if (!jso1 || !jso2)
		return 0;

	if (jso1->o_type != jso2->o_type)
		return 0;

	switch (jso1->o_type)
	{
	case cvi_json_type_boolean: return (JC_BOOL(jso1)->c_boolean == JC_BOOL(jso2)->c_boolean);

	case cvi_json_type_double: return (JC_DOUBLE(jso1)->c_double == JC_DOUBLE(jso2)->c_double);

	case cvi_json_type_int:
	{
		struct cvi_json_object_int *int1 = JC_INT(jso1);
		struct cvi_json_object_int *int2 = JC_INT(jso2);
		if (int1->cint_type == cvi_json_object_int_type_int64)
		{
			if (int2->cint_type == cvi_json_object_int_type_int64)
				return (int1->cint.c_int64 == int2->cint.c_int64);
			if (int1->cint.c_int64 < 0)
				return 0;
			return ((uint64_t)int1->cint.c_int64 == int2->cint.c_uint64);
		}
		// else jso1 is a uint64
		if (int2->cint_type == cvi_json_object_int_type_uint64)
			return (int1->cint.c_uint64 == int2->cint.c_uint64);
		if (int2->cint.c_int64 < 0)
			return 0;
		return (int1->cint.c_uint64 == (uint64_t)int2->cint.c_int64);
	}

	case cvi_json_type_string:
	{
		return (_cvi_json_object_get_string_len(JC_STRING(jso1)) ==
		            _cvi_json_object_get_string_len(JC_STRING(jso2)) &&
		        memcmp(get_string_component(jso1), get_string_component(jso2),
		               _cvi_json_object_get_string_len(JC_STRING(jso1))) == 0);
	}

	case cvi_json_type_object: return cvi_json_object_all_values_equal(jso1, jso2);

	case cvi_json_type_array: return cvi_json_array_equal(jso1, jso2);

	case cvi_json_type_null: return 1;
	};

	return 0;
}

static int cvi_json_object_copy_serializer_data(struct cvi_json_object *src, struct cvi_json_object *dst)
{
	if (!src->_userdata && !src->_user_delete)
		return 0;

	if (dst->_to_cvi_json_string == cvi_json_object_userdata_to_cvi_json_string ||
	    dst->_to_cvi_json_string == _cvi_json_object_userdata_to_cvi_json_string)
	{
		dst->_userdata = strdup(src->_userdata);
	}
	// else if ... other supported serializers ...
	else
	{
		_cvi_json_c_set_last_err(
		    "cvi_json_object_deep_copy: unable to copy unknown serializer data: %p\n",
		    (void *)dst->_to_cvi_json_string);
		return -1;
	}
	dst->_user_delete = src->_user_delete;
	return 0;
}

/**
 * The default shallow copy implementation.  Simply creates a new object of the same
 * type but does *not* copy over _userdata nor retain any custom serializer.
 * If custom serializers are in use, cvi_json_object_deep_copy() must be passed a shallow copy
 * implementation that is aware of how to copy them.
 *
 * This always returns -1 or 1.  It will never return 2 since it does not copy the serializer.
 */

int cvi_json_c_shallow_copy_default(cvi_json_object *src, cvi_json_object *parent, const char *key,
                                size_t index, cvi_json_object **dst)
{
	switch (src->o_type)
	{
	case cvi_json_type_boolean: *dst = cvi_json_object_new_boolean(JC_BOOL(src)->c_boolean); break;

	case cvi_json_type_double: *dst = cvi_json_object_new_double(JC_DOUBLE(src)->c_double); break;

	case cvi_json_type_int:
		switch (JC_INT(src)->cint_type)
		{
		case cvi_json_object_int_type_int64:
			*dst = cvi_json_object_new_int64(JC_INT(src)->cint.c_int64);
			break;
		case cvi_json_object_int_type_uint64:
			*dst = cvi_json_object_new_uint64(JC_INT(src)->cint.c_uint64);
			break;
		default: cvi_json_abort("invalid cint_type");
		}
		break;

	case cvi_json_type_string:
		*dst = cvi_json_object_new_string_len(get_string_component(src),
		                                  _cvi_json_object_get_string_len(JC_STRING(src)));
		break;

	case cvi_json_type_object: *dst = cvi_json_object_new_object(); break;

	case cvi_json_type_array: *dst = cvi_json_object_new_array(); break;

	default: errno = EINVAL; return -1;
	}

	if (!*dst)
	{
		errno = ENOMEM;
		return -1;
	}
	(*dst)->_to_cvi_json_string = src->_to_cvi_json_string;
	// _userdata and _user_delete are copied later
	return 1;
}

/*
 * The actual guts of cvi_json_object_deep_copy(), with a few additional args
 * needed so we can keep track of where we are within the object tree.
 *
 * Note: caller is responsible for freeing *dst if this fails and returns -1.
 */
static int cvi_json_object_deep_copy_recursive(struct cvi_json_object *src, struct cvi_json_object *parent,
                                           const char *key_in_parent, size_t index_in_parent,
                                           struct cvi_json_object **dst,
                                           cvi_json_c_shallow_copy_fn *shallow_copy)
{
	struct cvi_json_object_iter iter;
	size_t src_array_len, ii;

	int shallow_copy_rc = 0;
	shallow_copy_rc = shallow_copy(src, parent, key_in_parent, index_in_parent, dst);
	/* -1=error, 1=object created ok, 2=userdata set */
	if (shallow_copy_rc < 1)
	{
		errno = EINVAL;
		return -1;
	}
	assert(*dst != NULL);

	switch (src->o_type)
	{
	case cvi_json_type_object:
		cvi_json_object_object_foreachC(src, iter)
		{
			struct cvi_json_object *jso = NULL;
			/* This handles the `cvi_json_type_null` case */
			if (!iter.val)
				jso = NULL;
			else if (cvi_json_object_deep_copy_recursive(iter.val, src, iter.key, UINT_MAX, &jso,
			                                         shallow_copy) < 0)
			{
				cvi_json_object_put(jso);
				return -1;
			}

			if (cvi_json_object_object_add(*dst, iter.key, jso) < 0)
			{
				cvi_json_object_put(jso);
				return -1;
			}
		}
		break;

	case cvi_json_type_array:
		src_array_len = cvi_json_object_array_length(src);
		for (ii = 0; ii < src_array_len; ii++)
		{
			struct cvi_json_object *jso = NULL;
			struct cvi_json_object *jso1 = cvi_json_object_array_get_idx(src, ii);
			/* This handles the `cvi_json_type_null` case */
			if (!jso1)
				jso = NULL;
			else if (cvi_json_object_deep_copy_recursive(jso1, src, NULL, ii, &jso,
			                                         shallow_copy) < 0)
			{
				cvi_json_object_put(jso);
				return -1;
			}

			if (cvi_json_object_array_add(*dst, jso) < 0)
			{
				cvi_json_object_put(jso);
				return -1;
			}
		}
		break;

	default:
		break;
		/* else, nothing to do, shallow_copy already did. */
	}

	if (shallow_copy_rc != 2)
		return cvi_json_object_copy_serializer_data(src, *dst);

	return 0;
}

int cvi_json_object_deep_copy(struct cvi_json_object *src, struct cvi_json_object **dst,
                          cvi_json_c_shallow_copy_fn *shallow_copy)
{
	int rc;

	/* Check if arguments are sane ; *dst must not point to a non-NULL object */
	if (!src || !dst || *dst)
	{
		errno = EINVAL;
		return -1;
	}

	if (shallow_copy == NULL)
		shallow_copy = cvi_json_c_shallow_copy_default;

	rc = cvi_json_object_deep_copy_recursive(src, NULL, NULL, UINT_MAX, dst, shallow_copy);
	if (rc < 0)
	{
		cvi_json_object_put(*dst);
		*dst = NULL;
	}

	return rc;
}

static void cvi_json_abort(const char *message)
{
	if (message != NULL)
		fprintf(stderr, "cvi_json-c aborts with error: %s\n", message);
	abort();
}
