/* ============================================================================

   Copyright (C) 1991, 2001, 2007, 2009, 2010, 2014  Konrad Bernloehr

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

/** @file io_basic.h
    @short Basic header file for eventio data format.
    
    @author  Konrad Bernloehr
    @date    1991 to 2014
    @date    @verbatim CVS $Date: 2014/08/04 13:05:28 $ @endverbatim
    @version @verbatim CVS $Revision: 1.22 $ @endverbatim


    Header file for structures and function prototypes for
    the basic eventio functions.
    Not to be used to declare any project-specific structures
    and prototypes! Declare any such things in 'io_project.h'
    or in separate header files.
*/

#ifndef IO_BASIC_H__LOADED

#define IO_BASIC_H__LOADED 1

#ifdef __cplusplus
extern "C" {
#endif

#ifndef INITIAL_H__LOADED
#include "initial.h"
#endif

//#ifndef __cplusplus
#ifndef WARNING_H__LOADED
#include "warning.h"
#endif
//#endif

#define MAX_IO_ITEM_LEVEL 20

#define HAVE_EVENTIO_USER_FLAG 1
#define HAVE_EVENTIO_EXTENDED_LENGTH 1
#define HAVE_EVENTIO_HEADER_LENGTH 1

/* Flag bits to be used with put_item_begin_with_flag(): */
#define EVENTIO_USER_FLAG 1
#define EVENTIO_EXTENSION_FLAG 2

typedef unsigned char BYTE;

/** An IO_ITEM_HEADER is to access header info for an I/O block 
    and as a handle to the I/O buffer. */

struct _struct_IO_ITEM_HEADER
{
   unsigned long type;  /**< The type number telling the type of I/O block. */
   unsigned version;    /**< The version number used for the block. */
   int can_search;      /**< Set to 1 if I/O block consist of sub-blocks only. */
   int level;           /**< Tells how many levels deep we are nested now. */
   long ident;          /**< Identity number. */
   int user_flag;       /**< One more bit in the header available for user data. */
   int use_extension;   /**< Non-zero if the extension header field should be used. */
   size_t length;       /**< Length of data field, for information only. */
};
typedef struct _struct_IO_ITEM_HEADER IO_ITEM_HEADER;

/** The IO_BUFFER structure contains all data needed the manage the stuff. */

struct _struct_IO_BUFFER
{
   unsigned char *buffer;  /**< Pointer to allocated data space. */
   long buflen;            /**< Usable length of data space. */
   long r_remaining, w_remaining; /**< Byte available for reading/writing. */
   BYTE *data;             /**< Position for next get.../put... */
   /** @short Indicates if buffer is allocated by eventio.
    *  It is 1 if buffer is allocated by eventio,
    *  0 if buffer provided by user function (in which case the
    *  user should call allocate_io_buffer with the appropriate size;
    *  then the buffer always allocated in allocate_io_buffer() must
    *  be freed by the user function, replaced by its external
    *  buffer, and finally is_allocated set to 0).
   */
   int is_allocated;
   int item_level;         /**< Current level of nesting of items. */
   long item_length[MAX_IO_ITEM_LEVEL]; /**< Length of each level of items */
   long sub_item_length[MAX_IO_ITEM_LEVEL]; /**< Length of its sub-items */
   long item_start_offset[MAX_IO_ITEM_LEVEL]; /**< Where the item starts in buffer. */
   int item_extension[MAX_IO_ITEM_LEVEL]; /**< Where the extension field was used. */
   int input_fileno;  /**< For use of read() function for input. */
   int output_fileno; /**< For use of write() function for output. */
   FILE *input_file;  /**< For use of stream I/O for input. */
   FILE *output_file; /**< For use of stream I/O for output. */
   int (*user_function) (unsigned char *, long, int);
                      /**< For use of special type of I/O. */
   int byte_order;    /**< Set if block is not in internal byte order. */
   int data_pending;  /**< Set to 1 when header is read but not the data */
   long min_length;   /**< The initial and minimum length of the buffer */
   long max_length;   /**< The maximum length for extending the buffer */
   int aux_count;     /**< May be used for dedicated buffers */
   int regular;       /**< 1 if a regular file, 0 not known, -1 not regular */
   int extended;      /**< Set to 1 if you want to use the extension field always. */
   int sync_err_count;/**< Count of synchronization errors. */
   int sync_err_max;  /**< Maximum accepted number of synchronisation errors. */
};
typedef struct _struct_IO_BUFFER IO_BUFFER;
typedef int (*IO_USER_FUNCTION) (unsigned char *, long, int);

/* ------------------ Buffer size parameters ------------------ */

#ifdef OS_MSDOS
# define IO_BUFFER_INITIAL_LENGTH 8192L
# define IO_BUFFER_LENGTH_INCREMENT 8192L
# define IO_BUFFER_MAXIMUM_LENGTH 65500L
#else
# define IO_BUFFER_INITIAL_LENGTH 32768L
# define IO_BUFFER_LENGTH_INCREMENT 65536L
# ifdef OS_OS9
#  define IO_BUFFER_MAXIMUM_LENGTH 1000000L
# else
#  define IO_BUFFER_MAXIMUM_LENGTH 3000000L
# endif
#endif

/* ------------------- Macro definitions ---------------------- */

/* ------------------------ COPY_BYTES ------------------------- */

#define COPY_BYTES(_target,_source,_num) memcpy(_target,_source,_num)

/* -------------------------- put_byte ------------------------- */
/*
@@ put_byte(): Write a byte to the output buffer.
 */

#define put_byte(_c,_p) (--(_p)->w_remaining>=0 ? \
   (*(_p)->data++ = (BYTE)(_c)) : \
   (BYTE)extend_io_buffer(_p,(unsigned)(_c), \
     (IO_BUFFER_LENGTH_INCREMENT)))

/* -------------------------- get_byte ------------------------- */
/*
@@ get_byte(): Read a byte from the input buffer.
 */

#define get_byte(p) (--(p)->r_remaining>=0? *(p)->data++ : -1)


/* ------------------ Function prototypes --------------------- */

/* I/O buffer allocation: */
IO_BUFFER *allocate_io_buffer (size_t buflen);
int extend_io_buffer (IO_BUFFER *iobuf, unsigned next_byte,
   long increment);
void free_io_buffer (IO_BUFFER *iobuf);

/* Atomic data type handling: */
/* ... 8 bits integer data types ... */
void put_vector_of_byte (const BYTE *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_byte (BYTE *vec, int num, IO_BUFFER *iobuf);
#define put_vector_of_uint8 put_vector_of_byte
#define get_vector_of_uint8 get_vector_of_byte

/* Unsigned integers (counts) of unspecified length */
void put_count (uintmax_t num, IO_BUFFER *iobuf);
void put_count32 (uint32_t num, IO_BUFFER *iobuf);
void put_count16 (uint16_t num, IO_BUFFER *iobuf);
uintmax_t get_count (IO_BUFFER *iobuf);
uint32_t get_count32 (IO_BUFFER *iobuf);
uint16_t get_count16 (IO_BUFFER *iobuf);
/* Signed integers (counts) of unspecified length */
void put_scount (intmax_t num, IO_BUFFER *iobuf);
void put_scount32 (int32_t num, IO_BUFFER *iobuf);
void put_scount16 (int16_t num, IO_BUFFER *iobuf);
intmax_t get_scount (IO_BUFFER *iobuf);
int32_t get_scount32 (IO_BUFFER *iobuf);
int16_t get_scount16 (IO_BUFFER *iobuf);
void put_vector_of_int_scount (const int *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_int_scount (int *vec, int num, IO_BUFFER *iobuf);

/* ... 16 bits integer data types ... */
/* ... (native) ... */
void put_vector_of_short (const short *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_short (short *vec, int num, IO_BUFFER *iobuf);
#define put_vector_of_int16 put_vector_of_short
#define get_vector_of_int16 get_vector_of_short
void put_vector_of_uint16 (const uint16_t *uval, int num, IO_BUFFER *iobuf);
void get_vector_of_uint16 (uint16_t *uval, int num, IO_BUFFER *iobuf);
uint16_t get_uint16(IO_BUFFER *iobuf);
/* ... (with conversion) ... */
void put_short (int num, IO_BUFFER *iobuf);
int get_short (IO_BUFFER *iobuf);
void put_vector_of_int (const int *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_int (int *vec, int num, IO_BUFFER *iobuf);

/* ... 32 bits integer data types ... */
/* ... (native) ... */
void put_int32 (int32_t num, IO_BUFFER *iobuf);
int32_t get_int32 (IO_BUFFER *iobuf);
void put_uint32 (uint32_t num, IO_BUFFER *iobuf);
uint32_t get_uint32 (IO_BUFFER *iobuf);
void put_vector_of_int32 (const int32_t *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_int32 (int32_t *vec, int num, IO_BUFFER *iobuf);
void put_vector_of_uint32 (const uint32_t *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_uint32 (uint32_t *vec, int num, IO_BUFFER *iobuf);
/* ... (with conversion) ... */
void put_long (long num, IO_BUFFER *iobuf);
long get_long (IO_BUFFER *iobuf);
void put_vector_of_long (const long *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_long (long *vec, int num, IO_BUFFER *iobuf);

#ifdef HAVE_64BIT_INT
/* ... 64 bits integer data types (data may be non-portable!) ... */
void put_vector_of_int64 (const int64_t *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_int64 (int64_t *vec, int num, IO_BUFFER *iobuf);
void put_vector_of_uint64 (const uint64_t *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_uint64 (uint64_t *vec, int num, IO_BUFFER *iobuf);
#endif

/* ... run-length encoded character string ... */
int put_string (const char *s, IO_BUFFER *iobuf);
int get_string (char *s, int nmax, IO_BUFFER *iobuf);
int put_long_string (const char *s, IO_BUFFER *iobuf);
int get_long_string (char *s, int nmax, IO_BUFFER *iobuf);
int put_var_string (const char *s, IO_BUFFER *iobuf);
int get_var_string (char *s, int nmax, IO_BUFFER *iobuf);

/* ... 32 bits floating point ... */
/* ... (native) ... */
void put_vector_of_float (const float *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_float (float *vec, int num, IO_BUFFER *iobuf);
/* ... (with conversion) ... */
void put_real (double d, IO_BUFFER *iobuf);
double get_real (IO_BUFFER *iobuf);
void put_vector_of_real (const double *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_real (double *vec, int num, IO_BUFFER *iobuf);

/* ... 64 bits floating point ... */
void put_double (double d, IO_BUFFER *iobuf);
double get_double (IO_BUFFER *iobuf);
void put_vector_of_double (const double *vec, int num, IO_BUFFER *iobuf);
void get_vector_of_double (double *vec, int num, IO_BUFFER *iobuf);

/* ... 16 bits floating point ... */
void dbl_to_sfloat (double dnum, uint16_t *snum);
void fltp_to_sfloat (const float *fnum, uint16_t *snum);
void put_sfloat (double dnum, IO_BUFFER *iobuf);
double dbl_from_sfloat(const uint16_t *snum);
double get_sfloat (IO_BUFFER *iobuf);

/* General item management: */
int put_item_begin (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int put_item_begin_with_flags (IO_BUFFER *iobuf, 
   IO_ITEM_HEADER *item_header, int user_flag, int extended);
int put_item_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int unput_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int get_item_begin (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int get_item_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int unget_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int next_subitem_type (IO_BUFFER *iobuf);
long next_subitem_length (IO_BUFFER *iobuf);
long next_subitem_ident (IO_BUFFER *iobuf);
int skip_subitem (IO_BUFFER *iobuf);
int search_sub_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header,
    IO_ITEM_HEADER *sub_item_header);
int rewind_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int remove_item (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int list_sub_items (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header,
    int maxlevel, int verbosity);

/* File I/O: */
int reset_io_block (IO_BUFFER *iobuf);
int write_io_block (IO_BUFFER *iobuf);
int find_io_block (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int read_io_block (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int skip_io_block (IO_BUFFER *iobuf, IO_ITEM_HEADER *item_header);
int list_io_blocks (IO_BUFFER *iobuf, int verbosity);

int copy_item_to_io_block (IO_BUFFER *iobuf2, IO_BUFFER *iobuf,
    const IO_ITEM_HEADER *item_header);
int append_io_block_as_item (IO_BUFFER *iobuf,
    IO_ITEM_HEADER *item_header, BYTE *_buffer, long length);
    
/* Registry hook: */

struct ev_reg_entry
{
   unsigned long type; /**< The data block type number */
   char *name;         /**< The data block name (short) */
   char *description;  /**< Optional longer description of the data block */
};

/** This optionally available function is implemented externally */
struct ev_reg_entry *find_ev_reg(unsigned long t);

typedef struct ev_reg_entry *(*EVREGSEARCH)(unsigned long t);

/** This function should be used to set the find_ev_reg_ptr function pointer. */
void set_eventio_registry_hook(EVREGSEARCH fptr);

/** This functions using the stored function pointer are now in the core eventio code. */

const char *eventio_registered_typename(unsigned long type);
const char *eventio_registered_description(unsigned long type);


#ifdef __cplusplus
}
#endif

#endif
