/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0.3                     */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
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
/*  signal_functions.c                                                      */
/*                   Miscellaneous useful functions on signals				*/
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "signals.h"


    
/*********************************/
/* Print the values of a signal  */
/*********************************/

/*?????????? void C_Print(char **argv)
{
  SIGNAL signal;
  int j;

  argv = ParseArgv(argv,tSIGNALI,&signal,0);

  for (j = 0; j < signal->size; j++) {
    Printf("% .2e  %.8g\n", XSig(signal,j),signal->Y[j]);
  }
}*/

/*********************************/
/*     Put a signal to 0         */
/*********************************/

void ZeroSig(SIGNAL sig)
{
  int i;
  
  for(i=0;i<sig->size;i++) sig->Y[i] = 0;
  
}

/* 
 * Compute the Min and the max of a signal 
 * If *pxMin < *pxMax the yMin and yMax are computed only between these values
 */
void MinMaxSig(SIGNAL signal,float *pxMin,float *pxMax,float *pyMin,float *pyMax,
               int *piyMin,int *piyMax,int flagCausal)
{
  int i;
  int iMin,iMax;

  if(signal->size == 0) 
    Errorf("MinMaxSig() : signal is empty signal");

  if (flagCausal == YES) {
    iMin = signal->firstp;
    iMax = signal->lastp;
  }
  else {
    iMin = 0;
    iMax = signal->size-1;
  }
  
  if (*pxMin < *pxMax) {
    i = ISig(signal,*pxMin);
    iMin = MAX(iMin,i);
    i = ISig(signal,*pxMax);
    iMax = MIN(iMax,i);
  }
  
  for (i = iMin, (*pxMin) = (*pxMax) = XSig(signal,iMin),
              (*piyMin) = (*piyMax) = iMin, 
  			  (*pyMin) = (*pyMax) = signal->Y[iMin]; i <= iMax; i++) 
  	{
  		(*pxMin) = MIN(XSig(signal,i),(*pxMin));
  		if (signal->Y[i] < *pyMin) {
  		  (*pyMin) = signal->Y[i];
  		  *piyMin = i;
  		}
  		if (signal->Y[i] > *pyMax) {
  		  (*pyMax) = signal->Y[i];
  		  *piyMax = i;
  		}
  		(*pxMax) = MAX(XSig(signal,i),(*pxMax));
	}
	
}


/*****************************************************************/
/* This procedure thresholds a signal                            */
/*   flagX == YES ==> the threshold is on the X's                */
/*   flagY == YES ==> the threshold is on the Y's                */
/*   flagMin == YES ==> a min value is specified in 'min'        */
/*   flagMax == YES ==> a max value is specified in 'max'        */
/*****************************************************************/

void ThreshSig(SIGNAL in, SIGNAL out, int flagX, int flagY,
                  int flagMin, float min, int flagMax,float max)
{
  int i,j;
  float x,y;
  
  if (in == out) Errorf("ThreshSig() : the input and output signals must be different");
  
  if (flagY == YES) SizeSignal(out,in->size,XYSIG);
  else SizeSignal(out,in->size,in->type);
  
  out->firstp = out->lastp = -1;
  out->x0 = in->x0;
  out->dx = in->dx;
  out->size = 0;
  
  for (i=0,j=0;i<in->size;i++) {
    x = XSig(in,i);
    y = in->Y[i];
   
    if (flagX == YES && ((flagMin == YES && x < min) || (flagMax == YES && x > max))) continue;
    if (flagY == YES && ((flagMin == YES && y < min) || (flagMax == YES && y > max))) continue;

    if (out->firstp == -1 && i >= in->firstp) out->firstp = j;
    if (i <= in->lastp) out->lastp = j;
    
    if (out->type == XYSIG) out->X[j] = x;
    else if (j == 0) out->x0 = x;
    else if (j == 1) out->dx = x - out->x0;
      
    out->Y[j++] = y;
    out->size++; 
  }
  
  if (out->firstp == -1 || out->lastp == -1) {
    out->firstp = 1;
    out->lastp = 0;
  }
}


/* The corresponding command */
void C_Thresh(char **argv)
{
  SIGNAL in,out;
  int flagX,flagY,flagMin,flagMax;
  float min,max;
  char *arg1,*arg2;
  char opt;
    
  argv = ParseArgv(argv,tSIGNALI,&in,tSIGNAL,&out,-1);
  if (in == out) Errorf("Signals must be different");
  
  flagX = flagY = NO;
  while(opt = ParseOption(&argv)) { 
   switch(opt) {
   case 'x': 
     argv = ParseArgv(argv,tWORD,&arg1,tWORD,&arg2,-1);
     if (flagY == YES) Errorf("Can't set both option '-x' and '-y'");
     flagX = YES;
     if (!strcmp(arg1,"*")) flagMin = NO;
     else {
       flagMin = YES;
       ParseFloat(arg1,&min);
     }
     if (!strcmp(arg2,"*")) flagMax = NO;
     else {
       flagMax = YES;
       ParseFloat(arg2,&max);
     }
     break;
   case 'y': 
     argv = ParseArgv(argv,tWORD,&arg1,tWORD,&arg2,-1);
     if (flagX == YES) Errorf("Can't set both option '-x' and '-y'");
     flagY = YES;
     if (!strcmp(arg1,"*")) flagMin = NO;
     else {
       flagMin = YES;
       ParseFloat(arg1,&min);
     }
     if (!strcmp(arg2,"*")) flagMax = NO;
     else {
       flagMax = YES;
       ParseFloat(arg2,&max);
     }
     break;
    default:
     ErrorOption(opt);
   }
 }    
 NoMoreArgs(argv);

 if (flagMin == NO && flagMax == NO) Errorf("You have to specify either the Min or the Max");
   
 ThreshSig(in,out,flagX,flagY,flagMin,min,flagMax,max);
}
 

/****************************************/
/*    Sort a signal according           */
/****************************************/

/* Sorting two arrays of float        */
/* The sorting is made according to x */
/* The array y can be == NULL         */
/* 'n' is the size of the arrays      */

void SortArrays(float *x, float *y, int n)
{
  int l,j,ir,i;
  float xx,yy;
		
  l = ( n>>1)+1;
  ir = n;
  for (;;) {
    if(l>1) {
      xx = x[--l-1];
      if (y != NULL) yy = y[l-1];
    }
    else {
      xx = x[ir-1];
      x[ir-1] = x[0];
      if (y != NULL) {
        yy = y[ir-1];
        y[ir-1] = y[0];
      }
      if(--ir ==1) { 
      	x[0] = xx; 
      	if (y != NULL) y[0] = yy;
      	return; 
      }
    }
    i=l;
    j=l<<1;
    while( j<= ir) {
      if( j<ir && x[j-1]< x[j]) ++j;
      if(xx < x[j-1]) {
        x[i-1] = x[j-1];
        if (y != NULL) y[i-1] = y[j-1];
        j+=(i=j);
      }
      else j=ir+1;
    }
    x[i-1] = xx;
    if (y != NULL) y[i-1] = yy;
  }
}

/* Sort a signal either according to X or to Y */
void SortSig(SIGNAL signal)
{
  float *x,*y;
  int n;
	
  if (signal->size == 0) Errorf("SortSig() : Signal must be of size != 0");
  if (signal->size == 1) return;

  /* Do we need to sort ? */
  if (signal->type == XYSIG) {
    for (n=0;n<signal->size-1;n++) {
      if (signal->X[n] > signal->X[n+1]) break;
    }
    if (n == signal->size-1) return;
  }
  	    	
  /* Let's sort */	    	
  n = signal->size;
  switch (signal->type) {
    case XYSIG: 
      x = signal->X; 
      y = signal->Y; 
      break;
    case YSIG: 
      x = signal->Y; 
      if (signal->type == XYSIG) y = signal->X; 
      else y = NULL;
      break;
    default:
      Errorf("SortSig() : (Weired bug) Unknown signal type");
  }
  
  SortArrays(x,y,n);
}

/* The corresponding command */
void C_Sort(char **argv)
{
  SIGNAL signal;
  
  argv = ParseArgv(argv,tSIGNALI,&signal,0);
  
  SortSig(signal);
}


/******************************************/
/*    Make one XY signal from two signals */
/******************************************/

/* flag == YES  ==> out->X = in1->Y and out->Y = in2->Y            */
/* flag == NO   ==> 'in1->X' is put in 'in2' and 'in1->Y' in 'out' */

void MergeSig(SIGNAL in1, SIGNAL in2, SIGNAL out, int flag)
{
  int i;
  
  if (in1 == out || in2 == out)
    Errorf("MergeSig() : the output signal must be different from the input signals");
  
  if (flag == NO) {
    SizeSignal(out,in1->size,YSIG);
    SizeSignal(in2,in1->size,YSIG);
    
    for(i=0;i<in1->size;i++) {
      in2->Y[i] = XSig(in1,i);
      out->Y[i] = in1->Y[i];
    }
  } 
  else {
    SizeSignal(out,in1->size,XYSIG);  
    for(i=0;i<in1->size;i++) {
      out->X[i] = in1->Y[i];
      out->Y[i] = in2->Y[i];
    }
    SortSig(out);
  }
}



/*************************************
 *
 * Extract a signal from another one 
 *
 *************************************/

void ExtractSig(SIGNAL sig,SIGNAL sigOut, int borderType, int firstPoint,int newSize)
{
  SIGNAL sigOut1;
  int i,j,f;
  
  /* Some Checkings */
  if (sig->type == XYSIG && (firstPoint < 0 || firstPoint+newSize > sig->size))
    Errorf("ExtractSig() : Sorry I do not know how to manage border effects of a XY signal !");
    
  /* Set the sigOut1 */
  if (sigOut == sig) sigOut1 = NewSignal();
  else sigOut1 = sigOut;
    
  /* Init the sigOut1 */
  SizeSignal(sigOut1,newSize,sig->type);
  sigOut1->dx = sig->dx;  
  sigOut1->x0 = sig->x0 + firstPoint*sig->dx;
  sigOut1->firstp = sig->firstp - firstPoint;
  if (sigOut1->firstp <=0) sigOut1->firstp = 0;
  sigOut1->lastp = sig->lastp - firstPoint;
  if (sigOut1->lastp >= sigOut1->size) sigOut1->lastp = sigOut1->size-1;

  /* The main loop in case of XY sig */
  if (sig->type == XYSIG) {
    for (i= firstPoint,j=0;j<newSize;i++,j++) {
      sigOut1->Y[j] = sig->Y[i];
      sigOut1->X[j] = sig->X[i];
    }
  }

  /* The main loop in case of Y sig */
  else {  
  switch( borderType) {

    case BorderPad :
      for (i= firstPoint,j=0;j<newSize;i++,j++) {
        if (i<0) sigOut1->Y[j] = sig->Y[0];
        else if (i>=sig->size) sigOut1->Y[j] = sig->Y[sig->size-1];
        else sigOut1->Y[j] = sig->Y[i];
      }
      break;

    case BorderPad0 :
      for (i= firstPoint,j=0;j<newSize;i++,j++) {
        if (i<0) sigOut1->Y[j] = 0;
        else if (i>=sig->size) sigOut1->Y[j] = 0;
        else sigOut1->Y[j] = sig->Y[i];
      }
      break;
      
    case BorderPeriodic :
      if (firstPoint < 0) f = ((-firstPoint)/sig->size+1)*sig->size+firstPoint;
      else f = firstPoint;
      for (i= f,j=0;j<newSize;i++,j++) {
        sigOut1->Y[j] = sig->Y[i%sig->size];
      }
      break;

    case BorderMirror :
      if (firstPoint < 0) f = 2*((-firstPoint)/sig->size+1)*sig->size+firstPoint;
      else f = firstPoint;
      for (i= f,j=0;j<newSize;i++,j++) {
        if ((i/sig->size)%2 == 0) sigOut1->Y[j] = sig->Y[i%sig->size];
        else sigOut1->Y[j] = sig->Y[sig->size-1-i%sig->size];
      }
      break;
          
    default: Errorf("ExtractSig() : Bad Border type");
  }
  }
      
  
  /* Then set the sigOut */
  if (sigOut1 != sigOut) {
    CopySig(sigOut1,sigOut);
    DeleteSignal(sigOut1);
  }
  
}


/*********************/
/* Padd a signal     */
/*********************/

void PaddSig(SIGNAL sig,SIGNAL sigOut, int borderType,int newSize)
{
  SIGNAL sig1;
  int i,iMin,iMax,n,j;
  int sizen,size;
  
  /* Compute the new size */
  size = sig->size;
  if (newSize == -1) {
    if (size != 1) newSize = 1 << (1 + (int) (log((double) size-1)/log(2.))); 
    else newSize = 1;
  }
     
  /* If the same size don't do anything */
  if (newSize == size) return;
  
  /* Let's fill the new signal with the old one */
  sig1 = NewSignal();
  SizeSignal(sig1,newSize,YSIG);
  sig1->dx = sig->dx;
  iMin = (newSize-size)/2;
  iMax = iMin+size-1;
  for(i=iMin;i<=iMax;i++) sig1->Y[i] = sig->Y[i-iMin];
  sig1->x0 = sig->x0-iMin*sig->dx;
  
  /* Let's fill everything else */
  switch (borderType) {
    case BorderPad : 
      for(i=0;i<iMin;i++) sig1->Y[i] = sig->Y[0];
      for(i=iMax+1;i<newSize;i++) sig1->Y[i] = sig->Y[size-1];
      break;

    case BorderPad0 : 
      for(i=0;i<iMin;i++) sig1->Y[i] = 0;
      for(i=iMax+1;i<newSize;i++) sig1->Y[i] = 0;
      break;
      
    case BorderPeriodic : 
      n = newSize/size+1;
      sizen = n*size;
      for(i=0;i<iMin;i++) sig1->Y[i] = sig->Y[(i-iMin+sizen)%size];
      for(i=iMax+1;i<newSize;i++) sig1->Y[i] = sig->Y[(i-iMin+sizen)%size];
      break;

    case BorderMirror : 
      n = newSize/(2*size) +1;
      sizen = n*2*size;
      for(i=0;i<iMin;i++) {
        j = (i-iMin+sizen)%(2*size);
        if (j<size) sig1->Y[i] = sig->Y[j];
        else sig1->Y[i] = sig->Y[2*size-j-1];
      }
      for(i=iMax+1;i<newSize;i++) {
        j = (i-iMin+sizen)%(2*size);
        if (j<size) sig1->Y[i] = sig->Y[j];
        else sig1->Y[i] = sig->Y[2*size-j-1];
      }
      break;
    
    default: Errorf("PaddSig() : Bad Border type");
  }
      
  
  CopySig(sig1,sigOut);	
  
  DeleteSignal(sig1);
}

/* The corresponding command */
void C_Padd(char **argv)
{
  SIGNAL signal,signalOut;
  char *borderName;
  int borderType;
  int size;
  char opt;
  
  argv = ParseArgv(argv,tSIGNALI,&signal,tSIGNAL_,NULL,&signalOut,-1);
  if (signalOut == NULL) signalOut = signal;
  
  if (*argv != NULL || **argv != '-') argv = ParseArgv(argv,tWORD_,"*bconst",&borderName,-1);
  
  if(!strcmp(borderName,"pad") || !strcmp(borderName,"*bconst")) borderType = BorderPad;
  else if(!strcmp(borderName,"pad0") || !strcmp(borderName,"*b0")) borderType = BorderPad0;
  else if(!strcmp(borderName,"mir") || !strcmp(borderName,"*bmirror")) borderType = BorderMirror;
  else if(!strcmp(borderName,"per") || !strcmp(borderName,"*bperiodic")) borderType = BorderPeriodic;
  else Errorf("Undefined border effect: %s",borderName);

  size = -1;
  while(opt = ParseOption(&argv))
    {
      switch(opt) 
        {
       case 's': /*final size*/
	      argv = ParseArgv(argv,tINT,&size,-1);
	      if (size<=signal->size) Errorf("Size is not big enough!");
	      break;      
        default:
          ErrorOption(opt);
        }
    }
  NoMoreArgs(argv);
 
  PaddSig(signal,signalOut,borderType,size);
}

/**************************************/
/*    Compute statistical values      */
/**************************************/

/* Get moment of order 'n' (and returns the mean) */
float GetNthMoment(SIGNAL signal, int n, float *pNthMoment,int flagCausal, int flagCentered)
{
  int i,j;
  int iMin,iMax;
  float m,mn,s,f;
  
  if (n< 0) Errorf("GetNthMoment() : 'n' should be positive");
  
  if (n==0) {
    *pNthMoment = 1;
    return(1);
  }
  
  /* Border effects */
  if (flagCausal == YES) {
    iMin = signal->firstp;
    iMax = signal->lastp;
  }
  else {
    iMin = 0;
    iMax = signal->size-1;
  }
    
  /* Compute the mean */
  m = 0;
  if (flagCentered || n == 1) {
    for (i = iMin;i<=iMax;i++) m += signal->Y[i];
    m /= (iMax-iMin+1);
  }
  
  /* If n == 1 we are done ! */
  if (n == 1) {
    *pNthMoment = m;
    return(m);
  }
  
  /* Compute the Nth moment */
  mn = 0;
  for (i = iMin;i<=iMax;i++) {
    f = signal->Y[i]-m;
    s = 1;
    for (j=0;j<n;j++) s *= f;
    mn += s;
  }
  mn /= (iMax-iMin+1);

  *pNthMoment = mn;
  return(m);
}

/* Get absolute moment of order 'f1' (and returns the mean) */
float GetAbsMoment(SIGNAL signal, float f1, float *pMoment,int flagCausal, int flagCentered)
{
  int i;
  int iMin,iMax;
  float m,mn;
  
  if (f1==0) {
    *pMoment = 1;
    return(1);
  }
  
  /* Border effects */
  if (flagCausal == YES) {
    iMin = signal->firstp;
    iMax = signal->lastp;
  }
  else {
    iMin = 0;
    iMax = signal->size-1;
  }
    
  /* Compute the mean */
  m = 0;
  if (flagCentered) {
    for (i = iMin;i<=iMax;i++) m += signal->Y[i];
    m /= (iMax-iMin+1);
  }
  
  /* Compute the Nth moment */
  mn = 0;
  for (i = iMin;i<=iMax;i++) {
    mn += pow(fabs(signal->Y[i]-m),f1);
  }
  mn /= (iMax-iMin+1);

  *pMoment = mn;
  return(m);
}

/* Correlation (if signal2 == NULL the correlation is computed between X and Y) */
float GetCorrelation(SIGNAL signal1,SIGNAL signal2,int flagCausal)
{  
  int i;
  float y,y2,xy,x,x2;
  int iMin,iMax;
  SIGNAL s1,s2;
  char type1,type2;
  
  /* Border effects */
  if (flagCausal == YES) {
    iMin = signal1->firstp;
    iMax = signal1->lastp;
  }
  else {
    iMin = 0;
    iMax = signal1->size-1;
  }
  
  /* The values */
  if (signal2 == NULL) {
    s1 = s2 = signal1;
    type1 = 'X';
    type2 = 'Y';
  }
  else {
    s1 = signal1;
    s2 = signal2;
    type1 = type2 = 'Y';
  }
  
  for (i = iMin,x=y=xy=x2=y2=0.;i<=iMax;i++) {
    xy += XYSig(s1,i,type1)*XYSig(s2,i,type2);
    x += XYSig(s1,i,type1);
    x2 += (XYSig(s1,i,type1)*XYSig(s1,i,type1));
    y += XYSig(s2,i,type2);
    y2 += (XYSig(s2,i,type2)*XYSig(s2,i,type2));
  }
  xy /= (iMax-iMin+1);
  x /= (iMax-iMin+1);
  x2 /= (iMax-iMin+1);
  y /= (iMax-iMin+1);
  y2 /= (iMax-iMin+1);
  
  return((xy-x*y)*(xy-x*y)/((y2-y*y)*(x2-x*x)));
}

/****************************************/
/* Fit a signal with a straight line    */
/****************************************/

/*
 * signal       :   the signal to fit 
 * pA,pB        :   the equation line is y = (*pA)x+(*pB)
 * pSigA,pSigB  :   variance on (*pA) and (*pB)
 * iMin,iMax    :   the fit should be performed between iMin and iMax
 */
 
void LineFitSig(SIGNAL signal,float *pA,float *pSigA,float *pB,float *pSigB,int iMin,int iMax) 
{
  int i;
  float chi2,tmp;
  float t,sxoss,sx=0.0,sy=0.0,st2=0.0,ss,sigdat;

  iMin = MAX(0,iMin);
  iMax = MIN(signal->size-1,iMax);
  if (iMax <= 0 || iMin >= iMax) {
    iMin = 0;
    iMax = signal->size-1;
  }
  
  *pA = 0.0;
  for (i=iMin;i<iMax+1;i++) {
       sx += XSig(signal,i);
       sy += signal->Y[i];
  }
  ss = iMax-iMin+1;
  sxoss = sx/ss;
  for (i=iMin;i<iMax+1;i++){
       t = XSig(signal,i) - sxoss;
       st2 += t*t;
       *pA += t*signal->Y[i];
  }
  *pA /= st2;
  *pB = (sy-sx*(*pA))/ss;
  *pSigB = sqrt((1.0 + sx*sx/(ss*st2))/ss);
  *pSigA =  sqrt(1.0/st2);
  chi2 = 0.0;
  for (i=iMin;i<iMax+1;i++){
   tmp = signal->Y[i]-(*pB)-(*pA)*XSig(signal,i);
   chi2 += tmp*tmp;
  }
  sigdat = sqrt(chi2/(iMax-iMin-1));
  *pSigB *= sigdat;
  *pSigA *= sigdat;
}

/* Get Lp Norm (using the dx field) */
float GetLpNormSig(SIGNAL signal, float p,int flagCausal)
{
  int i;
  int iMin,iMax;
  float lp;
  
  /* Border effects */
  if (flagCausal == YES) {
    iMin = signal->firstp;
    iMax = signal->lastp;
  }
  else {
    iMin = 0;
    iMax = signal->size-1;
  }
     
  /* Compute the Lp Norm */
  lp = 0;
  for (i = iMin;i<=iMax;i++) {
    lp += pow(fabs(signal->Y[i]),p);
  }
  lp *= signal->dx;
  lp = pow(lp,1.0/p);

  return(lp);
}


/*
 * Command for computing some statistical values of a signal 
 */
 
void C_Stats(char **argv)
{
  char *action;
  SIGNAL signal,signal1;
  float f,m,f1;
  char opt;
  float xMin,xMax,yMin,yMax,a,b,sigA,sigB,p;
  int iMin,iMax;
  int flagCausal,flagAbs,flagCentered;
  LISTV lv;
    
  argv = ParseArgv(argv,tWORD,&action,-1);

  /* 'mean' action */
  if (!strcmp(action,"mean")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    GetNthMoment(signal,1,&f,flagCausal,NO);
    SetResultFloat(f);
  }

  /* 'var' action */
  else if (!strcmp(action,"var")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    GetNthMoment(signal,2,&f,flagCausal,YES);
    SetResultFloat(f);
  }

  /* 'skew' action */
  else if (!strcmp(action,"skew")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    GetNthMoment(signal,3,&f,flagCausal,YES);
    SetResultFloat(f);
  }

  /* 'kurt' action */
  else if (!strcmp(action,"kurt")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    GetNthMoment(signal,4,&f,flagCausal,YES);
    SetResultFloat(f);
  }
    
  /* 'nth' action */
  else if (!strcmp(action,"nth")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,tFLOAT,&f1,-1);
    flagCausal = NO;
    flagAbs = NO;
    flagCentered = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'a': flagAbs = YES; break;
        case 'c': flagCausal = YES; break;
        case 'C': flagCentered = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
   if (!flagAbs) {
     if (f1 != (int) f1 || f1 < 0) Errorf("<n> should be a positive integer unless you set option '-a'");
     m = GetNthMoment(signal,(int) f1,&f,flagCausal,flagCentered);
   }
   else m = GetAbsMoment(signal,f1,&f,flagCausal,flagCentered);
   SetResultFloat(f);
  }

  /* 'minmax' action */
  else if (!strcmp(action,"minmax")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    xMin = 1;
    xMax = -1;
    MinMaxSig(signal,&xMin,&xMax,&yMin,&yMax,&iMin,&iMax,flagCausal);
    lv = TNewListv();
    AppendInt2Listv(lv,iMin);
    AppendInt2Listv(lv,iMax);
    SetResultValue(lv);
  }

  /* 'lp' action */
  else if (!strcmp(action,"lp")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,tFLOAT,&p,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    f = GetLpNormSig(signal,p,flagCausal);
    SetResultFloat(f);
  }

  /* 'corr' action */
  else if (!strcmp(action,"corr")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,tSIGNALI_,NULL,&signal1,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    f = GetCorrelation(signal,signal1,flagCausal);
    SetResultFloat(f);
  }

  /* 'fit' action */
  else if (!strcmp(action,"fit")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    xMax = -1;
    xMin = 1;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'x': argv = ParseArgv(argv,tFLOAT,&xMin,tFLOAT,&xMax,-1); break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);  
    if (xMin >= xMax) {iMin = -1; iMax = -1;}
    else {
      iMin = ISig(signal,xMin);
      iMax = ISig(signal,xMax);
    }
    
    LineFitSig(signal,&a,&sigA,&b,&sigB,iMin,iMax);

   lv = TNewListv();
   AppendFloat2Listv(lv,a);
   AppendFloat2Listv(lv,sigA);
   AppendFloat2Listv(lv,b);
   AppendFloat2Listv(lv,sigB);
   AppendFloat2Listv(lv,iMin);
   AppendFloat2Listv(lv,iMax);
   SetResultValue(lv);
  }
  
  /* 'print' action */
  else if (!strcmp(action,"print")) {
    argv = ParseArgv(argv,tSIGNALI,&signal,-1);
    flagCausal = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'c': flagCausal = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    m = GetLpNormSig(signal,2,flagCausal);
    Printf("L2 norm   : %.8g\n",m);
    m = GetNthMoment(signal,2,&f,flagCausal,YES);
    Printf("Mean      : %.8g\n",m);
    Printf("Variance  : %.8g\n",f);
    m = GetNthMoment(signal,3,&f,flagCausal,YES);
    Printf("Skewness  : %.8g\n",f);
    m = GetNthMoment(signal,4,&f,flagCausal,YES);
    Printf("Kurtosis  : %.8g\n",f);
    xMin = 1;
    xMax = -1;
    MinMaxSig(signal,&xMin,&xMax,&yMin,&yMax,&iMin,&iMax,flagCausal);
    Printf("Minimum at x=%.8g (index = %d) is y = %.8g\n",XSig(signal,iMin),iMin,yMin); 
    Printf("Maximum at x=%.8g (index = %d) is y = %.8g\n",XSig(signal,iMax),iMax,yMax); 
  }

  else Errorf("Unknow action '%s'",action);    	
}


 
/**************************************/
/* Compute the 'n' branches histogram */
/* of the signal Y's.                 */
/**************************************/

/*
 * input    : the signal to make the histogram on 
 * output   : the signal to put the histogram into
 * n        : the number of branches of the histogram
 * ymin,ymax  : the yMin and yMax of the histogram (not used if min > max)
 * xmin,xmax  : the xMin and xMax between which the histogram is made
 * weight   : signal of weights (or NULL). It must have the same size as input 
 */
 
void HistoSig(SIGNAL input, SIGNAL output, int n,float xmin, float xmax,float ymin, float ymax, SIGNAL weight,int flagCausal)
{
  int i,index;
  float min,max;
  int iMin,iMax;
  float theWeight;  

  if (output == input) 
    Errorf("HistoSig() : the output signal must be different from the input signal");

  SizeSignal(output,n,YSIG); 
  ZeroSig(output);
  
  if (ymin >= ymax)
    if( xmin >= xmax) 
      MinMaxSig(input,&xmin,&xmax,&ymin,&ymax,&iMin,&iMax,flagCausal);
    else 
      MinMaxSig(input,&min,&max,&ymin,&ymax,&iMin,&iMax,flagCausal);
  else
    if( xmin >= xmax) 
      MinMaxSig(input,&xmin,&xmax,&min,&max,&iMin,&iMax,flagCausal);
 
  if (ymin == ymax) {ymin -= .5;ymax += .5;}

  if(flagCausal == YES)
    {
      iMin = input->firstp;
      iMax = input->lastp;
    }
  else
    {
      iMin = 0;
      iMax = input->size - 1;
    }

  for (i=iMin;i<=iMax;i++)
    {
      if(XSig(input,i)>xmax || XSig(input,i) < xmin) 
	continue;
      index = (int)((input->Y[i]-ymin)*n/(ymax-ymin));
      if (index < 0 || index > n) continue;
      if (index == n) index = n-1;
      
      if (weight == NULL) theWeight = 1;
      else theWeight = weight->Y[i];
      
      output->Y[index] += theWeight;
    }
  
  output->x0 = ymin + (ymax-ymin)/(2*n);
  output->dx = (ymax-ymin)/n;
}


/* The corresponding command */
void C_Histo (char **argv)
{
  SIGNAL input,output;
  int n,flagCausal=NO;
  float xmin,xmax;
  float ymin,ymax;
  SIGNAL weight;
  char opt;
  
  argv = ParseArgv(argv,tSIGNALI,&input,tSIGNAL,&output,tINT,&n,-1);

  xmin = 1;
  xmax = -1;
  ymin = 1;
  ymax = -1;
  weight = NULL;
  while(opt = ParseOption(&argv)) { 

    switch(opt) 
      {
      case 'x':
	argv = ParseArgv(argv,tFLOAT,&xmin,tFLOAT,&xmax,-1);
	if (xmin >= xmax) Errorf("xmin should be smaller than xmax");
	break;
      case 'y':
	argv = ParseArgv(argv,tFLOAT,&ymin,tFLOAT,&ymax,-1);
	if (ymin >= ymax) Errorf("ymin should be smaller than ymax");
	break;
      case 'w':
	argv = ParseArgv(argv,tSIGNAL,&weight,-1);
	if (weight->size != input->size) 
	  Errorf("The weight signal should be of the same size as the input signal");
	break;
      case 'c':
	flagCausal=YES;
	break;
      default:
	ErrorOption(opt);
      }
  }    
  NoMoreArgs(argv);
  
  if (input == output)
    Errorf("Input and Output must be different\n");
  
  HistoSig(input, output, n,xmin,xmax,ymin,ymax,weight,flagCausal);  
}



/**********************************************
 *
 * Convolution of a signal with a filter 
 *
 **********************************************/

void ConvSig(SIGNAL in, SIGNAL filter, SIGNAL out, int borderType,int method)
{
  int firstp, lastp;
  int filterOrigin;
  
  if (out == in) Errorf("ConvSig() : The input signal and the output signal must be different");
  if (out == filter) Errorf("ConvSig() : The filter signal and the output signal must be different");

  SizeSignal(out,in->size,YSIG);
  filterOrigin = (int) ((0-filter->x0)/filter->dx+.5);
  
  if (filterOrigin < 0 || filterOrigin > filter->size-1)
    Errorf("ConvSig() : The origin of the filter should be represented in the filter signal"); 

  cv_sig_init(CV_RC_FORM,in->Y,NULL,in->size);
  cv_flt_init_n(CV_RC_FORM,filter->size,filterOrigin,0,0,filter->Y,NULL);
  cv_set_method(method);
  cv_compute(borderType,out->Y,&firstp,&lastp);

  out->firstp = in->firstp+firstp;
  out->lastp = in->lastp - (in->size-1-lastp);
  
  out->dx = in->dx;
  out->x0 = in->x0;
}
  

void C_OldConv(char **argv)
{
  SIGNAL in,out,filter;
  char *borderName;
  int borderType;
  char opt;
  float time;
  int method = CV_UNDEFINED;

  argv = ParseArgv(argv,tSIGNALI,&in,tSIGNALI,&filter,tSIGNAL,&out,-1);
  
  borderName = "pad";
  if (*argv!=NULL && **argv != '-') argv = ParseArgv(argv,tSTR_,"pad",&borderName,-1);

  if(!strcmp(borderName,"pad")) borderType = BorderPad;
  else if(!strcmp(borderName,"mir")) borderType = BorderMirror;
  else if(!strcmp(borderName,"per")) borderType = BorderPeriodic;
  else if(!strcmp(borderName,"pad0")) borderType = BorderPad0;
  else Errorf("Undefined border effect: %s",borderName);


  while(opt = ParseOption(&argv))
    {
      switch(opt) 
        {
        case 'd': 
          method = CV_DI;
          break;
        case 'm': 
          method = CV_MP;
          break;
        case 'f': 
          method = CV_FT;
          break;          
        default:
          ErrorOption(opt);
        }
    }
  NoMoreArgs(argv);


  time = MyTime();
  
  ConvSig(in,filter,out,borderType,method);

  SetResultFloat(MyTime()-time);                 
}

/* A correlation function */
void CorrSig(SIGNAL in1, SIGNAL in2, SIGNAL out1, float dxmin, float dxmax, char flagNormalized, char flagCausal,SIGNAL npoints)
{
  int imin,imax,i,di;
  int jmin,jmax,j;
  int first1,last1;
  int first2,last2;
  float mean1,mean2,var1,var2;
  SIGNAL out;
  
  if (in1->dx != in2->dx) Errorf("CorrSig() : Sorry the signals must have the same dx");
  if (out1 == in1 || out1 == in2) out = NewSignal();
  else out = out1;

  imin = (int) (dxmin/in1->dx);
  imax = (int) (dxmax/in1->dx);
  di = (in2->x0-in1->x0)/in1->dx;

  if (npoints) {
    SizeSignal(npoints,imax-imin+1,YSIG); 
    ZeroSig(npoints);
  }

  SizeSignal(out,imax-imin+1,YSIG);
  out->dx = in1->dx;
  out->x0 = imin;
  ZeroSig(out);
  
  if (imin > imax) {
    if (out != out1) DeleteSignal(out);
    Errorf("CorrSig() : Sorry, dxmin should be smaller than dxmax");
  }
    
  if (flagCausal) {
    first1 = in1->firstp;
    last1 = in1->lastp;
    first2 = in2->firstp;
    last2 = in2->lastp;
  }
  else {
    first1 = 0;
    last1 = in1->size-1;
    first2 = 0;
    last2 = in2->size-1;
  }

  mean1 = 0;
  var1 = 0;
  for (i = first1; i<=last1;i++) {
    mean1 += in1->Y[i];    
    var1 += in1->Y[i]*in1->Y[i];
  }
  mean1 /= last1-first1+1;
  var1 /= last1-first1+1;
  var1 -= mean1*mean1;
    
  mean2 = 0;
  var2 = 0;
  for (i = first2; i<=last2;i++) {
    mean2 += in2->Y[i];    
    var2 += in2->Y[i]*in2->Y[i];
  }
  mean2 /= last2-first2+1;
  var2 /= last2-first2+1;
  var2 -= mean2*mean2;

  if (!flagNormalized) var1 = var2 = 1;
  
  for (i = imin; i<= imax;i++) {
    
    jmin = MAX(first1,first2+i+di);
    jmax = MIN(last1,last2+i+di);
    
    for (j=jmin;j<=jmax;j++) out->Y[i-imin] += (in1->Y[j]-mean1)*(in2->Y[j-i-di]-mean2);

    if (npoints) npoints->Y[i-imin]+=(jmax-jmin+1);

    out->Y[i-imin] /= (jmax-jmin+1);
    out->Y[i-imin] /= sqrt(var1*var2);

  }        
  
  if (out != out1) {
    CopySig(out,out1);
    DeleteSignal(out);
  }
}

void C_CorrSig(char **argv)
{
  SIGNAL in1,in2,out;
  float dxmin, dxmax;
  char flagCausal,flagNormalized;
  char opt;
  SIGNAL npoints;
  
  argv = ParseArgv(argv,tSIGNALI,&in1,tSIGNALI,&in2,tSIGNAL,&out,tFLOAT,&dxmin,tFLOAT,&dxmax,tSIGNAL_,NULL,&npoints,-1);
  
  if (dxmin >= dxmax) Errorf("Sorry, dxmin should smaller than dxmax");
  
  flagCausal = YES;
  flagNormalized = YES;
  while(opt = ParseOption(&argv)) {
    switch(opt) {
    case 'c':
      flagCausal = NO;
      break;
    case 'n':
      flagNormalized = NO;
      break;
    default: 
      ErrorOption(opt);
    }
  }    
  NoMoreArgs(argv);

  CorrSig(in1,in2,out,dxmin,dxmax,flagNormalized,flagCausal,npoints);
}
