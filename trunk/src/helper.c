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

#include <string.h>
#include <memory.h>
#include <stdlib.h>

char * appendchar(char * text, char c)
{
	if(text == NULL)
	{
		text = malloc(2);	// space for the character and the string termination
		if(text == NULL)
			return NULL;
		text[0] = c;
		text[1] = 0;
	}
	else
	{
		size_t n = strlen(text);
		text = realloc(text,n+2);
		if(text == NULL)
			return NULL;
		text[n] = c;
		text[n+1] = 0;
	}
	return text;
}


char * appendstring(char *text, char * postfix)
{
	if(postfix == NULL)
		return NULL;
	if(text == NULL)
	{
		size_t newsize;
		newsize = strlen(postfix)+1;
		text = malloc(newsize);
		strncpy(text,postfix,newsize);
		return text;
	}
	else
	{
		size_t pre = strlen(text);
		size_t pos = strlen(postfix);
		text = realloc(text, pre+pos+1);
		strcpy(text+pre+1,postfix);
		return text;
	}
}
