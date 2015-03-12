/*
 * cv_a.c --
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv_a.c,v 1.6 1998/09/09 16:40:26 decoster Exp $
 */

#include "lastwave.h"
#include <string.h>

#include "cv_int.h"
#include "cv_limits.h"

#ifndef M_PI 
#define M_PI 3.14159265358979323846
#endif


/*
 * Array to store all the basic analytical convolution fonctions. Stored by
 * method.
 */

void * (* cv_a_fct_ptr_array[2][3])() = {
  {
    cv_a_real_d,
    cv_a_real_mp,
    cv_a_real_ft
  }, {
    cv_a_cplx_d,
    cv_a_cplx_mp,
    cv_a_cplx_ft
  }
};

/*
 * cv_a_real --
 */

void *
cv_a_real (int  border_effect,
	   void *res_data,
	   int  *first_exact_ptr,
	   int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv_fct_ptr)();

  LogMessage("a r ");
  if (cv_method == CV_UNDEFINED) {
    method =
      cv_convolution_method_ (sig_n, flt_d_n, lim_array[ANALYTICAL][border_effect]);
  } else {
    method = cv_method;
  }
  if ((method == FOURIER_TRANSFORM_CONVOLUTION)
      && !cv_is_power_of_2_ (sig_n)) {
    method = MULTI_PART_CONVOLUTION;
  }

#ifdef LOG_MESSAGES
    LogMessage2("%s ", method_str[method]);
    LogMessage2("%d ", sig_n);
    LogMessage2("%d ", flt_d_n);
#endif
  the_cv_fct_ptr = cv_a_fct_ptr_array[REAL][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


/*
 * cv_a_real_d --
 */

void *
cv_a_real_d (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  int  i;
  real *filter_data = 0;
  int  filter_begin_index;
  int  filter_end_index;

  int  old_flt_def;
  int  old_flt_d_begin;
  int  old_flt_d_end;
  void * old_flt_d_data;

  assert (flt_def == ANALYTICAL);
  assert (flt_form != CV_UNDEFINED);
  assert (sig_form != CV_UNDEFINED);
  assert (sig_n >= flt_d_n);

  assert ((border_effect == CV_PERIODIC)
	  || (border_effect == CV_MIRROR)
	  || (border_effect == CV_PADDING)
	  || (border_effect == CV_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  if (flt_scale == CV_NO_SCALE) {
    filter_begin_index = (int) floor (flt_d_begin) ;
    filter_end_index = (int) ceil (flt_d_end) ;
  } else {
    filter_begin_index = (int) floor (flt_d_begin*flt_scale) ;
    filter_end_index = (int) ceil (flt_d_end*flt_scale) ;
  }

  filter_data = (real *) malloc (sizeof(real)*flt_d_n);
  if (!filter_data) {
    EX_RAISE(mem_alloc_error);
  }

  if (flt_scale == CV_NO_SCALE) {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index] =
	(* flt_d_real_ptr)((double)(i));
    }
  } else {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index] =
	(* flt_d_real_ptr)((double)(i), flt_scale);
    }
  }

  old_flt_def = flt_def;
  old_flt_d_begin = flt_d_begin;
  old_flt_d_end = flt_d_end;
  old_flt_d_data = flt_d_data;
  flt_def = NUMERICAL;
  flt_d_begin = (real) filter_begin_index;
  flt_d_end = (real) filter_end_index;
  flt_d_data = (void *) filter_data;

  result_data = cv_n_real_d (border_effect, result_data,
			     first_exact_ptr, last_exact_ptr);

  free (filter_data);

  flt_def = old_flt_def;
  flt_d_begin = old_flt_d_begin;
  flt_d_end = old_flt_d_end;
  flt_d_data = old_flt_d_data;

  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return result_data;

mem_alloc_error:
  free (filter_data);

  return 0;
}


/*
 * cv_a_real_mp --
 */

void *
cv_a_real_mp (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  real    *signal_part = 0;
  complex *signal_part_ft = 0;
  complex *filter_ft = 0;

  int     nb_of_parts, part_nb, part_n;
  int     size_of_exact_data;
  int     i;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     flt_d_end_index;
  real    f_step;

  assert (flt_def == ANALYTICAL);
  assert (flt_form != CV_UNDEFINED);
  assert (sig_form != CV_UNDEFINED);
  assert (sig_n >= flt_d_n);

  assert ((border_effect == CV_PERIODIC)
	  || (border_effect == CV_MIRROR)
	  || (border_effect == CV_PADDING)
	  || (border_effect == CV_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  part_n = cv_next_power_of_2_ (2*flt_d_n);
  size_of_exact_data = part_n - flt_d_n + 1;

  f_step = 2*M_PI/(part_n);

  nb_of_parts = ceil (((double) sig_n)/size_of_exact_data);

  assert (nb_of_parts >= 1);

  if (flt_scale == CV_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt_d_begin/f_step) ;
    flt_f_end_index = (int) ceil (flt_d_end/f_step) ;
    flt_d_end_index = (int) ceil (flt_d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt_f_begin/flt_scale/f_step);
    flt_f_end_index   = (int) ceil  (flt_f_end/flt_scale/f_step);
    flt_d_end_index = (int) ceil (flt_d_end*flt_scale) ;
  }

  signal_part = (real *) malloc (sizeof (real)*part_n);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_n/2);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  filter_ft = (complex *) malloc (sizeof (complex)*part_n/2);
  if (!filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  filter_ft[0].real = (* flt_f_real_ptr)(0.0, flt_scale);
  filter_ft[0].imag =
    (* flt_f_real_ptr)((double)(part_n*f_step/2), flt_scale);
  if (flt_scale == CV_NO_SCALE) {
    if (flt_f_imag_ptr) {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step));
	filter_ft[i].imag =
	  (* flt_f_imag_ptr)((double)(i*f_step));
      }
    } else {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step));
	filter_ft[i].imag = 0.0;
      }    
    }
  } else { /* flt_scale != CV_NO_SCALE */
    if (flt_f_imag_ptr) {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step), flt_scale);
	filter_ft[i].imag =
	  (* flt_f_imag_ptr)((double)(i*f_step), flt_scale);
      }
    } else {
      for (i = 1; i < part_n/2; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step), flt_scale);
	filter_ft[i].imag = 0.0;
      }    
    }
  }

  for (part_nb = 0; part_nb < nb_of_parts; part_nb++) {
    int part_begin_in_signal;

    part_begin_in_signal = part_nb*size_of_exact_data - flt_d_end_index;
    cv_get_part_r_ (signal_part, part_n,
		    signal_data, sig_n,
		    part_begin_in_signal,
		    border_effect);


    cv_fft_r (signal_part, signal_part_ft, part_n);

    signal_part_ft[0].real *= filter_ft[0].real;
    signal_part_ft[0].imag *= filter_ft[0].imag;
    cv_cplx_mult_ (signal_part_ft, filter_ft,
		   1, part_n/2 - 1);

    cv_fft_r_i (signal_part_ft, signal_part, part_n);

    if (part_nb < (nb_of_parts - 1)) {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      size_of_exact_data*sizeof (real));
    } else {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      (sig_n - part_nb*(size_of_exact_data))*sizeof (real));
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);

  return 0;
}


/*
 * cv_a_real_ft --
 */

void *
cv_a_real_ft (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  real   *signal_data;
  real   *result_data;

  real    *new_signal = 0;
  complex *new_signal_ft = 0;

  int     new_size;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     flt_d_begin_index;
  int     flt_d_end_index;
  real    f_step;
  
  assert (flt_def == ANALYTICAL);
  assert (flt_form != CV_UNDEFINED);
  assert (sig_form != CV_UNDEFINED);
  assert (sig_n >= flt_d_n);

  assert ((border_effect == CV_PERIODIC)
	  || (border_effect == CV_MIRROR)
	  || (border_effect == CV_PADDING)
	  || (border_effect == CV_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);
  assert (cv_is_power_of_2_ (sig_n));

  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  switch (border_effect)
    {
    case CV_0_PADDING:
      new_size = sig_n*2;
      break;
    case CV_PADDING:
      new_size = sig_n*2;
      break;
    case CV_MIRROR:
      new_size = sig_n*2;
      break;
    case CV_PERIODIC:
      new_size = sig_n;
      break;
    }
  f_step = 2*M_PI/(new_size);

  if (flt_scale == CV_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt_f_begin) ;
    flt_f_end_index = (int) ceil (flt_f_end) ;
    flt_d_begin_index = (int) floor (flt_d_begin) ;
    flt_d_end_index = (int) ceil (flt_d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt_f_begin/flt_scale/f_step);
    flt_f_end_index   = (int) ceil  (flt_f_end/flt_scale/f_step);
    flt_d_begin_index = (int) floor (flt_d_begin*flt_scale);
    flt_d_end_index   = (int) ceil  (flt_d_end*flt_scale);
  }

  assert (new_size >= (flt_f_end_index - flt_f_begin_index +1));

  switch (border_effect)
    {
    case CV_0_PADDING:
      new_size = sig_n*2;
      new_signal = cv_0_padding_transform_ (signal_data,
					    sig_n,
					    flt_d_end_index);
      break;
    case CV_PADDING:
      new_size = sig_n*2;
      new_signal = cv_padding_transform_ (signal_data,
					  sig_n,
					  flt_d_end_index);
      break;
    case CV_MIRROR:
      new_size = sig_n*2;
      new_signal = cv_mirror_transform_ (signal_data,
					 sig_n,
					 flt_d_end_index);
      break;
    case CV_PERIODIC:
      new_size = sig_n;
      new_signal = signal_data;
      break;
    }
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size/2);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv_fft_r (new_signal, new_signal_ft, new_size);

  if (flt_scale == CV_NO_SCALE) {
    new_signal_ft[0].real *= (* flt_f_real_ptr) (0.0);
    new_signal_ft[0].imag *= (* flt_f_real_ptr) ((double)new_size/2*f_step);
    cv_cplx_mult_num_ana_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			 1, new_size/2 - 1, f_step, 0.0);
  } else {
    new_signal_ft[0].real *= (* flt_f_real_ptr) (0.0, flt_scale);
    new_signal_ft[0].imag *=
      (* flt_f_real_ptr) ((double)new_size/2*f_step, flt_scale);
    cv_cplx_mult_num_ana_1p_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			    flt_scale, 1, new_size/2 - 1, f_step, 0.0);
  }
  
  if (border_effect == CV_PERIODIC) {
      cv_fft_r_i (new_signal_ft, result_data, new_size);
    }
  else {
      cv_fft_r_i (new_signal_ft, new_signal, new_size);
      memcpy (result_data, new_signal, sig_n*sizeof (real));
      free (new_signal);
    }

  free (new_signal_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);
  return res_data;

mem_alloc_error:
  free (new_signal_ft);
  free (new_signal);


  return 0;
}


/*
 * cv_a_cplx --
 */

void *
cv_a_cplx (int  border_effect,
	   void *res_data,
	   int  *first_exact_ptr,
	   int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv_fct_ptr)();

  LogMessage("a c ");
  if (cv_method == CV_UNDEFINED) {
    method =
      cv_convolution_method_ (sig_n, flt_d_n, lim_array[ANALYTICAL][border_effect]);
  } else {
    method = cv_method;
  }
  if ((method == FOURIER_TRANSFORM_CONVOLUTION)
      && !cv_is_power_of_2_ (sig_n)) {
    method = MULTI_PART_CONVOLUTION;
  }

#ifdef LOG_MESSAGES
  LogMessage2("%s ", method_str[method]);
  LogMessage2("%d ", sig_n);
  LogMessage2("%d ", flt_d_n);
#endif
  the_cv_fct_ptr = cv_a_fct_ptr_array[CPLX][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();
  return ret_value;
}


/*
 * cv_a_cplx_d --
 */
void *
cv_a_cplx_d (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  int  i;

  complex *filter_data = 0;

  int  filter_begin_index;
  int  filter_end_index;

  int  old_flt_def;
  int  old_flt_d_begin;
  int  old_flt_d_end;
  void * old_flt_d_data;

  assert (flt_def == ANALYTICAL);
  assert (flt_form != CV_UNDEFINED);
  assert (sig_form != CV_UNDEFINED);
  assert (sig_n >= flt_d_n);

  assert ((border_effect == CV_PERIODIC)
	  || (border_effect == CV_MIRROR)
	  || (border_effect == CV_PADDING)
	  || (border_effect == CV_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  if (flt_scale == CV_NO_SCALE) {
    filter_begin_index = (int) ceil (flt_d_begin) ;
    filter_end_index = (int) ceil (flt_d_end) ;
  } else {
    filter_begin_index = (int) ceil (flt_d_begin*flt_scale) ;
    filter_end_index = (int) ceil (flt_d_end*flt_scale) ;
  }


  filter_data = (complex *) malloc (sizeof(complex)*flt_d_n);
  if (!filter_data) {
    EX_RAISE(mem_alloc_error);
  }

  if (flt_scale == CV_NO_SCALE) {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index].real =
	(* flt_d_real_ptr)((double)(i));
      filter_data[i - filter_begin_index].imag =
	(* flt_d_imag_ptr)((double)(i));
    }
  } else {
    for (i = filter_begin_index; i <= filter_end_index; i++) {
      filter_data[i - filter_begin_index].real =
	(* flt_d_real_ptr)((double)(i), flt_scale);
      filter_data[i - filter_begin_index].imag =
	(* flt_d_imag_ptr)((double)(i), flt_scale);
    }
  }

  old_flt_def = flt_def;
  old_flt_d_begin = flt_d_begin;
  old_flt_d_end = flt_d_end;
  old_flt_d_data = flt_d_data;
  flt_def = NUMERICAL;
  flt_d_begin = (real) filter_begin_index;
  flt_d_end = (real) filter_end_index;
  flt_d_data = (void *) filter_data;

  result_data = cv_n_cplx_d (border_effect, result_data,
			     first_exact_ptr, last_exact_ptr);

  free (filter_data);

  flt_def = old_flt_def;
  flt_d_begin = old_flt_d_begin;
  flt_d_end = old_flt_d_end;
  flt_d_data = old_flt_d_data;

  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (filter_data);

  return 0;
}


/*
 * cv_a_cplx_mp --
 */

void *
cv_a_cplx_mp (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  complex *signal_part = 0;
  complex *signal_part_ft = 0 ;
  complex *filter_ft = 0;

  int     nb_of_parts, part_nb, part_n;
  int     size_of_exact_data;
  int     i;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     flt_d_end_index;
  real    f_step;

  assert (flt_def == ANALYTICAL);
  assert (flt_form != CV_UNDEFINED);
  assert (sig_form != CV_UNDEFINED);
  assert (sig_n >= flt_d_n);

  assert ((border_effect == CV_PERIODIC)
	  || (border_effect == CV_MIRROR)
	  || (border_effect == CV_PADDING)
	  || (border_effect == CV_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  part_n = cv_next_power_of_2_ (2*flt_d_n);
  size_of_exact_data = part_n - flt_d_n + 1;

  nb_of_parts = ceil (((double) sig_n)/size_of_exact_data);
  assert (nb_of_parts >= 1);

  f_step = 2*M_PI/(part_n);

  if (flt_scale == CV_NO_SCALE) {
    flt_f_begin_index = (int) floor (flt_f_begin/f_step) ;
    flt_f_end_index = (int) ceil (flt_f_end/f_step) ;
    flt_d_end_index = (int) ceil (flt_d_end) ;
  } else {
    flt_f_begin_index = (int) floor (flt_f_begin/flt_scale/f_step);
    flt_f_end_index   = (int) ceil  (flt_f_end/flt_scale/f_step);
    flt_d_end_index   = (int) ceil  (flt_d_end*flt_scale);
  }

  signal_part = (complex *) malloc (sizeof (complex)*part_n);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_n);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  filter_ft = (complex *) malloc (sizeof (complex)*part_n);
  if (!filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  /* Fill filter fourier form array. */

  if (flt_scale == CV_NO_SCALE) {
    if (flt_f_imag_ptr) {
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step));
	filter_ft[i].imag =
	  (* flt_f_imag_ptr)((double)(i*f_step));
      }
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)((i-flt_f_end_index)*f_step));
	filter_ft[i].imag =
	  (* flt_f_imag_ptr)((double)((i-flt_f_end_index)*f_step));
      }
    } else { /* No function pointer for the imaginary part. */
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step));
	filter_ft[i].imag = 0.0;
      }    
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)((i-flt_f_end_index)*f_step));
	filter_ft[i].imag = 0.0;
      }
    }
  } else { /* flt_scale != CV_NO_SCALE */
    if (flt_f_imag_ptr) {
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step), flt_scale);
	filter_ft[i].imag =
	  (* flt_f_imag_ptr)((double)(i*f_step), flt_scale);
      }
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)((i-flt_f_end_index)*f_step), flt_scale);
	filter_ft[i].imag =
	  (* flt_f_imag_ptr)((double)((i-flt_f_end_index)*f_step), flt_scale);
      }
    } else { /* No function pointer for the imaginary part. */
      for (i = 0; i <= flt_f_end_index; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)(i*f_step), flt_scale);
	filter_ft[i].imag = 0.0;
      }    
      for (; i < part_n+flt_f_begin_index; i++) {
	filter_ft[i].real = 0.0;
	filter_ft[i].imag = 0.0;
      }
      for (; i < part_n; i++) {
	filter_ft[i].real =
	  (* flt_f_real_ptr)((double)((i-flt_f_end_index)*f_step), flt_scale);
	filter_ft[i].imag = 0.0;
      }
    }
  }

  for (part_nb = 0; part_nb < nb_of_parts; part_nb++) {
    int part_begin_in_signal;

    part_begin_in_signal = part_nb*size_of_exact_data - flt_d_end_index;
    cv_get_part_c_ (signal_part, part_n,
		    signal_data, sig_n,
		    part_begin_in_signal,
		    border_effect);

    cv_fft_c (signal_part, signal_part_ft, part_n);

    cv_cplx_mult_ (signal_part_ft, filter_ft,
		   0, part_n - 1);

    cv_fft_c_i (signal_part_ft, signal_part, part_n);

    if (part_nb < (nb_of_parts - 1)) {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      size_of_exact_data*sizeof (complex));
    } else {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + flt_d_end_index,
	      (sig_n - part_nb*(size_of_exact_data))*sizeof (complex));
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (filter_ft);

  return 0;
}


/*
 * cv_a_cplx_ft --
 */

void *
cv_a_cplx_ft (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  complex *signal_data;
  complex *result_data;

  complex *new_signal = 0;
  complex *new_signal_ft = 0;

  int     new_size;
  int     flt_f_begin_index;
  int     flt_f_end_index;
  int     filter_ft_size;
  real    f_step;
  int     tmp_index;
  
  assert (flt_def == ANALYTICAL);
  assert (flt_form != CV_UNDEFINED);
  assert (sig_form != CV_UNDEFINED);
  assert (sig_n >= flt_d_n);

  assert ((border_effect == CV_PERIODIC)
	  || (border_effect == CV_MIRROR)
	  || (border_effect == CV_PADDING)
	  || (border_effect == CV_0_PADDING));
  assert (res_data != 0);
  assert (first_exact_ptr != 0);
  assert (last_exact_ptr != 0);

  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  switch (border_effect)
    {
    case CV_0_PADDING:
      new_size = sig_n*2;
      break;
    case CV_PADDING:
      new_size = sig_n*2;
      break;
    case CV_MIRROR:
      new_size = sig_n*2;
      break;
    case CV_PERIODIC:
      new_size = sig_n;
      break;
    }
  f_step = 2*M_PI/(new_size);

  if (flt_scale == CV_NO_SCALE) {
    flt_f_begin_index = (int) ceil (flt_d_begin/f_step) ;
    flt_f_end_index = (int) ceil (flt_d_end/f_step) ;
  } else {
    flt_f_begin_index = (int) floor (flt_f_begin/flt_scale/f_step);
    flt_f_end_index   = (int) ceil  (flt_f_end/flt_scale/f_step);
  }

  filter_ft_size = flt_f_end_index - flt_f_begin_index + 1;
  assert (sig_n >= filter_ft_size);

  switch (border_effect)
    {
    case CV_0_PADDING:
      new_signal = cv_cplx_0_padding_transform_ (signal_data,
					     sig_n,
					     flt_f_end_index);
      break;
    case CV_PADDING:
      new_signal = cv_cplx_padding_transform_ (signal_data,
					     sig_n,
					     flt_f_end_index);
      break;
    case CV_MIRROR:
      new_signal = cv_cplx_mirror_transform_ (signal_data,
					    sig_n,
					    flt_f_end_index);
      break;
    case CV_PERIODIC:
      new_signal = signal_data;
      break;
    }
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv_fft_c (new_signal, new_signal_ft, new_size);

  if (flt_scale == CV_NO_SCALE) {
    tmp_index = flt_f_end_index + (new_size - filter_ft_size)/2;
    cv_cplx_mult_num_ana_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			 0, tmp_index - 1, f_step,
			 0.0);
    cv_cplx_mult_num_ana_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			 tmp_index, new_size - 1, f_step,
			 (real)((- new_size)*f_step));


    new_signal_ft[0].real *= (* flt_f_real_ptr) (0.0);
    new_signal_ft[0].imag *= (* flt_f_real_ptr) ((double)new_size/2*f_step);
    cv_cplx_mult_num_ana_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			 1, new_size/2 - 1, f_step, 0.0);
  } else {
    tmp_index = flt_f_end_index + (new_size - filter_ft_size)/2;
    cv_cplx_mult_num_ana_1p_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			    flt_scale, 0, tmp_index - 1, f_step,
			    0.0);
    cv_cplx_mult_num_ana_1p_ (new_signal_ft, flt_f_real_ptr, flt_f_imag_ptr,
			    flt_scale, tmp_index, new_size - 1, f_step,
			    (real)((- new_size)*f_step));
  }
  
  if (border_effect == CV_PERIODIC)
    {
      cv_fft_c_i (new_signal_ft, result_data, new_size);
    }
  else
    {
      cv_fft_c_i (new_signal_ft, new_signal, new_size);
      memcpy (result_data, new_signal, sig_n*sizeof (complex));
      free (new_signal);
    }

  free (new_signal_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (new_signal);
  free (new_signal_ft);

  return 0;
}

