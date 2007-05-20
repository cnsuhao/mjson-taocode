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
#include "rstring/rstring.h"


int test_json_file ( char *filename )
{
	FILE *file;
	printf ( "%s:\n",filename );
	file = fopen ( filename,"r" );
	if ( file == NULL )
		return 0;
	rstring *text = rs_create ( L"" );
	char c;
	while ( ( c = getc ( file ) ) != EOF )
	{
		if ( json_white_space ( c ) == 0 )
			rs_catc ( text,c );
	}

	printf("%ls\n",text->s);

	struct json_value *root = json_string_to_tree ( text->s );
	if ( root != NULL )
	{
		if ( json_tree_to_string ( root ) != NULL )
		{
			rs_destroy ( &text );
			json_free_value ( &root );
			printf ( "pass.\n" );
			return 1;
		}
	}
	rs_destroy ( &text );
	printf ( "failed.\n" );
	return 0;
}



int main ( )
{
	setlocale(LC_CTYPE,"");
	// tests the example JSON document taken from the JSON site
	if ( !test_json_file ( "test/test1.json" ) )
	{
		printf ( "error\n" );
		return EXIT_SUCCESS;
	}
// 	if ( !test_json_file ( "test/test2.json" ) )
// 	{
// 		printf ( "error\n" );
// 		return EXIT_SUCCESS;
// 	}
// 	if ( !test_json_file ( "test/test3.json" ) )
// 	{
// 		printf ( "error\n" );
// 		return EXIT_SUCCESS;
// 	}
// 	if ( !test_json_file ( "test/test4.json" ) )
// 	{
// 		printf ( "error\n" );
// 		return EXIT_SUCCESS;
// 	}
// 	if ( !test_json_file ( "test/test5.json" ) )
// 	{
// 		printf ( "error\n" );
// 		return EXIT_SUCCESS;
// 	}
	return EXIT_SUCCESS;
}
