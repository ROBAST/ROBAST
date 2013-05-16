/* ============================================================================

   Copyright (C) 1991, 2001, 2008, 2009, 2010  Konrad Bernloehr

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

/* ------------------------------------------------------------------ */
/** @file initial.h
    @short Indentification of the system and including some basic include file.
    
    @author  Konrad Bernloehr
    @date    1991 to 2010
    @date    @verbatim $Date: 2012/11/13 16:28:15 $ @endverbatim
    @version @verbatim $Revision: 1.14 $ @endverbatim
    
    This file identifies a range of supported operating systems
    and processor types. As a result, some preprocessor definitions
    are made. A basic set of system include files (which may vary
    from one system to another) are included.
    In addition, compatibility between different systems is improved,
    for example between K&R compiler systems and ANSI C compilers
    of various flavours.

@verbatim
    Identification of the host operating system (not CPU):
 
    Supported identifiers are
    OS_MSDOS
    OS_VAXVMS
    OS_UNIX
	+ variant identifiers like
	OS_ULTRIX, OS_LYNX, OS_LINUX, OS_DECUNIX, OS_AIX, OS_HPUX,
        OS_DARWIN (Mac OS X).
	Note: ULTRIX may be on VAX or MIPS, LINUX on Intel or Alpha,
	OS_LYNX on 68K or PowerPC.
    OS_OS9
 
    You might first reset all identifiers here.
 
    Then set one or more identifiers according to the system.
 
    Identification of the CPU architecture:

    Supported CPU identifiers are
       CPU_I86
       CPU_X86_64
       CPU_VAX
       CPU_MIPS
       CPU_ALPHA
       CPU_68K
       CPU_RS6000
       CPU_PowerPC
       CPU_HPPA
@endverbatim   
*/

/* ------------------------------------------------------------------ */

#ifndef INITIAL_H__LOADED

#define INITIAL_H__LOADED 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NO_IEEE_FLOAT_FORMAT
#define IEEE_FLOAT_FORMAT 1
#endif

#if ( defined(__x86_64__) || defined(__amd64__) )
# define CPU_X86_64 1
# define SIXTY_FOUR_BITS 1
#endif
#if ( defined(__i386__) || defined(__i386) )
# define CPU_I86
#endif
#if ( defined(__alpha) )
# define CPU_ALPHA 1
# define SIXTY_FOUR_BITS 1
#endif
#if ( defined(__powerpc__) )
# define CPU_PowerPC 1
#endif

#if ( defined(__WORDSIZE) )
# if ( __WORDSIZE == 64 )
#  define SIXTY_FOUR_BITS 1
# endif
#endif

#if ( defined(MSDOS) || defined(__MSDOS__) )  /* MS C or Turbo C */
#   define OS_MSDOS 1
#   define CPU_I86 1
#elif ( defined(vax) && defined(vms) )  /* VAX/VMS (what about OpenVMS?) */
#   define OS_VAXVMS 1
#   define VAX_FLOAT_FORMAT 1
#   undef IEEE_FLOAT_FORMAT
#   define CPU_VAX 1
#   define MISSING_SNPRINTF 1
#elif ( defined(linux) || defined(__linux__) ) /* Linux OS */
#   define OS_UNIX 1
#   define OS_LINUX 1
#elif ( defined(ultrix) )  /* Ultrix (MIPS or VAX) */
#   define OS_UNIX 1
#   define OS_ULTRIX 1
#   ifndef ULTRIX
#      define ULTRIX 1
#   endif
#   ifndef vax
#      define CPU_MIPS 1
#   else
#      define VAX_FLOAT_FORMAT 1
#      undef IEEE_FLOAT_FORMAT
#      define CPU_VAX 1
#   endif
#   define MISSING_SNPRINTF 1
#elif ( defined(lynx) || defined(__lynx__) ) /* Lynx-OS */
#   define OS_UNIX 1
#   define OS_LYNX 1
#   if ( defined(_ARCH_PPC) || defined(__powerpc__) ) /* Lynx-OS on PowerPC */
#      define CPU_PowerPC 1
#   elif ( defined(M68K) ) /* Lynx-OS on Motorola 68K */
#      define CPU_68K 1
#   endif
#   define MISSING_SNPRINTF 1
#elif ( defined(AIX) )  /* IBM RS/6000 or PowerPC under AIX */
#   define OS_UNIX 1
#   define OS_AIX 1
#   define CPU_RS6000 1
#   define CPU_PowerPC 1
#elif ( defined(__osf__) && ( defined(__alpha) || defined(__alpha__) ) ) /* DEC Unix */
#   define OS_UNIX 1
#   define OS_DECUNIX 1
#   define CPU_ALPHA 1
#   define SIXTY_FOUR_BITS 1
#   define MISSING_SNPRINTF 1
#elif ( defined(__APPLE__) && defined(__MACH__) ) /* Mac OS X (at least) */
#   define OS_UNIX 1
#   define OS_DARWIN 1
#elif ( defined(OSK) )  /* OS-9 68k */
#   define OS_OS9 1
#   define CPU_68K 1
#   define FSTAT_NOT_AVAILABLE 1
#elif ( defined(M68K) )  /* seems to be LynxOS on a 68k */
#   define OS_UNIX 1
#   define OS_LYNX 1
#   define CPU_68K 1
#elif ( defined(__hpux) )  /* HP-UX */
#   define OS_HPUX 1
#   define OS_UNIX 1
#   ifdef __hppa
#      define CPU_HPPA 1
#   endif 
#else
#   if defined(unix) || defined(__unix)
#      define OS_UNIX 1    /* UNIX variant and CPU type are unknown */
#   endif
#endif

/* On UNIX the version of files extracted from SCCS are available for 'what' */
#ifdef OS_UNIX
#   ifndef lint
#      define USING_SCCS 1
#      define USING_SCCS_ID 1
#   endif
#endif

/* The identifier 'ANSI_C' tells that a compiler supporting ANSI C is used */
#if defined(__STDC__) /* Usually means 'ANSI C and nothing more than ANSI C' */
#   ifndef ANSI_C
#      define ANSI_C 1
#   endif
#endif

/* On the following systems all known C compilers (should) support ANSI C: */
#if ( defined(OS_MSDOS) || defined(OS_VAXVMS) || defined(OS_UNIX) )
#   ifndef ANSI_C
#      define ANSI_C 1
#   endif
#endif

/* To test the code the 'ANSI_C' identifier may be reset */
#ifdef SIMPLE_C
#   ifdef ANSI_C
#      undef ANSI_C
#   endif
#endif

/* Which types of systems have a VME bus? */
#if ( defined(OS_OS9) || defined(OS_LYNX) )
#   define VME_BUS 1
#endif

/* -------------------------------------------------- */

#ifndef NO_INITIAL_INCLUDES

/* Include standard OS-dependent include files */
#if ( defined(OS_OS9) || defined(BSD) )
#   include <strings.h>
#else
#   include <string.h>
#endif

#include <stdio.h>
#include <math.h>
#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif
#include <time.h>
#if defined(ANSI_C) || defined(__STDC__)
#   include <stdlib.h>
#endif
#ifdef OS_UNIX
#   include <unistd.h>
#endif
#ifdef OS_MSDOS
#   include <io.h>
#endif
#endif
/* ^-- end of '#ifndef NO_INITIAL_INCLUDES' */

/* -------------------------------------------------- */

#if ANSI_C
#   define ARGLIST(a) a
#else
#   define ARGLIST(a) ()
#endif

/* -------------------------------------------------- */

#ifndef NO_INITIAL_MACROS

#ifndef ANSI_C
#   ifndef NULL
#      define NULL (void *) 0
#   endif
#   ifdef OS_OS9
  typedef unsigned size_t;
#   endif
#   define ARGLIST(a) ()
#endif

/* Definitions normally found in include files: */
/* OS-9 include files don't define SEEK_CUR */
#ifndef SEEK_CUR
#   define SEEK_CUR 1
#endif
/* OS-9 does not provide a isatty() function */
#ifdef OS_OS9
#   define ISATTY_NOT_AVAILABLE 1
#   define strchr(arg1,arg2) index(arg1,arg2)
char *getenv ARGLIST((char *));
#endif
#ifdef OS_LYNX
#ifdef CPU_PowerPC
    int bcopy ARGLIST((char *b, char *a, int n));
#else
#   define memmove(a,b,n) bcopy(b,a,n)
    void bcopy ARGLIST((char *b, char *a, int n));
#endif
#   define tsleep(t) usleep(10000*t)
#endif

/* fopen() options needed for portability to MS-DOS */
#ifdef OS_MSDOS
#   define WRITE_TEXT    "wt"
#   define WRITE_BINARY  "wb"
#   define READ_TEXT     "rt"
#   define READ_BINARY   "rb"
#   define APPEND_TEXT   "at"
#   define APPEND_BINARY "ab"
#else
#   define WRITE_TEXT    "w"
#   define WRITE_BINARY  "w"
#   define READ_TEXT     "r"
#   define READ_BINARY   "r"
#   define APPEND_TEXT   "a"
#   define APPEND_BINARY "a"
#endif

#ifndef __cplusplus

/* Nearest integer macro */
#define Nint(a) (((a)>=0.)?((long)(a+0.5)):((long)(a-0.5)))
/* Absolute value independent of data type */
#define Abs(a) (((a)>=0)?(a):(-1*(a)))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifdef CRT
/* Old names for the above macros as used in CRT code */
#   define NINT(a) Nint(a)
#   define ABS(a) Abs(a)
#endif
/* Minimum and maximum macros (same definition as in X11) */
#ifndef min
#   define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#   define max(a,b) ((a)>(b)?(a):(b))
#endif

#define REGISTER register
#else
inline long int Nint(double a) { return (((a)>=0.)?((long)(a+0.5)):((long)(a-0.5))); }
#define REGISTER
#endif
/* ^-- end of '#ifndef __cplusplus' */
#endif
/* ^-- end of '#ifndef NO_INITIAL_MACROS' */

/* -------------------------------------------------- */

#define CONST_QUAL

/* =========== Integer data types ============ */

#ifdef __STDC__
# ifdef __STDC_VERSION__
#  if ( __STDC_VERSION__ >= 199901L )
#   define ANSI_C_99 1
#  endif
# elif ( !defined(__STRICT_ANSI__) && defined(__GNUC__) )
/* GCC version 3 and up should be ANSI C 99 capable unless */
/*  -ansi or -traditional or -std=c89 flags have been used. */
#  if ( __GNUC__ >= 3 )
#   define ANSI_C_99 1
#  endif
# endif
#endif

#ifdef ANSI_C_99
# undef CONST_QUAL
# define CONST_QUAL const /* Qualifier for constant data */
#endif

#ifdef __cplusplus
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS 1
#endif
#endif

#if defined(ANSI_C) || defined(__STDC__) || defined(__cplusplus)
# include <sys/types.h>
#endif

#if defined(ANSI_C_99) || defined(__int8_t_defined) || defined(__cplusplus)
/* ANSI C99 (C9x) has the following types specified in stdint.h */
/* Fairly recent gcc implementations have them as well (in sys/types.h) */
/* but stdint.h is provided as well and duplication does no harm. */
# include <stdint.h>
/* Older (2.7.x) gcc versions have some of them defined in */
/* <sys/bitypes.h> (which is included from <sys/types.h>) and the rest */
/* is slightly different but we have no stdint.h. */
#elif defined(__BIT_TYPES_DEFINED__)
  typedef u_int8_t uint8_t;
  typedef u_int16_t uint16_t;
  typedef u_int32_t uint32_t;
#else
/* Only for the older compilers we have to declare all the types here. */
# if !defined(__GNUC__) && !defined(OS_UNIX)
  typedef /*signed*/ char int8_t;
# else
  typedef signed char int8_t;
# endif
  typedef unsigned char uint8_t;

  typedef short int16_t;
  typedef unsigned short uint16_t;

# ifdef OS_MSDOS
  typedef long int32_t;
  typedef unsigned long uint32_t;
# else
  typedef int int32_t;
  typedef unsigned int uint32_t;
# endif
# if defined(SIXTY_FOUR_BITS)
  typedef long int64_t;
  typedef unsigned long uint64_t;
# endif
# if defined(__INTMAX_TYPE__)
  typedef __INTMAX_TYPE__ intmax_t;
# else
  typedef long intmax_t;
# endif
# if defined(__UINTMAX_TYPE__)
  typedef __UINTMAX_TYPE__ uintmax_t;
# else
  typedef unsigned long uintmax_t;
# endif
#endif

/* This should do it for most other 64 bit systems than DEC Unix */
#ifndef SIXTY_FOUR_BITS
# ifdef __WORDSIZE
#  if __WORDSIZE == 64
#   define SIXTY_FOUR_BITS 1
#  endif
# endif
#endif

/* Many 32 bit systems have long long int as 64 bit integer */
#if defined(SIXTY_FOUR_BITS) || ( defined(INT64_MAX) && defined(__GNUC__) )
# define HAVE_64BIT_INT 1
#endif

#ifdef MISSING_SNPRINTF
  extern int snprintf ARGLIST((char *s, size_t l, CONST_QUAL char *fmt, ...));
#endif

#ifdef __cplusplus
}
#endif

/* Optional memory debugging includes. */
#ifdef DMALLOC
# include <dmalloc.h>
#endif

#ifdef USE_MPATROL
# include <mpatrol.h>
#endif

#if defined(INTMAX_MAX) || defined(__cplusplus)
# define WITH_INTMAX_T 1
#endif
#if defined(UINTMAX_MAX) || defined(__cplusplus)
# define WITH_UINTMAX_T 1
#endif

#endif
