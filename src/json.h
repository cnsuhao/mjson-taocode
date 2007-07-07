/***************************************************************************
 *   Copyright (C) 2007 by Rui Maciel   *
 *   rui.maciel@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/** @file json.h A small library that helps deal with JSON-encoded information
\ingroup JSON

\note error handling is only in a very rudimentary form.
\author Rui Maciel	rui_maciel@users.sourceforge.net
\version v0.5
*/

#include "rstring/rstring.h"	///todo remove this helper library in favour of regular c-string handling code

#ifndef JSON_H
#define JSON_H

#define JSON_MAX_STRING_LENGTH 4096

/**
The descriptions of the json_value node type
**/
enum json_value_type
{ JSON_STRING = 0, JSON_NUMBER, JSON_OBJECT, JSON_ARRAY, JSON_TRUE, JSON_FALSE, JSON_NULL };

/**
The error messages produced by the JSON parsers
**/
enum json_error
{
	JSON_INCOMPLETE_DOCUMENT = 0,	//!< the parsed document didn't ended
	JSON_OK = 1,		//!< everything went smoothly
	JSON_INCOMPATIBLE_TYPE,	//!< the currently parsed type does not belong here
	JSON_MEMORY,		//!< an error occurred when allocating memory
	JSON_ILLEGAL_CHARACTER,	//!< the currently parsed character does not belong here
	JSON_BAD_TREE_STRUCTURE,	//!< the currently parsed tree is malformed
	JSON_MAXIMUM_LENGTH,	//!< the parsed string reached the maximum allowed size
	JSON_UNKNOWN_PROBLEM	//!< some random, unaccounted problem occurred
};

/**
The JSON document tree node, which is a basic JSON type
**/
struct json_value
{
	enum json_value_type type;	//!< the type of node
	rstring *text;		//!< The text stored by the node. It is used exclusively by the JSON_STRING and JSON_NUMBER node types

	// FIFO queue data
	struct json_value *next;	//!< The pointer pointing to the next element in the FIFO sibling list
	struct json_value *previous;	//!< The pointer pointing to the previous element in the FIFO sibling list
	struct json_value *parent;	//!< The pointer pointing to the parent node in the document tree
	struct json_value *child;	//!< The pointer pointing to the first child node in the document tree
	struct json_value *child_end;	//!< The pointer pointing to the last child node in the document tree
};

/**
The structure holding all information needed to resume parsing
**/
struct json_parsing_info
{
	unsigned int state;	//!< the state where the parsing was left on the last parser run
	struct json_value *cursor;	//!< pointers to nodes belonging to the document tree which aid the document parsing
	struct json_value *temp;	//!< temporary node which the parser uses to build up the parsed document
};


/**
The structure which holds the pointers to the functions that will be called by the saxy parser whenever their evens are triggered
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
	unsigned int state;	//!< current parser state
	rstring *temp;		//!< temporary string which will be used to build up parsed strings between parser runs.
};


/**
Creates a new JSON value and defines it's type
@param type the value's type
@return a pointer to the newly created value structure
**/
struct json_value *json_new_value (enum json_value_type type);


/**
Creates a new JSON string and defines it's text
@param text the value's text
@return a pointer to the newly created JSON string value
**/
struct json_value *json_new_string (wchar_t * text);


/**
Creates a new JSON number and defines it's text
@param text the value's number
@return a pointer to the newly created JSON string value
**/
struct json_value *json_new_number (wchar_t * text);


/**
Creates a new JSON object
@return a pointer to the newly created JSON object value
**/
struct json_value *json_new_object (void);


/**
Creates a new JSON array
@return a pointer to the newly created JSON array value
**/
struct json_value *json_new_array (void);


/**
Creates a new JSON null
@return a pointer to the newly created JSON null value
**/
struct json_value *json_new_null (void);


/**
Creates a new JSON true
@return a pointer to the newly created JSON true value
**/
struct json_value *json_new_true (void);


/**
Creates a new JSON false
@return a pointer to the newly created JSON false value
**/
struct json_value *json_new_false (void);


/**
Frees the memory appointed to the value fed as the parameter, as well as all the child nodes
@param value the root node of the tree being freed
**/
void json_free_value (struct json_value **value);


/**
Inserts a child node into a parent node, as well as performs some document tree integrity checks.
@param parent the parent node
@param child the node being added as a child to parent
@return the error code corresponding to the operation result
**/
enum json_error json_insert_child (struct json_value *parent, struct json_value *child);


/**
Inserts a label:value pair into a parent node, as well as performs some document tree integrity checks.
@param parent the parent node
@param text_label a wchar_t string which serves as the label in the label:value pair
@param value the value in the label:value pair
@return the error code corresponding to the operation result
**/
enum json_error json_insert_pair_into_object (struct json_value *parent, wchar_t * text_label, struct json_value *value);


/**
Renders the tree structure where root is the tree's root, which can also be a tree branch. This function is used recursively by json_render_tree()
@param root the tree's root node (may be a child node)
@param level the indentation level (number of tabs)
**/
void json_render_tree_indented (struct json_value *root, int level);


/**
Renders the tree structure where root is the tree's root, which can also be a tree branch.
@param root the tree's root node (may be a child node)
**/
void json_render_tree (struct json_value *root);


/**
Produces a JSON markup text document from a document tree
@param root The document's root node
@param text a pointer to a wchar_t string that will hold the JSON document text.
@return  a json_error code describing how the operation went
**/
enum json_error json_tree_to_string (struct json_value *root, wchar_t ** text);


/**
Checks if the character in question is a JSON markup white space
@param c the character to be analized
@return 1 if it is, 0 if it isn't
**/
int json_white_space (const wchar_t c);


/**
Strips all JSON white spaces from the given string
@param text a wchar_t string holding a JSON formatted document
@return a wchar_t string holding the stripped down document
**/
wchar_t *json_strip_white_spaces (wchar_t * text);


/**
Formats a JSON markup text contained in the given string
@param text a JSON formatted document
@return a wchar_t string holding the formated document
**/
wchar_t *json_format_string (wchar_t * text);


/**
Outputs a new wchar_t string which holds the same characters as the given string but replaces all escapable characters the respective escape sequence.
Please notice that this function produces a new string separate from wchar_t *text which, if unaccounted for, may contribute to memory leaks.
@param text a wchar_t text string
@return a wchar_t string holding the same text string but with escaped characters
**/
wchar_t *json_escape (wchar_t * text);


/**
This function performs the same tast as json_escape() but it also escapes non-ASCII characters
As with json_escape(), the produced string, if unaccounted for, may contribute to memory leaks.
@param text a wchar_t text string
@return a wchar_t string holding the same text string but composed of ASCII characters
**/
wchar_t *json_escape_to_ascii (wchar_t * text);


/**
Produces a document tree from a JSON markup text string
@param info the information necessary to resume parsing any incomplete document
@param text a text string containing information described by the JSON language, partial or complete.
@return a code describing how the operation ended up
**/
enum json_error json_parse_string (struct json_parsing_info *info, wchar_t * text);


/**
Produces a document tree from a JSON markup text string that contains a complete document
@param text a text string containing a complete JSON text document
@return a pointer to the new document tree or NULL if some error occurred
**/
struct json_value *json_parse_document (wchar_t * text);

/**
Function to perform a SAX-like parsing of any JSON document or document fragment that is passed to it
@param jsps a structure holding the status information of the current parser
@param jsf a structure holding the function pointers to the event functions
@param c the character to be parsed
@return a json_error code informing how the parsing went
**/
enum json_error json_saxy_parse (struct json_saxy_parser_status *jsps, struct json_saxy_functions *jsf, wchar_t c);

#endif
