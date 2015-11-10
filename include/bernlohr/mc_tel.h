/* ============================================================================

   Copyright (C) 1997, 2001, 2009, 2010  Konrad Bernloehr

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

/** @file mc_tel.h 
 *  @short Definitions and structures for CORSIKA Cherenkov light interface.
 *
 *  This file contains definitions of data structures and of
 *  function prototypes as needed for the Cherenkov light
 *  extraction interfaced to the modified CORSIKA code.
 *
 *  @author  Konrad Bernloehr 
 *  @date    1997 to 2010
 *  @date    @verbatim CVS $Date: 2014/02/20 10:53:06 $ @endverbatim
 *  @version @verbatim CVS $Revision: 1.15 $ @endverbatim
 */
/* ========================================================= */

#ifndef _MC_TEL_LOADED

#define _MC_TEL_LOADED 2

#ifndef _EVENTIO_BASIC_LOADED
# include "io_basic.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Data types: */

#ifdef DOUBLEPREC
typedef double bernlohr_real;
#else
typedef float bernlohr_real;
#endif

typedef short INT16;
typedef unsigned short UINT16;
#ifdef OS_MSDOS
typedef long INT32;
typedef unsigned long UINT32;
#else
typedef int INT32;
typedef unsigned int UINT32;
#endif

/* Data structures: */
/**
 * Photons collected in bunches of identical direction, position, time,
 * and wavelength. The wavelength will normally be unspecified as
 * produced by CORSIKA (lambda=0).
*/

struct bunch
{
   float photons; /**< Number of photons in bunch */
   float x, y;    /**< Arrival position relative to telescope (cm) */
   float cx, cy;  /**< Direction cosines of photon direction */
   float ctime;   /**< Arrival time (ns) */
   float zem;     /**< Height of emission point above sea level (cm) */
   float lambda;  /**< Wavelength in nanometers or 0 */
};

/**
 *  The compact_bunch struct is equivalent to the bunch struct
 *  except that we try to use less memory.
 *  And that has a number of limitations:
 *  1) Bunch sizes must be less than 327.
 *  2) photon impact points in a horizontal plane through the centre
 *     of each detector sphere must be less than 32.7 m from the
 *     detector centre in both x and y coordinates. Thus,
 *        sec(z) * R < 32.7 m 
 *     is required, with 'z' being the zenith angle and 'R' the radius
 *     of the detecor sphere. When accounting for multiple scattering
 *     and Cherenkov emission angles, the actual limit is reached 
 *     even earlier than that.
 *  3) Only times within 3.27 microseconds from the time, when the 
 *     primary particle propagated with the speed of light would cross
 *     the altitude of the sphere centre, can be treated.
 *     For large zenith angle observations this limits horizontal
 *     core distances to about 1000 m.
 *  For efficiency reasons, no checks are made on these limits.
 */

struct compact_bunch
{
   short photons; /**< ph*100 */
   short x, y;    /**< x,y*10 (mm) */
   short cx, cy;  /**< cx,cy*30000 */
   short ctime;   /**< ctime*10 (0.1ns) after subtracting offset */
   short log_zem; /**< log10(zem)*1000 */
   short lambda;  /**< (nm) or 0 */
};

/** A photo-electron produced by a photon hitting a pixel. */

struct photo_electron
{
   int pixel;     /**< The pixel that was hit. */
   int lambda;    /**< The wavelength of the photon. */
   double atime;  /**< The time [ns] when the photon hit the pixel. */
   // double amplitude; /**< Amplitude [mean p.e.] of the resulting signal. */
};

/** The linked_string is mainly used to keep CORSIKA input. */

struct linked_string
{
   char *text;
   struct linked_string *next;
};

/** Extra shower parameters of unspecified nature. Useful for things
    to be used like in the event header but which may only become available
    while processing a shower. Should be initialized with the
    init_shower_extra_parameters(int ni_max, int nf_max) function. */

struct shower_extra_parameters
{
   long id;       /**< May identify to the user what the parameters should mean. */
   int is_set;    /**< May be reset after writing the parameter block
                       and must thus be set to 1 for each shower for
                       which the extra parameters should get recorded. */
   double weight; /**< To be used if the weight of a shower may change during
                       processing, e.g. when shower processing can be aborted
                       depending on how quickly the electromagnetic component
                       builds up and the remaining showers may have a larger
                       weight to compensate for that.
                       For backwards compatibility this should be set to 1.0
                       when no additional weight is needed. */
   size_t niparam;/**< Number of extra integer parameters. */
   int *iparam;   /**< Space for extra integer parameters, at least of size
                       niparam. */
   size_t nfparam;/**< Number of extra floating-point parameters. */
   float *fparam;   /**< Space for extra floats, at least of size nfparam. */ 
};

/* I/O item types: */

/* Never change the following numbers after MC data is created: */
#define IO_TYPE_MC_BASE     1200
#define IO_TYPE_MC_RUNH     (IO_TYPE_MC_BASE+0)
#define IO_TYPE_MC_TELPOS   (IO_TYPE_MC_BASE+1)
#define IO_TYPE_MC_EVTH     (IO_TYPE_MC_BASE+2)
#define IO_TYPE_MC_TELOFF   (IO_TYPE_MC_BASE+3)
#define IO_TYPE_MC_TELARRAY (IO_TYPE_MC_BASE+4)
#define IO_TYPE_MC_PHOTONS  (IO_TYPE_MC_BASE+5)
#define IO_TYPE_MC_LAYOUT   (IO_TYPE_MC_BASE+6)
#define IO_TYPE_MC_TRIGTIME (IO_TYPE_MC_BASE+7)
#define IO_TYPE_MC_PE       (IO_TYPE_MC_BASE+8)
#define IO_TYPE_MC_EVTE     (IO_TYPE_MC_BASE+9)
#define IO_TYPE_MC_RUNE     (IO_TYPE_MC_BASE+10)
#define IO_TYPE_MC_LONGI    (IO_TYPE_MC_BASE+11)
#define IO_TYPE_MC_INPUTCFG (IO_TYPE_MC_BASE+12)
#define IO_TYPE_MC_TELARRAY_HEAD (IO_TYPE_MC_BASE+13)
#define IO_TYPE_MC_TELARRAY_END (IO_TYPE_MC_BASE+14)
#define IO_TYPE_MC_EXTRA_PARAM (IO_TYPE_MC_BASE+15)

/* Function prototypes: */

/* io_telescope.c */
int write_tel_block (IO_BUFFER *iobuf, int type, int num,
      bernlohr_real *data, int len);
int read_tel_block (IO_BUFFER *iobuf, int type, bernlohr_real *data,
      int maxlen);
int print_tel_block (IO_BUFFER *iobuf);

int write_input_lines (IO_BUFFER *iobuf, 
      struct linked_string *list);
int read_input_lines (IO_BUFFER *iobuf, 
      struct linked_string *list);

int write_tel_pos (IO_BUFFER *iobuf, int ntel, double *x,
      double *y, double *z, double *r);
int read_tel_pos (IO_BUFFER *iobuf, int max_tel, int *ntel,
      double *x, double *y, double *z, double *r);
int print_tel_pos (IO_BUFFER *iobuf);

int write_tel_offset (IO_BUFFER *iobuf, int narray, double toff,
      double *xoff, double *yoff);
int write_tel_offset_w (IO_BUFFER *iobuf, int narray, double toff,
      double *xoff, double *yoff, double *weight);
int read_tel_offset (IO_BUFFER *iobuf, int max_array, int *narray,
      double *toff, double *xoff, double *yoff);
int read_tel_offset_w (IO_BUFFER *iobuf, int max_array, int *narray,
      double *toff, double *xoff, double *yoff, double *weight);
int print_tel_offset (IO_BUFFER *iobuf);

int begin_write_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int array);
int end_write_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih);
int begin_read_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int *array);
int end_read_tel_array (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih);

int write_tel_array_head (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int array);
int write_tel_array_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int array);
int read_tel_array_head (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int *array);
int read_tel_array_end (IO_BUFFER *iobuf, IO_ITEM_HEADER *ih, int *array);

int write_tel_photons (IO_BUFFER *iobuf, int array, int tel,
      double photons, struct bunch *bunches, int nbunches,
      int ext_bunches, char *ext_fname);
int write_tel_compact_photons (IO_BUFFER *iobuf, int array, int tel,
      double photons, struct compact_bunch *cbunches, int nbunches,
      int ext_bunches, char *ext_fname);
int read_tel_photons (IO_BUFFER *iobuf, int max_bunches, int *array,
      int *tel, double *photons, struct bunch *bunches, int *nbunches);
int print_tel_photons (IO_BUFFER *iobuf);

int write_shower_longitudinal (IO_BUFFER *iobuf, int event, int type, 
      double *data, int ndim, int np, int nthick, double thickstep);
int read_shower_longitudinal (IO_BUFFER *iobuf, int *event, 
      int *type, double *data, int ndim, int *np, int *nthick, 
      double *thickstep, int max_np);

int write_camera_layout (IO_BUFFER *iobuf, int itel, int type, 
      int pixels, double *xp, double *yp);
int read_camera_layout (IO_BUFFER *iobuf, int max_pixels, int *itel,
      int *type, int *pixels, double *xp, double *yp);
int print_camera_layout (IO_BUFFER *iobuf);

int write_photo_electrons (IO_BUFFER *iobuf, int array, int tel, 
      int npe, int pixels, int flags, int *pe_counts, int *tstart, 
      double *t, double *a);
int read_photo_electrons (IO_BUFFER *iobuf, int max_pixel,
      int max_pe, int *array, int *tel, int *npe, int *pixels, int *flags, 
      int *pe_counts, int *tstart, double *t, double *a);
int print_photo_electrons (IO_BUFFER *iobuf);

int write_shower_extra_parameters (IO_BUFFER *iobuf,
      struct shower_extra_parameters *ep);
int read_shower_extra_parameters (IO_BUFFER *iobuf,
      struct shower_extra_parameters *ep);
int print_shower_extra_parameters (IO_BUFFER *iobuf);
int init_shower_extra_parameters(struct shower_extra_parameters *ep,
   size_t ni_max, size_t nf_max);
int clear_shower_extra_parameters (struct shower_extra_parameters *ep);
struct shower_extra_parameters *get_shower_extra_parameters(void);

#ifdef __cplusplus
}
#endif

#endif
