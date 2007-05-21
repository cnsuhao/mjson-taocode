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
		rs_catc ( text,c );
	}

	struct json_value *root = json_string_to_tree ( text->s );
	if ( root != NULL )
	{
		rs_destroy ( &text );
		text = rs_wrap( json_tree_to_string( root ) );
		json_free_value ( &root );
		if ( text != NULL )
		{
			printf ( "pass.\n%ls\n",text->s );
			rs_destroy ( &text );
			return 1;
		}
	}
// 	rs_destroy ( &text );
	printf ( "failed.\n" );
	return 0;
}



int main ( )
{
	setlocale(LC_CTYPE,"");
	// tests the example JSON document taken from the JSON site
// 	if ( !test_json_file ( "test/test1.json" ) )
// 	{
// 		printf ( "error\n" );
// 		return EXIT_SUCCESS;
// 	}
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

	rstring *foo = rs_create(L"goody");
	wchar_t *bar = rs_unwrap(foo);
	free(bar);
	return EXIT_SUCCESS;
}
