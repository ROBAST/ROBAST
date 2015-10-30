/* ============================================================================

   Copyright (C) 2001, 2009, 2010  Konrad Bernloehr

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
   @file warning.c
   @brief Pass warning messages to the screen or a usr function as set up.

    @author  Konrad Bernloehr
    @date    @verbatim CVS $Date: 2014/02/20 10:53:06 $ @endverbatim
    @version @verbatim CVS $Revision: 1.9 $ @endverbatim

   One of the most import parameter for setting up the bevaviour
   is the warning level:

@verbatim   
   ------------------------------------------------------------
    Warning level: The lowest level of messages to be displayed
   ------------------------------------------------------------
    Warning mode:
    bit 0: display on screen (stderr),
    bit 1: write to file,
    bit 2: write with user-defined logging function.
    bit 3: display origin if supplied.
    bit 4: open log file for appending.
    bit 5: call auxilliary function for time/date etc.
    bit 6: use the auxilliary function output as origin string
	   if no explicit origin was supplied.
    bit 7: use syslog().
   ------------------------------------------------------------
@endverbatim
*/

#include "initial.h"
#define __WARNING_MODULE 1
#include "warning.h"
#include <errno.h>

/** A struct used to store thread-specific data. */

struct warn_specific_data
{
   /* Warning level: The lowest level of messages to be displayed */
   int warninglevel;
/*
 * -----------------------------------------------------------
 *  Warning mode:
 *  bit 0: display on screen (stderr),
 *  bit 1: write to file,
 *  bit 2: write with user-defined logging function.
 *  bit 3: display origin if supplied.
 *  bit 4: open log file for appending.
 *  bit 5: call auxilliary function for time/date etc.
 *  bit 6: use the auxilliary function output as origin string
 *         if no explicit origin was supplied.
 *  bit 7: use syslog().
 * -----------------------------------------------------------
 */
   int warningmode;
   char output_buffer[2048];
   /** The name of the log file. Used only when opening the file. */
   const char *logfname;
   char saved_logfname[256];
   int buffered;
   FILE *logfile;
   void (*log_function) (const char *, const char *, int, int);
   void (*output_function) (const char *);
   char *(*aux_function) (void);
   int recursive;
};

static struct warn_specific_data warn_defaults =
{ 
   0,    /* warninglevel */
   1+8,  /* warningmode */
   "",   /* output_buffer */
   "warning.log", /* logfname */
   "",   /* saved_logfname */
   0,    /* buffered */
   NULL, /* logfile */
   NULL, /* log_function */
   NULL, /* output_function */
   NULL, /* aux_function */
   0     /* recursive */
};

#ifdef _REENTRANT

#include <pthread.h>

#ifndef PTHREAD_ONCE_INIT
#define PTHREAD_ONCE_INIT pthread_once_init
#define PTHREAD_MUTEX_INITIALIZER 0
#endif

/** Global key for thread-specific data. */ 
static pthread_key_t warn_tsd_key;
static pthread_once_t warn_key_once = PTHREAD_ONCE_INIT;

static int delete_warn_specific(void);
static void warn_destructor (void *whatever);
static void warn_func_once(void);
static int create_warn_specific(void);
static struct warn_specific_data *get_warn_specific(void);

static int delete_warn_specific()
{
   void *specific;
   
   if ( (specific = pthread_getspecific(warn_tsd_key)) == NULL )
      return 0;
   free(specific);
   if ( pthread_setspecific(warn_tsd_key,NULL) < 0 )
      return -1;
   return 0;
}

static void warn_destructor (void *whatever)
{
   fprintf(stderr,"delete_warn_specific() called\n");
   delete_warn_specific();
}

static void warn_func_once()
{
   fprintf(stderr,"Doing one-time thread-specific warning data initialization.\n");
#ifdef OS_LYNX
   pthread_keycreate(&warn_tsd_key,warn_destructor);
#else
   pthread_key_create(&warn_tsd_key,warn_destructor);
#endif
}

static int create_warn_specific()
{
   void *specific;
   /* Make sure that the key has been set up */
   if ( pthread_once(&warn_key_once,warn_func_once) != 0 )
   {
      fprintf(stderr,"Thread specific one-time initialization failed.\n");
      return -1;
   }
   /* Any prior data ? This would be a memory leak. */
   if ( (specific = pthread_getspecific(warn_tsd_key)) != NULL )
   {
      fprintf(stderr,"Prior thread-specific warning being deleted.\n");
      free(specific);
      pthread_setspecific(warn_tsd_key,NULL);
   }
   
   /* New data allocated and initialized with defaults */
   if ( (specific = calloc(1,sizeof(struct warn_specific_data))) == NULL )
      return -1;
   memcpy(specific,&warn_defaults,sizeof(struct warn_specific_data));
   if ( pthread_setspecific(warn_tsd_key,specific) != 0 )
      return -1;
   return 0;
}

static struct warn_specific_data *get_warn_specific()
{
   void *specific;
   
   /* Make sure that the key has been set up */
   if ( pthread_once(&warn_key_once,warn_func_once) != 0 )
   {
      fprintf(stderr,"Thread specific one-time initialization failed.\n");
      return NULL;
   }
   if ( (specific = pthread_getspecific(warn_tsd_key)) == NULL )
   {
#if 0
      fprintf(stderr,"Dynamically creating warning thread-specific data.\n");
      fprintf(stderr,"Failure to call warn_delete_specific() at thread termination may result in memory leaks.\n");
#endif
      create_warn_specific();
      specific = pthread_getspecific(warn_tsd_key);
   }
   
   return (struct warn_specific_data *) specific;
}

#else
#define get_warn_specific() (&warn_defaults)
#endif

/* ------------------------- warn_f_warning ------------------------- */
/**
 *  @short Issue a warning to screen or other configured target.
 *
 *  Issue a warning to screen and/or file if the warning
 *  has a sufficiently large message 'level' (high enough severity).
 *  This function should best be called through the macros
 *  'Information', 'Warning', and 'Error'.
 *  The name of this function has been changed from 'warning'
 *  to '_warning' to avoid trouble if you call 'warning' instead
 *  of 'Warning'. Now such a typo causes an error in the link step.
 *
 *  @param  msgtext    Warning or error text.
 *  @param  msgorigin  Optional origin (e.g. function name) or NULL.
 *  @param  msglevel   Level of message importance:
 *      	         negative: debugging if needed,
 *      	              0-9: informative,
 *      	            10-19: warning,
 *      	            20-29: error.
 *  @param  msgno    Number of message or 0.
 *
 *  @return (none)
 *
 */

void warn_f_warning (const char *msgtext, const char *msgorigin, 
   int msglevel, int msgno)
{
   char enumtext[32];
   struct warn_specific_data *wt = get_warn_specific();

   if ( wt == NULL )
      return;
      
   if ( msgtext == (char *) NULL || wt == NULL )
      return;

   /* Don't record or print message text if not severe enough. */
   if ( msglevel < wt->warninglevel )
      return;

   /* Make sure that warning() is not executed recursively, */
   /* for example due to errors in a user-defined log function. */
   if ( wt->recursive )
   {
      fputs("E0003: Recursive call to warning function!\n",stderr);
      fputs("Text of last call: ",stderr);
      fputs(msgtext,stderr);
      fputs("\n",stderr);
      if ( (wt->warningmode & 0x02) && wt->logfile != (FILE *) NULL )
      {
         fputs("E0003: Recursive call to warning function!\n",wt->logfile);
         fputs("Text of last call: ",wt->logfile);
         fputs(msgtext,wt->logfile);
         fputs("\n",wt->logfile);
         fflush(wt->logfile);
      }
      return;        /* We could do more sophisticated things here */
   }
   wt->recursive++;

   if ( msgorigin == (char *) NULL && (wt->warningmode & 0x40) &&
        wt->aux_function != NULL )
      msgorigin = wt->aux_function();

   /* Sould the message be printed on the screen (stderr) ? */
   if ( wt->warningmode & 0x01 )
   {
      if ( msgno > 0 )
      {
         (void) sprintf(enumtext,"%s%d: ",((msglevel>=20)?"E":"W"),msgno);
         fputs(enumtext,stderr);
      }
      if ( msgorigin != (char *) NULL && ( wt->warningmode & 0x08 ) )
      {
         fputs(msgorigin,stderr);
         fputs(": ",stderr);
      }
      if ( (wt->warningmode & 0x20) && wt->aux_function != NULL )
         fprintf(stderr,"%s: ",wt->aux_function());
      fputs(msgtext,stderr);
      fputs("\n",stderr);
   }

   /* Should the warning be written to a file ? */
   if ( wt->warningmode & 0x02 )
   {
      if ( wt->logfile == (FILE *) NULL )
      {
         if ( wt->logfname == (char *) NULL )
         {
            fputs("E0004: No error log file name specified\n",stderr);
            wt->warningmode &= ~((int)0x02);
            { wt->recursive--; return; }
         }
         if ( (wt->logfile = fopen(wt->logfname,(wt->warningmode&0x10)?"a":"w")) ==
              (FILE *) NULL )
         {
            fputs("E0001: Error opening log file '",stderr);
            fputs(wt->logfname,stderr);
            fputs("'.\n",stderr);
            /* Disable writing to the logfile */
            wt->warningmode &= ~((int)0x02);
            { wt->recursive--; return; }
         }
      }

      if ( msgno > 0 )
         (void) fprintf(wt->logfile,"%s%04d: ",((msglevel>=20)?"E":"W"),msgno);
      if ( msgorigin != (char *) NULL && ( wt->warningmode & 8 ) )
      {
         (void) fputs(msgorigin,wt->logfile);
         (void) fputs(": ",wt->logfile);
      }
      if ( (wt->warningmode & 0x20) && wt->aux_function != NULL )
         fprintf(wt->logfile,"%s: ",wt->aux_function());
      (void) fputs(msgtext,wt->logfile);
      (void) fputs("\n",wt->logfile);
      fflush(wt->logfile);

      if ( ferror(wt->logfile) )
      {
         clearerr(wt->logfile);
         fputs("E0002: Error writing to log file '",stderr);
         fputs(wt->logfname,stderr);
         fputs("'.\n",stderr);
         /* Disable writing to the logfile if the disk is full. */
         if ( errno == ENOSPC )
         {
            fputs("E0005: File system is full. Disabling logging now.\n",
               stderr);
            wt->warningmode &= ~((int)0x02);
         }
      }
   }

   if ( wt->log_function != NULL && (wt->warningmode & 0x04) )
      (*wt->log_function)(msgtext,msgorigin,msglevel,msgno);

   wt->recursive--;
   return;
}

/* ------------------------ set_warning ------------------------- */
/**
 *  @short Set a specific warning level and mode.
 *
 *  @param  level   Warnings with level below this are ignored.
 *  @param  mode    To screen, to file, with user function ...
 *
 *  @return 0 if ok, -1 if level and/or mode could not be set.
 *
 */

int set_warning (int level, int mode)
{
   struct warn_specific_data *wt = get_warn_specific();
   
   if ( wt == NULL )
      return -1;

   if ( level != -1 ) /* -1 means: keep the old value */
      wt->warninglevel = level;
   if ( mode != -1 )
      wt->warningmode = mode;
   return 0; /* So far, always ok */
}

int set_default_warning (int level, int mode)
{
   struct warn_specific_data *wt = &warn_defaults;
   
   if ( wt == NULL )
      return -1;

   if ( level != -1 ) /* -1 means: keep the old value */
      wt->warninglevel = level;
   if ( mode != -1 )
      wt->warningmode = mode;
   return 0; /* So far, always ok */
}

/* --------------------- warning_status ---------------------- */
/**
 *  Inquire status of warning settings.
 *
 *  @param  plevel   Pointer to variable for storing current level.
 *  @param  pmode    Pointer to store the current warning mode.
 *
 *  @return (none)
 *
 */

void warning_status (int *plevel, int *pmode)
{
   struct warn_specific_data *wt = get_warn_specific();
   if ( wt == NULL )
      return;
   if ( plevel )
      *plevel = wt->warninglevel;
   if ( pmode )
      *pmode = wt->warningmode;
}

/* --------------- set_logging_function ------------- */
/**
 *  @short Set user-defined function for logging warnings and errors.
 *
 *  Set a user-defined function as the function to be
 *  used for logging warnings and errors.
 *  To enable usage of this function, bit 2 of the
 *  warning mode must be set and other bits reset, if
 *  logging to screen and/or disk file is no longer wanted.
 *
 *  Parameter  userfunc:    Pointer to a function taking two strings
 *			(the message text and the origin text,
 *			which may be NULL) and two integers
 *			(message level and message number).
 *
 *  @return  (none)
 *
*/

void set_logging_function ( void (*user_function) (
   const char *, const char *, int, int) )
{
   struct warn_specific_data *wt = get_warn_specific();
   if ( wt == NULL )
   {
      Warning("Cannot set logging function");
      return;
   }
   wt->log_function = user_function;
}

void set_default_logging_function ( void (*user_function) (
   const char *, const char *, int, int) )
{
   struct warn_specific_data *wt = &warn_defaults;
   if ( wt == NULL )
   {
      Warning("Cannot set logging function");
      return;
   }
   wt->log_function = user_function;
}

/* --------------------- set_log_file -------------------- */
/**
 *  @short Set a new log file name and save it in local storage.
 *
 *  If there was a log file with a different name opened
 *  previously, close it.
 *
 *  @param  fname   New name of log file for warnings
 *
 *  @return  0 (o.k.),  -1 (error)
 *
 */

int set_log_file (const char *fname)
{
   struct warn_specific_data *wt = get_warn_specific();
   if ( wt == NULL )
      return -1;
   /* No log file? */
   if ( fname == (char *) NULL )
   {
      if ( wt->logfile != (FILE *) NULL )
      {
         wt->logfname = (char *) NULL;
         fclose(wt->logfile);
         wt->logfile = (FILE *) NULL;
      }
      return 0;
   }

   /* Is length of name o.k.? */
   if ( strlen(fname) > sizeof(wt->saved_logfname)-1 )
   {
      Warning("Log file name is too long.");
      return -1;
   }

   /* Is this the old log file name again? */
   if ( wt->logfname != (char *) NULL )
      if ( strcmp(fname,wt->logfname) == 0 )
         return 0;

   /* If there was a different log file before, close it. */
   if ( wt->logfile != (FILE *) NULL )
   {
      fclose(wt->logfile);
      wt->logfile = (FILE *) NULL;
   }

   strcpy(wt->saved_logfname,fname);
   wt->logfname = wt->saved_logfname;

   return 0;
}

/* ---------------------- output_text -------------------- */
/**
 *  Print a text string (without appending a newline etc.)
 *  on the screen or send it to a controlling process, depending
 *  on the setting of the output function.
 *
 *  @param  text  A text string to be displayed.
 *
 *  @return  (none)
 *
 */

void warn_f_output_text (const char *text)
{
   size_t len;
   struct warn_specific_data *wt = get_warn_specific();

   if ( wt == NULL )
      return ;

   if ( *text == '\0' )
      return;

   if ( wt->output_function == NULL )
      fputs(text,stdout);
   else
   {
      len = strlen(text);
      if ( wt->buffered + len + 1 >= sizeof(wt->output_buffer) )
      {
         if ( wt->buffered )
            (*wt->output_function)(wt->output_buffer);
         wt->output_buffer[0] = '\0';
         wt->buffered = 0;
      }
      if ( len + 1 >= sizeof(wt->output_buffer) )
      {
         (*wt->output_function)(text);
      }
      else if ( len > 0 )
      {
         memcpy((wt->output_buffer+wt->buffered),text,(size_t)len);
         wt->buffered += len;
         wt->output_buffer[wt->buffered] = '\0';
      }
   }
}

/* ------------------ flush_output ----------------- */
/**
 *  @short Flush buffered output.
 *  Output is flushed, no matter if it is standard output
 *  or a special output function;
 *
 *  @return  (none)
 *
 */

void flush_output ()
{
   struct warn_specific_data *wt = get_warn_specific();

   if ( wt == NULL )
      return ;

   if ( wt->output_function == NULL )
      fflush(stdout);
   else if ( wt->buffered )
      (*wt->output_function)(wt->output_buffer);
   wt->buffered = 0;
}

/* --------------- set_output_function ------------- */
/**
 *  Set a user-defined function as the function to be used
 *  for normal text output. Such a function may be used to
 *  send output back to a remote control process via network.
 *
 *  Parameter userfunc:  Pointer to a function taking a string
 *			    (the text to be displayed) as argument.
 *
 *  @return (none)
 *
*/

void set_output_function ( void (*user_function) (const char *) )
{
   struct warn_specific_data *wt = get_warn_specific();

   if ( wt == NULL )
      return ;

   if ( wt->buffered )
      flush_output();
   wt->output_function = user_function;
}

void set_default_output_function ( void (*user_function) (const char *) )
{
   struct warn_specific_data *wt = &warn_defaults;

   if ( wt == NULL )
      return ;

   if ( wt->buffered )
      flush_output();
   wt->output_function = user_function;
}

/* --------------------- set_aux_warning_function --------------- */
/**
 *  @short Set an auxilliary function for warnings. 
 *  This function may be
 *  used to insert time and date or origin etc. at the beginning
 *  of the warning text.
 *
 *  @param  auxfunc -- Pointer to a function taking no argument
 *			   and returning a character string.
 *
 *  @return  (none)
 *
 */

void set_aux_warning_function ( char *(*auxfunc) (void) )
{
   struct warn_specific_data *wt = get_warn_specific();

   if ( wt == NULL )
      return ;

   wt->aux_function = auxfunc;
}

void set_default_aux_warning_function ( char *(*auxfunc) (void) )
{
   struct warn_specific_data *wt = &warn_defaults;

   if ( wt == NULL )
      return ;

   wt->aux_function = auxfunc;
}
