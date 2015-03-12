/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry                        */
/*      email  : remi.gribonval@inria.fr                                    */
/*               lastwave@cmapx.polytechnique.fr                            */
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



/****************************************************/
/*
 * 	The COMPLEX Short Time Fourier Transform :
 *
 *		COMPUTATION and UPDATE
 */
/****************************************************/
#include "lastwave.h"

#include "stft.h"


/*
 * FFT : To get rid of (use a library!)
 */
 
 
#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

#define FD float

static void FourierTransform(FD *data,int nn,int fftSign)
{
	int n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	float tempr,tempi;

	n=nn << 1;
	j=1;
	for (i=1;i<n;i+=2) {
		if (j > i) {
			SWAP(data[j],data[i]);
			SWAP(data[j+1],data[i+1]);
		}
		m=n >> 1;
		while (m >= 2 && j > m) {
			j -= m;
			m >>= 1;
		}
		j += m;
	}
	mmax=2;
	while (n > mmax) {
		istep=2*mmax;
		theta=6.28318530717959/(fftSign*mmax);
		wtemp=sin(0.5*theta);
		wpr = -2.0*wtemp*wtemp;
		wpi=sin(theta);
		wr=1.0;
		wi=0.0;
		for (m=1;m<mmax;m+=2) {
			for (i=m;i<=n;i+=istep) {
				j=i+mmax;
				tempr=wr*data[j]-wi*data[j+1];
				tempi=wr*data[j+1]+wi*data[j];
				data[j]=data[i]-tempr;
				data[j+1]=data[i+1]-tempi;
				data[i] += tempr;
				data[i+1] += tempi;
			}
			wr=(wtemp=wr)*wpr-wi*wpi+wr;
			wi=wi*wpr+wtemp*wpi+wi;
		}
		mmax=istep;
	}
}

#undef SWAP

/* Get the real part of the fftData and put it in the signal 'out' */

static void GetRealPart(FD *fftData,int size,SIGNAL out)
{
   int i;

   SizeSignal(out,size,YSIG);
   for (i=0;i<size;i++)
	   out->Y[i] = fftData[2*i+1];
}


/* Get the imaginary part of the fftData and put it in the signal 'out' */

static void GetImagPart(FD *fftData,int size,SIGNAL out)
{
   int i;
   
   SizeSignal(out,size,YSIG);
   for (i=0;i<size;i++)
	  out->Y[i] = fftData[2*i+2];
}

/***********************************************************************/
/* Make FFT-form signal from realPart and imagPart                     */
/* Add zeros if the size != newSize (newSize must be a power of 2      */
/* If imagPart == NULL then it is considered to be made of zero        */
/***********************************************************************/

static void MakeFFTData(FD *fftData,float *realPart,float *imagPart,int size,int newSize)
{
   int   i;

   if (imagPart != NULL) {     
   	for (i=0;i<newSize;i++)
       	 if (i>=size)
          	fftData[2*i+1] = fftData[2*i+2] = 0.0;
         else {
           	fftData[2*i+1] = realPart[i];
           	fftData[2*i+2] = imagPart[i];
         }
     }
    else {
      for (i=0;i<newSize;i++)
       	 if (i>=size)
          	fftData[2*i+1] = fftData[2*i+2] = 0.0;
         else {
           	fftData[2*i+1] = realPart[i];
           	fftData[2*i+2] = 0.0;
         }
     }  
}

/***********************************************************/
/*                                                         */
/*         Compute the fft transform of a SIGNAL           */
/*             (inImag can be NULL)                        */
/*                                                         */
/***********************************************************/


/* fft :  SIGNAL --> float *          */
/* fftOut must be of size 2*newSize+2 */
static void Fft1_(SIGNAL inReal,SIGNAL inImag,FD *fftOut, int newSize, int fftSign)
{

  if (inImag == NULL) 
   	  MakeFFTData(fftOut,inReal->Y,NULL,inReal->size,newSize);
  else
      MakeFFTData(fftOut,inReal->Y,inImag->Y,inReal->size,newSize);
    	  
  FourierTransform(fftOut,newSize,fftSign);
}
   

/* fft :  SIGNAL --> SIGNAL          */
static void Fft1(SIGNAL inReal,SIGNAL inImag,SIGNAL outReal,SIGNAL outImag,int fftSign,char flagShift)
{
   int size2, newSize,size;
   FD *fftData,val;
   int i;
   
   size2 = (int) (0.5+(log((double) inReal->size)/log(2.)));
   newSize = 1 << size2;
   
   /* allocation of the result */
   fftData = (FD *) Malloc(sizeof(FD)*(2*newSize+2));
   
   Fft1_(inReal,inImag,fftData,newSize,fftSign);  
   
   GetRealPart(fftData,newSize,outReal);
   GetImagPart(fftData,newSize,outImag);
   
   if (fftSign == -1) 
   	for(i=0;i<outReal->size;i++) {
   		outReal->Y[i] /= newSize;
   		outImag->Y[i] /= newSize;
   	}
   
   if (!flagShift) {
     outImag->dx = outReal->dx = 2*M_PI/outImag->size; 
     outImag->x0 = outReal->x0 = 0;
   } 
   else {
     size = outImag->size;
     for (i=0;i<size/2;i++ ) {
       val = outImag->Y[i];
       outImag->Y[i] = outImag->Y[size/2+i];
       outImag->Y[size/2+i] = val;
       val = outReal->Y[i];
       outReal->Y[i] = outReal->Y[size/2+i];
       outReal->Y[size/2+i] = val;
     }
     outImag->dx = outReal->dx = 1/(outImag->size*inReal->dx); 
     outImag->x0 = outReal->x0 = -1/(2*inReal->dx);
   }
   
   
   Free(fftData);
}


/************************************************************************
 *
 *    Multiply signal with the window and FFT
 *   (Sum_n  signal(n+shift) * window(n) * exp(2*i*PI*k*fRate*n/GABOR_MAX_FREQID)
 *    
 *
 ************************************************************************/
void FilterMultiplyFft(SIGNAL signal,char borderType,SIGNAL window,char windowShape,unsigned long timeId, unsigned long fRate,
		       SIGNAL resultR,SIGNAL resultI)
{
  unsigned long maxWindowShape;
  unsigned long paddFftSize;
  unsigned long i;
  static SIGNAL timeSignal = NULL;
  
  /* Checking arguments */
  if(signal == NULL) Errorf("FilterMultiplyFft : NULL signal");
  if(window == NULL) Errorf("FilterMultiplyFft : NULL window");
  if(window->size%2 != 0) Errorf("FilterMultiplyFft   : window size %d should be even",window->size);
  if(GABOR_MAX_FREQID%fRate != 0) Errorf("FilterMultiplyFft   : bad fRate %d does not divide GABOR_MAX_FREQID",fRate,GABOR_MAX_FREQID);
  
  /* Allocation if necessary */
  if (timeSignal == NULL) timeSignal = NewSignal();
  
  /* 
   * Compute the size of the (possibly zero padded) 
   * signal I want to perform the FFT on :
   * I need at least 'GABOR_MAX_FREQID/fRate' bins.
   */
  paddFftSize = MAX(window->size,GABOR_MAX_FREQID/fRate);
  
  /* 
   * Extract just the piece of the signal that we need,
   * with treatment of the border effects.
   * 
   * Checked correct on the 21/02/2001 by R.Gribonval
   *
   * timeId-maxWindowShape is the index of the first 
   * point of the window (which value is set to zero)
   * This is also the first point of the signal to extract.
   */
  maxWindowShape = GetMaxWindowShape(windowShape,window->size); 
  ExtractSig(signal,timeSignal,borderType,timeId-maxWindowShape,paddFftSize);
  
  /* Multiplication by window */
  for(i=0; i < window->size; i++) timeSignal->Y[i] *= window->Y[i];
    
  /* Zero padding if necessary */
  for(; i < paddFftSize; i++) timeSignal->Y[i]  = 0.0;
  
  /* Rotation to take into account the maxWindowShape : 
   * the window must be 'centered' at maxWindowShape */
  SizeSignal(resultR,paddFftSize,YSIG);
  ZeroSig(resultR);
  for(i=0; i<resultR->size; i++) 
    resultR->Y[i] = timeSignal->Y[(i+maxWindowShape)%timeSignal->size];		
  
  /* Fft */
  SizeSignal(resultI,paddFftSize,YSIG);
  ZeroSig(resultI);
  Fft1(resultR,NULL,resultR,resultI,1,NO);
  
  /* Taking the conjugate value : Why ???? */
  for(i=0; i<resultR->size; i++) resultI->Y[i] *= -1;
  
  /* Properly setting the 'dx' field */
  resultR->dx = GABOR_MAX_FREQID/paddFftSize;
  resultI->dx = GABOR_MAX_FREQID/paddFftSize;
}

void C_FilterMultiplyFft(char **argv)
{
  SIGNAL signal;
  char borderType = STFT_DEFAULT_BORDERTYPE;
  unsigned long fRate;
  char windowShape = STFT_DEFAULT_WINDOWSHAPE;

  SIGNAL window ;
  unsigned long timeId;
  SIGNAL resultR,resultI;
  char opt;
  char *name;
  argv = ParseArgv(argv,tSIGNALI,&signal,tSIGNALI,&window,tINT,&timeId,tINT,&fRate,tSIGNAL,&resultR,tSIGNAL,&resultI,-1);
  
  while((opt = ParseOption(&argv))) {
    switch(opt) {
    case 'b' :
      argv = ParseArgv(argv,tSTR,&name,-1);
      borderType = Name2BorderType(name);
      break;
    default :
      ErrorOption(opt);
    }
  }
  NoMoreArgs(argv);
  FilterMultiplyFft(signal,borderType,window,windowShape,timeId,fRate,resultR,resultI);
}
#ifdef TODO
void C_OverlapAdd(char **argv)
{
  STFT stft;
  SIGNAL signalR,signalI;
  int timeId;
  float *real,*imag;
  argv = ParseArgv(argv,tSTFT_,NULL,&stft,tSIGNAL,&signalR,tSIGNAL,&signalI,0);
  if(stft == NULL)
    stft = GetStftCur();
  CheckStftComplex(stft);
  SizeSignal(signalR,stft->signalSize,YSIG);
  SizeSignal(signalI,stft->signalSize,YSIG);
  
  for(timeId = 0; 
      timeId < stft->nFrames*stft->tRate;
      timeId += stft->tRate) {
    GetStftData(stft,timeId,&real,&imag);

    MakeFFTData(fftOut,real,imag,stft->nSubBands,newSize);
    FourierTransform(fftOut,newSize,0);
  }
}
#endif // TODO

/*******************************************************************
*     Convolution of a signal with a complex filter
************************ WARNING************************************
* We want to do the convolution with the window reversed
* so that we obtain scalar product (modulo the conjugate)
* Beware of the asymmetric windows !
************************ WARNING*************************************/
void FilterConvol(SIGNAL signal,int borderType,SIGNAL window,int windowShape,int freqId,int tRate,
		  SIGNAL resultR,SIGNAL resultI)
{
  static SIGNAL filterR = NULL;
  static SIGNAL filterI = NULL;
  SIGNAL expikR = NULL;
  SIGNAL expikI = NULL;
  int maxWindowShape;
  int index,ishift;
  int i;
  
  /* Checking arguments */
  if(signal == NULL) Errorf("FilterConvol : NULL signal");
  if(window == NULL) Errorf("FilterConvol : NULL window");
  if(window->size%2 != 0) Errorf("FilterConvol   : window size %d should be even",window->size);

  /* 
   * Build the complex filter
   */
  /* Allocating (only once) */
  if(filterR == NULL) {
    filterR = NewSignal();
    filterI = NewSignal();
  }
  /* Their window =  reversed time window */
  SizeSignal(filterR,window->size,YSIG);
  SizeSignal(filterI,window->size,YSIG);
  for(i=0;i<window->size;i++) {
    index = window->size-1-i;
    filterR->Y[i] = window->Y[index];
    filterI->Y[i] = window->Y[index];
  }
  /*
   * Their modulation = exp(2*i*pi*freqId*(i-ishift)/GABOR_MAX_FREQID)
   * with i-ishift=0 when i is at the maximum of the window
   * of the filter
   * i.e. when i = window->size-1-maxWindowShape
   */
  maxWindowShape = GetMaxWindowShape(windowShape,window->size);
  ishift        = window->size-1-maxWindowShape;

  GetTabExponentials(&expikR,&expikI);
  for(i=0;i<window->size;i++) {
    index = (freqId*(i-ishift+GABOR_MAX_FREQID))%GABOR_MAX_FREQID;
    if(!INRANGE(0,i,expikR->size-1))
      Errorf("FilterConvol : (Weird) internal expikR");
    filterR->Y[i] *= expikR->Y[index];
    filterI->Y[i] *= expikI->Y[index];
  }

  /* The maximum of the filter is at ishift. 
   * We shift it by -ishift to put the maximum at zero */
  filterR->x0 = -ishift;
  filterI->x0 = -ishift;
  
  ConvSig(signal,filterR,resultR,borderType,CV_UNDEFINED);
  ConvSig(signal,filterI,resultI,borderType,CV_UNDEFINED);
}

void C_FilterConvol(char **argv)
{
  SIGNAL signal;
  char borderType = STFT_DEFAULT_BORDERTYPE;
  int octave;
  static SIGNAL window = NULL;
  int windowShape  = STFT_DEFAULT_WINDOWSHAPE;
  int freqId,tRate;

  SIGNAL resultR,resultI;
  
  char opt;
  char *name;
  
  argv = ParseArgv(argv,tSIGNALI,&signal,tINT,&octave,tINT,&freqId,tINT,&tRate,
		   tSIGNAL,&resultR,tSIGNAL,&resultI,-1);
  
  while((opt = ParseOption(&argv))) {
    switch(opt) {
    case 'b' :
      argv = ParseArgv(argv,tSTR,&name,-1);
      borderType = Name2BorderType(name);
      break;
    case 'w' :
      argv = ParseArgv(argv,tSTR,&name,-1);
      windowShape = Name2WindowShape(name);
      break;
    default :
      ErrorOption(opt);
    }
  }
  NoMoreArgs(argv);

  /* Allocate once, if necessary */
  if(window == NULL) {
    window  = NewSignal();
  }

  GetTabWindow(windowShape,1<<octave,&window);
  FilterConvol(signal,borderType,window,windowShape,freqId,tRate,resultR,resultI);
}

 
/**************************************************************/
/*
 * Update of the COMPLEX stft data, two methods :
 *
 *	'Time' : we loop on time, within a SPECIFIED TIME RANGE
 *		     and update all frequencies with a 
 *			'filter-multiply + FFT' algorithm
 *	'Freq' : we loop on ALL frequencies and update with a 
 *                    CONVOLUTION (Direct or FFT = optimized?)
 */
/**************************************************************/


/*
 * Very helpful function to set the values of a complex stft 
 * at a given time 'timeId', for all frequencies.
 * 'freqSignal' is a signal of 'total size' (size*dx) 'GABOR_MAX_FREQID'
 * representing the values of the complex spectrogram
 * at different frequencies (from 0 to Nyquist=GABOR_MAX_FREQID/2 with a step freqSignal->dx)
 */
static void StftComplexSetFreq(STFT stftComplex,SIGNAL freqSignalR,SIGNAL freqSignalI,int timeId)
{
    int freqId,fC,sigScale;
    float *innerProdR,*innerProdI;

    /* Checking arguments */
    CheckStftComplex(stftComplex);
    if(!INRANGE(0,timeId,stftComplex->signalSize-1))
      Errorf("StftComplexSetFreq() : bad timeId %d not in [0 %d]",timeId,stftComplex->signalSize-1);
    if(freqSignalR == NULL || freqSignalI == NULL)
      Errorf("StftComplexSetFreq() : NULL freqSignalR or freqSignalI");
    if(freqSignalR->size != freqSignalI->size)
      Errorf("StftComplexSetFreq() : freqSignalR and freqSignalI have different size");
    if(freqSignalR->dx != freqSignalI->dx)
      Errorf("StftComplexSetFreq() : freqSignalR and freqSignalI have different dx");
    sigScale = (int) freqSignalR->dx;
    if (sigScale != stftComplex->fRate)
	  Errorf("StftComplexSetFreq() : fRate %d != signal dx %d\n", stftComplex->fRate,sigScale);
    if (freqSignalR->size*freqSignalR->dx != GABOR_MAX_FREQID)
      Errorf("StftComplexSetFreq() : freqSignal total size %g must be GABOR_MAX_FREQID %d\n",
	     freqSignalR->size*freqSignalR->dx,GABOR_MAX_FREQID);

    /* Getting the complex data location */
    GetStftData(stftComplex,timeId,&innerProdR,&innerProdI);
  
    /* We first deal with the frequencies 0 */
    innerProdR[0] 	= freqSignalR->Y[0];
    innerProdI[0] 	= 0.0;	/* The inner-product is real */

    /* Then we deal with frequency Nyquist (if exists) */
    if (stftComplex->nSubBands >= 2) {
      fC = GABOR_NYQUIST_FREQID/stftComplex->fRate;
      // TODO : check it is correct !!!
      innerProdR[fC] 	= freqSignalR->Y[GABOR_NYQUIST_FREQID/sigScale];
      innerProdI[fC] 	= 0.0;  /* The inner-product is real */
    }

    /* Then we deal with the other frequencies */
    for(freqId = stftComplex->fRate, fC = 1; fC < stftComplex->nSubBands-1; freqId += stftComplex->fRate,fC++) {
      innerProdR[fC] 	= freqSignalR->Y[freqId/sigScale];
      innerProdI[fC] 	= freqSignalI->Y[freqId/sigScale];
    }
} 

/**************************************************************
 *
 * Computes the values of the COMPLEX stft of a given signal,
 * given that this signal has changed between the time-indexes
 * [timeIdMin, timeIdMax]. 
 * If the borderType is BorderPad or BorderPad0, 
 *   [timeIdMin,timeIdMax] must be a subinterval of [0,signalSize-1]
 * If the borderType is BorderPeriodic
 *   [timeIdMin,timeIdMax] must be a subinterval of [0,2*signalSize-1]
 **************************************************************/

void UpdateStftComplex(STFT stftComplex,SIGNAL signal,int timeIdMin,int timeIdMax)
{    
    static SIGNAL freqSignalR = NULL;
    static SIGNAL freqSignalI = NULL;

    char flagTwoLoops = NO;
    int timeIdMinStft,timeIdMaxStft;
    int timeIdMinStft2,timeIdMaxStft2;

    int timeId;

    SIGNAL window = NULL;
    
    int octave;

    /* Checking arguments */
    CheckStftComplex(stftComplex);
    if(signal == NULL) Errorf("UpdateStftComplex : NULL signal");
    
    /* Allocation if necessary */
    if(freqSignalR == NULL) {
	  freqSignalR = NewSignal();	
	  freqSignalI = NewSignal();
    }

    ComputeUpdateLimits(stftComplex,timeIdMin,timeIdMax,
			&timeIdMinStft,&timeIdMaxStft,
			&flagTwoLoops,&timeIdMinStft2,&timeIdMaxStft2);
    /**************************
     *
     * Actual Computation
     *
     **************************/
    
    octave = (int) (log(stftComplex->windowSize)/log(2.0)+0.5);
    if(1<<octave != stftComplex->windowSize)
      Errorf("UpdateStftComplex : windowSize is not a power of two!");
    GetTabWindow(stftComplex->windowShape,stftComplex->windowSize,&window);
    // First time loop
    for (timeId = timeIdMinStft;
	 timeId <= timeIdMaxStft; 
	 timeId += stftComplex->tRate) {
      // Gets the FFT of signal(timeId+n)*window(n) in a complex signal.
      FilterMultiplyFft(signal,stftComplex->borderType,window,stftComplex->windowShape,timeId,stftComplex->fRate,freqSignalR,freqSignalI);
      // Set the complex spectrogram data
      StftComplexSetFreq(stftComplex,freqSignalR,freqSignalI,timeId);
    }
    // Second loop if needed
    if(flagTwoLoops) {
      for (timeId = timeIdMinStft2; 
	   timeId <= timeIdMaxStft2; 
	   timeId += stftComplex->tRate) {
	FilterMultiplyFft(signal,stftComplex->borderType,window,stftComplex->windowShape,timeId,stftComplex->fRate,freqSignalR,freqSignalI);
	StftComplexSetFreq(stftComplex,freqSignalR,freqSignalI,timeId);
      }
    }
    
    /* Now the COMPLEX spectrogram is up to date */
    stftComplex->flagUpToDate = YES;
}

/************************************************************************/
/*
 *			 FREQUENCY LOOP 
 *
 *
 * Computes the values of the complex stft of the given signal,
 * for all time and freq.
 *
 */
/*************************************************************************/
/* Frequency Loop NOT YET RE-IMPLEMENTED */

void UpdateStftComplexFreq(STFT stftComplex,SIGNAL signal)
{
    int freqId;
    static SIGNAL timeSignalR = NULL;
    static SIGNAL timeSignalI = NULL;
    SIGNAL window    = NULL;

    int octave;

    /* Checking arguments */
    CheckStftComplex(stftComplex);
    Errorf("UpdateStftComplexFreq : not re-implemented yet");

    /* Initializing (just once) */
    if(timeSignalR == NULL) {
      timeSignalR = NewSignal();
      timeSignalI = NewSignal();
    }

    octave = (int) (log(stftComplex->windowSize)/log(2.0)+0.5);
    if(1<<octave != stftComplex->windowSize)
      Errorf("UpdateStftComplexFreq : windowSize is not a power of two!");
    GetTabWindow(stftComplex->windowShape,stftComplex->windowSize,&window);

    /* Frequency Loop */
    for (freqId = 0; freqId < stftComplex->fRate*stftComplex->nSubBands; freqId += stftComplex->fRate) {
      FilterConvol(signal,stftComplex->borderType,window,stftComplex->windowShape,
		   freqId,stftComplex->tRate,timeSignalR,timeSignalI);
      Errorf("UpdateStftComplexFreq : subsampling not re-implemented yet");
/*	StftComplexSetTimeData(stftComplex,freqId,timeSignalR,timeSignalI);*/
    }
    /* Now the complex part is up-to-date */
    stftComplex->flagUpToDate = YES;
}

/* EOF */



