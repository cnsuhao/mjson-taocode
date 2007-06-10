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

#include "json.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <wchar.h>


struct json_value *
json_new_value (enum json_value_type type)
{
	struct json_value *new_object;
	// allocate memory to the new object
	new_object = malloc (sizeof (struct json_value));
	if (new_object == NULL)
		return NULL;

	// initialize members
	new_object->text = NULL;
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next = NULL;
	new_object->type = type;
	return new_object;
}


struct json_value *
json_new_string (wchar_t * text)
{
	assert (text != NULL);

	struct json_value *new_object;
	// allocate memory to the new object
	new_object = malloc (sizeof (struct json_value));
	if (new_object == NULL)
		return NULL;

	// initialize members
	new_object->text = rs_create (text);
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next = NULL;
	new_object->type = JSON_STRING;
	return new_object;
}


struct json_value *
json_new_number (wchar_t * text)
{
	assert (text != NULL);

	//TODO enforce number string correctness or leave it to the user?

	struct json_value *new_object;
	// allocate memory to the new object
	new_object = malloc (sizeof (struct json_value));
	if (new_object == NULL)
		return NULL;

	// initialize members
	new_object->text = rs_create (text);
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next = NULL;
	new_object->type = JSON_NUMBER;
	return new_object;
}


struct json_value *
json_new_object (void)
{
	return json_new_value (JSON_OBJECT);
}


struct json_value *
json_new_array (void)
{
	return json_new_value (JSON_ARRAY);
}


struct json_value *
json_new_null (void)
{
	return json_new_value (JSON_NULL);
}


struct json_value *
json_new_true (void)
{
	return json_new_value (JSON_TRUE);
}


struct json_value *
json_new_false (void)
{
	return json_new_value (JSON_FALSE);
}


void
json_free_value (struct json_value **value)
{
	assert ((*value) != NULL);

	// free each and every child nodes
	if ((*value)->child != NULL)
	{
		///fixme write function to free entire subtree recursively
		struct json_value *i, *j;
		i = (*value)->child_end;
		while (i != NULL)
		{
			j = i->previous;
			json_free_value (&i);
			i = j;
		}
	}

	// fixing sibling linked list connections
	if ((*value)->previous && (*value)->next)
	{
		(*value)->previous->next = (*value)->next;
		(*value)->next->previous = (*value)->previous;
	}
	if ((*value)->previous)
	{
		(*value)->previous->next = NULL;
	}
	if ((*value)->next)
	{
		(*value)->next->previous = NULL;
	}

	//fixing parent node connections
	if ((*value)->parent)
	{
		if ((*value)->parent->child == (*value))
		{
			if ((*value)->next)
			{
				(*value)->parent->child = (*value)->next;	// the parent node always points to the first node
			}
			else
			{
				if ((*value)->previous)
					(*value)->parent->child = (*value)->next;	// the parent node always points to the first node
				(*value)->parent->child = NULL;
			}
		}
	}

	//finally, freeing the memory allocated for this value
	if ((*value)->text != NULL)
	{
		rs_destroy (&(*value)->text);	// the string
	}
	free ((*value));	// the json value
	(*value) = NULL;
}


enum json_error
json_insert_child (struct json_value *parent, struct json_value *child)
{
	assert (parent != NULL);	// the parent must exist
	assert (child != NULL);	// the child must exist
	assert (parent != child);	// parent and child must not be the same. if they are, it will enter an infinite loop
	assert ((parent->type == JSON_OBJECT) || (parent->type == JSON_ARRAY) || (parent->type == JSON_STRING));	// must be a valid parent type
	///todo implement a way to enforce object->text->value sequence
	assert (!(parent->type == JSON_OBJECT && child->type == JSON_OBJECT));

	child->parent = parent;
	if (parent->child)
	{
		child->previous = parent->child_end;
		parent->child_end->next = child;
		parent->child_end = child;
	}
	else
	{
		parent->child = child;
		parent->child_end = child;
	}

	return JSON_OK;
}


enum json_error
json_insert_pair_into_object (struct json_value *parent, struct json_value *label, struct json_value *value)
{
	// verify if the parameters are valid
	assert (parent != NULL);
	assert (label != NULL);
	assert (value != NULL);
	assert (parent != label);
	assert (parent != value);
	assert (label != value);

	// enforce type coherence
	assert (parent->type == JSON_OBJECT);
	assert (label->type == JSON_STRING);

	enum json_error error;

	//insert value and check for error
	error = json_insert_child (label, value);
	if (error != JSON_OK)
		return error;
	//insert value and check for error
	error = json_insert_child (parent, label);
	if (error != JSON_OK)
		return error;

	return JSON_OK;
}


void
json_render_tree_indented (struct json_value *root, int level)
{
	assert (root != NULL);
	int tab;
	for (tab = 0; tab < level; tab++)
	{
		printf ("> ");
	}
	switch (root->type)
	{
	case JSON_STRING:
		printf ("STRING: %ls\n", root->text->s);
		break;
	case JSON_NUMBER:
		printf ("NUMBER: %ls\n", root->text->s);
		break;
	case JSON_OBJECT:
		printf ("OBJECT: \n");
		break;
	case JSON_ARRAY:
		printf ("ARRAY: \n");
		break;
	case JSON_TRUE:
		printf ("TRUE:\n");
		break;
	case JSON_FALSE:
		printf ("FALSE:\n");
		break;
	case JSON_NULL:
		printf ("NULL:\n");
		break;
	}
	//iterate through children
	if (root->child != NULL)
	{
		struct json_value *ita, *itb;
		ita = root->child;
		while (ita != NULL)
		{
			json_render_tree_indented (ita, level + 1);
			itb = ita->next;
			ita = itb;
		}
	}
}


void
json_render_tree (struct json_value *root)
{
	json_render_tree_indented (root, 0);
}


wchar_t *
json_tree_to_string (struct json_value *root)	///fixme this function leaks memory
{
	assert (root != NULL);

	struct json_value *cursor = root;
	// set up the output string
	rstring *output = rs_create (L"");
	if (output == NULL)
		return NULL;

	// start the convoluted fun
      state1:			// open value
	{
		if ((cursor->previous) && (cursor != root))	//if cursor is children and not root than it is a followup sibling
		{
			if (rs_catwc (output, L',') != RS_OK)
				goto error;
		}
		switch (cursor->type)
		{
		case JSON_STRING:
			if (rs_catwc (output, L'\"') != RS_OK)
				goto error;
			if (rs_catrs (output, cursor->text) != RS_OK)
				goto error;
			if (rs_catwc (output, L'\"') != RS_OK)
				goto error;

			if (cursor->parent != NULL)
			{
				if (cursor->parent->type == JSON_OBJECT)	// cursor is label in label:value pair
				{
					// error checking: if parent is object and cursor is string then cursor must have a single child
					if (cursor->child != NULL)
					{
						if (rs_catwc (output, L':') != RS_OK)
							goto error;
					}
					else
					{
						// malformed document tree: label without value in label:value pair
						printf ("Tree integrity error: string as object children must be label:value pair\n");
						goto error;
					}
				}
			}
			else	// does not have a parent
			{
				if (cursor->child != NULL)	// is root label in label:value pair
				{
					if (rs_catwc (output, L':') != RS_OK)
						goto error;
				}
				else
				{
					// malformed document tree: label without value in label:value pair
					goto error;	// no root but siblings
				}
			}
			break;

		case JSON_NUMBER:
			// must not have any children
			if (rs_catrs (output, cursor->text) != RS_OK)
				goto error;
			goto state2;	// close value
			break;

		case JSON_OBJECT:
			if (rs_catwc (output, L'{') != RS_OK)
				goto error;

			if (cursor->child)
			{
				cursor = cursor->child;
				goto state1;
			}
			else
			{
				goto state2;	// close value
			}
			break;

		case JSON_ARRAY:
			if (rs_catwc (output, L'[') != RS_OK)
				goto error;
			if (cursor->child != NULL)
			{
				cursor = cursor->child;
				goto state1;
			}
			else
			{
				goto state2;	// close value
			}
			break;

		case JSON_TRUE:
			// must not have any children
			if (rs_catwcs (output, L"true", 4) != RS_OK)
				goto error;
			goto state2;	// close value
			break;

		case JSON_FALSE:
			// must not have any children
			if (rs_catwcs (output, L"false", 5) != RS_OK)
				goto error;
			goto state2;	// close value
			break;

		case JSON_NULL:
			// must not have any children
			if (rs_catwcs (output, L"null", 5) != RS_OK)
				goto error;
			goto state2;	// close value
			break;

		default:
			goto error;
		}
		if (cursor->child)
		{
			cursor = cursor->child;
			goto state1;
		}
		else
		{
			// does not have any children
			goto state2;	// close value
		}
	}

      state2:			// close value
	{
		switch (cursor->type)
		{
		case JSON_OBJECT:
			if (rs_catwc (output, L'}') != RS_OK)
				goto error;
			break;

		case JSON_ARRAY:
			if (rs_catwc (output, L']') != RS_OK)
				goto error;
			break;

		case JSON_STRING:
			break;
		case JSON_NUMBER:
			break;
		case JSON_TRUE:
			break;
		case JSON_FALSE:
			break;
		case JSON_NULL:
			break;
		default:
			goto error;
		}
		if ((cursor->parent == NULL) || (cursor == root))
		{
			goto end;
		}
		else if (cursor->next)
		{
			cursor = cursor->next;
			goto state1;
		}
		else
		{
			cursor = cursor->parent;
			goto state2;	// close value
		}
	}

      error:
	{
		printf ("ERROR!");
		rs_destroy (&output);
		return NULL;	///todo implement better, usable error handling
	}

      end:
	return rs_unwrap (output);
}


int
json_white_space (const wchar_t c)
{
	switch (c)
	{
	case L'\x20':
	case L'\x09':
	case L'\x0A':
	case L'\x0D':
		return 1;
	default:
		return 0;
	}
}


wchar_t *
json_strip_white_spaces (wchar_t * text)	///fixit this function should not strip white spaces from JSON strings
{
	assert (text != NULL);
	// declaring the variables
	rstring *txt = rs_wrap (text);
	if (txt == NULL)
		return NULL;	// failed memory allocation
	size_t pos = 0;
	rstring *output = rs_create (L"");
	if (output == NULL)
	{
		rs_unwrap (txt);
		return NULL;
	}

	while (pos < txt->length)
	{
		switch (txt->s[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// JSON white spaces
			pos++;
			break;

		case L'\"':	//open string
			if (rs_catwc (output, txt->s[pos]) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			char loop = 1;	// inner string loop trigger
			while (loop)	// parse the inner part of the string
			{
				if (txt->s[pos] == L'\\')	// escaped sequence
				{
					if (rs_catwc (output, txt->s[pos]) != RS_OK)
					{
						rs_unwrap (txt);
						rs_destroy (&output);
						return NULL;
					}
					pos++;
					if (txt->s[pos] == L'\"')	// don't consider a \" escaped sequence as an end of string
					{

						if (rs_catwc (output, txt->s[pos]) != RS_OK)
						{
							rs_unwrap (txt);
							rs_destroy (&output);
							return NULL;
						}
						pos++;
					}
				}
				else if (txt->s[pos] == L'\"')	// reached end of string
				{
					loop = 0;
				}

				if (rs_catwc (output, txt->s[pos]) != RS_OK)
				{
					rs_unwrap (txt);
					rs_destroy (&output);
					return NULL;
				}
				pos++;
				if (pos >= txt->length)
					loop = 0;
			}
			break;

		default:
			if (rs_catwc (output, txt->s[pos]) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			break;
		}
	}
	rs_unwrap (txt);	// free memory allocated to wrapper
	return rs_unwrap (output);
}


wchar_t *
json_format_string (wchar_t * text)
{
	size_t pos = 0;
	unsigned int indentation = 0, i;
	rstring *txt = rs_wrap (text);
	if (txt == NULL)
		return NULL;
	rstring *output = rs_create (L"");
	if (output == NULL)
	{
		rs_unwrap (txt);
		return NULL;
	}
	while (pos < txt->length)
	{
		switch (txt->s[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// JSON white spaces
			pos++;
			break;

		case L'{':
			if (rs_catwcs (output, L"{\n", 2) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			indentation++;
			for (i = 0; i < indentation; i++)
			{
				if (rs_catwc (output, L'\t') != RS_OK)
				{
					rs_unwrap (txt);
					rs_destroy (&output);
					return NULL;
				}
			}
			break;

		case L'}':
			if (rs_catwc (output, L'\n') != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			indentation--;
			for (i = 0; i < indentation; i++)
			{
				if (rs_catwc (output, L'\t') != RS_OK)
				{
					rs_unwrap (txt);
					rs_destroy (&output);
					return NULL;
				}
			}
			if (rs_catwc (output, L'}') != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			break;

		case L':':
			if (rs_catwcs (output, L": ", 2) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			break;

		case L',':
			if (rs_catwcs (output, L",\n", 2) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			for (i = 0; i < indentation; i++)
			{
				rs_catwc (output, L'\t');
			}
			break;

		case L'\"':	//open string
			if (rs_catwc (output, txt->s[pos]) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			char loop = 1;	// inner string loop trigger
			while (loop)	// parse the inner part of the string
			{
				if (txt->s[pos] == L'\\')	// escaped sequence
				{
					if (rs_catwc (output, txt->s[pos]) != RS_OK)
					{
						rs_unwrap (txt);
						rs_destroy (&output);
						return NULL;
					}
					pos++;
					if (txt->s[pos] == L'\"')	// don't consider a \" escaped sequence as an end of string
					{

						if (rs_catwc (output, txt->s[pos]) != RS_OK)
						{
							rs_unwrap (txt);
							rs_destroy (&output);
							return NULL;
						}
						pos++;
					}
				}
				else if (txt->s[pos] == L'\"')	// reached end of string
				{
					loop = 0;
				}

				if (rs_catwc (output, txt->s[pos]) != RS_OK)
				{
					rs_unwrap (txt);
					rs_destroy (&output);
					return NULL;
				}
				pos++;
				if (pos >= txt->length)
					loop = 0;
			}
			break;

		default:
			if (rs_catwc (output, txt->s[pos]) != RS_OK)
			{
				rs_unwrap (txt);
				rs_destroy (&output);
				return NULL;
			}
			pos++;
			break;
		}
	}

	rs_unwrap (txt);
	return rs_unwrap (output);
}


wchar_t *
json_escape_string (wchar_t * text)
{
	rstring *output = rs_create (L"");
	if (output == NULL)
		return NULL;

	size_t i;
	for (i = 0; i < wcslen (text); i++)
	{

		if (text[i] == L'\\')
			rs_catwcs (output, L"\\\\", 2);
		else if (text[i] == '\"')
			rs_catwcs (output, L"\\\"", 2);
		else if (text[i] == L'/')
			rs_catwcs (output, L"\\/", 2);
		else if (text[i] == L'\b')
			rs_catwcs (output, L"\\b", 2);
		else if (text[i] == L'\f')
			rs_catwcs (output, L"\\f", 2);
		else if (text[i] == L'\n')
			rs_catwcs (output, L"\\n", 2);
		else if (text[i] == L'\r')
			rs_catwcs (output, L"\\r", 2);
		else if (text[i] == L'\t')
			rs_catwcs (output, L"\\t", 2);
		else if ((text[i] >= 0x20) && (text[i] <= 0x7E))	// ascii printable characters
		{
			rs_catwc (output, text[i]);
		}
		else		// beyond ascii characters?
		{
			wchar_t *temp = malloc (7 * sizeof (wchar_t));
			swprintf (temp, 7, L"\\u%.4x\t", text[i]);
			rs_catwcs (output, temp, 6);
			free (temp);
		}
	}

	return rs_unwrap (output);
}


enum json_error
json_parse_string (struct json_parsing_info *info, wchar_t * text)
{
	///todo sanitize the state numbers.
	/*
	   redundant states which were eliminated:
	   state22
	   state28
	   state33
	 */

	assert (info != NULL);
	// setup the initial values
	size_t pos = 0;
	size_t length = wcslen (text);

	// go to the state that we should be to resume parsing
	switch (info->state)
	{
	case 0:
		goto state0;	// general purpose state: starting point
	case 1:
		goto state1;	// start value in object
	case 2:
		goto state2;	// start object
	case 3:
		goto state3;	// end object
	case 4:
		goto state4;	// end array
	case 5:
		goto state5;	// start string
	case 6:
		goto state6;	// continue string
	case 7:
		goto state7;	// continue string: escaped character
	case 8:
		goto state8;	// continue string: escaped unicode character 1 of 4
	case 9:
		goto state9;	// continue string: escaped unicode character 2 of 4
	case 10:
		goto state10;	// continue string: escaped unicode character 3 of 4
	case 11:
		goto state11;	// continue string: escaped unicode character 4 of 4
	case 12:
		goto state12;	// string followup
	case 13:
		goto state13;	// pair
	case 14:
		goto state14;	// start number
	case 15:
		goto state15;	// number: leading zero
	case 16:
		goto state16;	// number: decimal part
	case 17:
		goto state17;	// number: start fractional part
	case 18:
		goto state18;	// number: fractional part
	case 19:
		goto state19;	// number: start exponential part
	case 20:
		goto state20;	// number: exponential part following signal
	case 21:
		goto state21;	// number: exponential part
	case 23:
		goto state23;	// value followup
	case 24:
		goto state24;	// sibling
	case 25:
		goto state25;	// true: t
	case 26:
		goto state26;	// true: r
	case 27:
		goto state27;	// true: u
//      case 28:
//              goto state28;   // true: e
	case 29:
		goto state29;	// false: f
	case 30:
		goto state30;	// false: a
	case 31:
		goto state31;	// false: l
	case 32:
		goto state32;	// false: s
	case 34:
		goto state34;	// null: n
	case 35:
		goto state35;	// null: u
	case 36:
		goto state36;	// null: l 1 of 2
	case 37:
		goto state37;	// start array
	case 38:
		goto state38;	// fix
	case 39:
		goto state39;	// start value in array

	default:
		return JSON_SOME_PROBLEM;	// IF THIS PART IS REACHED THEN THERE IS A BUG SOMEWHERE
		break;
	}


	// let's start the juicy bits

      state0:			// general purpose state: starting point
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 0;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state0;
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state2;	// start object
			break;

		case L'[':
			info->state = 37;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state37;	// start object
			break;

		case L'\"':
			info->state = 5;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state5;	// start string
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}

	}

      state1:			// start value in object
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 1;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state1;
			break;

		case L'}':
			info->state = 3;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state3;	// end object
			break;

		case L'\"':
			info->state = 5;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state5;	// start string
			break;

		case L',':
			if (info->cursor == NULL)
				return JSON_ILLEGAL_CHARACTER;
			if (info->cursor->child_end == NULL)
				return JSON_ILLEGAL_CHARACTER;
			info->state = 24;	// sibling
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state24;	// sibling
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
	}

      state2:			// start object
	{
		// tree integrity checking
		if (info->cursor)	// cursor will be the parent node of this structure
		{
			if ((info->cursor->type != JSON_ARRAY) && (info->cursor->type != JSON_STRING))
			{
				return JSON_ILLEGAL_CHARACTER;
			}
		}

		// create the new children objects
		info->temp = json_new_object ();

		// create new tree node and add as child
		if (info->cursor != NULL)
		{
			json_insert_child (info->cursor, info->temp);
		}
		// traverse cursor up child node
		info->cursor = info->temp;
		info->temp = NULL;

		info->state = 1;
		goto state1;	// start value in object
	}

      state3:			// end object
	{
		// verify tree integrity
		if (info->cursor == NULL)
			return JSON_ILLEGAL_CHARACTER;
		if (info->cursor->type != JSON_OBJECT)
			return JSON_ILLEGAL_CHARACTER;
		if (info->cursor->parent == NULL)
		{
			return JSON_OK;
		}

		// move on down
		info->cursor = info->cursor->parent;

		// check if we descended into the root node
		switch (info->cursor->type)
		{
		case JSON_STRING:
			if (info->cursor->parent == NULL)
				return JSON_OK;
			else
				info->cursor = info->cursor->parent;	///todo test this
			break;

		case JSON_ARRAY:
			///todo finish this step
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}

		// proceed to the next state
		switch (info->cursor->type)
		{
		case JSON_OBJECT:
			info->state = 1;
			goto state1;	// start value in object
			break;

		case JSON_ARRAY:
			info->state = 39;
			goto state39;	// start value in array
			break;

		default:
			return JSON_BAD_TREE_STRUCTURE;	///TODO is this the right return code?
		}
	}

      state4:			// end array
	{
		// verify tree integrity
		if (info->cursor == NULL)
			return JSON_ILLEGAL_CHARACTER;
		if (info->cursor->type != JSON_ARRAY)
			return JSON_ILLEGAL_CHARACTER;
		if (info->cursor->parent == NULL)
		{
			return JSON_OK;
		}

		// move on down
		info->cursor = info->cursor->parent;

		// check if we descended into the root node
		switch (info->cursor->type)
		{
		case JSON_STRING:
			if (info->cursor->parent == NULL)
				return JSON_OK;
			else
				info->cursor = info->cursor->parent;	///todo test this
			break;

		case JSON_ARRAY:
			///todo finish this step
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}

		// proceed to the next state
		switch (info->cursor->type)
		{
		case JSON_OBJECT:
			info->state = 1;
			goto state1;	// start value in object
			break;

		case JSON_ARRAY:
			info->state = 39;
			goto state39;	// start value in array
			break;

		default:
			return JSON_BAD_TREE_STRUCTURE;	///TODO is this the right return code?
		}
	}

      state5:			// start string
	{
		// verify tree integrity
		if (info->cursor != NULL)
		{
			switch (info->cursor->type)
			{
			case JSON_OBJECT:
			case JSON_ARRAY:
				break;
			case JSON_STRING:
				if (info->cursor->parent)
				{
					if (info->cursor->parent->type != JSON_OBJECT)	//a parent of a parent string must be an object
						return JSON_ILLEGAL_CHARACTER;
				}
				break;

			default:	// a string can't be a child of a value type other than object, array or string
				return JSON_ILLEGAL_CHARACTER;
			}
		}

		// prepare the temporary node to get the string
		if (info->temp == NULL)
		{
			info->temp = json_new_string (L"");
			if (info->temp == NULL)
				return JSON_MEMORY;
		}

		// move to the next state
		info->state = 6;
		goto state6;	// continue string
	}

      state6:			// continue string
	{
		switch (text[pos])
		{
		case L'\\':	// escaped characters
			if (rs_catwc (info->temp->text, L'\\') != RS_OK)
				return JSON_MEMORY;

			info->state = 7;	// continue string: escaped character
			pos++;
			if (pos >= length)
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state7;	// continue string: escaped character
			break;

		case L'"':	// closing string
			if (info->cursor == NULL)
			{
				if (info->temp == NULL)
				{
					///TODO does this need some memory cleanup?
					return JSON_SOME_PROBLEM;
				}
				info->cursor = info->temp;
				info->temp = NULL;
			}
			else
			{
				json_insert_child (info->cursor, info->temp);
				// return the cursor to a sane place
				switch (info->cursor->type)
				{
				case JSON_ARRAY:
					///todo finish this
					break;

				case JSON_STRING:
					if (info->cursor->parent != NULL)
					{
						// todo perform tree sanity check
						switch (info->cursor->parent->type)
						{
						case JSON_OBJECT:
						case JSON_ARRAY:
							break;
						default:
							return JSON_BAD_TREE_STRUCTURE;
							break;
						}
						// set cursor to continue parsing
						info->cursor = info->cursor->parent;
					}
					else
					{
						/// TODO clean up info structure
						return JSON_OK;
					}
					break;

				case JSON_OBJECT:
					info->cursor = info->temp;	// point cursor at inserted child label
					break;

				default:
					return JSON_BAD_TREE_STRUCTURE;
				}
				info->temp = NULL;
			}
			info->state = 12;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state12;	// goto string followup
			break;


		default:
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 6;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state6;
		}

	}

      state7:			// continue string: escaped characters
	{
		switch (text[pos])
		{
		case L'\"':
		case L'\\':
		case L'/':
		case L'b':
		case L'f':
		case L'n':
		case L'r':
		case L't':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 6;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state6;	// continue string
			break;

		case L'u':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 8;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state8;	// continue string: escaped unicode character 1
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state8:			// continue string: escaped unicode character 1
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		info->state = 9;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state9;	// continue string: escaped unicode character 2
	}

      state9:			// continue string: escaped unicode character 2
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		info->state = 10;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state10;	// continue string: escaped unicode character 3
	}

      state10:			// continue string: escaped unicode character 3
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		info->state = 11;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state11;	// continue string: escaped unicode character 3
	}

      state11:			// continue string: escaped unicode character 4
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		info->state = 6;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state6;	// continue string
	}

      state12:			// string followup
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 12;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state12;
			break;

		case L'}':	// end object
			info->state = 3;
			pos++;
			if (pos > length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state3;	// end object
			break;

		case L']':	// end array
			info->state = 4;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state4;	// end array
			break;

		case L':':
			info->state = 13;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state13;	// pair
			break;

		case L',':
			info->state = 24;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state24;	// sibling
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state13:			// pair
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 13;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state13;	// restart loop
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state2;	// start object
			break;

		case L'[':
			info->state = 37;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state37;
			break;


		case L'\"':
			info->state = 5;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state5;	// start string

		case L'-':
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			info->state = 14;
			goto state14;	// number
			break;

		case L't':
			info->state = 25;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state25;	// true
			break;

		case L'f':
			info->state = 29;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state29;	// false
			break;

		case L'n':
			info->state = 34;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state34;	// null
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state14:			// start number
	{
		info->temp = json_new_number (L"");
		// start number
		switch (text[pos])
		{
		case L'0':
			if (rs_catwc (info->temp->text, L'0') != RS_OK)
				return JSON_MEMORY;
			info->state = 15;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state15;	// number: leading zero
			break;

		case L'-':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 16;	// number: decimal part
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state16;	// number: decimal part
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state15:			// number: leading zero
	{
		switch (text[pos])
		{
		case L'.':
			if (rs_catwc (info->temp->text, L'.') != RS_OK)
				return JSON_MEMORY;
			info->state = 17;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state17;	// number: start fractional part

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
		case L',':
		case L'}':
		case L']':
			info->state = 38;
			goto state38;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state16:			// number: decimal part
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 16;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state16;	// number: decimal part
			break;

		case L'.':
			if (rs_catwc (info->temp->text, L'.') != RS_OK)
				return JSON_INCOMPLETE_DOCUMENT;
			info->state = 17;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state17;	// number: start fractional part

		case L'e':
		case L'E':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 19;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state19;	// start exponential

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
		case L',':
		case L'}':
		case L']':
			info->state = 38;
			goto state38;	// number: end
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state17:			// number: start fractional part
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 18;	// decimal part
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state18;	// number: fractional part
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state18:			// number: fractional part
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 18;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state18;	// number: fractional part
			break;

		case L'e':
		case L'E':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 19;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state19;	// number: start exponential part

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
		case L',':	// some other acceptable characters which mark the end of a number
		case L'}':
		case L']':
			info->state = 38;
			goto state38;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state19:			// number: start exponential part
	{
		switch (text[pos])
		{
		case L'+':
		case L'-':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 20;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state20;	// number: exponential part
			break;

		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 21;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state21;	// number: exponential part
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state20:			// number: exponential part following signal
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 21;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state21;	// number: exponential part
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state21:			// number: exponential part
	{
		switch (text[pos])
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (info->temp->text, text[pos]) != RS_OK)
				return JSON_MEMORY;
			info->state = 21;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state21;	// exponential part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
		case L',':
		case L'}':
		case L']':
			info->state = 38;
			goto state38;
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}


      state23:			// value followup
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 23;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state23;	// number followup
			break;

		case L',':
			info->state = 24;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state24;	// sibling
			break;

		case L'}':
			info->state = 3;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state3;	// end array
			break;

		case L']':
			info->state = 4;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state4;	// end array
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state24:			// sibling
	{
		if (info->cursor == NULL)	// a new root must not be a root
			return JSON_BAD_TREE_STRUCTURE;
		if ((info->cursor->type != JSON_OBJECT) && (info->cursor->type != JSON_ARRAY))
		{
			///TODO perform memory cleanup
			return JSON_BAD_TREE_STRUCTURE;
		}

		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 24;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state24;
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state2;	// start object
			break;

		case L'[':
			info->state = 37;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state37;	// start array
			break;

		case L'\"':
			info->state = 5;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state5;	// start string
			break;

		case L'-':
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			info->state = 14;
			goto state14;	// start number
			break;

		case L't':
			info->state = 25;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state25;	// true
			break;

		case L'f':
			info->state = 29;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state29;	// false
			break;

		case L'n':
			info->state = 34;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state34;	// true
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
	}

      state25:			// true: t
	{
		if (text[pos] != L'r')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 26;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state26;	// true: r

	}

      state26:			// true: r
	{
		if (text[pos] != L'u')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 27;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state27;	// true: u

	}

      state27:			// true: u
	{
		if (text[pos] != L'e')
			return JSON_ILLEGAL_CHARACTER;
		info->temp = json_new_true ();
		info->state = 38;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state38;	// true: e

	}

      state29:			// false: f
	{
		if (text[pos] != L'a')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 30;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state30;	// false: a

	}

      state30:			// false: a
	{
		if (text[pos] != L'l')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 31;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state31;	// false: l

	}

      state31:			// false: l
	{
		if (text[pos] != L's')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 32;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state32;	// false: s

	}

      state32:			// false: e
	{
		info->temp = json_new_false ();

		// fix the loose strings
		info->state = 38;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state38;	// fix literal cursor position
	}

      state34:			// null: n
	{
		if (text[pos] != L'u')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 35;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state35;	// null: u

	}

      state35:			// null: u
	{
		if (text[pos] != L'l')
			return JSON_ILLEGAL_CHARACTER;
		info->state = 36;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state36;	// null: l 1 of 2
	}

      state36:			// null: l 1 of 2
	{
		if (text[pos] != L'l')
			return JSON_ILLEGAL_CHARACTER;
		// set up the temp
		info->temp = json_new_false ();
		// move onto the next one
		info->state = 38;
		pos++;
		if (pos >= length)
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state38;	// null: l 2 of 2
	}

      state37:			// start array
	{
		// tree integrity checking
		if (info->cursor)	// cursor will be the parent node of this structure
		{
			if ((info->cursor->type != JSON_ARRAY) && (info->cursor->type != JSON_STRING))
			{
				return JSON_ILLEGAL_CHARACTER;
			}
		}

		// create the new children objects
		info->temp = json_new_array ();

		// create new tree node and add as child
		if (info->cursor != NULL)
		{
			json_insert_child (info->cursor, info->temp);
		}
		// traverse cursor up child node
		info->cursor = info->temp;
		info->temp = NULL;

		info->state = 39;	// start value in array
		goto state39;	// start value in array
	}

      state38:			// fix literal cursor position
	{
		// set the value
		if (info->cursor == NULL)
		{
			info->cursor = info->temp;
			info->temp = NULL;
			return JSON_OK;
		}
		else
		{
			json_insert_child (info->cursor, info->temp);
			info->temp = NULL;
		}
		// fix cursor position
		switch (info->cursor->type)
		{
		case JSON_STRING:
			if (info->cursor->parent == NULL)
			{
				return JSON_OK;
			}

			info->cursor = info->cursor->parent;
			break;

		case JSON_ARRAY:
			break;

		default:
			return JSON_BAD_TREE_STRUCTURE;
			break;
		}

		// move onto next state
		info->state = 23;
		goto state23;	// value followup
	}

      state39:			// start value in array
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	// white spaces
			info->state = 39;
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state39;
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state2;	// start object

		case L'[':
			info->state = 37;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state37;	// start array

		case L']':
			info->state = 4;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state4;	// end array

		case L'\"':
			info->state = 5;
			pos++;
			if (pos >= length)
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state5;	// start string

		case L't':
			info->state = 25;	// true: t
			pos++;
			if (pos >= length)	// current buffer string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state25;	// start value in object
			break;
			break;

		case L'f':
			info->state = 29;	// false: f
			pos++;
			if (pos >= length)	// current buffer string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state29;	// start value in object
			break;
			break;

		case L'n':
			info->state = 34;	// null: n
			pos++;
			if (pos >= length)	// current buffer string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state34;	// null: n
			break;

		case L',':
			if (info->cursor == NULL)
				return JSON_ILLEGAL_CHARACTER;
			if (info->cursor->child_end == NULL)
				return JSON_ILLEGAL_CHARACTER;
			info->state = 24;	// sibling
			pos++;
			if (pos >= length)	// current string ended
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state24;	// sibling
			break;


		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
	}
	return 1;		///fixit return a relevant JSON error code
}


enum json_error
json_saxy_parse (struct json_saxy_parser_status *jsps, struct json_saxy_functions *jsf, wchar_t c)
{
	// make sure everything is in it's place
	assert (jsps != NULL);
	assert (jsf != NULL);
	// goto where we left off
	switch (jsps->state)
	{
	case 0:		// general state. everything goes.
		goto state0;
		break;
	case 1:		// parse string
		goto state1;
		break;
	case 2:		// parse string: escaped character
		goto state2;
		break;
	case 3:		// parse string: escaped unicode 1
		goto state3;
		break;
	case 4:		// parse string: escaped unicode 2
		goto state4;
		break;
	case 5:		// parse string: escaped unicode 3
		goto state5;
		break;
	case 6:		// parse string: escaped unicode 4
		goto state6;
		break;
	case 7:		// parse true: tr
		goto state7;
		break;
	case 8:		// parse true: tru
		goto state8;
		break;
	case 9:		// parse true: true
		goto state9;
		break;
	case 10:		// parse false: fa
		goto state10;
		break;
	case 11:		// parse false: fal
		goto state11;
		break;
	case 12:		// parse false: fals
		goto state12;
		break;
	case 13:		// parse false: false
		goto state13;
		break;
	case 14:		// parse null: nu
		goto state14;
		break;
	case 15:		// parse null: nul
		goto state15;
		break;
	case 16:		// parse null: null
		goto state16;
		break;
	case 17:		// parse number: 0
		goto state17;
		break;
	case 18:		// parse number: start fraccional part
		goto state18;
		break;
	case 19:		// parse number: fraccional part
		goto state19;
		break;
	case 20:		// parse number: start exponent part
		goto state20;
		break;
	case 21:		// parse number: exponent part
		goto state21;
		break;
	case 22:		// parse number: exponent sign part
		goto state22;
		break;
	case 23:		// parse number: start negative
		goto state23;
		break;
	case 24:		// parse number: decimal part
		goto state24;
		break;
	case 25:		// open object
		goto state25;
		break;
	case 26:		// close object/array
		goto state26;
		break;
	case 27:		// sibling followup
		goto state27;
		break;

	default:		// oops... this should never be reached
		return JSON_SOME_PROBLEM;
	}

      state0:			// starting point
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			break;

		case L'\"':
			jsps->state = 1;
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			break;

		case L'{':
			if (jsf->open_object != NULL)
				jsf->open_object ();
			jsps->state = 25;	//open object
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	// close object/array
			break;

		case L'[':
			if (jsf->open_array != NULL)
				jsf->open_array ();
//                      jsps->state = 0;        // redundant
			break;

		case L']':
			if (jsf->close_array != NULL)
				jsf->close_array ();
			jsps->state = 26;	// close object/array
			break;

		case L't':
			jsps->state = 7;	// parse true: tr
			break;

		case L'f':
			jsps->state = 10;	// parse false: fa
			break;

		case L'n':
			jsps->state = 14;	// parse null: nu
			break;

		case L':':
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
//                      jsps->state = 0;        // redundant
			break;

		case L',':
			if (jsf->sibling_separator != NULL)
				jsf->sibling_separator ();
			jsps->state = 27;	// sibling followup
			break;

		case L'0':
			jsps->state = 17;	// parse number: 0
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, L'0') != RS_OK)
				return JSON_MEMORY;
			break;

		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			jsps->state = 24;	// parse number: decimal
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			break;

		case L'-':
			jsps->state = 23;	// number:
			jsps->temp = rs_create (L"");
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, L'-') != RS_OK)
			{
				return JSON_MEMORY;
			}
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state1:			// parse string
	{
		switch (c)
		{
		case L'\\':
			if (rs_catwc (jsps->temp, L'\\') != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			jsps->state = 2;	// parse string: escaped character
			break;

		case L'\"':
			if (jsps->temp != NULL)
			{
				wchar_t *t = rs_unwrap (jsps->temp);
				jsps->state = 0;	// starting point
				if (jsf->new_string != NULL)
					jsf->new_string (t);
			}
			else
				return JSON_SOME_PROBLEM;	///TODO find out what is the best error return code for this situation
			break;

		default:
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			break;
		}
		return JSON_OK;
	}

      state2:			// parse string: escaped character
	{
		switch (c)
		{
		case L'\"':
		case L'\\':
		case L'/':
		case L'b':
		case L'f':
		case L'n':
		case L'r':
		case L't':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			break;

		case L'u':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			jsps->state = 3;	// parse string: escaped unicode 1;
			break;

		default:
			///todo cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state3:			// parse string: escaped unicode 1
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 4;	// parse string. escaped unicode 2
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state4:			// parse string: escaped unicode 2
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 5;	// parse string. escaped unicode 3
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state5:			// parse string: escaped unicode 3
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 6;	// parse string. escaped unicode 4
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state6:			// parse string: escaped unicode 4
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 1;	// parse string
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state7:			// parse true: tr
	{
		if (c != L'r')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 8;	// parse true: tru
		return JSON_OK;
	}

      state8:			// parse true: tru
	{
		if (c != L'u')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 9;	// parse true: true
		return JSON_OK;
	}

      state9:			// parse true: true
	{
		if (c != L'e')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	// general state. everything goes.
		if (jsf->new_true != NULL)
			jsf->new_true ();
		return JSON_OK;
	}

      state10:			// parse false: fa
	{
		if (c != L'a')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 11;	// parse true: fal
		return JSON_OK;
	}

      state11:			// parse false: fal
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 12;	// parse true: fals
		return JSON_OK;
	}

      state12:			// parse false: fals
	{
		if (c != L's')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 13;	// parse true: false
		return JSON_OK;
	}

      state13:			// parse false: false
	{
		if (c != L'e')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	// general state. everything goes.
		if (jsf->new_false != NULL)
			jsf->new_false ();
		return JSON_OK;
	}

      state14:			// parse null: nu
	{
		if (c != L'u')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 15;	// parse null: nul
		return JSON_OK;
	}

      state15:			// parse null: nul
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 16;	// parse null: null
		return JSON_OK;
	}

      state16:			// parse null: null
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	// general state. everything goes.
		if (jsf->new_null != NULL)
			jsf->new_null ();
		return JSON_OK;
	}

      state17:			// parse number: 0
	{
		switch (c)
		{
		case L'.':
			if (rs_catwc (jsps->temp, L'.') != RS_OK)
			{
				//TODO cleanup?
				return JSON_MEMORY;
			}
			jsps->state = 18;	// parse number: fraccional part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	// close object
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	// close object/array
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	// sibling followup
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}

		return JSON_OK;
	}

      state18:			// parse number: start fraccional part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 19;	// parse number: fractional part
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state19:			// parse number: fraccional part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
//                              jsps->state = 19;       // parse number: fractional part
			break;

		case L'e':
		case L'E':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 20;	// parse number: start exponent part
			break;


		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	// close object/array
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	// close object/array
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	// sibling followup
			break;


		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state20:			// parse number: start exponent part
	{
		switch (c)
		{
		case L'+':
		case L'-':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 22;	// parse number: exponent sign part
			break;

		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 21;	// parse number: exponent part
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state21:			// parse number: exponent part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
//                              jsps->state = 21;       // parse number: exponent part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	// close object
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	// close object/array
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	// sibling followup
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state22:			// parse number: start exponent part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 21;	// parse number: exponent part
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state23:			// parse number: start negative
	{
		switch (c)
		{
		case L'0':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 17;	// parse number: 0
			break;

		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 24;	// parse number: start decimal part
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state24:			// parse number: decimal part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
//                              jsps->state = 24;       // parse number: decimal part
			break;

		case L'.':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 18;	// parse number: start exponent part
			break;

		case L'e':
		case L'E':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 20;	// parse number: start exponent part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	// close object/array
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	// close object/array
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	// sibling followup
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state25:			// open object
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			break;

		case L'\"':
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			jsps->state = 1;
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	// close object
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state26:			// close object/array
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
//                      jsp->state = 26;        // close object
			break;

		case L']':
			if (jsf->close_array != NULL)
				jsf->close_array ();
//                      jsps->state = 26;       // close object/array
			break;

		case L',':
			if (jsf->sibling_separator != NULL)
				jsf->sibling_separator ();
			jsps->state = 27;	// sibling followup
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state27:			// sibling followup
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			break;

		case L'\"':
			jsps->state = 1;
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			break;

		case L'{':
			if (jsf->open_object != NULL)
				jsf->open_object ();
			jsps->state = 25;	//open object
			break;

		case L'[':
			if (jsf->open_array != NULL)
				jsf->open_array ();
//                      jsps->state = 0;        // redundant
			break;

		case L't':
			jsps->state = 7;	// parse true: tr
			break;

		case L'f':
			jsps->state = 10;	// parse false: fa
			break;

		case L'n':
			jsps->state = 14;	// parse null: nu
			break;

		case L'0':
			jsps->state = 17;	// parse number: 0
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, L'0') != RS_OK)
				return JSON_MEMORY;
			break;

		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			jsps->state = 24;	// parse number: decimal
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			break;

		case L'-':
			jsps->state = 23;	// number:
			jsps->temp = rs_create (L"");
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, L'-') != RS_OK)
			{
				return JSON_MEMORY;
			}
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}



	return JSON_SOME_PROBLEM;
}
