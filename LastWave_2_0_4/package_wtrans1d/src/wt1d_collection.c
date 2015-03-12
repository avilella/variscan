/*
 * collection.c
 *
 * Definition of usual wavelets.
 *
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *   $Id: collection.c,v 1.4 1998/07/08 11:52:00 decoster Exp $
 */

#include "wt1d_int.h"

#define SQRT_PI  1.77245385090551602729
#define SQRT_2PI 2.50662827463100050241

/*
 *  For all the predefined wavelets the domains are defined as [x_min x_max],
 * with :
 *   x_min : the first point that is greater than 0.000001*(max value)
 *   x_max : the last point that is greater than 0.000001*(max value)
 */

double
null_fct (double x,
	  double a)
{
  return 0.0;
}


/*
 * Definition of the gaussian wavelet.
 */

#define gfact 1.7
#define sqrt2log10 2.14597 /* sqrt(2*log(10)) */
#define geps 2.44949  /* The square root of epsilon (and error is 10^epsilon */

double
d_r_gauss (double x,
	   double a)
{
  register double x_a = x/(a*gfact);
  return exp(-x_a*x_a/2);
}

double
f_r_gauss (double k,
	   double a)
{
  register double ka = k*a;
  return SQRT_2PI*a*exp(-ka*ka/2);
}

Wavelet _gaussian_ = {
  WAVE_REAL_REAL,
  &d_r_gauss,
  NULL,
  &f_r_gauss,
  NULL,
  -gfact*sqrt2log10*geps,
  gfact*sqrt2log10*geps,
  -5.44921875,
  5.44921875,
  1.0,
  1.0
};

Wavelet *wt1d_gaussian_ptr = &_gaussian_;

/*
 * Definition of the first derivative of the gaussian wavelet.
 */

#define g1fact 2.8

double
d_r_d1_gauss (double x,
	      double a)
{
  register double x_a = x/(a*g1fact);
  return x_a*exp(-x_a*x_a/2);
}

double
f_i_d1_gauss (double k,
	      double a)
{
  register double ka = k*a;
  return SQRT_2PI*a*ka*exp(-ka*ka/2);
}

Wavelet _d1_gaussian_ = {
  WAVE_REAL_CPLX,
  &d_r_d1_gauss,
  NULL,
  &null_fct,
  &f_i_d1_gauss,
  -6*g1fact,
  6*g1fact,
  -5.76171875,
  5.76171875,
  1.0,
  1.0
};

Wavelet *wt1d_d1_gaussian_ptr = &_d1_gaussian_;

/*
 * Definition of the second derivative of the gaussian wavelet.
 */

#define g2fact 3

double
d_r_d2_gauss (double x,
	      double a)
{
  register double x_a = x/(a*g2fact);
  register double x_a2 = x_a*x_a;
  return (x_a2-1)*exp(-x_a2/2);
}

double
f_r_d2_gauss (double k,
	      double a)
{
  register double ka = k*a;
  register double ka2 = ka*ka;
  return -SQRT_2PI*a*ka2*exp(-ka2/2);
}

Wavelet _d2_gaussian_ = {
  WAVE_REAL_REAL,
  &d_r_d2_gauss,
  NULL,
  &f_r_d2_gauss,
  NULL,
  -6*g2fact,
  6*g2fact,
  -6.09375,
  6.09375,
  1.0,
  1.0
};

Wavelet *wt1d_d2_gaussian_ptr = &_d2_gaussian_;

/*
 * Definition of the third derivative of the gaussian wavelet.
*/

#define g3fact 3.2

double
d_r_d3_gauss (double x,
	      double a)
{
  register double x_a = x/(a*g3fact);
  register double x_a2 = x_a*x_a;
  return -x_a*(3-x_a2)*exp(-x_a2/2);
}

double
f_i_d3_gauss (double k,
	      double a)
{
  register double ka = k*a;
  register double ka2 = ka*ka;
  return -SQRT_2PI*a*ka*ka2*exp(-ka2/2);
}

Wavelet _d3_gaussian_ = {
  WAVE_REAL_CPLX,
  &d_r_d3_gauss,
  NULL,
  &null_fct,
  &f_i_d3_gauss,
  -6.3*g3fact,
  6.3*g3fact,
  -6.40625,
  6.40625,
  1.0,
  1.0
};

Wavelet *wt1d_d3_gaussian_ptr = &_d3_gaussian_;

/*
 * Definition of the fourth derivative of the gaussian wavelet.
 */

#define g4fact 3.2

double
d_r_d4_gauss (double x,
	      double a)
{
  register double x_a = x/(a*g4fact);
  register double x_a2 = x_a*x_a;
  return (3-6*x_a2+x_a2*x_a2)*exp(-x_a2/2);
}

double
f_r_d4_gauss (double k,
	      double a)
{
  register double ka = k*a;
  register double ka2 = ka*ka;
  return SQRT_2PI*a*ka2*ka2*exp(-ka2/2);
}

Wavelet _d4_gaussian_ = {
  WAVE_REAL_REAL,
  &d_r_d4_gauss,
  NULL,
  &f_r_d4_gauss,
  NULL,
  -6.4*g4fact,
  6.4*g4fact,
  -6.71875,
  6.71875,
  1.0,
  1.0
};

Wavelet *wt1d_d4_gaussian_ptr = &_d4_gaussian_;

/*
 * Definition of the Morlet wavelet.
 */

#define OMEGA 5.336446

double
d_r_morlet (double x,
	    double a)
{
  register double x_a = x/a;
  return cos(OMEGA*x_a)*exp(-x_a*x_a/2);
}

double
d_i_morlet (double x,
	    double a)
{
  register double x_a = x/a;
  return sin(OMEGA*x_a)*exp(-x_a*x_a/2);
}

double
f_r_morlet (double k,
	    double a)
{
  register double ka = (k*a-OMEGA);
  return SQRT_2PI*a*exp(-ka*ka/2);
}

Wavelet _morlet_ = {
  WAVE_CPLX_REAL,
  &d_r_morlet,
  &d_i_morlet,
  &f_r_morlet,
  NULL,
  -5.26,
  5.26,
  -0.01,
  10.57,
  1.0,
  1.0
};

Wavelet *wt1d_morlet_ptr = &_morlet_;

