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
/*  dwt1d.c :   dyadic decomposition and reconstruction           */ 
/*                                                                  */
/*                                                                  */
/********************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"


/* Some external functions (in DCONVOLUTION.c) */

extern void conv_syml(SIGNAL signal_input,SIGNAL signal_result,FILTER filt, 
                      int factor, int scale, int flag_remember, int parity);
extern void conv_asyl(SIGNAL signal_input, SIGNAL signal_result, FILTER filt, 
                      int factor, int scale, float fnorm, int flag_remember, int parity);
extern void conv_symr(SIGNAL signal_input, SIGNAL signal_result, FILTER filt, 
                      int factor, int scale, int parity);
extern void conv_asyr(SIGNAL signal_input, SIGNAL signal_result, FILTER filt, 
                      int factor, int scale, float fnorm, int parity);
                      
                      

/****************************************************************/
/*    This function compute the normalization factor            */
/*    that is used for the wavelet decomposition of the         */
/*    level 0 of 'wtrans' at scale 'oct', 'voice'.              */
/*    The result is stored in (*pfnorm).                        */
/****************************************************************/

static void get_fnorm(WTRANS wtrans,int oct,int voice,float *pfnorm)
{
  float *fnorms = wtrans->fg->factors;
  
  
  if (voice == 0) (*pfnorm) = fnorms[oct-1];
  else
    (*pfnorm) = 
      (float) 
	(exp(log((double) fnorms[oct-1]) +
	     ((log((double) fnorms[oct]) 
	       - log((double) fnorms[oct-1]))*((double) voice))
	     /((double) wtrans->nVoice)));

}

/**************************************************************/
/* This procedure decomposes the original signal in its       */
/* dyadic wavelet decomposition.                              */
/**************************************************************/

/* Decompose a 'signal' on dyadic scales corresponding to the voice 'voice' */

/* WARNING : It is VERY important to have set the wtrans->nVoice beforehand */
   
void DWtd_(WTRANS wtrans,SIGNAL signal,int num_oct,int voice,
              int flag_remember,int parity,int p)
{
  int l;
  int filt_dilatation = 1 << p;
  float fnorm = 1;
 
  for(l = 1; l <= num_oct; l++) {   
    
    /* We first get the normalization factor */ 
    get_fnorm(wtrans,l,voice,&fnorm); 

    /* Then we process one H and one G */
    if (l == 1) {
       conv_syml(signal, wtrans->A[l][voice],wtrans->fg->H1,1,filt_dilatation,flag_remember,parity);
	   conv_asyl(signal, wtrans->D[l][voice],wtrans->fg->G1,1,filt_dilatation,fnorm,flag_remember,parity);
	}
	else  {
	   conv_syml(wtrans->A[l-1][voice], wtrans->A[l][voice],wtrans->fg->H1,1,filt_dilatation,flag_remember,parity);    
       conv_asyl(wtrans->A[l-1][voice], wtrans->D[l][voice],wtrans->fg->G1,1,filt_dilatation,fnorm,flag_remember,parity);
	}
	          
    filt_dilatation <<= 1;
  }
  
}

/* This is the function to call for processing a regular dyadic decomposition
   The original signal is in A[0][0]
   and 'flag_remember' is the real-time flag */
   
void DWtd(WTRANS wtrans, int num_oct, int flag_remember)
{
  int parity = wtrans->A[0][0]->size%2;
  
  wtrans->nOct = num_oct;
  wtrans->nVoice = 1;
  wtrans->type = W_DYAD;
  wtrans->x0 = wtrans->A[0][0]->x0;
  wtrans->dx = wtrans->A[0][0]->dx;
  wtrans->size = wtrans->A[0][0]->size;
  
  if (wtrans->wName != NULL) {
    Free(wtrans->wName);
    wtrans->wName = NULL;
  }
  
  DWtd_(wtrans,wtrans->A[0][0],num_oct,0,flag_remember,parity,0);
}


/* Associated Command */

void C_DWtd(char **argv)
{
  int noct;
  int flagRemember;
  WTRANS wtrans;
  char opt;
  
  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tINT_,-1,&noct,-1);
  
  if (wtrans == NULL) wtrans = GetWtransCur();
  
  if (wtrans->A[0][0]->size == 0) Errorf("No signal to decompose");
  
  /* Some options */
  flagRemember = NO;
  while(opt = ParseOption(&argv)) { 
    switch(opt) {
      case 'r' : flagRemember = YES; break;
      default : ErrorOption(opt);
    }
  }  
  NoMoreArgs(argv);
  
  if ((flagRemember == NO && (noct < 1 || noct>NOCT)) ||
      (flagRemember == YES && noct != -1)) ErrorUsage1();
  if (flagRemember == YES && wtrans->nOct == 0) Errorf("There is nothing to remember!!");
  if (flagRemember == YES) noct = wtrans->nOct;

  SetDyadFG(NULL,wtrans); 
  DWtd(wtrans,noct,flagRemember);
}


/**************************************************************/
/* This procedure reconstructs the original signal given its  */
/* dyadic wavelet decomposition.                              */
/**************************************************************/

void DWtr(WTRANS wtrans,SIGNAL signal)
{
  int l,i;
  int filt_dilatation = 1 << (wtrans->nOct-1);
  SIGNAL temp_signal;
  float fnorm = 1;
  int parity;
    
 /* Checks that a dyadic transform has been computed */
  if (wtrans->type != W_DYAD) Errorf("Cannot run 'drecons' on a not dyadic wtrans\n");
  
  /* Checks that a consistent filter is loaded */
  if (wtrans->fg == NULL) SetDyadFG(NULL,wtrans); 
  if (wtrans->fg == NULL || wtrans->fg->type != F_DYAD) 
    Errorf("DWtr() : The loaded filter is not a dyadic filter");

  temp_signal = TNewSignal();
  
  parity = wtrans->D[1][0]->size%2;
  
  CopySig(wtrans->A[wtrans->nOct][0],signal);

  for(l = wtrans->nOct; l >= 1; l--) {
    conv_symr(signal, temp_signal,wtrans->fg->H2, 1, filt_dilatation,parity);
    get_fnorm(wtrans,l,0,&fnorm);
    conv_asyr(wtrans->D[l][0],signal, wtrans->fg->G2, 1, filt_dilatation,fnorm,parity);

    for (i=0;i<temp_signal->size;i++) signal->Y[i] += temp_signal->Y[i];
        
    filt_dilatation >>= 1;
  }  
  
  signal->x0 = wtrans->x0;
  signal->dx = wtrans->dx;
}


/* Associated Command */

void C_DWtr(char **argv)
{ 
  SIGNAL output;
  WTRANS wtrans;
  
  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tSIGNAL,&output,0);
  
  if (wtrans == NULL) wtrans = GetWtransCur();
  
  DWtr(wtrans,output);
}



