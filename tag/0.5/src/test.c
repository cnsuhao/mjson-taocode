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

	json_t *root = json_new_object ();
	json_insert_pair_into_object (root, L"first", json_new_string (L"número um"));
	json_insert_pair_into_object (root, L"second", json_new_string (L"número dois"));
	json_insert_pair_into_object (root, L"third", json_new_string (L"número três"));
	json_insert_pair_into_object (root, L"fourth", json_new_string (L"número quatro"));

	wchar_t *test;
	json_tree_to_string (root, &test);
	wprintf (L"%ls\n", test);
	free(test);

	json_free_value (&root);
	return EXIT_SUCCESS;
}
