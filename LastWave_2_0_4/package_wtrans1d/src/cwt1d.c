/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
/*                                                                          */
/*      Copyright (C) 1999 B.Audit, E.Bacry, N.Decoster.                    */
/*      emails : audit@crpp.u-bordeaux.fr                                   */
/*               lastwave@cmap.polytechnique.fr                             */
/*               decoster@crpp.u-bordeaux.fr                                */
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
/*  cwt1d.c    compute the wavelet decomposition using fft              */
/*                                                                          */
/****************************************************************************/



#include <time.h>
#include "lastwave.h"
#include "wtrans1d.h"
#include "extrema1d.h"


/* Computes the continuous wavelet transform using wavelets defined in the physical space */
void CWtd(WTRANS wtrans,float aMin,int nOct,int nVoi,Wavelet *wavelet,
                  int borderType,int flagMemoryOptimized,int flagCausal,
                  float expo)
{
  double a,factor;
  float mult;
  float threshold;
  int size;
  int o,v,i;
  int nb,nb1;
  int firstp,lastp;
  float ro, phase;

  SIGNAL filter = TNewSignal();
  int type;
  int sizeG,sizeD,sizeTot;
  float d_x_min,d_x_max;
  double (*d_r_fct_ptr)(double,double);
  double (*d_i_fct_ptr)(double,double);

  wt1d_get_wavelet_attributes(wavelet,&type,&d_r_fct_ptr,&d_i_fct_ptr,NULL,NULL,
                              &d_x_min,&d_x_max,NULL,NULL,NULL,NULL);

  /* Some init */
  wtrans->nOct = nOct;
  wtrans->nVoice = nVoi;
  wtrans->aMin = aMin; 
  wtrans->exponent = expo;
  wtrans->x0 = wtrans->A[0][0]->x0;
  wtrans->dx = wtrans->A[0][0]->dx;
  wtrans->size = wtrans->A[0][0]->size;
  if (flagMemoryOptimized == YES) {
    InitExtrep(wtrans,wtrans->extrep);
    nb = 0;
  }
  
 
  size = wtrans->A[0][0]->size;
  SizeSignal(filter,size,YSIG);

  factor = pow(2.,1./nVoi);
  for (o = 1, a = (double) aMin ; o <= nOct ; o++)   
    for(v=0;v<nVoi;v++)
      {   
        if (v == 0 && o == 1) Printf("Start octave %d",o);
        else if (v == 0 && o != 1) Printf("\nStart octave %d",o);
        else Printf(".");
        Flush();
        
        SizeSignal(wtrans->D[o][v],size,YSIG);

        sizeD = (int) (a*d_x_max);
        sizeG = (int) (-a*d_x_min);
        sizeTot = sizeD + sizeG + 1;
        if(sizeTot>size) Errorf("CWtd(): sizeTot>size!!!!!!");
        
        for(i=0;i<sizeTot;i++) 
          filter->Y[sizeTot-i-1] = (float) (*d_r_fct_ptr)((double) (i-sizeG),a);
                  
        cv_sig_init(CV_RC_FORM,wtrans->A[0][0]->Y,NULL,size);
        cv_flt_init_n(CV_RC_FORM,sizeTot,sizeD,0,0,filter->Y,NULL);
        cv_set_method(CV_UNDEFINED);
        cv_compute(borderType,wtrans->D[o][v]->Y,&firstp,&lastp);

        wtrans->D[o][v]->dx  = wtrans->A[0][0]->dx;
        wtrans->D[o][v]->x0  = wtrans->A[0][0]->x0;
        wtrans->D[o][v]->firstp  = wtrans->A[0][0]->firstp+firstp;
        wtrans->D[o][v]->lastp  = wtrans->A[0][0]->lastp + lastp - size + 1;

        mult = pow(a,expo);
        for(i=0;i<size;i++) wtrans->D[o][v]->Y[i] *= mult;
        
        
        /*
         * Case of complex wavelets 
         */
        if (type == WAVE_CPLX_REAL || type == WAVE_CPLX_CPLX) {
          if (d_i_fct_ptr == NULL) Errorf("CWtd() : Weird error");

          SizeSignal(wtrans->A[o][v],size,YSIG);
         
          for(i=0;i<sizeTot;i++) 
            filter->Y[sizeTot-i-1] = (float) (*d_i_fct_ptr)((double) (i-sizeG),a);
                  
          cv_sig_init(CV_RC_FORM,wtrans->A[0][0]->Y,NULL,size);
          cv_flt_init_n(CV_RC_FORM,sizeTot,sizeD,0,0,filter->Y,NULL);
          cv_set_method(CV_UNDEFINED);
          cv_compute(borderType,wtrans->A[o][v]->Y,&firstp,&lastp);

          wtrans->A[o][v]->dx  = wtrans->A[0][0]->dx;
          wtrans->A[o][v]->x0  = wtrans->A[0][0]->x0;
          wtrans->A[o][v]->firstp  = wtrans->A[0][0]->firstp+firstp;
          wtrans->A[o][v]->lastp  = wtrans->A[0][0]->lastp + lastp - size + 1;
       
          mult = pow(a,expo);
          for(i=0;i<size;i++) wtrans->A[o][v]->Y[i] *= mult;
      
          /* Compute the phase in A and the module in D */
          for (i=0;i<size;i++) {
            ro = sqrt(wtrans->A[o][v]->Y[i]*wtrans->A[o][v]->Y[i] + wtrans->D[o][v]->Y[i]*wtrans->D[o][v]->Y[i]);
            if (ro == 0) phase = 0;
            else {
              phase = acos(wtrans->D[o][v]->Y[i]/ro);
              if (wtrans->A[o][v]->Y[i] < 0) phase += M_PI;
            }
            wtrans->D[o][v]->Y[i] = ro;
            wtrans->A[o][v]->Y[i] = phase;
          }
        }
        
        
        /*
         * Then we go on
         */
                      
        a *= factor;
        if (flagMemoryOptimized == YES) 
          {
            nb1 = ComputeExtOctVoice(wtrans,wtrans->extrep,flagCausal,NO,1e-5,o,v,&threshold);
            nb += nb1;
            ClearSignal(wtrans->D[o][v]);
            if (type == WAVE_CPLX_REAL || type == WAVE_CPLX_CPLX) ClearSignal(wtrans->A[o][v]);
          }
      }
  Printf("\n");
  
  if (flagMemoryOptimized == YES)
    {
      Printf("nb of extrema : %d\n",nb);
      wtrans->nOct = 0;
      wtrans->nVoice = 0;
    }
}




/* The corresponding command */
void C_CWtd(char **argv)
{
  float aMin,aMinTest,aMaxTest;
  int nOct,nVoi;
  int flagMemoryOptimized,flagCausal;
  int border;
  char *borderName;
  char opt;
  WTRANS wtrans;
  char  *waveName;
  Wavelet *wavelet;
  float expo;
  float time;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tFLOAT,&aMin,tINT,&nOct,
                   tINT,&nVoi,tSTR_,"g2",&waveName,-1);
  
  if (wtrans == NULL) wtrans = GetWtransCur();
  /*  if (waveName == NULL) wavelet = wt1d_d2_gaussian_ptr;
  else*/
  if (!strcmp(waveName,"g0")) wavelet = wt1d_gaussian_ptr;
  else if (!strcmp(waveName,"g1")) wavelet = wt1d_d1_gaussian_ptr;
  else if (!strcmp(waveName,"g2")) wavelet = wt1d_d2_gaussian_ptr;
  else if (!strcmp(waveName,"g3")) wavelet = wt1d_d3_gaussian_ptr;
  else if (!strcmp(waveName,"g4")) wavelet = wt1d_d4_gaussian_ptr;
  else if (!strcmp(waveName,"morlet")) wavelet = wt1d_morlet_ptr;
  else Errorf("%s is not an implemented wavelet",waveName);

  if (wtrans->A[0][0]->size == 0) Errorf("No signal to analyze");
  
  if (!INRANGE(1,nOct,NOCT-1)) Errorf("Bad range for <noct>");
  if (!INRANGE(1,nVoi,NVOICE)) Errorf("Bad range for <nvoice> \n");
  
  wt1d_wavelet_scale_limits(wavelet,wtrans->A[0][0]->size,&aMinTest,&aMaxTest);
  aMinTest = 1.0;

  if (aMin<aMinTest)
    Warningf("I think amin is too small, it should be bigger than %g.",
             aMinTest);
  if ((pow(2.0,nOct - 1.0/((double) nVoi))*aMin)>aMaxTest) 
    Errorf("Sorry, the maximum scale is too big, it should be smaller than %g.",aMaxTest);

  /***************/
  /* The Options */
  /***************/
  flagCausal = NO;
  flagMemoryOptimized = NO;
  expo = -1.;
  border = BorderMirror;
  while(opt = ParseOption(&argv))
    {
      switch(opt) 
        {
        case 'm': 
          flagMemoryOptimized = YES;
          break;
        case 'c': 
          flagCausal = YES;
          break;
        case 'e':
          argv = ParseArgv(argv,tFLOAT,&expo,-1);
          break;
        case 'b':
          argv = ParseArgv(argv,tSTR,&borderName,-1);
          if(!strcmp(borderName,"pad"))
            border =  BorderPad;
          else if(!strcmp(borderName,"mir"))
            border = BorderMirror;
          else if(!strcmp(borderName,"pad0"))
            border = BorderPad0;
          else if(!strcmp(borderName,"per"))
            border = BorderPeriodic;
          else 
            Errorf("Undefined border effect: %s",borderName);
          break;
        default: 
          ErrorOption(opt);
        }
    }    
  NoMoreArgs(argv);
  
  
  if (wtrans->wName != NULL) Free(wtrans->wName);
  wtrans->wName = CopyStr(waveName);
  
  time = MyTime();
  CWtd(wtrans,aMin,nOct,nVoi,wavelet,border,flagMemoryOptimized,
                 flagCausal,expo);

  SetResultFloat(MyTime()-time);                 
}

void C_Wt1dNoctMax(char **argv)
{
  float aMin,aMinWavelet,aMaxWavelet;
  int signalSize,nvoice;
  Wavelet *wavelet;
  char *waveName;

  argv = ParseArgv(argv,tINT,&signalSize,tFLOAT,&aMin,tINT,&nvoice,
                   tSTR,&waveName,0);
  
  if (!strcmp(waveName,"g0")) wavelet = wt1d_gaussian_ptr;
  else if (!strcmp(waveName,"g1")) wavelet = wt1d_d1_gaussian_ptr;
  else if (!strcmp(waveName,"g2")) wavelet = wt1d_d2_gaussian_ptr;
  else if (!strcmp(waveName,"g3")) wavelet = wt1d_d3_gaussian_ptr;
  else if (!strcmp(waveName,"g4")) wavelet = wt1d_d4_gaussian_ptr;
  else Errorf("%s is not an implemented wavelet",waveName);
  
  wt1d_wavelet_scale_limits(wavelet,signalSize,&aMinWavelet,&aMaxWavelet);
  aMinWavelet = 1.0;

  SetResultInt((int) floor(log(aMaxWavelet/aMin)/log(2.)+1./nvoice) );
  return;
}




