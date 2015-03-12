/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'compress2d' 2.0                   */
/*                                                                          */
/*      Copyright (C) 1998-2002 Geoff Davis, Emmanuel Bacry, Jerome Fraleu. */
/*                                                                          */
/*      The original program was written in C++ by Geoff Davis.             */
/*      Then it has been translated in C and adapted to LastWave by         */
/*      J. Fraleu and E. Bacry.                                             */
/*                                                                          */
/*      If you are interested in the C++ code please go to                  */
/*          http://www.cs.dartmouth.edu/~gdavis                             */
/*                                                                          */
/*      emails : geoffd@microsoft.com                                       */
/*               fraleu@cmap.polytechnique.fr                               */
/*               lastwave@cmap.polytechnique.fr                             */
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
#include "bitio.h"
#include "ihisto.h"
#include "arithcode.h"
#include "coder.h"
#include "quantize.h"

#define MaxReal (FLT_MAX-1)


static float intToReal (int n, float StepSize)
{ 
  return ((float)(n) * StepSize); 
}

static int realToInt (float x,  float StepSize) 
{
  return (int)(x / StepSize);	  
}


/*---------------------------------------------------------------------------*/
UNIFORMQUANT NewUniformQuant (MONOLAYERCODER entropy, int paramPrecision,float StepSize,int zeroCenter) 
{ 
  UNIFORMQUANT u;
  int  p=2;
  
  u = (UNIFORMQUANT) Malloc(sizeof(UniformQuant));
  
  u->entropy = entropy;
  u->paramPrecision = paramPrecision;
  u->stepSize = StepSize;
  u->zeroCenter = zeroCenter;
  
  u->data = NULL;
  u->nData = 0;
  u->max = u->min = u->sum = u->sumSq = u->mean = u->var = 0;
  u->initialDist = 0;
  u->symbol = NULL;
  u->nSymbol = 0;

  u->p = p;
  
  return(u);
}

/*---------------------------------------------------------------------------*/
void DeleteUniformQuant (UNIFORMQUANT u)
{        
  if (u->nSymbol != 0) Free(u->symbol);
  Free(u);
}


/*---------------------------------------------------------------------------*/
void GetStatsUniformQuant (UNIFORMQUANT u)
{
  int i;
  
  u->max = -MaxReal;
  u->min = MaxReal;
  u->sum = u->sumSq = 0;

  for (i = 0; i < u->nData; i++) {
    if (u->data[i] < u->min)
      u->min = u->data[i];
    if (u->data[i] > u->max)
      u->max = u->data[i];
    u->sum += u->data[i];
    u->sumSq += u->data[i]*u->data[i];
  }
  u->mean = u->sum / (float) u->nData;
  u->var = u->sumSq / (float) u->nData - u->mean*u->mean;
}

/*---------------------------------------------------------------------------*/
void SetDataEncodeUniformQuant (UNIFORMQUANT u, float *newData, int newNData)
{ 
  int i;
  
  u->data = newData;
  u->nData = newNData;

  GetStatsUniformQuant (u);
  
  if (u->zeroCenter) {
    u->max = fabs(u->max) > fabs(u->min) ? fabs(u->max) : fabs(u->min);
    u->min = -u->max;
  }
 
 
  u->imax = realToInt (u->max, u->stepSize); 
  u->qmax = intToReal (u->imax,u->stepSize);
  
  /* Make sure qmax >= max and -qmax <= min*/
   if (u->imax!=0)
  while (u->qmax < u->max )
    u->qmax = intToReal (++(u->imax), u->stepSize);
 
  if (u->zeroCenter) {
    u->imin = -u->imax;
    u->qmin = -u->qmax;
 
  } else {  
    u->imin = realToInt (u->min, u->stepSize);
   u->qmin = intToReal (u->imin, u->stepSize);
    
    /* Make sure qmin <= min */
   if (u->imin!=0)
  
    while (u->qmin > u->min )
      u->qmin = intToReal (--(u->imin), u->stepSize);
   
  }

  
  u->imean = realToInt (u->mean, u->stepSize);
  u->qmean = intToReal (u->imean, u->stepSize);
 
 
  u->initialDist = 0;
  if (u->zeroCenter)
    for (i = 0; i < u->nData; i++)
      u->initialDist += pow(fabs(u->data[i]),u->p);
  else
    for (i = 0; i < u->nData; i++)
      u->initialDist += pow(fabs(u->data[i] - u->qmean),u->p);
}



/*---------------------------------------------------------------------------*/
void SetDataDecodeUniformQuant (UNIFORMQUANT u, float *newData, int newNData, int imax, int imin, int imean)
{
  u->data = newData;
  u->nData = newNData;

  if (imin < imax) {
    u->qmax = intToReal (imin, u->stepSize);
    
    if (u->zeroCenter) {
      u->qmin = -u->qmax;
      imin= -imax;
    } else {
      u->qmin = intToReal (imin, u->stepSize);
    }
    u->qmean = intToReal (imean, u->stepSize);
  }
 
 
  
}
/*---------------------------------------------------------------------------*/
void GetRateDistUniformQuant( UNIFORMQUANT u, float minStepSize,float *rate, float *dist) 
{
   float reconstruct;
   int  nSteps;
   int i;
   
   *rate=0.0;
   *dist =0.0;
    if (u->stepSize < minStepSize) {
      *rate = MaxReal;
      *dist = MaxReal;
      return;
  }
    if (u->symbol) Free(u->symbol);
    u->nSymbol = u->nData;
    u->symbol = IntAlloc(u->nSymbol);

     if (u->qmin ==0 && u->qmax==0) 
       { *rate = 0.0;
         for (i = 0; i < u->nData; i++) 
           { *dist  += pow(u->data[i],u->p);
	     u->symbol[i]=0; 
        }
       }
     else { 

     if (u->qmin>=0)  {/*  !(Zerocenter)  */
      nSteps = u->imax-u->imin;
      
      if (nSteps==0) SetNSymMonoLayerCoder(u->entropy,1);
      else  SetNSymMonoLayerCoder(u->entropy,nSteps);
     }
     else {
        nSteps = u->imax-u->imin -1 ;
     
      if (nSteps<=0) nSteps =0; 
      if (nSteps==0) SetNSymMonoLayerCoder(u->entropy,1);
      else SetNSymMonoLayerCoder(u->entropy,nSteps); 
     }
    
    *rate = *dist = 0;


    for (i = 0; i < u->nData; i++) {
     /*  encode */  
	if (u->qmin>=0)  {/*  !(Zerocenter)  */        
       /* data[i] always >0*/
         u->symbol[i] = (int)(u->data[i]/ u->stepSize) -u->imin;
       
       }  
    else {
      if (fabs(u->data[i]) < u->stepSize) u->symbol[i] = 0;
      else {
       if (u->data[i]>0) 
        
          u->symbol[i] = (int)(u->data[i]/ u->stepSize) ;
       else 
	  u->symbol[i] = (int)(fabs(u->data[i])/ u->stepSize)  +u->imax-1;
       }
    
     }
	
     
      *rate += CostEntropyCoder((ENTROPYCODER) (u->entropy),u->symbol[i], YES,-1,-1); 
  
   	/*  decode */  
    if (u->qmin>=0)  {/*  !(Zerocenter)  */
     /* data[i] always >0*/
        reconstruct = (float) (u->symbol[i]+ u->imin +0.5) *u->stepSize ;
    }  
    else {/*  (Zerocenter)  */
     
      if (u->symbol[i] == 0)  reconstruct = 0; 
       else {
       if (u->symbol[i] > (int) ((nSteps-1)/2))
          reconstruct = - (float) (u->symbol[i] +1-u->imax+0.5) *u->stepSize ;
       else 
          reconstruct = (float) (u->symbol[i]+0.5) *u->stepSize ;
     }
    }     
     *dist += pow(fabs(u->data[i]-reconstruct),u->p);
    }
  }
  

}

/*---------------------------------------------------------------------------*/
void EncodeUniformQuant (UNIFORMQUANT u, ENCODER encoder)
{   
  int i=0;
  int nSteps;
  
  if (!(u->qmin ==0 && u->qmax ==0)) {
     
   if (u->qmin>=0)  {/*  !(Zerocenter)  */
     
      nSteps = u->imax-u->imin;

      if (nSteps == 0) 
        SetNSymMonoLayerCoder (u->entropy,1);
      else 
        SetNSymMonoLayerCoder (u->entropy,nSteps);
    
     
       for (i = 0; i < u->nSymbol; i++)  /* data[i] always >0*/
         WriteEntropyCoder((ENTROPYCODER) (u->entropy), encoder, u->symbol[i], YES,-1,-1);
       
    }  
    else { 
      nSteps = u->imax-u->imin -1 ;
     
     if (nSteps<=0) nSteps =0;  
       
    if (nSteps == 0) 
        SetNSymMonoLayerCoder(u->entropy,1);
      else 
        SetNSymMonoLayerCoder(u->entropy,nSteps);
  
   for (i = 0; i < u->nSymbol; i++) 
         WriteEntropyCoder((ENTROPYCODER) (u->entropy), encoder, u->symbol[i], YES,-1,-1);
 
     
    }

  }
}


/*---------------------------------------------------------------------------*/

void DequantizeUniformQuant (UNIFORMQUANT u, DECODER decoder)
{   
  int i;
  int nSteps;
    
    if (u->nSymbol) Free(u->symbol);
        u->nSymbol = u->nData;
      u->symbol = IntAlloc(u->nSymbol);

     if (u->qmin == 0 && u->qmax == 0)
       { 
          for (i = 0; i < u->nData; i++)
           u->data[i]=0;
       }
     else {
    
      if (u->qmin>=0)  {/*  !(Zerocenter)  */
      nSteps = u->imax-u->imin;
     
      if (nSteps == 0) 
        SetNSymMonoLayerCoder(u->entropy,1);
      else 
        SetNSymMonoLayerCoder(u->entropy,nSteps);
    
       for (i = 0; i < u->nData; i++) { /* data[i] always >0*/
      
         u->symbol[i] =  ReadEntropyCoder((ENTROPYCODER) (u->entropy), decoder,  YES,-1,-1);

        u->data[i] = (float) (u->symbol[i]+ u->imin +0.5) *u->stepSize ;
        
       }
    }  
    else {/*  (Zerocenter)  */
      nSteps = u->imax-u->imin -1 ;
     
      if (nSteps<=0) nSteps =0;  
        if (nSteps == 0) 
       SetNSymMonoLayerCoder(u->entropy,1);
      else 
        SetNSymMonoLayerCoder(u->entropy,nSteps);
   
   
     for (i = 0; i < u->nData; i++) {
        u->symbol[i] =  ReadEntropyCoder((ENTROPYCODER) (u->entropy), decoder,  YES,-1,-1);
      
      if (u->symbol[i] == 0)  u->data[i] = 0; 
       else {
       if (u->symbol[i] > (int)((nSteps-1)/2))
          u->data[i] = - (float) ((u->symbol[i]-u->imax+0.5) *u->stepSize) ;
       else 
          u->data[i] = (float) (u->symbol[i]+0.5) *u->stepSize ;
     }
    
    } 
    }
  }
}

/*---------------------------------------------------------------------------*/

void WriteHeaderUniformQuant (UNIFORMQUANT u, ENCODER encoder)
{
    WriteIntEncoder (encoder,u->imax);
    if (!u->zeroCenter)
      WriteIntEncoder (encoder,u->imin);  
}


/*---------------------------------------------------------------------------*/
void ReadHeaderUniformQuant(UNIFORMQUANT u, DECODER decoder)
{
    u->imax = ReadIntDecoder (decoder);
    u->qmax = intToReal (u->imax, u->stepSize);
    if (u->zeroCenter) {
      u->qmin = -u->qmax;
      u->imin= -u->imax;
    } else {
      u->imin = ReadIntDecoder (decoder);
      u->qmin = intToReal (u->imin, u->stepSize);
    }
    u->qmean = 0;



}

/*---------------------------------------------------------------------------*/

