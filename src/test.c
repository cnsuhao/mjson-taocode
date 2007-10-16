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
#include <string.h>

#include "json.h"

#define BUFFER 80
int
main ()
{
	// set the variables
	setlocale (LC_CTYPE, "");
	wchar_t buffer[BUFFER];
	FILE *file;

	struct json_parsing_info jpi;
	jpi.cursor = NULL;
	jpi.temp = NULL;
	jpi.state = 0;
	enum json_error error = JSON_OK;

	// open the file
	file = fopen("documents/test6.json", "r");
	if(file == NULL)
	{
		printf("error opening file\n");
		return 1;
	}
	fwide(file,1);

	// parse the file
	while( (fgetws(buffer,BUFFER, file) != NULL) && ((error == JSON_OK) || error == JSON_INCOMPLETE_DOCUMENT) )
	{
		printf("%ls\n",buffer);
		error = json_parse_string (&jpi, buffer, wcslen(buffer));
		switch (error)
		{
			case JSON_INCOMPLETE_DOCUMENT:
				printf("incomplete\t");
			case JSON_OK:
				break;

			default:
				printf("some error\n");
				return;
				break;
		}
		printf("%ls",buffer);
	}
	fclose(file);

	// render the tree
	json_render_tree(jpi.cursor);
	return EXIT_SUCCESS;
}
