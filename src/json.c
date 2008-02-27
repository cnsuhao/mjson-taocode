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


json_t *
json_new_value (const enum json_value_type type)
{
	json_t *new_object;
	/* allocate memory to the new object */
	new_object = malloc (sizeof (json_t));
	if (new_object == NULL)
		return NULL;

	/* initialize members */
	new_object->text = NULL;
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next = NULL;
	new_object->type = type;
	return new_object;
}


json_t *
json_new_string (const wchar_t * text)
{
	json_t *new_object;
	char *temp;
	assert (text != NULL);

	/* allocate memory for the new object */
	new_object = malloc (sizeof (json_t));
	if (new_object == NULL)
		return NULL;

	/* initialize members */
	temp = wchar_to_utf8 (text, wcslen (text));
	if (temp == NULL)
	{
		free (new_object);
		return NULL;
	}
	new_object->text = rcs_wrap (temp);
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next = NULL;
	new_object->type = JSON_STRING;
	return new_object;
}


json_t *
json_new_number (const wchar_t * text)
{
	json_t *new_object;
	char *temp;
	assert (text != NULL);

	/* allocate memory for the new object */
	new_object = malloc (sizeof (json_t));
	if (new_object == NULL)
		return NULL;

	/* initialize members */
	temp = wchar_to_utf8 (text, wcslen (text));
	if (temp == NULL)
	{
		free (new_object);
		return NULL;
	}

	new_object->text = rcs_wrap (temp);
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next = NULL;
	new_object->type = JSON_NUMBER;
	return new_object;
}


json_t *
json_new_object (void)
{
	return json_new_value (JSON_OBJECT);
}


json_t *
json_new_array (void)
{
	return json_new_value (JSON_ARRAY);
}


json_t *
json_new_null (void)
{
	return json_new_value (JSON_NULL);
}


json_t *
json_new_true (void)
{
	return json_new_value (JSON_TRUE);
}


json_t *
json_new_false (void)
{
	return json_new_value (JSON_FALSE);
}


void
json_free_value (json_t ** value)
{
	assert (value != NULL);
	assert ((*value) != NULL);

	/* free each and every child node */
	if ((*value)->child != NULL)
	{
		json_t *i, *j;
		i = (*value)->child_end;
		while (i != NULL)
		{
			j = i->previous;
			json_free_value (&i);
			i = j;
		}
	}

	/* fixing sibling linked list connections */
	if ((*value)->previous && (*value)->next)
	{
		(*value)->previous->next = (*value)->next;
		(*value)->next->previous = (*value)->previous;
	}
	else
	{
		if ((*value)->previous)
		{
			(*value)->previous->next = NULL;
		}
		if ((*value)->next)
		{
			(*value)->next->previous = NULL;
		}
	}

	/*fixing parent node connections */
	if ((*value)->parent)
	{
		if ((*value)->parent->child == (*value))
		{
			if ((*value)->next)
			{
				(*value)->parent->child = (*value)->next;	/* the parent node always points to the first node */
			}
			else
			{
				if ((*value)->previous)
					(*value)->parent->child = (*value)->next;	/* the parent node always points to the first node */
				(*value)->parent->child = NULL;
			}
		}
	}

	/*finally, freeing the memory allocated for this value */
	if ((*value)->text != NULL)
	{
		rcs_free (&(*value)->text);
	}
	free (*value);		/* the json value */
	(*value) = NULL;
}


enum json_error
json_insert_child (json_t * parent, json_t * child)
{
	assert (parent != NULL);	/* the parent must exist */
	assert (child != NULL);	/* the child must exist */
	assert (parent != child);	/* parent and child must not be the same. if they are, it will enter an infinite loop */

	/* enforce tree structure correctness */
	switch (parent->type)
	{
	case JSON_STRING:
		/* a string accepts every JSON type as a child value */
		/* therefore, the sanity check must be performed on the child node */
		switch (child->type)
		{
		case JSON_STRING:
		case JSON_NUMBER:
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
			if (child->child != NULL)
				return JSON_BAD_TREE_STRUCTURE;
			break;

		case JSON_OBJECT:
		case JSON_ARRAY:
			break;

		default:
			return JSON_BAD_TREE_STRUCTURE;	/* this part should never be reached */
			break;
		}
		break;

	case JSON_OBJECT:	/* JSON objects may only accept JSON string objects which already have child nodes of their own */
		if (child->type != JSON_STRING)
			return JSON_BAD_TREE_STRUCTURE;
		break;

	case JSON_ARRAY:
		switch (child->type)
		{
		case JSON_STRING:
		case JSON_TRUE:
		case JSON_FALSE:
		case JSON_NULL:
		case JSON_NUMBER:
			if (child->child)
				return JSON_BAD_TREE_STRUCTURE;
			break;

		case JSON_OBJECT:
		case JSON_ARRAY:
			break;
		default:
			return JSON_BAD_TREE_STRUCTURE;
		}
		break;

	default:
		return JSON_BAD_TREE_STRUCTURE;
	}

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
json_insert_pair_into_object (json_t * parent, wchar_t * text_label, json_t * value)
{
	enum json_error error;
	json_t *label;
	/* verify if the parameters are valid */
	assert (parent != NULL);
	assert (text_label != NULL);
	assert (value != NULL);
	assert (parent != value);

	/* enforce type coherence */
	assert (parent->type == JSON_OBJECT);


	/* create label json_value */
	label = json_new_string (text_label);
	if (label == NULL)
		return JSON_MEMORY;

	/*insert value and check for error */
	error = json_insert_child (label, value);
	if (error != JSON_OK)
		return error;
	/*insert value and check for error */
	error = json_insert_child (parent, label);
	if (error != JSON_OK)
		return error;

	return JSON_OK;
}


enum json_error
json_tree_to_string (json_t * root, wchar_t ** text)
{
	json_t *cursor;
	rwstring *output;
	assert (root != NULL);
	assert (text != NULL);
	assert (*text != NULL);

	cursor = root;
	/* set up the output and temporary rwstrings */
	output = rws_create (1);

	/* start the convoluted fun */
      state1:			/* open value */
	{
		if ((cursor->previous) && (cursor != root))	/*if cursor is children and not root than it is a followup sibling */
		{
			/* append comma */
			if (rws_catwc (output, L',') != RS_OK)
			{
				return JSON_MEMORY;
			}
		}
		switch (cursor->type)
		{
		case JSON_STRING:
			/* append the "text"\0, which means 1 + wcslen(cursor->text) + 1 + 1 */
			/* set the new output size */
			if (rws_catwc (output, L'\"') != RS_OK)
			{
				return JSON_MEMORY;
			}
			if (rws_catrcs (output, cursor->text) != RS_OK)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (output, L'\"') != RS_OK)
			{
				return JSON_MEMORY;
			}

			if (cursor->parent != NULL)
			{
				if (cursor->parent->type == JSON_OBJECT)	/* cursor is label in label:value pair */
				{
					/* error checking: if parent is object and cursor is string then cursor must have a single child */
					if (cursor->child != NULL)
					{
						if (rws_catwc (output, L':') != RS_OK)
						{
							return JSON_MEMORY;
						}
					}
					else
					{
						/* malformed document tree: label without value in label:value pair */
						rws_free (&output);
						text = NULL;
						return JSON_BAD_TREE_STRUCTURE;
					}
				}
			}
			else	/* does not have a parent */
			{
				if (cursor->child != NULL)	/* is root label in label:value pair */
				{
					if (rws_catwc (output, L':') != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					/* malformed document tree: label without value in label:value pair */
					rws_free (&output);
					text = NULL;
					return JSON_BAD_TREE_STRUCTURE;
				}
			}
			break;

		case JSON_NUMBER:
			/* must not have any children */
			/* set the new size */
			if (rws_catrcs (output, cursor->text) != RS_OK)
			{
				return JSON_MEMORY;
			}
			goto state2;	/* close value */
			break;

		case JSON_OBJECT:
			if (rws_catwc (output, L'{') != RS_OK)
			{
				return JSON_MEMORY;
			}

			if (cursor->child)
			{
				cursor = cursor->child;
				goto state1;	/* open value */
			}
			else
			{
				goto state2;	/* close value */
			}
			break;

		case JSON_ARRAY:
			if (rws_catwc (output, L'[') != RS_OK)
			{
				return JSON_MEMORY;
			}

			if (cursor->child != NULL)
			{
				cursor = cursor->child;
				goto state1;
			}
			else
			{
				goto state2;	/* close value */
			}
			break;

		case JSON_TRUE:
			/* must not have any children */
			if (rws_catwcs (output, L"true", 4) != RS_OK)
			{
				return JSON_MEMORY;
			}
			goto state2;	/* close value */
			break;

		case JSON_FALSE:
			/* must not have any children */
			if (rws_catwcs (output, L"false", 5) != RS_OK)
			{
				return JSON_MEMORY;
			}
			goto state2;	/* close value */
			break;

		case JSON_NULL:
			/* must not have any children */
			if (rws_catwcs (output, L"null", 4) != RS_OK)
			{
				return JSON_MEMORY;
			}
			goto state2;	/* close value */
			break;

		default:
			goto error;
		}
		if (cursor->child)
		{
			cursor = cursor->child;
			goto state1;	/* open value */
		}
		else
		{
			/* does not have any children */
			goto state2;	/* close value */
		}
	}

      state2:			/* close value */
	{
		switch (cursor->type)
		{
		case JSON_OBJECT:
			if (rws_catwc (output, L'}') != RS_OK)
			{
				return JSON_MEMORY;
			}
			break;

		case JSON_ARRAY:
			if (rws_catwc (output, L']') != RS_OK)
			{
				return JSON_MEMORY;
			}
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
			goto state1;	/* open value */
		}
		else
		{
			cursor = cursor->parent;
			goto state2;	/* close value */
		}
	}

      error:
	{
		rws_free (&output);
		return JSON_UNKNOWN_PROBLEM;
	}

      end:
	{
		*text = output->text;
		return JSON_OK;
	}
}


int
json_white_space (const wchar_t c)
{
	switch (c)
	{
	case L'\x20':		/* space */
	case L'\x09':		/* horizontal tab */
	case L'\x0A':		/* line feed or new line */
	case L'\x0D':		/* Carriage return */
		return 1;
	default:
		return 0;
	}
}


void 
json_strip_white_spaces (wchar_t * text)
{
	size_t in, out, length;
	int state;

	assert (text != NULL);
	
	in = 0;
	out = 0;
	length = wcslen(text);
	state = 0;	/* possible states: 0 -> document, 1 -> inside a string */
	
	while(in < length)
	{
		switch(text[in])
		{
			case L'\x20':		/* space */
			case L'\x09':		/* horizontal tab */
			case L'\x0A':		/* line feed or new line */
			case L'\x0D':		/* Carriage return */
				if(state == 1)
				{
					text[out++] = text[in];
				}
				break;

			case L'\"':
				switch(state)
				{
					case 0:	/* not inside a JSON string */
						state = 1;
						break;

					case 1: /* inside a JSON string */
						if(text[in-1] != L'\\')
						{
							state = 0;
						}
						break;

					default:
						assert(0);
				}
				text[out++] = text[in];
				break;

			default:
				text[out++] = text[in];
		}
		++in;
	}
	text[out] = L'\0';
}


	wchar_t *
json_format_string (wchar_t * text)
{
	size_t pos = 0;
	unsigned int indentation = 0;	/* the current indentation level */
	unsigned int i;		/* loop iterator variable */
	char loop;

	wchar_t *output = NULL;
	wchar_t *temp = NULL;
	size_t length = 0;	/* temp variable to calculate the new realloc size */

	while (pos < wcslen (text))
	{
		switch (text[pos])
		{
			case L'\x20':
			case L'\x09':
			case L'\x0A':
			case L'\x0D':	/* JSON insignificant white spaces */
				pos++;
				break;

			case L'{':
				indentation++;
				length = 3 + indentation;
				if (output)
					length += wcslen (output);
				if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
				{
					return NULL;
				}
				output = temp;
				wcsncat (output, L"{\n", 2);
				for (i = 0; i < indentation; i++)	/*/todo find a better way */
				{
					wcsncat (output, L"\t", 1);
			}

			pos++;
			break;

		case L'}':
			indentation--;
			length = 3 + indentation;	/* \n + indent*\t + } + \0 */
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\n", 1);
			for (i = 0; i < indentation; i++)
			{
				wcsncat (output, L"\t", 1);
			}
			wcsncat (output, L"}", 1);

			pos++;
			break;

		case L':':
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L": ", 2);

			pos++;
			break;

		case L',':
			length = 3 + indentation;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L",\n", 2);
			for (i = 0; i < indentation; i++)
			{
				wcsncat (output, L"\t", 1);
			}

			pos++;
			break;

		case L'\"':	/*open string   ///todo rethink this one */
			length = 2;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, &text[pos], 1);

			pos++;
			loop = 1;	/* inner string loop trigger is enabled */
			while (loop)	/* parse the inner part of the string   ///todo rethink this loop */
			{
				if (text[pos] == L'\\')	/* escaped sequence */
				{
					length = 2;
					if (output)
						length += wcslen (output);
					if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
					{
						return NULL;
					}
					output = temp;
					wcsncat (output, L"\\", 1);

					pos++;
					if (text[pos] == L'\"')	/* don't consider a \" escaped sequence as an end of string */
					{
						if ((temp = realloc (output, sizeof (wchar_t) * (wcslen (output) + 2))) == NULL)
						{
							return NULL;
						}
						output = temp;
						wcsncat (output, L"\"", 1);

						pos++;
					}
				}
				else if (text[pos] == L'\"')	/* reached end of string */
				{
					loop = 0;
				}


				if ((temp = realloc (output, sizeof (wchar_t) * (wcslen (output) + 2))) == NULL)
				{
					return NULL;
				}
				output = temp;
				wcsncat (output, &text[pos], 1);

				pos++;
				if (pos >= wcslen (text))
				{
					loop = 0;
				}
			}
			break;

		default:
			length = 2;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, &text[pos], 1);

			pos++;
			break;
		}
	}

	return output;
}


wchar_t *
json_escape (wchar_t * text)
{
	wchar_t *output;
	wchar_t *temp;
	size_t length;
	size_t i;
	/* check if pre-conditions are met */
	assert (text != NULL);

	/* defining the temporary variables */
	output = NULL;
	temp = NULL;
	length = 0;
	for (i = 0; i < wcslen (text); i++)
	{
		if (text[i] == L'\\')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\\\", 2);
		}
		else if (text[i] == L'\"')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\\"", 2);
		}
		else if (text[i] == L'/')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\/", 2);
		}
		else if (text[i] == L'\b')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\b", 2);
		}
		else if (text[i] == L'\f')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\f", 2);
		}
		else if (text[i] == L'\n')
		{
			length = 3;
			if (output)
				length = wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\n", 2);
		}
		else if (text[i] == L'\r')
		{
			length = 3;
			if (output)
				length = wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\r", 2);
		}
		else if (text[i] == L'\t')
		{
			length = 3;
			if (output)
				length = wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\t", 2);
		}
		else if (text[i] <= 0x1F)	/* escape the characters as declared in 2.5 of http://www.ietf.org/rfc/rfc4627.txt */
		{
			wchar_t tmp[6];
			swprintf (tmp, 6, L"\\u%4x", text[i]);
			length = 7;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, tmp, 6);
		}
		else
		{
			length = 2;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, &text[i], 1);
		}
	}
	return output;
}


wchar_t *
json_escape_to_ascii (wchar_t * text)
{
	wchar_t *output;
	wchar_t *temp;
	size_t length;
	size_t i;

	/*check if pre-conditions are met */
	assert (text != NULL);
	/* temporary variables */
	output = NULL;
	temp = NULL;
	length = 0;
	for (i = 0; i < wcslen (text); i++)
	{
		if (text[i] == L'\\')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\\\", 2);
		}
		else if (text[i] == L'\"')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\\"", 2);
		}
		else if (text[i] == L'/')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\/", 2);
		}
		else if (text[i] == L'\b')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\b", 2);
		}
		else if (text[i] == L'\f')
		{
			length = 3;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\f", 2);
		}
		else if (text[i] == L'\n')
		{
			length = 3;
			if (output)
				length = wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\n", 2);
		}
		else if (text[i] == L'\r')
		{
			length = 3;
			if (output)
				length = wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\r", 2);
		}
		else if (text[i] == L'\t')
		{
			length = 3;
			if (output)
				length = wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, L"\\t", 2);
		}
		else if (text[i] <= 0x1F)	/* escape the characters as declared in 2.5 of http://www.ietf.org/rfc/rfc4627.txt */
		{
			wchar_t tmp[6];
			swprintf (tmp, 6, L"\\u%4x", text[i]);
			length = 7;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, tmp, 6);
		}
		else if ((text[i] >= 0x20) && (text[i] <= 0x7E))	/* ascii printable characters */
		{
			length = 2;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, &text[i], 1);
		}
		else		/* beyond ascii characters? */
		{
			wchar_t tmp[6];
			swprintf (tmp, 6, L"\\u%4x", text[i]);
			length = 7;
			if (output)
				length += wcslen (output);
			if ((temp = realloc (output, sizeof (wchar_t) * length)) == NULL)
			{
				return NULL;
			}
			output = temp;
			wcsncat (output, temp, 6);
		}
	}
	return output;
}


enum json_error
json_parse_string (struct json_parsing_info *info, wchar_t * text, size_t length)
{
	size_t pos;
	/*/todo sanitize the state numbers. */
	/*
	   redundant states which were eliminated:
	   state33
	 */
	/* ckeck if pre-conditions are met */
	assert (info != NULL);
	assert (text != NULL);

	/* setup the initial values */
	pos = 0;

	/* go to the state that we should be to resume parsing */
	switch (info->state)	/* list of valid states in json_parse_string() */
	{
	case 0:
		goto state0;	/* general purpose state: starting point */
	case 1:
		goto state1;	/* start label string in object */
	case 2:
		goto state2;	/* start object */
	case 3:
		goto state3;	/* end object */
	case 4:
		goto state4;	/* end array */
	case 5:
		goto state5;	/* start string */
	case 6:
		goto state6;	/* continue string */
	case 7:
		goto state7;	/* continue string: escaped character */
	case 8:
		goto state8;	/* continue string: escaped unicode character 1 of 4 */
	case 9:
		goto state9;	/* continue string: escaped unicode character 2 of 4 */
	case 10:
		goto state10;	/* continue string: escaped unicode character 3 of 4 */
	case 11:
		goto state11;	/* continue string: escaped unicode character 4 of 4 */
	case 12:
		goto state12;	/* value in array */
	case 13:
		goto state13;	/* pair */
	case 14:
		goto state14;	/* start number */
	case 15:
		goto state15;	/* number: leading zero */
	case 16:
		goto state16;	/* number: decimal part */
	case 17:
		goto state17;	/* number: start fractional part */
	case 18:
		goto state18;	/* number: fractional part */
	case 19:
		goto state19;	/* number: start exponential part */
	case 20:
		goto state20;	/* number: exponential part following signal */
	case 21:
		goto state21;	/* number: exponential part */
	case 22:
		goto state22;	/* parse whitespaces until the end */
	case 23:
		goto state23;	/* value followup */
	case 24:
		goto state24;	/* sibling */
	case 25:
		goto state25;	/* true: t */
	case 26:
		goto state26;	/* true: r */
	case 27:
		goto state27;	/* true: u */
	case 28:
		goto state28;	/* label followup */
	case 29:
		goto state29;	/* false: f */
	case 30:
		goto state30;	/* false: a */
	case 31:
		goto state31;	/* false: l */
	case 32:
		goto state32;	/* false: s */
/*      case 33:*/
/*              goto state33:*/
	case 34:
		goto state34;	/* null: n */
	case 35:
		goto state35;	/* null: u */
	case 36:
		goto state36;	/* null: l 1 of 2 */
	case 37:
		goto state37;	/* start array */
	case 38:
		goto state38;	/* fix literal cursor position */
	case 39:
		goto state39;	/* handle the JSON_ILLEGAL_CHAR error */

	default:
		return JSON_UNKNOWN_PROBLEM;	/* IF THIS PART IS REACHED THEN THERE IS A BUG SOMEWHERE */
		break;
	}


	/* let's start the juicy bits */

      state0:			/* general purpose state: starting point */
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			pos++;
			if (pos > length)	/* current string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state0;	/* general purpose state: starting point */
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos > length)	/* current string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state2;	/* start object */
			break;

		case L'[':
			info->state = 37;
			pos++;
			if (pos > length)	/* current string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state37;	/* start object */
			break;

		case L'\"':
			info->state = 5;
			pos++;
			if (pos > length)	/* current string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state5;	/* start string */
			break;

		default:
			info->state = 39;	/* return JSON_ILLEGAL_CHARACTER */
			goto state39;
			break;
		}

	}

      state1:			/* start label string in object */
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
/*                      info->state = 1;*/
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state1;	/* start label string in object */
			break;

		case L'}':
			info->state = 3;
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state3;	/* end object */
			break;

		case L'\"':
			info->state = 5;
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state5;	/* start string */
			break;

		case L',':
			if (info->cursor == NULL)
			{
				info->state = 39;
				goto state39;	/* handle JSON_ILLEGAL_CHAR */
			}

			if (info->cursor->child_end == NULL)
			{
				info->state = 39;
				goto state39;	/* handle JSON_ILLEGAL_CHAR */
			}

			info->state = 24;	/* sibling */
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state24;	/* sibling */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
			break;
		}
	}

      state2:			/* start object */
	{
		/* tree integrity checking */
		if (info->cursor)	/* cursor will be the parent node of this structure */
		{
			if ((info->cursor->type != JSON_ARRAY) && (info->cursor->type != JSON_STRING))
			{
				info->state = 39;
				goto state39;	/* handle JSON_ILLEGAL_CHAR */
			}
		}

		/* create the new children objects */
		info->temp = json_new_object ();

		/* create new tree node and add as child */
		if (info->cursor != NULL)
		{
			json_insert_child (info->cursor, info->temp);
		}
		/* traverse cursor up child node */
		info->cursor = info->temp;
		info->temp = NULL;

		info->state = 1;
		goto state1;	/* start label string in object */
	}

      state3:			/* end object */
	{
		/* verify tree integrity */
		if (info->cursor == NULL)
		{
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}

		if (info->cursor->type != JSON_OBJECT)
		{
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}

		if (info->cursor->parent == NULL)
		{
			info->state = 22;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_OK;
			else
				goto state22;	/* parse whitespaces until the end */

		}

		/* point cursor to the parent node */
		info->cursor = info->cursor->parent;

		/* check if we descended into the root node */
		switch (info->cursor->type)	/*/ todo rethink these switch statements */
		{
		case JSON_STRING:
			if (info->cursor->parent == NULL)
			{
				info->state = 22;
				pos++;
				if (pos > length)	/* current string buffer ended */
					return JSON_OK;
				else
					goto state22;	/* parse whitespaces until the end */
			}
			else
				info->cursor = info->cursor->parent;
			break;

		case JSON_ARRAY:
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
			break;
		}

		/* proceed to the next state */
		switch (info->cursor->type)
		{
		case JSON_OBJECT:
			info->state = 1;
			goto state1;	/* start label string in object */
			break;

		case JSON_ARRAY:
			info->state = 12;
			goto state12;	/* start value in array */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}
	}

      state4:			/* end array */
	{
		/* verify tree integrity */
		if (info->cursor == NULL)
		{
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}

		if (info->cursor->type != JSON_ARRAY)
		{
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}

		if (info->cursor->parent == NULL)
		{
			info->state = 22;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_OK;
			else
				goto state22;	/* parse whitespaces until the end */
		}

		/* move on down */
		info->cursor = info->cursor->parent;

		/* check if we descended into the root node */
		switch (info->cursor->type)
		{
		case JSON_STRING:
			if (info->cursor->parent == NULL)
			{

				info->state = 22;
				pos++;
				if (pos > length)	/* current string buffer ended */
					return JSON_OK;
				else
					goto state22;	/* parse whitespaces until the end */
			}
			else
				info->cursor = info->cursor->parent;	/* point the cursor to a parent node which supports children */
			break;

		case JSON_ARRAY:
			/*/todo finish this step */
			break;

		default:	/* The parent node of a JSON_ARRAY can only be a JSON_ARRAY or JSON_STRING */

			if (info->temp != NULL)
				free (info->temp), info->temp = NULL;
			if (info->cursor != NULL)
			{
				while (info->cursor->parent != NULL)
					info->cursor = info->cursor->parent;
				json_free_value (&info->cursor);
			}
			return JSON_BAD_TREE_STRUCTURE;	/* the tree shouldn't be like this */
			break;
		}

		/* proceed to the next state */
		/*
		   In the previous switch() statement the cursor was pointed to the true parent node which supports children. That parent node can only be of type JSON_OBJECT or JSON_ARRAY
		 */
		switch (info->cursor->type)
		{
		case JSON_OBJECT:
			info->state = 1;
			goto state1;	/* start label string in object */
			break;

		case JSON_ARRAY:
			info->state = 12;
			goto state12;	/* start value in array */
			break;

		default:
			if (info->temp != NULL)
				free (info->temp), info->temp = NULL;
			if (info->cursor != NULL)
			{
				while (info->cursor->parent != NULL)
					info->cursor = info->cursor->parent;
				json_free_value (&info->cursor);
			}
			return JSON_BAD_TREE_STRUCTURE;	/* other types besides JSON_ARRAY and JSON_OBJECT mean that the tree is malformed */
		}
	}

      state5:			/* start string */
	{
		/* verify tree integrity */
		if (info->cursor != NULL)
		{
			switch (info->cursor->type)	/* JSON_STRING must be a child of JSON_OBJECT, JSON_ARRAY or JSON_STRING */
			{
			case JSON_OBJECT:
			case JSON_ARRAY:
				break;
			case JSON_STRING:
				if (info->cursor->parent)
				{
					if (info->cursor->parent->type != JSON_OBJECT)	/*a parent of a parent string must be an object */
					{
						info->state = 39;
						goto state39;	/* handle JSON_ILLEGAL_CHAR */
					}
				}
				break;

			default:	/* ...otherwise, the tree is malformed */
				info->state = 39;
				goto state39;	/* handle JSON_ILLEGAL_CHAR */
			}
		}

		/* prepare the temporary node to get the string */
		if (info->temp == NULL)
		{
			info->temp = json_new_string (L"");
			if (info->temp == NULL)
			{
				if (info->cursor != NULL)
				{
					while (info->cursor->parent != NULL)
						info->cursor = info->cursor->parent;
					json_free_value (&info->cursor);
				}
				return JSON_MEMORY;
			}
			info->string_length_limit_reached = 0;	/* reset the max string length checker */
		}

		/* move to the next state */
		info->state = 6;
		goto state6;	/* continue string */
	}

      state6:			/* continue string */
	{
		/* check if there is a valid temporary function */
		if (info->temp == NULL)	/* something weird just happened. The temp variable is expected to be non-null */
		{
			/* cleanup */
			if (info->cursor)
			{
				/* clean the current tree structure */
				while (info->cursor->parent)	/* point info->cursor to the root node */
				{
					info->cursor = info->cursor->parent;
				}
				/* clean */
				json_free_value (&info->cursor);
			}
			return JSON_UNKNOWN_PROBLEM;
		}

		/* proceed with the string parsing */
		switch (text[pos])
		{
		case L'\\':	/* escaped characters */
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH - 6)	/* 6 = \u[:xdigit:]{4} */
				{
					if (rcs_catc (info->temp->text, '\\') != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;	/* string length limit was reached. drop every subsequent character. */
				}
			}

			info->state = 7;	/* continue string: escaped character */
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state7;	/* continue string: escaped character */
			break;

		case L'"':	/* closing string */
			if (info->cursor == NULL)	/* this string will be the root node */
			{
				info->cursor = info->temp;
				info->temp = NULL;

				info->state = 28;
				pos++;
				if (pos > length)	/* current string buffer ended */
					return JSON_INCOMPLETE_DOCUMENT;
				else
					goto state28;	/* label string followup */
			}

			/* append the child string to the parent node */
			json_insert_child (info->cursor, info->temp);

			/* make sure the cursor points to a valid parent node */
			switch (info->cursor->type)
			{
			case JSON_ARRAY:
				info->temp = NULL;

				info->state = 12;
				pos++;
				if (pos > length)	/* current string buffer ended */
					return JSON_INCOMPLETE_DOCUMENT;
				else
					goto state12;	/* value in array */
				break;

			case JSON_STRING:	/* parent is label in label:value pair */
				info->temp = NULL;
				if (info->cursor->parent != NULL)
				{
					/* todo perform tree sanity check */
					if (info->cursor->parent->type != JSON_OBJECT)
					{
						if (info->cursor != NULL)
						{
							while (info->cursor->parent != NULL)
								info->cursor = info->cursor->parent;
							json_free_value (&info->cursor);
						}
						return JSON_BAD_TREE_STRUCTURE;
					}

					/* set cursor to continue parsing */
					info->cursor = info->cursor->parent;

					info->state = 1;
					pos++;
					if (pos > length)	/* current string buffer ended */
						return JSON_OK;
					else
						goto state1;	/* start label string in object */
				}
				else
				{
					/* root node is JSON_STRING, new child node is JSON_STRING. There is no more document to parse. */
					info->state = 22;
					pos++;
					if (pos > length)	/* current string buffer ended */
						return JSON_OK;
					else
						goto state22;	/* parse whitespaces until the end */
				}
				break;

			case JSON_OBJECT:
				info->cursor = info->temp;	/* point cursor at inserted child label */
				info->temp = NULL;

				info->state = 28;
				pos++;
				if (pos > length)	/* current string buffer ended */
					return JSON_INCOMPLETE_DOCUMENT;
				else
					goto state28;	/* label string followup */
				break;

			default:
				if (info->temp != NULL)
					free (info->temp), info->temp = NULL;
				if (info->cursor != NULL)
				{
					while (info->cursor->parent != NULL)
						info->cursor = info->cursor->parent;
					json_free_value (&info->cursor);
				}
				return JSON_BAD_TREE_STRUCTURE;
			}
			break;

		case L'\x0000':	/* range of characters that must be escaped according to 2.5 of http://www.ietf.org/rfc/rfc4627.txt */
		case L'\x0001':
		case L'\x0002':
		case L'\x0003':
		case L'\x0004':
		case L'\x0005':
		case L'\x0006':
		case L'\x0007':
		case L'\x0008':
		case L'\x0009':
		case L'\x000a':
		case L'\x000b':
		case L'\x000c':
		case L'\x000d':
		case L'\x000e':
		case L'\x000f':
		case L'\x0010':
		case L'\x0011':
		case L'\x0012':
		case L'\x0013':
		case L'\x0014':
		case L'\x0015':
		case L'\x0016':
		case L'\x0017':
		case L'\x0018':
		case L'\x0019':
		case L'\x001a':
		case L'\x001b':
		case L'\x001c':
		case L'\x001d':
		case L'\x001e':
		case L'\x001f':
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
			break;

		default:
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}
			info->state = 6;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state6;
		}

	}

      state7:			/* continue string: escaped characters */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH - 5)
				{
					if (rcs_catc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}
			info->state = 6;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state6;	/* continue string */
			break;

		case L'u':
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH - 4)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}
			info->state = 8;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state8;	/* continue string: escaped unicode character 1 */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}
	}

      state8:			/* continue string: escaped unicode character 1 */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH - 3)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}

			info->state = 9;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state9;	/* continue string: escaped unicode character 2 */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}
		/* this part shouldn't be reached */
		return JSON_UNKNOWN_PROBLEM;
	}

      state9:			/* continue string: escaped unicode character 2 */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH - 2)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}

			info->state = 10;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state10;	/* continue string: escaped unicode character 3 */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}
		/* this section shouldn't be reached */
		return JSON_UNKNOWN_PROBLEM;
	}

      state10:			/* continue string: escaped unicode character 3 */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH - 1)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}

			info->state = 11;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state11;	/* continue string: escaped unicode character 3 */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR error */
		}

		/* this section shouldn't be reached */
		return JSON_UNKNOWN_PROBLEM;
	}

      state11:			/* continue string: escaped unicode character 4 */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					info->string_length_limit_reached = 1;
				}
			}

			info->state = 6;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state6;	/* continue string */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
		}
		/* this section shouldn't be reached */
		return JSON_UNKNOWN_PROBLEM;
	}

      state12:			/* value in array */
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JOSN insignificant white spaces */
			info->state = 12;
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state12;
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state2;	/* start object */

		case L'[':
			info->state = 37;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state37;	/* start array */

		case L']':
			info->state = 4;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state4;	/* end array */

		case L'\"':
			info->state = 5;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state5;	/* start string */

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
			goto state14;	/* number */
			break;

		case L't':
			info->state = 25;	/* true: t */
			pos++;
			if (pos > length)	/* current buffer string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state25;	/* true: t */
			break;
			break;

		case L'f':
			info->state = 29;	/* false: f */
			pos++;
			if (pos > length)	/* current buffer string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state29;	/* start value in object */
			break;
			break;

		case L'n':
			info->state = 34;	/* null: n */
			pos++;
			if (pos > length)	/* current buffer string ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state34;	/* null: n */
			break;

		case L',':
			if (info->cursor == NULL)
			{
				info->state = 39;
				goto state39;	/* handle JSON_ILLEGAL_CHAR */
			}

			if (info->cursor->child_end == NULL)
			{
				info->state = 39;
				goto state39;	/* handle JSON_ILLEGAL_CHAR */
			}

			info->state = 24;	/* sibling */
			pos++;
			if (pos > length)	/* current string buffer ended */
			{
				return JSON_INCOMPLETE_DOCUMENT;
			}
			else
				goto state24;	/* sibling */
			break;


		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR */
			break;
		}
	}

      state13:			/* pair */
	{
		/* perform pre-condition sanity checks */
		if (info->cursor == NULL)
		{
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR error */
		}
		if (info->cursor->type != JSON_STRING)
		{
			if (info->temp != NULL)
				json_free_value (&info->temp);
			if (info->cursor != NULL)
			{
				while (info->cursor->parent != NULL)
					info->cursor = info->cursor->parent;
				json_free_value (&info->cursor);
			}
			return JSON_BAD_TREE_STRUCTURE;
		}

		/* go on */
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			info->state = 13;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state13;	/* pair */
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state2;	/* start object */
			break;

		case L'[':
			info->state = 37;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state37;
			break;


		case L'\"':
			info->state = 5;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state5;	/* start string */

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
			goto state14;	/* number */
			break;

		case L't':
			info->state = 25;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state25;	/* true */
			break;

		case L'f':
			info->state = 29;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state29;	/* false */
			break;

		case L'n':
			info->state = 34;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state34;	/* null */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state14:			/* start number */
	{
		info->temp = json_new_number (L"");
		info->string_length_limit_reached = 0;
		/* start number */
		switch (text[pos])
		{
		case L'0':
			if (rcs_catc (info->temp->text, '0') != RS_OK)
			{
				return JSON_MEMORY;
			}

			info->state = 15;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state15;	/* number: leading zero */
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
			if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
			{
				return JSON_MEMORY;
			}

			info->state = 16;	/* number: decimal part */
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state16;	/* number: decimal part */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state15:			/* number: leading zero */
	{
		switch (text[pos])
		{
		case L'.':
			if (rcs_catc (info->temp->text, '.') != RS_OK)
			{
				return JSON_MEMORY;
			}


			info->state = 17;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state17;	/* number: start fractional part */

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
		case L',':
		case L'}':
		case L']':
			info->state = 38;
			goto state38;	/* fix literal cursor position */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state16:			/* number: decimal part */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH / 2)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}

				}
				else
				{
					info->string_length_limit_reached = 1;	/* string length limit was reached. drop every subsequent character. */
				}
			}

			info->state = 16;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state16;	/* number: decimal part */
			break;

		case L'.':
			info->string_length_limit_reached = 0;
			if (rcs_catc (info->temp->text, '.') != RS_OK)
			{
				return JSON_MEMORY;
			}

			info->state = 17;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state17;	/* number: start fractional part */

		case L'e':
		case L'E':
			if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
			{
				return JSON_MEMORY;
			}

			info->state = 19;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state19;	/* start exponential */

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
		case L',':
		case L'}':
		case L']':
			info->state = 38;
			goto state38;	/* fix literal cursor position */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state17:			/* number: start fractional part */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH / 2)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}

				}
				else
				{
					info->string_length_limit_reached = 1;	/* string length limit was reached. drop every subsequent character. */
				}
			}

			info->state = 18;	/* decimal part */
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state18;	/* number: fractional part */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state18:			/* number: fractional part */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH / 2)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}

				}
				else
				{
					info->string_length_limit_reached = 1;	/* string length limit was reached. drop every subsequent character. */
				}
			}

			info->state = 18;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state18;	/* number: fractional part */
			break;

		case L'e':
		case L'E':
			if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
			{
				return JSON_MEMORY;
			}


			info->state = 19;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state19;	/* number: start exponential part */

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
		case L',':	/* characters which mark the end of a number */
		case L'}':
		case L']':
			info->state = 38;
			goto state38;	/* fix literal cursor position */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state19:			/* number: start exponential part */
	{
		info->string_length_limit_reached = 0;
		switch (text[pos])
		{
		case L'+':
		case L'-':
			if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
			{
				return JSON_MEMORY;
			}


			info->state = 20;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state20;	/* number: exponential part following signal */
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
			if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
			{
				return JSON_MEMORY;
			}


			info->state = 21;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state21;	/* number: exponential part */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state20:			/* number: exponential part following signal */
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
			if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
			{
				return JSON_MEMORY;
			}

			info->state = 21;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state21;	/* number: exponential part */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state21:			/* number: exponential part */
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
			if (!info->string_length_limit_reached)
			{
				if (rcs_length (info->temp->text) < JSON_MAX_STRING_LENGTH)
				{
					if (rcs_catwc (info->temp->text, text[pos]) != RS_OK)
					{
						return JSON_MEMORY;
					}

				}
				else
				{
					info->string_length_limit_reached = 1;	/* string length limit was reached. drop every subsequent character. */
				}
			}
			info->state = 21;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state21;	/* exponential part */
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
		case L',':
		case L'}':
		case L']':
			info->state = 38;
			goto state38;	/* fix literal cursor position */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state22:			/* parse whitespaces until the end */
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_OK;
			else
				goto state22;
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
			break;
		}
	}


      state23:			/* value followup */
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			info->state = 23;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state23;	/* number followup */
			break;

		case L',':
			info->state = 24;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state24;	/* sibling */
			break;

		case L'}':
			info->state = 3;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state3;	/* end array */
			break;

		case L']':
			info->state = 4;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state4;	/* end array */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state24:			/* sibling */
	{
		if (info->cursor == NULL)	/* a new root must not be a root */
			return JSON_BAD_TREE_STRUCTURE;
		if ((info->cursor->type != JSON_OBJECT) && (info->cursor->type != JSON_ARRAY))
		{
			if (info->temp != NULL)
				json_free_value (&info->temp);
			if (info->cursor != NULL)
			{
				while (info->cursor->parent != NULL)
					info->cursor = info->cursor->parent;
				json_free_value (&info->cursor);
			}
			return JSON_BAD_TREE_STRUCTURE;
		}

		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			info->state = 24;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state24;
			break;

		case L'{':
			info->state = 2;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state2;	/* start object */
			break;

		case L'[':
			info->state = 37;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state37;	/* start array */
			break;

		case L'\"':
			info->state = 5;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state5;	/* start string */
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
			goto state14;	/* start number */
			break;

		case L't':
			info->state = 25;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state25;	/* true */
			break;

		case L'f':
			info->state = 29;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state29;	/* false */
			break;

		case L'n':
			info->state = 34;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state34;	/* true */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
	}

      state25:			/* true: t */
	{
		if (text[pos] != L'r')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}

		info->state = 26;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state26;	/* true: r */

	}

      state26:			/* true: r */
	{
		if (text[pos] != L'u')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}

		info->state = 27;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state27;	/* true: u */

	}

      state27:			/* true: u */
	{
		if (text[pos] != L'e')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
		info->temp = json_new_true ();
		info->state = 38;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state38;	/* fix literal cursor position */

	}

      state28:			/* label string followup */
	{
		switch (text[pos])
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			info->state = 28;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state28;	/* value in label:value pair */
			break;

		case L':':
			info->state = 13;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_INCOMPLETE_DOCUMENT;
			else
				goto state13;	/* pair */
			break;

		default:
			info->state = 39;
			goto state39;	/* handle JSON_ILLEGAL_CHAR error */
			break;
		}
	}

      state29:			/* false: f */
	{
		if (text[pos] != L'a')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
		info->state = 30;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state30;	/* false: a */

	}

      state30:			/* false: a */
	{
		if (text[pos] != L'l')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}

		info->state = 31;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state31;	/* false: l */

	}

      state31:			/* false: l */
	{
		if (text[pos] != L's')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}

		info->state = 32;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state32;	/* false: s */

	}

      state32:			/* false: e */
	{
		if (text[pos] != L'e')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
		info->temp = json_new_false ();

		/* fix the loose strings */
		info->state = 38;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state38;	/* fix literal cursor position */
	}

      state34:			/* null: n */
	{
		if (text[pos] != L'u')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}

		info->state = 35;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state35;	/* null: u */

	}

      state35:			/* null: u */
	{
		if (text[pos] != L'l')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}

		info->state = 36;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state36;	/* null: l 1 of 2 */
	}

      state36:			/* null: l 1 of 2 */
	{
		if (text[pos] != L'l')
		{
			info->state = 39;
			goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
		}
		/* set up the temp */
		info->temp = json_new_null ();
		/* move onto the next one */
		info->state = 38;
		pos++;
		if (pos > length)	/* current string buffer ended */
			return JSON_INCOMPLETE_DOCUMENT;
		else
			goto state38;	/* fix literal cursor position */
	}

      state37:			/* start array */
	{
		/* tree integrity checking */
		if (info->cursor)	/* cursor will be the parent node of this structure */
		{
			if ((info->cursor->type != JSON_ARRAY) && (info->cursor->type != JSON_STRING))
			{
				info->state = 39;
				goto state39;	/* handle the JSON_ILLEGAL_CHAR error */
			}
		}

		/* create the new children objects */
		info->temp = json_new_array ();

		/* create new tree node and add as child */
		if (info->cursor != NULL)
		{
			json_insert_child (info->cursor, info->temp);
		}
		/* traverse cursor up child node */
		info->cursor = info->temp;
		info->temp = NULL;

		info->state = 12;	/* start value in array */
		goto state12;	/* start value in array */
	}

      state38:			/* fix literal cursor position */
	{
		/* set the value */
		if (info->cursor == NULL)
		{
			info->cursor = info->temp;
			info->temp = NULL;

			info->state = 22;
			pos++;
			if (pos > length)	/* current string buffer ended */
				return JSON_OK;
			else
				goto state22;	/* parse whitespaces until the end */
		}
		else
		{
			json_insert_child (info->cursor, info->temp);
			info->temp = NULL;
		}
		/* fix cursor position */
		switch (info->cursor->type)
		{
		case JSON_STRING:
			if (info->cursor->parent == NULL)
			{
				info->state = 22;
				pos++;
				if (pos > length)	/* current string buffer ended */
					return JSON_OK;
				else
					goto state22;	/* parse whitespaces until the end */
			}

			info->cursor = info->cursor->parent;
			break;

		case JSON_ARRAY:
			break;

		default:
			if (info->temp != NULL)
				json_free_value (&info->temp);
			if (info->cursor != NULL)
			{
				while (info->cursor->parent != NULL)
					info->cursor = info->cursor->parent;
				json_free_value (&info->cursor);
			}
			return JSON_BAD_TREE_STRUCTURE;
			break;
		}

		/* move onto next state */
		info->state = 23;
		goto state23;	/* value followup */
	}

      state39:			/* handle the JSON_ILLEGAL_CHAR error */
	{
		if (info->temp != NULL)
			free (info->temp), info->temp = NULL;
		if (info->cursor != NULL)
		{
			while (info->cursor->parent != NULL)
				info->cursor = info->cursor->parent;
			json_free_value (&info->cursor);
		}
		return JSON_ILLEGAL_CHARACTER;
	}

	/* if the execution reached this part then we have a problem */
	return JSON_UNKNOWN_PROBLEM;
}


json_t *
json_parse_document (wchar_t * text)
{
	struct json_parsing_info jpi;
	enum json_error error;
	jpi.cursor = NULL;
	jpi.temp = NULL;
	jpi.state = 0;

	error = json_parse_string (&jpi, text, wcslen (text));
	if (error != JSON_OK)
	{
		/*/TODO check if jpi.temp is freed from within json_parse_string(); */
		return NULL;
	}
	else
	{
		return jpi.cursor;	/*/todo test if this is really the root node */
	}
}

enum json_error
json_saxy_parse (struct json_saxy_parser_status *jsps, struct json_saxy_functions *jsf, wchar_t c)
{
	/* temp variables */
	rwstring *temp;

	/* make sure everything is in it's place */
	assert (jsps != NULL);
	assert (jsf != NULL);
	temp = NULL;

	/* goto where we left off */
	switch (jsps->state)
	{
	case 0:		/* general state. everything goes. */
		goto state0;
		break;
	case 1:		/* parse string */
		goto state1;
		break;
	case 2:		/* parse string: escaped character */
		goto state2;
		break;
	case 3:		/* parse string: escaped unicode 1 */
		goto state3;
		break;
	case 4:		/* parse string: escaped unicode 2 */
		goto state4;
		break;
	case 5:		/* parse string: escaped unicode 3 */
		goto state5;
		break;
	case 6:		/* parse string: escaped unicode 4 */
		goto state6;
		break;
	case 7:		/* parse true: tr */
		goto state7;
		break;
	case 8:		/* parse true: tru */
		goto state8;
		break;
	case 9:		/* parse true: true */
		goto state9;
		break;
	case 10:		/* parse false: fa */
		goto state10;
		break;
	case 11:		/* parse false: fal */
		goto state11;
		break;
	case 12:		/* parse false: fals */
		goto state12;
		break;
	case 13:		/* parse false: false */
		goto state13;
		break;
	case 14:		/* parse null: nu */
		goto state14;
		break;
	case 15:		/* parse null: nul */
		goto state15;
		break;
	case 16:		/* parse null: null */
		goto state16;
		break;
	case 17:		/* parse number: 0 */
		goto state17;
		break;
	case 18:		/* parse number: start fraccional part */
		goto state18;
		break;
	case 19:		/* parse number: fraccional part */
		goto state19;
		break;
	case 20:		/* parse number: start exponent part */
		goto state20;
		break;
	case 21:		/* parse number: exponent part */
		goto state21;
		break;
	case 22:		/* parse number: exponent sign part */
		goto state22;
		break;
	case 23:		/* parse number: start negative */
		goto state23;
		break;
	case 24:		/* parse number: decimal part */
		goto state24;
		break;
	case 25:		/* open object */
		goto state25;
		break;
	case 26:		/* close object/array */
		goto state26;
		break;
	case 27:		/* sibling followup */
		goto state27;
		break;

	default:		/* oops... this should never be reached */
		return JSON_UNKNOWN_PROBLEM;
	}

      state0:			/* starting point */
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			break;

		case L'\"':	/* starting a string */
			jsps->string_length_limit_reached = 0;
			jsps->state = 1;
			break;

		case L'{':
			if (jsf->open_object != NULL)
				jsf->open_object ();
			jsps->state = 25;	/*open object */
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	/* close object/array */
			break;

		case L'[':
			if (jsf->open_array != NULL)
				jsf->open_array ();
/*                      jsps->state = 0;        // redundant*/
			break;

		case L']':
			if (jsf->close_array != NULL)
				jsf->close_array ();
			jsps->state = 26;	/* close object/array */
			break;

		case L't':
			jsps->state = 7;	/* parse true: tr */
			break;

		case L'f':
			jsps->state = 10;	/* parse false: fa */
			break;

		case L'n':
			jsps->state = 14;	/* parse null: nu */
			break;

		case L':':
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
/*                      jsps->state = 0;        // redundant*/
			break;

		case L',':
			if (jsf->sibling_separator != NULL)
				jsf->sibling_separator ();
			jsps->state = 27;	/* sibling followup */
			break;

		case L'0':
			jsps->string_length_limit_reached = 0;
			jsps->state = 17;	/* parse number: 0 */
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, L'0') != RS_OK)
			{
				return JSON_MEMORY;
			}
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
			jsps->string_length_limit_reached = 0;
			jsps->state = 24;	/* parse number: decimal */
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			break;

		case L'-':
			jsps->string_length_limit_reached = 0;
			jsps->state = 23;	/* number: */
			jsps->temp = NULL;
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, L'-') != RS_OK)
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

      state1:			/* parse string */
	{
		switch (c)
		{
		case L'\\':
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH - 1)	/* check if there is space for a two character escape sequence */
				{
					if (rws_catwc (jsps->temp, L'\\') != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 2;	/* parse string: escaped character */
			break;

		case L'\"':	/* end of string */
			if (jsps->temp != NULL)
			{
				jsps->state = 0;	/* starting point */
				if (jsf->new_string != NULL)
					jsf->new_string (jsps->temp->text);	/*copied or integral? */
				rws_free (&jsps->temp);
			}
			else
				return JSON_UNKNOWN_PROBLEM;
			break;

		default:
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH)	/* check if there is space for a two character escape sequence */
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			break;
		}
		return JSON_OK;
	}

      state2:			/* parse string: escaped character */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			break;

		case L'u':
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH - 4)
				{
					if (rws_catwc (jsps->temp, L'u') != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 3;	/* parse string: escaped unicode 1; */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state3:			/* parse string: escaped unicode 1 */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH - 3)
				{
					if (rws_catwc (jsps->temp, L'u') != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 4;	/* parse string. escaped unicode 2 */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state4:			/* parse string: escaped unicode 2 */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH - 2)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 5;	/* parse string. escaped unicode 3 */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state5:			/* parse string: escaped unicode 3 */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH - 1)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 6;	/* parse string. escaped unicode 4 */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state6:			/* parse string: escaped unicode 4 */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 1;	/* parse string */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state7:			/* parse true: tr */
	{
		if (c != L'r')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 8;	/* parse true: tru */
		return JSON_OK;
	}

      state8:			/* parse true: tru */
	{
		if (c != L'u')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 9;	/* parse true: true */
		return JSON_OK;
	}

      state9:			/* parse true: true */
	{
		if (c != L'e')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	/* back to general state. */
		if (jsf->new_true != NULL)
			jsf->new_true ();
		return JSON_OK;
	}

      state10:			/* parse false: fa */
	{
		if (c != L'a')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 11;	/* parse true: fal */
		return JSON_OK;
	}

      state11:			/* parse false: fal */
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 12;	/* parse true: fals */
		return JSON_OK;
	}

      state12:			/* parse false: fals */
	{
		if (c != L's')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 13;	/* parse true: false */
		return JSON_OK;
	}

      state13:			/* parse false: false */
	{
		if (c != L'e')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	/* general state. everything goes. */
		if (jsf->new_false != NULL)
			jsf->new_false ();
		return JSON_OK;
	}

      state14:			/* parse null: nu */
	{
		if (c != L'u')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 15;	/* parse null: nul */
		return JSON_OK;
	}

      state15:			/* parse null: nul */
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 16;	/* parse null: null */
		return JSON_OK;
	}

      state16:			/* parse null: null */
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	/* general state. everything goes. */
		if (jsf->new_null != NULL)
			jsf->new_null ();
		return JSON_OK;
	}

      state17:			/* parse number: 0 */
	{
		switch (c)
		{
		case L'.':
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, L'.') != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 18;	/* parse number: fraccional part */
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			jsps->state = 0;
			break;

		case L'}':
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	/* close object/array */
			break;

		case L']':

			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	/* close object/array */
			break;

		case L',':

			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	/* sibling followup */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}

		return JSON_OK;
	}

      state18:			/* parse number: start fraccional part */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH / 2)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 19;	/* parse number: fractional part */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state19:			/* parse number: fraccional part */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH / 2)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
/*                      jsps->state = 19;       // parse number: fractional part*/
			break;

		case L'e':
		case L'E':
			if (rws_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}

			jsps->state = 20;	/* parse number: start exponent part */
			break;


		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */

			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			jsps->state = 0;
			break;

		case L'}':

			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	/* close object/array */
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				if (jsps->temp == NULL)
					return JSON_MEMORY;
				jsf->new_number (jsps->temp->text);
				rws_free (&jsps->temp);
			}
			else
			{
				rws_free (&jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	/* close object/array */
			break;

		case L',':

			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	/* sibling followup */
			break;


		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state20:			/* parse number: start exponent part */
	{
		switch (c)
		{
		case L'+':
		case L'-':
			jsps->string_length_limit_reached = 0;
			if (rws_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}

			jsps->state = 22;	/* parse number: exponent sign part */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}

				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 21;	/* parse number: exponent part */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state21:			/* parse number: exponent part */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH)
				{
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
/*                              jsps->state = 21;       // parse number: exponent part*/
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */

			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			jsps->state = 0;
			break;

		case L'}':
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	/* close object */
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				if (jsps->temp == NULL)
					return JSON_MEMORY;
				jsf->new_number (jsps->temp->text);
				free (jsps->temp);
				jsps->temp = NULL;
			}
			else
			{
				free (jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	/* close object/array */
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				if (jsps->temp == NULL)
					return JSON_MEMORY;
				jsf->new_number (jsps->temp->text);
				free (jsps->temp);
				jsps->temp = NULL;
			}
			else
			{
				free (jsps->temp);
				jsps->temp = NULL;
			}
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	/* sibling followup */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state22:			/* parse number: start exponent part */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH)
				{
					rws_catwc (jsps->temp, c);
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
			jsps->state = 21;	/* parse number: exponent part */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state23:			/* parse number: start negative */
	{
		switch (c)
		{
		case L'0':
			rws_catwc (jsps->temp, c);
			jsps->state = 17;	/* parse number: 0 */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH / 2)
				{
					if ((jsps->temp = rws_create (1)) == NULL)
					{
						return JSON_MEMORY;
					}
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
					else
					{
						jsps->string_length_limit_reached = 1;
					}
				}
			}
			jsps->state = 24;	/* parse number: start decimal part */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state24:			/* parse number: decimal part */
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
			if (!jsps->string_length_limit_reached)
			{
				if (rws_length (jsps->temp) < JSON_MAX_STRING_LENGTH / 2)
				{
					if ((jsps->temp = rws_create (1)) == NULL)
					{
						return JSON_MEMORY;
					}
					if (rws_catwc (jsps->temp, c) != RS_OK)
					{
						return JSON_MEMORY;
					}
				}
				else
				{
					jsps->string_length_limit_reached = 1;
				}
			}
/*                              jsps->state = 24;       // parse number: decimal part*/
			break;

		case L'.':
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, L'.') != RS_OK)
			{
				return JSON_MEMORY;
			}

			jsps->state = 18;	/* parse number: start exponent part */
			break;

		case L'e':
		case L'E':
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}

			jsps->string_length_limit_reached = 0;	/* reset to accept the exponential part */
			jsps->state = 20;	/* parse number: start exponent part */
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			jsps->state = 0;
			break;

		case L'}':
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	/* close object/array */
			break;

		case L']':
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->open_object != NULL)
				jsf->close_array ();
			jsps->state = 26;	/* close object/array */
			break;

		case L',':
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (jsf->new_number != NULL)
			{
				jsf->new_number (jsps->temp->text);
			}
			rws_free (&jsps->temp);

			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			jsps->state = 27;	/* sibling followup */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state25:			/* open object */
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			break;

		case L'\"':
			jsps->temp = NULL;
			jsps->state = 1;
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
			jsps->state = 26;	/* close object */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state26:			/* close object/array */
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
/*                      jsp->state = 26;        // close object*/
			break;

		case L']':
			if (jsf->close_array != NULL)
				jsf->close_array ();
/*                      jsps->state = 26;       // close object/array*/
			break;

		case L',':
			if (jsf->sibling_separator != NULL)
				jsf->sibling_separator ();
			jsps->state = 27;	/* sibling followup */
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state27:			/* sibling followup */
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':	/* JSON insignificant white spaces */
			break;

		case L'\"':
			jsps->state = 1;
			jsps->temp = NULL;
			break;

		case L'{':
			if (jsf->open_object != NULL)
				jsf->open_object ();
			jsps->state = 25;	/*open object */
			break;

		case L'[':
			if (jsf->open_array != NULL)
				jsf->open_array ();
/*                      jsps->state = 0;        // redundant*/
			break;

		case L't':
			jsps->state = 7;	/* parse true: tr */
			break;

		case L'f':
			jsps->state = 10;	/* parse false: fa */
			break;

		case L'n':
			jsps->state = 14;	/* parse null: nu */
			break;

		case L'0':
			jsps->state = 17;	/* parse number: 0 */
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, L'0') != RS_OK)
			{
				return JSON_MEMORY;
			}
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
			jsps->state = 24;	/* parse number: decimal */
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			break;

		case L'-':
			jsps->state = 23;	/* number: */
			if ((jsps->temp = rws_create (1)) == NULL)
			{
				return JSON_MEMORY;
			}
			if (rws_catwc (jsps->temp, L'-') != RS_OK)
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

	return JSON_UNKNOWN_PROBLEM;
}


json_t *
json_find_first_label (const json_t * object, const wchar_t * text_label)
{
	json_t *cursor;
	char *tmp;
	assert (object != NULL);
	assert (text_label != NULL);
	assert (object->type == JSON_OBJECT);

	if (object->child == NULL)
		return NULL;
	cursor = object->child;
	while (cursor != NULL)
	{
		tmp = wchar_to_utf8 (text_label, wcslen (text_label));
		if (tmp == NULL)
		{
			/* memory allocation problem */
			return NULL;
		}
		if (strncmp (cursor->text->text, tmp, strlen (tmp)) == 0)
			return cursor;
		cursor = cursor->next;
	}
	return NULL;
}


size_t
utf8wcslen (const wchar_t * intext)
{
	size_t utfsize;		/* size of wchar_t string */
	size_t wsize;		/* size of the c string */

	assert (intext != NULL);
	/* get the size of the new wchar_t string */
	for (wsize = 0, utfsize = 0; wsize < wcslen (intext); wsize++)
	{
		if (intext[wsize] <= 0x7f)	/* one byte character */
		{
			++utfsize;
		}
		else if (intext[wsize] <= 0x7FF)	/* two byte character */
		{
			utfsize += 2;
		}
		else if (intext[wsize] <= 0xFFFF)	/* three byte character */
		{
			utfsize += 3;
		}
		else if (intext[wsize] <= 0x1FFFFF)	/* four byte character */
		{
			utfsize += 4;
		}
		else if (intext[wsize] <= 0x3FFFFFF)	/* five byte character */
		{
			utfsize += 5;
		}
		else if (intext[wsize] <= 0x7FFFFFFF)	/* five byte character */
		{
			utfsize += 6;
		}
		else
			return 0;
	}
	return utfsize;
}


char *
wchar_to_utf8 (const wchar_t * input, const size_t n)
{
	rcstring *output;	/* utf8 string */
	size_t wpos;		/* input string position */

	assert (input != NULL);

	output = rcs_create (n);

	/* convert the input string to the output string */
	for (wpos = 0; wpos < n; wpos++)
	{

		if (input[wpos] <= 0x7F)
		{
			rcs_catwc (output, input[wpos]);
		}
		else if (input[wpos] <= 0x7FF)
		{
			rcs_catwc (output, (input[wpos] >> 6) | 192);
			rcs_catwc (output, (input[wpos] & 63) | 128);
		}
		else if (input[wpos] <= 0xFFFF)
		{
			rcs_catwc (output, input[wpos] >> 12 | 224);
			rcs_catwc (output, (input[wpos] >> 6 & 63) | 128);
			rcs_catwc (output, (input[wpos] & 63) | 128);
		}
		else if (input[wpos] <= 0x1FFFFF)
		{
			rcs_catwc (output, input[wpos] >> 18 | 240);
			rcs_catwc (output, (input[wpos] >> 12 & 63) | 128);
			rcs_catwc (output, (input[wpos] >> 6 & 63) | 128);
			rcs_catwc (output, (input[wpos] & 63) | 128);
		}
		else if (input[wpos] <= 0x3FFFFFF)
		{
			rcs_catwc (output, input[wpos] >> 24 | 248);
			rcs_catwc (output, (input[wpos] >> 18 & 63) | 128);
			rcs_catwc (output, (input[wpos] >> 12 & 63) | 128);
			rcs_catwc (output, (input[wpos] >> 6 & 63) | 128);
			rcs_catwc (output, (input[wpos] & 63) | 128);
		}
		else if (input[wpos] <= 0x7FFFFFFF)
		{
			rcs_catwc (output, input[wpos] >> 30 | 252);
			rcs_catwc (output, (input[wpos] >> 24 & 63) | 128);
			rcs_catwc (output, (input[wpos] >> 18 & 63) | 128);
			rcs_catwc (output, (input[wpos] >> 12 & 63) | 128);
			rcs_catwc (output, (input[wpos] >> 6 & 63) | 128);
			rcs_catwc (output, (input[wpos] & 63) | 128);
		}
	}

	return rcs_unwrap (output);
}


size_t
utf8cslen (const char *input)
{
	size_t utf8pos = 0;
	size_t utf8_size = 0;
	while (utf8pos < strlen ((char *) input))
	{
		if ((unsigned char) input[utf8pos] <= 128)
		{
			++utf8pos;
		}
		else if ((unsigned char) input[utf8pos] <= 0xDF)
		{
			utf8pos += 2;
		}
		else if ((unsigned char) input[utf8pos] <= 0xEF)
		{
			utf8pos += 3;
		}
		else if ((unsigned char) input[utf8pos] <= 0xF7)
		{
			utf8pos += 4;
		}
		else if ((unsigned char) input[utf8pos] <= 0xFB)
		{
			utf8pos += 5;
		}
		else if ((unsigned char) input[utf8pos] <= 0xFD)
		{
			utf8pos += 6;
		}
		else
		{
			/* just in case there is a problem */
			return 0;
		}
		++utf8_size;
	}
	return utf8_size;
}


wchar_t *
utf8_to_wchar (const char *input, const size_t n)
{
	wchar_t *output;
	size_t utf8pos;
	size_t wpos;
	char i;			/* static loop counter */

	assert (input != NULL);
	output = calloc (sizeof (wchar_t), n + 1);	/* length + '\0' */
	if (output == NULL)
	{
		return NULL;
	}

	/* starting the conversion */
	utf8pos = 0;
	wpos = 0;

	while (utf8pos < n)
	{
		if ((input[utf8pos] & 0x80) == 0)
		{
			output[wpos] = input[utf8pos++];
		}
		else if ((input[utf8pos] & 0xE0) == 0xC0)
		{
			output[wpos] = (input[utf8pos++] & 0x1F) << 6;
			if ((input[utf8pos] & 0xC0) == 0x80)
			{
				output[wpos] |= (input[utf8pos++] & 63);
			}
			else
			{	/* Invalid utf8 string */
				free (output);
				return NULL;
			}
		}
		else if ((input[utf8pos] & 0xF0) == 0xE0)
		{
			output[wpos] = input[utf8pos++] & 0xF << 12;
			for (i = 1; i >= 0; i--)
			{
				if ((input[utf8pos] & 0xC0) == 0x80)
				{
					output[wpos] &= input[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					free (output);
					return NULL;
				}
			}

		}
		else if ((input[utf8pos] & 0xF8) == 0xF0)
		{
			output[wpos] = input[utf8pos++] & 0xF << 12;
			for (i = 2; i >= 0; i--)
			{
				if ((input[utf8pos] & 0xC0) == 0x80)
				{
					output[wpos] &= input[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					free (output);
					return NULL;
				}
			}

		}
		else if ((input[utf8pos] & 0xFC) == 0xF8)
		{
			for (i = 3; i >= 0; i--)
			{
				if ((input[utf8pos] & 0xC0) == 0x80)
				{
					output[wpos] &= input[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					free (output);
					return NULL;
				}
			}
		}
		else if ((input[utf8pos] & 0xFE) == 0xFC)
		{
			for (i = 4; i >= 0; i--)
			{
				if ((input[utf8pos] & 0xC0) == 0x80)
				{
					output[wpos] &= input[utf8pos++] & 0x3F << i;
				}
				else
				{	/* Invalid utf8 string */
					free (output);
					return NULL;
				}
			}
		}
		else		/* Invalid utf8 string */
		{
			free (output);
			return NULL;
		}
		++wpos;
	}
	output[wpos] = L'\0';
	return output;
}
