/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0                       */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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



/* FFT routines are from  NUMERICAL RECIPES */


#include "lastwave.h"
#include "signals.h"


#ifndef _REAL_
#define _REAL_
typedef float real;
#endif

#ifndef _COMPLEX_
#define _COMPLEX_
typedef struct
{
  real real;
  real imag;
} complex;
#endif


/* 1/N pour la ifft ??? */

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr

/*
 * Basic FFT Algorithm for computing the Fft of a complex signal
 * The result is in the original signal.
 */
void FftComplex_ (float *data,int nn,int fftSign)
{
	int n,mmax,m,j,istep,i;
	double wtemp,wr,wpr,wpi,wi,theta;
	float tempr,tempi;

    data--;
    
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


/* 
 * Interface with fastPkg
 */
 
static void FftComplex(complex *in_comp,complex *out_comp,int size)
{
  int i;
  complex tempr;
  
  if (sizeof(complex) != 2*sizeof(float)) 
     Errorf("FftComplex() : Sorry, you must declare real and imaginary parts of 'complex' as being floats");
  memcpy(out_comp,in_comp,size*sizeof(complex));
  FftComplex_((float *) out_comp,size,1);
  for (i=1;i<size/2;i++) {SWAP(out_comp[i],out_comp[size-i]);}
}
void (*cv_fft_c) (complex *in_comp,complex *out_comp,int size)  = FftComplex;


static void IFftComplex(complex *in_comp,complex *out_comp,int size)
{
  int i;
  complex tempr;

  if (sizeof(complex) != 2*sizeof(float)) 
     Errorf("IFftComplex() : Sorry, you must declare real and imaginary parts of 'complex' as being floats");
  memcpy(out_comp,in_comp,size*sizeof(complex));
  FftComplex_((float *) out_comp,size,-1);
  for (i=1;i<size/2;i++) {
    SWAP(out_comp[i],out_comp[size-i]);
    }
  for (i=0;i<size;i++) {
    out_comp[i].real /= size;
    out_comp[i].imag /= size;
  }
}
void (*cv_fft_c_i) (complex *in_comp,complex *out_comp,int size)  = IFftComplex;




/*
 * Basic FFT Algorithm for computing the Fft of a real signal
 * The result is in the orginal signal.
 */
 
static void FftReal_(float *data,int n,int isign)
{
	int i,i1,i2,i3,i4,n2p3;
	float c1=0.5,c2,h1r,h1i,h2r,h2i;
	double wr,wi,wpr,wpi,wtemp,theta;
    
    data--;
    
	theta=3.141592653589793/(double) n;
	if (isign == 1) {
		c2 = -0.5;
		FftComplex_(data+1,n,1);
	} else {
		c2=0.5;
		theta = -theta;
	}
	wtemp=sin(0.5*theta);
	wpr = -2.0*wtemp*wtemp;
	wpi=sin(theta);
	wr=1.0+wpr;
	wi=wpi;
	n2p3=2*n+3;
	for (i=2;i<=n/2;i++) {
		i4=1+(i3=n2p3-(i2=1+(i1=i+i-1)));
		h1r=c1*(data[i1]+data[i3]);
		h1i=c1*(data[i2]-data[i4]);
		h2r = -c2*(data[i2]+data[i4]);
		h2i=c2*(data[i1]-data[i3]);
		data[i1]=h1r+wr*h2r-wi*h2i;
		data[i2]=h1i+wr*h2i+wi*h2r;
		data[i3]=h1r-wr*h2r+wi*h2i;
		data[i4] = -h1i+wr*h2i+wi*h2r;
		wr=(wtemp=wr)*wpr-wi*wpi+wr;
		wi=wi*wpr+wtemp*wpi+wi;
	}
	if (isign == 1) {
		data[1] = (h1r=data[1])+data[2];
		data[2] = h1r-data[2];
	} else {
		data[1]=c1*((h1r=data[1])+data[2]);
		data[2]=c1*(h1r-data[2]);
		FftComplex_(data+1,n,-1);
	}
}

#undef SWAP

/* 
 * Interface with fastPkg
 */
 
static void FftReal(real *in_real,complex *out_comp,int size)
{
  int i;
  
  if (sizeof(real) != sizeof(float)) 
     Errorf("cv_fft_r() : Sorry, you must declare real as being floats");
  memcpy((float *) out_comp,(float *) in_real,size*sizeof(float));
  FftReal_((float *) out_comp,size/2,1);
  for (i=1;i<size/2;i++) out_comp[i].imag = -out_comp[i].imag;
}
void (*cv_fft_r) (real *in_real,complex *out_comp,int size)  = FftReal;


static void IFftReal(complex *in_comp,real *out_real,int size)
{
  int i;
  complex *c;
  
  if (sizeof(real) != sizeof(float)) 
     Errorf("IFftReal() : Sorry, you must declare real as being floats");
  memcpy((float *) out_real,(float *) in_comp,size*sizeof(float));
  c = (complex *) out_real;
  for (i=1;i<size/2;i++) c[i].imag = -c[i].imag;
  FftReal_((float *) out_real,size/2,-1);  

  for (i=0;i<size;i++) {
    out_real[i] /= size/2;
  }

}
void (*cv_fft_r_i) (complex *in_comp,real *out_real,int size)  = IFftReal;


/* 
 * Routines for extraction of real and imaginary parts
 */

static void Complex2RealImag(complex *com,real *rea,real *ima,int size)
{
  int i;

  for(i=0;i<size;i++)
    {
      rea[i] = com[i].real;
      ima[i] = com[i].imag;
    }
}

static void RealImag2Complex(float *rea,float *ima,complex *com,int size)
{
  int i;

  for(i=0;i<size;i++)
    {
      com[i].real = rea[i];
      com[i].imag = ima[i];
    }
}

static void RealImag2ComplexShift(float *rea,float *ima,complex *com,int size)
{
  int i;

  for(i=0;i<size/2;i++)
    {
      com[i].real = rea[i+size/2];
      com[i].imag = ima[i+size/2];
      com[i+size/2].real = rea[i];
      com[i+size/2].imag = ima[i];
    }
}

static void RealFT2RealImag(complex *com,float *rea,float *ima,int size)
{
  int i,sizeComp;

  sizeComp = size/2;
  for(i=1;i<sizeComp;i++)
    {
      rea[i] = com[i].real;
      ima[i] = com[i].imag;
    }

  rea[0] = com[0].real;
  rea[sizeComp] = com[0].imag;

  ima[0] = 0.;
  ima[sizeComp]= 0.;
}

static void RealImag2RealFT(float *rea,float *ima,complex *com,int size)
{
  int i,sizeComp;
  
  sizeComp = size-1;

  for(i=1;i<sizeComp;i++)
    {
      com[i].real = rea[i];
      com[i].imag = ima[i];
    }

  com[0].real = rea[0];
  com[0].imag =  rea[sizeComp];  
}



/*
 * The main Fourier transform function
 */
 
void Fft(SIGNAL inReal,SIGNAL inImag,SIGNAL outReal,SIGNAL outImag,int fftSign,char flagShift)
{
   float *fftData,*data,val;
   int i,size;

  /* Basic tests */
  if (inImag != NULL && inReal->size != inImag->size) 
     Errorf("Fft() : The size of the real and imaginary parts of the input signal must be equal");
  if (inImag == NULL && fftSign == -1) Errorf("Fft() : For ifft input signal must complex"); 
  if (outImag == NULL && fftSign == 1) Errorf("Fft() : For fft output signal must complex"); 
   


   /* 
    * Case of an ifft to obtain a real signal
    */
   if (outImag == NULL && fftSign == -1) {
   
     /*
      * We first test the size of the signals :
      * In case of an ifft to obtain a real signal, the size of the input signal
      * should be of the form 2^n+1 
      * (first point is frequency 0 and last one is frequency pi and both have real values)
      */  
     if (1 << ((int) (0.5+(log((double) (inReal->size-1))/log(2.)))) != inReal->size-1)
        Errorf("Fft() : The size of the input signal must be a power of 2 plus 1");
     if (inImag->Y[0] != 0) Errorf("Fft() : The imaginary part of frequency 0 should be 0");
     if (inImag->Y[inImag->size-1] != 0) Errorf("Fft() : The imaginary part of frequency Pi should be 0");

     /* Let's do it ! */
     fftData = (float *) Malloc(sizeof(float)*(inReal->size-1)*2);
     RealImag2RealFT(inReal->Y,inImag->Y,(complex *) fftData,inReal->size);
     SizeSignal(outReal,(inReal->size-1)*2,YSIG);
     IFftReal((complex *) fftData,(real *) outReal->Y,outReal->size);
     outReal->x0 = 0;
     outReal->dx = 1/(inReal->dx*outReal->size);
     Free(fftData);
     return;
   }
   

   /*
    * We test the size of the signals for all the other cases
    */
   if (1 << ((int) (0.5+(log((double) inReal->size)/log(2.)))) != inReal->size)
      Errorf("Fft() : The size of the input signal must be a power of 2");
   
   
   /*
    * Case of a fft of a real signal
    */
   if (inImag == NULL && fftSign == 1) {
     fftData = (float *) Malloc(sizeof(float)*inReal->size);
     FftReal((real *) inReal->Y,(complex *) fftData,inReal->size);
     SizeSignal(outReal,inReal->size/2+1,YSIG);
     SizeSignal(outImag,inReal->size/2+1,YSIG);
     RealFT2RealImag((complex *) fftData,(real *) outReal->Y,(real *) outImag->Y,inReal->size);
     Free(fftData);
     outImag->dx = outReal->dx = 1/(inReal->dx*inReal->size); 
     outImag->x0 = outReal->x0 = 0;
     return;
   }


   /*
    * Case of complex fft or ifft
    */
     if (inImag->size != inReal->size) 
       Errorf("Fft() : The real and imaginary parts of the input signal must have the same size");
     fftData = (float *) Malloc(2*sizeof(float)*inImag->size);
     data = (float *) Malloc(2*sizeof(float)*inImag->size);
     
     /* (I)FFT Complex */
     if (fftSign == -1 && flagShift) RealImag2ComplexShift((real *) inReal->Y,(real *) inImag->Y,(complex *) data,inReal->size);
     else RealImag2Complex((real *) inReal->Y,(real *) inImag->Y,(complex *) data,inReal->size);
   
     if (fftSign == 1) FftComplex((complex *) data,(complex *) fftData,inReal->size);
     else IFftComplex((complex *) data,(complex *) fftData,inReal->size);
     SizeSignal(outReal,inReal->size,YSIG);
     SizeSignal(outImag,inReal->size,YSIG);
     Complex2RealImag((complex *) fftData,(real *) outReal->Y,(real *) outImag->Y,inReal->size);
         
     Free(data);
     Free(fftData);
     
     if (fftSign == 1) {
       if (!flagShift) {
         outImag->dx = outReal->dx = 1/(inReal->dx*outImag->size); 
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
         outImag->x0 = outReal->x0 = -1./(2*inReal->dx);
       }
     }   
     else {
       outImag->dx = outReal->dx = 1/(inReal->dx*outImag->size); 
       outImag->x0 = outReal->x0 = 0;
     }
}

/*
 * Command for fft 
 */
void C_Fft(char **argv)
{
  SIGNAL sig1,sig2,sig3,sig4;
  char flagShift;
  int fftSign;
  char opt;
  float time;
  
  argv = ParseArgv(argv,tSIGNALI,&sig1,tSIGNAL,&sig2,tSIGNAL,&sig3,tSIGNAL_,NULL,&sig4,-1);
  
  flagShift = YES;
  fftSign = 1;
  while(opt = ParseOption(&argv))
    {
      switch(opt) 
        {
        case 's': 
          flagShift = NO;
          break;
        case 'i': 
         fftSign = -1;
          break;
        default:
          ErrorOption(opt);
        }
    }
  NoMoreArgs(argv);

  if (sig4 == NULL) {
    if (fftSign == 1) {
     sig4 = sig3;
     sig3 = sig2;
     sig2 = NULL;
   }
  }

  time = MyTime();
  
  Fft(sig1,sig2,sig3,sig4,fftSign,flagShift);
  
  SetResultFloat(MyTime()-time);                 
}

