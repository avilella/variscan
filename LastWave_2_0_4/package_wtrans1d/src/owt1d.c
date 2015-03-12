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
/*  owt1d.c       Fast Orthog. decomposition and reconstruction           */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"


/* Some external functions (in OCONVOLUTION.c) */
extern void SubsampledConvolution(SIGNAL input,SIGNAL output,FILTER F,int border);
extern void ExpandedConvolution(SIGNAL input, SIGNAL output, FILTER F, int border);


/**************************************************************/
/* This procedure decomposes the signal in level 0 in 'wtrans'*/
/* in its orthog. wavelet decomposition. This decomposition    */
/* is made on 'num_oct' octaves and is stored in 'wtrans'     */
/**************************************************************/

void OWtd(WTRANS wtrans,int num_oct)
{
  int n;
  float sqrt_half = 1. / sqrt(2.);

  /* Checks that a [bi]orthogonal filter is loaded */
  if (wtrans->fg == NULL || wtrans->fg->type == F_DYAD) SetBiorFG(NULL,wtrans);

  wtrans->size = wtrans->A[0][0]->size;
  wtrans->dx = wtrans->A[0][0]->dx;
  wtrans->x0 = wtrans->A[0][0]->x0;

  if (wtrans->wName != NULL) {
    Free(wtrans->wName);
    wtrans->wName = NULL;
  }

  /* Modif by Ben 17/08/98 */
  wtrans->aMin = 1.0;
  if(wtrans->fg->filename != NULL)
    {
/*       int size,i; */
      unsigned long size,i;
      
      size = strlen(wtrans->fg->filename);
      for(i=size-1;(i>=0) && (wtrans->fg->filename[i] != '/');i--)
	;
      /* NOW i IS THE INDEX OF THE LAST '/' or -1 */  
      
      /* wtrans->wName IS EVERYTHING AFTER THE LAST '/' */
      wtrans->wName = CopyStr(wtrans->fg->filename + i + 1);
    }
  /* End of Modif Ben */


  /* Antisymetric or periodic border condition */
  if (wtrans->fg->type == F_ORTH)  wtrans->border = B_PERIODIC;
  else if (wtrans->fg->type == F_BIOR_NO_SYM)  wtrans->border = B_PERIODIC;
  else wtrans->border = B_ANTISYMETRIC;
  
  /* Let's go */
  for(n = 1; n <= num_oct; n++) {
    SubsampledConvolution(wtrans->A[n-1][0], wtrans->A[n][0],
			   wtrans->fg->H1,wtrans->border);
    SubsampledConvolution(wtrans->A[n-1][0], wtrans->D[n][0],
			   wtrans->fg->G1,wtrans->border);
    wtrans->A[n][0]->dx = (1 << n) * wtrans->A[0][0]->dx;
    wtrans->D[n][0]->dx = (1 << n) * wtrans->A[0][0]->dx;
    wtrans->D[n][0]->x0 = wtrans->A[n][0]->x0 = wtrans->A[0][0]->x0;
    /*    wtrans->D[n][0]->x0 = - wtrans->A[0][0]->dx * (1 << n) / 2. + 
	  wtrans->A[0][0]->x0; */
  }
  
  /* Some inits */
  wtrans->nOct = num_oct;
  wtrans->nVoice = 1;
  wtrans->type = W_ORTH;
}


/* Associated command */
void C_OWtd(char **argv)
{
  int noct;
  WTRANS wtrans;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tINT,&noct,0);
  
  if (wtrans == NULL) wtrans = GetWtransCur();

  if (wtrans->A[0][0]->size == 0) Errorf("No signal to analyze");
  
  OWtd(wtrans,noct);
}

/**************************************************************/
/* This procedure reconstructs the original signal given its  */
/* orthog wavelet decomposition which is in 'wtrans'.         */
/* The reconstruction is put in 'signal'                      */
/**************************************************************/

void OWtr(WTRANS wtrans, SIGNAL signal)
{
  int n, final_size, initial_size,i;
  SIGNAL sig1,sig2;
  float sqrt_two =  sqrt(2.);

  /* Checks that a [bi]orthogonal transform has been computed */
  if (wtrans->type != W_ORTH) Errorf("Cannot run 'orecons' on a not [bi]orthogonal wtrans\n");
  
  /* Checks that a consistent filter is loaded */
  if (wtrans->fg == NULL) SetBiorFG(NULL,wtrans); 
  if (wtrans->fg == NULL || wtrans->fg->type == F_DYAD) 
    Errorf("OWtr() : The loaded filter is not a [bi]orthogonal filter");

  sig1 = NewSignal();
  sig2 = NewSignal();

  initial_size= wtrans->A[wtrans->nOct][0]->size;
  final_size = initial_size * (1 << (wtrans->nOct)) ;
  SizeSignal(signal,final_size,YSIG);
  SizeSignal(sig1,final_size,YSIG);
  SizeSignal(sig2,final_size,YSIG);

  CopySig(wtrans->A[wtrans->nOct][0],signal);

  for(n = wtrans->nOct; n >= 1; n--) {
    ExpandedConvolution(signal,sig1,wtrans->fg->H2,
			 wtrans->border);			   
    ExpandedConvolution(wtrans->D[n][0],sig2,wtrans->fg->G2,
			 wtrans->border);			   

    SizeSignal(signal,sig1->size,YSIG);
    signal->dx = sig1->dx;
    signal->x0 = sig1->x0;
    for (i=0;i<sig1->size;i++) signal->Y[i] = sig1->Y[i]+sig2->Y[i];
    

    /* Modif by Ben 18/06/98 
       signal->firstp = sig2->firstp;
       signal->lastp = sig2->lastp;
       */
    
    signal->firstp = MAX(sig1->firstp,sig2->firstp);
    signal->lastp = MIN(sig1->lastp,sig2->lastp);
  }
  DeleteSignal(sig1);
  DeleteSignal(sig2);
  
  signal->x0 =  wtrans->x0;
  signal->dx = wtrans->dx;
}

/* Corresponding Command */

void C_OWtr(char **argv)
{
  SIGNAL output;
  WTRANS wtrans;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tSIGNAL,&output,0);
  
  if (wtrans == NULL) wtrans = GetWtransCur();
  
  OWtr(wtrans,output);
}





