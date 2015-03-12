/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry and J.Abadia           */
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


/*#define DEBUGSHIFT*/

/************************************************************/
/*     The windows                                          */
/*       the following functions must                       */
/*       fill up a signal                                   */
/*       corresponding to f(i+shift)                        */
/*       (starting i = 0)                                   */ 
/*       The signal must be normalized                      */
/*       The return value is the normalization factor       */
/************************************************************/


/*************************/
/*    Order 0 spline     */
/*************************/

static float Spline0(SIGNAL window,float shift)
{
  int j;
  float i;
  float energy = 0.0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Spline0 : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Spline0 : bad shift %g",shift);
  
  /* The window */
  for(j = 0, i=shift; i < window->size; j++, i += 1.0) {
    window->Y[j] = 1.0;
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j = 0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  
  return(factor);
}

/*************************/
/*    Order 1 spline     */
/*************************/

static float Spline1(SIGNAL window,float shift)
{
  int j;
  float i;
  float temp;
  float energy = 0.0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Spline1 : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Spline1 : bad shift %g",shift);
  
  temp = window->size/2.0;
  /* The window */
  for(j=0, i = shift; i < window->size/2; j++, i += 1.0) {
    window->Y[j] = i/temp;
    energy += window->Y[j]*window->Y[j];
  }
  for(; i < window->size; j++, i += 1.0) {
    window->Y[j] = (2-i/temp);
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  
  return(factor);
}

/*************************/
/*    Order 2 spline     */
/*************************/

static float Spline2(SIGNAL window,float shift)
{
  int j;
  float i;
  float temp;
  float x;
  float energy = 0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Spline2 : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Spline2 : bad shift %g",shift);
  
  temp = window->size/3.0;
  
  /* The window */
  for(i = shift, j=0; i<temp; i++,j++) {
    x = i/temp;
    window->Y[j] = .5*x*x;
    energy += window->Y[j]*window->Y[j];
  }
  for(;i<2*temp;i++,j++) {
    x = i/temp-1.5;
    window->Y[j] = (3.0/4.0 - x*x);
    energy += window->Y[j]*window->Y[j];
  }
  for( ; i < window->size && j < window->size; i++,j++) {
    x = 3-i/temp;
    window->Y[j] = .5*x*x;
    energy += window->Y[j]*window->Y[j];
  }
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0;j<window->size;j++) {
    window->Y[j] *= factor;
  }
  
  return(factor);
}


/*************************/
/*    Order 3 spline     */
/*************************/

static float Spline3(SIGNAL window,float shift)
{
  float i;
  int j;
  float temp = window->size/4.0;
  float x;
  float energy = 0;
  float factor;
  
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Spline3 : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Spline3 : bad shift %g",shift);
  
  /* The window */
  for(i=shift, j=0; i<temp; i++,j++) {
    x = i/temp;
    window->Y[j] = x*x*x/6;
    energy += window->Y[j]*window->Y[j];
  }
  for(; i < 2*temp; i++,j++) {
    x = i/temp;
    window->Y[j] = -.5*x*x*x+2*x*x-2*x+2.0/3;
    energy += window->Y[j]*window->Y[j];
  }
  for(; i < 3*temp; i++,j++) {
    x = 4-i/temp;
    window->Y[j] = -.5*x*x*x+2*x*x-2*x+2.0/3;
    energy += window->Y[j]*window->Y[j];
  }
  for(; i < window->size; i++,j++) {
    x = 4-i/temp;
    window->Y[j] = x*x*x/6;
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  
  return(factor);
}


/*************************/
/*   Gaussian            */
/*************************/

/*double theSigma2 = 0.014;   */
double theGaussianSigma2 = 0.02;  

static float Gauss(SIGNAL window,float shift)
{
  float i;
  int   j;
  float x;
  float energy = 0.0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Gauss : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Gauss : bad shift %g",shift);
  
  /* The window */
  for(i = shift, j=0; j < window->size; j++,i++) {
    x = (i-window->size/2)/window->size;
    window->Y[j] = exp(-x*x/(2*theGaussianSigma2));
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0;j<window->size;j++) {
    window->Y[j] *= factor;
  }
  return(factor);
}


static float Hamming(SIGNAL window,float shift)
{
  float i;
  int   j;
  float x;
  float energy = 0.0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Hamming : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Hamming : bad shift %g",shift);
  
  /* The window */
  for(i = shift, j=0; j < window->size; j++,i++) {
    x = (i-window->size/2)/window->size;
    window->Y[j] = 0.54+0.46*cos(2*M_PI*x);
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0;j<window->size;j++) {
    window->Y[j] *= factor;
  }
  return(factor);
}

static float Hanning(SIGNAL window,float shift)
{
  float i;
  int   j;
  float x;
  float energy = 0.0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Hanning : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Hanning : bad shift %g",shift);
  
  /* The window */
  for(i = shift, j=0; j < window->size; j++,i++) {
    x = (i-window->size/2)/window->size;
    window->Y[j] = cos(M_PI*x);
    window->Y[j] *= window->Y[j];
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0;j<window->size;j++) {
    window->Y[j] *= factor;
  }
  return(factor);
}

static float Blackman(SIGNAL window,float shift)
{
  float i;
  int   j;
  float x;
  float energy = 0.0;
  float factor;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Blackman : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Blackman : bad shift %g",shift);
  
  /* The window */
  for(i = shift, j=0; j < window->size; j++,i++) {
    x = (i-window->size/2)/window->size;
    window->Y[j] =  0.42+0.5*cos(2*M_PI*x)+0.08*cos(4*M_PI*x);
    window->Y[j] *= window->Y[j];
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero */
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  factor = 1/sqrt(energy);
  for(j=0;j<window->size;j++) {
    window->Y[j] *= factor;
  }
  return(factor);
}
/*************************/
/*    	Exponential      */
/*************************/

/*  Exponential asymmetric window */

static float Exponential(SIGNAL window,float shift)
{
  float decay = 1e4;
  int j;
  float i;
  float energy = 0.0;
  float factor;
  float a;
  float expon;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Asymmetric : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Asymmetric : bad shift %g",shift);
  
  /* Damping factor */
  
  a = log(decay);
  expon = a/window->size;  /* scaled */
  
  
  /* The window */
  
  for(j=0, i=-1; i < window->size-1; j++, i++) {
    window->Y[j] = exp(-expon*i);
    energy += window->Y[j]*window->Y[j];
  }
  
  
  /* Putting the first point to zero*/
  
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  
  factor = 1/sqrt(energy);
  
  for(j = 0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  /*  Printf("Asymmetric Window:(a: %f,factor: %f,size: %d)\n",a,factor,window->size); javi */
  
  return(factor);
}

/*************************/
/*          FoF	         */
/*************************/

/*  FoF (Fonction d'onde Formantique) window 			   */
/* 		g(t)= 0.5*exp(-a*t)*(1-cos(b*t))     0<= t <= PI/b */
/*		g(t)=     exp(-a*t)                      t >  PI/b */

float decayFoF = 1e5; /* 1e5 */ 
float betaFoF = M_PI/0.25; /* pi/0.25 */

static float FoF(SIGNAL window,float shift)
{ 
  int j;
  float limit;
  float i;
  float beta;
  float energy = 0.0;
  float factor;
  float a;
  float expon;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("FoF : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("FoF : bad shift %g",shift);
  
  /* Damping factor */
  
  a = log(decayFoF);
  
  
  /* scale */
  
  expon = a/window->size; 
  beta = betaFoF/window->size;
  
  /* limit of the cos */
  
  limit = M_PI/beta;
  
  /* The window */
  
  for(j=0, i=0.0; i <= limit ; j++, i += 1.0) {
    window->Y[j] = 0.5*(1-cos(beta*i))*exp(-expon*i);
    energy += window->Y[j]*window->Y[j];
  }
  
  for(; i < window->size; j++, i += 1.0) {
    window->Y[j] = exp(-expon*i);
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero*/
  
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  
  factor = 1/sqrt(energy);
  
  for(j = 0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  /*  Printf("FoF:(a: %f,factor: %f,size: %d,limit: %f)\n",a,factor,window->size,limit);  javi */
  
  return(factor);
}

/*************************/
/*    Asymmetric3        */
/*************************/

/*  Another asymmetric window */


static float Asymmetric3(SIGNAL window,float shift)
{
  float decay = 1e4;  
  int j;
  float limit;
  float beta = M_PI/(log(window->size)/log(2)-1);	/* beta = PI/octave */
  float i;
  float energy = 0.0;
  float factor;
  float a;
  float expon;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Asymmetric : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Asymmetric : bad shift %g",shift);
  
  /* Damping factor */
  
  a = log(decay);
  expon = a/window->size;  /* scaled */
  
  /* limit of the cos */
  
  limit = M_PI/beta;
  
  /* The window */
  
  for(j=0, i=0.0; i <= limit ; j++, i += 1.0) {
    window->Y[j] = 0.5*(1-cos(beta*i))*exp(-expon*limit);
    energy += window->Y[j]*window->Y[j];
  }
  
  for(; i < window->size; j++, i += 1.0) {
    window->Y[j] = exp(-expon*i);
    energy += window->Y[j]*window->Y[j];
  }
  
  /* Putting the first point to zero*/
  
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  
  factor = 1/sqrt(energy);
  
  for(j = 0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  /*  Printf("Asymmetric Window:(a: %f,factor: %f,size: %d,limit: %f)\n",a,factor,window->size,limit);  javi */
  
  return(factor);
}

/*  asymmetric window */
static float Asymmetric32(SIGNAL window,float shift)
{
  float decay = 1e4;  
  int j;
  float i;
  float energy = 0.0;
  float factor;
  float a;
  float expon;
  
  /* Checking argument */
  if(window == NULL)
    Errorf("Asymmetric : NULL window");
  if(shift < 0 || shift >= 1)
    Errorf("Asymmetric : bad shift %g",shift);
  
  /* Damping factor */
  
  a = log(decay);
  expon = a/window->size;  /* scaled */
  
  
  /* The window */
  
  for(j=0, i=0; i < window->size; j++, i++) {
    window->Y[j] = (i+1)*exp(-expon*i);
    energy += window->Y[j]*window->Y[j];
  }
  
  
  /* Putting the first point to zero*/
  
  energy -= window->Y[0]*window->Y[0];
  window->Y[0] = 0.0;
  
  /* Normalizing */
  
  factor = 1/sqrt(energy);
  
  for(j = 0; j < window->size; j++) {
    window->Y[j] *= factor;
  }
  Printf("Asymmetric Window:(a: %f,factor: %f,size: %d)\n",a,factor,window->size); /* javi */
  
  return(factor);
}


/**********************************************/
/*      Get the window function            */
/**********************************************/

void GetWindowShapeFunc(char windowShape, float (**f)(SIGNAL,float))
{
  if(!WindowShapeIsOK(windowShape))
    Errorf("GetWindowShapeFunc : unknown window shape %d",windowShape);
  
  switch(windowShape) {
  case Spline0WindowShape :
    *f = &Spline0;
    break;
  case Spline1WindowShape :
    *f = &Spline1; 
    break;
  case Spline2WindowShape : 
    *f = &Spline2; 
    break;
  case Spline3WindowShape : 
    *f = &Spline3; 
    break;
  case GaussWindowShape : 
    *f = &Gauss; 
    break;
  case HammingWindowShape : 
    *f = &Hamming; 
    break;
  case HanningWindowShape : 
    *f = &Hanning; 
    break;
  case BlackmanWindowShape : 
    *f = &Blackman; 
    break;
  case ExponentialWindowShape : 
    *f = &Exponential; 
    break;
  case FoFWindowShape : 
    *f = &FoF; 
    break;
  case Asym3WindowShape : 
    *f = &Asymmetric3; 
    break;
  default: 
    Errorf("GetWindowShapeFunc : bad window shape %d",windowShape);
    break;
  }
}

char  Name2WindowShape(char *name)
{
  if(!strcmp(name,"spline0")) return(Spline0WindowShape);
  if(!strcmp(name,"spline1")) return(Spline1WindowShape);
  if(!strcmp(name,"spline2")) return(Spline2WindowShape);
  if(!strcmp(name,"spline3")) return(Spline3WindowShape);
  if(!strcmp(name,"gauss")) return(GaussWindowShape);
  if(!strcmp(name,"hamming")) return(HammingWindowShape);
  if(!strcmp(name,"hanning")) return(HanningWindowShape);
  if(!strcmp(name,"blackman")) return(BlackmanWindowShape);
  if(!strcmp(name,"exponential")) return(ExponentialWindowShape);
  if(!strcmp(name,"FoF")) return(FoFWindowShape);
  if(!strcmp(name,"asymmetric3")) return(Asym3WindowShape);
  Errorf("Unknown window name '%s'",name);
  
  // This should never be reached but the compiler may want to be sure the function returns a value
  return(LastWindowShape);
}

char * WindowShape2Name(char windowShape)
{
  if(!WindowShapeIsOK(windowShape))
    Errorf("WindowShape2Name : Unknown window shape %d",windowShape);
  
  switch(windowShape) {
  case Spline0WindowShape : return("spline0");
  case Spline1WindowShape : return("spline1");
  case Spline2WindowShape : return("spline2");
  case Spline3WindowShape : return("spline3");
  case GaussWindowShape   : return("gauss");
  case HammingWindowShape   : return("hamming");
  case HanningWindowShape   : return("hanning");
  case BlackmanWindowShape   : return("blackman");
  case ExponentialWindowShape   : return("exponential");
  case FoFWindowShape   : return("FoF");
  case Asym3WindowShape   : return("asymmetric3");	
  }
  Errorf("Unknown window shape %d",windowShape);
  // This should never be reached but the compiler may want to be sure the function returns a value
  return("unknown");
}

#ifdef STFT_ADVANCED
void C_GetWindowShape(char **argv)
{
  char *windowShapeName;
  float (*f) (SIGNAL,float);
  
  SIGNAL signal;
  float shift;
  int size;
  float factor;
  
  argv = ParseArgv(argv,tSIGNAL,&signal,tINT,&size,tFLOAT_,0.0,&shift,tSTR_,"gauss",&windowShapeName,0);
  if(shift < 0 || shift >= 1)
    Errorf("shift = %g should remain within [0,1[");
  GetWindowShapeFunc(Name2WindowShape(windowShapeName),&f);
  SizeSignal(signal,size,YSIG);
  factor = (*f)(signal,shift);
  /*    SetResultFloat(factor); */
  return;
}
#endif

char GetFlagAsymWindowShape(char windowShape)
{
  char flagAsym;
  
  switch(windowShape) {
  case ExponentialWindowShape :
    flagAsym = YES;
    break;
  case FoFWindowShape :
    flagAsym = YES;
    break;
  case Asym3WindowShape :
    flagAsym = YES;
    break;
  default:
    flagAsym = NO;
    break;
  }
  return(flagAsym);
}

/* 
 * 'Max window' is the index of the 'maximum'
 * of the window (sometimes it is not actually the maximum ...
 * in the array of size 'size', 
 * where indexes run from 0 to size-1
 */
int GetMaxWindowShape(char windowShape,int size)		/* javi */
{
  int max;
  
  switch(windowShape) {	
  case ExponentialWindowShape:
    max = 1;
    break;
  case FoFWindowShape:
    /*	max = GetMaxFoF(size); */
    max = 1;
    break;
  case Asym3WindowShape:
    max = log(size)/log(2)-1;
    break;
  default:
    max = size/2;
    break;
  }
  return(max);
}

int GetMaxFoF(int size)
{
  int Max;
  int octave;
  
  octave = log(size)/log(2);
  
  switch(octave) {	
  case 2:
    Max = 1;
    break;
  case 3:
    Max = 1;
    break;
  case 4:
    Max = 1;
    break;
  case 5:
    Max = 3;
    break;
  case 6:
    Max = 5;    
    break;
  case 7:
    Max = 10;
    break;
  case 8:
    Max = 21;
    break;
  case 9:
    Max = 42;
    break;
  case 10:
    Max = 84;
    break;
  case 11:
    Max = 168;
    break;
  case 12:
    Max = 335;
    break;
  case 13:
    Max = 670;
    break;
  case 14:
    Max = 1341;
    break;
  case 15:
    Max = 2682;
    break;
  case 16:
    Max = 5364;
    break;
  default:
    Errorf("GetMaxFof : octave %d not treated",octave);
    Max = 1; // The compiler wants to see an initialized return value
    break;
  }
  return(Max);
}

/* Computes the time support of a window, that is to say 
 * the closed interval [timeIdMin,timeIdMax] on which the
 * window is non zero
 * WARNING : it is the calling function's responsibility
 *           to check whether this support intersects the
 *           range of indexes of an array (e.g. [0,signalSize-1])
 */
void ComputeWindowSupport(int windowSize,char windowShape,float timeId,
			  int *pTimeIdMin,int *pTimeIdMax)
{
  int maxWindowShape;
  
  /* Checking arguments */
  if(!INRANGE(1,windowSize,STFT_MAX_WINDOWSIZE))
    Errorf("ComputeWindowSupport : bad windowSize %d",windowSize);
  if(!WindowShapeIsOK(windowShape))
    Errorf("ComputeWindowSupport : unknown windowShape %d",windowShape);
  if(pTimeIdMin == NULL || pTimeIdMax == NULL)
    Errorf("ComputeWindowSupport : NULL output");
  
  /* Computing */
  // Location index of the maximum of the window
  maxWindowShape=GetMaxWindowShape(windowShape,windowSize);
  
  if(timeId == (int) timeId) {
    /* Checked correct on the 20/02/2001 by R. Gribonval
     *
     * a/timeId-maxWindowShape is the index of the first 
     * point of the window, which value is set to zero, hence the +1
     * b/the indexes range from 0 to windowSize-1, shifted by 
     * timeId-maxWindowShape
     */
    *pTimeIdMin = timeId-maxWindowShape+1;
    *pTimeIdMax = timeId-maxWindowShape+windowSize-1; 
  }
  /* We should define what happens when timeId != integer */
  else
    Errorf("ComputeWindowSupport : timeId %g is not an integer, behavior undefined",timeId);
  
  /* *pTimeIdMin = timeId-windowSize2+1;
   *pTimeIdMax = timeId+windowSize2-1;  */
}

/* EOF */

