/* ============================================================================

   Copyright (C) 2001, 2010  Konrad Bernloehr

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

/** @file straux.h
 *  @short Check for abbreviations of strings and get words from strings.
 *
 *  @author  Konrad Bernloehr
 *  @date    @verbatim CVS $Date: 2010/07/20 13:37:45 $ @endverbatim
 *  @version @verbatim CVS $Revision: 1.2 $ @endverbatim
 */

#ifndef STRAUX_H__LOADED

#define STRAUX_H__LOADED 1

/*---------------------------------------------------------------------*/
/* PROTOTYPES */

#ifdef __cplusplus
extern "C" {
#endif

#ifdef ANSI_C

#ifndef CONST
# define CONST const
#endif

int abbrev (CONST char *s, CONST char *t);
int getword (CONST char *s, int *spos, char *word, int maxlen, char blank,
             char endchar);
int stricmp (CONST char *a, CONST char *b);


#endif

#ifdef __cplusplus
}
#endif

#endif
