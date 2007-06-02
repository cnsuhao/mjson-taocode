/*
*  C Implementation: saxy
*
* Description:
*
*
* Author: Rui Maciel <rui_maciel@users.sourceforge.net>, (C) 2007
*
* Copyright: See COPYING file that comes with this distribution
*
*/

#include "saxy.h"

#include <assert.h>
#include <wchar.h>

enum json_error
json_saxy_parse (struct json_saxy_parser_status *jsps, struct json_saxy_functions *jsf, wchar_t c)
{
	// make sure everything is in it's place
	assert (jsps != NULL);
	assert (jsf != NULL);
	// goto where we left off
	switch (jsps->state)
	{
	case 0:		// general state. everything goes.
		goto state0;
		break;

	case 1:		// parse string
		goto state1;
		break;

	case 2:		// parse string: escaped character
		goto state2;
		break;

	case 3:		// parse string: escaped unicode 1
		goto state3;
		break;

	case 4:		// parse string: escaped unicode 2
		goto state4;
		break;

	case 5:		// parse string: escaped unicode 3
		goto state5;
		break;

	case 6:		// parse string: escaped unicode 4
		goto state6;
		break;

	case 7:		// parse true: tr
		goto state7;
		break;

	case 8:		// parse true: tru
		goto state8;
		break;

	case 9:		// parse true: true
		goto state9;
		break;

	case 10:		// parse false: fa
		goto state10;
		break;

	case 11:		// parse false: fal
		goto state11;
		break;

	case 12:		// parse false: fals
		goto state12;
		break;

	case 13:		// parse false: false
		goto state13;
		break;

	case 14:		// parse null: nu
		goto state14;
		break;

	case 15:		// parse null: nul
		goto state15;
		break;

	case 16:		// parse null: null
		goto state16;
		break;

	case 17:		// parse number: 0
		goto state17;
		break;

	case 18:		// parse number: start fraccional part
		goto state18;
		break;

	case 19:		// parse number: fraccional part
		goto state19;
		break;

	case 20:		// parse number: start exponent part
		goto state20;
		break;

	case 21:		// parse number: exponent part
		goto state21;
		break;

	case 22:		// parse number: exponent sign part
		goto state22;
		break;

	case 23:		// parse number: start negative
		goto state23;
		break;

	case 24:		// parse number: decimal part
		goto state24;
		break;

	default:		// oops... this should never be reached
		return JSON_SOME_PROBLEM;
	}

	// starting point
      state0:
	{
		switch (c)
		{
		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			break;

		case L'\"':
			jsps->state = 1;
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			break;

		case L'{':
			if (jsf->open_object != NULL)
				jsf->open_object ();
			break;

		case L'}':
			if (jsf->close_object != NULL)
				jsf->close_object ();
			break;

		case L'[':
			if (jsf->open_array != NULL)
				jsf->open_array ();
			break;

		case L']':
			if (jsf->close_array != NULL)
				jsf->close_array ();
			break;

		case L't':
			jsps->state = 7;	// parse true: tr
			break;

		case L'f':
			jsps->state = 10;	// parse false: fa
			break;

		case L'n':
			jsps->state = 14;	// parse null: nu
			break;

		case L':':
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			break;

		case L',':
			if (jsf->sibling_separator != NULL)
				jsf->sibling_separator ();
			break;

			///todo implement number parsing
		case L'0':
			jsps->state = 17;	// parse number: 0
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, L'0') != RS_OK)
				return JSON_MEMORY;
			break;

		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			jsps->state = 24;	// parse number: decimal
			jsps->temp = rs_create (L"");	///TODO replace custom rstring with regular c-string handling
			if (jsps->temp == NULL)
				return JSON_MEMORY;
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			break;

		case L'-':
//                      jsps->state = 7;        // number
//                      jsps->temp = rs_create(L"");
//                      if(jsps->temp == NULL)
//                                      return JSON_MEMORY;
//                      if(rs_catwc(jsps->temp, c) != RS_OK)
//                      {
//                                      ///TODO does this need extra cleaning?
//                                      return JSON_MEMORY;
//                      }
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state1:			// parse string
	{
		switch (c)
		{
		case L'\\':
			if (rs_catwc (jsps->temp, L'\\') != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			jsps->state = 2;	// parse string: escaped character
			break;

		case L'\"':
			if (jsps->temp != NULL)
			{
				wchar_t *t = rs_unwrap (jsps->temp);
				jsps->state = 0;	// starting point
				if (jsf->new_string != NULL)
					jsf->new_string (t);
			}
			else
				return JSON_SOME_PROBLEM;	///TODO find out what is the best error return code for this situation
			break;

		default:
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			break;
		}
		return JSON_OK;
	}

      state2:			// parse string: escaped character
	{
		switch (c)
		{
		case L'\"':
		case L'\\':
		case L'/':
		case L'b':
		case L'f':
		case L'n':
		case L'r':
		case L't':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			break;

		case L'u':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				///TODO does this need extra cleaning?
				return JSON_MEMORY;
			}
			jsps->state = 3;	// parse string: escaped unicode 1;
			break;

		default:
			///todo cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state3:			// parse string: escaped unicode 1
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 4;	// parse string. escaped unicode 2
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state4:			// parse string: escaped unicode 2
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 5;	// parse string. escaped unicode 3
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state5:			// parse string: escaped unicode 3
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 6;	// parse string. escaped unicode 4
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state6:			// parse string: escaped unicode 4
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
		case L'a':
		case L'b':
		case L'c':
		case L'd':
		case L'e':
		case L'f':
		case L'A':
		case L'B':
		case L'C':
		case L'D':
		case L'E':
		case L'F':
			if (rs_catwc (jsps->temp, c) != RS_OK)
				return JSON_MEMORY;
			jsps->state = 1;	// parse string
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
		}
		return JSON_OK;
	}

      state7:			// parse true: tr
	{
		if (c != L'r')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 8;	// parse true: tru
		return JSON_OK;
	}

      state8:			// parse true: tru
	{
		if (c != L'u')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 9;	// parse true: true
		return JSON_OK;
	}

      state9:			// parse true: true
	{
		if (c != L'e')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	// general state. everything goes.
		if (jsf->new_true != NULL)
			jsf->new_true ();
		return JSON_OK;
	}

      state10:			// parse false: fa
	{
		if (c != L'a')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 11;	// parse true: fal
		return JSON_OK;
	}

      state11:			// parse false: fal
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 12;	// parse true: fals
		return JSON_OK;
	}

      state12:			// parse false: fals
	{
		if (c != L's')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 13;	// parse true: false
		return JSON_OK;
	}

      state13:			// parse false: false
	{
		if (c != L'e')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	// general state. everything goes.
		if (jsf->new_false != NULL)
			jsf->new_false ();
		return JSON_OK;
	}

      state14:			// parse null: nu
	{
		if (c != L'u')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 15;	// parse null: nul
		return JSON_OK;
	}

      state15:			// parse null: nul
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 16;	// parse null: null
		return JSON_OK;
	}

      state16:			// parse null: null
	{
		if (c != L'l')
		{
			return JSON_ILLEGAL_CHARACTER;
		}

		jsps->state = 0;	// general state. everything goes.
		if (jsf->new_null != NULL)
			jsf->new_null ();
		return JSON_OK;
	}

      state17:			// parse number: 0
	{
		switch (c)
		{
		case L'.':
			if (rs_catwc (jsps->temp, L'.') != RS_OK)
			{
				//TODO cleanup?
				return JSON_MEMORY;
			}
			jsps->state = 18;	// parse number: fraccional part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_object ();
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_array ();
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->label_value_separator ();
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}

		return JSON_OK;
	}

      state18:			// parse number: start fraccional part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 19;	// parse number: fractional part
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state19:			// parse number: fraccional part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
//                              jsps->state = 19;       // parse number: fractional part
			break;

		case L'e':
		case L'E':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 20;	// parse number: start exponent part
			break;


		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_object ();
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_array ();
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			break;


		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state20:			// parse number: start exponent part
	{
		switch (c)
		{
		case L'+':
		case L'-':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 22;	// parse number: exponent sign part
			break;

		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 21;	// parse number: exponent part
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state21:			// parse number: exponent part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
//                              jsps->state = 21;       // parse number: exponent part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_object ();
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_array ();
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state22:			// parse number: start exponent part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 21;	// parse number: exponent part
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state23:			// parse number: start negative
	{
		switch (c)
		{
		case L'0':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 17;	// parse number: 0
			break;

		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 24;	// parse number: start decimal part
			break;

		default:
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

      state24:			// parse number: decimal part
	{
		switch (c)
		{
		case L'0':
		case L'1':
		case L'2':
		case L'3':
		case L'4':
		case L'5':
		case L'6':
		case L'7':
		case L'8':
		case L'9':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
//                              jsps->state = 24;       // parse number: decimal part
			break;

		case L'.':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 18;	// parse number: start exponent part
			break;

		case L'e':
		case L'E':
			if (rs_catwc (jsps->temp, c) != RS_OK)
			{
				return JSON_MEMORY;
			}
			jsps->state = 20;	// parse number: start exponent part
			break;

		case L'\x20':
		case L'\x09':
		case L'\x0A':
		case L'\x0D':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			break;

		case L'}':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_object ();
			break;

		case L']':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->open_object != NULL)
				jsf->close_array ();
			break;

		case L',':
			if (jsf->new_number != NULL)
			{
				wchar_t *text = rs_unwrap (jsps->temp);
				jsps->temp = NULL;
				if (text == NULL)
					return JSON_MEMORY;
				jsf->new_number (text);
			}
			else
			{
				rs_destroy (&jsps->temp);
				jsps->temp = NULL;
			}
			jsps->state = 0;
			if (jsf->label_value_separator != NULL)
				jsf->label_value_separator ();
			break;

		default:
			//TODO cleanup?
			return JSON_ILLEGAL_CHARACTER;
			break;
		}
		return JSON_OK;
	}

	return JSON_SOME_PROBLEM;
}

#include "saxy.h"
