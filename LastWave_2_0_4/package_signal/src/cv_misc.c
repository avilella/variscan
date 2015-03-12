/*
 * cv_misc.c --
 *
 *  Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster, April 1997.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *  or  decoster@info.enserb.u-bordeaux.fr
 *
 *   $Id: cv_misc.c,v 1.5 1998/09/09 16:41:46 decoster Exp $
 */

#include "lastwave.h"
#include <string.h>
#include "cv_int.h"

/*
 * _my_log2_ --
 * 
 *  New log2 function on integers.
 *
 * Arguments :
 *   i - value to compute. Must be greater or equal than 1.
 *
 * Return Value :
 *   log2(i).
 */

static int
_my_log2_ (int i)
{
  register int tmp, index;

  assert (i >= 1);

  tmp = i;
  index = 0;
  while (tmp != 1) {
    tmp /= 2;
    index++;
  }

  return index;
}


/*
 */
static void
_real_copy_ (real *dest,
	     real *src,
	     int  begin,
	     int  end)
{
  assert (dest != 0);
  assert (src != 0);
  assert (end >= begin);

  memcpy (dest, src + begin, (end - begin + 1)*sizeof (real)); 
}


/*
 */
static void
_cplx_copy_ (complex *dest,
	     complex *src,
	     int  begin,
	     int  end)
{
  assert (dest != 0);
  assert (src != 0);
  assert (end >= begin);

  memcpy (dest, src + begin, (end - begin + 1)*sizeof (complex)); 
}


/*
 */
int
cv_next_power_of_2_ (int i)
{
  int j;

  for (j = 1; j < i; j *= 2);

  return j;
}


/*
 */
int
cv_is_power_of_2_ (int i)
{
  real j;

  for (j = i; j > 1; j /= 2);

  return (j == 1);
}


/*
 */
void
cv_cplx_mult_ (complex *a,
	       complex *b,
	       int     begin,
	       int     end)
{
  complex      tmp;
  register int i;

  assert (a != 0);
  assert (b != 0);

  for (i = begin; i <= end; i++) {
    tmp = a[i];
    a[i].real = a[i].real*b[i].real - a[i].imag*b[i].imag;
    a[i].imag = tmp.real*b[i].imag  + tmp.imag*b[i].real;
  }
}


/* 
 * Compute from BEGIN to END the multiplication between an array of
 * complex data and a complex function. The complex function is given
 * by 2 C-function pointer, one for its real part and one for its
 * imaginary part. If the pointer for the imaginary part is set to
 * NULL, it is considered the complex function has no imaginary
 * part.
 */ 
void
cv_cplx_mult_num_ana_ (complex *a,
		       double  (*b_real) (),
		       double  (*b_imag) (),
		       int     begin,
		       int     end,
		       real    b_scale,
		       real    b_begin)
{
  register complex tmp;
  register real    tmp2;
  register int     i;

  assert (a != 0);
  assert (b_real != 0);

  if (b_imag) {
    for (i = begin; i <= end; i++) {
      tmp = a[i];
      tmp2 = b_begin + i*b_scale;
      a[i].real = (a[i].real*(*b_real)((double)(tmp2))
		   - a[i].imag*(*b_imag)((double)(tmp2)));
      a[i].imag = (tmp.real*(*b_imag)((double)(tmp2))
		   + tmp.imag*(*b_real)((double)(tmp2)));
    }
  } else {
    for (i = begin; i <= end; i++) {
      tmp2 = (*b_real)((double)(b_begin+i*b_scale));
      a[i].real *= tmp2;
      a[i].imag *= tmp2;
    }
  }
}


/* 
 * Compute from BEGIN to END the multiplication between an array of
 * complex data and a complex function. The complex function is given
 * by 2 C-function pointer, one for its real part and one for its
 * imaginary part. If the pointer for the imaginary part is set to
 * NULL, it is considered the complex function has no imaginary
 * part.
 */ 
void
cv_cplx_mult_num_ana_1p_ (complex *a,
			  double  (*b_real) (),
			  double  (*b_imag) (),
			  double  param,
			  int     begin,
			  int     end,
			  real    b_scale,
			  real    b_begin)
{
  register complex tmp;
  register real    tmp2;
  register int     i;

  assert (a != 0);
  assert (b_real != 0);

  if (b_imag) {
    for (i = begin; i <= end; i++) {
      tmp = a[i];
      tmp2 = b_begin + i*b_scale;
      a[i].real = (a[i].real*(*b_real)((double)(tmp2), param)
		   - a[i].imag*(*b_imag)((double)(tmp2), param));
      a[i].imag = (tmp.real*(*b_imag)((double)(tmp2), param)
		   + tmp.imag*(*b_real)((double)(tmp2), param));
    }
  } else {
    for (i = begin; i <= end; i++) {
      tmp2 = (*b_real)((double)(b_begin+i*b_scale), param);
      a[i].real *= tmp2;
      a[i].imag *= tmp2;
    }
  }
}


/*
 * Return the convolution method to use with a signal of SIGNAL_SIZE
 * data, and a filter of FILTER_SIZE data.
 */
int
cv_convolution_method_ (int signal_size,
			int filter_size,
			int limits_tab[LIMITS_TAB_SIZE][2])
{
  int index;
  
  assert (signal_size >= filter_size);

  index = _my_log2_ (signal_size) - 1;

  if (index >= LIMITS_TAB_SIZE) {
    index = LIMITS_TAB_SIZE - 1;
  }

  if (filter_size <= limits_tab[index][DIRECT_LIM]) {
    return DIRECT_CONVOLUTION;
  }
  else if (filter_size <= limits_tab[index][MULTI_PART_LIM]) {
    return MULTI_PART_CONVOLUTION;
  }
  else {
    return FOURIER_TRANSFORM_CONVOLUTION;
  }
}


/*
 * RESULT is a periodic signal which period is NEW_SIZE. Between BEGIN
 * and END, RESULT equals to SOURCE. Outside this, it equals to
 * 0.0. RESULT contains data from 0 to NEW_SIZE-1 (one period).
 *
 *                     |
 *                   /"|"""\
 *   transform      /  |    """\  
 *               __/   |        \__
 *               B     0          E    B = BEGIN, E = END
 *
 *              |                                |
 *              |"""\                          /"|
 *   into       |    """\                     /  |
 *              |        \___________________/   |
 *              +                                +
 *              0                               N-1   N = NEW_SIZE
 *
 *
 * Real data.
 */
real *
cv_pure_periodic_extend_ (real *source_data,
			  int  begin,
			  int  end,
			  int  new_size)
{
  real *result, *source;
  int  i;

  assert (begin <= 0);
  assert (end >= 0);
  assert (source_data != 0);
  assert (new_size >= (end - begin +1));

  source = source_data - begin;
  result = (real *) malloc (sizeof (real) * new_size);
  if (!result) {
    return 0;
  }

  for (i = 0; i <= end; i++) {
    result[i] = source[i];
  }
  for (; i < (new_size + begin); i++) {
    result[i] = 0.0;
  }
  for (; i < new_size; i++) {
    result[i] = source[i - new_size];
  }

  return result;
}


/*
 * Complex data.
 */
complex *
cv_pure_cplx_periodic_extend_ (complex *source_data,
			       int     begin,
			       int     end,
			       int     new_size)
{
  complex *result, *source;
  int  i;

  assert (source_data != 0);
  assert (new_size >= (end - begin +1));

  source = source_data - begin;
  result = (complex *) malloc (sizeof (complex) * new_size);
  if (!result) {
    return 0;
  }

  for (i = 0; i <= end; i++) {
    result[i] = source[i];
  }
  for (; i < (new_size + begin); i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }
  for (; i < new_size; i++) {
    result[i] = source[i - new_size];
  }

  return result;
}


/*
 * Create a new signal from signal : 
 *  - which size is SIZE multiply by 2,
 *  - its period is equal to this new_size,
 *  - the data are organized from 0 to new_size-1,
 *  - additionnal data are :
 *
 *              |
 *              |__/"""""""\
 *   transform  |           \__/"""  
 *              +                 +
 *              0                 S-1       S = SIZE
 *
 *              |
 *   into       |__/"""""""\             /""..""""\__
 *              |           \__/"""""\__/  
 *              +                 +                 +
 *              0                 S-1               2*S-1
 *                                            <----->
 *                                              CUT
 * Real data.
 */
real *
cv_mirror_transform_ (real *source_data,
		      int  size,
		      int  cut)
{
  real *result;
  int  i;
  int  new_size = size*2;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(real));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[new_size - i - 2];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[new_size - i];
  }

  return result;
}


/*
 * Complex data.
 */
complex *
cv_cplx_mirror_transform_ (complex *source_data,
			   int     size,
			   int     cut)
{
  complex *result;
  int     i;
  int     new_size = size*2;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(complex));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[new_size - i - 2];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[new_size - i];
  }

  return result;
}


/*
 * Create a new signal from signal : 
 *  - which size is SIZE multiply by 2,
 *  - its period is equal to this new_size,
 *  - the data are organized from 0 to new_size-1,
 *  - additionnal data are :
 *
 *              |
 *              |__/"""""""\
 *   transform  |           \__/"""  
 *              +                 +
 *              0                 S-1       S = SIZE
 *
 *              |
 *   into       |__/"""""""\                .._______
 *              |           \__/""""""""""""  
 *              +                 +                 +
 *              0                 S-1               2*S-1
 *                                            <----->
 *                                              CUT
 * Real data. "
 */
real *
cv_padding_transform_ (real *source_data,
		       int  size,
		       int  cut)
{
  real *result;
  int  i;
  int  new_size = size*2;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(real));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[size - 1];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[0];
  }

  return result;
}


real *
cv_0_padding_transform_ (real *source_data,
			 int  size,
			 int  cut)
{
  real *result;
  int  i;
  int  new_size = size*2;

  assert (source_data != 0);

  result = (real *) malloc (sizeof (real)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(real));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = 0.0;
  }
  for (; i < new_size; i++) {
    result[i] = 0.0;
  }

  return result;
}


/*
 * Complex data.
 */
complex *
cv_cplx_padding_transform_ (complex *source_data,
			    int     size,
			    int     cut)
{
  complex *result;
  int     i;
  int     new_size = size*2;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(complex));
  for (i = size; i <= (new_size - cut); i++) {
    result[i] = source_data[size - 1];
  }
  for (; i < new_size; i++) {
    result[i] = source_data[0];
  }

  return result;
}


/*
 * Complex data.
 */
complex *
cv_cplx_0_padding_transform_ (complex *source_data,
			      int     size,
			      int     cut)
{
  complex *result;
  int     i;
  int     new_size = size*2;

  assert (source_data != 0);

  result = (complex *) malloc (sizeof (complex)*new_size);
  if (!result) {
    return 0;
  }

  memcpy (result, source_data, size*sizeof(complex));
  for (i = size; i <= (new_size - cut); i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }
  for (; i < new_size; i++) {
    result[i].real = 0.0;
    result[i].imag = 0.0;
  }

  return result;
}


void
_get_part_r_pe_ (real *signal_part,
		 int  part_size,
		 real *signal_data,
		 int  signal_size,
		 int  part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int k;
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  if (j < 0) {
    tmp_n = -j;
    k = signal_size + j;
    _real_copy_ (signal_part + i, signal_data + k, 0, tmp_n - 1);
    i += tmp_n;
    j += tmp_n;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _real_copy_ (signal_part + i, signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    tmp_n = min (part_size - i, signal_size);
    _real_copy_ (signal_part + i, signal_data, 0, tmp_n - 1);
    i += tmp_n;
    j += tmp_n;
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
_get_part_r_mi_ (real *signal_part,
		 int  part_size,
		 real *signal_data,
		 int  signal_size,
		 int  part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int k;
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  while (j < 0) {
    signal_part[i] = signal_data[-j];
    i++;
    j++;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _real_copy_ (signal_part + i, signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    /*
     * Mirrored signal
     */

    tmp_n = min (part_size - i, signal_size);
    for (k = signal_size - 1; k >= signal_size - tmp_n; k--, i++, j++) {
      signal_part[i] = signal_data[k];
    }

    /*
     * Normal signal.
     */

    if (i < part_size) {
      tmp_n = min (part_size - i, signal_size);
      _real_copy_ (signal_part + i, signal_data + j, 0, tmp_n - 1);
    }
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
_get_part_r_pa_ (real *signal_part,
		 int  part_size,
		 real *signal_data,
		 int  signal_size,
		 int  part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  while (j < 0) {
    signal_part[i] = signal_data[0];
    i++;
    j++;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _real_copy_ (signal_part + i,
	       signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    signal_part[i] = signal_data[signal_size - 1];
    i++;
    j++;
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
_get_part_r_0p_ (real *signal_part,
		 int  part_size,
		 real *signal_data,
		 int  signal_size,
		 int  part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  while (j < 0) {
    signal_part[i] = 0.0;
    i++;
    j++;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _real_copy_ (signal_part + i,
	       signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    signal_part[i] = 0.0;
    i++;
    j++;
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
cv_get_part_r_ (real *signal_part,
		int  part_size,
		real *signal_data,
		int  signal_size,
		int  part_begin_in_signal,
		int  border_effect)
{
  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  switch (border_effect) {
  case CV_PERIODIC:
    _get_part_r_pe_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV_MIRROR:
    _get_part_r_mi_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV_PADDING:
    _get_part_r_pa_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV_0_PADDING:
    _get_part_r_0p_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  }
}


void
_get_part_c_pe_ (complex *signal_part,
		 int     part_size,
		 complex *signal_data,
		 int     signal_size,
		 int     part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int k;
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  if (j < 0) {
    tmp_n = -j;
    k = signal_size + j;
    _cplx_copy_ (signal_part + i, signal_data + k, 0, tmp_n - 1);
    i += tmp_n;
    j += tmp_n;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _cplx_copy_ (signal_part + i, signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    tmp_n = min (part_size - i, signal_size);
    _cplx_copy_ (signal_part + i, signal_data, 0, tmp_n - 1);
    i += tmp_n;
    j += tmp_n;
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
_get_part_c_mi_ (complex *signal_part,
		 int     part_size,
		 complex *signal_data,
		 int     signal_size,
		 int     part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int k;
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  while (j < 0) {
    signal_part[i] = signal_data[-j];
    i++;
    j++;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _cplx_copy_ (signal_part + i, signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    /*
     * Mirrored signal
     */

    tmp_n = min (part_size - i, signal_size);
    for (k = signal_size - 1; k >= signal_size - tmp_n; k--, i++, j++) {
      signal_part[i] = signal_data[k];
    }

    /*
     * Normal signal.
     */

    if (i < part_size) {
      tmp_n = min (part_size - i, signal_size);
      _cplx_copy_ (signal_part + i, signal_data + j, 0, tmp_n - 1);
    }
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
_get_part_c_pa_ (complex *signal_part,
		 int     part_size,
		 complex *signal_data,
		 int     signal_size,
		 int     part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  while (j < 0) {
    signal_part[i] = signal_data[0];
    i++;
    j++;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _cplx_copy_ (signal_part + i,
	       signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    signal_part[i] = signal_data[signal_size - 1];
    i++;
    j++;
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
_get_part_c_0p_ (complex *signal_part,
		 int     part_size,
		 complex *signal_data,
		 int     signal_size,
		 int     part_begin_in_signal)
{
  int i;	/* Index for signal_part. */
  int j;	/* Index for signal_data. */
  int tmp_n;

  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  i = 0;
  j = part_begin_in_signal;

  /*
   * Left border.
   */

  while (j < 0) {
    signal_part[i].real = 0.0;
    signal_part[i].imag = 0.0;
    i++;
    j++;
  }

  /*
   * Middle.
   */

  tmp_n = min (part_size - i, signal_size - j);
  _cplx_copy_ (signal_part + i,
	       signal_data + j, 0, tmp_n - 1);
  i += tmp_n;
  j += tmp_n;

  /*
   * Right border.
   */

  while (i < part_size) {
    signal_part[i].real = 0.0;
    signal_part[i].imag = 0.0;
    i++;
    j++;
  }

  assert (i == part_size);
  assert (j == part_size+part_begin_in_signal);

  return;
}


void
cv_get_part_c_ (complex *signal_part,
		int     part_size,
		complex *signal_data,
		int     signal_size,
		int     part_begin_in_signal,
		int     border_effect)
{
  assert (signal_part != 0);
  assert (signal_data != 0);
  assert (part_size > 0);
  assert (signal_size > 0);

  switch (border_effect) {
  case CV_PERIODIC:
    _get_part_c_pe_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV_MIRROR:
    _get_part_c_mi_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV_PADDING:
    _get_part_c_pa_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  case CV_0_PADDING:
    _get_part_c_0p_ (signal_part, part_size,
		     signal_data, signal_size,
		     part_begin_in_signal);
    break;
  }
}


/*
 */
void
set_f_l_exact (int *first_exact_ptr,
	       int *last_exact_ptr)
{
  int i_begin;
  int i_end;

  switch (flt_def) {
  case ANALYTICAL:
    if (flt_scale == CV_NO_SCALE) {
      i_begin = (int) floor (flt_d_begin);
      i_end   = (int) ceil  (flt_d_end);
    } else {
      i_begin = (int) floor (flt_d_begin*flt_scale);
      i_end   = (int) ceil  (flt_d_end*flt_scale);
    }
    *first_exact_ptr = (int) i_end;
    *last_exact_ptr  = sig_n + (int) i_begin - 1;
    break;
  case NUMERICAL:
    *first_exact_ptr = (int) flt_d_end;
    *last_exact_ptr  = sig_n + (int) flt_d_begin - 1;
    break;
  }
}
