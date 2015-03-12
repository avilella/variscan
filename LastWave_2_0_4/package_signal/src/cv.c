/*
 * cv.c --
 *
 *   Copyright 1998 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: cv.c,v 1.3 1998/07/13 16:23:11 decoster Exp $
 */


#include "lastwave.h"
#include "cv_int.h"


extern void (*cv_fft_r)  (real    *in, complex *out, int n);
extern void (*cv_fft_r_i)(complex *in, real    *out, int n);
extern void (*cv_fft_c)  (complex *in, complex *out, int n);
extern void (*cv_fft_c_i)(complex *in, complex *out, int n);



#ifdef LOG_MESSAGES
char* be[4] = {
  "pe", "mi", "pa", "0p"
};
#endif

/*
 * Global variables to handle filter parameters.
 */

int    flt_form = CV_UNDEFINED; /* Direct and fourier property. CV_RR_FORM... */
int    flt_def = CV_UNDEFINED;  /* Kind of definition : NUMERICAL or */
                                /* ANALYTICAL. */
void * flt_d_data = 0;          /* Storage of direct numerical data */
void * flt_f_data = 0;          /* Storage of fourier numerical data */
int    flt_d_n = 0;             /* Size of the direct data arrays */
real   flt_d_begin;             /* Domain of the direct form. Used for */
real   flt_d_end;               /* analytical computation.*/
int    flt_f_n = 0;             /* Size of the fourier data arrays */
real   flt_f_begin;             /* Domain of the fourier form. Used for */
real   flt_f_end;               /* analytical computation.*/
double (* flt_d_real_ptr)();    /* Function pointers to filter defs for */
double (* flt_d_imag_ptr)();    /* its direct and its fourier forms. Used */
double (* flt_f_real_ptr)();    /* for analytical computation. */
double (* flt_f_imag_ptr)();
real   flt_scale = CV_NO_SCALE; /* Scale to apply to the filter. Used for */
                                /* analytical computation.*/

/*
 * Global variables to handle signal parameters.
 */

int  sig_form = CV_UNDEFINED; /* Direct and fourier property. CV_RC_FORM or */
                              /* CV_CC_FORM. */
void *sig_d_data = 0;         /* Storage of direct numerical data */
void *sig_f_data = 0;         /* Storage of fourier numerical data */
int  sig_n = 0;               /* Size of the data arrays */

/*
 * Global variable to store requested convolution method (CV_DI, CV_MP or CV_FT)
 */

int cv_method = CV_UNDEFINED;

/*
 * cv_flt_init_a --
 * This function initialize the parameters for an analytical filter.
 * No return value.
 */

void
cv_flt_init_a (real d_begin,
	       real d_end,
	       real f_begin,
	       real f_end,
	       double (* d_real_ptr)(),
	       double (* d_imag_ptr)(),
	       double (* f_real_ptr)(),
	       double (* f_imag_ptr)(),
	       real scale)
{
  int i_begin, i_end;

  assert ((d_begin <= 0) && (d_end >= 0));
  assert ((f_begin <= 0) && (f_end >= 0));
  assert (d_real_ptr != 0);
  assert (f_real_ptr != 0);
  assert ((scale > 0) || (scale == CV_NO_SCALE));

  if (d_imag_ptr == 0) {
    if (f_imag_ptr == 0) {
      flt_form = CV_RR_FORM;
    } else {
      flt_form = CV_RC_FORM;
    }
  } else {
    flt_form = CV_CC_FORM;
  }

  flt_def = ANALYTICAL;
  flt_d_data = 0;
  flt_f_data = 0;

  if (scale == CV_NO_SCALE) {
    i_begin = (int) floor (d_begin);
    i_end   = (int) ceil  (d_end);
  } else {
    i_begin = (int) floor (d_begin*scale);
    i_end   = (int) ceil  (d_end*scale);
  }
  flt_d_n = i_end - i_begin +1;

  if (scale == CV_NO_SCALE) {
    i_begin = (int) floor (f_begin);
    i_end   = (int) ceil  (f_end);
  } else {
    i_begin = (int) floor (f_begin/scale);
    i_end   = (int) ceil  (f_end/scale);
  }
  flt_f_n = i_end - i_begin +1;

  /* For now, we store the minimum acceptable numerical filter size in */
  /* flt_d_n. */
  /*  flt_d_n = max(flt_d_n, flt_f_n);*/

  flt_d_begin = d_begin;
  flt_d_end = d_end;
  flt_f_begin = f_begin;
  flt_f_end = f_end;
  flt_d_real_ptr = d_real_ptr;
  flt_d_imag_ptr = d_imag_ptr;
  flt_f_real_ptr = f_real_ptr;
  flt_f_imag_ptr = f_imag_ptr;
  flt_scale = scale;

  return;
}


/*
 * cv_flt_init_n --
 * This function initialize the parameters for a numerical filter.
 * No return value.
 */

void
cv_flt_init_n (int form,
	       int d_n,
	       int d_0_index,
	       int f_n,
	       int f_0_index,
	       void * d_data,
	       void * f_data)
{
  assert ((form == CV_RR_FORM) || (form == CV_RC_FORM) || (form == CV_CC_FORM));
  assert (d_n > 0);
  assert (d_0_index < d_n);
  assert (d_data != 0);
  if (f_data != 0) {
    assert (f_n > 0);
    assert (f_0_index < f_n);
    assert (d_n > f_n);
  }

  flt_form = form;
  flt_def = NUMERICAL;
  flt_d_data = d_data;
  flt_f_data = f_data;
  flt_d_n = d_n;
  flt_d_begin = (real) -d_0_index;
  flt_d_end = (real) d_n - d_0_index - 1;
  if (f_data != 0) {
    flt_f_begin = (real) -f_0_index;
    flt_f_end = (real) f_n - f_0_index - 1;
    flt_f_n = f_n;
  } else {
    flt_f_begin = 0;
    flt_f_end = 0;
  }

  /* For now, we store the minimum acceptable numerical filter size in */
  /* flt_d_n. */

  flt_d_n = max(flt_d_n, flt_f_n);

  flt_d_real_ptr = 0;
  flt_d_imag_ptr = 0;
  flt_f_real_ptr = 0;
  flt_f_imag_ptr = 0;
  flt_scale = CV_NO_SCALE;

  return;
}


/*
 * cv_sig_init --
 * This function initialize the parameters for a numerical signal.
 * No return value.
 */

void
cv_sig_init (int  form,
	     void * d_data,
	     void * f_data,
	     int  n)
{
  assert ((form == CV_RR_FORM) || (form == CV_RC_FORM) || (form == CV_CC_FORM));
  assert (d_data != 0);
  assert (n > 0);

  sig_form = form;
  sig_d_data = d_data;
  sig_f_data = f_data;
  sig_n = n;

  return;
}


/*
 * cv_set_method --
 */

void
cv_set_method (int method)
{
  assert ((method == CV_UNDEFINED)
	  || (method == CV_DI)
	  || (method == CV_MP)
	  || (method == CV_FT));

  cv_method = method;
}

/*
 * cv_compute --
 */

void *
cv_compute (int  border_effect,
	    void *res_data,
	    int  *first_exact_ptr,
	    int  *last_exact_ptr)
{
  void * ret_value = 0;
  real * old_sig_d_data_r;
  complex * old_sig_d_data_c;
  complex * old_flt_d_data_c;
  complex * old_res_data_c;

  real    * sig_d_data_r = 0;
  complex * sig_d_data_c = 0;
  real    * flt_d_data_r = 0;
  real    * res_data_r   = 0;

  int  i;

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

  LogMessage2("%s ", be[border_effect]);

  switch (flt_def) {
  case ANALYTICAL:
    switch (sig_form) {
    case CV_RC_FORM:
      switch (flt_form) {

      case CV_RR_FORM: /* signal RC - filter RR */
      case CV_RC_FORM: /* signal RC - filter RC */
	ret_value = (void *)
	  cv_a_real (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	break;

      case CV_CC_FORM: /* signal RC - filter CC */
	old_sig_d_data_r = (real *) sig_d_data;

	/* We transform the signal into a complex signal. Create a new array. */

	sig_d_data_c = (complex *) Malloc (sig_n*sizeof (complex));
	if (!sig_d_data_c) {
	  EX_RAISE(mem_alloc_error);
	}

	for (i = 0; i < sig_n; i++) {
	  sig_d_data_c[i].real = old_sig_d_data_r[i];
	  sig_d_data_c[i].imag = 0.0;
	}

	/* Point the global var to the good array, then compute. */

	sig_d_data = (void *) sig_d_data_c;
	ret_value = (void *)
	  cv_a_cplx (border_effect, res_data, first_exact_ptr, last_exact_ptr);

	/* Restore the old state. */

	free (sig_d_data_c);

	sig_d_data = (void *) old_sig_d_data_r;
	break;
      }
      break;
    case CV_CC_FORM:
      switch (flt_form) {

      case CV_RR_FORM: /* signal CC - filter RR */
      case CV_RC_FORM: /* signal CC - filter RC */
	old_sig_d_data_c = (complex *) sig_d_data;
	old_res_data_c = (complex *) res_data;

	/* We transform the signal into a real signal that contains the real */
	/* part. Create a new array. */

	sig_d_data_r = (real *) Malloc (sig_n*sizeof (real));
	if (!sig_d_data_r) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < sig_n; i++) {
	  sig_d_data_r[i] = old_sig_d_data_c[i].real;
	}

	res_data_r = (real *) Malloc (sig_n*sizeof (real));
	if (!res_data) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Point the global var to the good array, then compute. */

	sig_d_data = (void *) sig_d_data_r;
	ret_value = (void *)
	  cv_a_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);

	/* Fill the result array. */

	for (i = 0; i < sig_n; i++) {
	  old_res_data_c[i].real = res_data_r[i];
	}

	/* We transform the signal into a real signal that contains the */
	/* imaginary part. */

	for (i = 0; i < sig_n; i++) {
	  sig_d_data_r[i] = old_sig_d_data_c[i].imag;
	}

	/* Point the global var to the good array, then compute. */

	sig_d_data = (void *) sig_d_data_r;
	ret_value = (void *)
	  cv_a_real (border_effect, res_data, first_exact_ptr, last_exact_ptr);

	/* Fill the result array. */

	for (i = 0; i < sig_n; i++) {
	  old_res_data_c[i].imag = res_data_r[i];
	}

	/* Restore the old state. */

	free (res_data_r);
	free (sig_d_data_r);

	sig_d_data = (void *) old_sig_d_data_c;
	res_data = (void *) old_res_data_c;
	break;

      case CV_CC_FORM: /* signal CC - filter CC */
	ret_value = (void *)
	  cv_a_cplx (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	break;
      }
      break;
    }
    break;
  case NUMERICAL:
    switch (sig_form) {

    case CV_RC_FORM:
      switch (flt_form) {

      case CV_RR_FORM: /* signal RC - filter RR */
      case CV_RC_FORM: /* signal RC - filter RC */
	ret_value = (void *)
	  cv_n_real (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	break;

      case CV_CC_FORM: /* signal RC - filter CC */
	old_flt_d_data_c = (complex *) flt_d_data;
	old_res_data_c = (complex *) res_data;
	
	/* We transform the filter into a real signal that contains the real */
	/* part. Create a new array. */

	flt_d_data_r = (real *) Malloc (flt_d_n*sizeof (real));
	if (!flt_d_data_r) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < flt_d_n; i++) {
	  flt_d_data_r[i] = old_flt_d_data_c[i].real;
	}

	res_data_r = (real *) Malloc (sig_n*sizeof (real));
	if (!res_data_r) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Point the global var to the good array, then compute. */

	flt_d_data = (void *) flt_d_data_r;
	ret_value = (void *)
	  cv_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);
	for (i = 0; i < sig_n; i++) {
	  old_res_data_c[i].real = res_data_r[i];
	}

	/* We transform the filter into a real signal that contains the */
	/* imaginary part. */

	for (i = 0; i < flt_d_n; i++) {
	  flt_d_data_r[i] = old_flt_d_data_c[i].imag;
	}

	/* Point the global var to the good array, then compute. */

	flt_d_data = (void *) flt_d_data_r;
	ret_value = (void *)
	  cv_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);

	/* Fill the result array. */

	for (i = 0; i < sig_n; i++) {
	  old_res_data_c[i].imag = res_data_r[i];
	}

	/* Restore the old state. */

	free (res_data_r);
	free (flt_d_data_r);

	flt_d_data = (void *) old_flt_d_data_c;
	res_data = (void *) old_res_data_c;
	break;
      }
      break;

    case CV_CC_FORM:
      switch (flt_form) {

      case CV_RR_FORM: /* signal CC - filter RR */
      case CV_RC_FORM: /* signal CC - filter RC */
	old_sig_d_data_c = (complex *) sig_d_data;
	old_res_data_c = (complex *) res_data;

	/* We transform the signal into a real signal that contains the real */
	/* part. Create a new array. */

	sig_d_data_r = (real *) malloc (sig_n*sizeof (real));
	if (!sig_d_data_r) {
	  EX_RAISE(mem_alloc_error);
	}
	for (i = 0; i < sig_n; i++) {
	  sig_d_data_r[i] = old_sig_d_data_c[i].real;
	}

	res_data_r = (real *) malloc (sig_n*sizeof (real));
	if (!res_data_r) {
	  EX_RAISE(mem_alloc_error);
	}

	/* Point the global var to the good array, then compute. */

	sig_d_data = (void *) sig_d_data_r;
	ret_value = (void *)
	  cv_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);

	/* Fill the result array. */

	for (i = 0; i < sig_n; i++) {
	  old_res_data_c[i].real = res_data_r[i];
	}

	/* We transform the signal into a real signal that contains the real */
	/* part. */

	for (i = 0; i < sig_n; i++) {
	  sig_d_data_r[i] = old_sig_d_data_c[i].imag;
	}

	/* Point the global var to the good array, then compute. */

	sig_d_data = (void *) sig_d_data_r;
	ret_value = (void *)
	  cv_n_real (border_effect, res_data_r, first_exact_ptr, last_exact_ptr);

	/* Fill the result array. */

	for (i = 0; i < sig_n; i++) {
	  old_res_data_c[i].imag = res_data_r[i];
	}

	/* Restore the old state. */

	free (res_data_r);
	free (sig_d_data_r);

	sig_d_data = (void *) old_sig_d_data_c;
	res_data = (void *) old_res_data_c;
	break;

      case CV_CC_FORM: /* signal CC - filter CC */
	ret_value = (void *)
	  cv_n_cplx (border_effect, res_data, first_exact_ptr, last_exact_ptr);
	break;
      }
      break;
    }
    break;
  }

  assert (ret_value != 0);

  LogMessage("\n");

  return ret_value;

mem_alloc_error:
  free (sig_d_data_r);
  free (sig_d_data_c);
  free (res_data_r);
  free (flt_d_data_r);

  return 0;
}


