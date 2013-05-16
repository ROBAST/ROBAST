/* ============================================================================

   Copyright (C) 2001, 2003, 2009, 2010  Konrad Bernloehr

   This file is part of the eventio/hessio library.

   The eventio/hessio library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library. If not, see <http://www.gnu.org/licenses/>.

============================================================================ */

/** @file straux.c
 *  @short Check for abbreviations of strings and get words from strings.
 *
 *  @author  Konrad Bernloehr
 *  @date    @verbatim CVS $Date: 2010/07/20 13:37:45 $ @endverbatim
 *  @version @verbatim CVS $Revision: 1.4 $ @endverbatim
 */

#define NO_INITIAL_MACROS 1
#include "initial.h"

#include <ctype.h>
#include "straux.h"

#ifndef CONST
# define CONST
#endif

/* =========================== abbrev =============================== */

/**
    Compare strings s and t. s may be an abbreviation of t.
    Upper/lower case in s is ignored. s has to be at least as
    long as the leading upper case, digit, and '_' part of t.

    @param  s   The string to be checked.
    @param  t   The test string with minimum part in upper case.

    @return  1 if s is an abbreviation of t, 0 if not.

 */

int abbrev (CONST char *s, CONST char *t)
{
   int had_lower;
   had_lower = 0;
   /* Look for end of matching part of both strings */
   for ( ; *s!='\0' && *t!='\0' && 
         tolower((int)(*s))==tolower((int)(*t)); ++s, ++t )
      if ( islower((int)(*t)) )
         had_lower = 1;
   if ( *s == '\0' && ( *t=='\0' || had_lower || islower((int)(*t)) ) )
      return(1);
   else
      return(0);
}

/* ========================== getword =============================== */

/**
   Copies a blank or '\\0' or < endchar > delimeted word
   from position *spos of the string s to the string word and
   increment *spos to the position of the first non-blank
   character after the word. The word must have a length less
   than or equal to maxlen.

   @param   s       string with any number of words.
   @param   spos    position in the string where we start and end.
   @param   word    the extracted word.
   @param   maxlen  the maximum allowed length of word.
   @param   blank   has the same effect as ' ', i.e. end-of-word.
   @param   endchar his terminates the whole string ( as '\\0' ).

   @return
	-2 : Invalid string or NULL
	-1 : The word was longer than maxlen (without the
	     terminating '\\0');
	 0 : There were no more words in the string s.
	 1 : ok, we have a word and there are still more of
	     them in the string s
	 2 : ok, but this was the last word

 */

int getword (CONST char *s, int *spos, char *word, int maxlen, char blank,
   char endchar)
{
   int i, j, n;

   if ( s==(char *)NULL || word==(char *)NULL || spos==(int *)NULL )
     {
      if ( word != (char *) NULL )
         word[0] = '\0';
      return(-2);
     }
   for ( ; s[*spos] == ' ' || s[*spos] == blank ; (*spos)++ )
      ;      /* Position to the first non-blank character */
   if ( s[*spos]=='\0' || s[*spos]=='\r' ||
        s[*spos]=='\n' || s[*spos] == endchar )
     {
      word[0] = '\0';
      return(0);
     }
   for ( i=(*spos), n=0 ;
         s[i]!='\0' && s[i]!='\n' && s[i]!='\r' && s[i]!='\t' && 
         s[i]!=' ' && s[i]!=blank && s[i]!=endchar ; i++ )
      n++ ;
   if ( s[i]!='\0' && s[i]!='\n' && s[i]!='\r' && s[i]!=endchar )
      for ( ; s[i] == '\t' || s[i] == ' ' || s[i] == blank; i++ )
         ;      /* Increment i until we have a non-blank character */
   for ( j=0; j<(n<maxlen?n:maxlen) ; j++)
      word[j] = s[*spos+j];
   word[j] = '\0';
   *spos = i;
   if ( n > maxlen )
      return(-1);
   if ( s[i]=='\0' || s[i]=='\n' || s[i]=='\r' || s[i] == endchar )
      return(2);
   else
      return(1);
}

/* ============================ stricmp ============================= */
/**
    Case independent comparison of character strings.

    @param  a, b  --  strings to be compared.

    @return        0 :  strings are equal (except perhaps for case)
		  >0 :  a is lexically 'greater' than b
		  <0 :  a is lexically 'smaller' than b

 */

#ifndef OS_MSDOS

int stricmp (CONST char *a, CONST char *b)
{
   int i, d;
   for ( i=0; a[i] || b[i]; i++ )
      if ( tolower((int)a[i]) != tolower((int)b[i]) )
      {
         if ( (d = tolower((int)a[i]) - tolower((int)b[i])) != 0 )
            return (d);
      }
   return (0);
}

#endif
