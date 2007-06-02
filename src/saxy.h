//
// C++ Interface: saxy
//
// Description:
//
//
// Author: Rui Maciel <rui_maciel@users.sourceforge.net>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "json.h"
#include "rstring/rstring.h"

#ifndef SAXY_H
#define SAXY_H

struct json_saxy_functions
{
	int (*open_object)();
	int (*close_object)();
	int (*open_array)();
	int (*close_array)();
	int (*new_string)(wchar_t *text);
	int (*new_number)(wchar_t *text);
	int (*new_true)();
	int (*new_false)();
	int (*new_null)();
	int (*label_value_separator)();
	int (*sibling_separator)();
};

struct json_saxy_parser_status
{
	unsigned int state;
	rstring *temp;
};

enum json_error json_saxy_parse(struct json_saxy_parser_status *jsps, struct json_saxy_functions *jsf, wchar_t c);

#endif
