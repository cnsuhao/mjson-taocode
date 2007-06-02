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

/**
The structure which holds the functions that will be used by the saxy parser whenever the evens pop up
**/
struct json_saxy_functions
{
	int (*open_object) ();
	int (*close_object) ();
	int (*open_array) ();
	int (*close_array) ();
	int (*new_string) (wchar_t * text);
	int (*new_number) (wchar_t * text);
	int (*new_true) ();
	int (*new_false) ();
	int (*new_null) ();
	int (*label_value_separator) ();
	int (*sibling_separator) ();
};


/**
The structure holding the information needed for json_saxy_parse to resume parsing
**/
struct json_saxy_parser_status
{
	unsigned int state;
	rstring *temp;
};


/**
Function to perform a SAX-like parsing of any JSON document
@param jsps a structure holding the status information of the current parser
@param jsf a structure holding the function pointers to the event functions
@param c the character to be parsed
@return a json_error code informing how the parsing went
**/
enum json_error json_saxy_parse (struct json_saxy_parser_status *jsps, struct json_saxy_functions *jsf, wchar_t c);

#endif
