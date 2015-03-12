/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry and J.McKelvie         */
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



#include "lastwave.h"

#include "stft.h"


/*
 * To get rid of
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


//
// Tabulation of some data for the Stft package
//

//
// THE STATIC VARIABLES BELOW ARE INITIALIZED IN InitStftTabulation 
// AND FREED IN FreeStftTabulation
//

// The number of values of 'windowSize' for which some tabulation occurs
// and an array that maps i->windowSize for 0<= i < nTabWindowSizes
static unsigned short nTabWindowSizes = 0;
static unsigned long* stftTabWindowSizes = NULL;
// The function to convert from a value to an index 0<=i<nTabWindowSizes
// If 'windowSize' is not tabulated we return nTabWindowSizes;
static unsigned short GetIndexWindowSize(unsigned long windowSize) {
  unsigned short i;
  for(i=0;i<nTabWindowSizes;i++) {
    if(stftTabWindowSizes[i]==windowSize) return(i);
  }
  return(i);
}

// Memory is allocated for these signals in InitStftTabulations
static SIGNAL** stftTabGGI;
static SIGNAL** stftTabGGR;

/**********************************************/
/*   TODO : explain
 *   Tabulation of the GG
 *   (realGG[o-oMin]->Y[freqId],imagGG[o-oMin]->Y[freqId]) 
 *   is the inner product <g,\overline{g}> for the atom g
 *   at octave 'o' and frequency freqId.
 *
 *   -For symmetric 'windowShape', 'imagGG' is always zero so 
 *   *pImagGG is set to NULL
 *
 *   -(realGG,imagGG) at frequency freqId is the ?? as at freqId
 *    (GABOR_NYQUIST_FREQID-freqId) so we don't need to store the frequencies for
 *    freqId > FreqNyquist/2.
 *
 *   -the values |(realGG,imagGG)| <= CCATOM_PRECISION are not stored.
 *
 * -----------------------------------------------------------------
 *   -at the zero frequency
 *	realGG[o-oMin]->Y[0] == 1.0
 *	imagGG[o-oMin]->Y[0] == 0.0
 *
 ***********************************************************/

static void StftTabulateGG(char windowShape)
{ /* Declaration of variables used in 'for' loops below */
  unsigned short i;
  unsigned long windowSize;
  SIGNAL window,paddedWindow,shiftedWindow;
  char flagAsymWindow = NO;
  // TODO : explain
  unsigned long maxWindowShape;

  SIGNAL *tab1GGR  = NULL;
  SIGNAL *tab1GGI  = NULL;
  SIGNAL GGR      = NULL;
  SIGNAL GGI      = NULL;

  float realGG,imagGG;
  unsigned long freqId,n;

  // TODO : remove !
  float precision = CCATOM_PRECISION*CCATOM_PRECISION;

  /* Checking arguments */
  if(!WindowShapeIsOK(windowShape)) Errorf("StftTabulateGG : bad windowShape %d",windowShape);
  // Quit without dong anything if already tabulated
  if(stftTabGGR[windowShape]!=NULL) return;

  flagAsymWindow = GetFlagAsymWindowShape(windowShape);

  // TODO : try to load from file here

  /* Allocating */
  tab1GGR = stftTabGGR[windowShape] = (SIGNAL*) Calloc(nTabWindowSizes,sizeof(SIGNAL));
  tab1GGI = stftTabGGI[windowShape] = (SIGNAL*) Calloc(nTabWindowSizes,sizeof(SIGNAL));
  paddedWindow  = NewSignal();
  shiftedWindow = NewSignal();
  SizeSignal(paddedWindow,GABOR_MAX_FREQID,YSIG);
  SizeSignal(shiftedWindow,GABOR_MAX_FREQID,YSIG);
  
  //DEBUG
  Printf("Tabulating GG for '%s' windows\n",WindowShape2Name(windowShape));Flush();

  // Allocating the signals and filling them
  for(i=0; i<nTabWindowSizes; i++) {
    windowSize = stftTabWindowSizes[i];
    GetTabWindow(windowShape,windowSize,&window);
    // TODO : CLARIFY AND EXPLAIN THIS 
    maxWindowShape = GetMaxWindowShape(windowShape,windowSize);
    //DEBUG
    Printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
    Printf("***tabulating size %6d",windowSize);Flush();
    //DEBUG
    Printf("--maxWindowShape=%6d",maxWindowShape);Flush();
    
    // Initializing squared window
    for(n=0; n<window->size; n++)    paddedWindow->Y[n] = window->Y[n]*window->Y[n];
    // Zero padding
    for(; n<paddedWindow->size; n++) paddedWindow->Y[n] = 0.0;
    // Rotation to take into account the maxWindowShape : 
    // the window must be 'centered' at maxWindowShape 
    for(n = 0; n<shiftedWindow->size; n++) 
      shiftedWindow->Y[n] = paddedWindow->Y[(n+maxWindowShape)%paddedWindow->size];		

    //
    // Allocating and tabulating the resultin GG signals
    //
    GGR = tab1GGR[i] = NewSignal();
    GGI = tab1GGI[i] = NewSignal();
    SizeSignal(GGR,GABOR_MAX_FREQID,YSIG);
    SizeSignal(GGI,GABOR_MAX_FREQID,YSIG);
    ZeroSig(GGR);
    ZeroSig(GGI);

    /* Fft */
    Fft1(shiftedWindow,NULL,GGR,GGI,1,NO);

    // The zero frequency : this must be strictly ensured
    GGR->Y[0] = realGG = 1.0;
    GGI->Y[0] = imagGG = 0.0;
    // The other frequencies
    for(freqId = 1; freqId<=GABOR_MAX_FREQID/4 && realGG*realGG+imagGG*imagGG>precision; freqId++) {
      realGG = GGR->Y[2*freqId];
      imagGG = GGI->Y[2*freqId];
      // Deal with computation round-off errors : 
      // -if the window has symmetry the imagGG should be zero
      // -in any case the resulting (realGG,imagGG) should have modulus at most 1
      if (flagAsymWindow==NO) imagGG = 0.0;
      if(realGG>1.0) {
	realGG=1.0;
	imagGG=0.0;
      }
      if(realGG<-1.0) {
	realGG=-1.0;
	imagGG=0.0;
      }
      // This situation has only been observed so far with spline3,exponential and FoF windows
      // for windowSize=16,32,64 and freqId = 1,2,3 with excess of modulus of the order of 1e-07
      if(realGG*realGG+imagGG*imagGG>1.0) {
	if(realGG*realGG+imagGG*imagGG-1.0>1e-06)
	  Errorf("StftTabulateGG : (WEIRED) modulus of GG exceeds 1 by more than %g",1e-06);
	imagGG=0.0;
      }
      GGR->Y[freqId] = realGG;
      GGI->Y[freqId] = imagGG;
    }
    for(;freqId<GGR->size; freqId++) {
      GGR->Y[freqId] = 0.0;
      GGI->Y[freqId] = 0.0;
    }
  }
  // DEBUG
  Printf("\n");
  DeleteSignal(paddedWindow);
  DeleteSignal(shiftedWindow);
}

// Gets a GG from the the tabulation
// -If the tabulation was not done for the windowShape, then we do it for all windowSizes
// -If the windowSize is not tabulated (i.e. so far not a power of 2), an ERROR is generated
// -TODO should we COMPUTE the GGR ? But should we allocate it ?????
void GetTabGG(char windowShape, unsigned long windowSize,SIGNAL *GGRptr,SIGNAL *GGIptr)
{
  SIGNAL *tab1GGR  = NULL;
  SIGNAL *tab1GGI  = NULL;
  unsigned short i;
  /* Check args */
  if(!WindowShapeIsOK(windowShape)) Errorf("GetTabGG : bad windowShape %d",windowShape);
  if(GGRptr==NULL||GGIptr==NULL)    Errorf("GetTabGG : NULL output");

  *GGRptr = NULL;
  *GGIptr = NULL;
  // If the GGR+GGI were not tabulated we do it now
  if(stftTabGGR[windowShape]==NULL) StftTabulateGG(windowShape);
  tab1GGR = stftTabGGR[windowShape];
  tab1GGI = stftTabGGI[windowShape];
  if(tab1GGR==NULL) Errorf("GetTabGG: NULL tab1GGR");
  if(tab1GGI==NULL) Errorf("GetTabGG: NULL tab1GGI");


  // Find the content at the desired windowSize
  i = GetIndexWindowSize(windowSize);
  if(i<nTabWindowSizes) {
    *GGRptr = tab1GGR[i];
    *GGIptr = tab1GGI[i];
  } else   
    Errorf("GetTabGG : windowSize = %d is not tabulated",windowSize);
  // DEBUG
  if(*GGRptr==NULL) Errorf("GetTabGG: NULL realGG");
  if(*GGIptr==NULL) Errorf("GetTabGG: NULL imagGG");
}


// stftTabWindowShapes and stftTabWindowFactors are initialised in InitStftTabulations
static SIGNAL** stftTabWindowShapes  = NULL;
static float**  stftTabWindowFactors = NULL;
// If the windows with the given 'windowShape' are not tabulated,
// we load them from a file or tabulate them,
// else just do nothing.
static void StftTabulateWindowShape(char windowShape)
{
  float (*f)(SIGNAL,float);

  unsigned short i;
  SIGNAL* windowShapes = NULL;
  float*  windowFactors= NULL;

  /* Checking arguments */
  if(!WindowShapeIsOK(windowShape)) 
    Errorf("StftTabulateWindowShape : bad windowShape %d",windowShape);
  // Quit without doing anything if already tabulated
  if(stftTabWindowShapes[windowShape]!=NULL) return;

  // TODO ?? : try to load from file here

  // Getting the right window
  GetWindowShapeFunc(windowShape,&f);

  /* Allocating */
  stftTabWindowShapes[windowShape]  = (SIGNAL*) Calloc(nTabWindowSizes,sizeof(SIGNAL));
  stftTabWindowFactors[windowShape] = (float *) Calloc(nTabWindowSizes,sizeof(float));
  windowShapes  = stftTabWindowShapes[windowShape];
  windowFactors = stftTabWindowFactors[windowShape];

  // Allocating the signals and filling them
  for(i=0; i<nTabWindowSizes; i++) {
    windowShapes[i] = NewSignal();
    SizeSignal(windowShapes[i],stftTabWindowSizes[i],YSIG);
    windowFactors[i]=(*f)(windowShapes[i],0.0);
  }
}

// Gets a window from the the tabulated windows.
// -If the tabulation was not done for the windowShape, then we do it for all windowSizes
// -If the windowSize is not tabulated (i.e not a power of 2) an ERROR is generated
// - TODO should we COMPUTE the window ? But should we allocate it ?????
void GetTabWindow(char windowShape,unsigned long windowSize,SIGNAL *windowPtr)
{
  unsigned short i;

  /* Checking arguments */
  if(!WindowShapeIsOK(windowShape)) 
    Errorf("GetTabWindow : bad windowShape %d",windowShape);
  if(windowPtr == NULL)
    Errorf("GetTabWindow : NULL output");
    
  // If the windowShape was not tabulated we do it now
  if(stftTabWindowShapes[windowShape]==NULL) 
    StftTabulateWindowShape(windowShape);

  // Find the tabulated windowShape at the right 'windowSize'
  i=GetIndexWindowSize(windowSize);
  if(i<nTabWindowSizes) {
    *windowPtr = stftTabWindowShapes[windowShape][i];
    // DEBUG
    if((*windowPtr)->size!=windowSize) 
      Errorf("GetTabWindow : corrupted windows");
    return;
  } else
    Errorf("GetTabWindow : windowSize %d is not tabulated",windowSize);
}

// Gets a windowFactor from the the tabulated windows.
// -If the tabulation was not done for the windowShape, then we do it for all windowSizes
// -If the windowSize is not tabulated (i.e not a power of 2) an ERROR is generated
// - TODO should we COMPUTE the window ? But should we allocate it ?????
void GetTabWindowFactor(char windowShape,unsigned long windowSize,float *windowFactorPtr)
{
  unsigned short i;

  /* Checking arguments */
  if(!WindowShapeIsOK(windowShape)) 
    Errorf("GetTabWindowFactor : bad windowShape %d",windowShape);
  if(windowFactorPtr == NULL)
    Errorf("GetTabWindowFactor : NULL output");
    
  // If the windowShape was not tabulated we do it now
  if(stftTabWindowShapes[windowShape]==NULL) 
    StftTabulateWindowShape(windowShape);

  // Find the tabulated windowShape at the right 'windowSize'
  i=GetIndexWindowSize(windowSize);
  if(i<nTabWindowSizes) {
    *windowFactorPtr = stftTabWindowFactors[windowShape][i];
    return;
  } else
    Errorf("GetTabWindowFactor : windowSize %d is not tabulated",windowSize);
}

//
// The signals where complex exponentials are tabulated
//
static SIGNAL expikR=NULL;
static SIGNAL expikI=NULL;

//  Function to tabulate complex exponentials
//  TODO ?? - try to load them from a file
//  Else we allocate and tabulate them
static void StftTabulateComplexExponentials(void)
{
  unsigned long i;
  // TODO ?? : try to load from file here

  //
  // Allocating and tabulating the finest resolution complex exponentials
  //
  expikR = NewSignal();
  expikI = NewSignal();
  SizeSignal(expikR,GABOR_MAX_FREQID,YSIG);
  SizeSignal(expikI,GABOR_MAX_FREQID,YSIG);
  ZeroSig(expikR);
  ZeroSig(expikI);

  // Tabulation of the frequency 0
  expikR->Y[0] = 1.0;
  expikI->Y[0] = 0.0;
  // Tabulation of the Nyquist frequency
  expikR->Y[GABOR_NYQUIST_FREQID] = -1.0;
  expikI->Y[GABOR_NYQUIST_FREQID] = 0.0;
  // Tabulation of the other frequencies
  for(i=1; i < GABOR_NYQUIST_FREQID; i++) {
    expikR->Y[GABOR_MAX_FREQID-i] = expikR->Y[i] = cos(2.0*(M_PI*i)/GABOR_MAX_FREQID);
    expikI->Y[i]            = sin(2.0*(M_PI*i)/GABOR_MAX_FREQID);
    expikI->Y[GABOR_MAX_FREQID-i] = -expikI->Y[i];
  }
}

// Gets the complex exponentials from the tabulation.
void GetTabExponentials(SIGNAL *expikRptr,SIGNAL *expikIptr)
{
  /* Checking arguments */
  if(expikRptr==NULL||expikIptr==NULL) Errorf("GetTabExponentials : NULL output");
  *expikRptr = expikR;
  *expikIptr = expikI;
}

// This function is called when the Stft package is loaded
void InitStftTabulations(void)
{
  unsigned long windowSize;
  unsigned long i;

  // Compute the number of values of 'windowSize' for which tabulation occurs
  for(nTabWindowSizes=0,windowSize=4; windowSize <= STFT_MAX_WINDOWSIZE; nTabWindowSizes++, windowSize<<=1) {}
  // Allocating an computing the map i -> windowSize
  stftTabWindowSizes = (unsigned long*) Calloc(nTabWindowSizes,sizeof(unsigned long));
  for(i=0,windowSize=4; i<nTabWindowSizes; i++, windowSize<<=1) {
    stftTabWindowSizes[i]=windowSize;
  }

  // Allocate and tabulate all complex exponentials
  StftTabulateComplexExponentials();

  // Allocate memory for future tabulation of windows for every windowShape,
  // Allocation for a given windowShape will only occur if necessary.
  stftTabWindowShapes  = (SIGNAL **)  Calloc(LastWindowShape,sizeof(SIGNAL*));
  stftTabWindowFactors = (float **)   Calloc(LastWindowShape,sizeof(float*));
  stftTabGGR           = (SIGNAL **) Calloc(LastWindowShape,sizeof(SIGNAL*));
  stftTabGGI           = (SIGNAL **) Calloc(LastWindowShape,sizeof(SIGNAL*));
}

// This function is called when the Stft package is unloaded
// (for example when LastWave quits)
void FreeStftTabulations(void)
{
  unsigned long i;
  char windowShape;
  // Free memory allocated for tabulation of windows for every windowshape
  if(stftTabWindowShapes != NULL) {
    for(windowShape = 0; windowShape < LastWindowShape; windowShape++) {
      if(stftTabWindowShapes[windowShape] != NULL) {
	for(i=0; i < nTabWindowSizes; i++) {
	  if(stftTabWindowShapes[windowShape][i]!=NULL) {
	    DeleteSignal(stftTabWindowShapes[windowShape][i]);
	    stftTabWindowShapes[windowShape][i]=NULL;
	  }
	  else 
	    Warningf("FreeStftTabulations - Weird error - no signal found");
	}
	Free(stftTabWindowShapes[windowShape]);
	stftTabWindowShapes[windowShape] = NULL;
      }
    }
    Free(stftTabWindowShapes);
    stftTabWindowShapes = NULL;
  }
  else
    Warningf("FreeStftTabulations - stftTabWindowShapes is NULL!");
  // Free memory allocated for tabulation of real GG 
  if(stftTabGGR != NULL) {
    for(windowShape = 0; windowShape < LastWindowShape; windowShape++) {
      if(stftTabGGR[windowShape] != NULL) {
	for(i = 0; i < nTabWindowSizes; i++)
	  if(stftTabGGR[windowShape][i] != NULL) {
	    DeleteSignal(stftTabGGR[windowShape][i]);
	    stftTabGGR[windowShape][i] = NULL;
	  }
	Free(stftTabGGR[windowShape]);
	stftTabGGR[windowShape] = NULL;
      }
    }
    Free(stftTabGGR);
    stftTabGGR = NULL;
  }
  else
    Errorf("FreeStftTabulations - stftTabGGR is NULL!");

  //  Free memory allocated for tabulation of imag GG
  if(stftTabGGI != NULL) {
    for(windowShape = 0; windowShape < LastWindowShape; windowShape++) {
      if(stftTabGGI[windowShape] != NULL) {
	for(i = 0; i < nTabWindowSizes; i++)
	  if(stftTabGGI[windowShape][i] != NULL) {
	    DeleteSignal(stftTabGGI[windowShape][i]);
	    stftTabGGI[windowShape][i] = NULL;
	  }
	Free(stftTabGGI[windowShape]);
	stftTabGGI[windowShape] = NULL;
      }
    }
    Free(stftTabGGI);
    stftTabGGI = NULL;
  }
  else
    Warningf("FreeStftTabulations - stftTabGGI is NULL!");

  Free(stftTabWindowSizes);
  stftTabWindowSizes = NULL;
  nTabWindowSizes    = 0;
}
/* EOF */

