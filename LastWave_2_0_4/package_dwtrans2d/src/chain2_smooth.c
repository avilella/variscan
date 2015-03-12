/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'dwtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  E.Bacry, J.Fraleu, J.Kalifa, E. Le Pennec, */
/*                         W.L. Hwang , S.Mallat, S.Zhong                   */
/*      emails : lastwave@cmap.polytechnique.fr                             */
/*               fraleu@cmap.polytechnique.fr                               */
/*               kalifa@cmap.polytechnique.fr                               */
/*               lepennec@cmap.polytechnique.fr                             */
/*               mallat@cmap.polytechnique.fr                               */
/*               whwang@iis.sinica.edu.tw                                   */
/*               szhong@chelsea.princeton.edu                               */
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




#include "lastwave.h"
#include "extrema2d.h"
#include "signals.h"
  
#define CHAIN_LENGTH 1024

/* #define W2_pt_sym(k, sigsize) \
  ((k) < 0 ? -(k) : ((k) < (sigsize) ? (k) : 2*(sigsize) - (k) -2)) */

int W2_pt_sym(int k, int sigsize)
{
  if (k<0) k = -k;
  k = k%sigsize;
  return(k);
}

/***************************************************/
/* Smooth with a Gaussian filter of variance sigma */
/***************************************************/

static void W2_sig_smooth(SIGNAL input,SIGNAL output,float sigma,int  flag)
     
{
  int j, n, sigsize;
  double sum, sum0;
  float *filter;
  /* float sigma; */
  int begin, end;
  double param;

  if(input == output)
    Printf("The input and output levels must be different \n");
  else {
    begin = 0;
    end = input->size;

    SizeSignal(output,input->size,YSIG);
  
     
    filter = (float *) Malloc(sizeof(float) * ((int) (3 * sigma+1)));
    sigsize = input->size;

    sum0 = 0.;
    if(flag == 0) { /* filter with a center ext */
      for( n = 0; n < (int)(3 * sigma) ; n++) {
        param = (double)(-1.0 * (n * n)/(2.0 * sigma));
        filter[n] = (float)(exp(param));
        sum0 = sum0 + (double)filter[n];
      }
      sum0 += sum0 - 1.;
    }
    else { /* filter without center ext */
      for( n = 1; n < (int)(3 * sigma) ; n++) {
        filter[n] = (float)(exp( - (float)((n + .5) * (n + .5) / (2. * sigma))));
        sum0 += (double)filter[n];
      }
      sum0 += sum0;
    }
  
    for(j = 0; j < begin; j++) output->Y[j] = input->Y[j] ;
    for(j = end ; j < sigsize; j++) output->Y[j] = input->Y[j] ;
    for(j = begin; j < end; j++) {
      if(flag == 0) sum = filter[0] * input->Y[j];
      else sum = 0;
      for(n = 1; n < (int)(3 * sigma) ; n++)
        sum += filter[n] * (input->Y[W2_pt_sym(j+n,input->size)] +
                            input->Y[W2_pt_sym(j-n,input->size)]);
      output->Y[j] = (float)(sum/sum0);
    }

    CopyFieldsSig(input,output);
  }
}

  


static void W2_smooth_chain(float c_array[CHAIN_LENGTH],int size,int variation,int option_mode,int middle)
{
  int i, k, half, absi;
  float s_array[CHAIN_LENGTH], sum, sum0;

  half = (variation -1)/2;

  for (k = middle ; k < size- middle; k++) {
    sum0 = 0.0;
    sum = 0.0;
    for (i = -1 * half; i <= half; i++) {
      if (i < 0) absi = -1 *i;
      else absi = i;
      if ((option_mode == 0) && INRANGE(0, k + i, size-1)) {
	sum += (1.0 - (float)absi/half) * c_array[k+i];
	sum0 += 1.0 - (float)absi/half;
      }
      if ((option_mode == 1) && INRANGE(0, k + i, size-1)) {
	sum += c_array[k+i];
	sum0 += 1.0;
      }
    }
    s_array[k] = sum/sum0;
    /* if (fabs((double)sum0) < 0.0000001) s_array[k] = c_array[k]; */

  }

  for (k = middle; k < size-middle; k++)
    c_array[k] = s_array[k];
}


/* smooth abscissa */

static void W2_smooth_chain_pic_abscissa(CHAINLIS2 chainlis,int  variation,int option_mode,int middle)
{
  CHAIN2 chain;
  float x_array[CHAIN_LENGTH], y_array[CHAIN_LENGTH];
  EXT2 ext;
  int i, size;

  for (chain = chainlis->first; chain; chain = chain->next) {
    i = 0;
    for (ext = chain->first; ext; ext = ext->next) {
      x_array[i] = (float)ext->x;
      y_array[i] = (float)ext->y;
      i++;
    }
    size = i;
    W2_smooth_chain(x_array, size, variation, option_mode, middle);
    W2_smooth_chain(y_array, size, variation, option_mode, middle);
    i = 0;
    for (ext = chain->first; ext; ext = ext->next) {
      ext->x = (int)(x_array[i]+ 0.5);
      ext->y = (int)(y_array[i] + 0.5);
      i++;
    }
  }
}
		   
/* smooth amplitude */

static void W2_smooth_chain_pic_amplitude(CHAINLIS2 chainlis,int variation,int option_mode,int middle)
{
  CHAIN2 chain;
  float amp_array[CHAIN_LENGTH];
  EXT2 ext;
  int i, size;

  for (chain = chainlis->first; chain; chain = chain->next) {
    i = 0;
    for (ext = chain->first; ext; ext = ext->next) {
	amp_array[i] = ext->mag;
	i++;
      }
    size = i;
    W2_smooth_chain(amp_array, size, variation, option_mode, middle);

    i = 0;
    for (ext = chain->first; ext; ext = ext->next) {
      ext->mag = amp_array[i];
      i++;
    }
  }
}


/*********************************************/
/* smooth chain feature by a gaussain kernel */
/*********************************************/

void W2_chain_pic_smooth_gauss(CHAINLIS2 chainlis,float sigma,int ampflag,int absflag,int argflag)
     
{
  CHAIN2 chain;
  SIGNAL input, output, input1, output1;
  EXT2 ext;
  float *vinput, *voutput, *vinput1, *voutput1;
  int i, nb;

  input =   NewSignal();
  input1 =   NewSignal();
  output =   NewSignal();
  output1 =   NewSignal();

  for(chain = chainlis->first; chain; chain = chain->next) {
    nb = chain->size;
    ext = chain->first;

    if(nb > 0) {
       SizeSignal(input,nb,YSIG);
       SizeSignal(input1,nb,YSIG);
       SizeSignal(output,nb,YSIG);
       SizeSignal(output1,nb,YSIG);

      vinput = (float *)input->Y;
      vinput1 = (float *)input1->Y;
      voutput = (float *)output->Y;
      voutput1 = (float *)output1->Y;

      i = 0;
      ext = chain->first;
      while(ext) {
	if(ampflag) vinput[i++] = ext->mag;
	if(argflag) vinput[i++] = ext->arg;
	if(absflag) {
	  vinput[i] = (float)ext->x;
	  vinput1[i++] = (float)ext->y;
	}
	ext = ext->next;
      }
      if((int)(3.0 * sigma) <= 2 * input->size -2) W2_sig_smooth(input, output, sigma, 0);
      else CopySig(input, output);

      if(absflag) {
	if((int)(3.0 * sigma) <= 2 * input1->size -2)
	  W2_sig_smooth(input1,output1, sigma,0);
	else CopySig(input1, output1);

	vinput = (float *)input->Y;
	vinput1 = (float *)input1->Y;
	voutput = (float *)output->Y;
	voutput1 = (float *)output1->Y;
      
	voutput[0] = vinput[0];
	voutput[nb -1] = voutput[nb -1];
	voutput1[0] = vinput1[0];
	voutput1[nb -1] = vinput1[nb -1];
      }

      i = 0; 
      ext = chain->first;
      while(ext) {
	if(ampflag) ext->mag = voutput[i++];
	if(argflag) ext->arg = voutput[i++];
	if(absflag) {
	  ext->x = (int)(voutput[i] + 0.5);
	  ext->y = (int)(voutput1[i++] + 0.5);
	}
	ext = ext->next;
      }
    }
    }
}

/********************************************************************/
/* convolve the amplitude of points in a chain by a gaussian kernel */
/********************************************************************/


 
void W2_chainpicbluramp(WTRANS2 wtrans2,int level, float sigma)
{
  int ampflag = YES, absflag = NO, argflag = NO;
  EXTLIS2 extlis;
  CHAINLIS2 chainlis;

  if(sigma < 1.0) return;

  if(INRANGE(1, level, wtrans2->chainrep->noct)) {
    W2_chain_pic_smooth_gauss(wtrans2->chainrep->array[level], sigma,
			   ampflag, absflag, argflag);
  }
  else Errorf("Level is not in the range \n");

  extlis = wtrans2->extrep->array[level];
  chainlis = wtrans2->chainrep->array[level];

  W2_collect_point_in_point_pic(extlis, chainlis);
  W2_remove_point_in_chain_pic(extlis, chainlis);

}

void C_ChainPicBlurAmp(char **argv)
{
  float sigma;
 int level;
  WTRANS2 wtrans2;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,tINT,&level,tFLOAT,&sigma,0);

  if (wtrans2 ==NULL) wtrans2= GetWtrans2Cur();

  W2_chainpicbluramp(wtrans2,level, sigma);
}
/***************************************************************************/
/* convolve the x and y abscissa of points in a chain by a gaussian kernel */
/***************************************************************************/

void C_ChainPicBlurAbs(char **argv)
{
  CHAINLIS2 chainlis;
  EXTLIS2 extlis;
  WTRANS2 wtrans2;

  float sigma;
  int level, ampflag = NO, absflag = YES, argflag = NO;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,tINT,&level,tFLOAT,&sigma,0);

  if (wtrans2 ==NULL) wtrans2= GetWtrans2Cur();
 
  if(sigma < 1.0) return;

  if(INRANGE(1, level, wtrans2->chainrep->noct)) {
    extlis = wtrans2->extrep->array[level];
    chainlis = wtrans2->chainrep->array[level];

    W2_chain_pic_smooth_gauss(chainlis,sigma,ampflag,absflag,argflag);

    W2_point_pic_relocate(extlis, chainlis); 

    W2_chain_pic_interpolation(extlis,chainlis);

  }
  else Errorf("Level is out of range \n");
    
}



