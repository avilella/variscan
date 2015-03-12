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



/********************************************************************/
/*                                                                  */
/*    dconvolution.c :                                              */
/*        fast convolution algorithms for dyadic transform          */
/*                                                                  */
/********************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"




/*
Four types of signal are considered.

example of n == 4:

position:  -7 -6 -5 -4 -3 -2 -1  0  1  2  3  4  5  6  7  8  9 10 11


TYPE 0:     3  5  7  7  5  3  2  2  3  5  7  7  5  3  2  2  3  5  7

TYPE 1:     3  5  7 -7 -5 -3 -2  2  3  5  7 -7 -5 -3 -2  2  3  5  7

TYPE 2:     3  5  7  9  7  5  3  2  3  5  7  9  7  5  3  2  3  5  7

TYPE 3:     3  5  7  0 -7 -5 -3  0  3  5  7  0 -7 -5 -3  0  3  5  7
*/



/* The 4 functions corresponding to the 4 types */

#define SYMREF0(k,size,input) (((k) < (size))?(input[k]):(input[2*size-k-1]))
#define SYMREF1(k,size,input) (((k) < (size))?(input[k]):(input[2*size-k-2]))
#define ASYMREF0(k,size,input) (((k) < (size))?(input[k]):(-input[2*size-k-1]))
#define ASYMREF1(k,size,input) ((k == size)?(0):((k) < (size))?(input[k]):(-input[2*size-k]))



/* The periodizing function */

#define per(n,k) (((k % n) + n ) % n)


/**************************************************************/
/* Convolution for the symetrical case	(decomposition)       */
/**************************************************************/

void conv_syml(SIGNAL signal_input,SIGNAL signal_result,FILTER filt, 
               int factor, int scale, int flag_remember, int parity)
{
  float *input = signal_input->Y;
  int sigsize = signal_input->size;
  float *result;
  int filtsize = filt->size;
  int filtshift = filt->shift;
  float *filter = filt->Y;
  float sum;
  int halfsizefilt, nbinterfilt, filtshift1;
  SIGNAL temp_signal;
  int j, k, left, right, l1, r1,start_index;
  int type,symsize;
  
  int sigsize2;
     
    
  if (parity == 0) sigsize2 = 2*(sigsize-sigsize%2);
  else sigsize2 = 2*(sigsize-(1-sigsize%2));
  
  /**************************************************************/
  /** Compute :                                                **/
  /**           halfsizefilt : half size of the scaled filter  **/
  /**           nbinterfilt  : number of interval of length 1  **/
  /**                          of the support of the scaled    **/
  /**                          filter                          **/
  /**           filtshift1   : shift of the scaled filter      **/
  /**                          (according to integer values    **/
  /**************************************************************/
  
  if (scale == 1)
    {
      filtshift1 = filtshift;
      halfsizefilt = filtsize-1;
      nbinterfilt=2*halfsizefilt-filtshift;
    }
  else {
    filtshift1 = 0;
    halfsizefilt = scale*(filtsize-1) - filtshift*scale/2;
    nbinterfilt = 2*halfsizefilt;
  }
  
  
  /**************************************************************/
  /** Compute :                                                **/
  /**            l1 : minimum abscissa of the scaled filter    **/
  /**            r1 : maximum abscissa of the scaled filter    **/
  /**************************************************************/

  if (flag_remember == YES)  start_index = signal_result->lastp+1;
  else start_index = 0;
  
  if(filtshift == 0) {
    l1 = r1 = scale;
    type = 0;
    symsize = sigsize;
  } 
  else {
    if(scale == 1) {
      l1 = 1;
      r1 = 0;
      type = 0;
      symsize = sigsize;
      sigsize++;
    } 
    else {
      l1 = r1 = scale / 2;
	  type = 1;
	  symsize = sigsize;
    }
  }

  
  /**************************************************************/
  /** Put the fields of the output signal unless there is      **/
  /** something to remember                                    **/
  /**************************************************************/

  if (flag_remember == YES) {
    if (signal_result->sizeMallocY < sigsize) {
      temp_signal = NewSignal();
      CopySig(signal_result,temp_signal);
      SizeSignal(signal_result,sigsize+200,YSIG);
      CopySig(temp_signal,signal_result);
      DeleteSignal(temp_signal);
    }
    signal_result->size = sigsize;
  }
  else {
    SizeSignal(signal_result,sigsize,YSIG);
    signal_result->dx = signal_input->dx;
    signal_result->x0 = signal_input->x0;
    if (scale==1 && filt->shift==1)
      signal_result->x0 -= 0.5*signal_input->dx;
    signal_result->firstp = signal_input->firstp + halfsizefilt - filtshift1;
    signal_result->param = nbinterfilt + signal_input->param;
  }

  signal_result->lastp = signal_input->lastp - halfsizefilt;
  result = signal_result->Y;



  /**************************************************************/
  /**           The convolution                                **/
  /**************************************************************/


  switch(type) {
    
    case 0 :
      for(j = start_index; j < sigsize; j += factor) {
          sum = filter[0] * SYMREF0(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	         sum += filter[k] *
	             (SYMREF0(per(sigsize2,left),symsize,input) + 
	              SYMREF0(per(sigsize2,right),symsize,input));
	     result[j] = sum;
	  }
      break;
      
    default :    
      for(j = start_index; j < sigsize; j += factor) {
         sum = filter[0] * SYMREF1(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	        sum += filter[k] *
	             (SYMREF1(per(sigsize2,left),symsize,input) + 
	             SYMREF1(per(sigsize2,right),symsize,input));
         result[j] = sum;
      }
  }
}


/**************************************************************/
/* Convolution for the asymetrical case	(decomposition)       */
/* fnorm is the factor renormalization signal result should be*/
/* divided by                                                 */
/**************************************************************/

void conv_asyl(SIGNAL signal_input, SIGNAL signal_result, FILTER filt, 
               int factor, int scale, float fnorm, int flag_remember, int parity)
{
  float *input = signal_input->Y;
  int sigsize = signal_input->size;
  float *result;
  float *filter = filt->Y;
  int filtsize = filt->size;
  int filtshift = filt->shift;
  int halfsizefilt, nbinterfilt, filtshift1;
  int j, k, left, right, l1, r1,start_index;
  float sum;
  SIGNAL temp_signal;
  int type,symsize;
  int sigsize2;
     
    
  if (parity == 0) sigsize2 = 2*(sigsize-sigsize%2);
  else sigsize2 = 2*(sigsize-(1-sigsize%2));

  /**************************************************************/
  /** Compute :                                                **/
  /**           halfsizefilt : half size of the scaled filter  **/
  /**           nbinterfilt  : number of interval of length 1  **/
  /**                          of the support of the scaled    **/
  /**                          filter                          **/
  /**           filtshift1   : shift of the scaled filter      **/
  /**                          (according to integer values    **/
  /**************************************************************/
  
  if (scale == 1)
    {
      filtshift1 = filtshift;
      halfsizefilt = filtsize-1;
      nbinterfilt=2*halfsizefilt-filtshift;
    }
  else {
    filtshift1 = 0;
    halfsizefilt = scale*(filtsize-1) - filtshift*scale/2;
    nbinterfilt = 2*halfsizefilt;
  }
  
  
  /**************************************************************/
  /** Compute :                                                **/
  /**            l1 : minimum abscissa of the scaled filter    **/
  /**            r1 : maximum abscissa of the scaled filter    **/
  /**************************************************************/

  if (flag_remember == YES)  start_index = signal_result->lastp+1;
  else start_index = 0;
 
  if(scale == 1) {
    l1 = 1;
    r1 = 0;
    type = 0;
    symsize = sigsize;
  } 
  else {
    l1 = r1 = scale / 2;
    if(filtshift == 0) {
      symsize = sigsize;
      type = 0;
    } 
    else {
      type = 1;
      symsize = sigsize;
      if (parity == 0) sigsize -= sigsize % 2;
      else sigsize -= (1-sigsize%2);
    }
  }

 
  /**************************************************************/
  /** Put the fields of the output signal unless there is      **/
  /** something to remember                                    **/
  /**************************************************************/

  if (flag_remember == YES) {
    if (signal_result->sizeMallocY < sigsize) {
      temp_signal = NewSignal();
      CopySig(signal_result,temp_signal);
      SizeSignal(signal_result,sigsize+200,YSIG);
      CopySig(temp_signal,signal_result);
      DeleteSignal(temp_signal);
    }
    signal_result->size = sigsize;
  }
  else {
    SizeSignal(signal_result,sigsize,YSIG);
    signal_result->dx = signal_input->dx;
    signal_result->x0 = signal_input->x0;
    if (scale==1 && filt->shift==1)
      signal_result->x0 -= 0.5*signal_input->dx;
    signal_result->firstp = signal_input->firstp + halfsizefilt - filtshift1;
    signal_result->param = nbinterfilt + signal_input->param;
  };

  signal_result->lastp = signal_input->lastp - halfsizefilt;
  result = signal_result->Y;



  /**************************************************************/
  /**           The convolution                                **/
  /**************************************************************/
  
  switch(type) {
    
    case 0 :
      for(j = start_index; j < sigsize; j += factor) {
         sum = filter[0] * SYMREF0(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	         sum += filter[k] *
	             (-SYMREF0(per(sigsize2,left),symsize,input) + 
	              SYMREF0(per(sigsize2,right),symsize,input));
	     result[j] = sum/fnorm;
	  }
      break;
      
    default :    
      for(j = start_index; j < sigsize; j += factor) {
         sum = filter[0] * SYMREF1(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	        sum += filter[k] *
	             (-SYMREF1(per(sigsize2,left),symsize,input) + 
	             SYMREF1(per(sigsize2,right),symsize,input));
         result[j] = sum/fnorm;
      }
  }
}


/**************************************************************/
/* Convolution for the symetrical case	(reconstruction)      */
/**************************************************************/

void conv_symr(SIGNAL signal_input, SIGNAL signal_result, FILTER filt, 
               int factor, int scale, int parity)
{
  float *input = signal_input->Y;
  int sigsize = signal_input->size;
  float *result;
  int filtsize = filt->size;
  float *filter = filt->Y;
  int filtshift = filt->shift;
  int last = sigsize-1;
  int j, k, left, right, l1, r1;
  float sum;
  int halfsizefilt, nbinterfilt, filtshift1;
  int symsize,type;
  int sigsize2;
     
    
  if (parity == 0) sigsize2 = 2*(sigsize-sigsize%2);
  else sigsize2 = 2*(sigsize-(1-sigsize%2));   
    
  
  /**************************************************************/
  /** Compute :                                                **/
  /**           halfsizefilt : half size of the scaled filter  **/
  /**           nbinterfilt  : number of interval of length 1  **/
  /**                          of the support of the scaled    **/
  /**                          filter                          **/
  /**           filtshift1   : shift of the scaled filter      **/
  /**                          (according to integer values    **/
  /**************************************************************/
  
  if (scale == 1)
    {
      filtshift1 = filtshift;
      halfsizefilt = filtsize-1;
      nbinterfilt=2*halfsizefilt-filtshift;
    }
  else {
    filtshift1 = 0;
    halfsizefilt = scale*(filtsize-1) - filtshift*scale/2;
    nbinterfilt = 2*halfsizefilt;
  }
  

  if(filtshift == 0) {
    l1 = r1 = scale;
    type = 0;
    symsize = sigsize;
  } 
  else {
    if(scale == 1) {
      l1 = 0;
      r1 = 1;
      type = 1;
      symsize = sigsize;
      sigsize--;
    } 
    else {
      l1 = r1 = scale / 2;
      type = 1;
      symsize = sigsize;
    }
  }

  SizeSignal(signal_result,sigsize,YSIG);
  result = signal_result->Y;
  signal_result->dx = signal_input->dx;
  signal_result->x0 = signal_input->x0;

  switch(type) {
    
    case 0 :
      for(j = 0; j < sigsize; j += factor) {
         sum = filter[0] * SYMREF0(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	        sum += filter[k] *
	               (SYMREF0(per(sigsize2,left),symsize,input) + 
	                SYMREF0(per(sigsize2,right),symsize,input));
	     result[j] = sum;
	  }
      break;
      
    default :    
      for(j = 0; j < sigsize; j += factor) {
         sum = filter[0] * SYMREF1(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	        sum += filter[k] *
	               (SYMREF1(per(sigsize2,left),symsize,input) + 
	                SYMREF1(per(sigsize2,right),symsize,input));
         result[j] = sum;
      }
  }
}


/**************************************************************/
/* Convolution for the asymetrical case	(reconstruction)      */
/* fnorm is the factor renormalization signal_input should be */
/* multiplied by                                              */
/**************************************************************/

void conv_asyr(SIGNAL signal_input, SIGNAL signal_result, FILTER filt, 
               int factor, int scale, float fnorm, int parity)
{
  float *input = signal_input->Y;
  int sigsize = signal_input->size;
  float *result;
  int filtsize = filt->size;
  float *filter = filt->Y;
  int filtshift = filt->shift;
  int last = sigsize-1;
  int j, k, left, right, l1, r1;
  float sum;
  int halfsizefilt, nbinterfilt, filtshift1;
  int symsize,type;
  
  int sigsize2;
     
    
  if (parity == 0) sigsize2 = 2*(sigsize-sigsize%2);
  else sigsize2 = 2*(sigsize-(1-sigsize%2));
   
    
  
  /**************************************************************/
  /** Compute :                                                **/
  /**           halfsizefilt : half size of the scaled filter  **/
  /**           nbinterfilt  : number of interval of length 1  **/
  /**                          of the support of the scaled    **/
  /**                          filter                          **/
  /**           filtshift1   : shift of the scaled filter      **/
  /**                          (according to integer values    **/
  /**************************************************************/
  
  if (scale == 1)
    {
      filtshift1 = filtshift;
      halfsizefilt = filtsize-1;
      nbinterfilt=2*halfsizefilt-filtshift;
    }
  else {
    filtshift1 = 0;
    halfsizefilt = scale*(filtsize-1) - filtshift*scale/2;
    nbinterfilt = 2*halfsizefilt;
  }
  
  
  if(scale == 1) {
    l1 = 0;
    r1 = 1;
    type = 1;
    symsize = sigsize;
  } 
  else {
    l1 = r1 = scale / 2;
    if(filtshift == 0) {
    type = 0;
    symsize = sigsize;
    } 
    else {
      type = 1;
      symsize = sigsize;
      sigsize++;
    }
  }

  SizeSignal(signal_result,sigsize,YSIG);
  result = signal_result->Y;
  signal_result->dx = signal_input->dx;
  signal_result->x0 = signal_input->x0;

  switch(type) {
    
    case 0 :
      for(j = 0; j < sigsize; j += factor) {
         sum = filter[0] * ASYMREF0(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	        sum += filter[k] *
	               (ASYMREF0(per(sigsize2,left),symsize,input) - 
	                ASYMREF0(per(sigsize2,right),symsize,input));
	     result[j] = sum*fnorm;
	  }
      break;
      
    default :    
      for(j = 0; j < sigsize; j += factor) {
         sum = filter[0] * ASYMREF1(j,symsize,input);
         for(k = 1, left = j-l1, right = j+r1;k < filtsize ;
	         k++, left -= scale, right += scale)
	        sum += filter[k] *
	               (ASYMREF1(per(sigsize2,left),symsize,input) - 
	               ASYMREF1(per(sigsize2,right),symsize,input));
         result[j] = sum*fnorm;
      }
  }
}

