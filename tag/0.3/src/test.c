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
main ()
{
	setlocale (LC_CTYPE, "");
	wchar_t text[80];

	struct json_parsing_info info;
	info.cursor = NULL;
	info.temp = NULL;
	info.state = 0;

	enum json_error error = 0;

	while ((!feof (stdin)) && (error == 0))
	{
		fgetws (text, 80, stdin);
		printf ("%ls", text);
		error = json_parse_string (&info, text);
		if (error != 0)
			printf ("error: %i\n", error);
	}

	if (error == 1)
	{
		wchar_t *output = json_tree_to_string (info.cursor);
		wchar_t *clean = json_strip_white_spaces (output);
		printf ("%ls\n", clean);
		free (output);
		free (clean);
	}

	return EXIT_SUCCESS;
}
