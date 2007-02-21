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


struct json_value* json_new_value ( enum json_value_type type )
{
	struct json_value *new_object;
	new_object = malloc ( sizeof ( struct json_value ) );
	new_object->text = NULL;
	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next =NULL;
	new_object->type = type;
	return new_object;
}

struct json_value* json_new_string ( char *text )
{
	struct json_value *new_object;
	new_object = malloc ( sizeof ( struct json_value ) );

	if(text)
	{
		new_object->text = malloc ( strlen(text) );
		strncpy ( new_object->text, text, strlen ( text ) );
	}
	else
		new_object->text = NULL;

	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next =NULL;
	new_object->type = JSON_STRING;
	return new_object;
}

struct json_value* json_new_number ( char *text )
{
	struct json_value *new_object;
	new_object = malloc ( sizeof ( struct json_value ) );
	
	if(text)
	{
		new_object->text = malloc ( strlen(text) );
		strncpy ( new_object->text, text, strlen ( text ) );
	}
	else
		new_object->text = NULL;

	new_object->parent = NULL;
	new_object->child = NULL;
	new_object->child_end = NULL;
	new_object->previous = NULL;
	new_object->next =NULL;
	new_object->type = JSON_NUMBER;
	return new_object;
}


struct json_value* json_new_object ( void )
{
	return json_new_value ( JSON_OBJECT );
}

struct json_value* json_new_array ( void )
{
	return json_new_value ( JSON_ARRAY );
}


void json_free_value ( struct json_value *value )
{
	///todo rethink this due to FIFO queue
	// free each and every child nodes
	if ( value->child != NULL )
	{
		///todo write function to free entire subtree recursively
		json_free_value ( value->child );
	}

	// fix sibling linked list connections
	if ( value->previous && value->next )
	{
		value->previous->next = value->next;
		value->next->previous = value->previous;
	}
	if ( value->previous )
	{
		value->previous->next = NULL;
	}
	if ( value->next )
	{
		value->next->previous = NULL;
	}

	//fix parent node connections
	if ( value->parent )
	{
		if ( value->parent->child == value )
		{
			if ( value->next )
				value->parent->child = value->next;	// the parent node always points to the first node
			else
				value->parent->child = NULL;
		}
	}

	//finally, free the memory
	free ( value );
}


enum json_errors json_insert_child ( struct json_value *parent, struct json_value *child )
{
	assert ( parent != NULL );	// the parent must exist
	assert ( child != NULL );	// the child must exist
	assert ( ( parent->type == JSON_OBJECT ) || ( parent->type == JSON_ARRAY ) || ( parent->type == JSON_STRING ) );	// must be a valid parent type
	///todo implement a way to enforce object->text->value sequence
	assert( !(parent->type == JSON_OBJECT && child->type == JSON_OBJECT));

	child->parent = parent;
	if ( parent->child )
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


enum json_errors json_insert_pair_into_object ( struct json_value *parent, struct json_value *label, struct json_value *value )
{
	// verify if the parameters are valid
	assert ( parent != NULL );
	assert ( label != NULL );
	assert ( value != NULL );
	// enforce type coherence
	assert ( parent->type == JSON_OBJECT );
	assert ( label->type == JSON_STRING );

	enum json_errors error;

	//insert value and check for error
	error = json_insert_child ( label,value );
	if ( error != JSON_OK )
		return error;
	//insert value and check for error
	error = json_insert_child ( parent,label );
	if ( error != JSON_OK )
		return error;

	return JSON_OK;
}

void json_render_tree_indented ( struct json_value *root, int level )
{
	assert ( root != NULL );
	int tab;
	for ( tab = 0; tab < level; tab++ )
	{
		printf ( "> " );
	}
	switch ( root->type )
	{
		case JSON_STRING:
			printf ( "STRING: %s\n",root->text );
			break;
		case JSON_NUMBER:
			printf ( "NUMBER: %s\n",root->text );
			break;
		case JSON_OBJECT:
			printf ( "OBJECT: \n" );
			break;
		case JSON_ARRAY:
			printf ( "ARRAY: \n" );
			break;
		case JSON_TRUE:
			printf ( "TRUE:\n" );
			break;
		case JSON_FALSE:
			printf ( "FALSE:\n" );
			break;
		case JSON_NULL:
			printf ( "NULL:\n" );
			break;
	}
	//iterate through children
	if ( root->child != NULL )
	{
		struct json_value *ita, *itb;
		ita = root->child;
		while ( ita != NULL )
		{
			json_render_tree_indented ( ita,level+1 );
			itb = ita->next;
			ita = itb;
		}
	}
}

void json_render_tree ( struct json_value *root )
{
	json_render_tree_indented ( root, 0 );
}

char *json_tree_to_string ( struct json_value* root )
{
	struct json_value* cursor = root;
	if(cursor == NULL)	// must try to render an existing tree
		goto end;

	char *output = NULL;
	size_t n;

state1:	// open value
	{
		if( (cursor->previous) && (cursor != root))	//TODO does this make sense?
		{
			n = strlen(",");
			if(output == NULL)
			{
				output = malloc(n);
			}
			else
			{
				output = realloc(output, strlen(output)+n);
			}
			strcat(output,",");
		}
		switch ( cursor->type )
		{
			case JSON_STRING:
				n = 2*strlen("\"")+strlen(cursor->text);
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output,strlen(output)+ n);
				}
				strcat ( output,"\"" );
				strcat ( output,cursor->text );
				strcat ( output,"\"" );
				if ( cursor->parent )
				{
					if ( cursor->parent->type == JSON_OBJECT )
					{
						n = strlen(":");
						output = realloc(output,strlen(output)+n);
						strcat ( output,":" );
						
					}
				}
				else
				{
					if ( cursor->child )
					{
						n = strlen(":");
						output = realloc(output,strlen(output)+n);
						strcat ( output,":" );
					}
					else
					{
						goto error;	// no root but siblings
					}
				}
				break;
				
			case JSON_NUMBER:
				n = strlen(cursor->text);
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output, strlen(output)+n);
				}
				strcat ( output,cursor->text );
				///todo integrity check
				goto state2;
				break;
				
			case JSON_OBJECT:
				n = strlen("{");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output, strlen(output)+n);
				}
				strcat ( output,"{" );
				if ( cursor->child )
				{
					cursor = cursor->child;
					goto state1;
				}
				else
				{
					goto state2;
				}
				break;
				
			case JSON_ARRAY:
				n = strlen("[");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output, strlen(output)+n);
				}
				strcat ( output,"[" );
				
				if ( cursor->child )
				{
					cursor = cursor->child;
					goto state1;
				}
				else
				{
					goto state2;
				}
				break;
				
			case JSON_TRUE:
				n = strlen("true");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output,strlen(output)+ n);
				}
				strcat ( output,"true" );
				
				///todo integrity check
				goto state2;
				break;
				
			case JSON_FALSE:
				n = strlen("false");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output, strlen(output)+n);
				}
				strcat ( output,"false" );
				
				///todo integrity check
				goto state2;
				break;

			case JSON_NULL:
				n = strlen("null");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output, strlen(output)+n);
				}
				strcat ( output,"null" );
				///todo integrity check
				goto state2;
				break;

			default:
				goto error;
		}
		if(cursor->child)
		{
			cursor = cursor->child;
			goto state1;
		}
		else
			goto state2;
	}

state2:	// close value
	{
			switch ( cursor->type )
		{
			case JSON_OBJECT:
				n = strlen("}");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output,strlen(output)+ n);
				}
				strcat(output,"}");
				break;
				
			case JSON_ARRAY:
				n = strlen("]");
				if(output == NULL)
				{
					output = malloc(n);
				}
				else
				{
					output = realloc(output,strlen(output)+ n);
				}
				strcat(output,"]");
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
		if(cursor->next)
		{
			cursor = cursor->next;
			goto state1;
		}
		else if((cursor->parent == NULL)||(cursor == root))
		{
			goto end;
		}
		else
		{
			cursor = cursor->parent;
			goto state2;
		}
	}

error:
	{
		printf("ERROR!");
		return NULL;	///todo finish error handling
	}
	
end:
	return output;
}


struct json_value * json_string_to_tree(char * text)
{
	size_t pos = 0;
	size_t length = strlen(text);
	struct json_value *cursor = NULL, *temp = NULL;

state1:	// start value
	{
		if(pos >= length)
			goto state20;	//end tree

		switch(text[pos])
		{
			case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state1;
				break;
			case '{': case '[':
				goto state2;	// start structure

			case '}': case ']':
				goto state3;	// end structure

			case '\"':
				goto state4;	// string

			case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				goto state5;	// number
				
			case 't':
				goto state6;	// true
			case 'f':
				goto state7;	// false
			case 'n':
				goto state8;	// null
			case ',':
				goto state10;	// sibling
			
			default:
				printf("Step 1: illegal character (%c) at position %i\n",text[pos], pos);
				goto error;
		}
	}

state2:	// start structure
	{
		//TODO  implement error tree coherency error checking
		if(text[pos] == '[')
			temp = json_new_array();
		else
			temp = json_new_object();

		pos++;
		if(pos >= length)
			goto state20;	//end tree

		if(cursor)	// cursor will be the parent node of this structure
		{
			switch ( cursor->type )	//make sure that the parent node can be the parent of a structure node
			{
				case JSON_ARRAY:	// types allowed
				case JSON_STRING:
					break;
				case JSON_OBJECT:	// types dissallowed
				case JSON_NUMBER:
				case JSON_TRUE:
				case JSON_FALSE:
				case JSON_NULL:
				default:
					printf("state 2: dissallowed parent type\n");
					goto error;
			}
		}
		
		// create new tree node and add as child
		if(cursor != NULL)
		{
			json_insert_child(cursor,temp);
		}
		// traverse cursor up child node
		cursor = temp;
		temp = NULL;
		goto state1;	// start value
	}
	
state3:	// end structure
	{
		if(cursor->parent == NULL)	// if current node is already the root node then we have ended.
		{
			//TODO implement extraction of child nodes from a text
			goto state20;	// end tree
		}
		else
		{
			cursor = cursor->parent;

			switch(cursor->type)
			{
				case JSON_STRING:
					///todo potential problem. does this type make sense as a child?
				case JSON_OBJECT:
					if(cursor->parent == NULL)
					{
						goto state20;	//end tree
					}
					else
					{
						cursor = cursor->parent;
					}
					break;
				case JSON_ARRAY:
					break;
					
				default:
					goto error;
			}

			// check following characters
			endstructure1:
			switch(text[pos])
			{
				case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto endstructure1;
					break;

				case '}':
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto state3;	// end structure

				case ',':
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto state10;	//sibling

				default:
					printf("Step 3: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
					
			}
			goto state1;
		}
	}

state4:	// process string
	{
		pos++;	//TODO the "enter string" state must receive the cursor past the \" character
		if(pos >= length)
			goto state20;	//end tree
		temp = json_new_string(NULL);

		// tree structure integrity check
		if(cursor)
		{
			switch(cursor->type)
			{
				case JSON_OBJECT:
				case JSON_ARRAY:
					break;
				case JSON_STRING:
					if(cursor->parent)
					{
						if(cursor->parent->type != JSON_OBJECT)	//a parent of a parent string must be an object
							goto error;
					}
					break;
					
				default:	// a string can't be a child of other values besides object, array and string
					goto error;
			}
		}

		// extract string
		str1:
		switch(text[pos])
		{
			case '\\':	// escaped characters
				if(temp->text == NULL)
				{
					temp->text = malloc(1);
				}
				else
				{
					temp->text = realloc(temp->text, strlen(temp->text)+1);
				}
					
				strncat(temp->text, &text[pos],1);	//appends single character
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				switch(text[pos])
				{
					case '\"':
					case '\\':
					case '/': case 'b': case 'f': case 'n': case 'r': case 't':
						temp->text = realloc(temp->text, strlen(temp->text)+1);
						strncat(temp->text, &text[pos],1);	//appends single character
						pos++;
						if(pos >= length)
							goto state20;	//end tree
						goto str1;
						break;
					//TODO implement hexadecimal part

					default:
						printf("Step 4: illegal character (%c) at position %i\n",text[pos], pos);
						goto error;
						
				}
				
			case '"':	// closing string
				if(cursor == NULL)
					cursor = temp;
				else
				{
					json_insert_child(cursor,temp);
					temp = NULL;
					switch ( cursor->type )
					{
						case JSON_ARRAY:
							break;
						case JSON_STRING:
							if(cursor->parent)
							{
								if(cursor->parent->type == JSON_OBJECT)
								{
									cursor = cursor->parent;	// point cursor at child label
								}
								else
								{
									printf("quack!\n");
									goto error;
								}
							}
							break;
						case JSON_OBJECT:
							cursor = cursor->child_end;	// point cursor at inserted child label
							break;
						case JSON_NUMBER:
						case JSON_TRUE:
						case JSON_FALSE:
						case JSON_NULL:
						default:
							goto error;
					}
				}
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state11;	// goto string followup
				break;
				
				
			default:
				if(temp->text == NULL)
				{
					temp->text = malloc(1);
				}
				else
				{
					temp->text = realloc(temp->text, strlen(temp->text)+1);
				}
				strncat(temp->text, &text[pos],1);	//appends single character
				pos++;
				if(pos >= length)
					goto state20;	//end tree
		}
		goto str1;
	}
	
state5: 	// process number
	{
		temp = json_new_number(NULL);
			// start number
			switch(text[pos])
			{
				case '-':
					if(temp->text == NULL)
					{
						temp->text = malloc(2);
						strncpy(temp->text,"-",2);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+1);
						strncat(temp->text, "-",1);	//appends single character
					}
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number3;	//decimal part
					break;
				case '0':
					if(temp->text == NULL)
					{
						temp->text = malloc(2);
						strncpy(temp->text,"0",2);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, "0",1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number2;	// leading zero
					break;
				case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
					if(temp->text == NULL)
					{
						temp->text = malloc(2);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number3;	// decimal part
					break;
					
				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}
			
		number2:	// leading zero
			switch(text[pos])
			{
				case '.':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, ".",1);
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number4;	// start fractional part
					
				case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto numberend;
					break;
					
				case ',': case '}': case ']':
					goto numberend;
					break;

				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}
			
		number3:	// decimal part
			switch(text[pos])
			{
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number3;	// decimal part
					break;
				
				case '.':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, ".",1);
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number4;	// fractional part

				case 'e': case 'E':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number6;	// start exponential
					
				case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto numberend;
					break;
					
				case ',': case '}': case ']':
					goto numberend;
					break;
						
				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}
			
		number4:	// start fractional part
			switch(text[pos])
			{
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number5;	// decimal part
					break;

				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}

		number5:	// fractional part
			switch(text[pos])
			{
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number5;	// decimal part
					break;

				case 'e': case 'E':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number6;	// start exponential

				case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto numberend;
					break;

				case ',': case '}': case ']':
					goto numberend;
					break;

				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}

		number6:	// start exponential part
			switch(text[pos])
			{
				case '+':
				case '-':
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number7;	// exponential part
					break;
					
				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}

		number7:	// start exponential part
			switch(text[pos])
			{
				case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
					if(temp->text == NULL)
					{
						temp->text = malloc(1);
					}
					else
					{
						temp->text = realloc(temp->text, strlen(temp->text)+ 1);
					}
					strncat(temp->text, &text[pos],1);	//appends single character
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto number7;	// exponential part
					break;

				case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto numberend;
					break;

				case ',': case '}': case ']':
					goto numberend;
					break;

				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}
		

		numberend:	// number end
				// where to go now?
			json_insert_child(cursor, temp);
			temp = NULL;
			if(cursor->type == JSON_STRING)
				cursor = cursor->parent;
			/* TODO
			potential error on:
			-> JSON malformed text
			-> started parsing child node on JSON text snippet

			needs to:
			-> perform error checking
			*/
			
			switch(text[pos])
			{
				case  '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto numberend;
					break;
				case ',':
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto state10;	// sibling
					break;

				case '}': case ']':
					pos++;
					if(pos >= length)
						goto state20;	//end tree
					goto state3;	// end structure

				default:
					printf("Step 5: illegal character (%c) at position %i\n",text[pos], pos);
					goto error;
			}
	}

state6:	// true
	{
		if(text[++pos] != 'r')
			goto error;
		if(text[++pos] != 'u')
			goto error;
		if(text[++pos] != 'e')
			goto error;
		switch(text[++pos])
		{
			case ',':
			case '}':
			case ']':
				case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
					json_insert_child(cursor,json_new_value(JSON_TRUE));
					goto state1;	// start structure

			default:
				printf("Step 6: illegal character (%c) at position %i\n",text[pos], pos);
				goto error;
		}
	}

state7:	// false
	{
		if(text[++pos] != 'a')
			goto error;
		if(text[++pos] != 'l')
			goto error;
		if(text[++pos] != 's')
			goto error;
		if(text[++pos] != 'e')
			goto error;
		pos++;
		if(pos >= length)
			goto state20;	//end tree
		json_insert_child(cursor,json_new_value(JSON_FALSE));

		if(cursor->type == JSON_STRING)
		{
			if(cursor->parent)
				cursor = cursor->parent;
		}
		
		false1:
		switch(text[pos])
		{
			case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
				pos++;	// ignore white spaces
				if(pos >= length)
					goto state20;	//end tree
				goto false1;
				break;

			case '}': case ']':
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state3;	// end structure
				break;
				
			case ',':
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state10;	// sibling
				break;
			
			default:
				printf("Step 7: illegal character (%c) at position %i\n",text[pos], pos);
				goto error;
		}
	}

state8:	// null
	{
		if(text[pos++] != 'u')
			goto error;
		if(text[pos++] != 'l')
			goto error;
		if(text[pos++] != 'l')
			goto error;
		switch(text[pos])
		{
			case ',':
			case '}':
			case ']':
			case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
				json_insert_child(cursor,json_new_value(JSON_NULL));
				goto state1;	// start structure

			default:
				goto error;
		}
	}	

state9:	// pair
	{
		switch(text[pos])
		{
			case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state9;	// restart loop
				break;
			case '{': case '[':
				goto state2;	// start structure

			case '\"':
				goto state4;	// string

			case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				goto state5;	// number
				break;
				
			case 't':
				goto state6;	// true
			case 'f':
				goto state7;	// false
			case 'n':
				goto state8;
			
			default:
				printf("Step 9: illegal character (%c) at position %i\n",text[pos], pos);
				goto error;
		}
	}

state10:	// sibling
	{
		//TODO perform tree integrity checks
		if(cursor == NULL)	// a new root must not be a root
			goto error;

		sibling1:
		switch(text[pos])
		{
			case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto sibling1;
				break;
			case '{': case '[':
				goto state2;	// start structure

			case '\"':
				goto state4;	// string

			case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
				goto state5;	// number
			case 't':
				goto state6;	// true
			case 'f':
				goto state7;	// false
			case 'n':
				goto state8;
			
			default:
				printf("Step 10: illegal character (%c) at position %i\n",text[pos], pos);
				goto error;
		}
	}

state11:	// string followup
	{
		switch(text[pos])
		{
			case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state11;
				break;
				
			case '}': case ']':
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state3;	// end structure
				break;
				
			case ':':
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state9;	// pair
				break;
				
			case ',':
				pos++;
				if(pos >= length)
					goto state20;	//end tree
				goto state10;	// sibling
				break;

			default:
				printf("Step 11: illegal character (%c) at position %i\n",text[pos], pos);
				goto error;
		}
	}

state20:	// end tree
	{
		if(cursor == NULL) goto end;	// only whitespaces. no tree.
		if(cursor->parent != NULL)
		{
			pos++;
			goto error;
		}
		else
		{
			state20clean:
			if(pos < length)
			{
				switch(text[pos])
				{
					
					case '\x20': case '\x09': case '\x0A': case '\x0D':	// white spaces
						pos++;
						if(pos >= length)
							goto state20;	//end tree
						goto state20clean;
						break;
					default:
						printf("Step 20: illegal character (%c) at position %i\n",text[pos], pos);
						goto error;
						
				}
			}
			goto end;
		}
	}

error:
	{
		printf("ERROR!\n");
		if(cursor != NULL)
		{
			// TODO implement a way to delete the entire JSON tree
			free(cursor);
		}
		return NULL;
	}

end:
{
	return cursor;
}
}




