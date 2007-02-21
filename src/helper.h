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

 /** @file helper.h A small library set of utility funtions that are helpfull
\ingroup JSON

\note error handling is only in a very rudimentary form.
\author Rui Maciel rui.maciel@gmail.com
 */

/**
Appends a single character to the end of a string
\param text the string which will receive the character
\param c	the character which will be appended
\return the appended string
**/
char * appendchar(char * text, char c);

/**
Appends a null-terminated string to the end of a string
\param text the string which will receive the character
\param postfix the string which will be appended
\return the appended string
**/
char * appendstring(char *text, char * postfix);