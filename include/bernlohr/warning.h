/* ============================================================================

   Copyright (C) 2001, 2007, 2010  Konrad Bernloehr

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

/**
   @file warning.h
   @brief Pass warning messages to the screen or a usr function as set up.

   @author  Konrad Bernloehr
   @date    @verbatim CVS $Date: 2010/07/20 13:37:45 $ @endverbatim
   @version @verbatim CVS $Revision: 1.5 $ @endverbatim
*/

#ifndef WARNING_H__LOADED

#define WARNING_H__LOADED 1

#ifdef __cplusplus
extern "C" {
#endif

void warn_f_warning (const char *text, const char *origin, 
   int level, int msgno);
int set_warning (int level, int mode);
int set_default_warning (int level, int mode);
void warning_status (int *plevel, int *pmode);
void set_logging_function ( void (*user_function)(
   const char *, const char *, int, int) );
void set_default_logging_function ( void (*user_function) (
   const char *, const char *, int, int) );
int set_log_file (const char *fname);
void warn_f_output_text (const char *text);
void flush_output (void);
void set_output_function ( void (*user_function) (
   const char *) );
void set_default_output_function ( void (*user_function) (
   const char *) );
void set_aux_warning_function ( char *(*auxfunc) (
   void) );
void set_default_aux_warning_function ( char *(*auxfunc) (
   void) );
char *warn_f_get_message_buffer (void);

#ifndef WARNING_ORIGIN
# define WARNING_ORIGIN     (char *) NULL
#endif

#ifdef __cplusplus
inline void Information(const char *str) { warn_f_warning(str,WARNING_ORIGIN,0,0); }
inline void Warning(const char *str) { warn_f_warning(str,WARNING_ORIGIN,10,0); }
inline void Error(const char *str) {warn_f_warning(str,WARNING_ORIGIN,20,0); }

inline void Output(const char *str) { warn_f_output_text(str); }
#define WITH_INLINE_WARNING 1
#else
#define Information(string) warn_f_warning(string,WARNING_ORIGIN,0,0)
#define Warning(string)     warn_f_warning(string,WARNING_ORIGIN,10,0)
#define Error(string)       warn_f_warning(string,WARNING_ORIGIN,20,0)

#define Output(string)      warn_f_output_text(string)
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __WARNING_LOADED */
