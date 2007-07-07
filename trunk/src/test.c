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

	wchar_t *test = L"{\"test\":\"\\u0050\\u0050\"}";
	wprintf (L"%ls\n", test);

	struct json_value *root = json_parse_document (test);

	if (root == NULL)
	{
		wprintf (L"snafu\n");
		json_free_value (&root);
		return EXIT_FAILURE;
	}
	else
	{
		wprintf (L"style\n");
	}

	json_render_tree (root);

	json_free_value (&root);
	return EXIT_SUCCESS;
}
