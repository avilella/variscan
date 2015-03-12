/*
 * cv_int.h --
 *
 *  Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv_int.h,v 1.6 1998/08/20 12:55:40 decoster Exp $
 */

#ifndef _CV_INT_H_
#define _CV_INT_H_

#include <math.h>
#include <stdlib.h>

#include "cv.h"

extern void *Malloc(size_t size);
extern void *Calloc(int n, size_t size);
extern void Free(void * ptr);

#define malloc Malloc
#define calloc Calloc
#define free Free

/*
 * Include for optionnal messages log. This is used to keep trace of computation
 * times of the different convolution algorithm.
 */

#include "logMsg.h"
#ifdef LOG_MESSAGES
static char* method_str[3] = {
  "di", "mp", "ft"
};
#endif


#include "assert.h"

#ifndef swap
#define swap(a,b) tmp=(a);(a)=(b);(b)=tmp
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

/*
 * Global variables that handle the filter parameters.
 */

enum {
  NUMERICAL,
  ANALYTICAL
};

enum {
  REAL,
  CPLX
};

extern int    flt_form;             /* Direct and fourier property. */
                                    /* CV_RR_FORM...*/
extern int    flt_def;              /* Kind of definition : NUMERICAL or */
                                    /* ANALYTICAL.*/
extern void * flt_d_data;           /* Storage of direct numerical data */
extern void * flt_f_data;           /* Storage of fourier numerical data */
extern int    flt_d_n;              /* Size of the direct data arrays */
extern real   flt_d_begin;          /* Domain of the direct form. Used for */
extern real   flt_d_end;            /* analytical computation.*/
extern int    flt_f_n;              /* Size of the fourier data arrays */
extern real   flt_f_begin;          /* Domain of the fourier form. Used for */
extern real   flt_f_end;            /* analytical computation.*/
extern double (* flt_d_real_ptr)(); /* Function pointers to filter defs for */
extern double (* flt_d_imag_ptr)(); /* its direct and its fourier forms. Used */
extern double (* flt_f_real_ptr)(); /* for analytical computation. */
extern double (* flt_f_imag_ptr)();
extern real   flt_scale;            /* Scale to apply to the filter. Used for */
                                    /* analytical computation.*/

/*
 * Global variables to handle signal parameters.
 */

extern int  sig_form;    /* Direct and fourier property. CV_RC_FORM or */
                         /* CV_CC_FORM. */
extern void *sig_d_data; /* Storage of direct numerical data */
extern void *sig_f_data; /* Storage of fourier numerical data */
extern int  sig_n;       /* Size of the data arrays */

/*
 * Global variable to store requested convolution method (CV_D, CV_MP or CV_FT)
 */

extern int cv_method;

enum {
  DIRECT_CONVOLUTION,
  MULTI_PART_CONVOLUTION,
  FOURIER_TRANSFORM_CONVOLUTION
};

enum {
  DIRECT_LIM,
  MULTI_PART_LIM
};

#define LIMITS_TAB_SIZE 18

/*
 * Internal functions defined in cv_misc.c.
 */

extern int       cv_next_power_of_2_ (int);
extern int       cv_is_power_of_2_   (int);

extern void      cv_cplx_mult_            (complex *, complex *, int, int);
extern void      cv_cplx_mult_num_ana_    (complex *, double (*)(), double (*)(), int, int, real, real);
extern void      cv_cplx_mult_num_ana_1p_ (complex *a, double (*)(), double (*)(), double, int, int, real, real);

extern int       cv_convolution_method_ (int, int, int[LIMITS_TAB_SIZE][2]);

extern real *    cv_pure_periodic_extend_      (real *, int, int, int);
extern real *    cv_mirror_transform_          (real *, int, int);
extern real *    cv_padding_transform_         (real *, int, int);
extern real *    cv_0_padding_transform_ (real *source_data,int  size,int  cut);
extern complex * cv_pure_cplx_periodic_extend_ (complex *, int, int, int);
extern complex * cv_cplx_mirror_transform_     (complex *, int, int);
extern complex * cv_cplx_padding_transform_    (complex *, int, int);
extern complex * cv_cplx_0_padding_transform_    (complex *, int, int);

extern void cv_get_part_r_ (real *, int, real *, int, int, int);
extern void cv_get_part_c_ (complex *, int, complex *, int, int, int);

extern void      set_f_l_exact (int *, int *);

#define EX_RAISE(exception) goto exception

#endif /* _CV_INT_H_ */
