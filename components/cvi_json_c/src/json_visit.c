/*
 * Copyright (c) 2016 Eric Haszlakiewicz
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See COPYING for details.
 */

#include <stdio.h>

#include "json_define.h"
#include "json_inttypes.h"
#include "json_object.h"
#include "json_visit.h"
#include "linkhash.h"

static int _cvi_json_c_visit(cvi_json_object *jso, cvi_json_object *parent_jso, const char *jso_key,
                         size_t *jso_index, cvi_json_c_visit_userfunc *userfunc, void *userarg);

int cvi_json_c_visit(cvi_json_object *jso, int future_flags, cvi_json_c_visit_userfunc *userfunc, void *userarg)
{
	int ret = _cvi_json_c_visit(jso, NULL, NULL, NULL, userfunc, userarg);
	switch (ret)
	{
	case JSON_C_VISIT_RETURN_CONTINUE:
	case JSON_C_VISIT_RETURN_SKIP:
	case JSON_C_VISIT_RETURN_POP:
	case JSON_C_VISIT_RETURN_STOP: return 0;
	default: return JSON_C_VISIT_RETURN_ERROR;
	}
}
static int _cvi_json_c_visit(cvi_json_object *jso, cvi_json_object *parent_jso, const char *jso_key,
                         size_t *jso_index, cvi_json_c_visit_userfunc *userfunc, void *userarg)
{
	int userret = userfunc(jso, 0, parent_jso, jso_key, jso_index, userarg);
	switch (userret)
	{
	case JSON_C_VISIT_RETURN_CONTINUE: break;
	case JSON_C_VISIT_RETURN_SKIP:
	case JSON_C_VISIT_RETURN_POP:
	case JSON_C_VISIT_RETURN_STOP:
	case JSON_C_VISIT_RETURN_ERROR: return userret;
	default:
		fprintf(stderr, "ERROR: invalid return value from cvi_json_c_visit userfunc: %d\n",
		        userret);
		return JSON_C_VISIT_RETURN_ERROR;
	}

	switch (cvi_json_object_get_type(jso))
	{
	case cvi_json_type_null:
	case cvi_json_type_boolean:
	case cvi_json_type_double:
	case cvi_json_type_int:
	case cvi_json_type_string:
		// we already called userfunc above, move on to the next object
		return JSON_C_VISIT_RETURN_CONTINUE;

	case cvi_json_type_object:
	{
		cvi_json_object_object_foreach(jso, key, child)
		{
			userret = _cvi_json_c_visit(child, jso, key, NULL, userfunc, userarg);
			if (userret == JSON_C_VISIT_RETURN_POP)
				break;
			if (userret == JSON_C_VISIT_RETURN_STOP ||
			    userret == JSON_C_VISIT_RETURN_ERROR)
				return userret;
			if (userret != JSON_C_VISIT_RETURN_CONTINUE &&
			    userret != JSON_C_VISIT_RETURN_SKIP)
			{
				fprintf(stderr, "INTERNAL ERROR: _cvi_json_c_visit returned %d\n",
				        userret);
				return JSON_C_VISIT_RETURN_ERROR;
			}
		}
		break;
	}
	case cvi_json_type_array:
	{
		size_t array_len = cvi_json_object_array_length(jso);
		size_t ii;
		for (ii = 0; ii < array_len; ii++)
		{
			cvi_json_object *child = cvi_json_object_array_get_idx(jso, ii);
			userret = _cvi_json_c_visit(child, jso, NULL, &ii, userfunc, userarg);
			if (userret == JSON_C_VISIT_RETURN_POP)
				break;
			if (userret == JSON_C_VISIT_RETURN_STOP ||
			    userret == JSON_C_VISIT_RETURN_ERROR)
				return userret;
			if (userret != JSON_C_VISIT_RETURN_CONTINUE &&
			    userret != JSON_C_VISIT_RETURN_SKIP)
			{
				fprintf(stderr, "INTERNAL ERROR: _cvi_json_c_visit returned %d\n",
				        userret);
				return JSON_C_VISIT_RETURN_ERROR;
			}
		}
		break;
	}
	default:
		fprintf(stderr, "INTERNAL ERROR: _cvi_json_c_visit found object of unknown type: %d\n",
		        cvi_json_object_get_type(jso));
		return JSON_C_VISIT_RETURN_ERROR;
	}

	// Call userfunc for the second type on container types, after all
	//  members of the container have been visited.
	// Non-container types will have already returned before this point.

	userret = userfunc(jso, JSON_C_VISIT_SECOND, parent_jso, jso_key, jso_index, userarg);
	switch (userret)
	{
	case JSON_C_VISIT_RETURN_SKIP:
	case JSON_C_VISIT_RETURN_POP:
		// These are not really sensible during JSON_C_VISIT_SECOND,
		// but map them to JSON_C_VISIT_CONTINUE anyway.
		// FALLTHROUGH
	case JSON_C_VISIT_RETURN_CONTINUE: return JSON_C_VISIT_RETURN_CONTINUE;
	case JSON_C_VISIT_RETURN_STOP:
	case JSON_C_VISIT_RETURN_ERROR: return userret;
	default:
		fprintf(stderr, "ERROR: invalid return value from cvi_json_c_visit userfunc: %d\n",
		        userret);
		return JSON_C_VISIT_RETURN_ERROR;
	}
	// NOTREACHED
}
