/***************************************************************************
 *   Copyright (C) 2007 by Rui Maciel   *
 *   rui_maciel@users.sourceforge.org   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

#include "json.h"


/**
The structure which holds the indentation properties
**/
struct options
{
	unsigned int use_tabs:1;	// tabs or spaces?
	unsigned int break_inside_object:1;
	unsigned int break_outside_object:1;
	unsigned int break_inside_array:1;
	unsigned int break_outside_array:1;
	unsigned int indent_inside_object:1;	// increment indentation after opening/before closing object
	unsigned int indent_outside_object:1;	// increment indentation before opening/after closing object
	unsigned int indent_inside_array:1;	// increment indentation after opening/before closing array
	unsigned int indent_outside_array:1;	// increment indentation before opening/after closing array

	unsigned int spaces_per_tab;
};


struct options opt;		// the program options
struct json_saxy_functions jsf;	// struct holding pointer to functions that will be invoked by the saxy parser
struct json_saxy_parser_status jsps;
unsigned int indent_level;	// parser's current indentation level

void
print_indentation (void)
{
	unsigned char i;
	if (opt.use_tabs)
	{
		for (i = 0; i < indent_level; i++)
			printf ("\t");
	}
	else
	{
		for (i = 0; i < indent_level * opt.spaces_per_tab; i++)
			printf (" ");
	}
}

// The parser functions

int
open_object (void)
{
	if (opt.indent_outside_object)
		indent_level++;
	if (opt.break_outside_object)
	{
		printf ("\n");
		print_indentation ();
	}
	printf ("{");
	if (opt.indent_inside_object)
		indent_level++;
	if (opt.break_inside_object)
	{
		printf ("\n");
		print_indentation ();
	}
}

int
close_object (void)
{
	if (opt.indent_inside_object)
		indent_level--;
	if (opt.break_inside_object)
	{
		printf ("\n");
		print_indentation ();
	}
	printf ("}");
	if (opt.indent_outside_object)
		indent_level--;
	if (opt.break_outside_object)
	{
		printf ("\n");
		print_indentation ();
	}
}

int
open_array (void)
{
	printf ("[");
}

int
close_array (void)
{
	printf ("]");
}

int
new_string (wchar_t * text)
{
	printf ("\"%ls\"", text);
}

int
new_number (wchar_t * text)
{
	printf ("%ls", text);
}

int
new_true (void)
{
	printf ("true");
}

int
new_false (void)
{
	printf ("false");
}

int
new_null (void)
{
	printf ("null");
}

int
label_value_separator (void)
{
	printf (":");
}

int
sibling_separator (void)
{
	printf (",");
	printf ("\n");
	print_indentation ();
}


/**
The function responsible to initialize all options and global variables
**/
void
constructor (void)
{
	///TODO set options from the command line
	// the options
	opt.use_tabs = 0;
	opt.break_inside_object = 1;
	opt.break_outside_object = 1;
	opt.break_inside_array = 1;
	opt.break_outside_array = 0;
	opt.indent_inside_object = 1;
	opt.indent_outside_object = 0;
	opt.indent_inside_array = 0;
	opt.indent_outside_array = 0;
	opt.spaces_per_tab = 1;

	// the global variables
	indent_level = 0;


	// set the JSON parser functions
	jsf.open_object = open_object;
	jsf.close_object = close_object;
	jsf.open_array = open_array;
	jsf.close_array = close_array;
	jsf.new_string = new_string;
	jsf.new_number = new_number;
	jsf.new_true = new_true;
	jsf.new_false = new_false;
	jsf.new_null = new_null;
	jsf.label_value_separator = label_value_separator;
	jsf.sibling_separator = sibling_separator;

	// init the JSON parser status struct
	jsps.state = 0;
	jsps.temp = NULL;
}


int
main (int argc, char *argv[])
{
	setlocale (LC_CTYPE, "");
	// set the default options and global variable values
	constructor ();

	// handle input through
	wint_t c;
	enum json_error error = JSON_OK;
	clearerr (stdin);

	// set stdin as a wide oriented stream
	fwide (stdin, 1);

	while (((c = fgetwc (stdin)) != WEOF) && (error == JSON_OK))
	{
//              printf("%lc",(wchar_t)c);
		error = json_saxy_parse (&jsps, &jsf, (wchar_t) c);
	}
	printf ("\n");
	///TODO: analize ending status: EOF or error?

	return EXIT_SUCCESS;
}
