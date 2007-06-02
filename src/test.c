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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

#include "json.h"


int
open_object ()
{
	printf ("{\n");
	return 1;
}

int
close_object ()
{
	printf ("}\n");
	return 1;
}

int
new_string (wchar_t * text)
{
	printf ("%ls\n", text);
	return 1;
}

int
new_number (wchar_t * text)
{
	printf ("%ls\n", text);
	return 1;
}


int
main ()
{
	setlocale (LC_CTYPE, "");

	wchar_t text[80];
	struct json_parsing_info info;
	info.cursor = NULL;
	info.temp = NULL;
	info.state = 0;

//      enum json_error error = 0;
	enum json_error error = 1;

	struct json_saxy_functions jsf;
	jsf.open_object = &open_object;
	jsf.close_object = &close_object;
	jsf.open_array = NULL;
	jsf.close_array = NULL;
	jsf.new_string = &new_string;
	jsf.new_number = &new_number;
	jsf.new_true = NULL;
	jsf.new_false = NULL;
	jsf.new_null = NULL;
	jsf.sibling_separator = NULL;
	jsf.label_value_separator = NULL;

	struct json_saxy_parser_status jsps;
	jsps.state = 0;
	jsps.temp = NULL;

	while ((!feof (stdin)) && (error == 1))
	{
		fgetws (text, 80, stdin);
//              printf ("%ls", text);
//              error = json_parse_string (&info, text);
		error = JSON_OK;
		size_t i = 0;
		while ((error == JSON_OK) && (i < wcslen (text)))
		{
			error = json_saxy_parse (&jsps, &jsf, text[i]);
			i++;
		}
		if (error != 0)
			printf ("value returned: %i\n", error);
	}

//      if (error == 1)
//      {
//              wchar_t *output = json_tree_to_string (info.cursor);
//              wchar_t *clean = json_strip_white_spaces (output);
//              printf ("%ls\n", clean);
//              free (output);
//              free (clean);
//      }

	return EXIT_SUCCESS;
}
