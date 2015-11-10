/* ============================================================================

   Copyright (C) 1997, 2000, 2001, 2009, 2010  Konrad Bernloehr

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

/** @file io_simtel.c
 *  @short Write and read CORSIKA blocks and simulated Cherenkov photon bunches.
 *
 *  This file provides functions for writing and reading of CORSIKA
 *  header and trailer blocks, positions of telescopes/detectors,
 *  lists of simulated Cherenkov photon bunches before any detector
 *  simulation for the telescopes as well as of photoelectrons after
 *  absorption, telescope ray-tracing and quantum efficiency applied.
 *
 *  @author  Konrad Bernloehr
 *  @date    1997 to 2010
 *  @date    @verbatim CVS $Date: 2015/04/27 10:10:29 $ @endverbatim
 *  @version @verbatim CVS $Revision: 1.26 $ @endverbatim
 */

/* ================================================================ */

#include "initial.h"      /* This file includes others as required. */
#include "io_basic.h"     /* This file includes others as required. */
#include "mc_tel.h"
#include "fileopen.h"

/* -------------------------- write_tel_block --------------------- */
/**
 *  Write a CORSIKA block as given type number (see mc_tel.h).
 *
 *  @param  iobuf I/O buffer descriptor
 *  @param  type  block type (see mc_tel.h)
 *  @param  num   Run or event number depending on type
 *  @param  data  Data as passed from CORSIKA
 *  @param  len   Number of elements to be written
 *  @return 0 (OK), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_block (IO_BUFFER *iobuf, int type, int num, 
    bernlohr_real *data, int len)
{
   IO_ITEM_HEADER item_header;
   int i;
   
   if ( iobuf == (IO_BUFFER *) NULL || data == NULL )
      return -1;
   
   item_header.type = type;             /* Data type */
   item_header.version = 0;             /* Version 0 (test) */
   item_header.ident = num;
   put_item_begin(iobuf,&item_header);

   put_long(len,iobuf);
   put_long((long) *((INT32 *)data),iobuf);
   for (i=1; i<len; i++)
      put_real((double)data[i],iobuf);
   
   return put_item_end(iobuf,&item_header);
}

/* ------------------------ read_tel_block ------------------------ */
/**
 *  Read a CORSIKA header/trailer block of given type (see mc_tel.h)
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @param  type      block type (see mc_tel.h)
 *  @param  data      area for data to be read
 *  @param  maxlen    maximum number of elements to be read
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_tel_block (IO_BUFFER *iobuf, int type, bernlohr_real *data, int maxlen)
{
   IO_ITEM_HEADER item_header;
   int i, len, rc;
   
   if ( iobuf == (IO_BUFFER *) NULL || data == NULL )
      return -1;
   
   item_header.type = type;             /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   len = get_long(iobuf);
   if ( len > maxlen )
   {
      fprintf(stderr,"Too much data in item type %d.\n",type);
      return -1;
   }
   *((INT32 *)data) = get_long(iobuf);
   for (i=1; i<len; i++)
      data[i] = get_real(iobuf);
   for ( ; i<maxlen; i++ )
      data[i] = 0.;
   
   return get_item_end(iobuf,&item_header);
}

/* ------------------------ print_tel_block ------------------------ */
/**
 *  Print a CORSIKA header/trailer block of any type (see mc_tel.h)
 *
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int print_tel_block (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int i, len, rc;
   char txt[5];
   float data[273];
   int idata;
   static int first_event_in_run = 0;
   
   if ( iobuf == (IO_BUFFER *) NULL || data == NULL )
      return -1;
   
   item_header.type = 0;             /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.type != IO_TYPE_MC_RUNH && 
        item_header.type != IO_TYPE_MC_RUNE &&
        item_header.type != IO_TYPE_MC_EVTH &&
        item_header.type != IO_TYPE_MC_EVTE )
   if ( item_header.version != 0 )
      return -1;

   len = get_long(iobuf);

   idata = get_long(iobuf);
   memcpy(txt,&idata,4);
   txt[4] = '\0';
   for (i=1; i<len && (size_t)i<sizeof(data)/sizeof(data[0]); i++)
      data[i] = get_real(iobuf);

   if ( strncmp(txt,"RUNH",4) == 0 )
   {
      int idate = (int)(data[2]+0.1);
      int y = idate/10000;
      int m = (idate%10000)/100;
      int d = idate%100;
      if ( y <= 39 )
         y += 2000;
      else if (y < 100)
         y += 1900;
      printf("\nCorsika run header\n");
      first_event_in_run = 1;
      printf("   Run number %d started on %d-%d-%d with version %5.3f.\n",
         (int)(data[1]+0.1), y,m,d, data[3]);
      printf("   Energy range %g to %g GeV with slope of %f.\n",
         data[16], data[17], data[15]);
   }
   else if ( strncmp(txt,"RUNE",4) == 0 )
      printf("\nCorsika run end\n");
   else if ( strncmp(txt,"EVTH",4) == 0 )
   {
      double E = data[3];
      int ptype = Nint(data[2]);
      printf("\nCorsika event header: primary of type %d and energy %g GeV at %5.2f km.\n",
         ptype, E, fabs(data[6]*1e-5));
      if ( first_event_in_run )
      {
         int iv;
         first_event_in_run = 0;
         printf("   Low energy model %d, high energy model %d.\n",
            (int)(data[74]+0.1), (int)(data[75]+0.1));
         iv = (int)(data[76]+0.1);
         printf("   Cherenkov flag: %d (", iv);
         if ( (iv&0x01) )
            printf("CERENKOV");
         if ( (iv&0x02) )
            printf(",IACT");
         if ( (iv&0x04) )
            printf(",CEFFIC");
         if ( (iv&0x08) )
            printf(",ATMEXT");
         if ( (iv&0x10) )
            printf(" with refraction");
         if ( (iv&0x20) )
            printf(",VOLUMEDET");
         if ( (iv&0x80) )
            printf(" (adapted)");
         if ( (iv&0x40) )
            printf(",CURVED");
         if ( (iv&0x100) )
            printf(",SLANT");
         if ( (iv&0x200) )
            printf(",???");
         printf(", profile %d).\n", (iv>>10)&0x3ff);
         if ( data[6] > 0. )
            printf("   TSTART is off.\n");
         printf("   Cherenkov bunch size %4.2f from %1.0f to %1.0f nm.\n",
           data[84], data[95], data[96]);
         printf("   Interaction: SIBYLL %d/%d, QGSJET %d/%d, DPMJET %d/%d, V/N/E %d\n",
            (int)(data[138]+0.1), (int)(data[139]+0.1),
            (int)(data[140]+0.1), (int)(data[141]+0.1),
            (int)(data[142]+0.1), (int)(data[143]+0.1),
            (int)(data[144]+0.1));
#if 0
         printf("   SIBYLL interaction flag = %d, cross section = %d\n",
            (int)(data[138]+0.1), (int)(data[139]+0.1));
         printf("   QGSJET interaction flag = %d, cross section = %d\n",
            (int)(data[140]+0.1), (int)(data[141]+0.1));
         printf("   DPMJET interaction flag = %d, cross section = %d\n",
            (int)(data[142]+0.1), (int)(data[143]+0.1));
         printf("   VENUS/NeXus/EPOS = %d\n", (int)(data[144]+0.1));
#endif
         printf("   Muon multiple scattering = %d\n",(int)(data[145]+0.1));
      }
   }
   else if ( strncmp(txt,"EVTE",4) == 0 )
      printf("\nCorsika event end\n");
   else
      printf("\nUnknown CORSIKA block of type %s.\n", txt);
   
   return get_item_end(iobuf,&item_header);
}

/* --------------------- write_input_lines --------------------- */
/**
 *  Write a linked list of character strings (normally containing
 *  the text of the CORSIKA inputs file) as a dedicated block.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  list    starting point of linked list
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_input_lines (IO_BUFFER *iobuf, struct linked_string *list)
{
   IO_ITEM_HEADER item_header;
   int n;
   struct linked_string *xl;
   
   if ( list == NULL )
      return -1;
   
   for ( n=0, xl=list; xl != NULL; xl=xl->next )
   {
      if ( xl->text == NULL )
         break;
      n++;
   }

   if ( n<=0 )
      return 0;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_INPUTCFG;   /* Data type */
   item_header.version = 0;                /* Version 0 (test) */
   item_header.ident = 0;
   put_item_begin(iobuf,&item_header);
   
   put_long(n,iobuf);
   for ( n=0, xl=list; xl != NULL; xl=xl->next )
   {
      if ( xl->text == NULL )
         break;
      put_string(xl->text,iobuf);
   }

   return put_item_end(iobuf,&item_header);
}

/* --------------------- read_input_lines --------------------- */
/**
 *  Read a block with several character strings (normally containing
 *  the text of the CORSIKA inputs file) into a linked list.
 *
 *  @param  iobuf  I/O buffer descriptor
 *  @param  list   starting point of linked list
 *		   (on first call this should be a link
 *		   to an empty list, i.e. the first
 *		   element has text=NULL and next=NULL;
 *		   on additional calls the new lines will be
 *		   appended.)
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_input_lines (IO_BUFFER *iobuf, struct linked_string *list)
{
   IO_ITEM_HEADER item_header;
   int i, n, l, rc;
   struct linked_string *xl, *xln;
   char line[512];

   if ( iobuf == (IO_BUFFER *) NULL || list == NULL )
      return -1;

   item_header.type = IO_TYPE_MC_INPUTCFG;     /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   n = get_long(iobuf);

   /* Skip to end of existing list */
   for (xl=list; xl->text != NULL && xl->next != NULL; xl=xl->next)
      ;

   for (i=0; i<n; i++)
   {
      l = get_string(line,sizeof(line)-1,iobuf);
      if ( l <= 0 ) 
      	 continue;
      if ( xl->text == NULL )
      {
      	 if ( (xl->text = (char *) malloc(l+1)) == NULL )
	    break;
      }
      else
      {
      	 if ( (xln = (struct linked_string *) calloc(1,sizeof(struct linked_string))) == NULL )
	    break;
	 if ( (xln->text = (char *) malloc(l+1)) == NULL )
	    break;
	 xl->next = xln;
	 xl = xln;
      }
      strncpy(xl->text,line,l);
      xl->text[l] = '\0';
   }

   return get_item_end(iobuf,&item_header);
}

/* ------------------------ write_tel_pos ---------------------- */
/**
 *  Write positions of telescopes/detectors within a system or array.
 *
 *  @param  iobuf  I/O buffer descriptor
 *  @param  ntel   number of telescopes/detectors
 *  @param  x	   X positions
 *  @param  y	   Y positions
 *  @param  z	   Z positions
 *  @param  r	   radius of spheres including the whole devices
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_pos (IO_BUFFER *iobuf, int ntel, double *x, double *y,
     double *z, double *r)
{
   IO_ITEM_HEADER item_header;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_TELPOS;   /* Data type */
   item_header.version = 0;                /* Version 0 (test) */
   item_header.ident = 0;
   put_item_begin(iobuf,&item_header);

   put_long((long)ntel,iobuf);
   put_vector_of_real(x,ntel,iobuf);
   put_vector_of_real(y,ntel,iobuf);
   put_vector_of_real(z,ntel,iobuf);
   put_vector_of_real(r,ntel,iobuf);
   
   return put_item_end(iobuf,&item_header);
}

/* ------------------------ read_tel_pos ------------------------ */
/**
 *  Read positions of telescopes/detectors within a system or array.
 *
 *  @param  iobuf    I/O buffer descriptor
 *  @param  max_tel  maximum number of telescopes allowed
 *  @param  ntel     number of telescopes/detectors
 *  @param  x	     X positions
 *  @param  y	     Y positions
 *  @param  z	     Z positions
 *  @param  r	     radius of spheres including the whole devices
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
 *
*/

int read_tel_pos (IO_BUFFER *iobuf, int max_tel, int *ntel, double *x, double *y,
     double *z, double *r)
{
   IO_ITEM_HEADER item_header;
   int rc;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;

   item_header.type = IO_TYPE_MC_TELPOS;    /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   *ntel = get_long(iobuf);
   if ( *ntel > max_tel )
   {
      int i;
      fprintf(stderr,"Too many telescopes in item type %d\n",IO_TYPE_MC_TELPOS);
      fprintf(stderr,"Reading only the first %d of %d telescopes.\n",max_tel,*ntel);
      get_vector_of_real(x,max_tel,iobuf);
      for (i=max_tel; i<(*ntel); i++)
         (void) get_real(iobuf);
      get_vector_of_real(y,max_tel,iobuf);
      for (i=max_tel; i<(*ntel); i++)
         (void) get_real(iobuf);
      get_vector_of_real(z,max_tel,iobuf);
      for (i=max_tel; i<(*ntel); i++)
         (void) get_real(iobuf);
      get_vector_of_real(r,max_tel,iobuf);
      for (i=max_tel; i<(*ntel); i++)
         (void) get_real(iobuf);
      get_item_end(iobuf,&item_header);
      *ntel = max_tel;
      return -1;
   }
   get_vector_of_real(x,*ntel,iobuf);
   get_vector_of_real(y,*ntel,iobuf);
   get_vector_of_real(z,*ntel,iobuf);
   get_vector_of_real(r,*ntel,iobuf);

   return get_item_end(iobuf,&item_header);
}


/* ------------------------ print_tel_pos ------------------------ */
/**
 *  Print positions of telescopes/detectors within a system or array.
 *
 *  @param  iobuf    I/O buffer descriptor
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
 *
*/

int print_tel_pos (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int rc;
   int i, j, ntel;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;

   item_header.type = IO_TYPE_MC_TELPOS;    /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   ntel = get_long(iobuf);
   printf("\nCORSIKA IACT positions and sizes for %d telescopes:\n", ntel);
   for (i=0; i<4; i++ )
   {
      printf("   %s:", i==0?"x pos.":(i==1)?"y pos.":
            (i==2)?"z pos.":"radius");
      for (j=0; j<ntel; j++ )
      {
         if ( j>0 )
            printf(",");
         printf(" %5.3f", 0.01*get_real(iobuf));
      }
      printf(" m\n");
   }

   return get_item_end(iobuf,&item_header);
}
/* ------------------------ write_tel_offset ---------------------- */
/**
 *  Write offsets of randomly scattered arrays with respect to shower core.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  narray  Number of arrays of telescopes/detectors
 *  @param  toff  Time offset (ns, from first interaction to ground)
 *  @param  xoff  X offsets of arrays
 *  @param  yoff  Y offsets of arrays
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_offset (IO_BUFFER *iobuf, int narray, 
   double toff, double *xoff, double *yoff)
{
   return write_tel_offset_w(iobuf,narray,toff,xoff,yoff,NULL);
}

/* ------------------------ write_tel_offset_w ---------------------- */
/**
 *  Write offsets and weights of randomly scattered arrays with respect to shower core.
 *
 *  With respect to the backwards-compatible non-weights version write_tel_offset(),
 *  this version adds a weight to each offset position which should be normalized
 *  in such a way that with uniform sampling it should be the area over which
 *  showers are thrown divided by the number of array in each shower.
 *  With importance sampling the same relation should hold on average.
 *  So in either case, the average sum of weights for the different offsets
 *  in one shower equals just the area over which cores are randomized.
 *  This leaves the possibility to change the number of offsets from
 *  shower to shower.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  narray  Number of arrays of telescopes/detectors
 *  @param  toff  Time offset (ns, from first interaction to ground)
 *  @param  xoff  X offsets of arrays
 *  @param  yoff  Y offsets of arrays
 *  @param  weight Area weight for uniform or importance sampled core offset.
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_offset_w (IO_BUFFER *iobuf, int narray, 
   double toff, double *xoff, double *yoff, double *weight)
{
   IO_ITEM_HEADER item_header;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_TELOFF;   /* Data type */
   if ( weight != NULL )
      item_header.version = 1;             /* Version 1 (with weights) */
   else
      item_header.version = 0;             /* Version 0 (no weights) */
   item_header.ident = 0;
   put_item_begin(iobuf,&item_header);

   put_long((long)narray,iobuf);
   put_real(toff,iobuf);
   put_vector_of_real(xoff,narray,iobuf);
   put_vector_of_real(yoff,narray,iobuf);
   if ( weight != NULL )
      put_vector_of_real(weight,narray,iobuf);
   
   return put_item_end(iobuf,&item_header);
}

/* ----------------------- read_tel_offset ----------------------- */
/**
 *  Read offsets of randomly scattered arrays with respect to shower core.
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @param  max_array Maximum number of arrays that can be treated
 *  @param  narray    Number of arrays of telescopes/detectors
 *  @param  toff      Time offset (ns, from first interaction to ground)
 *  @param  xoff      X offsets of arrays
 *  @param  yoff      Y offsets of arrays
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_tel_offset (IO_BUFFER *iobuf, int max_array, int *narray,
   double *toff, double *xoff, double *yoff)
{
   return read_tel_offset_w(iobuf,max_array,narray,toff,xoff,yoff,NULL);
}

/* ----------------------- read_tel_offset_w ----------------------- */
/**
 *  Read offsets and weights of randomly scattered arrays with respect to shower core.
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @param  max_array Maximum number of arrays that can be treated
 *  @param  narray    Number of arrays of telescopes/detectors
 *  @param  toff      Time offset (ns, from first interaction to ground)
 *  @param  xoff      X offsets of arrays
 *  @param  yoff      Y offsets of arrays
 *  @param  weight    Area weight for uniform or importance sampled core offset.
 *                    For old version data (uniformly sampled), 0.0 is returned.
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_tel_offset_w (IO_BUFFER *iobuf, int max_array, int *narray,
   double *toff, double *xoff, double *yoff, double *weight)
{
   IO_ITEM_HEADER item_header;
   int rc;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_TELOFF;   /* Data type */
   if ( (rc=get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 && item_header.version != 1 )
      return -1;

   *narray = get_long(iobuf);
   if ( *narray > max_array )
   {
      fprintf(stderr,
         "Too many arrays in item type %d: %d found but only %d allowed.\n",
         IO_TYPE_MC_TELOFF,*narray, max_array );
      return -1;
   }
   *toff = get_real(iobuf);
   get_vector_of_real(xoff,*narray,iobuf);
   get_vector_of_real(yoff,*narray,iobuf);
   if ( weight == NULL )
   {
      if ( item_header.version == 1 )
      {
         int i;
         fprintf(stderr,"Core offsets may be weighted but weights are not used.\n");
         for (i=0; i<(*narray); i++)
            (void) get_real(iobuf);
      }
   }
   else
   {
      if ( item_header.version == 1 )
         get_vector_of_real(weight,*narray,iobuf);
      else
      {
         /* Since the range of core offsets is not known here, the proper area */
         /* weight for older uniformly sampled data cannot be calculated. */
         int i;
         for (i=0; i<(*narray); i++)
            weight[i] = 0.;
      }
   }

   return get_item_end(iobuf,&item_header);
}

/* ----------------------- print_tel_offset ----------------------- */
/**
 *  Print offsets and weights of randomly scattered arrays with respect to shower core.
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int print_tel_offset (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int rc;
   int i, narray;
   double toff;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_TELOFF;   /* Data type */
   if ( (rc=get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 && item_header.version != 1 )
      return -1;

   narray = get_long(iobuf);
   toff = get_real(iobuf);
   printf("\nCORSIKA IACT array offsets for %d arrays (common time offset = %g ns)\n",
      narray, toff);
   printf("   x offsets:");
   for (i=0; i<narray; i++)
   {
      if ( i>0 )
         printf(",");
      printf(" %5.3f", 0.01*get_real(iobuf));
   }
   printf(" m\n");
   printf("   y offsets:");
   for (i=0; i<narray; i++)
   {
      if ( i>0 )
         printf(",");
      printf(" %5.3f", 0.01*get_real(iobuf));
   }
   printf(" m\n");
   if ( item_header.version == 1 )
   {
      printf("   weights:");
      for (i=0; i<narray; i++)
      {
         if ( i>0 )
            printf(",");
         printf(" %g", get_real(iobuf));
      }
      printf("\n");
   }

   return get_item_end(iobuf,&item_header);
}

/* ------------------------ begin_write_tel_array -------------------- */
/**
 *  Begin writing data for one array of telescopes/detectors.
 *  Note: this function does not finish writing to the I/O block
 *  but after writing of the photons a call to end_write_tel_array()
 *  is needed.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  ih      I/O item header (for item opened here)
 *  @param  array   Number of array
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
 *
*/

int begin_write_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int array)
{
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   ih->type = IO_TYPE_MC_TELARRAY;   /* Data type */
   ih->version = 0;                  /* Version 0 (test) */
   ih->ident = array;
   return put_item_begin(iobuf,ih);
}

/* ----------------------- end_write_tel_array --------------------- */
/**
 *  End writing data for one array of telescopes/detectors.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  ih      I/O item header (as opened in begin_write_tel_array() )
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int end_write_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih)
{
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   return put_item_end(iobuf,ih);
}

/* ----------------------- begin_read_tel_array --------------------- */
/**
 *  Begin reading data for one array of telescopes/detectors.
 *  Note: this function does not finish reading from the I/O block
 *  but after reading of the photons a call to end_read_tel_array()
 *  is needed.
 *
 *  @param iobuf  --  I/O buffer descriptor
 *  @param ih	  --  I/O item header (for item opened here)
 *  @param array  --  Number of array
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
 *
*/

int begin_read_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int *array)
{
   int rc;
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   ih->type = IO_TYPE_MC_TELARRAY;     /* Data type */
   rc = get_item_begin(iobuf,ih);
   if ( ih->version != 0 )
      return -1;
   
   *array = ih->ident;
   return rc;
}

/* ----------------------- end_read_tel_array ---------------------- */
/**
 *  End reading data for one array of telescopes/detectors.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  ih      I/O item header (as opened in begin_write_tel_array() )
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int end_read_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih)
{
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   return get_item_end(iobuf,ih);
}

/* ------------------------ begin_write_tel_array -------------------- */
/**
 *  Begin writing data for one array of telescopes/detectors.
 *  Note: this function does not finish writing to the I/O block
 *  but after writing of the photons a call to end_write_tel_array()
 *  is needed.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  ih      I/O item header (for item opened here)
 *  @param  array   Number of array
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
 *
*/

int write_tel_array_head (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int array)
{
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   ih->type = IO_TYPE_MC_TELARRAY_HEAD;   /* Data type */
   ih->version = 0;                  /* Version 0 (test) */
   ih->ident = array;
   put_item_begin(iobuf,ih);
   return put_item_end(iobuf,ih);
}

/* ----------------------- write_tel_array_end --------------------- */
/**
 *  End writing data for one array of telescopes/detectors.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  ih      I/O item header (as opened in begin_write_tel_array() )
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_array_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int array)
{
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   ih->type = IO_TYPE_MC_TELARRAY_END;   /* Data type */
   ih->version = 0;                  /* Version 0 (test) */
   ih->ident = array;
   put_item_begin(iobuf,ih);
   return put_item_end(iobuf,ih);
}

/* ----------------------- read_tel_array_head --------------------- */
/**
 *  Begin reading data for one array of telescopes/detectors.
 *  Note: this function does not finish reading from the I/O block
 *  but after reading of the photons a call to end_read_tel_array()
 *  is needed.
 *
 *  @param iobuf  --  I/O buffer descriptor
 *  @param ih	  --  I/O item header (for item opened here)
 *  @param array  --  Number of array
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
 *
*/

int read_tel_array_head (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int *array)
{
   int rc;
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   ih->type = IO_TYPE_MC_TELARRAY_HEAD;     /* Data type */
   rc = get_item_begin(iobuf,ih);
   if ( rc < 0 )
      return rc;
   if ( ih->version != 0 )
      return -1;
   
   *array = ih->ident;
   return get_item_end(iobuf,ih);
}

/* ----------------------- read_tel_array_end ---------------------- */
/**
 *  End reading data for one array of telescopes/detectors.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  ih      I/O item header (as opened in begin_write_tel_array() )
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_tel_array_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int *array)
{
   int rc;
   if ( iobuf == (IO_BUFFER *) NULL || ih == (IO_ITEM_HEADER *) NULL )
      return -1;
   
   ih->type = IO_TYPE_MC_TELARRAY_HEAD;     /* Data type */
   rc = get_item_begin(iobuf,ih);
   if ( rc < 0 )
      return rc;
   if ( ih->version != 0 )
      return -1;

   *array = ih->ident;
   return get_item_end(iobuf,ih);
}

/* ----------------------- write_tel_photons --------------------- */
/**
 *  Write all the photon bunches for one telescope to an I/O buffer.
 *  Usually, calls to this function for each telescope in an array
 *  should be enclosed within calls to begin_write_tel_array() and
 *  end_write_tel_array().
 *  This routine writes the less compact format (32 bytes per bunch).
 *
 *  @param  iobuf        I/O buffer descriptor
 *  @param  array        array number
 *  @param  tel          telescope number
 *  @param  photons      sum of photons (and fractions) in this device
 *  @param  bunches      list of photon bunches
 *  @param  nbunches     number of elements in bunch list
 *  @param  ext_bunches  number of elements in external file
 *  @param  ext_fname	 name of external (temporary) file
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_photons (IO_BUFFER *iobuf, int array, int tel, 
   double photons, struct bunch *bunches, int nbunches,
   int ext_bunches, char *ext_fname)
{
   IO_ITEM_HEADER item_header;
   FILE *ext = NULL;
   int i;
   struct bunch tbunch;
#ifdef DEBUG_PHOTONS_SUM
   double sum = 0.;
#endif
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PHOTONS;   /* Data type */
   item_header.version = 0;                 /* Version 0 (test) */
   item_header.ident = 1000*array+tel;
   put_item_begin(iobuf,&item_header);

   put_short(array,iobuf);
   put_short(tel,iobuf);
   put_real(photons,iobuf);
   if ( ext_bunches <= 0 || ext_fname == NULL )
      ext_bunches = 0;
   else if ( ext_fname[0] == '\0' )
      ext_bunches = 0;
   else if ( (ext = fileopen(ext_fname,"r")) == NULL )
      ext_bunches = 0;

   put_long(nbunches+ext_bunches,iobuf);

   if ( ext != NULL )
   {
      int err_count = 0;
      for (i=0; i<ext_bunches; i++)
      {
         if ( fread((void *)&tbunch,1,sizeof(tbunch),ext) < 1 )
         {
            if ( err_count++ == 0 )
            {
               perror(ext_fname);
               fprintf(stderr,
                  "Filling in zeros for remaining external bunches.\n");
            }
            tbunch.x=tbunch.y=tbunch.cx=tbunch.cy=tbunch.ctime=tbunch.zem=
                     tbunch.photons=tbunch.lambda=0.;
         }
         put_real(tbunch.x,iobuf);
         put_real(tbunch.y,iobuf);
         put_real(tbunch.cx,iobuf);
         put_real(tbunch.cy,iobuf);
         put_real(tbunch.ctime,iobuf);
         put_real(tbunch.zem,iobuf);
         put_real(tbunch.photons,iobuf);
         put_real(tbunch.lambda,iobuf);
#ifdef DEBUG_PHOTONS_SUM
         if ( tbunch.lambda < 9000. )
            sum += tbunch.photons;
#endif
      }
      fileclose(ext);

      if ( err_count > 0 )
         fprintf(stderr,"A total of %d photon bunches were lost.\n", err_count);
   }

   for (i=0; i<nbunches; i++)
   {
      put_real(bunches[i].x,iobuf);
      put_real(bunches[i].y,iobuf);
      put_real(bunches[i].cx,iobuf);
      put_real(bunches[i].cy,iobuf);
      put_real(bunches[i].ctime,iobuf);
      put_real(bunches[i].zem,iobuf);
      put_real(bunches[i].photons,iobuf);
      put_real(bunches[i].lambda,iobuf);
#ifdef DEBUG_PHOTONS_SUM
      if ( bunches[i].lambda < 9000. )
         sum += bunches[i].photons;
#endif
   }

#ifdef DEBUG_PHOTONS_SUM
   if ( photons > 1. )
      if ( fabs(sum/photons-1.) > 0.02 && fabs(sum-photons) > 1. )
         fprintf(stderr,"Number of photons should be %f but sum of %d"
            " bunches results in %f photons.\n", 
            photons, nbunches+ext_bunches, sum);
#endif

   return put_item_end(iobuf,&item_header);
}

/* ------------------- write_tel_compact_photons ------------------ */
/**
 *  Write all the photon bunches for one telescope to an I/O buffer.
 *  Usually, calls to this function for each telescope in an array
 *  should be enclosed within calls to begin_write_tel_array() and
 *  end_write_tel_array().
 *  This routine writes the more compact format (16 bytes per bunch).
 *  The more compact format should usually be used to save memory and
 *  disk space.
 *
 *  @param  iobuf	 I/O buffer descriptor
 *  @param  array	 array number
 *  @param  tel 	 telescope number
 *  @param  photons	 sum of photons (and fractions) in this device
 *  @param  cbunches	 list of photon bunches
 *  @param  nbunches	 number of elements in bunch list
 *  @param  ext_bunches  number of elements in external file
 *  @param  ext_fname	 name of external (temporary) file
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_tel_compact_photons (IO_BUFFER *iobuf, int array, int tel, 
   double photons, struct compact_bunch *cbunches, int nbunches,
   int ext_bunches, char *ext_fname)
{
   IO_ITEM_HEADER item_header;
   int i;
   FILE *ext = NULL;
   struct compact_bunch tbunch;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PHOTONS;   /* Data type */
   item_header.version = 1000;              /* Version 0 (test) */
   item_header.ident = 1000*array+tel;
   put_item_begin(iobuf,&item_header);

   put_short(array,iobuf);
   put_short(tel,iobuf);
   put_real(photons,iobuf);

   if ( ext_bunches <= 0 || ext_fname == NULL )
      ext_bunches = 0;
   else if ( ext_fname[0] == '\0' )
      ext_bunches = 0;
   else if ( (ext = fileopen(ext_fname,"r")) == NULL )
      ext_bunches = 0;

   put_long(nbunches+ext_bunches,iobuf);

   if ( ext != NULL )
   {
      int err_count = 0;
      for (i=0; i<ext_bunches; i++)
      {
         if ( fread((void *)&tbunch,1,sizeof(tbunch),ext) < 1 )
         {
            if ( err_count++ == 0 )
            {
               perror(ext_fname);
               fprintf(stderr,
                  "Filling in zeros for remaining external bunches.\n");
            }
            tbunch.x=tbunch.y=tbunch.cx=tbunch.cy=tbunch.ctime=tbunch.log_zem=
                     tbunch.photons=tbunch.lambda=0;
         }
         put_short(tbunch.x,iobuf);
         put_short(tbunch.y,iobuf);
         put_short(tbunch.cx,iobuf);
         put_short(tbunch.cy,iobuf);
         put_short(tbunch.ctime,iobuf);
         put_short(tbunch.log_zem,iobuf);
         put_short(tbunch.photons,iobuf);
         put_short(tbunch.lambda,iobuf);
      }
      fileclose(ext);

      if ( err_count > 0 )
         fprintf(stderr,"A total of %d photon bunches were lost.\n", err_count);
   }

   for (i=0; i<nbunches; i++)
   {
      put_short(cbunches[i].x,iobuf);
      put_short(cbunches[i].y,iobuf);
      put_short(cbunches[i].cx,iobuf);
      put_short(cbunches[i].cy,iobuf);
      put_short(cbunches[i].ctime,iobuf);
      put_short(cbunches[i].log_zem,iobuf);
      put_short(cbunches[i].photons,iobuf);
      put_short(cbunches[i].lambda,iobuf);
   }
   
   return put_item_end(iobuf,&item_header);
}

/* ------------------------- read_tel_photons --------------------- */
/**
 *  Read bunches of Cherenkov photons for one telescope/detector.
 *  The data format may be either the more or less compact one.
 *
 *  @param  iobuf	 I/O buffer descriptor
 *  @param  max_bunches  maximum number of bunches that can be treated
 *  @param  array	 array number
 *  @param  tel 	 telescope number
 *  @param  photons	 sum of photons (and fractions) in this device
 *  @param  bunches	 list of photon bunches
 *  @param  nbunches	 number of elements in bunch list
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/


int read_tel_photons (IO_BUFFER *iobuf, int max_bunches, int *array,
   int *tel, double *photons, struct bunch *bunches, int *nbunches)
{
   IO_ITEM_HEADER item_header;
   int i, rc;
   double check_photons = 0.;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PHOTONS;  /* Data type */
   if ( (rc=get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version%1000 != 0 )
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }

   *array = get_short(iobuf);
   *tel = get_short(iobuf);
   *photons = get_real(iobuf);
   *nbunches = get_long(iobuf);

   /* We may have a first attempt at finding out the numbers. */
   if ( bunches == NULL )
   {
      unget_item(iobuf,&item_header);
      return -10;
   }
   
   if ( *nbunches > max_bunches )
   {
      fflush(NULL);
      fprintf(stderr,"Too many photon bunches in item type %d.\n",
         IO_TYPE_MC_PHOTONS);
      fflush(NULL);
      get_item_end(iobuf,&item_header);
      return -1;
   }

   if ( item_header.version/1000 == 0 ) /* The long format */
   {
      for (i=0; i<*nbunches; i++)
      {
         bunches[i].x = get_real(iobuf);
         bunches[i].y = get_real(iobuf);
         bunches[i].cx = get_real(iobuf);
         bunches[i].cy = get_real(iobuf);
         bunches[i].ctime = get_real(iobuf);
         bunches[i].zem = get_real(iobuf);
         bunches[i].photons = get_real(iobuf);
         bunches[i].lambda = get_real(iobuf);
         check_photons += bunches[i].photons;
      }
   }
   else if ( item_header.version/1000 == 1 ) /* The compact format */
   {
      for (i=0; i<*nbunches; i++)
      {
         bunches[i].x = 0.1*get_short(iobuf);
         bunches[i].y = 0.1*get_short(iobuf);
         bunches[i].cx = get_short(iobuf)/30000.;
         if ( bunches[i].cx > 1. )
            bunches[i].cx = 1.;
         else if ( bunches[i].cx < -1. )
            bunches[i].cx = -1.;
         bunches[i].cy = get_short(iobuf)/30000.;
         if ( bunches[i].cy > 1. )
            bunches[i].cy = 1.;
         else if ( bunches[i].cy < -1. )
            bunches[i].cy = -1.;
         bunches[i].ctime = 0.1*get_short(iobuf);
         bunches[i].zem = pow(10.,0.001*get_short(iobuf));
         bunches[i].photons = 0.01*get_short(iobuf);
         bunches[i].lambda = get_short(iobuf);
         check_photons += bunches[i].photons;
      }
   }
   else
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }
   
   if ( *photons > 10.0 )
      if ( fabs(check_photons-(*photons)) / (*photons) > 0.01 )
         fprintf(stderr,"Photon numbers do not match. Maybe problems with disk space?\n");

   return get_item_end(iobuf,&item_header);
}

/* ------------------------- print_tel_photons --------------------- */
/**
 *  Print bunches of Cherenkov photons for one telescope/detector.
 *  The data format may be either the more or less compact one.
 *
 *  @param  iobuf	 I/O buffer descriptor
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/


int print_tel_photons (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int i, rc;
   double check_photons = 0.;
   int array, tel;
   double photons;
   struct bunch b;
   int nbunches;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PHOTONS;  /* Data type */
   if ( (rc=get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version%1000 != 0 )
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }

   array = get_short(iobuf);
   tel = get_short(iobuf);
   photons = get_real(iobuf);
   nbunches = get_long(iobuf);

   if ( array == 999 && tel == 999 )
   {
      printf("There are %d particles arriving at ground level (encoded like photon bunches):\n",
         nbunches);
   }
   else
   {
      printf("Telescope no. %d in array %d gets %f photons in %d bunches (%s format)\n",
         tel, array, photons, nbunches, item_header.version/1000 == 0 ? "long" : "compact");
   }

   if ( item_header.version/1000 == 0 ) /* The long format */
   {
      for (i=0; i<nbunches; i++)
      {
         b.x = get_real(iobuf);
         b.y = get_real(iobuf);
         b.cx = get_real(iobuf);
         b.cy = get_real(iobuf);
         b.ctime = get_real(iobuf);
         b.zem = get_real(iobuf);
         b.photons = get_real(iobuf);
         b.lambda = get_real(iobuf);
         if ( b.lambda < 9990. && !(array == 999 && tel == 999) )
            check_photons += b.photons;
         if ( i<10 )
         {
            if ( array == 999 && tel == 999 )
               printf("   Particle of type %d at %f m, %f m in direction %f,%f, arrival time %f ns, "
                   "momentum %f GeV/c, with weight %f at level %d.\n",
                   (int)(b.lambda*0.001+0.1), b.x*0.01, b.y*0.01, 
                   b.cx, b.cy, b.ctime, b.zem, b.photons,
                   (int)(b.lambda+0.1)%10);
            else
               printf("   Bunch at %f m, %f m in direction %f,%f, arrival time %f ns, "
                   "emission at %f m height, with %f photons of wavelength %f nm.\n",
                   b.x*0.01, b.y*0.01, b.cx, b.cy, b.ctime, b.zem*0.01, b.photons, b.lambda);
         }
         else if ( i==10 )
            printf("   ...\n");
      }
   }
   else if ( item_header.version/1000 == 1 ) /* The compact format */
   {
      for (i=0; i<nbunches; i++)
      {
         b.x = 0.1*get_short(iobuf);
         b.y = 0.1*get_short(iobuf);
         b.cx = get_short(iobuf)/30000.;
         if ( b.cx > 1. )
            b.cx = 1.;
         else if ( b.cx < -1. )
            b.cx = -1.;
         b.cy = get_short(iobuf)/30000.;
         if ( b.cy > 1. )
            b.cy = 1.;
         else if ( b.cy < -1. )
            b.cy = -1.;
         b.ctime = 0.1*get_short(iobuf);
         b.zem = pow(10.,0.001*get_short(iobuf));
         b.photons = 0.01*get_short(iobuf);
         b.lambda = get_short(iobuf);
         if ( b.lambda < 9990. )
            check_photons += b.photons;
         if ( i<10 )
            printf("   Bunch at %f,%f direction %f,%f, arrival time %f, "
                   "emission at %f, with %f photons of wavelength %f nm.\n",
                   b.x, b.y, b.cx, b.cy, b.ctime, b.zem, b.photons, b.lambda);
         else if ( i==10 )
            printf("   ...\n");
      }
   }
   else
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }

   if ( photons > 10.0 )
      if ( fabs(check_photons-(photons)) / (photons) > 0.01 )
         fprintf(stderr,"Photon numbers do not match. Maybe problems with disk space?\n");

   return get_item_end(iobuf,&item_header);
}

/* ---------------- write_shower_longitudinal ------------------ */
/**
 *  Write CORSIKA shower longitudinal distributions.
 *  See tellng_() in iact.c for more detailed parameter description.
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @param  event     event number
 *  @param  type      1 = particle numbers, 2 = energy, 3 = energy deposits
 *  @param  data      set of (usually 9) distributions
 *  @param  ndim      maximum number of entries per distribution
 *  @param  np	      number of distributions (usually 9)
 *  @param  nthick    number of entries actually filled per distribution
 *      		(is 1 if called without LONGI being enabled).
 *  @param  thickstep step size in g/cm**2
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_shower_longitudinal (IO_BUFFER *iobuf, int event, int type, 
    double *data, int ndim, int np, int nthick, double thickstep)
{
   IO_ITEM_HEADER item_header;
   int i;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_LONGI;  /* Data type */
   item_header.version = 0;               /* Version 0 (test) */
   item_header.ident = (event%100000000)*10+type%10;
   put_item_begin(iobuf,&item_header);

   put_long(event,iobuf);
   put_long(type,iobuf);
   put_short(np,iobuf);
   put_short(nthick,iobuf);
   put_real(thickstep,iobuf);
   for ( i=0; i<np; i++)
      put_vector_of_real(data+i*ndim,nthick,iobuf);
   
   return put_item_end(iobuf,&item_header);
}

/* ----------------- read_shower_longitudinal ------------------ */
/**
 *  Read CORSIKA shower longitudinal distributions.
 *  See tellng_() in iact.c for more detailed parameter description.
 *
 *  @param  iobuf     I/O buffer descriptor
 *  @param  event     return event number
 *  @param  type      return 1 = particle numbers, 2 = energy, 3 = energy deposits
 *  @param  data      return set of (usually 9) distributions
 *  @param  ndim      maximum number of entries per distribution
 *  @param  np	      return number of distributions (usually 9)
 *  @param  nthick    return number of entries actually filled per distribution
 *      		(is 1 if called without LONGI being enabled).
 *  @param  thickstep return step size in g/cm**2
 *  @param  max_np    maximum number of distributions for which we have space.
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_shower_longitudinal (IO_BUFFER *iobuf, int *event, int *type, 
   double *data, int ndim, int *np, 
   int *nthick, double *thickstep, int max_np)
{
   IO_ITEM_HEADER item_header;
   int rc, i;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_LONGI;   /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   *event = get_long(iobuf);
   *type  = get_long(iobuf);
   *np    = get_short(iobuf);
   *nthick= get_short(iobuf);
   *thickstep = get_real(iobuf);
   if ( (*nthick) > 0 && (*np) > 0 && (*nthick) <= ndim && (*np) <= max_np )
   {
      for (i=0; i<(*np); i++)
      	 get_vector_of_real(data+i*ndim,(*nthick),iobuf);
   }
   else
   {
      fprintf(stderr,"Invalid size %d * %d of longitudinal shower data\n",
         *nthick, *np);
      get_item_end(iobuf,&item_header);
      return -4;
   }
   
#ifdef DEBUG_TEST_ALL
 { int j;
   printf("\n%s longitudinal distribution:\n",(*type)==1?"Particle":(*type)==2?"Energy":(*type)==3?"Energy deposit":"Other");
   for (j=0; j<(*nthick); j++)
   {
      printf("%5.0f",j*(*thickstep));
      for (i=0; i<(*np); i++)
      	 printf(" %13.6g",data[i*ndim+j]);
      printf("\n");
   }
   printf("\n");
 }
#endif

   return get_item_end(iobuf,&item_header);
}

/* ---------------------- write_camera_layout ------------------- */
/**
 *  Write the layout (pixel positions) of a camera used for converting
 *  from photons to photo-electrons in a pixel.
 *
 *  @param  iobuf  I/O buffer descriptor
 *  @param  itel   telescope number
 *  @param  type   camera type (hex/square)
 *  @param  pixels number of pixels
 *  @param  xp     X positions of pixels
 *  @param  yp     Y position of pixels
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_camera_layout (IO_BUFFER *iobuf, int itel, int type, int pixels,
    double *xp, double *yp)
{
   IO_ITEM_HEADER item_header;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_LAYOUT;  /* Data type */
   item_header.version = 0;               /* Version 0 (test) */
   item_header.ident = itel;
   put_item_begin(iobuf,&item_header);

   put_short(type,iobuf);
   put_short(pixels,iobuf);
   put_vector_of_real(xp,pixels,iobuf);
   put_vector_of_real(yp,pixels,iobuf);
   
   return put_item_end(iobuf,&item_header);
}

/* ---------------------- read_camera_layout ------------------- */
/**
 *  Read the layout (pixel positions) of a camera used for converting
 *  from photons to photo-electrons in a pixel.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @param  max_pixels The maximum number of pixels that can be stored in xp, yp.
 *  @param  itel    telescope number
 *  @param  type    camera type (hex/square)
 *  @param  pixels  number of pixels
 *  @param  xp      X positions of pixels
 *  @param  yp      Y position of pixels
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_camera_layout (IO_BUFFER *iobuf, int max_pixels, int *itel, 
   int *type, int *pixels, double *xp, double *yp)
{
   IO_ITEM_HEADER item_header;
   int rc;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_LAYOUT;   /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   *itel = item_header.ident;
   *type = get_short(iobuf);
   *pixels = get_short(iobuf);
   if ( *pixels > max_pixels )
   {
      get_item_end(iobuf,&item_header);
      return -4;
   }
   
   get_vector_of_real(xp,*pixels,iobuf);
   get_vector_of_real(yp,*pixels,iobuf);
   
   return get_item_end(iobuf,&item_header);
}

/* ---------------------- print_camera_layout ------------------- */
/**
 *  Print the layout (pixel positions) of a camera used for converting
 *  from photons to photo-electrons in a pixel.
 *
 *  @param  iobuf   I/O buffer descriptor
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int print_camera_layout (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int rc;
   int ipix, itel, type, pixels;
   double xp, yp;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_LAYOUT;   /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 0 )
      return -1;

   itel = item_header.ident;
   type = get_short(iobuf);
   pixels = get_short(iobuf);
   
   printf("Camera of telescope %d is of type %d with %d pixels.\n",
      itel, type, pixels);
   printf("   Pixel x positions: ");
   for ( ipix=0; ipix<pixels; ipix++ )
   {
      xp = get_real(iobuf);
      if ( ipix<10 )
         printf("%f, ", xp);
      else if ( ipix==10 )
         printf("...");
   }
   printf("\n");
   printf("   Pixel y positions: ");
   for ( ipix=0; ipix<pixels; ipix++ )
   {
      yp = get_real(iobuf);
      if ( ipix<10 )
         printf("%f, ", yp);
      else if ( ipix==10 )
         printf("...");
   }
   printf("\n");
   
   return get_item_end(iobuf,&item_header);
}

/* ------------------------ write_photo_electrons ----------------------- */
/**
 *  Write the photo-electrons registered in a Cherenkov telescope camera.
 *
 *  @param  iobuf      I/O buffer descriptor
 *  @param  array      array number
 *  @param  tel        telescope number
 *  @param  npe        Total number of photo-electrons in the camera.
 *  @param  pixels     No. of pixels to be written
 *  @param  flags      Bit 0: amplitudes available, bit 1: includes NSB p.e.
 *  @param  pe_counts  Numbers of photo-electrons in each pixel
 *  @param  tstart     Offsets in 't' at which data for each pixel starts
 *  @param  t	       Time of arrival of photons at the camera.
 *  @param  a		 Amplitudes of p.e. signals [mean p.e.] (optional, may be NULL).
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int write_photo_electrons (IO_BUFFER *iobuf, int array, int tel, int npe, int flags,
   int pixels, int *pe_counts, int *tstart, double *t, double *a)
{
   IO_ITEM_HEADER item_header;
   int i, nonempty;
   if ( a == NULL )
      flags &= 0xe; /* Don't tell we have amplitudes if there aren't any */
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PE;   /* Data type */
   item_header.version = 2;            /* Version 2 */
   item_header.ident = 1000*array+tel;
   put_item_begin(iobuf,&item_header);

   put_long(npe,iobuf);
   put_long(pixels,iobuf);
   if ( item_header.version > 1 )
      put_short(flags,iobuf);

   if ( npe <= 0 )
      /* If there are no photo-electrons, all pixels are empty. */
      put_long(0,iobuf);
   else
   {
      /* Let's first find out how many are non-empty. */
      for (i=nonempty=0; i<pixels; i++)
      {
	 if ( pe_counts[i] <= 0 )
            continue;
	 nonempty++;
      }
      put_long(nonempty,iobuf);
      for (i=0; i<pixels; i++)
      {
	 if ( pe_counts[i] <= 0 )
            continue;
         /* FIXME: limiting number of pixels to <= 32767 */
	 put_short(i,iobuf);
	 put_long(pe_counts[i],iobuf);
	 put_vector_of_real(t+tstart[i],pe_counts[i],iobuf);
         if ( item_header.version > 1 && (flags&1) != 0 )
	    put_vector_of_real(a+tstart[i],pe_counts[i],iobuf);
      }
   }
   
   return put_item_end(iobuf,&item_header);
}


/* ------------------------ read_photo_electrons ----------------------- */
/**
 *  Read the photoelectrons registered in a Cherenkov telescope camera.
 *
 *  @param  iobuf	 I/O buffer descriptor
 *  @param  max_pixels   Maximum number of pixels which can be treated
 *  @param  max_pe	 Maximum number of photo-electrons
 *  @param  array	 Array number
 *  @param  tel 	 Telescope number
 *  @param  npe          The total number of photo-electrons read.
 *  @param  pixels	 Number of pixels read. 
 *  @param  flags        Bit 0: amplitudes available, bit 1: includes NSB p.e.
 *  @param  pe_counts	 Numbers of photo-electrons in each pixel
 *  @param  tstart	 Offsets in 't' at which data for each pixel starts
 *  @param  t		 Time of arrival of photons at the camera.
 *  @param  a		 Amplitudes of p.e. signals [mean p.e.] (optional, may be NULL).
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int read_photo_electrons (IO_BUFFER *iobuf, int max_pixels, int max_pe,
   int *array, int *tel, int *npe, int *pixels, int *flags,
   int *pe_counts, int *tstart, double *t, double *a)
{
   IO_ITEM_HEADER item_header;
   int i, it, ipix, rc, nonempty;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PE;    /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 1 && item_header.version != 2 )
   {
      fprintf(stderr,"Invalid version %d of photo-electrons block.\n", 
         item_header.version);
      get_item_end(iobuf,&item_header);
      return -1;
   }

   *array = item_header.ident/1000;
   *tel = item_header.ident%1000;

   *npe = get_long(iobuf);
   *pixels = get_long(iobuf);
   if ( item_header.version > 1 )
      *flags = get_short(iobuf);
   else
      *flags = 0;
   nonempty = get_long(iobuf);

   /* We may have a first attempt for finding out how many there are. */
   if ( pe_counts == NULL || tstart == NULL || t == NULL )
   {
      unget_item(iobuf,&item_header);
      return -10;
   }

   if ( (*pixels) > max_pixels || (*npe) > max_pe || 
        (*pixels) < 0 || (*npe) < 0 ||
        nonempty > (*pixels) || nonempty < 0 )
   {
      if ( *pixels > max_pixels || *pixels < 0 )
         fprintf(stderr,
            "Too many pixels specified in photo-electrons block: %d > %d\n",
            *pixels, max_pixels);
      if ( *npe > max_pe || *npe < 0 )
         fprintf(stderr,"Number of photo-electrons exceeds list size: %d > %d\n",
            *npe, max_pe);
      if ( nonempty > (*pixels) || nonempty < 0 )
         fprintf(stderr,"Number of non-empty pixels not consistent with total number: %d / %d\n",
            nonempty, *pixels);
      get_item_end(iobuf,&item_header);
      return -4;
   }
   for (ipix=0; ipix<*pixels; ipix++)
      pe_counts[ipix] = tstart[ipix] = 0;
   for (i=it=0; i<nonempty; i++)
   {
      /* FIXME: limiting number of pixels to <= 32767 */
      ipix = get_short(iobuf);
      if ( ipix < 0 || ipix >= max_pixels )
      {
      	 Warning("Invalid pixel number for photo-electron list");
	 get_item_end(iobuf,&item_header);
	 return -5;
      }
      pe_counts[ipix] = get_long(iobuf);
      if ( pe_counts[ipix] < 0 || pe_counts[ipix] > max_pe )
      {
         fprintf(stderr,"Invalid number of photo-electrons for pixel %d: %d\n",
            ipix, pe_counts[ipix]);
	 get_item_end(iobuf,&item_header);
	 return -5;
      }
      if ( it + pe_counts[ipix] > max_pe )
      {
      	 pe_counts[ipix] = 0;
         fprintf(stderr,"Would read beyond end of photo-electron list\n");
	 get_item_end(iobuf,&item_header);
	 return -5;
      }
      tstart[ipix] = it;
      get_vector_of_real(t+it,pe_counts[ipix],iobuf);
      if ( ((*flags) & 1) != 0 && a != NULL )
         get_vector_of_real(a+it,pe_counts[ipix],iobuf);
      else if ( ((*flags) & 1) != 0 && a == NULL )
      {
         int j;
         for (j=0; j<pe_counts[ipix]; j++ )
            (void) get_real(iobuf); /* Amplitudes available but not used */
      }
      it += pe_counts[ipix];
   }

   return get_item_end(iobuf,&item_header);
}

/* ------------------------ print_photo_electrons ----------------------- */
/**
 *  List the the photoelectrons registered in a Cherenkov telescope camera.
 *
 *  @param  iobuf    I/O buffer descriptor
 *  @return 0 (o.k.), -1, -2, -3 (error, as usual in eventio)
*/

int print_photo_electrons (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int i, j, it, ipix, rc, nonempty;
   int array, tel, npe, pixels, flags = 0;
   int pe_counts, tstart;
   
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   
   item_header.type = IO_TYPE_MC_PE;    /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 1 && item_header.version != 2 )
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }

   array = item_header.ident/1000;
   tel = item_header.ident%1000;

   npe = get_long(iobuf);
   pixels = get_long(iobuf);
   if ( item_header.version > 1 )
      flags =  get_short(iobuf);
   nonempty = get_long(iobuf);

   printf("Photo-electrons for telescope no. %d in array %d with "
          "%d p.e. in %d pixels of which %d are non-empty (%s, %s).\n",
          tel, array, npe, pixels, nonempty,
          (flags&1)?"with amplitudes":"no amplitudes",
          (flags&2)?"including NSB":"no NSB");

   for (i=it=0; i<nonempty; i++)
   {
      ipix = get_short(iobuf);
      pe_counts = get_long(iobuf);
      tstart = it;
      if ( i<10 )
         printf("   Pixel %d: %d p.e. starting at offset %d.\n", 
            ipix, pe_counts, tstart);
      else if ( i==10 )
         printf("   ...\n");
      for ( j=0; j<pe_counts; j++ )
      {
         double tpe = get_real(iobuf);
         if ( i < 10 )
         {
            if ( j==0 )
               printf("       p.e. at time %4.2f ns", tpe);
            else if ( j<10 )
               printf(", %4.2f ns", tpe);
            else if ( j==10 )
               printf(", ...");
         }
      }
      if ( j > 0 )
         printf("\n");

      if ( (flags&1) != 0 )
      {
         for ( j=0; j<pe_counts; j++ )
         {
            double ape = get_real(iobuf);
            if ( i < 10 )
            {
               if ( j==0 )
                  printf("       p.e. with ampl. %4.2f pe", ape);
               else if ( j<10 )
                  printf(", %4.2f pe", ape);
               else if ( j==10 )
                  printf(", ...");
            }
         }
         if ( j > 0 )
            printf("\n");
      }
      it += pe_counts;
   }

   return get_item_end(iobuf,&item_header);
}

/* ------------------ write_shower_extra_parameters ------------------ */

int write_shower_extra_parameters (IO_BUFFER *iobuf,
      struct shower_extra_parameters *ep)
{
   IO_ITEM_HEADER item_header;
   size_t ni, nf, i;

   if ( iobuf == (IO_BUFFER *) NULL || ep == NULL )
      return -1;
   if ( !ep->is_set )
      return 0;

   item_header.type = IO_TYPE_MC_EXTRA_PARAM;   /* Data type */
   item_header.version = 1;            /* Version 1 */
   item_header.ident = ep->id;
   put_item_begin(iobuf,&item_header);

   put_real(ep->weight,iobuf);
   ni = ep->niparam;
   if ( ep->iparam == NULL )
      ni = 0;
   nf = ep->nfparam;
   if ( ep->fparam == NULL )
      nf = 0;
   put_count(ni,iobuf);
   put_count(nf,iobuf);
   for (i=0; i<ni; i++)
      put_int32((int32_t)ep->iparam[i],iobuf);
   for (i=0; i<nf; i++)
      put_real(ep->fparam[i],iobuf);

   return put_item_end(iobuf,&item_header);
}

/* ------------------ read_shower_extra_parameters ------------------ */

int read_shower_extra_parameters (IO_BUFFER *iobuf,
      struct shower_extra_parameters *ep)
{
   IO_ITEM_HEADER item_header;
   size_t ni=0, nf=0, i;
   int rc;

   if ( iobuf == (IO_BUFFER *) NULL || ep == NULL )
      return -1;
   ep->is_set = 0;

   item_header.type = IO_TYPE_MC_EXTRA_PARAM;    /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 1 || ep == NULL )
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }
   ep->id = item_header.ident;

   ep->weight = get_real(iobuf);
   ni = (size_t) get_count(iobuf);
   nf = (size_t) get_count(iobuf);

   if ( ni > 0 )
   {
      if ( ni != ep->niparam )
      {
         if ( ep->iparam != NULL )
            free(ep->iparam);
         if ( (ep->iparam = (int *) calloc(ni,sizeof(int))) == NULL )
         {
            get_item_end(iobuf,&item_header);
            return -2;
         }
         for (i=0; i<ni; i++)
            ep->iparam[i] = (int) get_int32(iobuf);
      }
   }
   ep->niparam = ni;

   if ( nf > 0 )
   {
      if ( nf != ep->nfparam )
      {
         if ( ep->fparam != NULL )
            free(ep->fparam);
         if ( (ep->fparam = (float *) calloc(nf,sizeof(float))) == NULL )
         {
            get_item_end(iobuf,&item_header);
            return -2;
         }
         for (i=0; i<nf; i++)
            ep->fparam[i] = (float) get_real(iobuf);
      }
   }
   ep->nfparam = nf;

   ep->is_set = 1;

   return get_item_end(iobuf,&item_header);
}

/* ------------------ print_shower_extra_parameters ------------------ */

int print_shower_extra_parameters (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   size_t ni, nf, i;
   int rc;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;

   item_header.type = IO_TYPE_MC_EXTRA_PARAM;    /* Data type */
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   if ( item_header.version != 1 )
   {
      get_item_end(iobuf,&item_header);
      return -1;
   }

   printf("Shower extra parameters (ID %ld):\n", item_header.ident);
   printf("   Weight: %f\n", get_real(iobuf));
   ni = (size_t) get_count(iobuf);
   nf = (size_t) get_count(iobuf);
   if ( ni > 0 )
   {
      printf("   Integer parameters:");
      for (i=0; i<ni; i++)
      {
         if ( i < 10 )
            printf(" %d", (int) get_int32(iobuf));
         else
         {
            if ( i == 10 )
               printf(" ...");
            (void) get_int32(iobuf);
         }
      }
      printf("\n");
   }
   if ( nf > 0 )
   {
      printf("   Floating-point parameters:");
      for (i=0; i<nf; i++)
      {
         if ( i < 10 )
            printf(" %f", get_real(iobuf));
         else
         {
            if ( i == 10 )
               printf(" ...");
            (void) get_real(iobuf);
         }
      }
      printf("\n");
   }

   return get_item_end(iobuf,&item_header);
}

/* ------------------ init_shower_extra_parameters ------------------ */
/**
 *  @short Initialize, resize, clear shower extra parameters.
 *
 *  @param ep Pointer to parameter block. A NULL value indicates that
 *      the static block is meant.
 *  @param ni_max The number of integer parameters to be used.
 *  @param nf_max The number of float parameters to be used.
 */

int init_shower_extra_parameters (struct shower_extra_parameters *ep,
   size_t ni_max, size_t nf_max)
{
   size_t i;

   if ( ep == NULL )
   {
      /* Not a user-defined parameter block; assume the static one was meant. */
      if ( (ep = get_shower_extra_parameters()) == NULL )
         return -1;
   }

   ep->id = 0;
   ep->is_set = 0;
   ep->weight = 1.0;

   if ( ep->iparam != NULL && ep->niparam == ni_max && ni_max > 0 )
   {
      /* No need to re-allocate, just reset. */
      for (i=0; i<ni_max; i++)
         ep->iparam[i] = 0;
   }
   else
   {
      if ( ep->iparam != NULL )
         free(ep->iparam);
      ep->niparam = 0;
      ep->iparam = NULL;
      if ( ni_max > 0 )
      {
         if ( (ep->iparam = (int *) calloc(ni_max,sizeof(int))) == NULL )
            return -1;
         ep->niparam = ni_max;
      }
   }

   if ( ep->fparam != NULL && ep->nfparam == nf_max && nf_max > 0 )
   {
      /* No need to re-allocate, just reset. */
      for (i=0; i<nf_max; i++)
         ep->fparam[i] = 0.;
   }
   else
   {
      if ( ep->fparam != NULL )
         free(ep->fparam);
      ep->nfparam = 0;
      ep->fparam = NULL;
      if ( nf_max > 0 )
      {
         if ( (ep->fparam = (float *) calloc(nf_max,sizeof(float))) == NULL )
            return -1;
         ep->nfparam = nf_max;
      }
   }

   return 0;
}

/* ------------------ clear_shower_extra_parameters ------------------ */
/**
 *  @short Similar to init_shower_extra_parameters() but without any
 *     attempts to re-allocate or resize buffers. Just clear contents.
 *
 *  @param ep Pointer to parameter block. A NULL value indicates that
 *      the static block is meant.
 */

int clear_shower_extra_parameters (struct shower_extra_parameters *ep)
{
   size_t i;

   if ( ep == NULL )
   {
      /* Not a user-defined parameter block; assume the static one was meant. */
      if ( (ep = get_shower_extra_parameters()) == NULL )
         return -1;
   }

   ep->id = 0;
   ep->is_set = 0;
   ep->weight = 1.0;

   if ( ep->iparam != NULL )
   {
      for (i=0; i<ep->niparam; i++)
         ep->iparam[i] = 0;
   }
   if ( ep->fparam != NULL )
   {
      for (i=0; i<ep->nfparam; i++)
         ep->fparam[i] = 0;
   }
   
   return 0;
}

/* ------------------ get_shower_extra_parameters ------------------ */
/**
 *  @short There is one global (more precisely: static) block of extra
 *      shower parameters as, for example, used in the CORSIKA IACT interface.
 *      Get a pointer to this block.
 */

static struct shower_extra_parameters private_shower_extra_parameters;

struct shower_extra_parameters *get_shower_extra_parameters()
{
   return &private_shower_extra_parameters;
}
