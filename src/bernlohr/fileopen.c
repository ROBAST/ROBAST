/* ============================================================================

   Copyright (C) 2000, 2008, 2009, 2010  Konrad Bernloehr

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

/** @file fileopen.c 
 *  @short Allow searching of files in declared include paths (fopen replacement).
 *
 *  The functions provided in this file provide an enhanced replacement
 *  @c fileopen() for the C standard library's @c fopen() function.
 *  The enhancements are in several areas:
 *
 *  @li Where possible files are opened such that more than 2 gigabytes
 *      of data can be accessed on 32-bit systems when suitably compiled.
 *      This also works with software where a @c '-D_FILE_OFFSET_BITS=64' at
 *      compile-time cannot be used (of which ROOT is an infamous example).
 *  @li For reading files, a list of paths can be configured before the
 *      the first fileopen() call and all files without absolute paths
 *      will be searched in these paths. Writing always strictly
 *      follows the given file name and will not search in the path list.
 *  @li Files compressed with @c gzip or @c bzip2 can be handled on the
 *      fly. Files with corresponding file name extensions 
 *      ( @c .gz and @c .bz2 ) will be
 *      automatically decompressed when reading or compressed when
 *      writing (in a pipe, i.e. without producing temporary copies).
 *  @li In the same way, files compressed with @c lzop (for extension
 *      @c .lzo ) and @c lzma (for extension  @c .lzma ) are handled
 *      on the fly. No check is made if these programs are installed.
 *  @li URIs (uniform resource identifiers) starting with @c http:,
 *      @c https:, or @c ftp: will also be opened in a pipe, with optional
 *      decompression, depending on the ending of the URI name.
 *      You can therefore easily process files located on a web or
 *      ftp server. Access is limited to reading.
 *  @li Files on any SSH server where you can login without a password
 *      can be read as @c 'ssh://user@host:filepath' where filepath can be
 *      an absolute path (starting with @c '/') or one relative to the
 *      users home directory.
 *
 *  @author  Konrad Bernloehr 
 *  @date    Nov. 2000
 *  @date    @verbatim CVS $Date: 2011/05/31 15:36:51 $ @endverbatim 
 *  @version @verbatim CVS $Revision: 1.11 $ @endverbatim 
 */

#include "initial.h"
#include "straux.h"
#include "fileopen.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

/** An element in a linked list of include paths. */
struct incpath
{
   char *path;       	   /**< The path name */
   struct incpath *next;   /**< The next element */
};

/** The starting element of include paths. */
static struct incpath *root_path = NULL;

static void freepath(void);

/** Init the path list, with default_path as the only entry. */

void initpath (const char *default_path)
{
   int ipos = 0;
   char pathname[1024];

   if ( default_path == NULL )
      default_path = ".";

   if ( root_path != NULL )
      freepath();
      
   while ( getword(default_path,&ipos,pathname,sizeof(pathname)-1,':','\n') > 0 )
      addpath(pathname);
}

/** Free a whole list of include path elements. */

static void freepath()
{
   struct incpath *path, *next = NULL;
   for ( path = root_path; path != NULL; path = next )
   {
      next = path->next;
      if ( path->path != NULL )
      	 free(path->path);
      free(path);
   }
   root_path = NULL;
}

/** Show the list of include paths. */

void listpath (char *buffer, size_t bufsize)
{
   struct incpath *path;
   if ( buffer == NULL || bufsize <= 0 )
      return;
   for ( path = root_path; path != NULL; path = path->next )
   {
      if ( path->path == NULL )
      	 continue;
      if ( bufsize > strlen(path->path)+3 )
      {
      	 strcpy(buffer," -I");
	 buffer += 3;
         bufsize -= 3;
	 strcpy(buffer,path->path);
	 buffer += strlen(path->path);
	 bufsize -= strlen(path->path);
      }
   }
}

/** 
 *  @short Add a path to the list of include paths, if not already there.
 *  The path name is always copied to a newly allocated memory location.
 *  This path name can actually be a colon-separated list, as for initpath().
*/

void addpath (const char *name)
{
   struct incpath *path, *last = NULL;
   char pathname[1024];
   int ipos = 0;
   if ( name == NULL )
      return;
   if ( *name == '\0' )
      return;
   for ( path = root_path; path != NULL; path = path->next )
   {
      last = path;
      if ( strcmp(name,path->path) == 0 )
      	 return;
   }
   while ( getword(name,&ipos,pathname,sizeof(pathname)-1,':','\n') > 0 )
   {
      if ( last == NULL )
         path = root_path = calloc(1,sizeof(struct incpath));
      else
         path = last->next = calloc(1,sizeof(struct incpath));
      if ( path != NULL )
         path->path = strdup(pathname);
      if ( last == NULL )
         last = root_path;
      else
         last = last->next;
   }
}

/** Helper function for opening a compressed file through a fifo. */

static FILE *cmp_popen (const char *fname, const char *mode, int compression)
{
   char *cmd = NULL; /* Fifo command without input/output file name */
   char *s;          /* Full fifo command with redirection */
   FILE *f = NULL;   /* File pointer returned */
   char *pmd = "?";  /* Fifo mode ("w" or "r") */

   int rc = 0;

   if ( fname == NULL || mode == NULL )
      return NULL;

   errno = 0;
   switch ( *mode )
   {
      case 'w':
         /* If we have no write access, we do not try to open the */
         /* fifo since it would result in a broken pipe later. */
         if ( (rc=access(fname,W_OK)) != 0 )
            if ( errno != ENOENT )
            {
               int k = errno;
               fprintf(stderr,"Cannot write to ");
               perror(fname);
               errno = k;
               return NULL;
            }
         pmd = "w";
         switch ( compression )
         {
            case 1:
               cmd = "exec gzip -c >";
               break;
            case 2:
               cmd = "exec bzip2 -c >";
               break;
            case 3:
               cmd = "exec lzop -c >";
               break;
            case 4:
               cmd = "exec lzma -c >";
               break;
         }
         break;
      case 'a':
         /* If we have no write access, we do not try to open the */
         /* fifo since it would result in a broken pipe later. */
         if ( (rc=access(fname,W_OK)) != 0 )
            if ( errno != ENOENT )
            {
               int k = errno;
               fprintf(stderr,"Cannot write to ");
               perror(fname);
               errno = k;
               return NULL;
            }
         pmd = "w";
         switch ( compression )
         {
            case 1:
               cmd = "exec gzip -c >>";
               break;
            case 2:
               cmd = "exec bzip2 -c >>";
               break;
            case 3:
               cmd = "exec lzop -c >>";
               break;
            case 4:
               cmd = "exec lzma -c >>";
               break;
         }
         break;
      case 'r':
         /* If we have no read access, we do not try to open the */
         /* fifo since it would result in a broken pipe later. */
         if ( (rc=access(fname,R_OK)) != 0 )
            return NULL;
         pmd = "r";
         switch ( compression )
         {
            case 1:
               cmd = "exec gzip -d -c <";
               break;
            case 2:
               cmd = "exec bzip2 -d -c <";
               break;
            case 3:
               cmd = "exec lzop -d -c <";
               break;
            case 4:
               cmd = "exec lzma -d -c <";
               break;
         }
         break;
   }

   if ( cmd == NULL )
   {
      errno = EPERM;
      return NULL;
   }

   s = malloc(strlen(cmd)+strlen(fname)+3);
   if ( s == NULL )
      return NULL;
   strcpy(s,cmd);
   strcat(s,"'");
   strcat(s,fname);
   strcat(s,"'");

   f = popen(s,pmd);
   free(s);

   return f;
}

/** Helper function for opening a file with a URI (http:// etc.). */

static FILE *uri_popen (const char *fname, const char *mode, int compression);

static FILE *uri_popen (const char *fname, const char *mode, int compression)
{
   char *get_cmd = "curl -s -S --netrc-optional "; /* Fifo command without input file name */
   char *cmp_cmd = "";  /* Decompression of data */
   char *s;             /* Full fifo command with redirection */
   FILE *f = NULL;      /* File pointer returned */

   if ( fname == NULL || mode == NULL )
      return NULL;

   if ( *mode != 'r' ) /* Only read-mode is supported. */
   {
      fprintf(stderr,"Cannot write to %s.\n", fname);
      errno = EPERM;
      return NULL;
   }
   errno = 0;
   
   switch ( compression )
   {
      case 1:
         cmp_cmd = " | gzip -d";
         break;
      case 2:
         cmp_cmd = " | bzip2 -d";
         break;
      case 3:
         cmp_cmd = " | lzop -d";
         break;
      case 4:
         cmp_cmd = " | lzma -d";
         break;
   }

   s = malloc(strlen(get_cmd)+strlen(fname)+strlen(cmp_cmd)+3);
   if ( s == NULL )
      return NULL;
   strcpy(s,get_cmd);
   strcat(s,"'");
   strcat(s,fname);
   strcat(s,"'");
   strcat(s,cmp_cmd);

   f = popen(s,"r");
   free(s);

   return f;
}

/** Helper function for opening a file on a remote SSH server */

static FILE *ssh_popen (const char *fname, const char *mode, int compression);

static FILE *ssh_popen (const char *fname, const char *mode, int compression)
{
   char *get_cmd = "ssh '%s' \"cat '%s'\"%s"; /* Fifo command to be filled in. */
   char *cmp_cmd = "";  /* Decompression of data */
   char *s;
   const char *c;
   FILE *f = NULL;      /* File pointer returned */
   size_t i, n;
   char remote_loc[256]; /* user@host format, intentionally no password support */
   char *remote_fn = NULL; /* relative or absolute path on remote host */

   errno = EPROTO;

   if ( fname == NULL || mode == NULL )
      return NULL;

   if ( strncmp(fname,"ssh://",6) != 0 )
      return NULL;

   /* Check for some shell-relevant ASCII characters, even though the filename is in quotes. */
   for ( i=0, n=strlen(fname); i<n; i++ )
      if ( fname[i] < 32 || fname[i] == 127 || 
           fname[i] == ';' ||
           fname[i] == '$' ||
           fname[i] == '\\' ||
           fname[i] == '>' || /* No worries about '<' */
           fname[i] == '&' ||
           fname[i] == '!' ||
           fname[i] == '|' ||
           fname[i] == 96 /* back-quote */)
      {
         fprintf(stderr,"Invalid character in '%s'.\n", fname);
         errno = EPERM;
         return NULL;
      }

   s = strchr(fname+7,':');
   if ( s == NULL )
      return NULL;
   if ( (size_t)(s - fname - 7) >= sizeof(remote_loc) )
      return NULL;
   for (n=0, c=fname+6; c!=s; c++, n++)
      remote_loc[n] = *c;
   remote_loc[n] = '\0';
   remote_fn = s+1;

   if ( *mode != 'r' ) /* Only read-mode is supported. */
   {
      fprintf(stderr,"Cannot write to %s.\n", fname);
      errno = EPERM;
      return NULL;
   }
   errno = 0;
   
   switch ( compression )
   {
      case 1:
         cmp_cmd = " | gzip -d";
         break;
      case 2:
         cmp_cmd = " | bzip2 -d";
         break;
      case 3:
         cmp_cmd = " | lzop -d";
         break;
      case 4:
         cmp_cmd = " | lzma -d";
         break;
   }

   n = strlen(get_cmd)+strlen(fname)+strlen(cmp_cmd)+1;
   s = malloc(n);
   if ( s == NULL )
      return NULL;
   snprintf(s,n,get_cmd,remote_loc,remote_fn,cmp_cmd);

   f = popen(s,"r");
   free(s);

   return f;
}

/** Search for a file in the include path list and open it if possible. */

FILE *fileopen (const char *fname, const char *mode)
{
   struct incpath *path = root_path;
   char try_fname[1024];
   FILE *f = NULL;
   int nerr = 0;
   int compression = 0;
   int l;
   
   /* The set of search paths might not be initialized yet */
   if ( path == NULL )
   {
      initpath(NULL);
      if ( (path = root_path) == NULL )
      	 return NULL;
   }
   /* Arguments could be incomplete */
   if ( fname == NULL || mode == NULL )
      return NULL;

   /* Check if compressed files are meant. */
   l = strlen(fname);
   if ( l > 3 && strcmp(fname+l-3,".gz") == 0 )
      compression = 1;
   else if ( l > 4 && strcmp(fname+l-4,".bz2") == 0 )
      compression = 2;
   else if ( l > 4 && strcmp(fname+l-4,".lzo") == 0 )
      compression = 3;
   else if ( l > 5 && strcmp(fname+l-5,".lzma") == 0 )
      compression = 4;

   if ( strchr(fname,':') != NULL )
   {
      if ( strncmp(fname,"http://",7) == 0 ||
           strncmp(fname,"https://",8) == 0 ||
           strncmp(fname,"ftp://",6) == 0 ||
           strncmp(fname,"file://",7) == 0 )
         return uri_popen(fname,mode,compression);
      if ( strncmp(fname,"ssh://",6) == 0 )
         return ssh_popen(fname,mode,compression);
   }

   /* For modes other than read-only, no search is done. */
   if ( *mode != 'r' )
   {
      switch (compression)
      {
         case 0: /* Normal file */
#ifdef __USE_LARGEFILE64
            /* With explicit support of large files on 32-bit machines. */
            return fopen64(fname,mode);
#else
            /* Support for large files is either implicit or not available. */
            return fopen(fname,mode);
#endif
            break;

         case 1: /* Create FIFO to gzip to file */
         case 2: /* Create FIFO to bzip2 to file */
         case 3: /* Create FIFO to lzop to file */
         case 4: /* Create FIFO to lzma to file */
            return cmp_popen(fname,mode,compression);

         default:
            return NULL;
      }
   }            

   /* If the name includes (part of) a path, no search is done. */
   if ( strchr(fname,'/') != NULL )
   {
      switch (compression)
      {
         case 0: /* Normal file */
#ifdef __USE_LARGEFILE64
            return fopen64(fname,mode);
#else
            return fopen(fname,mode);
#endif
            break;

         case 1: /* Create FIFO from gunzip from file */
         case 2: /* Create FIFO from bunzip2 from file */
         case 3: /* Create FIFO from lzop from file */
         case 4: /* Create FIFO from lzma from file */
            return cmp_popen(fname,mode,compression);
            break;

         default:
            return NULL;
      }
   }            

   /* Now try all the paths we have */
   for ( ; path != NULL; path=path->next )
   {
#ifdef __USE_LARGEFILE64
      struct stat64 st;
#else
      struct stat st;
#endif
      if ( path->path == NULL )
      	 continue;
      if ( strlen(path->path)+strlen(fname)+1 >= sizeof(try_fname) )
      	 continue;
      if ( strcmp(path->path,".") == 0 )
      	 strcpy(try_fname,fname); /* No need to expand local path */
      else
      	 sprintf(try_fname,"%s/%s",path->path,fname);

      errno = 0;
      /* Check if the file is available */
#ifdef __USE_LARGEFILE64
      if ( stat64(try_fname,&st) != 0 )
#else
      if ( stat(try_fname,&st) != 0 )
#endif
      {
         if ( errno != 0 && errno != ENOENT )
         {
      	    perror(try_fname);
	    nerr++;
         }
         continue;
      }

      errno = 0;
      /* The file exists and is hopefully readable */
      switch (compression)
      {
         case 0: /* Normal file */
#ifdef __USE_LARGEFILE64
            if ( (f = fopen64(try_fname,mode)) != NULL )
#else
            if ( (f = fopen(try_fname,mode)) != NULL )
#endif
      	       return f;
            break;

         case 1: /* Create FIFO from gunzip from file */
         case 2: /* Create FIFO from bunzip2 from file */
         case 3: /* Create FIFO from lzop from file */
         case 4: /* Create FIFO from lzma from file */
            if ( (f = cmp_popen(try_fname,"r",compression)) != NULL )
               return f;
            break;

         default:
            return NULL;
      }

      if ( errno != 0 && errno != ENOENT )
      {
      	 perror(try_fname);
	 nerr++;
      }
   }

   if ( f != NULL )
      return f;

   if ( f == NULL && nerr == 0 )
   {
      fprintf(stderr,"%s: Not found in any include directory.\n",fname);
      if ( errno == 0 )
      	 errno = ENOENT;
   }

   return NULL;
}

/** Close a file or fifo but not if it is one of the standard streams. */

int fileclose (FILE *f)
{
#ifdef __USE_LARGEFILE64
   struct stat64 st;
#else
   struct stat st;
#endif
   int rc = 0;

   if ( f == NULL )
      return -1;

   errno = 0;

   /* We never close the standard streams here. */
   if ( f == stdin || f == stdout || f == stderr )
      return 0;

      /* Check what kind of stream we have */
   if ( fileno(f) == -1 )
   {
      fprintf(stderr,"Trying to close stream: no file handle\n");
      errno = EBADF;
      return -1;
   }

#ifdef __USE_LARGEFILE64
   if ( fstat64(fileno(f),&st) != 0 )
#else
   if ( fstat(fileno(f),&st) != 0 )
#endif
   {
      if ( errno != 0 )
         perror("Trying to close stream");
      errno = EBADF;
      return -1;
   }

#ifdef S_ISFIFO
   if ( S_ISFIFO(st.st_mode) )
#else
   if ( (st.st_mode & S_IFMT) == S_IFIFO )
#endif
      rc = pclose(f);
   else
      rc = fclose(f);

   if ( rc != 0 )
   {
      if ( errno == 0 )
         errno = EBADMSG;
   }
   return rc;
}
