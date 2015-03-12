/*
 * cv_n.c --
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv_n.c,v 1.6 1998/08/20 12:57:25 decoster Exp $
 */

#include "lastwave.h"
#include <string.h>

#include "cv_int.h"
#include "cv_limits.h"

/*
 * Array to store all the basic numerical convolution fonctions. Stored by
 * method.
 */

void * (* cv_n_fct_ptr_array[2][3])() = {
  {
    cv_n_real_d,
    cv_n_real_mp,
    cv_n_real_ft
  }, {
    cv_n_cplx_d,
    cv_n_cplx_mp,
    cv_n_cplx_ft
  }
};

/*
 * cv_n_real --
 */
void *
cv_n_real (int  border_effect,
	   void *res_data,
	   int  *first_exact_ptr,
	   int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv_fct_ptr)();


  LogMessage("n r ");

  if (cv_method == CV_UNDEFINED) {
    method =
      cv_convolution_method_ (sig_n, flt_d_n, lim_array[NUMERICAL][border_effect]);
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
  the_cv_fct_ptr = cv_n_fct_ptr_array[REAL][method];

  SetLogTimeBegin();
  ret_value = 
    the_cv_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


/*
 * cv_n_real_d --
 */
void *
cv_n_real_d (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  real *filter_data;
  real *signal_data;
  real *result_data;
  int  i, j1, j2;
  int  begin, end; /* Begin and end of the filter */

  assert (flt_def == NUMERICAL);
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

  begin = (int) flt_d_begin;
  end   = (int) flt_d_end;

  filter_data = (real *) (flt_d_data) - begin;
  signal_data = (real *) sig_d_data;
  result_data = (real *) res_data;

  for (i = 0; i < sig_n; i++) {
    result_data[i] = 0;
    if ((sig_n - 1 - i + begin) < 0) {
      for (j1 = 0, j2 = i;
	   j2 >= (i - end);
	   j1++, j2--) {
	result_data[i] += filter_data[j1] * signal_data[j2];
      }

      switch (border_effect) {
      case CV_PERIODIC:
	for (j1 = begin, j2 = i - begin - sig_n;
	     j2 >= 0;
	     j1++, j2--) {
	  result_data[i] += filter_data[j1] * signal_data[j2];
	}
	break;
      case CV_MIRROR:
	for (j1 = begin, j2 = 2*sig_n - 2 - i + begin;
	     j2 <= sig_n - 2;
	     j1++, j2++) {
	  result_data[i] += filter_data[j1] * signal_data[j2];
	}
	break;
      case CV_PADDING:
	for (j1 = begin, j2 = 2*sig_n - 2 - i + begin;
	     j2 <= sig_n - 2;
	     j1++, j2++) {
	  result_data[i] += filter_data[j1] * signal_data[sig_n - 1];
	}
	break;
      case CV_0_PADDING:
	break;
      }

      for (j2 = sig_n-1;
	   j2 >= (i + 1);
	   j1++, j2--) {
	result_data[i] += filter_data[j1] * signal_data[j2];
      }
    } else if ((i-end) < 0) {
      for (j1 = 0, j2 = i;
	   j2 >= 0;
	   j1++, j2--) {
	result_data[i] += filter_data[j1] * signal_data[j2];
      }

      switch (border_effect) {
      case CV_PERIODIC:
	for (j2 = sig_n-1;
	     j2 >= (i - end + sig_n);
	     j1++, j2--) {
	  result_data[i] += filter_data[j1] * signal_data[j2];
	}
	break;
      case CV_MIRROR:
	for (j2 = 1;
	     j2 <= (end - i);
	     j1++, j2++) {
	  result_data[i] += filter_data[j1] * signal_data[j2];
	}
	break;
      case CV_PADDING:
	for (j2 = 1;
	     j2 <= (end - i);
	     j1++, j2++) {
	  result_data[i] += filter_data[j1] * signal_data[0];
	}
	break;
      case CV_0_PADDING:
	break;
      }

      for (j1 = begin, j2 = (i - begin);
	   j2 >= (i + 1);
	   j1++, j2--) {
	result_data[i] += filter_data[j1] * signal_data[j2];
      }
    }
    else
      {
	for (j1 = 0, j2 = i;
	     j2 >= (i - end);
	     j1++, j2--) {
	  result_data[i] += filter_data[j1] * signal_data[j2];
	}

	for (j1 = begin, j2 = (i - begin);
	     j2 >= (i + 1);
	     j1++, j2--) {
	  result_data[i] += filter_data[j1] * signal_data[j2];
	}
      }
  }

  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;
}


/*
 * cv_n_real_mp --
 */
void *
cv_n_real_mp (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  real    *signal_data;
  real    *result_data;
  real    *filter_data;
  real    *signal_part;
  real    *new_filter;
  complex *signal_part_ft;
  complex *new_filter_ft;
  int     nb_of_parts, part_nb, part_size;
  int     size_of_exact_data;
  int     filter_begin;
  int     filter_end;

  assert (flt_def == NUMERICAL);
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
  filter_data = (real *) flt_d_data;
  result_data = (real *) res_data;

  filter_begin = (int) flt_d_begin;
  filter_end   = (int) flt_d_end;

  part_size = cv_next_power_of_2_ (2*flt_d_n);
  size_of_exact_data = part_size - flt_d_n + 1;

  new_filter = cv_pure_periodic_extend_ (filter_data,
					 filter_begin,
					 filter_end,
					 part_size);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter_ft = (complex *) malloc (sizeof (complex)*part_size/2);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv_fft_r (new_filter, new_filter_ft, part_size);

  nb_of_parts = ceil (((double) sig_n)/size_of_exact_data);

  signal_part = (real *) malloc (sizeof (real)*part_size);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }
  signal_part_ft = (complex *) malloc (sizeof (complex)*part_size/2);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }

  for (part_nb = 0; part_nb < nb_of_parts; part_nb++) {
    int part_begin_in_signal;

    part_begin_in_signal = part_nb*size_of_exact_data - filter_end;
    cv_get_part_r_ (signal_part, part_size,
		    signal_data, sig_n,
		    part_begin_in_signal,
		    border_effect);

    cv_fft_r (signal_part, signal_part_ft, part_size);

    signal_part_ft[0].real *= new_filter_ft[0].real;
    signal_part_ft[0].imag *= new_filter_ft[0].imag;
    cv_cplx_mult_ (signal_part_ft, new_filter_ft, 1, part_size/2 - 1);

    cv_fft_r_i (signal_part_ft, signal_part, part_size);

    if (part_nb < (nb_of_parts - 1)) {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + filter_end,
	      size_of_exact_data*sizeof (real));
    } else {
      memcpy (result_data + part_nb*size_of_exact_data,
	      signal_part + filter_end,
	      (sig_n - part_nb*(size_of_exact_data))*sizeof (real));
    }
  }

  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}


/*
 * cv_n_real_ft --
 */
void *
cv_n_real_ft (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  real    *signal_data;
  real    *filter_data;
  int     filter_begin;
  int     filter_end;
  real    *result_data;
  real    *new_signal;
  real    *new_filter;
  complex *new_signal_ft;
  complex *new_filter_ft;
  int     new_size;
  
  assert (flt_def == NUMERICAL);
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
  filter_data = (real *) flt_d_data;
  result_data = (real *) res_data;

  filter_begin = (int) flt_d_begin;
  filter_end   = (int) flt_d_end;

  switch (border_effect) {
  case CV_0_PADDING:
    new_size = sig_n*2;
    new_signal = cv_0_padding_transform_ (signal_data,
					  sig_n,
					  filter_end);
	break;
  case CV_PADDING:
    new_size = sig_n*2;
    new_signal = cv_padding_transform_ (signal_data,
					sig_n,
					filter_end);
    break;
  case CV_MIRROR:
    new_size = sig_n*2;
    new_signal = cv_mirror_transform_ (signal_data,
				       sig_n,
				       filter_end);
    break;
  case CV_PERIODIC:
    new_size = sig_n;
    new_signal = signal_data;
    break;
  }
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter = cv_pure_periodic_extend_ (filter_data,
					 filter_begin,
					 filter_end,
					 new_size);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter_ft = (complex *) malloc (sizeof (complex)*new_size/2);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv_fft_r (new_filter, new_filter_ft, new_size);

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size/2);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }

  cv_fft_r (new_signal, new_signal_ft, new_size);

  new_signal_ft[0].real *= new_filter_ft[0].real;
  new_signal_ft[0].imag *= new_filter_ft[0].imag;
  cv_cplx_mult_ (new_signal_ft, new_filter_ft, 1, new_size/2-1);
  
  if (border_effect == CV_PERIODIC)
    {
      cv_fft_r_i (new_signal_ft, result_data, new_size);
    }
  else
    {
      cv_fft_r_i (new_signal_ft, new_signal, new_size);
      memcpy (result_data, new_signal, sig_n*sizeof (real));
      free (new_signal);
    }

  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  if (border_effect != CV_PERIODIC) {
    free (new_signal);
  }
  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}


/*
 * cv_n_cplx --
 */
void *
cv_n_cplx (int  border_effect,
	   void *res_data,
	   int  *first_exact_ptr,
	   int  *last_exact_ptr)
{
  int method;
  real * ret_value = 0;
  void * (*the_cv_fct_ptr)();

  LogMessage("n c ");
  if (cv_method == CV_UNDEFINED) {
    method =
      cv_convolution_method_ (sig_n, flt_d_n, lim_array[NUMERICAL][border_effect]);
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
  the_cv_fct_ptr = cv_n_fct_ptr_array[CPLX][method];
  SetLogTimeBegin();
  ret_value = 
    the_cv_fct_ptr (border_effect, res_data, first_exact_ptr, last_exact_ptr);
  LogTime();

  return ret_value;
}


#define cplx_mult_and_add(r,a,b) (r).real+=(a).real*(b).real-(a).imag*(b).imag;\
(r).imag+=(a).real*(b).imag+(a).imag*(b).real

/*
 * cv_n_cplx_d --
 */
void *
cv_n_cplx_d (int  border_effect,
	     void *res_data,
	     int  *first_exact_ptr,
	     int  *last_exact_ptr)
{
  complex *signal_data;
  complex *filter_data;
  complex *result_data;
  int     i, j1, j2;
  int     begin, end; /* Begin and end of the filter */

  assert (flt_def == NUMERICAL);
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

  begin = (int) flt_d_begin;
  end   = (int) flt_d_end;

  filter_data = (complex *) flt_d_data - begin;
  signal_data = (complex *) sig_d_data;
  result_data = (complex *) res_data;

  for (i = 0; i < sig_n; i++) {
    result_data[i].real = 0.0;
    result_data[i].imag = 0.0;
    if ((sig_n - 1 - i + begin) < 0) {
      for (j1 = 0, j2 = i;
	   j2 >= (i - end);
	   j1++, j2--) {
	cplx_mult_and_add (result_data[i],
			   filter_data[j1],
			   signal_data[j2]);
      }
      switch (border_effect) {
      case CV_PERIODIC:
	for (j1 = begin, j2 = i - begin - sig_n;
	     j2 >= 0;
	     j1++, j2--) {
	  cplx_mult_and_add (result_data[i],
			     filter_data[j1],
			     signal_data[j2]);
	}
	break;
      case CV_MIRROR:
	for (j1 = begin, j2 = 2*sig_n - 2 - i + begin;
	     j2 <= sig_n - 2;
	     j1++, j2++) {
	  cplx_mult_and_add (result_data[i],
			     filter_data[j1],
			     signal_data[j2]);
	}
	break;
      case CV_PADDING:
	for (j1 = begin, j2 = 2*sig_n - 2 - i + begin;
	     j2 <= sig_n - 2;
	     j1++, j2++) {
	  cplx_mult_and_add (result_data[i],
			     filter_data[j1],
			     signal_data[sig_n - 1]);
	}
	break;
      case CV_0_PADDING:
	break;
      }

      for (j2 = sig_n-1;
	   j2 >= (i + 1);
	   j1++, j2--) {
	cplx_mult_and_add (result_data[i],
			   filter_data[j1],
			   signal_data[j2]);
      }
    } else if ((i-end) < 0) {
      for (j1 = 0, j2 = i;
	   j2 >= 0;
	   j1++, j2--) {
	cplx_mult_and_add (result_data[i],
			   filter_data[j1],
			   signal_data[j2]);
      }

      switch (border_effect) {
      case CV_PERIODIC:
	for (j2 = sig_n-1;
	     j2 >= (i - end + sig_n);
	     j1++, j2--) {
	  cplx_mult_and_add (result_data[i],
			     filter_data[j1],
			     signal_data[j2]);
	}
	break;
      case CV_MIRROR:
	for (j2 = 1;
	     j2 <= (end - i);
	     j1++, j2++) {
	  cplx_mult_and_add (result_data[i],
			     filter_data[j1],
			     signal_data[j2]);
	}
	break;
      case CV_PADDING:
	for (j2 = 1;
	     j2 <= (end - i);
	     j1++, j2++) {
	  cplx_mult_and_add (result_data[i],
			     filter_data[j1],
			     signal_data[0]);
	}
	break;
      case CV_0_PADDING:
	break;
      }

      for (j1 = begin, j2 = (i - begin);
	   j2 >= (i + 1);
	   j1++, j2--) {
	cplx_mult_and_add (result_data[i],
			   filter_data[j1],
			   signal_data[j2]);
      }
    } else {
      for (j1 = 0, j2 = i;
	   j2 >= (i - end);
	   j1++, j2--) {
	cplx_mult_and_add (result_data[i],
			   filter_data[j1],
			   signal_data[j2]);
      }

      for (j1 = begin, j2 = (i - begin);
	   j2 >= (i + 1);
	   j1++, j2--) {
	cplx_mult_and_add (result_data[i],
			   filter_data[j1],
			   signal_data[j2]);
      }
    }
  }

  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;
}


/*
 * cv_n_cplx_mp --
 */
void *
cv_n_cplx_mp (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  complex *signal_data;
  complex *filter_data;
  int     filter_begin;
  int     filter_end;
  complex *result_data;
	
  complex *signal_part;
  complex *new_filter;
  complex *signal_part_ft;
  complex *new_filter_ft;
  int     nb_of_parts, part_nb, part_size;
  int     size_of_exact_data;

  assert (flt_def == NUMERICAL);
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
  filter_data = (complex *) flt_d_data;
  result_data = (complex *) res_data;

  filter_begin = (int) floor (flt_d_begin);
  filter_end   = (int) ceil  (flt_d_end);

  part_size = cv_next_power_of_2_ (2*flt_d_n);
  size_of_exact_data = part_size - flt_d_n + 1;

  new_filter = cv_pure_cplx_periodic_extend_ (filter_data,
					      filter_begin,
					      filter_end,
					      part_size);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }
  new_filter_ft = (complex *) malloc (sizeof (complex)*part_size);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }  cv_fft_c (new_filter, new_filter_ft, part_size);

  nb_of_parts = ceil (((double) sig_n)/size_of_exact_data);

  signal_part = (complex *) malloc (sizeof (complex)*part_size);
  if (!signal_part) {
    EX_RAISE(mem_alloc_error);
  }  signal_part_ft = (complex *) malloc (sizeof (complex)*part_size);
  if (!signal_part_ft) {
    EX_RAISE(mem_alloc_error);
  }
  for (part_nb = 0; part_nb < nb_of_parts; part_nb++)
    {
      int part_begin_in_signal;

      part_begin_in_signal = part_nb*size_of_exact_data - filter_end;
      cv_get_part_c_ (signal_part, part_size,
		      signal_data, sig_n,
		      part_begin_in_signal,
		      border_effect);

      cv_fft_c (signal_part, signal_part_ft, part_size);

      cv_cplx_mult_ (signal_part_ft, new_filter_ft, 0, part_size - 1);

      cv_fft_c_i (signal_part_ft, signal_part, part_size);

      if (part_nb < (nb_of_parts - 1))
	memcpy (result_data + part_nb*size_of_exact_data,
		signal_part + filter_end,
		size_of_exact_data*sizeof (complex));
      else
	memcpy (result_data + part_nb*size_of_exact_data,
		signal_part + filter_end,
		(sig_n - part_nb*(size_of_exact_data))*sizeof (complex));
    }

  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);
  
  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;

mem_alloc_error:
  free (signal_part);
  free (signal_part_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}


/*
 * cv_n_cplx_ft --
 */
void *
cv_n_cplx_ft (int  border_effect,
	      void *res_data,
	      int  *first_exact_ptr,
	      int  *last_exact_ptr)
{
  complex *signal_data;
  complex *filter_data;
  int     filter_begin;
  int     filter_end;
  complex *result_data;
  complex *new_signal;
  complex *new_filter;
  complex *new_signal_ft;
  complex *new_filter_ft;
  int     new_size;
  
  assert (flt_def == NUMERICAL);
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

  signal_data = (complex *) sig_d_data;
  filter_data = (complex *) flt_d_data;
  result_data = (complex *) res_data;

  filter_begin = (int) flt_d_begin;
  filter_end   = (int) flt_d_end;

  switch (border_effect)
    {
    case CV_0_PADDING:
      new_size = sig_n*2;
      new_signal = cv_cplx_0_padding_transform_ (signal_data,
						 sig_n,
						 filter_end);
      break;
    case CV_PADDING:
      new_size = sig_n*2;
      new_signal = cv_cplx_padding_transform_ (signal_data,
					       sig_n,
					       filter_end);
      break;
    case CV_MIRROR:
      new_size = sig_n*2;
      new_signal = cv_cplx_mirror_transform_ (signal_data,
					      sig_n,
					      filter_end);
      break;
    case CV_PERIODIC:
      new_size = sig_n;
      new_signal = signal_data;
      break;
    }
  if (!new_signal) {
    EX_RAISE(mem_alloc_error);
  }

  new_filter = cv_pure_cplx_periodic_extend_ (filter_data,
					      filter_begin,
					      filter_end,
					      new_size);
  if (!new_filter) {
    EX_RAISE(mem_alloc_error);
  }
  new_filter_ft = (complex *) malloc (sizeof (complex)*new_size);
  if (!new_filter_ft) {
    EX_RAISE(mem_alloc_error);
  }  cv_fft_c (new_filter, new_filter_ft, new_size);

  new_signal_ft = (complex *) malloc (sizeof (complex)*new_size);
  if (!new_signal_ft) {
    EX_RAISE(mem_alloc_error);
  }  cv_fft_c (new_signal, new_signal_ft, new_size);

  cv_cplx_mult_ (new_signal_ft, new_filter_ft, 0, new_size - 1);
  
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
  free (new_filter);
  free (new_filter_ft);
  

  set_f_l_exact (first_exact_ptr, last_exact_ptr);

  return res_data;
mem_alloc_error:
  free (new_signal);
  free (new_signal_ft);
  free (new_filter);
  free (new_filter_ft);

  return 0;
}

