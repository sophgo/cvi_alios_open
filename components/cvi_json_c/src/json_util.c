/*
 * $Id: cvi_json_util.c,v 1.4 2006/01/30 23:07:57 mclark Exp $
 *
 * Copyright (c) 2004, 2005 Metaparadigm Pte. Ltd.
 * Michael Clark <michael@metaparadigm.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 *
 */

#include "json_define.h"
#undef realloc

#include "strerror_override.h"

#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif /* HAVE_SYS_TYPES_H */

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif /* HAVE_SYS_STAT_H */

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif /* HAVE_UNISTD_H */

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <windows.h>
#endif /* defined(WIN32) */

#if !defined(HAVE_OPEN) && defined(WIN32)
#define open _open
#endif

#include "snprintf_compat.h"

#include "json_debug.h"
#include "json_inttypes.h"
#include "json_object.h"
#include "json_tokener.h"
#include "json_util.h"
#include "printbuf.h"

static int _cvi_json_object_to_fd(int fd, struct cvi_json_object *obj, int flags, const char *filename);

static char _last_err[256] = "";

const char *cvi_json_util_get_last_err()
{
	if (_last_err[0] == '\0')
		return NULL;
	return _last_err;
}

void _cvi_json_c_set_last_err(const char *err_fmt, ...)
{
	va_list ap;
	va_start(ap, err_fmt);
	// Ignore (attempted) overruns from snprintf
	(void)vsnprintf(_last_err, sizeof(_last_err), err_fmt, ap);
	va_end(ap);
}

struct cvi_json_object *cvi_json_object_from_fd(int fd)
{
	return cvi_json_object_from_fd_ex(fd, -1);
}
struct cvi_json_object *cvi_json_object_from_fd_ex(int fd, int in_depth)
{
	struct printbuf *pb;
	struct cvi_json_object *obj;
	char buf[JSON_FILE_BUF_SIZE];
	int ret;
	int depth = JSON_TOKENER_DEFAULT_DEPTH;
	cvi_json_tokener *tok;

	if (!(pb = cvi_printbuf_new()))
	{
		_cvi_json_c_set_last_err("cvi_json_object_from_file: cvi_printbuf_new failed\n");
		return NULL;
	}

	if (in_depth != -1)
		depth = in_depth;
	tok = cvi_json_tokener_new_ex(depth);
	if (!tok)
	{
		_cvi_json_c_set_last_err(
		    "cvi_json_object_from_fd: unable to allocate cvi_json_tokener(depth=%d): %s\n", depth,
		    strerror(errno));
		cvi_printbuf_free(pb);
		return NULL;
	}

	while ((ret = read(fd, buf, JSON_FILE_BUF_SIZE)) > 0)
	{
		cvi_printbuf_memappend(pb, buf, ret);
	}
	if (ret < 0)
	{
		_cvi_json_c_set_last_err("cvi_json_object_from_fd: error reading fd %d: %s\n", fd,
		                     strerror(errno));
		cvi_json_tokener_free(tok);
		cvi_printbuf_free(pb);
		return NULL;
	}

	obj = cvi_json_tokener_parse_ex(tok, pb->buf, printbuf_length(pb));
	if (obj == NULL)
		_cvi_json_c_set_last_err("cvi_json_tokener_parse_ex failed: %s\n",
		                     cvi_json_tokener_error_desc(cvi_json_tokener_get_error(tok)));

	cvi_json_tokener_free(tok);
	cvi_printbuf_free(pb);
	return obj;
}

struct cvi_json_object *cvi_json_object_from_file(const char *filename)
{
	struct cvi_json_object *obj;
	int fd;

	if ((fd = open(filename, O_RDONLY)) < 0)
	{
		_cvi_json_c_set_last_err("cvi_json_object_from_file: error opening file %s: %s\n", filename,
		                     strerror(errno));
		return NULL;
	}
	obj = cvi_json_object_from_fd(fd);
	close(fd);
	return obj;
}

/* extended "format and write to file" function */

int cvi_json_object_to_file_ext(const char *filename, struct cvi_json_object *obj, int flags)
{
	int fd, ret;
	int saved_errno;

	if (!obj)
	{
		_cvi_json_c_set_last_err("cvi_json_object_to_file: object is null\n");
		return -1;
	}

	if ((fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0644)) < 0)
	{
		_cvi_json_c_set_last_err("cvi_json_object_to_file: error opening file %s: %s\n", filename,
		                     strerror(errno));
		return -1;
	}
	ret = _cvi_json_object_to_fd(fd, obj, flags, filename);
	saved_errno = errno;
	close(fd);
	errno = saved_errno;
	return ret;
}

int cvi_json_object_to_fd(int fd, struct cvi_json_object *obj, int flags)
{
	if (!obj)
	{
		_cvi_json_c_set_last_err("cvi_json_object_to_fd: object is null\n");
		return -1;
	}

	return _cvi_json_object_to_fd(fd, obj, flags, NULL);
}

static int _cvi_json_object_to_fd(int fd, struct cvi_json_object *obj, int flags, const char *filename)
{
	int ret;
	const char *cvi_json_str;
	unsigned int wpos, wsize;

	filename = filename ? filename : "(fd)";

	if (!(cvi_json_str = cvi_json_object_to_cvi_json_string_ext(obj, flags)))
	{
		return -1;
	}

	/* CAW: probably unnecessary, but the most 64bit safe */
	wsize = (unsigned int)(strlen(cvi_json_str) & UINT_MAX);
	wpos = 0;
	while (wpos < wsize)
	{
		if ((ret = write(fd, cvi_json_str + wpos, wsize - wpos)) < 0)
		{
			_cvi_json_c_set_last_err("cvi_json_object_to_file: error writing file %s: %s\n",
			                     filename, strerror(errno));
			return -1;
		}

		/* because of the above check for ret < 0, we can safely cast and add */
		wpos += (unsigned int)ret;
	}

	return 0;
}

// backwards compatible "format and write to file" function

int cvi_json_object_to_file(const char *filename, struct cvi_json_object *obj)
{
	return cvi_json_object_to_file_ext(filename, obj, JSON_C_TO_STRING_PLAIN);
}

// Deprecated cvi_json_parse_double function.  See cvi_json_tokener_parse_double instead.
int cvi_json_parse_double(const char *buf, double *retval)
{
	char *end;
	*retval = strtod(buf, &end);
	return end == buf ? 1 : 0;
}

int cvi_json_parse_int64(const char *buf, int64_t *retval)
{
	char *end = NULL;
	int64_t val;

	val = strtoll(buf, &end, 10);
	if (end != buf)
		*retval = val;
	return (end == buf) ? 1 : 0;
}

int cvi_json_parse_uint64(const char *buf, uint64_t *retval)
{
	char *end = NULL;
	uint64_t val;

	while (*buf == ' ')
		buf++;
	if (*buf == '-')
		return 1; /* error: uint cannot be negative */

	val = strtoull(buf, &end, 10);
	if (end != buf)
		*retval = val;
	return (end == buf) ? 1 : 0;
}

#ifndef HAVE_REALLOC
void *rpl_realloc(void *p, size_t n)
{
	if (n == 0)
		n = 1;
	if (p == 0)
		return malloc(n);
	return realloc(p, n);
}
#endif

#define NELEM(a) (sizeof(a) / sizeof(a[0]))
/* clang-format off */
static const char *cvi_json_type_name[] = {
	/* If you change this, be sure to update the enum cvi_json_type definition too */
	"null",
	"boolean",
	"double",
	"int",
	"object",
	"array",
	"string",
};
/* clang-format on */

const char *cvi_json_type_to_name(enum cvi_json_type o_type)
{
	int o_type_int = (int)o_type;
	if (o_type_int < 0 || o_type_int >= (int)NELEM(cvi_json_type_name))
	{
		_cvi_json_c_set_last_err("cvi_json_type_to_name: type %d is out of range [0,%d]\n", o_type,
		                     NELEM(cvi_json_type_name));
		return NULL;
	}
	return cvi_json_type_name[o_type];
}
