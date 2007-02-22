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

#include "rstring.h"

#include <stdlib.h>
#include <assert.h>


rstring *rs_create(const char *cstring)
{
	assert(cstring != NULL);
	rstring *rs = malloc(sizeof(rstring));
	if(rs == NULL)
		return NULL;

	rs->length = rs->max = strlen(cstring);

	rs->s = NULL;
	rs->s = malloc(rs->length + 1);
	if(rs->s == NULL)
		return NULL;

	strncpy(rs->s, cstring, rs->length);
	return rs;
}


void rs_destroy(rstring **rs)
{
	assert(rs != NULL);
	if(*rs != NULL)
	{
		if((*rs)->s != NULL)
		{
			free((*rs)->s);
			(*rs)->s = NULL;
		}
		free(*rs);
		*rs = NULL;
	}
	
}

rstring *rs_duplicate(rstring *copied)
{
	assert(copied != NULL);
	rstring *copy = malloc(sizeof(rstring));
	if(copy == NULL)
		return NULL;

	copy->s = malloc(1);
	if(copy->s == NULL)
		return NULL;
	copy->s[0] = 0;
	copy->length = copy->max = 0;
	
	if(rs_copyrs(copy,copied) == RS_OK)
		return copy;
	else
  		return NULL;
}


size_t rs_length(rstring *rs)
{
	assert(rs != NULL);
	return rs->length;
}


int rs_copyrs(rstring *to, const rstring *from)
{
	assert(to != NULL);
	if(from == NULL)
		return RS_OK;	// nothing to copy

	//TODO implement intelligent memory allocation
	if(to->max < from->length)
	{
		to->s = realloc(to->s,from->length + 1);
		if(to->s == NULL)
		{
			return RS_MEMORY;
		}

		to->max = from->length;
	}
	strncpy(to->s, from->s, from->length);
	to->s[from->length] = 0;
	to->length = from->length;
	return RS_OK;
}


int rs_copycs(rstring *to, const char *from, const size_t length)
{
	assert(to != NULL);

	if(from == NULL)
		return RS_OK;
	

	//TODO implement intelligent memory allocation
	if(to->max < length)
	{
		to->s = realloc(to->s,length + 1);
		if(to->s == NULL)
		{
			return RS_MEMORY;
		}

		to->max = length;
	}
	strncpy(to->s, from, length);
	to->s[length] = 0;
	to->length = length;
	return RS_OK;
}

int rs_catrs(rstring *pre, const rstring *pos)
{
	assert(pre != NULL);
	if(pos == NULL)
		return RS_OK;

	if(pre->max < pre->length + pos->length + 1)
	{
		pre->s = realloc(pre->s,pre->length + pos->length + 1);
		if(pre->s == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + pos->length + 1;
	}
	strncpy(pre->s+pre->length, pos->s, pos->length);
	pre->s[pre->length+pos->length] = 0;
	pre->length = pre->length + pos->length + 1;
	return RS_OK;
}

int rs_catcs(rstring *pre, const char *pos, const size_t length)
{
	assert(pre != NULL);
	if(pos == NULL)
		return RS_OK;

	if(pre->max < pre->length + length + 1)
	{
		pre->s = realloc(pre->s,pre->length + length + 1);
		if(pre->s == NULL)
		{
			return RS_MEMORY;
		}

		pre->max = pre->length + length + 1;
	}
	strncpy(pre->s+pre->length, pos, length);
	pre->s[pre->length+length] = 0;
	pre->length = pre->length + length + 1;	//is this the correct value? 
	return RS_OK;
}


