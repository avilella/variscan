/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry, Stephane Mallat             */
/*      email : lastwave@cmap.polytechnique.fr                              */
/*                                                                          */
/*..........................................................................*/
/*                                                                          */
/*      This program is a free software, you can redistribute it and/or     */
/*      modify it under the terms of the GNU General Public License as      */
/*      published by the Free Software Foundation; either version 2 of the  */
/*      License, or (at your option) any later version                      */
/*                                                                          */
/*      This program is distributed in the hope that it will be useful,     */
/*      but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       */
/*      GNU General Public License for more details.                        */
/*                                                                          */
/*      You should have received a copy of the GNU General Public License   */
/*      along with this program (in a file named COPYRIGHT);                */
/*      if not, write to the Free Software Foundation, Inc.,                */
/*      59 Temple Place, Suite 330, Boston, MA  02111-1307  USA             */
/*                                                                          */
/*..........................................................................*/





/****************************************************************************/
/*                                                                          */
/*  oconvolution.c convolution functions for [bi]orthogonal wavelets        */
/*  It uses periodique or folded boundary conditions                        */
/*                                                                          */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"

/* The periodique boundary conditions periodizes the signal over n points */
#define PERIOD(j,n) ( j >= 0 ? ((j)%(n)) : (((n)+((j)%(n)))%(n)) ) 

/* The folding is using two types of symmetries with respect to border
   points. The symmetry 0 is a symmetry with respect to a given point. For
   example the sequence 9 8 7 8 9 is a symmetry 0 about the point of
   value 7. The symmetry 1 is a symmetry with respect to an intermediate 
   point. For example the sequence 9 8 7 7 8 9 is a symmtry 1 about the 
   center between the two values 7.
   The following folding procedure creates different foldings at
   the left and right borders. */

/* The function sym00 uses a symmetry 0 on the left border with respect
   to the point of abscissa 0 and a symmetry 0 on the right border
   with respect to the point of abscissa N-1.
   If the signal has N points, the extended symmetrical signal
   is of period 2N-2 */

static int sym00(int i,int N)
{
  if (i<0)
    i=-i;  
  if (i>=(2*N-2))
    i = i % (2*N-2);
  if(i < N)
    return(i);
  else
    return(2*N-2-i);
}

/* The function sym01 uses a symmetry 0 on the left border with respect
   to the point of abscissa 0 and a symmetry 1 on the right border
   with respect to the point of abscissa N-1.
   If the signal has N points, the extended symmetrical signal
   is of period 2N-1 */

static int sym01(int i,int N)
{
  if (i<0)
    i=-i;  
  if (i>=(2*N-1))
    i = i % (2*N-1);
  if(i < N)
    return(i);
  else
    return(2*N-1-i);
}

/* The function sym10 uses a symmetry 1 on the left border with respect
   to the point of abscissa 0 and a symmetry 0 on the right border
   with respect to the point of abscissa N-1.
   If the signal has N points, the extended symmetrical signal
   is of period 2N-1 */

static int sym10(int i,int N)
{
  if (i<0)
    i=-i-1;  
  if (i>=(2*N-1))
    i = i % (2*N-1);
  if(i < N)
    return(i);
  else
    return(2*N-2-i);
}


/**************************************************************/
/* Convolution for a signal reduction in a                    */
/* biorthogonal wavelet transform                             */
/* This function convoles the input with the filter F and     */
/* subsamples the output by a factor 2                        */
/* The treatement of borders depend upon the border value.    */
/* In the folding case,                                       */
/* the symmetry is of type 0 along the left and right borders */
/* of the input signal.                                       */
/**************************************************************/

void SubsampledConvolution(SIGNAL input,SIGNAL output,FILTER F,int border)
{
  float result;
  int k, k2 , l, n,n0;
  int Fleft,Fright;
  
  output->size = input->size / 2 ;
  SizeSignal(output,output->size,YSIG);

  Fleft = LEFT_FILT(F);
  Fright = RIGHT_FILT(F);
  
  n0 = ZERO(F);

  for(k = 0 ; k < output->size ; k++) {
    result = 0;
    k2 = 2 * k;
    for( n = Fleft ; n <= Fright ; n++) {
      if(border == B_PERIODIC)
	l = PERIOD(k2-n,input->size);
      else
	l = sym00(k2-n,input->size);
      result+= input->Y[l]* F->Y[n+n0];
    }
    output->Y[k] = result;
  }
  /* Modif by Ben 18/06/98 
     output->lastp = output->size -(input->size - input->lastp+Fright)/2.;
     output->firstp = (input->firstp - Fleft) / 2. ;
     */

  output->lastp = (input->lastp + Fleft)/2;
  output->firstp = (input->firstp + Fright)/2 + (input->firstp + Fright)%2;

}


/**************************************************************/
/* Convolution for a signal expansion in a                    */
/* biorthogonal/orthogonal wavelet transform                  */
/* This function puts a zero between any sample of the input  */
/* and convolve the result with the filter F                  */
/* The treatement of borders depend upon the border value     */
/* The symmetry depends upon the type of filter.              */
/**************************************************************/

void ExpandedConvolution(SIGNAL input, SIGNAL output, FILTER F, int border)
{
  float result;
  int k, k2, n0, n, l;
  int Fleft , Fright, left;

  output->size = input->size * 2;
  SizeSignal(output,output->size,YSIG);

  Fleft=LEFT_FILT(F);
  Fright=RIGHT_FILT(F);
  n0 = ZERO(F);

  /*loop trought the odd members of the output*/
  left = Fleft + 1 - (Fleft & 1);
  for(k = 1  ; k < output->size ;k +=2) {
    result = 0;
    k2 = k / 2 ;
    for( n = left ; n <= Fright ;n += 2) {
      if(border == B_PERIODIC)
	l = PERIOD(k2-(n-1)/2,input->size);
      else if(F->shift == 0)
      /*This is  the low-pass filter */
	l = sym01(k2-(n-1)/2,input->size);
      else
      /*This is  the high-pass filter */
	l = sym10(k2-(n-1)/2,input->size);

      result += input->Y[l]* F->Y[n+n0];
    }
    output->Y[k]=result;
  }
  /*loop trought the even members of the output*/
  left =  Fleft + (Fleft & 1) ;
  for(k = 0 ;k < output->size ;k +=2 ){
    result = 0;
    k2 = k / 2;
    for(n = left ;n <= Fright ; n+=2) {
      if(border == B_PERIODIC)
	l = PERIOD(k2-n/2,input->size);
      else if(F->shift == 0)
	/*This is  the low-pass filter */
	l = sym01(k2-n/2,input->size);
      else
	/*This is  the high-pass filter */
	l = sym10(k2-n/2,input->size);
      result+= input->Y[l]* F->Y[n+n0];
    }
    output->Y[k] = result;
  }
  /* Modif by Ben 18/06/98 
     output->lastp = output->size - 2 * (input->size - input->lastp) - Fright;
     output->firstp = 2 * input->firstp - Fleft;
     */

  output->lastp = 2 * input->lastp + Fleft;
  output->firstp = 2 * input->firstp + Fright;
}


