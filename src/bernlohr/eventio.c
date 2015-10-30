/* ============================================================================

   Copyright (C) 1991, 1992, 1993, 2000, 2007, 2008, 2009, 2010  Konrad Bernloehr

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
 *  @file eventio.c
 *  @short Basic functions for eventio data format.
 *   
 *  @author  Konrad Bernloehr
 *  @date    1991 to 2010
 *  @date    @verbatim CVS $Date: 2014/11/14 15:37:00 $ @endverbatim
 *  @version @verbatim CVS $Revision: 1.43 $ @endverbatim
    
@verbatim
 ================ General comments to eventio.c ======================

 'eventio.c' provides an interface for an (almost) machine-independent
 way to write and read event data, configuration data and Monte Carlo data.
 Byte ordering of the data is unimportant and data written in both
 byte orders are correctly read on any supported architecture.
 Usually the data is written to/read from a file (or separate files for
 different data types) to be opened before calling any eventio function.
 Other ways to 'save' data (e.g. into memory or via dedicated networking
 procedures can easily be incorporated by assigning an input and/or
 output function to an I/O buffer instead of a file handle or pointer.
 The data structure is designed to allow reading of a mixture of
 different types of items from a single file. For this purpose, 'items'
 (see below) should not be interspersed with low-level material and,
 therefore, low-level functions should not be called from anywhere
 outside eventio.c.

 -----------------------------------------------------------------------

 An 'item' has the following structure:

    Component	 Type	 Content      Description
    ---------	 ----	 -------      -----------
    sync-tag	 long	 0xD41F8A37   Signature of start of any item
				      (only for top item, not for sub-items).
    type/version long	 ...	      Item type (bits 0 to 15), a single
                                      bit (16) of user data, extension flag
                                      (bit 17), reserved bits (18 to 19), and 
                                      version of this item type (bits 20 to 31).
    ident	 long	 ...	      Unique identification number of the
				      item or -1.
    length	 long	 ...	      No. of bytes following for this item
				      (bits 0 to 29) and a flag indicating
				      whether the item consists entirely of
				      sub-items with known length (bit 30).
				      Bit 31 must be 0. The bytes needed
				      to pad the item to the next 4-byte
				      boundary are included in the length.
  [ extension    long    ...          Only present if the extension flag in the
                                      type/version field is set. At this time,
                                      bits 0 to 11 will extend the length field
                                      (as bits 30 to 41). Note that 32-bit
                                      machines will not be able to take advantage
                                      of more than the one or two of those bits.
                                      Bits 12 to 31 are reserved and must be 0. ]
    data	 ...	 ...	      Item data (may consist of elementary
				      data and of sub-items)

 Field 'sync-tag':
    The sync-tag is used to check that input is still synchronized.
    In the case of a synchronisation failure, all data should be skipped
    up to the next occurence of that byte combination or its reverse.
    The byte ordering of the sync-tag defines also the byte ordering
    of all data in the item. Only byte orders 0-1-2-3 and 3-2-1-0 are
    accepted at present.

 Field 'type/version':
    This field consists of a type number in bits 0 to 15 (values
    0 to 65535), a single bit of user data (user flag), reserved bits 
    17 to 19 (must be 0), and an item version number in bits 20 to 31 
    (values 0 to 4095). Whenever the format of an item changes in a way 
    which is incompatible with older reading software the version number 
    has to be increased. This ways the reading software can be adapted
    to multiple format versions.

 Field 'ident':
    Items of the same type can be distinguished if an identification
    number is supplied. Negative values are interpreted as 'no ident
    supplied'.

 Field 'length':
    Each item and sub-item must have the number of bytes in its
    data area, including padding bytes, in bits 0 to 30 of this field.
    If an item consists entirely of sub-items and no atomic data, it can
    be searched for a specific type of sub-item without having to 'decode'
    (read from the buffer) any of the sub-items. Such an item is kind of
    a directory of sub-items and is marked by setting bit 30 of the
    length field on. The longest possible item length is thus (2^30 - 1).
    Note that the length field specifies the length of the rest of the
    item but not the sync-tag, type/version number, and length fields.
    All (sub-) items are padded to make the total length a multiple of 4
    bytes and the no. of padded bytes must be included in 'length'.

 Field 'extension':
    This field is not present by default but requires the extension flag
    (as indicated in bit 17 of the 'type/version' field). Writing of
    data with the 'extension' flag can be forced by setting the 
    'extended' element of the I/O buffer to 1 in advance. It can also
    be activated on a per-item basis but complications would arise once
    a sub-item goes beyond the 1 GB limit and any of its ancestors does
    not have the extension activated.
    Only bits 0-11 are used at this time (as bits 30-41 of the item length).
    On 32-bit systems only one or two of these bits would be usable
    (depending if lengths are counted with signed or unsigned integers).
    All other bits are reserved and must be set to 0 for now.
    Data written with the extension field will not be readable with
    pre-mid-2007 versions of eventio. Apart from that and within the
    limitations of the host architecture, reading of data with or
    without extension field is completely transparent to the application.

 Data:
    Data of an item may be either sub-items or atomic data. An item may
    even consist of a mixture of both but in that case the sub-items
    are not accessible via 'directory' functions and can be processed
    only when the item data is 'decoded' by its corresponding 'read_...'
    function.
    The beginning of the data field is aligned on a 4-byte boundary to
    allow efficient access to data if the byte order needs not to be
    changed and if the data itself obeys the required alignment.

 -----------------------------------------------------------------------

 The 'atomic' data types are kept as close as possible to internal
 data types. This data is only byte-aligned unless all atomic data
 of an item obeys a 2-byte or 4-byte alignement.
 Note that the ANSI C internal type int32_t typically corresponds to
 both 'int' and 'long' on 32-bit machines but to 'int' only on
 64-bit machines and to 'long' only on 16-bit systems.
 Use the int32_t/uint32_t etc. types where the same length of
 internal variables is required.
 64-bit integer data are also implemented in eventio but not available
 on all systems.

   Type    Int. type   Size (bytes)   Comments
   ----    ---------   ------------   --------
   byte    [u]int8_t   1	      Character or very short integer.
   count   uintmax_t   1 to 9         Unsigned. Larger numbers need more bytes.
   scount  intmax_t    1 to 9         Signed. Larger numbers need more bytes.
   short   [u]int16_t  2	      Short integer (signed or unsigned).
   long    [u]int32_t  4	      Long integer (signed or unsigned).
   int64   [u]int64_t  8              Caution: not available on all systems.
   string  -	       2+length       Preceded by 2-byte length of string.
   long str.  -	       4+length       Preceded by 4-byte length of string.
   var str.   -        (1-5)+length   Preceded by length of string as 'count'.
   real    float       4	      32-bit IEEE floating point number with
				      the same byte order as a long integer.
   double  double      8	      64-bit IEEE floating point number.
   sfloat     -        2              16-bit OpenGL floating point number

 The byte-ordering of integers in input data is defined by that of
 the sync-tag (magic number) preceding top-level items. Therefore,
 the byte-ordering in a top-level item may differ from the ordering
 in a previous item. For output data the default ordering is so far to
 have the least-significant bytes first. This is the natural byte
 order on Mips R3000 and higher (under Ultrix), DEC Alpha, VAX, and Intel 
 (80)x86 CPUs but the inverse of the natural byte order on Motorola 680x0,
 RS6000, PowerPC, and Sparc CPUs. The ordering may change without
 notice and without changing version numbers. Except for performance
 considerations, the byte-ordering should not be relevant as long as
 only the 0-1-2-3 and 3-2-1-0 orders are considered, and byte ordering
 of floating point numbers is the same as for long integers.
 Byte ordering for writing may be changed during run-time with the
 'byte_order' element of the I/O buffer structure.
 Note that on CPUs with non-IEEE floating point format like VAX writing
 and reading of floating point numbers is likely to be less efficient
 than on IEEE-format CPUs.

 Note that if an 'int' variable is written via 'put_short()'
 and then read again via 'get_short()' not only the
 upper two bytes (on a 32-bit machine) are lost but
 also the sign bit is propagated from bit 15 to the
 upper 16 bits. Similarly, if a 'long' variable is written
 via 'put_long()' and later read via 'get_long()' on a
 64-bit-machine, not only the upper 4 bytes are lost but
 also the sign in bit 31 is propagated to the upper 32 bits.

 -----------------------------------------------------------------------

 Do not modify this file to include project-specific things!

 ====================================================================
@endverbatim
*/

#include "initial.h"      /* This file includes others as required. */
#define NO_FOREIGN_PROTOTYPES 1
#include "io_basic.h"     /* This file includes others as required. */
#ifndef FSTAT_NOT_AVAILABLE
#include <sys/types.h>
#include <sys/stat.h>
#endif
#ifdef OS_UNIX
#include <unistd.h>
#endif

#define IO_BUFFER_MINIMUM_SIZE 32L

/* Author: Konrad Bernloehr */

/* #define READ_BYTES(fd,buf,nb) read(fd,buf,nb) */
/* #define READ_BYTES(fd,buf,nb) fread(buf,1,nb,&_iob[fd]) */
#define READ_BYTES(fd,buf,nb) (((int)fd==0) ? \
  (ssize_t) fread((void *)buf,(size_t)1,(size_t)nb,stdin) : \
  read(fd,(void *)buf,(size_t)nb) )

#ifdef BUG_CHECK
static void bug_check (IO_BUFFER *iobuf)
{
   if ( iobuf->r_remaining > iobuf->buflen )
      Error("I/O buffer seems corrupt.\n");
   return;
}
#endif

/* ----------------------- allocate_io_buffer ------------------ */
/**
 *  @short Dynamic allocation of an I/O buffer.
 *
 *  Dynamic allocation of an I/O buffer. The actual length of
 *  the buffer is passed as an argument.
 *  The buffer descriptor is initialized.
 *
 *  @param  buflen  The length of the actual buffer in bytes.
 *			   A safety margin of 4 bytes is added.
 *
 *  @return Pointer to I/O buffer or NULL if allocation failed.
 */

IO_BUFFER *allocate_io_buffer (size_t buflen)
{
   IO_BUFFER *buf;
   
   /* Modern compilers might notice code never executed below */
   /* but for portability it is retained as it is. */
   /* Not every preprocessor would be able to handle that. */
   
   if ( sizeof(BYTE) != 1 )
   {
      Error("Sizes of bytes is not as expected.");
      Error("You better modify the sources and recompile.");
      Error("No buffers will be allocated.");
      return NULL;
   }
   
   if ( sizeof(int16_t) != 2 || sizeof(int32_t) != 4 )
   {
      Error("Sizes of 16-bit and 32-bit integers are not as expected.");
      Error("You better modify the sources and recompile.");
      Error("No buffers will be allocated.");
      return NULL;
   }
   if ( sizeof(uint16_t) != 2 || sizeof(uint32_t) != 4 )
   {
      Error("Sizes of 16-bit and 32-bit unsigned integers are not as expected.");
      Error("You better modify the sources and recompile.");
      Error("No buffers will be allocated.");
      return NULL;
   }
#ifdef SIXTY_FOUR_BITS
   if ( sizeof(long) != 8 )
   {
      Error("Size of long integers is not 64 bits as expected.");
      Error("You better modify the sources and recompile.");
      Error("No buffers will be allocated.");
      return NULL;
   }
#else
   if ( sizeof(long) != 4 )
   {
      Error("Size of long integers is not 32 bits as expected.");
      Error("You better modify the sources and recompile.");
      Error("No buffers will be allocated.");
      return NULL;
   }
#endif

   if ( (buf = (IO_BUFFER *) malloc(sizeof(IO_BUFFER))) ==
        (IO_BUFFER *) NULL )
   {
      Warning("Allocating I/O buffer failed");
      return(buf);
   }

   if ( buflen <= 0 )
      buflen = IO_BUFFER_INITIAL_LENGTH;
   else if ( buflen < IO_BUFFER_MINIMUM_SIZE )
      buflen = IO_BUFFER_MINIMUM_SIZE;

   /* If the allocation of the actual buffer fails, free the desciptor again */
   if ( (buf->buffer = (BYTE *) malloc((size_t)(buflen+8))) == (BYTE *) NULL )
   {
      char msg[256];
      (void) sprintf(msg,"Allocating %ld bytes for I/O buffer failed",
         (long)((size_t)(buflen)));
      Warning(msg);
      free((void *)buf);
      return((IO_BUFFER *) NULL);
   }
   
   buf->is_allocated = 1;

   buf->buflen = buf->w_remaining = buflen;
   buf->r_remaining = 0;
   buf->data = buf->buffer;
   buf->item_start_offset[0] = 0;
   buf->item_level = 0;
   buf->input_fileno = buf->output_fileno = -1;
   buf->input_file = buf->output_file = (FILE *) NULL;
   buf->regular = 0;
   buf->user_function = NULL;
   buf->item_length[0] = buf->sub_item_length[0] = 0;
   buf->data_pending = -1;
   buf->min_length = buflen;
   buf->max_length = IO_BUFFER_MAXIMUM_LENGTH;
   buf->aux_count = 0;
   buf->regular = 0;
   buf->extended = 0;
   buf->sync_err_count = 0;
   buf->sync_err_max = 100;

#if ( defined(CPU_68K) || defined(CPU_RS6000) || defined(CPU_PowerPC) )
# ifndef REVERSE_BYTE_ORDER
     buf->byte_order = 1;  /* Reverse byte order by default */
# else
     buf->byte_order = 0;  /* Natural byte order if wanted */
# endif
#else
# ifndef REVERSE_BYTE_ORDER
     buf->byte_order = 0;  /* Write with native byte order */
#  else
     buf->byte_order = 1;  /* Reverse byte order if wanted */
# endif
#endif

   return(buf);
}

/* ---------------------- extend_io_buffer ------------------------- */
/**
 *  @short Extend the dynamically allocated I/O buffer.
 *
 *  Extend the dynamically allocated I/O buffer and if an item
 *  has been started and the argument 'next_byte' is smaller
 *  than 256 that argument will be appended as the next
 *  byte to the buffer.
 *
 *  @param  iobuf      The I/O buffer descriptor
 *  @param  next_byte  The value of the next byte or >= 256
 *  @param  increment  The no. of bytes by which to increase
 *		           the buffer beyond the current point.
 *		           If there is remaining space for
 *		           writing, the buffer is extended by
 *		           less than 'increment'.
 *
 *  @return next_byte (modulo 256) if successful, -1 for failure
 */

int extend_io_buffer (IO_BUFFER *iobuf, unsigned next_byte, long increment)
{
   long new_length, offset, remaining;
   BYTE *tptr;
   static long last_failed_length;

   /* NULL argument passed? */
   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   /* No buffer content? */
   if ( iobuf->buffer == (BYTE *) NULL )
      return -1;
   /* Was the buffer obtained by other means than allocate_io_buffer()? */
   if ( !iobuf->is_allocated )
      return -1;
   if ( increment < 1048576 && iobuf->buflen >= 8388608 )
      increment = 1048576;
   else if ( increment < 262144 && iobuf->buflen >= 2097152 )
      increment = 262144;
   else if ( increment < 131072 && iobuf->buflen >= 1048567 )
      increment = 131072;
   else if ( increment < 8192 )
      increment = 8192;
   if ( 8*increment < iobuf->buflen && iobuf->buflen > 65536 )
   {
      long increment2 = ((iobuf->buflen-1)/65536)*8192;
      if ( increment2 > increment )
         increment = increment2;
   }
   if ( iobuf->item_level > 0 )
   {
      remaining = iobuf->buflen - (long) (iobuf->data-iobuf->buffer);
      if ( remaining >= increment )
      {
         iobuf->w_remaining += increment;
         if ( next_byte < 256 )
            *(iobuf->data++) = (BYTE) next_byte;
         return((int)next_byte&0xff);
      }
      else if ( increment > remaining )
         increment -= remaining;
   }

   /* If the reallocation fails, give a warning but not each time if */
   /* this function is called many times in an output loop before the */
   /* buffer status is actually checked. */
   if ( (new_length = iobuf->buflen + increment) > iobuf->max_length )
   {
      /* The following comparison is not strictly multi-threading safe */
      /* but is not considered a problem since it can only result in */
      /* too many warning messages and is only encountered in rare cases. */
      if ( iobuf->buflen != last_failed_length )
      {
      	 char msg[256];
         last_failed_length = iobuf->buflen;
         (void) sprintf(msg,
             "Cannot extend I/O buffer of length %ld by another %ld bytes",
             iobuf->buflen,increment);
         Warning(msg);
      }
      iobuf->w_remaining = -1;
      return -1;
   }
   offset = iobuf->data - iobuf->buffer;

   if ( (tptr = (BYTE *)
        realloc((void *)iobuf->buffer,(size_t)(new_length+8))) ==
        (BYTE *) NULL )
   {
      char msg[256];
      (void) sprintf(msg,
          "Insufficient memory for extending I/O block to %ld bytes",
          (long) ((size_t)(new_length+8)));
      Warning(msg);
      iobuf->w_remaining = -1;
      return -1;
   }
   else
   {
      char msg[256];
      sprintf(msg,"I/O block extended by %ld to %ld bytes",
         increment,new_length);
      Information(msg);
      iobuf->buffer = tptr;
      iobuf->data = iobuf->buffer + offset;
      iobuf->buflen = new_length;
      iobuf->w_remaining += increment;
      if ( iobuf->item_level > 0 && next_byte < 256 )
         *(iobuf->data++) = (BYTE) next_byte;
   }

   iobuf->r_remaining = iobuf->buflen - (int)(iobuf->data-iobuf->buffer);

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return (int)(next_byte&0xff);
}

/* ----------------------- free_io_buffer ---------------------- */
/**
 *  @short Free an I/O buffer that has been allocated at run-time.
 *
 *  Free an I/O buffer that has been allocated at
 *  run-time (e.g. by a call to allocate_io_buf()).
 *
 *  @param  iobuf   The buffer descriptor to be de-allocated.
 *
 *  @return (none)
 */

void free_io_buffer (IO_BUFFER *iobuf)
{
   if ( iobuf != (IO_BUFFER *) NULL )
   {
      if ( iobuf->buffer != (BYTE *) NULL && iobuf->is_allocated )
         free((void *)iobuf->buffer);
      free((void *)iobuf);
   }
}

/* --------------------- put_vector_of_byte -------------------- */
/**
 *  Put a vector of bytes into an I/O buffer.
 *
 *  @param  vec     Byte data vector.
 *  @param  num     Number of bytes to be put.
 *  @param  iobuf   I/O buffer descriptor.
 *
 *  @return (none)
 */

void put_vector_of_byte (const BYTE *vec, int num, IO_BUFFER *iobuf)
{
   if ( num <= 0 )
      return;

   if ( (iobuf->w_remaining-=num) < 0 )
   {
      long increment = IO_BUFFER_LENGTH_INCREMENT;
      if ( iobuf->w_remaining+increment < 0 )
         increment = IO_BUFFER_LENGTH_INCREMENT - iobuf->w_remaining;
      if ( extend_io_buffer(iobuf,256,increment) < 0 )
         return;
   }

   if ( vec == (BYTE *) NULL )
      memset((void *)iobuf->data,0,(size_t)num);
   else
      COPY_BYTES((void *)iobuf->data,(const void *)vec,(size_t)num);

   iobuf->data += num;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* --------------------- get_vector_of_byte -------------------- */
/**
 *  Get a vector of bytes from an I/O buffer.
 *
 *  @param  vec   --  Byte data vector.
 *  @param  num   --  Number of bytes to get.
 *  @param  iobuf --  I/O buffer descriptor.
 *
 *  @return (none)
 */

void get_vector_of_byte (BYTE *vec, int num, IO_BUFFER *iobuf)
{
   if ( num <= 0 )
      return;

   if ( (iobuf->r_remaining-=num) < 0 )
      return;

   if ( vec != (BYTE *) NULL )
      COPY_BYTES((void *)vec,(void *)iobuf->data,(size_t)num);
   iobuf->data += num;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* -------------------------- put_count ------------------------ */
/**
 *  @short Put an unsigned integer of unspecified length to an I/O buffer.
 *
 *  Put an unsigned integer of unspecified length in a way similar
 *  to the UTF-8 character encoding to an I/O buffer. The byte order
 *  resulting in the buffer is independent of the host byte order or the
 *  byte order in action for the I/O buffer, starting with as many
 *  leading bits in the first byte as extension bytes needed after 
 *  the first byte. While the scheme in principle allows for values
 *  of arbitrary length, the implementation is limited to 64 bits.
 *
 *  @param  n       The number to be saved. Even on systems with 64-bit
 *                  integers, this must not exceed 2**32-1 with the
 *                  current implementation.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void put_count (uintmax_t n, IO_BUFFER *iobuf)
{
   BYTE v[9]; /* Prepared for up to 64 bits */
   uintmax_t one = 1;
   int l = 0;
   if ( n < (one<<7) )
   {
      v[0] = (BYTE)n;
      l = 1;
   }
   else if ( n < (one<<14) )
   {
      v[0] = 0x80 | ((n>>8)&0x3f);
      v[1] = (n & 0xff);
      l = 2;
   }
   else if ( n < (one<<21) )
   {
      v[0] = 0xc0 | ((n>>16)&0x1f);
      v[1] = ((n>>8) & 0xff);
      v[2] = (n & 0xff);
      l = 3;
   }
   else if ( n < (one<<28) )
   {
      v[0] = 0xe0 | ((n>>24)&0x0f);
      v[1] = ((n>>16) & 0xff);
      v[2] = ((n>>8) & 0xff);
      v[3] = (n & 0xff);
      l = 4;
   }
#ifndef HAVE_64BIT_INT
   else
   {
      v[0] = 0xf0;
      v[1] = ((n>>24) & 0xff);
      v[2] = ((n>>16) & 0xff);
      v[3] = ((n>>8) & 0xff);
      v[4] = (n & 0xff);
      l = 5;
   }
#else
   else if ( n < (one<<35) ) /* possible for 64-bit integers */
   {
      v[0] = 0xf0 | ((n>>32)&0x07);
      v[1] = ((n>>24) & 0xff);
      v[2] = ((n>>16) & 0xff);
      v[3] = ((n>>8) & 0xff);
      v[4] = (n & 0xff);
      l = 5;
   }
   else if ( n < (one<<42) )
   {
      v[0] = 0xf8 | ((n>>40)&0x03);
      v[1] = ((n>>32) & 0xff);
      v[2] = ((n>>24) & 0xff);
      v[3] = ((n>>16) & 0xff);
      v[4] = ((n>>8) & 0xff);
      v[5] = (n & 0xff);
      l = 6;
   }
   else if ( n < (one<<49) )
   {
      v[0] = 0xfc | ((n>>48)&0x01);
      v[1] = ((n>>40) & 0xff);
      v[2] = ((n>>32) & 0xff);
      v[3] = ((n>>24) & 0xff);
      v[4] = ((n>>16) & 0xff);
      v[5] = ((n>>8) & 0xff);
      v[6] = (n & 0xff);
      l = 7;
   }
   else if ( n < (one<<56) )
   {
      v[0] = 0xfe;
      v[1] = ((n>>48) & 0xff);
      v[2] = ((n>>40) & 0xff);
      v[3] = ((n>>32) & 0xff);
      v[4] = ((n>>24) & 0xff);
      v[5] = ((n>>16) & 0xff);
      v[6] = ((n>>8) & 0xff);
      v[7] = (n & 0xff);
      l = 8;
   }
   else /* For n < 2^63 strictly but as long as we have no plans for */
   {    /* including numbers >= 2^64, it works up to 2^64-1. */
      v[0] = 0xff;
      v[1] = ((n>>56) & 0xff);
      v[2] = ((n>>48) & 0xff);
      v[3] = ((n>>40) & 0xff);
      v[4] = ((n>>32) & 0xff);
      v[5] = ((n>>24) & 0xff);
      v[6] = ((n>>16) & 0xff);
      v[7] = ((n>>8) & 0xff);
      v[8] = (n & 0xff);
      l = 9;
   }
#endif

   put_vector_of_byte(v,l,iobuf);
}

/* -------------------------- put_count32 ------------------------ */
/**
 *  @short Shortened version of put_count for up to 32 bits of data.
 *
 *  @return (none)
 */

void put_count32 (uint32_t n, IO_BUFFER *iobuf)
{
   BYTE v[5]; /* Prepared for up to 32 bits */
   uint32_t one = 1;
   int l = 0;
   if ( n < (one<<7) )
   {
      v[0] = (BYTE)n;
      l = 1;
   }
   else if ( n < (one<<14) )
   {
      v[0] = 0x80 | ((n>>8)&0x3f);
      v[1] = (n & 0xff);
      l = 2;
   }
   else if ( n < (one<<21) )
   {
      v[0] = 0xc0 | ((n>>16)&0x1f);
      v[1] = ((n>>8) & 0xff);
      v[2] = (n & 0xff);
      l = 3;
   }
   else if ( n < (one<<28) )
   {
      v[0] = 0xe0 | ((n>>24)&0x0f);
      v[1] = ((n>>16) & 0xff);
      v[2] = ((n>>8) & 0xff);
      v[3] = (n & 0xff);
      l = 4;
   }
   else
   {
      v[0] = 0xf0;
      v[1] = ((n>>24) & 0xff);
      v[2] = ((n>>16) & 0xff);
      v[3] = ((n>>8) & 0xff);
      v[4] = (n & 0xff);
      l = 5;
   }

   put_vector_of_byte(v,l,iobuf);
}

/* -------------------------- put_count16 ------------------------ */
/**
 *  @short Shortened version of put_count for up to 16 bits of data.
 *
 *  @return (none)
 */

void put_count16 (uint16_t n, IO_BUFFER *iobuf)
{
   BYTE v[3]; /* Prepared for up to 16 bits */
   uint16_t one = 1;
   int l = 0;
   if ( n < (one<<7) )
   {
      v[0] = (BYTE)n;
      l = 1;
   }
   else if ( n < (one<<14) )
   {
      v[0] = 0x80 | ((n>>8)&0x3f);
      v[1] = (n & 0xff);
      l = 2;
   }
   else
   {
      v[0] = 0xc0 | ((n>>16)&0x1f);
      v[1] = ((n>>8) & 0xff);
      v[2] = (n & 0xff);
      l = 3;
   }

   put_vector_of_byte(v,l,iobuf);
}

/* -------------------------- get_count ------------------------ */
/**
 *  @short Get an unsigned integer of unspecified length from an I/O buffer.
 *
 *  Get an unsigned integer of unspecified length from an I/O buffer
 *  where it is encoded in a way similar to the UTF-8 character encoding.
 *  Even though the scheme in principle allows for arbitrary length
 *  data, the current implementation is limited for data of up to 64 bits.
 *  On systems with @c uintmax_t shorter than 64 bits, the result
 *  could be clipped unnoticed. It could also be clipped unnoticed in
 *  the application calling this function.
 */

uintmax_t get_count (IO_BUFFER *iobuf)
{
   uintmax_t v[9]; /* Scheme implemented for 32 and 64 bit systems. */
   
   v[0] = get_byte(iobuf);
   
   if ( (v[0] & 0x80) == 0 )
      return v[0];
   v[1] = get_byte(iobuf);
   if ( (v[0] & 0xc0) == 0x80 )
      return ((v[0]&0x3f)<<8) | v[1];
   v[2] = get_byte(iobuf);
   if ( (v[0] & 0xe0) == 0xc0 )
      return ((v[0]&0x1f)<<16) | (v[1]<<8) | v[2];
   v[3] = get_byte(iobuf);
   if ( (v[0] & 0xf0) == 0xe0 )
      return ((v[0]&0x0f)<<24) | (v[1]<<16) | (v[2]<<8) | v[3];
   v[4] = get_byte(iobuf);
   if ( (v[0] & 0xf8) == 0xf0 )
      return ((v[0]&0x07)<<32) | (v[1]<<24) | (v[2]<<16) | (v[3]<<8) | v[4];
   /* With only 32-bit integers available, we may lose data from here on. */
#ifndef HAVE_64BIT_INT
   Warning("Data clipped to 32 bits in get_count function.");
#endif
   v[5] = get_byte(iobuf);
   if ( (v[0] & 0xfc) == 0xf8 )
      return ((v[0]&0x03)<<40) | (v[1]<<32) | (v[2]<<24) | (v[3]<<16) | (v[4]<<8) | v[5];
   v[6] = get_byte(iobuf);
   if ( (v[0] & 0xfe) == 0xfc )
      return ((v[0]&0x01)<<48) | (v[1]<<40) | (v[2]<<32) | (v[3]<<24) | 
             (v[4]<<16) | (v[5]<<8) | v[6];
   v[7] = get_byte(iobuf);
   if ( (v[0] & 0xff) == 0xfe )
      return (v[1]<<48) | (v[2]<<40) | (v[3]<<32) | (v[4]<<24) | 
             (v[5]<<16) | (v[6]<<8) | v[7];
   v[8] = get_byte(iobuf);
   return (v[1]<<56) | (v[2]<<48) | (v[3]<<40) | (v[4]<<32) | 
          (v[5]<<24) | (v[6]<<16) | (v[7]<<8) | v[8];
}

/* -------------------------- get_count32 ------------------------ */
/**
 *  @short Get an unsigned 32 bit integer of unspecified length from an I/O buffer.
 *
 *  Get an unsigned 32 bit integer of unspecified length from an I/O buffer
 *  where it is encoded in a way similar to the UTF-8 character encoding.
 *  This is a shorter version of get_count, for efficiency reasons.
 */

uint32_t get_count32 (IO_BUFFER *iobuf)
{
   uint32_t v[9];
   
   v[0] = get_byte(iobuf);
   
   if ( (v[0] & 0x80) == 0 )
      return v[0];
   v[1] = get_byte(iobuf);
   if ( (v[0] & 0xc0) == 0x80 )
      return ((v[0]&0x3f)<<8) | v[1];
   v[2] = get_byte(iobuf);
   if ( (v[0] & 0xe0) == 0xc0 )
      return ((v[0]&0x1f)<<16) | (v[1]<<8) | v[2];
   v[3] = get_byte(iobuf);
   if ( (v[0] & 0xf0) == 0xe0 )
      return ((v[0]&0x0f)<<24) | (v[1]<<16) | (v[2]<<8) | v[3];
   v[4] = get_byte(iobuf);
   if ( (v[0] & 0xf8) == 0xf0 )
   {
      if ( (v[0] & 0x07) != 0x00 )
         Warning("Data too large in get_count32 function, clipped.");
      return (v[1]<<24) | (v[2]<<16) | (v[3]<<8) | v[4];
   }
   /* With only 32-bit integers available, we may lose data from here on. */
   Warning("Data too large in get_count32 function.");
   v[5] = get_byte(iobuf);
   if ( (v[0] & 0xfc) == 0xf8 )
      return 0;
   v[6] = get_byte(iobuf);
   if ( (v[0] & 0xfe) == 0xfc )
      return 0;
   v[7] = get_byte(iobuf);
   if ( (v[0] & 0xff) == 0xfe )
      return 0;
   v[8] = get_byte(iobuf);
   return 0;
}

/* -------------------------- get_count16 ------------------------ */
/**
 *  @short Get an unsigned 16 bit integer of unspecified length from an I/O buffer.
 *
 *  Get an unsigned 16 bit integer of unspecified length from an I/O buffer
 *  where it is encoded in a way similar to the UTF-8 character encoding.
 *  This is a shorter version of get_count, for efficiency reasons.
 */

uint16_t get_count16 (IO_BUFFER *iobuf)
{
   int v[9]; /* 16 bit numbers encoded in a maximum of 3 bytes, everything else ignored */
   
   v[0] = get_byte(iobuf);
   
   if ( (v[0] & 0x80) == 0 )
      return v[0];
   v[1] = get_byte(iobuf);
   if ( (v[0] & 0xc0) == 0x80 )
      return ((v[0]&0x3f)<<8) | v[1];
   v[2] = get_byte(iobuf);
   if ( (v[0] & 0xe0) == 0xc0 )
   {
      if ( (v[0] & 0xf0) != 0xc0 ) /* Just one bit too much */
         Warning("Data too large in get_count16 function.");
      return ((v[0]&0x0f)<<16) | (v[1]<<8) | v[2];
   }
   /* Data written by the longer put_count... functions is unusable. 
      We just try to move by the correct number of bytes in the buffer. */
   Warning("Data too large in get_count16 function.");
   v[3] = get_byte(iobuf);
   if ( (v[0] & 0xf0) == 0xe0 )
      return 0;
   v[4] = get_byte(iobuf);
   if ( (v[0] & 0xf8) == 0xf0 )
      return 0;
   v[5] = get_byte(iobuf);
   if ( (v[0] & 0xfc) == 0xf8 )
      return 0;
   v[6] = get_byte(iobuf);
   if ( (v[0] & 0xfe) == 0xfc )
      return 0;
   v[7] = get_byte(iobuf);
   if ( (v[0] & 0xff) == 0xfe )
      return 0;
   v[8] = get_byte(iobuf);
   return 0;
}

/* -------------------------- put_scount ------------------------ */
/**
 *  @short Put a signed integer of unspecified length to an I/O buffer.
 *
 *  Put a signed integer of unspecified length in a way similar
 *  to the UTF-8 character encoding to an I/O buffer. The byte order
 *  resulting in the buffer is independent of the host byte order or the
 *  byte order in action for the I/O buffer, starting with as many
 *  leading bits in the first byte as extension bytes needed after 
 *  the first byte. While the scheme in principle allows for values
 *  of arbitrary length, the implementation is limited to 32 bits.
 *  To allow an efficient representation of negative numbers, the
 *  sign bit is stored in the least significant bit. Portability
 *  of data across machines with different intmax_t sizes and the
 *  need to represent also the most negative number (-(2^31), -(2^63),
 *  or -(2^127), depending on CPU type and compiler) is achieved by
 *  putting the number's modulus minus 1 into the higher bits.
 *
 *  @param  n       The number to be saved. It can be in the range
 *                  from -(2^63) to 2^63-1 on systems with 64 bit
 *                  integers (intrinsic or through the compiler) and
 *                  from -(2^31) to 2^31-1 on pure 32 bit systems.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void put_scount (intmax_t n, IO_BUFFER *iobuf)
{
   uintmax_t u;
   if ( n < 0 )
      u = (((intmax_t)(-(n+1)))<<1) | 1;
   else
      u = ((intmax_t)n) << 1;
   put_count(u,iobuf);
}

/* -------------------------- put_scount32 ------------------------ */
/**
 *  @short Shorter version of put_scount for up to 32 bytes of data.
 *
 *  Apart from efficiency, the data can be read with identical results
 *  through get_scount32 or get_scount.
 *
 *  @return (none)
 */

void put_scount32 (int32_t n, IO_BUFFER *iobuf)
{
   uint32_t u;
   if ( n < 0 )
      u = (((int32_t)(-(n+1)))<<1) | 1;
   else
      u = ((int32_t)n) << 1;
   put_count(u,iobuf);
}

/* -------------------------- put_scount16 ------------------------ */
/**
 *  @short Shorter version of put_scount for up to 16 bytes of data.
 *
 *  Apart from efficiency, the data can be read with identical results
 *  through get_scount16 or get_scount.
 *
 *  @return (none)
 */

void put_scount16 (int16_t n, IO_BUFFER *iobuf)
{
   uint16_t u;
   if ( n < 0 )
      u = (((int16_t)(-(n+1)))<<1) | 1;
   else
      u = ((int16_t)n) << 1;
   put_count(u,iobuf);
}

/* -------------------------- get_scount ------------------------ */
/**
 *  @short Get a signed integer of unspecified length from an I/O buffer.
 *
 *  Get a signed integer of unspecified length from an I/O buffer
 *  where it is encoded in a way similar to the UTF-8 character encoding.
 *  Even though the scheme in principle allows for arbitrary length
 *  data, the current implementation is limited for data of up to 64 bits.
 *  On systems with @c intmax_t shorter than 64 bits, the result
 *  could be clipped unnoticed.  
 */

intmax_t get_scount (IO_BUFFER *iobuf)
{
   uintmax_t u = get_count(iobuf);
   // u values of 0,1,2,3,4,... here correspond to signed values of
   //   0,-1,1,-2,2,... We have to test the least significant bit:
   if ( (u&1) == 1 ) // Negative number;
      return -((intmax_t)(u>>1)) - 1;
   else
      return (intmax_t) (u>>1);
}

/* -------------------------- get_scount16 ------------------------ */
/**
 *  @short Shortened version of get_scount for up to 16 bits of data.
 *
 */

int16_t get_scount16 (IO_BUFFER *iobuf)
{
   uint16_t u = get_count16(iobuf);
   if ( (u&1) == 1 ) // Negative number;
      return -((int16_t)(u>>1)) - (int16_t)1;
   else
      return (int16_t) (u>>1);
}

/* -------------------------- get_scount32 ------------------------ */
/**
 *  @short Shortened version of get_scount for up to 32 bits of data.
 *
 */

int32_t get_scount32 (IO_BUFFER *iobuf)
{
   uint32_t u = get_count32(iobuf);
   if ( (u&1) == 1 ) // Negative number;
      return -((int32_t)(u>>1)) - (int32_t)1;
   else
      return (int32_t) (u>>1);
}

/* ---------------- put_vector_of_int_scount ------------------- */
/**
 *  @short Put an array of ints as scount32 data into an I/O buffer.
 *
 */
void put_vector_of_int_scount (const int *vec, int num, IO_BUFFER *iobuf)
{
   int i;
   for (i=0; i<num; i++)
      put_scount32(vec[i],iobuf);
}

/* ---------------- get_vector_of_int_scount ------------------- */
/**
 *  @short Get an array of ints as scount32 data from an I/O buffer.
 *
 */
void get_vector_of_int_scount (int *vec, int num, IO_BUFFER *iobuf)
{
   int i;
   for (i=0; i<num; i++)
      vec[i] = get_scount32(iobuf);
}

/* -------------------------- put_short ------------------------ */
/**
 *  @short Put a two-byte integer on an I/O buffer.
 *
 *  Put a two-byte integer on an I/O buffer with least
 *  significant byte first. Should be machine independent
 *  as long as 'short' and 'unsigned short' are 16-bit integers,
 *  the two's complement is used for negative numbers, and
 *  the '>>' operator does a logical shift with unsigned short.
 *  Although the 'num' argument is a 4-byte integer on most
 *  machines, the value shoud be in the range -32768 to 32767.
 *
 *  @param  num     The number to be saved. Should fit into a
 *		    short integer and will be truncated otherwise.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void put_short(int num, IO_BUFFER *iobuf)
{
   union
   {
      int16_t sval;
      BYTE cval[2];
   } val[2];

   if ( (iobuf->w_remaining-=2) < 0 )
      if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
         return;

   if ( iobuf->byte_order == 0)
      val[1].sval = (int16_t) num;
   else
   {
      val[0].sval = (int16_t) num;
      val[1].cval[0] = val[0].cval[1];
      val[1].cval[1] = val[0].cval[0];
   }

   COPY_BYTES((void *)iobuf->data,(void *)&val[1].sval,(size_t)2);
   iobuf->data += 2;
}

/* ---------------------- put_vector_of_short --------------------- */
/**
 *  @short Put a vector of 2-byte integers on an I/O buffer.
 *
 *  Put a vector of 2-byte integers on an I/O buffer. This may be
 *  relaced by a more efficient but machine-dependent version later.
 *  May be called by a number of elements equal to 0. In this
 *  case, nothing is done.
 */

void put_vector_of_short (const short *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (short *) NULL )
   {
      for (i=0; i<num; i++)
         put_short(0,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_short((int)vec[i],iobuf);
}

/* ---------------------- put_vector_of_int --------------------- */
/**
 *  @short Put a vector of integers (range -32768 to 32767) into I/O buffer.
 *
 *  Put a vector of integers (with actual values in the range
 *  -32768 to 32767) into an I/O buffer. This may be relaced by a
 *  more efficient but machine-dependent version later.
 */

void put_vector_of_int (const int *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (int *) NULL )
   {
      for (i=0; i<num; i++)
         put_short(0,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_short(vec[i],iobuf);
}

/* ----------------- put_vector_of_uint16 ---------------------- */
/**
 *  @short Put a vector of unsigned shorts into an I/O buffer.
 *
 *  Put a vector of unsigned shorts into an I/O buffer with least
 *  significant byte first. The values are in the range 0 to 65535.
 *  The function should be used where sign propagation is of concern.
 *
 *  @param  uval    The vector of values to be saved.
 *  @param  num     The number of elements to save.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void put_vector_of_uint16 (const uint16_t *uval, int num, IO_BUFFER *iobuf)
{
   int i;
   union
   {
      uint16_t uval;
      BYTE cval[2];
   } val[2];

   for (i=0; i<num; i++)
   {
      if ( (iobuf->w_remaining-=2) < 0 )
	 if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
            return;

      if ( iobuf->byte_order == 0)
      {
	 COPY_BYTES((void *)iobuf->data,(const void *)(uval+i),(size_t)2);
      }
      else
      {
	 val[0].uval = uval[i];
	 val[1].cval[0] = val[0].cval[1];
	 val[1].cval[1] = val[0].cval[0];
         COPY_BYTES((void *)iobuf->data,(const void *)&val[1].uval,(size_t)2);
      }

      iobuf->data += 2;
   }
}

/* ----------------- get_vector_of_uint16 ---------------------- */
/**
 *  @short Get a vector of unsigned shorts from an I/O buffer.
 *
 *  Get a vector of unsigned shorts from an I/O buffer with least
 *  significant byte first. The values are in the range 0 to 65535.
 *  The function should be used where sign propagation is of concern.
 *
 *  @param  uval    The vector where the values should be loaded.
 *  @param  num     The number of elements to load.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void get_vector_of_uint16 (uint16_t *uval, int num, IO_BUFFER *iobuf)
{
   int i;
   union
   {
      uint16_t uval;
      BYTE cval[2];
   } val[2];

   for (i=0; i<num; i++)
   {
      if ( (iobuf->r_remaining-=2) < 0 )
         return;

      if ( iobuf->byte_order == 0)
      {
	 COPY_BYTES((void *)(uval+i),(void *)iobuf->data,(size_t)2);
      }
      else
      {
         COPY_BYTES((void *)&val[0].uval,(void *)iobuf->data,(size_t)2);
	 val[1].cval[0] = val[0].cval[1];
	 val[1].cval[1] = val[0].cval[0];
	 uval[i] = val[1].uval;
      }

      iobuf->data += 2;
   }

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* ------------------------- get_uint16 ---------------------- */
/**
 *  @short Get one unsigned short from an I/O buffer.
 *
 *  Get one unsigned short (16-bit unsigned int) from an I/O buffer.
 *  The function should be used where sign propagation is of concern.
 *
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return The value obtained from the I/O buffer.
 */

uint16_t get_uint16 (IO_BUFFER *iobuf)
{
   uint16_t uval;
   get_vector_of_uint16 (&uval,1,iobuf);
   return uval;
}

/* --------------------------- get_short ---------------------- */
/**
 *  @short Get a two-byte integer from an I/O buffer.
 *
 *  Get a two-byte integer with least significant byte
 *  first. Should be machine-independent (see put_short()).
 */

int get_short(IO_BUFFER *iobuf)
{

   int16_t num;

   union
   {
      int16_t sval;
      BYTE cval[2];
   } val[2];

   if ( (iobuf->r_remaining-=2) < 0 )
      return -1;

   if ( iobuf->byte_order == 0 )
      COPY_BYTES((void *) &num,(void *) iobuf->data,(size_t)2);
   else
   {
      COPY_BYTES((void *)&val[0].sval,(void *)iobuf->data,(size_t)2);
      val[1].cval[0] = val[0].cval[1];
      val[1].cval[1] = val[0].cval[0];
      num = val[1].sval;
   }

   iobuf->data += 2;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   /* Note that a sign propagation may happen here */
   return ((int) num);
}

/* -------------------- get_vector_of_short ------------------ */
/**
 *  Get a vector of short integers from I/O buffer.
 */

void get_vector_of_short (short *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (short *) NULL )
   {
      if ( (iobuf->r_remaining-=(2*num)) >= 0 )
         iobuf->data += 2*num;
      return;
   }

   for (i=0; i<num; i++)
      vec[i] = (short) get_short(iobuf);

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* --------------------- get_vector_of_int --------------------- */
/**
 *  Get a vector of (small) integers from I/O buffer.
 */

void get_vector_of_int (int *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (int *) NULL )
   {
      if ( (iobuf->r_remaining-=(2*num)) >= 0 )
         iobuf->data += 2*num;
      return;
   }

   for (i=0; i<num; i++)
      vec[i] = get_short(iobuf);

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* --------------------------- put_int32 ----------------------- */
/**
 *  @short Write a four-byte integer to an I/O buffer.
 *
 *  Write a four-byte integer with least significant bytes
 *  first. Should be machine independent (see put_short()).
 */

void put_int32(int32_t num, IO_BUFFER *iobuf)
{
   union
   {
      int32_t lval;
      BYTE cval[4];
   } val[2];
   int32_t ival;

   if ( (iobuf->w_remaining-=4) < 0 )
      if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
         return;

   if ( iobuf->byte_order == 0 )
   {
      ival = (int32_t) num;
      COPY_BYTES((void *)iobuf->data,(void *) &ival,(size_t)4);
   }
   else
   {
      val[0].lval = (int32_t) num;
      val[1].cval[0] = val[0].cval[3];
      val[1].cval[1] = val[0].cval[2];
      val[1].cval[2] = val[0].cval[1];
      val[1].cval[3] = val[0].cval[0];
      COPY_BYTES((void *)iobuf->data,(void *)&val[1].lval,(size_t)4);
   }

   iobuf->data += 4;
}

/* ---------------------- put_vector_of_int32 ------------------ */
/**
 *  Put a vector of 32 bit integers into I/O buffer.
 */

void put_vector_of_int32 (const int32_t *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (int32_t *) NULL )
   {
      for (i=0; i<num; i++)
         put_int32(0L,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_int32(vec[i],iobuf);
}

/* --------------------------- get_int32 ----------------------- */
/**
 *  @short Read a four byte integer from an I/O buffer.
 *  Read a four byte integer with little-endian or big-endian
 *  byte order from memory. Should be machine independent
 *  (see put_short()).
 *
 */

int32_t get_int32(IO_BUFFER *iobuf)
{
   int32_t num;

   union
   {
      int32_t lval;
      BYTE cval[4];
   } val[2];

   if ( (iobuf->r_remaining-=4) < 0 )
      return -1L;

   if ( iobuf->byte_order == 0 )
      COPY_BYTES((void *) &num,(void *)iobuf->data,(size_t)4);
   else
   {
      COPY_BYTES((void *)&val[0].lval,(void *)iobuf->data,(size_t)4);
      val[1].cval[0] = val[0].cval[3];
      val[1].cval[1] = val[0].cval[2];
      val[1].cval[2] = val[0].cval[1];
      val[1].cval[3] = val[0].cval[0];
      num = val[1].lval;
   }

   iobuf->data += 4;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return num;
}

/* ---------------------- get_vector_of_int32 ------------------ */
/**
 *  Get a vector of 32 bit integers from I/O buffer.
 */

void get_vector_of_int32 (int32_t *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   union
   {
      int32_t lval;
      BYTE cval[4];
   } val[2];

   if ( num <= 0 )
      return;

   if ( (iobuf->r_remaining-=4*num) < 0 )
      return;

   if ( vec == NULL )
   {
      iobuf->data += 4*num;
      return;
   }

   if ( iobuf->byte_order == 0 )
   {
      COPY_BYTES((void *) vec,(void *)iobuf->data,(size_t)(4*num));
      iobuf->data += 4*num;
   }
   else
   {
      for ( i=0; i<num; i++ )
      {
         COPY_BYTES((void *)&val[0].lval,(void *)iobuf->data,(size_t)4);
         val[1].cval[0] = val[0].cval[3];
         val[1].cval[1] = val[0].cval[2];
         val[1].cval[2] = val[0].cval[1];
         val[1].cval[3] = val[0].cval[0];
         vec[i] = val[1].lval;
         iobuf->data += 4;
      }
   }

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* --------------------------- put_int32 ----------------------- */
/**
 *  @short Put a four-byte integer into an I/O buffer.
 *
 *  Write a four-byte integer with least significant bytes
 *  first. Should be machine independent (see put_short()).
 */

void put_uint32(uint32_t num, IO_BUFFER *iobuf)
{
   union
   {
      uint32_t lval;
      BYTE cval[4];
   } val[2];
   uint32_t ival;

   if ( (iobuf->w_remaining-=4) < 0 )
      if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
         return;

   if ( iobuf->byte_order == 0 )
   {
      ival = (uint32_t) num;
      COPY_BYTES((void *)iobuf->data,(void *) &ival,(size_t)4);
   }
   else
   {
      val[0].lval = (uint32_t) num;
      val[1].cval[0] = val[0].cval[3];
      val[1].cval[1] = val[0].cval[2];
      val[1].cval[2] = val[0].cval[1];
      val[1].cval[3] = val[0].cval[0];
      COPY_BYTES((void *)iobuf->data,(void *)&val[1].lval,(size_t)4);
   }

   iobuf->data += 4;
}

/* ---------------------- put_vector_of_uint32 ------------------ */
/**
 *  Put a vector of 32 bit integers into I/O buffer.
 */

void put_vector_of_uint32 (const uint32_t *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (uint32_t *) NULL )
   {
      for (i=0; i<num; i++)
         put_uint32(0L,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_uint32(vec[i],iobuf);
}

/* --------------------------- get_uint32 ----------------------- */
/**
 *  @short Get a four-byte unsigned integer from an I/O buffer.
 *
 *  Read a four byte integer with little-endian or big-endian
 *  byte order from memory. Should be machine independent
 *  (see put_short()).
 *
 */

uint32_t get_uint32(IO_BUFFER *iobuf)
{
   uint32_t num;

   union
   {
      uint32_t lval;
      BYTE cval[4];
   } val[2];

   if ( (iobuf->r_remaining-=4) < 0 )
      return -1L; /* Compiler might complain about implicit conversion. */

   if ( iobuf->byte_order == 0 )
      COPY_BYTES((void *) &num,(void *)iobuf->data,(size_t)4);
   else
   {
      COPY_BYTES((void *)&val[0].lval,(void *)iobuf->data,(size_t)4);
      val[1].cval[0] = val[0].cval[3];
      val[1].cval[1] = val[0].cval[2];
      val[1].cval[2] = val[0].cval[1];
      val[1].cval[3] = val[0].cval[0];
      num = val[1].lval;
   }

   iobuf->data += 4;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return num;
}

/* ---------------------- get_vector_of_uint32 ------------------ */
/**
 *  Get a vector of 32 bit integers from I/O buffer.
 */

void get_vector_of_uint32 (uint32_t *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   union
   {
      uint32_t lval;
      BYTE cval[4];
   } val[2];

   if ( num <= 0 )
      return;

   if ( (iobuf->r_remaining-=4*num) < 0 )
      return;

   if ( vec == NULL )
   {
      iobuf->data += 4*num;
      return;
   }

   if ( iobuf->byte_order == 0 )
   {
      COPY_BYTES((void *) vec,(void *)iobuf->data,(size_t)(4*num));
      iobuf->data += 4*num;
   }
   else
   {
      for ( i=0; i<num; i++ )
      {
         COPY_BYTES((void *)&val[0].lval,(void *)iobuf->data,(size_t)4);
         val[1].cval[0] = val[0].cval[3];
         val[1].cval[1] = val[0].cval[2];
         val[1].cval[2] = val[0].cval[1];
         val[1].cval[3] = val[0].cval[0];
         vec[i] = val[1].lval;
         iobuf->data += 4;
      }
   }

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* --------------------------- put_long ----------------------- */
/**
 *  @short Put a four-byte integer taken from a 'long' into an I/O buffer.
 *
 *  Write a four-byte integer with least significant bytes
 *  first. Should be machine independent (see put_short()).
 */

void put_long(long num, IO_BUFFER *iobuf)
{
   union
   {
      int32_t lval;
      BYTE cval[4];
   } val[2];
   int32_t ival;

   if ( (iobuf->w_remaining-=4) < 0 )
      if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
         return;

   if ( iobuf->byte_order == 0 )
   {
      ival = (int32_t) num;
      COPY_BYTES((void *)iobuf->data,(void *) &ival,(size_t)4);
   }
   else
   {
      val[0].lval = (int32_t) num;
      val[1].cval[0] = val[0].cval[3];
      val[1].cval[1] = val[0].cval[2];
      val[1].cval[2] = val[0].cval[1];
      val[1].cval[3] = val[0].cval[0];
      COPY_BYTES((void *)iobuf->data,(void *)&val[1].lval,(size_t)4);
   }

   iobuf->data += 4;
}

/* ---------------------- put_vector_of_long ------------------ */
/**
 *  Put a vector of long int as 4-byte integers into an I/O buffer.
 */

void put_vector_of_long (const long *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   if ( vec == (long *) NULL )
   {
      for (i=0; i<num; i++)
         put_long(0L,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_long(vec[i],iobuf);
}

/* --------------------------- get_long ----------------------- */
/**
 *  @short Get 4-byte integer from I/O buffer and return as a long int.
 *
 *  Read a four byte integer with little-endian or big-endian
 *  byte order from memory. Should be machine independent
 *  (see put_short()).
 */

long get_long(IO_BUFFER *iobuf)
{
   int32_t num;

   union
   {
      int32_t lval;
      BYTE cval[4];
   } val[2];

   if ( (iobuf->r_remaining-=4) < 0 )
      return -1L;

   if ( iobuf->byte_order == 0 )
      COPY_BYTES((void *) &num,(void *)iobuf->data,(size_t)4);
   else
   {
      COPY_BYTES((void *)&val[0].lval,(void *)iobuf->data,(size_t)4);
      val[1].cval[0] = val[0].cval[3];
      val[1].cval[1] = val[0].cval[2];
      val[1].cval[2] = val[0].cval[1];
      val[1].cval[3] = val[0].cval[0];
      num = val[1].lval;
   }

   iobuf->data += 4;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   /* Note that for 64-bit machines a sign propagation may happen here */
   return (long) num;
}

/* ---------------------- get_vector_of_long ------------------ */
/**
 *  Get a vector of 4-byte integers as long int from I/O buffer.
 */

void get_vector_of_long (long *vec, int num, IO_BUFFER *iobuf)
{
   REGISTER int i;

   union
   {
      int32_t lval;
      BYTE cval[4];
   } val[2];

   if ( num <= 0 )
      return;

   if ( (iobuf->r_remaining-=4*num) < 0 )
      return;

   if ( vec == (long *) NULL )
   {
      iobuf->data += 4*num;
      return;
   }

   if ( iobuf->byte_order == 0 )
   {
#ifdef SIXTY_FOUR_BITS
      for ( i=0; i<num; i++ )
      {
         COPY_BYTES((void *)&val[1].lval,(void *)iobuf->data,(size_t)4);
          /* Note the possible sign propagation */
         vec[i] = (long) val[1].lval;
         iobuf->data += 4;
      }
#else
      COPY_BYTES((void *) vec,(void *)iobuf->data,(size_t)(4*num));
      iobuf->data += 4*num;
#endif
   }
   else
   {
      for ( i=0; i<num; i++ )
      {
         COPY_BYTES((void *)&val[0].lval,(void *)iobuf->data,(size_t)4);
         val[1].cval[0] = val[0].cval[3];
         val[1].cval[1] = val[0].cval[2];
         val[1].cval[2] = val[0].cval[1];
         val[1].cval[3] = val[0].cval[0];
         /* Note the possible sign propagation on 64-bit machines */
         vec[i] = (long) val[1].lval;
         iobuf->data += 4;
      }
   }

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

#ifdef HAVE_64BIT_INT

/* ----------------- put_vector_of_int64 ---------------------- */
/**
 *  @short Put a vector of signed 64-bit integers into an I/O buffer.
 *
 *  The function is only available where int64_t/uint64_t data
 *  types are implemented in the C compiler.
 *
 *  @param  uval    The vector of values to be saved.
 *  @param  num     The number of elements to save.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 *
 */

void put_vector_of_int64 (const int64_t *ival, int num, IO_BUFFER *iobuf)
{
   int i;
   union
   {
      int64_t ival;
      BYTE cval[8];
   } val[2];

   for (i=0; i<num; i++)
   {
      if ( (iobuf->w_remaining-=8) < 0 )
	 if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
            return;

      if ( iobuf->byte_order == 0)
      {
	 COPY_BYTES((void *)iobuf->data,(const void *)(ival+i),(size_t)8);
      }
      else
      {
	 val[0].ival = ival[i];
	 val[1].cval[0] = val[0].cval[7];
	 val[1].cval[1] = val[0].cval[6];
	 val[1].cval[2] = val[0].cval[5];
	 val[1].cval[3] = val[0].cval[4];
	 val[1].cval[4] = val[0].cval[3];
	 val[1].cval[5] = val[0].cval[2];
	 val[1].cval[6] = val[0].cval[1];
	 val[1].cval[7] = val[0].cval[0];
         COPY_BYTES((void *)iobuf->data,(const void *)&val[1].ival,(size_t)8);
      }

      iobuf->data += 8;
   }
}

/* ----------------- get_vector_of_int64 ---------------------- */
/**
 *  @short Get a vector of signed 64-bit integers from an I/O buffer.
 *
 *  The function is only available where int64_t/uint64_t data
 *  types are implemented in the C compiler.
 *
 *  @param  uval    The vector where the values should be loaded.
 *  @param  num     The number of elements to load.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void get_vector_of_int64 (int64_t *ival, int num, IO_BUFFER *iobuf)
{
   int i;
   union
   {
      int64_t ival;
      BYTE cval[8];
   } val[2];

   for (i=0; i<num; i++)
   {
      if ( (iobuf->r_remaining-=8) < 0 )
         return;

      if ( iobuf->byte_order == 0)
      {
	 COPY_BYTES((void *)(ival+i),(void *)iobuf->data,(size_t)8);
      }
      else
      {
         COPY_BYTES((void *)&val[0].ival,(void *)iobuf->data,(size_t)8);
	 val[1].cval[0] = val[0].cval[7];
	 val[1].cval[1] = val[0].cval[6];
	 val[1].cval[2] = val[0].cval[5];
	 val[1].cval[3] = val[0].cval[4];
	 val[1].cval[4] = val[0].cval[3];
	 val[1].cval[5] = val[0].cval[2];
	 val[1].cval[6] = val[0].cval[1];
	 val[1].cval[7] = val[0].cval[0];
	 ival[i] = val[1].ival;
      }

      iobuf->data += 8;
   }

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* ----------------- put_vector_of_uint64 ---------------------- */
/**
 *  @short Put a vector of unsigned 64-bit integers into an I/O buffer.
 * 
 *  The function is only available where int64_t/uint64_t data
 *  types are implemented in the C compiler.
 *
 *  @param  uval    The vector of values to be saved.
 *  @param  num     The number of elements to save.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void put_vector_of_uint64 (const uint64_t *uval, int num, IO_BUFFER *iobuf)
{
   int i;
   union
   {
      uint64_t uval;
      BYTE cval[8];
   } val[2];

   for (i=0; i<num; i++)
   {
      if ( (iobuf->w_remaining-=8) < 0 )
	 if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
            return;

      if ( iobuf->byte_order == 0)
      {
	 COPY_BYTES((void *)iobuf->data,(const void *)(uval+i),(size_t)8);
      }
      else
      {
	 val[0].uval = uval[i];
	 val[1].cval[0] = val[0].cval[7];
	 val[1].cval[1] = val[0].cval[6];
	 val[1].cval[2] = val[0].cval[5];
	 val[1].cval[3] = val[0].cval[4];
	 val[1].cval[4] = val[0].cval[3];
	 val[1].cval[5] = val[0].cval[2];
	 val[1].cval[6] = val[0].cval[1];
	 val[1].cval[7] = val[0].cval[0];
         COPY_BYTES((void *)iobuf->data,(const void *)&val[1].uval,(size_t)8);
      }

      iobuf->data += 8;
   }
}

/* ----------------- get_vector_of_uint64 ---------------------- */
/**
 *  @short Get a vector of unsigned 64-bit integers from an I/O buffer.
 *
 *  The function is only available where int64_t/uint64_t data
 *  types are implemented in the C compiler.
 *
 *  @param  uval    The vector where the values should be loaded.
 *  @param  num     The number of elements to load.
 *  @param  iobuf   The output buffer descriptor.
 *
 *  @return (none)
 */

void get_vector_of_uint64 (uint64_t *uval, int num, IO_BUFFER *iobuf)
{
   int i;
   union
   {
      uint64_t uval;
      BYTE cval[8];
   } val[2];

   for (i=0; i<num; i++)
   {
      if ( (iobuf->r_remaining-=8) < 0 )
         return;

      if ( iobuf->byte_order == 0)
      {
	 COPY_BYTES((void *)(uval+i),(void *)iobuf->data,(size_t)8);
      }
      else
      {
         COPY_BYTES((void *)&val[0].uval,(void *)iobuf->data,(size_t)8);
	 val[1].cval[0] = val[0].cval[7];
	 val[1].cval[1] = val[0].cval[6];
	 val[1].cval[2] = val[0].cval[5];
	 val[1].cval[3] = val[0].cval[4];
	 val[1].cval[4] = val[0].cval[3];
	 val[1].cval[5] = val[0].cval[2];
	 val[1].cval[6] = val[0].cval[1];
	 val[1].cval[7] = val[0].cval[0];
	 uval[i] = val[1].uval;
      }

      iobuf->data += 8;
   }
}

#endif

/* ------------------------ put_string --------------------- */
/**
 *  @short Put a string of ASCII characters into an I/O buffer.
 *
 *  Put a string of ASCII characters with leading count of
 *  bytes (stored with 16 bits) into an I/O buffer.
 *
 *  @param  s      The null-terminated ASCII string.
 *  @param  iobuf  The I/O buffer descriptor.
 *
 *  @return Length of string
 *
 */

int put_string (const char *s, IO_BUFFER *iobuf)
{
   int len = 0;

   if ( s == (char *) NULL )
      put_short(0,iobuf);
   else
   {
      len = (int) strlen(s);
      put_short(len,iobuf);
      put_vector_of_byte((const BYTE *) s,len,iobuf);
   }

   return len;
}

/* ----------------------- get_string ---------------------- */
/**
 *  @short Get a string of ASCII characters from an I/O buffer.
 *
 *  Get a string of ASCII characters with leading count of
 *  bytes (stored with 16 bits) from an I/O buffer.
 *
 *  NOTE: the nmax count does now account for the trailing zero
 *  byte which will be appended. This was different in an earlier
 *  version of this function where one additional byte had to
 *  be available for the trailing zero byte.
 *
 */

int get_string (char *s, int nmax, IO_BUFFER *iobuf)
{
   int nbytes, nread;

   nbytes = get_short(iobuf);
   nread = (nmax-1<nbytes) ? nmax-1 : nbytes; /* minimum of both */
   /* Read up to the accepted maximum length */
   get_vector_of_byte((BYTE *) s, nread, iobuf);
   /* Ignore the rest of the string */
   if ( nbytes > nread )
   {
      iobuf->r_remaining -= (nbytes-nread);
      iobuf->data += (nbytes-nread);
   }
   /* Terminate string with null character */
   s[nread] = '\0';

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return(nbytes);
}

/* ------------------------ put_long_string --------------------- */
/**
 *  @short Put a long string of ASCII characters into an I/O buffer.
 *
 *  Put a long string of ASCII characters with leading count of
 *  bytes into an I/O buffer. This is expected to work properly
 *  for strings of more than 32k only on machines with sizeof(int) > 2
 *  because 16-bit machines may not be able to represent lengths
 *  of long strings (as obtained with strlen).
 *
 *  @param  s      The null-terminated ASCII string.
 *  @param  iobuf  The I/O buffer descriptor.
 *
 *  @return Length of string
 *
 */

int put_long_string (const char *s, IO_BUFFER *iobuf)
{
   int32_t len = 0;

   if ( s == (char *) NULL )
      put_short(0,iobuf);
   else
   {
      len = (int32_t) strlen(s);
      put_int32(len,iobuf);
      put_vector_of_byte((const BYTE *) s,len,iobuf);
   }

   return (int) len;
}

/* ----------------------- get_long_string ---------------------- */
/**
 *  @short Get a long string of ASCII characters from an I/O buffer.
 *
 *  Get a long string of ASCII characters with leading count of
 *  bytes from an I/O buffer. Strings can be up to 2^31-1 bytes long
 *  (assuming you have so much memory).
 *
 *  To work properly with strings longer than 32k, a machine with
 *  sizeof(int) > 2 is actually required.
 *
 *  NOTE: the nmax count does account also for the trailing zero
 *  byte which will be appended.
 *
 */

int get_long_string (char *s, int nmax, IO_BUFFER *iobuf)
{
   int32_t nbytes, nread;

   nbytes = get_int32(iobuf);
   /* minimum of both */
   nread = (nmax-1<nbytes) ? nmax-1 : nbytes; 
   /* Read up to the accepted maximum length */
   get_vector_of_byte((BYTE *) s, nread, iobuf);
   /* Ignore the rest of the string */
   if ( nbytes > nread )
   {
      iobuf->r_remaining -= (nbytes-nread);
      iobuf->data += (nbytes-nread);
   }
   /* Terminate string with null character */
   s[nread] = '\0';

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return(nbytes);
}

/* ------------------------ put_var_string --------------------- */
/**
 *  @short Put a string of ASCII characters into an I/O buffer.
 *
 *  Put a string of ASCII characters with leading count of
 *  bytes (stored with variable length) into an I/O buffer.
 *  Note that storing strings of 32k or more length will not
 *  work on systems with sizeof(int)==2.
 *
 *  @param  s      The null-terminated ASCII string.
 *  @param  iobuf  The I/O buffer descriptor.
 *
 *  @return Length of string
 *
 */

int put_var_string (const char *s, IO_BUFFER *iobuf)
{
   int len = 0;

   if ( s == (char *) NULL )
      put_count(0,iobuf);
   else
   {
      len = (int) strlen(s);
      put_count(len,iobuf);
      put_vector_of_byte((const BYTE *) s,len,iobuf);
   }

   return len;
}

/* ----------------------- get_var_string ---------------------- */
/**
 *  @short Get a string of ASCII characters from an I/O buffer.
 *
 *  Get a string of ASCII characters with leading count of
 *  bytes (stored with variable length) from an I/O buffer.
 *
 *  NOTE: the nmax count does also account for the trailing zero
 *  byte which will be appended.
 *
 */

int get_var_string (char *s, int nmax, IO_BUFFER *iobuf)
{
   int nbytes, nread;

   nbytes = get_count(iobuf);
   nread = (nmax-1<nbytes) ? nmax-1 : nbytes; /* minimum of both */
   /* Read up to the accepted maximum length */
   get_vector_of_byte((BYTE *) s, nread, iobuf);
   /* Ignore the rest of the string */
   if ( nbytes > nread )
   {
      iobuf->r_remaining -= (nbytes-nread);
      iobuf->data += (nbytes-nread);
   }
   /* Terminate string with null character */
   s[nread] = '\0';

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return(nbytes);
}

/* ----------------------- put_real ------------------------ */
/**
 *  @short Put a 4-byte floating point number into an I/O buffer.
 *
 *  Put a 'double' (floating point) number in a
 *  specific but (almost) machine-independent format into
 *  an I/O buffer.
 *  Not the full precision of a 'double' is saved but
 *  a 32 bit IEEE floating point number is written (with the
 *  same byte ordering as long integers). On machines with
 *  other floating point format than IEEE the input number
 *  is converted to a IEEE number first. An optimized (machine-
 *  specific) version should compute the output data by shift and
 *  add operations rather than by log(), divide, and multiply
 *  operations on such non-IEEE-format machines (implemented
 *  for VAX only).
 *
 *  @param  dnum    The number to be put into the I/O buffer.
 *  @param  iobuf   The I/O buffer descriptor.
 *
 *  @return (none)
 *
 */

void put_real (double dnum, IO_BUFFER *iobuf)
{

#ifdef IEEE_FLOAT_FORMAT

   union
   {
      float fnum;
      int32_t lnum;
   } val;

   val.fnum = (float) dnum;
   put_int32(val.lnum,iobuf);

#else
# ifdef VAX_FLOAT_FORMAT

   union
   {
      float real;
      unsigned short words[2];
   } rswapit;
   union
   {
      long lword;
      unsigned short words[1];
   } lswapit;

   rswapit.real = (float) (dnum*0.25);
   lswapit.words[1] = rswapit.words[0];
   lswapit.words[0] = rswapit.words[1];
   put_long(lswapit.lword,iobuf);

# else

   unsigned long sign, exponent, mantissa, rnum;
   static double log2 = 0.6931471805599453;
   static double two23 = 8388608.;
   double tnum;

   if ( dnum==0. )
      sign = exponent = mantissa = 0;
   else
   {
      if ( dnum < 0 )
      {
         sign = 0x80000000L;
         dnum = -1.*dnum;
      }
      else
         sign = 0;
      /* Take care to avoid a factor of 2 effect due to tiny round-off */
      /* error with input numbers like 1.99999999 */
      exponent = (unsigned long) (127. + log(dnum)/log2);
      tnum = dnum/pow(2.,(double)exponent-127.);
      if ( (tnum-1.) * two23 + 0.499 >= two23 )
      {
         tnum /= 2;
         exponent += 1;
      }
      mantissa = ((unsigned long) ((tnum-1.) * two23 + 0.499)) & 0x007fffffL;
      exponent = (exponent&0xff) << 23;
   }

   rnum = sign|exponent|mantissa,iobuf;
   put_long(rnum,iobuf);

# endif
#endif

}

/* ------------------ put_vector_of_real ------------------ */
/**
 *  Put a vector of doubles as IEEE 'float' numbers into an I/O buffer.
 */

void put_vector_of_real (const double *dvec, int num, IO_BUFFER *iobuf)
{
   int i;

   if ( dvec == (double *) NULL )
   {
      for (i=0; i<num; i++)
         put_real(0.0,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_real(dvec[i],iobuf);
}

/* ------------------ put_vector_of_float ------------------ */
/**
 *  Put a vector of floats as IEEE 'float' numbers into an I/O buffer.
 */

void put_vector_of_float (const float *fvec, int num, IO_BUFFER *iobuf)
{
   int i;

   if ( fvec == (float *) NULL )
   {
      for (i=0; i<num; i++)
         put_real(0.0,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_real((double)fvec[i],iobuf);
}

/* --------------------- get_real ------------------------- */
/**
 *  @short Get a floating point number (as written by put_real) from the I/O buffer.
 *
 *  @param  iobuf   The I/O buffer descriptor;
 *
 *  @return  The floating point number.
 *
 */

double get_real (IO_BUFFER *iobuf)
{

#ifdef IEEE_FLOAT_FORMAT

   /* This is the simple way which can be used on most computers. */

   union
   {
      float fnum;
      int32_t lnum;
   } val;

   val.lnum = get_int32(iobuf);
# if ( defined(OS_LYNX) && defined(CPU_PowerPC) )
   /* Trick against compiler optimizer bug: */
   if ( val.fnum == 0. || val.lnum == 0 )
      return 0.;
# endif
#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
   return((double)val.fnum);

#else
# ifdef VAX_FLOAT_FORMAT

   /* On VAX computers it is a bit more complicated. */

   union
   {
      float real;
      uint16_t words[2];
   } rswapit;
   union
   {
      int32_t lword;
      uint16_t words[2];
   } lswapit;

   /* A value with sign bit ON and exponent==0 might cause a */
   /* 'reserved operand fault' and abnormal program termination on a VAX. */
   if ( ((lswapit.lword = get_long(iobuf)) & 0x0000FF80L) == 0x00001000L )
      lswapit.lword = 0;
   rswapit.words[1] = lswapit.words[0];
   rswapit.words[0] = lswapit.words[1];
#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
   return(4.0*(double)rswapit.real);

# else

   /* This way to obtain IEEE floating-point numbers can be used even */
   /* when the machine-dependent representation of floating-point numbers */
   /* is not known. */

   static double two23 = 8388608.;
   uint32_t rnum, exponent, mantissa;
   double dnum;

   rnum = (uint32_t) get_long(iobuf);
   exponent = (rnum & 0x7F800000L) >> 23;
   mantissa = (rnum & 0x007fffffL);

   dnum = ((double)mantissa/two23+1.)*pow(2.,(double)exponent-127.);
   if ( rnum & 0x80000000L )
      dnum *= -1;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif

   return(dnum);

# endif
#endif

}

/* ------------------- get_vector_of_real -------------------- */
/**
 *  Get a vector of floating point numbers as 'doubles' from an I/O buffer.
 */

void get_vector_of_real (double *dvec, int num, IO_BUFFER *iobuf)
{
   int i;

   if ( dvec == (double *) NULL )
   {
      if ( (iobuf->r_remaining-=(4*num)) >= 0 )
         iobuf->data += 4*num;
      return;
   }

   for (i=0; i<num; i++)
      dvec[i] = get_real(iobuf);
#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

/* ------------------- get_vector_of_float -------------------- */
/**
 *  Get a vector of floating point numbers as 'floats' from an I/O buffer.
 */

void get_vector_of_float (float *fvec, int num, IO_BUFFER *iobuf)
{
   int i;

   if ( fvec == (float *) NULL )
   {
      if ( (iobuf->r_remaining-=(4*num)) >= 0 )
         iobuf->data += 4*num;
      return;
   }

   for (i=0; i<num; i++)
      fvec[i] = (float) get_real(iobuf);
#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

#ifdef IEEE_FLOAT_FORMAT

/* ----------------------- put_double ------------------------ */
/**
 *  @short Put a 'double' as such into an I/O buffer.
 *
 *  Put a 'double' (floating point) number in a
 *  specific but (almost) machine-independent format into
 *  an I/O buffer.
 *  This implementation requires the machine to use IEEE
 *  double-precision floating point numbers. Only byte
 *  order conversion is done.
 *
 *  @param  dnum    The number to be put into the I/O buffer.
 *  @param  iobuf   The I/O buffer descriptor.
 *
 *  @return (none)
 */

void put_double (double dnum, IO_BUFFER *iobuf)
{

   union
   {
      double dnum;
      char cval[8];
   } val[2];

   val[0].dnum = dnum;

   if ( (iobuf->w_remaining-=8) < 0 )
      if ( extend_io_buffer(iobuf,256,IO_BUFFER_LENGTH_INCREMENT) < 0 )
         return;

   if ( iobuf->byte_order == 0 )
   {
      COPY_BYTES((void *) iobuf->data, (void *) val[0].cval, (size_t)8);
   }
   else
   {
      val[1].cval[0] = val[0].cval[7];
      val[1].cval[1] = val[0].cval[6];
      val[1].cval[2] = val[0].cval[5];
      val[1].cval[3] = val[0].cval[4];
      val[1].cval[4] = val[0].cval[3];
      val[1].cval[5] = val[0].cval[2];
      val[1].cval[6] = val[0].cval[1];
      val[1].cval[7] = val[0].cval[0];
      COPY_BYTES((void *) iobuf->data, (void *) val[1].cval, (size_t)8);
   }

   iobuf->data += 8;
}

/* ------------------ put_vector_of_double ------------------ */
/**
 *  @short Put a vector of doubles into an I/O buffer.
 *
 *  Put a vector of 'double' floating point numbers as IEEE
 *  'double' numbers into an I/O buffer.
 *
 */

void put_vector_of_double (const double *dvec, int num, IO_BUFFER *iobuf)
{
   int i;

   if ( dvec == (double *) NULL )
   {
      for (i=0; i<num; i++)
         put_double(0.0,iobuf);
      return;
   }

   for (i=0; i<num; i++)
      put_double(dvec[i],iobuf);
}

/* --------------------- get_double ------------------------- */
/**
 *  @short Get a double from the I/O buffer.
 *
 *  Get a double-precision floating point number (as written by
 *  put_double) from the I/O buffer.
 *  The current implementation is only for machines using
 *  IEEE format internally.
 *
 *  @param  iobuf  --  The I/O buffer descriptor;
 *
 *  @return  The floating point number.
 *
 */

double get_double (IO_BUFFER *iobuf)
{

   /* This is the simple way which can be used on most computers. */
   /* Requires that the internal floating-point representation is in */
   /* IEEE format and only byte-order conversions are needed. */

   union
   {
      double dnum;
      char cval[8];
   } val[2];

   if ( (iobuf->r_remaining-=8) < 0 )
      return -1L;

   if ( iobuf->byte_order == 0 )
      COPY_BYTES((void *) val[1].cval, (void *) iobuf->data, (size_t)8);
   else
   {
      COPY_BYTES((void *) val[0].cval, (void *) iobuf->data, (size_t)8);
      val[1].cval[0] = val[0].cval[7];
      val[1].cval[1] = val[0].cval[6];
      val[1].cval[2] = val[0].cval[5];
      val[1].cval[3] = val[0].cval[4];
      val[1].cval[4] = val[0].cval[3];
      val[1].cval[5] = val[0].cval[2];
      val[1].cval[6] = val[0].cval[1];
      val[1].cval[7] = val[0].cval[0];
   }

   iobuf->data += 8;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
   return val[1].dnum;
}

/* ------------------- get_vector_of_double -------------------- */
/**
 *  Get a vector of floating point numbers as 'doubles' from an I/O buffer.
 */

void get_vector_of_double (double *dvec, int num, IO_BUFFER *iobuf)
{
   int i;

   if ( dvec == (double *) NULL )
   {
      if ( (iobuf->r_remaining-=(4*num)) >= 0 )
         iobuf->data += 4*num;
      return;
   }

   for (i=0; i<num; i++)
      dvec[i] = get_double(iobuf);

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
}

#endif /* IEEE_FLOAT_FORMAT */

void fltp_to_sfloat (const float *fnum, uint16_t *snum);

/* ------------------------- dbl_to_sfloat ---------------------------- */
/** 
 *  @short Convert a double to the internal representation of a 16 bit
 *    floating point number as specified in the OpenGL 3.1 standard,
 *    also called a half-float.
 *    This is done via an intermediate float representation.
 *
 *  @param dnum The number to be converted.
 *  @param snum Pointer for the resulting representation, as stored in
 *              an unsigned 16-bit integer (1 bit sign, 5 bits exponent,
 *              10 bits mantissa).
 */

void dbl_to_sfloat (double dnum, uint16_t *snum)
{
   float fnum = dnum;
   fltp_to_sfloat (&fnum, snum);
   return;
}

/* ------------------------- fltp_to_sfloat ---------------------------- */
/** 
 *  @short Convert a float to the internal representation of a 16 bit
 *    floating point number as specified in the OpenGL 3.1 standard,
 *    also called a half-float.
 *    Both input and output come as pointers to avoid extra conversions.
 *
 *  @param fnum Pointer to the number to be converted.
 *  @param snum Pointer for the resulting representation, as stored in
 *              an unsigned 16-bit integer (1 bit sign, 5 bits exponent,
 *              10 bits mantissa).
 */

void fltp_to_sfloat (const float *fnum, uint16_t *snum)
{
#ifdef IEEE_FLOAT_FORMAT
   union
   {
      float fnum;
      uint32_t lnum;
   } val;
   int exponent_d;
   uint16_t sign = 0, exponent, mantissa;
   
   val.fnum = *fnum; /* Use float as an intermediate step */

   if ( (val.lnum & 0x80000000U) ) /* IEEE float sign bit test */
   {
      sign = 0x8000;
      // val.fnum *= -1.; /* Or perhaps better: val.lnum ^= 0x80000000U or &= 0x7fffffff */
   }

   if ( *fnum == 0. )
      exponent = mantissa = 0;
   else
   {
      uint32_t expflt = ((val.lnum & 0x7f800000U) >> 23); /* The float exponent */
      uint32_t mntflt = (val.lnum & 0x007fe000U); /* The relevant part of the float mantissa */
      exponent = (uint16_t) ((expflt - 127 + 15) & 0x1fU);
      exponent_d = (((int)expflt - 127 + 15));
      mantissa = (uint16_t) (mntflt >> 13);
      if ( expflt == 255 ) /* Infinity or NaN: */
         exponent = 31; /* Mantissa should take care which of those. */
      else if ( exponent_d <= 0 ) /* outside range, use zero or de-normalized */
      {
         exponent = 0;
         if ( exponent_d < -10 )
            mantissa = 0;
         else
            mantissa = (mantissa+1024) >> (1-exponent_d);
      }
      else if ( exponent_d > 31 ) /* convert to +- infinity */
      {
         exponent = 31;
         mantissa = 0;
      }
   }

   *snum = sign | (exponent << 10) | mantissa;

#else
   /* Thisa path will rarely get compiled, after the demise of the VAX ... */
   uint16_t sign = 0, exponent, mantissa;
   static double log2 = 0.6931471805599453;
   static double two10 = 1024.;
   double dnum = *fnum;

   /* Don't worry about NaN (E=31, M!=0) and +-Inf (E=31, M=0) yet. */

#if defined(_ISOC99_SOURCE) || ( defined(_XOPEN_SOURCE) && _XOPEN_SOURCE >= 600 )
   if ( signbit(dnum) )
#else
   if ( dnum < 0. ) /* Not recognizing '-0.' but who cares ...*/
#endif
   {
      sign = 0x8000;
      dnum *= -1.;
   }
   if ( dnum == 0. )
      exponent = mantissa = 0;
   else
   {
      double tnum;
      exponent = (uint16_t) (15.0 + log(dnum)/log2);
      tnum = dnum/pow(2.,(double)exponent-15.);

      mantissa = (uint16_t) ((unsigned) ((tnum-1.) * two10 + 0.4999) & 0x3ffU);
   }
   *snum = (uint16_t) (sign | ((exponent & 0x1fU) << 10) | (mantissa & 0x3ffU));
#endif
}

/* --------------------------- put_sfloat ----------------------------- */
/**
 * @short Put a 16-bit float to an I/O buffer.
 */

void put_sfloat (double dnum, IO_BUFFER *iobuf)
{
   uint16_t snum;
   dbl_to_sfloat(dnum,&snum);
   put_vector_of_uint16(&snum,1,iobuf);
}

/* ------------------------ dbl_from_sfloat --------------------------- */
/** 
 * @short Convert from the internal representation of an OpenGL 16-bit
 *     floating point number back to normal floating point representation.
 */

double dbl_from_sfloat (const uint16_t *snum)
{
   uint16_t sign = ((*snum) & 0x8000U) >> 15;
   uint16_t exponent = ((*snum) & 0x7c00U) >> 10;
   uint16_t mantissa = ((*snum) & 0x03ffU);
   double s = (sign == 0) ? 1. : -1.;
   if ( exponent == 0 ) /* De-normalized */
   {
      if ( mantissa == 0 )
         return s * 0.0;
      else
         return s * mantissa / (1024 * 16384);
   }
   else if ( exponent < 31 )
      return s * pow(2.,exponent-15.0) * (1. + mantissa/1024.);
#ifdef INF
   else if ( mantissa == 0 )
      return s * INF;
#endif
   else
#ifdef NAN
      return NAN;
#else
      return 0.;
#endif
}

/* --------------------------- get_sfloat ----------------------------- */
/**
 * @short Get a 16-bit float from an I/O buffer and expand it to a double.
 */

double get_sfloat (IO_BUFFER *iobuf)
{
   uint16_t snum = get_uint16(iobuf);
   return dbl_from_sfloat(&snum);
}

/* ------------------------ put_item_begin ------------------------ */
/**
 *  @short Begin putting another (sub-) item into the output buffer.
 *
 *  When putting another item to the output buffer which may
 *  be either a top item or a sub-item, put_item_begin()
 *  initializes the buffer (for a top item) and puts the item
 *  header on the buffer.
 *
 *  @param  iobuf	 The output buffer descriptor.
 *  @param  item_header  The item header descriptor.
 *
 *  @return  0 (O.k.)  or  -1 (error)
 *
 */

int put_item_begin (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   return put_item_begin_with_flags(iobuf, item_header, 0, 0);
}

/* -------------------- put_item_begin_with_flag -------------------- */
/**
 *  @short Begin putting another (sub-) item into the output buffer.
 *
 *  This is identical to put_item_begin() except for taking a third and
 *  fourth argument, a user flag to be included in the header data,
 *  and a flag indicating that the header extension should be used.
 *  In put_item_begin() these flags are forced to 0 (false) for
 *  backwards compatibility.
 *
 *  @param  iobuf	 The output buffer descriptor.
 *  @param  item_header  The item header descriptor.
 *  @param  flag         The user flag (0 or 1).
 *
 *  @return  0 (O.k.)  or  -1 (error)
 *
 */

int put_item_begin_with_flags (IO_BUFFER *iobuf, 
   IO_ITEM_HEADER *item_header, int user_flag, int extended)
{
   REGISTER int ilevel;
   unsigned long this_type;

   if ( iobuf == (IO_BUFFER *) NULL || item_header == (IO_ITEM_HEADER *) NULL )
      return -1;
   if ( !item_header->type )
      return -1;

   item_header->user_flag = (user_flag != 0);
   item_header->use_extension = 
         (extended != 0) || (iobuf->extended != 0);
   item_header->can_search = 0;
   item_header->length = 0;

   ilevel = item_header->level = iobuf->item_level;
   iobuf->r_remaining = -1;

   if ( ilevel >= MAX_IO_ITEM_LEVEL )
   {
      char msg[256];
      (void) sprintf(msg,
        "Maximum level of sub-items in I/O buffer exceeded for item type %ld",
        item_header->type);
      Warning(msg);
      item_header->level = MAX_IO_ITEM_LEVEL - 1;
      return -1;
   }

   /* When starting a top item, additional work has to be done. */
   if ( ilevel == 0 )
   {
      if ( iobuf->buffer == (BYTE *) NULL ||
           iobuf->buflen < 16 + (item_header->use_extension?4:0) )
         return -1;
      iobuf->data = iobuf->buffer;
      iobuf->w_remaining = iobuf->buflen;
      put_long((long)0xD41F8A37L,iobuf);
   }

   /* Make sure that at least the full header fits into the buffer. */
   if ( iobuf->w_remaining < 12 + (item_header->use_extension?4:0) )
   {
      if ( extend_io_buffer(iobuf,256,20) < 0 )
         return -1;
      if ( iobuf->w_remaining < 12 + (item_header->use_extension?4:0) )
         return -1;
   }

   this_type = (unsigned long) (item_header->type & 0x0000ffffL) |
               ((unsigned long) item_header->version << 20);
   if ( item_header->user_flag )
      this_type |= 0x00010000UL;
   if ( item_header->use_extension )
      this_type |= 0x00020000UL;
   put_long((long)this_type, iobuf);

   /* Write the identification number as supplied */
   put_long(item_header->ident,iobuf);

   /* The length of the item is stored at the next 4 bytes which cannot */
   /* be done immediately but only when the item or sub-item is completely */
   /* on the buffer. */
   iobuf->item_length[ilevel] = 0;
   put_long(0L,iobuf);

   /* If the extension field is foreseen from the beginning, reserve space for it now. */
   if ( item_header->use_extension )
      put_long(0L,iobuf);

   /* Add this item's header length to the sub-item lengths of superiour items. */
   if ( ilevel > 0 )
      iobuf->sub_item_length[ilevel-1] += 12 + (item_header->use_extension?4:0);

   /* Keep track where the data of this item starts. Because the actual */
   /* buffer might be dynamically reallocated and moved, only offsets with */
   /* respect to the beginning of the buffer are saved but no pointers. */
   iobuf->item_start_offset[ilevel] = /* item_header->start_of_data = */
      (long) (iobuf->data-iobuf->buffer);
   iobuf->sub_item_length[ilevel] = 0;
   iobuf->item_extension[ilevel] = item_header->use_extension;

   iobuf->item_level++;

   return 0;
}

/* ------------------------- put_item_end ------------------------- */
/**
 *  @short End of putting an item into the output buffer.
 *
 *  When finished with putting an item to the output buffer,
 *  check for errors and do housekeeping.
 *
 *  @param  iobuf         The output buffer descriptor.
 *  @param  item_header   The item header descriptor.
 *
 *  @return  0 (O.k.)  or  -1 (error)
 *
 */

int put_item_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   int rc;
   REGISTER int ilevel;
   long length, padding, j;
   BYTE *save_data_ptr;

   if ( item_header->level >= iobuf->item_level )
   {
      Warning("Attempt to finish putting an item which is no more active");
      return -1;
   }
   if ( iobuf->item_level <= 0 || item_header->level < 0 )
      return -1;

   /* There might be more than one item level having to be finished. */
   for ( ilevel = (--iobuf->item_level); ilevel>=item_header->level; ilevel-- )
   {
      uint32_t len1, len2, xbit = 0;
      /* The length of the item is the distance of the current write */
      /* position from the beginning of data of the item. */
      length = (long) (iobuf->data - iobuf->buffer) -
         iobuf->item_start_offset[ilevel];
      padding = (4 - length%4) % 4;
      for ( j=0; j<padding; j++ )
         put_byte(0,iobuf);
      length += padding;
      iobuf->item_length[ilevel] = length;
      item_header->length = (size_t) length;
      /* In order to keep track if an item can be searched for sub-items, */
      /* the sub-item length must be increased also by the no. of bytes padded. */
      iobuf->sub_item_length[ilevel] += padding;

      /* Add this item's length to the sub-item lengths of superiour items. */
      if ( ilevel > 0 )
         iobuf->sub_item_length[ilevel-1] += iobuf->item_length[ilevel];

      /* Now save the length field at its place right in front of the */
      /* beginning of data of that item. */
      save_data_ptr = iobuf->data;
      if ( iobuf->w_remaining < 0 )
         return -1;   /* Report error in case of buffer overflow */
      iobuf->w_remaining += 4 + (iobuf->item_extension[ilevel]?4:0);
      iobuf->data = iobuf->buffer +
         (iobuf->item_start_offset[ilevel] - 4 - 
         (iobuf->item_extension[ilevel]?4:0));
      len1 = (uint32_t)(((size_t)iobuf->item_length[ilevel]) & 0x3FFFFFFFUL);
      len2 = (uint32_t)(((size_t)iobuf->item_length[ilevel]) & ~0x3FFFFFFFUL) >> 30;
      if ( len2 > 4095 ) /* Only 12 bits for length in extension field so far. */
      {
         Warning("I/O block is too long for data format.");
         return -1;
      }
      if ( len2 != 0 )
      { 
         /* May also have been set before. */
         item_header->use_extension = 1;
         /* Do we need the extension field without having foreseen that ? */
         if ( iobuf->item_extension[ilevel] == 0 )
         {
            /* FIXME: Need to move all data by 4 bytes to make room for extension field. */
            Error("Unforeseen need to use the extension field is not fully implemented.");
            return -1;
            /* iobuf->item_extension[ilevel] = 1; */
         }
      }
      if ( iobuf->item_extension[ilevel] )
         xbit = (uint32_t) 0x80000000UL;
      /* If that item consists entirely of sub-items with known lengths */
      /* set bit 30 of the length field to mark that we can search for */
      /* sub-items in this item. */
      if ( iobuf->item_length[ilevel] == iobuf->sub_item_length[ilevel] )
         put_uint32(len1 | (uint32_t)0x40000000UL | xbit, iobuf);
      else
         put_uint32(len1 | xbit, iobuf);
      if ( xbit )
         put_uint32(len2 & (uint32_t)0x0FFFUL, iobuf); /* So far nothing except extended length here. */
      iobuf->data = save_data_ptr;
   }

   rc = 0;

   /* For a top-level item, write the whole data now. */
   if ( (iobuf->item_level=item_header->level) == 0 )
      rc = write_io_block(iobuf);

   return rc;
}

/* ------------------------- unput_item -------------------------- */
/**
 *  @short Undo writing at the present level.
 *
 *  When writing to an I/O buffer, revert anything yet written
 *  at the present level. If the buffer was extended, the last
 *  length is kept.
 *
 *  @param  iobuf        I/O buffer descriptor.
 *  @param  item_header  Header of item last read.
 *
 *  @return  0 (ok), -1 (error)
 *
 */

int unput_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   int old_level;

   /* Have we been in writing mode ? Or was it reading? */
   if ( iobuf->r_remaining != -1 )
      return -1;
   /* At what level of nesting was this item created ? */
   old_level = item_header->level;
   if ( old_level < 0 || old_level >= MAX_IO_ITEM_LEVEL ||
        old_level >= iobuf->item_level )
      return -1;
   /* Set the data pointer back to before the current item */
   if ( (iobuf->item_level = old_level) == 0 )
      iobuf->data = iobuf->buffer;
   else
      iobuf->data = iobuf->buffer +
         iobuf->item_start_offset[old_level] - 12 - 
         (item_header->use_extension ? 4 : 0);
   /* The rest of the buffer is available for writing */
   iobuf->w_remaining = iobuf->buflen -
      (long) (iobuf->data-iobuf->buffer);

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
   return 0;
}

/* ------------------------ get_item_begin ----------------------- */
/**
 *  @short Begin reading an item. Reads the header of an item.
 *
 *  Reads the header of an item. If a specific item
 *  type is requested but a different type is found and the
 *  length of that item is known, the item is skipped.
 *
 *  @param  iobuf	  The input buffer descriptor.
 *  @param  item_header   The item header descriptor.
 *
 *  @return 0 (O.k.),  -1 (error),  -2 (end-of-buffer)  or
 *		-3 (wrong item type).
 *
 */

int get_item_begin (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   int32_t sync_tag;
   unsigned long this_type;
   unsigned long wanted_type;
   REGISTER int ilevel;
   BYTE *previous_position;
   long previous_remaining;
   int previous_level, previous_order;
   size_t length, extension = 0;

   if ( iobuf == (IO_BUFFER *) NULL || item_header == (IO_ITEM_HEADER *) NULL )
      return -1;

   iobuf->w_remaining = -1;
   item_header->length = 0;

   previous_position = iobuf->data;
   previous_remaining = iobuf->r_remaining;
   previous_level = ilevel = iobuf->item_level;
   previous_order = iobuf->byte_order;

   if ( ilevel >= MAX_IO_ITEM_LEVEL )
   {
      Warning("Maximum level of sub-items in I/O Buffer exceeded");
      iobuf->r_remaining = -1;
   }

   /* Are we beyond the last sub-item? */
   if ( ilevel > 0 )
   {
      /* First check if we are already beyond the top item and then if we */
      /* will be beyond the next smaller level (superiour) item after */
      /* reading this item's header. */
      if ( (long) (iobuf->data-iobuf->buffer) >= 
               iobuf->item_length[0] + 16 + (iobuf->item_extension[0] ? 4 : 0) ||
           (long) (iobuf->data-iobuf->buffer) + 12 >=
           iobuf->item_start_offset[ilevel-1] + iobuf->item_length[ilevel-1] )
         return -2;
   }
   /* When starting a top item, additional work has to be done. */
   else if ( ilevel == 0 )
   {
      if ( iobuf->data_pending < 0 )
      {
         Warning("You must get an I/O block before you can read items");
         return -1;
      }
      /* Make sure that buffer memory has been allocated. */
      if ( iobuf->buffer == (BYTE *) NULL )
      {
         Warning("No memory allocated for I/O buffer");
         return -1;
      }
      iobuf->data = iobuf->buffer;
      iobuf->r_remaining = iobuf->buflen;

      /* Reset the byte-order flag and then check the byte order. */
      iobuf->byte_order = 0;
      sync_tag = get_long(iobuf);

#ifdef SIXTY_FOUR_BITS
      if ( sync_tag == (int32_t) 0xD41F8A37 )
         iobuf->byte_order = 0;
      else if ( sync_tag == (int32_t) 0x378A1FD4 )
         iobuf->byte_order = 1;
#else
      if ( sync_tag == (int32_t) 0xD41F8A37L )
         iobuf->byte_order = 0;
      else if ( sync_tag == (int32_t) 0x378A1FD4L )
         iobuf->byte_order = 1;
#endif
      else
      {
         Warning("Invalid byte ordering of input data");
         return -1;
      }
   }

   /* Remember the requested item type. */
   wanted_type = item_header->type;
   /* Extract the actual type and version from the 'type/version' field. */
   this_type = (unsigned long) get_long(iobuf);
   item_header->type = this_type & 0x0000ffffUL;
   item_header->version = (unsigned) (this_type >> 20) & 0xfff;
   if ( (item_header->version & 0x800) != 0 )
   {
      /* Encountering corrupted data seems more likely than having version numbers above 2047 */
      Warning("Version number invalid - may be corrupted data");
      return -1;
   }
   item_header->user_flag = ((this_type & 0x00010000UL) != 0);
   item_header->use_extension = ((this_type & 0x00020000UL) != 0);

   /* Extract the identification number */
   item_header->ident = get_long(iobuf);

   /* If bit 30 of length is set the item consists only of sub-items. */
   length = get_uint32(iobuf);
   if ( (length & 0x40000000UL) != 0 )
      item_header->can_search = 1;
   else
      item_header->can_search = 0;
   if ( (length & 0x80000000UL) != 0 )
   {
      item_header->use_extension = 1;
      /* Check again that we are not beyond the superior item after reading the extension */
      if ( ilevel > 0 &&
           (long) (iobuf->data-iobuf->buffer) + 16 >=
           iobuf->item_start_offset[ilevel-1] + iobuf->item_length[ilevel-1] )
         return -2;
      extension = get_uint32(iobuf);
      /* Actual length consists of bits 0-29 of length field plus bits 0-11 of extension field. */
      length = (length & 0x3FFFFFFFUL) | ((extension & 0x0FFFUL) << 30);
   }
   else
      length = (length & 0x3FFFFFFFUL);
   item_header->length = length;
   iobuf->item_length[ilevel] = (long) length;
   if ( item_header->can_search )
      iobuf->sub_item_length[ilevel] = (long) length;
   else
      iobuf->sub_item_length[ilevel] = 0;

   /* The present position in the I/O buffer is the start of item data. */
   iobuf->item_start_offset[ilevel] = (long) (iobuf->data - iobuf->buffer);

   /* For global offsets keep also track where header extensions were found. */
   iobuf->item_extension[ilevel] = item_header->use_extension;

   /* Only data up to the end of the top item may be read, not up to */
   /* end of the allocated I/O buffer memory. */
   if ( ilevel == 0 )
      iobuf->r_remaining = iobuf->item_length[0];

   /* If a specified item-type is wanted but a different one is */
   /* found, the previous state is restored. */
   if ( wanted_type > 0 && wanted_type != item_header->type )
   {
      iobuf->data = previous_position;
      iobuf->r_remaining = previous_remaining;
      iobuf->item_level = previous_level;
      iobuf->byte_order = previous_order;
      return -3;
   }

   /* The current item level is saved and then incremented. */
   item_header->level = iobuf->item_level++;

   if ( iobuf->r_remaining < 0 )
      return -1;

   return 0;
}

/* ------------------------ get_item_end ------------------------- */
/**
 *  @short End reading an item.
 *
 *  Finish reading an item. The pointer in the I/O buffer is at
 *  the end of the item after this call, if succesful.
 *
 *  @param  iobuf        I/O buffer descriptor.
 *  @param  item_header  Header of item last read.
 *
 *  @return 0 (ok), -1 (error)
 *
 */

int get_item_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   long length;
   REGISTER int ilevel;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
   if ( item_header->level != iobuf->item_level-1 )
   {
      if ( item_header->level >= iobuf->item_level )
      {
         Warning("Attempt to finish getting an item which is not active");
         return 0; /* This item level is (no more) active -- forget it */
      }
      else
         Warning("Item level is inconsistent");
   }
   if (item_header->level >= 0 && item_header->level <= MAX_IO_ITEM_LEVEL)
      ilevel = iobuf->item_level = item_header->level;
   else
      return -1;

   /* If the item has a length specified, check it. */
   if ( iobuf->item_length[ilevel] >= 0 )
      if ( iobuf->item_length[ilevel] !=
            (length = (long) (iobuf->data - iobuf->buffer) -
            iobuf->item_start_offset[ilevel]) )
      {
         if ( length > iobuf->item_length[ilevel] )
         {
	    char msg[256];
            (void) sprintf(msg,
               "Actual length of item type %lu exceeds specified length",
               item_header->type);
            Warning(msg);
         }
         iobuf->data = iobuf->buffer +
            (iobuf->item_start_offset[ilevel] + iobuf->item_length[ilevel]);
      }

   /* After having finished a top item no further data may be read. */
   if ( iobuf->item_level == 0 )
      iobuf->r_remaining = -1L;
   else
      iobuf->r_remaining = iobuf->item_length[0] + 
         16 + (iobuf->item_extension[0]?4:0) -
          (long) (iobuf->data - iobuf->buffer);
   iobuf->w_remaining = 0;

   return 0;
}

/* ------------------------- unget_item -------------------------- */
/**
 *  @short Go back to the beginning of an item being read.
 *
 *  When reading from an I/O buffer, go back to the beginning of
 *  an item (more precisely: its header) currently being read.
 *
 *  @param  iobuf        I/O buffer descriptor.
 *  @param  item_header  Header of item last read.
 *
 *  @return 0 (ok), -1 (error)
 *
 */

int unget_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   int old_level;

   old_level = item_header->level;
   if ( old_level < 0 || old_level >= MAX_IO_ITEM_LEVEL ||
        old_level >= iobuf->item_level )
      return -1;
   if ( (iobuf->item_level = old_level) == 0 )
      iobuf->data = iobuf->buffer;
   else
      iobuf->data = iobuf->buffer +
         iobuf->item_start_offset[old_level] - 12 - 
         (iobuf->item_extension[old_level]?4:0);
   iobuf->r_remaining = iobuf->item_length[0] + 
      16 + (iobuf->item_extension[0]?4:0) -
      (long) (iobuf->data-iobuf->buffer);
   iobuf->w_remaining = -1;

#ifdef BUG_CHECK
   bug_check(iobuf);
#endif
   return 0;
}

/* ------------------------ next_subitem_type ----------------------- */
/**
 *  Reads the header of a sub-item and return the type of it.
 *
 *  @param  iobuf  The input buffer descriptor.
 *
 *  @return  >= 0 (O.k.),  -1 (error),  -2 (end-of-buffer).
 */

int next_subitem_type (IO_BUFFER *iobuf)
{
   int this_type;
   REGISTER int ilevel;
   BYTE *previous_position;
   long previous_remaining;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;

   previous_position = iobuf->data;
   previous_remaining= iobuf->r_remaining;
   ilevel = iobuf->item_level;

   if ( ilevel >= MAX_IO_ITEM_LEVEL )
   {
      Warning("Maximum level of sub-items in I/O Buffer exceeded");
      iobuf->r_remaining = -1;
   }

   /* Are we beyond the last sub-item? */
   if ( ilevel > 0 )
   {
      /* First check if we are already beyond the top item and then if we */
      /* will be beyond the next smaller level (superiour) item after */
      /* reading this item's header. */
      if ( (long) (iobuf->data-iobuf->buffer) >= 
               iobuf->item_length[0] + 16 + (iobuf->item_extension[0]?4:0) ||
           (long) (iobuf->data-iobuf->buffer) + 12 >=
           iobuf->item_start_offset[ilevel-1] + iobuf->item_length[ilevel-1] )
         return -2;
   }
   /* Not for top-level items */
   else if ( ilevel == 0 )
      return -1;

   /* Extract the actual type and version from the 'type/version' field. */
   this_type = (int) ((unsigned long) get_long(iobuf)) & 0x0000ffffL;
   iobuf->data = previous_position;
   iobuf->r_remaining = previous_remaining;

   return this_type;
}

/* ------------------------ next_subitem_length ----------------------- */
/**
 *  Reads the header of a sub-item and return the length of it.
 *
 *  @param  iobuf  The input buffer descriptor.
 *
 *  @return  >= 0 (O.k.),  -1 (error),  -2 (end-of-buffer).
 */

long next_subitem_length (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int rc;
   long len;
   BYTE *previous_position;
   long previous_remaining;
   
   previous_position = iobuf->data;
   previous_remaining= iobuf->r_remaining;

   item_header.type = 0;
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      len = rc;
   else
   {
      len = iobuf->item_length[iobuf->item_level-1];
      if ( (rc = unget_item(iobuf,&item_header)) < 0 )
         len = rc;
   }

   iobuf->data = previous_position;
   iobuf->r_remaining = previous_remaining;

   return len;
}

/* ------------------------ next_subitem_ident ----------------------- */
/**
 *  Reads the header of a sub-item and return the identifier of it.
 *
 *  @param  iobuf  The input buffer descriptor.
 *
 *  @return  >= 0 (O.k.),  -1 (error),  -2 (end-of-buffer).
 */

long next_subitem_ident (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int rc;
   long id;
   BYTE *previous_position;
   long previous_remaining;
   
   previous_position = iobuf->data;
   previous_remaining= iobuf->r_remaining;

   item_header.type = 0;
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      id = rc;
   else
   {
      id = item_header.ident;
      if ( (rc = unget_item(iobuf,&item_header)) < 0 )
         id = rc;
   }

   iobuf->data = previous_position;
   iobuf->r_remaining = previous_remaining;
   
   return id;
}

/* ----------------------- skip_subitem ----------------------------- */
/**
 *  When the next sub-item is of no interest, it can be skipped. 
 *
 *  @param  iobuf        I/O buffer descriptor.
 *
 *  @return  0 (ok), -1 (error)
 *
 */

int skip_subitem (IO_BUFFER *iobuf)
{
   IO_ITEM_HEADER item_header;
   int rc;
   
   item_header.type = 0;
   if ( (rc = get_item_begin(iobuf,&item_header)) < 0 )
      return rc;
   return get_item_end(iobuf,&item_header);
}

/* ---------------------- search_sub_item ------------------------ */
/**
 *  @short Search for an item of a specified type.
 *
 *  Search for an item of a specified type, starting at the current
 *  position in the I/O buffer. After successful action the
 *  buffer data pointer points to the beginning of the header
 *  of the first item of that type. If no such item is found,
 *  it points right after the end of the item of
 *  the next higher level.
 *
 *  @param  iobuf  The I/O buffer descriptor.
 *  @param  item_header The header of the item within which we search.
 *  @param  sub_item_header To be filled with what we found.
 *
 *  @return  0 (O.k., sub-item was found),
 *	    -1 (error),
 *	    -2 (no such sub-item),
 *	    -3 (cannot skip sub-items),
 *
 */

int search_sub_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header, 
   IO_ITEM_HEADER *sub_item_header)
{
   int rc;
   unsigned long type;
   int old_level;

   if ( !item_header->can_search )
      return -3;
   old_level = item_header->level;
   if ( old_level < 0 || old_level >= MAX_IO_ITEM_LEVEL )
      return -1;
   if ( iobuf->item_level < 1 || iobuf->item_level >= MAX_IO_ITEM_LEVEL )
      return -1;
   sub_item_header->level = old_level + 1;
   type = sub_item_header->type;

   while ( iobuf->r_remaining > 0 )
   {
      sub_item_header->type = 0;
      rc = get_item_begin(iobuf,sub_item_header);
      if ( rc == -1 || rc == -2 )
      {
         iobuf->item_level = old_level + 1;
         return(rc); /* Error or end of the item. */
      }
      if ( sub_item_header->type == type || type <= 0 )
      {
         /* Similar to unget_item() we set the data pointing back
            to the beginning of the (sub-)block. */
         iobuf->data = iobuf->buffer +
            iobuf->item_start_offset[iobuf->item_level-1] - 
               12 - (iobuf->item_extension[iobuf->item_level-1]?4:0);
         iobuf->r_remaining = iobuf->item_length[0] + 16 + 
            (iobuf->item_extension[0]?4:0) -
            (long) (iobuf->data-iobuf->buffer);
         iobuf->w_remaining = -1;
         /* That is different from unget_item(): */
         iobuf->item_level = old_level + 1;
         return 0;   /* This is the right type of item. */
      }
      if ( iobuf->item_length[iobuf->item_level-1] == -1L )
         return -3;  /* Cannot skip because length is unknown. */
      get_item_end(iobuf,sub_item_header);
   }

   return -2;
}

/* ---------------------- rewind_item ----------------------- */
/**
 *  @short Go back to the beginning of an item.
 *  When reading from an I/O buffer, go back to the beginning
 *  of the data area of an item. This is typically used when
 *  searching for different types of sub-blocks but processing
 *  should not depend on the relative order of them.
 *
 *  @param  iobuf        I/O buffer descriptor.
 *  @param  item_header  Header of item last read.
 *
 *  @return  0 (ok), -1 (error)
 */

int rewind_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   int old_level;

   old_level = item_header->level;
   if ( old_level < 0 || old_level >= MAX_IO_ITEM_LEVEL ||
        old_level >= iobuf->item_level )
      return -1;
   iobuf->item_level = old_level + 1;
   iobuf->data = iobuf->buffer + iobuf->item_start_offset[old_level];
   iobuf->r_remaining = iobuf->item_length[0] + 16 +
      (iobuf->item_extension[0]?4:0) -
      (long) (iobuf->data-iobuf->buffer);
   iobuf->w_remaining = -1;

   return 0;
}

/* -------------------- remove_item ----------------------- */
/**
 *  @short Remove an item from an I/O buffer.
 *
 *  If writing an item has already started and then some
 *  condition was found to remove the item again, this is
 *  the function for it. The item to be removed should be the
 *  last one written, since anything following it will be forgotten too.
 *
 *  @param  iobuf        I/O buffer descriptor.
 *  @param  item_header  Header of item to be removed.
 *
 *  @return  0 (ok), -1 (error)
 */

int remove_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   REGISTER int ilevel, jlevel;
   BYTE *save_data_ptr;
   long old_length;

   if ( (ilevel = (--iobuf->item_level)) < 0 )
      return -1;
   if ( iobuf->item_length[ilevel] < 0 || iobuf->item_length[ilevel] +
        iobuf->item_start_offset[ilevel]
        > iobuf->buflen )
      return -1;

   old_length = iobuf->item_length[0] + 16 + (iobuf->item_extension[0]?4:0);

   for (jlevel=ilevel-1; jlevel>=0; jlevel--)
   {
      long lenx = iobuf->item_length[ilevel] + 12 + (iobuf->item_extension[ilevel]?4:0);
      uint32_t len1, len2, xbit = 0;
      if (iobuf->item_length[jlevel] >= iobuf->item_length[ilevel] )
         iobuf->item_length[jlevel] -= lenx;
      else
         iobuf->item_length[jlevel] = 0;
      iobuf->sub_item_length[jlevel] -= lenx;

      /* Now save the length field at its place right in front of the */
      /* beginning of data of that item. */
      save_data_ptr = iobuf->data;
      iobuf->w_remaining = 4 + (iobuf->item_extension[jlevel]?4:0);
      iobuf->data = iobuf->buffer + (iobuf->item_start_offset[jlevel] - 4 -
          (iobuf->item_extension[jlevel]?4:0));
      len1 = (uint32_t)(((size_t)iobuf->item_length[jlevel]) & 0x3FFFFFFFUL);
      len2 = (uint32_t)(((size_t)iobuf->item_length[jlevel]) & ~0x3FFFFFFFUL) >> 30;
      if ( len2 > 4095 ) /* Only 12 bits for length in extension field so far. */
      {
         Warning("I/O block is too long for data format.");
         return -1;
      }
      if ( len2 != 0 )
      { 
         /* May also have been set before. */
         item_header->use_extension = 1;
         /* Do we need the extension field without having foreseen that ? */
         if ( iobuf->item_extension[jlevel] == 0 )
         {
            /* FIXME: Need to move all data by 4 bytes to make room for extension field. */
            Error("Unforeseen need to use the extension field is not fully implemented.");
            return -1;
            /* iobuf->item_extension[jlevel] = 1; */
         }
      }
      if ( iobuf->item_extension[jlevel] )
         xbit = (uint32_t) 0x80000000UL;
      /* If that item consists entirely of sub-items with known lengths */
      /* set bit 30 of the length field to mark that we can search for */
      /* sub-items in this item. */
      if ( iobuf->item_length[jlevel] == iobuf->sub_item_length[jlevel] )
         put_uint32(len1 | 0x40000000UL | xbit, iobuf);
      else
         put_uint32(len1 | xbit, iobuf);
      if ( xbit )
         put_uint32(len2 & (uint32_t)0x0FFFUL, iobuf); /* So far nothing except extended length here. */
      iobuf->data = save_data_ptr;
   }

   if ( iobuf->item_length[0] > 0 )
   {
      memmove((void *)(iobuf->buffer + iobuf->item_start_offset[ilevel] - 
         12 - (iobuf->item_extension[ilevel]?4:0)),
         (void *)(iobuf->buffer + iobuf->item_start_offset[ilevel] +
         iobuf->item_length[ilevel]), (size_t)(old_length -
         (iobuf->item_start_offset[ilevel] + iobuf->item_length[ilevel])));
   }

   iobuf->data = iobuf->buffer + /* item_header->start_of_data */
      iobuf->item_start_offset[ilevel] - 4;
   item_header->type = 0;
   iobuf->item_length[ilevel] = 0;
   iobuf->r_remaining = iobuf->item_length[0] + 16 + (iobuf->item_extension[0]?4:0) -
        (long) (iobuf->data - iobuf->buffer);
   iobuf->w_remaining = -1;

   return 0;
}

/* -------------------- list_sub_items --------------------------- */
/**
 *  @short Display the contents of sub-items on standard output.
 *
 *  Display the contents (item types, versions, idents and lengths)
 *  of sub-items on standard output.
 *
 *  @param  iobuf        I/O buffer descriptor.
 *  @param  item_header  Header of the item from which to show contents.
 *  @param  maxlevel     The maximum nesting depth to show contents
 *                       (counted from the top-level item on).
 *  @param  verbosity    Try showing type name at >=1, description at >=2.
 *
 *  @return  0 (ok), -1 (error)
 */

int list_sub_items (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header, 
   int maxlevel, int verbosity)
{
   IO_ITEM_HEADER sub_item_header;
   int rc=-9, i;
   char msg[512];

   if ( iobuf->item_level == 0 )
   {
      item_header->type = 0;
      get_item_begin(iobuf,item_header);
      if ( iobuf->item_extension[0] )
         sprintf(msg,"\nType %4lu, version %u, extended length %ld",
            item_header->type, item_header->version, iobuf->item_length[0]);
      else
         sprintf(msg,"\nType %4lu, version %u, length %ld",
            item_header->type, item_header->version, iobuf->item_length[0]);
      Output(msg);
      if ( item_header->ident >= 0 )
      {
         sprintf(msg," (%sid %ld = 0x%lx)",
            item_header->user_flag?"*":"",
	    item_header->ident,item_header->ident);
         Output(msg);
      }
      if ( verbosity >= 1 )
      {
         const char *name = eventio_registered_typename(item_header->type);
         const char *description = "";
         if ( verbosity >= 2 )
            description = eventio_registered_description(item_header->type);
         if ( name != NULL && *name != '\0' )
         {
            snprintf(msg,sizeof(msg)-1,"\t[%s] %s", name, description);
            Output(msg);
         }
      }
      Output("\n");
   }

   if ( maxlevel >= 0 && maxlevel < iobuf->item_level )
      return 0;
   if ( (long) (iobuf->data-iobuf->buffer) >= iobuf->item_length[0] + 12 )
     return 0;

   sub_item_header.type = 0;
   iobuf->data = iobuf->buffer +
       iobuf->item_start_offset[iobuf->item_level-1];
   while ( (rc=search_sub_item(iobuf,item_header,&sub_item_header)) == 0 )
   {
      get_item_begin(iobuf,&sub_item_header);
      for (i=0; i<iobuf->item_level-1; i++)
         Output("  ");
      if ( sub_item_header.use_extension )
         sprintf(msg,"Type %4lu, version %u, extended length %ld",
            sub_item_header.type, sub_item_header.version,
            iobuf->item_length[iobuf->item_level-1]);
      else
         sprintf(msg,"Type %4lu, version %u, length %ld",
            sub_item_header.type, sub_item_header.version,
            iobuf->item_length[iobuf->item_level-1]);
      Output(msg);
      if ( sub_item_header.ident >= 0 )
      {
         sprintf(msg," (%sid %ld = 0x%lx)",
            sub_item_header.user_flag?"*":"",
            sub_item_header.ident, sub_item_header.ident);
         Output(msg);
      }
      if ( verbosity >= 1 )
      {
         const char *name = eventio_registered_typename(sub_item_header.type);
         const char *description = "";
         if ( verbosity >= 2 )
            description = eventio_registered_description(sub_item_header.type);
         if ( name != NULL && *name != '\0' )
         {
            snprintf(msg,sizeof(msg)-1,"\t[%s] %s", name, description);
            Output(msg);
         }
      }
      Output("\n");
      if ( sub_item_header.can_search )
         if ( (i=list_sub_items(iobuf,&sub_item_header,maxlevel,verbosity)) < 0 )
         {
            sprintf(msg,"  (rc=%d)\n",i);
            Output(msg);
         }
      get_item_end(iobuf,&sub_item_header);
      sub_item_header.type = 0;
   }
   if ( iobuf->item_level == 0 )
      Output("\n");

   if ( item_header->level == 0 )
      get_item_end(iobuf,item_header);
   if ( rc == -2 )
      return 0;
   else
      return rc;
}

/* ----------------------- reset_io_block ------------------------ */
/**
 *  Reset an I/O block to its empty status.
 *
 *  @param  iobuf  The I/O buffer descriptor.
 *
 *  @return  0 (O.k.),  -1 (error)
 *
 */

int reset_io_block (IO_BUFFER *iobuf)
{
   BYTE *tptr;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
   iobuf->w_remaining = iobuf->r_remaining = -1L;
   iobuf->item_level = 0;
   iobuf->item_length[0] = iobuf->sub_item_length[0] = 0;
   iobuf->item_extension[0] = 0;
   iobuf->data_pending = -1;
   iobuf->data = iobuf->buffer;
   if ( iobuf->buflen != iobuf->min_length )
   {
      tptr = (BYTE *) realloc((void *)iobuf->buffer,
          (size_t)iobuf->min_length);
      if ( tptr != (BYTE *) NULL )
      {
         iobuf->buffer = tptr;
         iobuf->buflen = iobuf->min_length;
      }
   }
   iobuf->data = iobuf->buffer;
   iobuf->regular = 0;
   /* Note: iobuf->extended mode is not reset */

   return 0;
}

/* ----------------------- write_io_block ------------------------ */
/**
 *  @short Write an I/O block to the block's output.
 *
 *  The complete I/O block is written to the output destination,
 *  which can be raw I/O (through write), buffered I/O (through
 *  fwrite) or user-defined I/O (through a user funtion).
 *  All items must have been closed before.
 *
 *  @param  iobuf  The I/O buffer descriptor.
 *
 *  @return  0 (O.k.),  -1 (error),  -2 (item has no data)
 *
 */

int write_io_block (IO_BUFFER *iobuf)
{
   int rc;
   size_t length;

   if ( iobuf == (IO_BUFFER *) NULL )
      return -1;
#ifdef OS_MSDOS
   if ( (long) (iobuf->data-iobuf->buffer) > 32767 )
   {
      Warning("Cannot write item of length exceeding 32767 bytes");
      return -1;
   }
#endif
   /* length = (int) (iobuf->data-iobuf->buffer); */
   /* Empty top items are not written */
   /* if ( length < 16 ) */
   /*    return -2;      */
   length = 16 + (iobuf->item_extension[0]?4:0) + iobuf->item_length[0];

   rc = 0;
   if ( iobuf->item_level > 0 )
   {
      Warning("Output cancelled because item level is not 0");
      rc = -1;
   }
   else if ( iobuf->item_length[0] < 0L )
   {
      Warning("Output cancelled due to invalid length of top item");
      rc = -1;
   }
   else if ( iobuf->output_fileno >= 0 )
   {
      if (write(iobuf->output_fileno,(char *)iobuf->buffer,(size_t)length) == -1 )
      {
         Warning("Output error for I/O buffer");
         rc = -1;
      }
   }
   else if ( iobuf->output_file != (FILE *) NULL )
   {
      if ( fwrite((void *)iobuf->buffer,(size_t)1,(size_t)length,
            iobuf->output_file) != length )
         if ( ferror(iobuf->output_file) )
         {
            Warning("Output error for I/O buffer");
            clearerr(iobuf->output_file);
            rc = -1;
         }
   }
   else if ( iobuf->user_function != NULL )
   {
      rc = (iobuf->user_function)(iobuf->buffer,(long)length,1);
   }
   else
   {
      Warning("Output cancelled because no output file/function set.");
      rc = -1;
   }

   iobuf->data = iobuf->buffer;
   iobuf->w_remaining = iobuf->r_remaining = -1L;

   return rc;
}

/* ----------------------- find_io_block ------------------------ */
/**
 *  @short Find the beginning of the next I/O data block in the input.
 *
 *  Read byte for byte from the input file specified
 *  for the I/O buffer and look for the sync-tag (magic
 *  number in little-endian or big-endian byte order.
 *  As long as the input is properly synchronized this
 *  sync-tag should be found in the first four bytes.
 *  Otherwise, input data is skipped until the next
 *  sync-tag is found. After the sync tag 10 more bytes
 *  (item type, version number, and length field) are read.
 *  The type of I/O (raw, buffered, or user-defined) depends
 *  on the settings of the I/O block.
 *
 *  @param  iobuf  The I/O buffer descriptor.
 *  @param  item_header An item header structure to be filled in.
 *
 *  @return  0 (O.k.),  -1 (error),  or  -2 (end-of-file)
 *
 */

int find_io_block (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   long sync_count = 0;
   int block_found, byte_number, byte_order;
   int rc = 0;
#ifdef ANSI_C_99
   const BYTE sync_tag_byte[] = { 0xD4, 0x1F, 0x8A, 0x37 };
#else
   static BYTE sync_tag_byte[] = { 0xD4, 0x1F, 0x8A, 0x37 };
#endif

   if ( iobuf == (IO_BUFFER *) NULL || item_header == (IO_ITEM_HEADER *) NULL )
      return -1;
   if ( iobuf->data_pending > 0 )
   {
      Warning("You forgot to read or skip the data of the previous I/O block");
      return -1;
   }
   iobuf->item_level = 0;
   iobuf->data = iobuf->buffer;
   iobuf->w_remaining = iobuf->r_remaining = -1L;
   iobuf->item_extension[0] = 0;
   if ( iobuf->buffer == (BYTE *) NULL || iobuf->buflen < 20 )
   {
      Warning("Attempt to read data failed due to invalid I/O buffer");
      return -1;
   }
   if ( iobuf->input_fileno < 0 && iobuf->input_file == (FILE *) NULL &&
        iobuf->user_function == NULL )
   {
      Warning("No file specified from which I/O buffer should be read");
      return -1;
   }

   if ( iobuf->input_fileno >= 0 || iobuf->input_file != (FILE *) NULL )
   {
      for ( sync_count=(-4L), block_found=byte_number=byte_order=0;
            !block_found; sync_count++ )
      {
         if ( iobuf->input_fileno >= 0 )  /* Use system read function */
            rc = READ_BYTES(iobuf->input_fileno,
                 (char *)(iobuf->buffer+byte_number),1L);
         else if ( (char) (*(char *)(iobuf->buffer+byte_number) = (char)
              getc(iobuf->input_file)) != (char) EOF )   /* Use getc macro */
            rc = 1;
         else if ( ferror(iobuf->input_file) )  /* EOF may be valid byte */
         {
            clearerr(iobuf->input_file);
            rc = -1;
         }
         else if ( feof(iobuf->input_file) )
         {
#ifdef OS_OS9
            cleareof(iobuf->input_file);
#else
            clearerr(iobuf->input_file);
#endif
            rc = 0;
         }
         if ( rc <= 0 )  /* End-of-file or read error */
         {
            item_header->type = 0;
            iobuf->item_length[0] = 0;
            if ( rc == 0 ) /* EOF */
               return -2;
            else           /* input error */
               return -1;
         }
         if ( byte_order == 0 )
         {
            if ( *iobuf->buffer == (BYTE) sync_tag_byte[0] )
               byte_order = 1;
            else if ( *iobuf->buffer == (BYTE) sync_tag_byte[3] )
               byte_order = -1;
            else
               continue;
            byte_number = 1;
         }
         else if ( byte_order == 1 )
         {
            if ( iobuf->buffer[byte_number] != sync_tag_byte[byte_number] )
            {
               byte_order = byte_number = 0;
               continue;
            }
            byte_number++;
         }
         else if ( byte_order == -1 )
         {
            if ( iobuf->buffer[byte_number] != sync_tag_byte[3-byte_number] )
            {
               byte_order = byte_number = 0;
               continue;
            }
            byte_number++;
         }
         if ( byte_number == 4 )
            block_found = 1;
      }

      if ( iobuf->input_fileno >= 0 )  /* Use system read function */
         rc = READ_BYTES(iobuf->input_fileno,(char *)(iobuf->buffer+4),12L);
      else if ( (rc = fread((void *)(iobuf->buffer+4),(size_t)1,(size_t)12,
              iobuf->input_file)) == 0 )
      {
         if ( ferror(iobuf->input_file) )
            rc = -1;
      }
      if ( rc > 0 && rc != 12 )
      {
      	 char msg[256];
         sprintf(msg,
              "Wrong number of bytes were read (%d instead of %d)",rc,12);
         Warning(msg);
         return -1;
      }
   }
   else if ( iobuf->user_function != NULL )
   {
      rc = (iobuf->user_function)(iobuf->buffer,16L,2);
   }
   
   /* The possibility of an extension field following in the header */
   /* leads to additional complications. */
   if ( rc > 0 )
   {
      int32_t sync_tag;
      uint32_t len1 = 0, xbit = 0;
      iobuf->data = iobuf->buffer;
      iobuf->r_remaining = iobuf->buflen;

      /* Reset the byte-order flag and then check the byte order. */
      iobuf->byte_order = 0;
      sync_tag = get_long(iobuf);

#ifdef SIXTY_FOUR_BITS
      if ( sync_tag == (int32_t) 0xD41F8A37 )
         iobuf->byte_order = 0;
      else if ( sync_tag == (int32_t) 0x378A1FD4 )
         iobuf->byte_order = 1;
#else
      if ( sync_tag == (int32_t) 0xD41F8A37L )
         iobuf->byte_order = 0;
      else if ( sync_tag == (int32_t) 0x378A1FD4L )
         iobuf->byte_order = 1;
#endif
      else
      {
         Warning("Invalid byte ordering of input data");
         return -1;
      }
      iobuf->data = iobuf->buffer+12;
      len1 = get_uint32(iobuf);
      xbit = len1 & (uint32_t)0x80000000UL;
      if ( xbit ) /* Really need to get the extension field now */
      {
         if ( iobuf->input_fileno >= 0 || iobuf->input_file != (FILE *) NULL )
         {
            if ( iobuf->input_fileno >= 0 )  /* Use system read function */
               rc = READ_BYTES(iobuf->input_fileno,(char *)(iobuf->buffer+16),4L);
            else if ( (rc = fread((void *)(iobuf->buffer+16),(size_t)1,(size_t)4,
                    iobuf->input_file)) == 0 )
            {
               if ( ferror(iobuf->input_file) )
                  rc = -1;
            }
            if ( rc > 0 && rc != 4 )
            {
      	       char msg[256];
               sprintf(msg,
                    "Wrong number of bytes were read (%d instead of %d)",rc,4);
               Warning(msg);
               return -1;
            }
         }
         else if ( iobuf->user_function != NULL )
         {
            rc = (iobuf->user_function)(iobuf->buffer+16,4L,2);
         }
         if ( sizeof(size_t) < 8 ) /* Check if the long I/O block could be read */
         {
            uint32_t len2;
            iobuf->data = iobuf->buffer+16;
            len2 = get_uint32(iobuf);
            if ( (len2 & 0x0FFF) > 1 )
            {
               Warning("I/O block is too long to be readable on this architecture.");
               return -1;
            }
         }
         iobuf->item_extension[0] = 1;
      }
      iobuf->data = iobuf->buffer;
      iobuf->w_remaining = iobuf->r_remaining = -1L;
   }

   if ( rc <= 0 )  /* End-of-file or read error */
   {
      item_header->type = 0;
      iobuf->item_length[0] = 0;
      item_header->can_search = 0;
      if ( rc == 0 ) /* EOF */
         return -2;
      else           /* input error */
         return -1;
   }

   item_header->type = 0;
   iobuf->data_pending = 1;
   if ( get_item_begin(iobuf,item_header) != 0 )
      return -1;
   iobuf->item_level = 0;

   if ( sync_count > 0 )
   {
      char msg[256];
      (void) sprintf(msg,
         "Synchronization error. %ld bytes of data have been skipped.",
         sync_count);
      Warning(msg);
      (void) sprintf(msg,"Now at %sitem type %lu, version %u, ID %ld, length %ld.",
         item_header->use_extension?"extended ":"",  item_header->type,
         item_header->version, item_header->ident, iobuf->item_length[0]);
      Warning(msg);
      if ( iobuf->sync_err_count++ >= iobuf->sync_err_max )
      {
         (void) sprintf(msg, "Too many synchronization errors.");
         Warning(msg);
         return -1;
      }
   }

   return 0;
}

/* ------------------------ read_io_block ------------------------ */
/**
 *  @short Read the data of an I/O block from the input.
 *
 *  This function is called for reading data after an I/O data block 
 *  has been found (with find_io_block) on input. 
 *  The type of I/O (raw, buffered, or user-defined) depends
 *  on the settings of the I/O block.
 *
 *  @param  iobuf       The I/O buffer descriptor.
 *  @param  item_header The item header descriptor.
 *
 *  @return  0 (O.k.),
 *	    -1 (error),
 *	    -2 (end-of-file),
 *	    -3 (block skipped because it is too large)
 *
 */

int read_io_block (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   int rc = 0;
   int length;

   if ( iobuf == (IO_BUFFER *) NULL || item_header == (IO_ITEM_HEADER *) NULL )
      return -1;

   if ( iobuf->data_pending <= 0 )
   {
      Warning("You must find an I/O block before you can read it");
      return -1;
   }

   if ( iobuf->item_level != 0 ||
      iobuf->item_length[0] < 0 )
      return -1;
#ifdef OS_MSDOS
   if ( iobuf->item_length[0] > 32767 )
   {
      Warning("Cannot read item of length exceeding 32767 bytes");
      return -1;
   }
#endif
   length = (size_t) iobuf->item_length[0];

   if ( iobuf->buffer == (BYTE *) NULL )
      return -1;

   if ( iobuf->buflen < iobuf->item_length[0]+16 )
   {
      if ( extend_io_buffer(iobuf,0,
           iobuf->item_length[0]+16-iobuf->buflen) == -1 )
      {
         Warning("I/O buffer too small; I/O block is skipped");
         if ( (rc = skip_io_block(iobuf,item_header)) < 0 )
            return rc;
         else
            return -3;
      }
   }

   if ( length > 0 )
   {
      int e4 = (iobuf->item_extension[0]?4:0);
      if ( iobuf->input_fileno >= 0 || iobuf->input_file != (FILE *) NULL )
      {
         if ( iobuf->input_fileno >= 0 )
            rc = READ_BYTES(iobuf->input_fileno,
               (char *)(iobuf->buffer+16+e4),length);
         else if ( (rc = fread((void *)(iobuf->buffer+16+e4),(size_t)1,(size_t)length,
              iobuf->input_file)) == 0 )
            if ( ferror(iobuf->input_file) )
               rc = -1;
         if ( rc > 0 && rc != length )
         {
	    char msg[256];
            sprintf(msg,
              "Wrong number of bytes were read (%d instead of %d)",rc,length);
            Warning(msg);
            return -1;
         }
      }
      else if ( iobuf->user_function != NULL )
         rc = (iobuf->user_function)(iobuf->buffer+16+e4,(long)length,3);
      else
         rc = -1;
   }
   else if ( length < 0 )
      rc = -1;

   if ( rc <= 0 && length != 0 )  /* End-of-file or read error */
   {
      item_header->type = 0;
      iobuf->item_length[0] = 0;
      if ( rc == 0 ) /* EOF */
         return -2;
      else           /* input error */
         return -1;
   }

   iobuf->data_pending = 0;
   return 0;
}

/* ------------------------ skip_io_block ------------------------ */
/**
 *  @short Skip the data of an I/O block from the input.
 *
 *  Skip the data of an I/O block from the input
 *  (after the block's header was read).
 *  This is the alternative to read_io_block() after having
 *  found an I/O block with find_io_block but realizing that
 *  this is a type of block you don't know how to read or
 *  simply not interested in.
 *  The type of I/O (raw, buffered, or user-defined) depends
 *  on the settings of the I/O block.
 *
 *  @param  iobuf        The I/O buffer descriptor.
 *  @param  item_header  The item header descriptor.
 *
 *  @return  0 (O.k.),  -1 (error)  or  -2 (end-of-file)
 *
 */

int skip_io_block (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header)
{
   char tbuf[512];
   long nbuf, ibuf, length;
   int rbuf, rc = 0;
#ifndef FSTAT_NOT_AVAILABLE
   struct stat st;
#endif

   if ( iobuf == (IO_BUFFER *) NULL || item_header == (IO_ITEM_HEADER *) NULL )
      return -1;
   if ( iobuf->data_pending <= 0 )
   {
      Warning("You must find an I/O block before you can skip it");
      return -1;
   }
   if ( iobuf->item_level != 0 || iobuf->item_length[0] < 0 )
      return -1;
   length = iobuf->item_length[0];
   if ( length == 0 ) /* We don't need to skip anything for blocks without data */
   {
      iobuf->item_length[0] = iobuf->sub_item_length[0] = -1;
      iobuf->data_pending = 0;
      return 0;
   }

   if ( iobuf->input_fileno < 0 && iobuf->user_function != NULL )
   {
      iobuf->item_length[0] = iobuf->sub_item_length[0] = -1;
      iobuf->data_pending = 0;
      return((iobuf->user_function)(iobuf->buffer,length,4));
   }

#ifndef FSTAT_NOT_AVAILABLE
   if ( iobuf->regular >= 0 )
   {
      if ( iobuf->input_fileno > 0 )
      {
         if ( iobuf->regular == 0 )
         {
            fstat(iobuf->input_fileno,&st);
#ifdef S_IFREG
            if ( st.st_mode & S_IFREG )
               iobuf->regular = 1;
            else
#endif
               iobuf->regular = -1;
         }
         if ( iobuf->regular == 1 )
         {
#ifdef __USE_LARGEFILE64
            /* Although blocks must be less than 2^31 bytes, we may */
	    /* have to take care of large files on 32 bit machines. */
            if ( lseek64(iobuf->input_fileno,(off64_t)length,SEEK_CUR) == -1 )
               return -1;
#else
#ifdef __USE_LARGEFILE
            if ( lseek(iobuf->input_fileno,(off_t)length,SEEK_CUR) == -1 )
#else
            if ( lseek(iobuf->input_fileno,length,SEEK_CUR) == -1 )
#endif
               return -1;
#endif
            iobuf->item_length[0] = iobuf->sub_item_length[0] = -1;
            iobuf->data_pending = 0;
            return 0;
         }
      }
      else if ( iobuf->input_file != (FILE *) NULL )
      {
         if ( iobuf->regular == 0 )
         {
            fstat(fileno(iobuf->input_file),&st);
#ifdef S_IFREG
            if ( st.st_mode & S_IFREG )
               iobuf->regular = 1;
            else
#endif
               iobuf->regular = -1;
         }
         if ( iobuf->regular == 1 )
         {
#ifdef __USE_LARGEFILE64
            /* Although blocks must be less than 2^31 bytes, we may */
	    /* have to take care of large files on 32 bit machines. */
            if ( fseeko64(iobuf->input_file,(off64_t)length,SEEK_CUR) == -1 )
               return -1;
#else
            if ( fseek(iobuf->input_file,length,SEEK_CUR) == -1 )
               return -1;
#endif
            iobuf->item_length[0] = iobuf->sub_item_length[0] = -1;
            iobuf->data_pending = 0;
            return 0;
         }
      }
   }
#endif

   nbuf = length/512;
   rbuf = length%512;
   if ( iobuf->input_fileno >= 0 )
   {
      for ( ibuf=0; ibuf<nbuf; ibuf++ )
         rc = READ_BYTES(iobuf->input_fileno,tbuf,512L);
      if ( rbuf > 0 )
         rc = READ_BYTES(iobuf->input_fileno,tbuf,rbuf);
   }
   else if ( iobuf->input_file != (FILE *) NULL )
   {
      for ( ibuf=0; ibuf<nbuf; ibuf++ )
         rc = fread((void *)tbuf,(size_t)1,(size_t)512,iobuf->input_file);
      if ( rbuf > 0 )
         rc = fread((void *)tbuf,(size_t)1,(size_t)rbuf,iobuf->input_file);
      if ( ferror(iobuf->input_file) )
         rc = -1;
   }

   iobuf->item_length[0] = iobuf->sub_item_length[0] = -1;
   iobuf->data_pending = 0;

   if ( rc <= 0 )  /* End-of-file or read error */
   {
      item_header->type = 0;
      if ( rc == 0 ) /* EOF */
         return -2;
      else           /* input error */
         return -1;
   }
   return 0;
}

/* ---------------------- list_io_blocks ------------------------- */
/**
 *  Show the top-level item of an I/O block on standard output.
 *
 *  List type, version, ident, and length) of the top item of all
 *  I/O blocks in input file onto standard output.
 *
 *  @param  iobuf        The I/O buffer descriptor.
 *  @param  verbosity    Try showing type name at >=1, description at >=2.
 *
 *  @return  0 (O.k.),  -1 (error)
 *
 */

int list_io_blocks (IO_BUFFER *iobuf, int verbosity)
{
   IO_ITEM_HEADER item_header;
   int rc;
   char msg[512];

   Output("\n");

   while ( (rc = find_io_block(iobuf,&item_header)) == 0 )
   {
      if ( iobuf->item_extension[0] )
         sprintf(msg,"Type %4lu, version %u, extended length %ld",
            item_header.type, item_header.version, iobuf->item_length[0]);
      else
         sprintf(msg,"Type %4lu, version %u, length %ld",
            item_header.type, item_header.version, iobuf->item_length[0]);
      Output(msg);
      if ( item_header.ident >= 0 )
      {
         sprintf(msg," (%sid %ld = 0x%lx)",
            item_header.user_flag?"*":"",
	    item_header.ident,item_header.ident);
         Output(msg);
      }
      if ( iobuf->byte_order )
         Output(" with inverse byte order");
      if ( verbosity >= 1 )
      {
         const char *name = eventio_registered_typename(item_header.type);
         const char *description = "";
         if ( verbosity >= 2 )
            description = eventio_registered_description(item_header.type);
         if ( name != NULL && *name != '\0' )
         {
            snprintf(msg,sizeof(msg)-1,"\t[%s] %s", name, description);
            Output(msg);
         }
      }
      Output("\n");
      if ( (rc = skip_io_block(iobuf,&item_header)) < 0 )
      {
         sprintf(msg,"(skip_io_block returned %d)\n\n",rc);
         Output(msg);
         return rc;
      }
   }

   if ( rc != -2 )
   {
      sprintf(msg,"(find_io_block returned %d)\n\n",rc);
      Output(msg);
      return rc;
   }
   else
   {
      Output("\n");
      return 0;
   }
}

/* ------------------ copy_item_to_io_block -------------------- */
/**
 *  Copy a sub-item to another I/O buffer as top-level item.
 *
 *  @param  iobuf2        Target I/O buffer descriptor.
 *  @param  iobuf         Source I/O buffer descriptor.
 *  @param  item_header   Header for the item in iobuf that
 *			  should be copied to iobuf2.
 *
 *  @return  0 (o.k.),  -1 (error),  -2 (not enough memory etc.)
 *
 */

int copy_item_to_io_block (IO_BUFFER *iobuf2, IO_BUFFER *iobuf, 
   const IO_ITEM_HEADER *item_header)
{
   int length;
   int ilevel;
   int ie4 = 0;

   if ( iobuf == (IO_BUFFER *) NULL || iobuf2 == (const IO_BUFFER *) NULL ||
        item_header == (const IO_ITEM_HEADER *) NULL )
      return -1;
   if ( iobuf->buffer == (BYTE *) NULL || iobuf2->buffer == (const BYTE *) NULL )
      return -1;

   if ( item_header->level != iobuf->item_level-1 )
   {
      Warning("Item level is inconsistent");
      return -1;
   }
   if (iobuf->item_level > 0 && iobuf->item_level <= MAX_IO_ITEM_LEVEL)
      ilevel = iobuf->item_level-1;
   else
      return -1;
#ifdef OS_MSDOS
   if ( iobuf->item_length[ilevel] > 32767 )
   {
      Warning("Cannot copy item of length exceeding 32767 bytes");
      return -1;
   }
#endif
   if ( (length = (int) iobuf->item_length[ilevel]) < 0 )
      return -1;

   reset_io_block(iobuf2);
   iobuf2->byte_order = iobuf->byte_order;
   
   if ( iobuf->item_extension[ilevel] )
      ie4 = 4;
   if ( iobuf2->buflen < length+16+ie4 )
   {
      if ( extend_io_buffer(iobuf2,0,length+16+ie4-iobuf2->buflen) == -1 )
      {
         Warning("I/O buffer too small; item not copied");
         return -2;
      }
   }

   /* Set identical byte order of source and target */
   memcpy((void *)iobuf2->buffer,(void *)iobuf->buffer,(size_t)4);
   /* Copy item header and data */
   memcpy((void *)(iobuf2->buffer+4),
     (void *)(iobuf->buffer+iobuf->item_start_offset[ilevel]-12-ie4),
     (size_t)(12+ie4+length));
   iobuf2->data = iobuf2->buffer+16+ie4+length;
   iobuf2->item_length[0] = length;
   iobuf2->sub_item_length[0] = iobuf->sub_item_length[ilevel];
   iobuf2->w_remaining = iobuf2->buflen - length - 16 - ie4;
   iobuf2->item_level = 0;

   iobuf->data = iobuf->buffer+iobuf->item_start_offset[ilevel]+length;
   iobuf->r_remaining -= length;

   return 0;
}

/* ---------------- append_io_block_as_item ------------------ */
/**
 *  @short Append data from one I/O block into another one.
 *
 *  Append the data from a complete i/o block as an additional
 *  subitem to another i/o block.
 *
 *  @param  iobuf      The target I/O buffer descriptor,
 *                     must be 'opened' for 'writing',
 *                     i.e. 'put_item_begin()' must be called.
 *  @param item_header Item header of the item in
 *                     iobuf which is currently being filled.
 *  @param buffer      Data to be filled in. Must be all
 *                     data from an I/O buffer, including the
 *                     4 signature bytes.
 *  @param length      The length of buffer in bytes.
 *
 *  @return  0 (o.k.),  -1 (error),  -2 (not enough memory etc.)
 *
 */

int append_io_block_as_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header, 
   BYTE *buffer, long length)
{
   int i, ilevel;

   if ( iobuf == (IO_BUFFER *) NULL || buffer == (BYTE *) NULL ||
        item_header == (IO_ITEM_HEADER *) NULL )
      return -1;
   if ( iobuf->buffer == (BYTE *) NULL )
      return -1;

   if ( iobuf->item_level <= 0 )
   {
      Warning("Cannot append to empty I/O block");
      return -1;
   }
   ilevel = iobuf->item_level - 1;
   if ( iobuf->w_remaining == -1 )
   {
      Warning("Cannot append to I/O block");
      return -1;
   }

#ifdef OS_MSDOS
   if ( length > 32767 )
   {
      Warning("Cannot append block of length exceeding 32767 bytes");
      return -1;
   }
#endif
   if ( length < 16 )
      return -1;
   length -= 4;

   for (i=0; i<4; i++)
      if ( iobuf->buffer[i] != buffer[i] )
      {
         if ( iobuf->buffer[0] != buffer[3] &&
              iobuf->buffer[1] != buffer[2] )
            Warning("Data to be appended is not an I/O buffer");
         else
            Warning("Cannot append to I/O block with different byte ordering");
         return -1;
      }

   if ( iobuf->w_remaining < length )
   {
      if ( extend_io_buffer(iobuf,256,length) == -1 )
      {
         Warning("I/O buffer too small: nothing appended");
         return -2;
      }
   }

   memcpy((void *)iobuf->data,(void *)(buffer+4),(size_t)(length));
   iobuf->data += length;
   iobuf->w_remaining -= length;
   iobuf->item_length[ilevel] += length;
   iobuf->sub_item_length[ilevel] += length;

   return 0;
}

/* ======== Interface to registry of well-known data block type ========= */
/* The implementation is available outside of the core eventio code and */
/* a function pointer has to be set up before it is used. */

/** We just keep a pointer to such a function in the core eventio code */
static EVREGSEARCH find_ev_reg_ptr;

/* ------------------ set_eventio_registry_hook ----------------- */
/**
 *  A single function for setting a registry search function is the
 *  interface between the eventio core code and any code implementing
 *  the registry. This search function is also responsible for initializing
 *  the registry. By default, no such registry is used.
 *
 *  @param fptr  A pointer to the registry search function.
 */

void set_eventio_registry_hook(struct ev_reg_entry * (* fptr)(unsigned long t))
{
   if ( fptr == &find_ev_reg )
   {
      fprintf(stderr,"Th generic eventio registry search function is not a valid implementation.\n");
      fprintf(stderr,"This would result in infinite recursion (up to stack overflow)\n");
   }
   else
      find_ev_reg_ptr = fptr;
}

/** ------------------  find_ev_reg  ------------------------- */
/*
 *  Generic registry search function checks for presence of the function
 *  pointer before starting the search. 
 */

static const char *none="";

struct ev_reg_entry *find_ev_reg(unsigned long t)
{
   if ( find_ev_reg_ptr != NULL )
      return (*find_ev_reg_ptr)(t);
   else
      return (struct ev_reg_entry *) NULL;
}

/** Extract the name for a given type number, if available */

const char *eventio_registered_typename(unsigned long type)
{
   struct ev_reg_entry *e = find_ev_reg(type);
   if ( e == NULL )
      return none;
   if ( e->name != NULL )
      return e->name;
   return none;
}

/** Extract the optional description for a given type number, if available */

const char *eventio_registered_description(unsigned long type)
{
   struct ev_reg_entry *e = find_ev_reg(type);
   if ( e == NULL )
      return none;
   if ( e->description != NULL )
      return e->description;
   return none;
}

