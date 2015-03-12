/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
/*      email : lastwave@polytechnique.fr                                   */
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
/*  wtrans_functions.c: Miscellaneous useful functions   		            */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"




/*
 * Compute the Min and the max of a wtrans 
 */

void MinMaxWtrans(WTRANS wtrans,float x1, float x2,float *pvMin,float *pvMax,int flagCausal)
{
  int o,v;
  int i,i1,i2;
  int iMin,iMax;

  (*pvMin) = 9e20;
  (*pvMax) = -9e20;
  for (o = 1; o <= wtrans->nOct; o++)
    for (v=0;v<wtrans->nVoice;v++) {	
      i1 = ISig(wtrans->D[o][v],x1);
      i2 = ISig(wtrans->D[o][v],x2);
      iMin = 0;
      iMax = wtrans->D[o][v]->size-1;
      if (flagCausal) {
	iMin = MAX(iMin,wtrans->D[o][v]->firstp);
	iMax = MIN(iMax,wtrans->D[o][v]->lastp);
      }
      if (x1 < x2) {
	iMin = MAX(iMin,i1);
	iMax = MAX(iMax,i2);
      }

      for (i=iMin;i<=iMax;i++) {
	if (wtrans->D[o][v]->Y[i] > *pvMax) *pvMax = wtrans->D[o][v]->Y[i];
	if (wtrans->D[o][v]->Y[i] < *pvMin) *pvMin = wtrans->D[o][v]->Y[i];
      }
    }
}


/*
 * Threshold of the wavelet transform
 */

void ThreshWtrans(WTRANS wtrans_in,WTRANS wtrans_out,float threshold,
                float x_left,float x_right,float alpha,int oct_min,int oct_max)
{
  int oct, noct = wtrans_in->nOct, voice, nvoice = wtrans_in->nVoice;
  float coeff;
  int iMin,iMax,i;
  SIGNAL in,out;

  CheckWtrans(wtrans_in);

  coeff =  pow(2.,(double)(alpha/nvoice));
  threshold *= pow(2.,(double)(alpha)) / coeff ;

  for(oct = oct_min ; oct <= oct_max; ++oct) {
    for(voice = 0 ; voice < nvoice; ++voice) {
      threshold *= coeff ;
      in = wtrans_in->D[oct][voice];
      out = wtrans_out->D[oct][voice];
      iMin = (x_left-in->x0)/in->dx;
      iMin = MAX(0,iMin);
      iMax = (x_right-in->x0)/in->dx;
      iMax = MIN(in->size-1,iMax);
      if (in != out) CopySig(in,out);
      for (i=iMin;i<=iMax;i++) 
        if (fabs(out->Y[i]) <= threshold) out->Y[i] = 0;
    }
  }

  if (wtrans_in != wtrans_out) {
    for(oct = 1 ; oct < oct_min; ++oct) 
      for(voice = 0 ; voice < nvoice; ++voice) 
	    CopySig(wtrans_in->D[oct][voice],wtrans_out->D[oct][voice]);

    for(oct = oct_max+1 ; oct <= noct ; ++oct) 
      for(voice = 0 ; voice < nvoice; ++voice) 
	CopySig(wtrans_in->D[oct][voice],wtrans_out->D[oct][voice]);

    CopySig(wtrans_in->A[noct][0],wtrans_out->A[noct][0]);
    CopyFieldsWtrans(wtrans_in , wtrans_out);
  }
}


/*
 * The corresponding command 
 */
 
void C_WThresh (char **argv)
{
  WTRANS wtransIn, wtransOut;
  float threshold, alpha = 0.;
  int octMin , octMax ;
  float xLeft ,xRight,sigShift;
  char opt;
  
  if (argv == NULL) DOC("{{{[<wtrans>=objCur] <threshold> [-x <xMin> <xMax>] [-o <oMin> <oMax>] [-e <alpha>] [-w <wtransOut>]} {Thresholds the \
wavelet coefficients of a wavelet transform <wtrans>. All the coefficients smaller than <threshold> are set to 0. \
The options are \n\
  -x : Only coefficients corresponding to abscissa between <xMin> and <xMax> are thresholded \n\
  -o : Only coefficients corresponding to octave between <oMin> and <oMax> are thresholded \n\
  -w : Specifies the wavelet transform the result should be stored in (by default, the initial <wtrans> is used) \n\
  -e : The exponent <alpha> is used to change the threshold value along the scales. The threshold at octave \
'o' and voice 'v' is <threshold>*(2^(alpha*(o-1+v/nvoice))).}}}");

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtransIn,tFLOAT,&threshold,-1);
  if (wtransIn == NULL) wtransIn = GetWtransCur();
  wtransOut =  wtransIn;
  
  sigShift = wtransIn->D[1][0]->x0 + wtransIn->D[1][0]->dx;

  xLeft = sigShift ;
  xRight = sigShift+wtransIn->D[1][0]->size * wtransIn->D[1][0]->dx ;
  octMin = 1;
  octMax = wtransIn->nOct;

  while(opt = ParseOption(&argv)) { 

   switch(opt) {
   case 'x':
      argv = ParseArgv(argv,tFLOAT,&xLeft,tFLOAT,&xRight,-1);
      if (xRight < xLeft) Errorf("The first abscissa parameter must be smaller than the second one\n");
      break;
   case 'o':
      argv = ParseArgv(argv,tINT,&octMin,tINT,&octMax,-1);
      if (octMax < octMin) Errorf("The first octave parameter must be smaller than the second one\n");
      break;
    case 'e':
      argv = ParseArgv(argv,tFLOAT,&alpha,-1);
      break;
    case 'w':
      argv = ParseArgv(argv,tWTRANS,&wtransOut,-1);
      break;
    default:
     ErrorOption(opt);
   }
 }    
 NoMoreArgs(argv);

 ThreshWtrans(wtransIn,wtransOut,threshold,xLeft,xRight,alpha,octMin, octMax);
}


