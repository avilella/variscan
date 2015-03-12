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
 * 	The ENERGY Short Time Fourier Transform :
 *
 *		COMPUTATION and UPDATE
 */
/****************************************************/
#include "lastwave.h"

#include "stft.h"

/*************************************************/
/*
 *         Update of the REAL ENERGY stft data,
 *         given a COMPLEX stft.
 * two methods :
 *
 *	'Time' : we loop on time, 
 *               within a SPECIFIED TIME RANGE
 *	'Freq  : we loop on ALL frequencies
 */
/*************************************************/
static void CheckStftRealOrPhaseComplex(STFT stft,STFT stftComplex)
{
    /* Checking arguments */
    CheckStft(stft);
    if(stft->type != RealStft && stft->type != PhaseStft)
      Errorf("CheckStftRealOrPhaseComplex : stft is not real or phase");
    CheckStftComplex(stftComplex);
    if(stftComplex->flagUpToDate == NO)
      Errorf("CheckStftRealOrPhaseComplex : stftComplex is out of date");
    CheckStftGridCompat(stft,stftComplex);
}

// Returns YES iff the fast computation was possible
char ComputeWindowGG(char windowShape,unsigned long windowSize,float freqId,float *pRealGG,float *pImagGG)
{
  SIGNAL GGR,GGI;
  char s;
  long iFreqId;

  // Checking
  if(pRealGG == NULL || pImagGG == NULL)   Errorf("ComputeWindowGG : NULL output");
  if(freqId != (long) freqId)
    return(NO);

  // Case of DC/Nyquist components
  iFreqId = (long)freqId;
  if((iFreqId%GABOR_NYQUIST_FREQID)==0) {
    *pRealGG = 1.0;
    *pImagGG = 0.0;
    return(YES);
  }

  /* When things are (should be) tabulated */
  GetTabGG(windowShape,windowSize,&GGR,&GGI);
  if(!INRANGE(0,iFreqId,GABOR_NYQUIST_FREQID)) 
    Errorf("ComputeWindowGG : Bad range for freqId (%d)",(int)freqId);
  if(iFreqId > GABOR_NYQUIST_FREQID/2) {
    iFreqId = GABOR_NYQUIST_FREQID-iFreqId;
    s = -1;
  } else 
    s = 1;

  *pRealGG = GGR->Y[iFreqId];
  *pImagGG = s*GGI->Y[iFreqId];
  return(YES);
}

/***********************
 *
 * Frequency is fixed 
 *
 ***********************/

/***************************************************
 *
 * Very helpful function to set a real/phase stft 
 * at a given time 'timeId', for all frequencies.
 *
 ***************************************************/
static void StftRealOrPhaseSetFreq(STFT stft,STFT stftComplex,int timeId)
{
  unsigned long freqId,fC;
  float realGG,imagGG,coeffR,coeffI;
  float energy,phase,cosPhase,sinPhase;
  float *real,*imag,*result;
  /* Checkings */
  CheckStftRealOrPhaseComplex(stft,stftComplex);
  if(!INRANGE(0,timeId,stft->signalSize-1)) Errorf("StftRealOrPhaseSetFreq() : bad timeId %d not in [0 %d]",timeId,stft->signalSize-1);
  
  /* Getting the complex data location */
  GetStftData(stftComplex,timeId,&real,&imag);
  /* Getting the real data location */
  GetStftData(stft,timeId,&result,NULL);
  
  /* Loop on all frequencies */
  for(freqId = 0; freqId < stft->fRate*stft->nSubBands; freqId += stft->fRate) {
    fC = freqId/stftComplex->fRate;
    coeffR = real[fC];
    coeffI = imag[fC];
    if(!ComputeWindowGG(stft->windowShape,stft->windowSize,freqId,&realGG,&imagGG))
      Errorf("StftRealOrPhaseSetFreq : error with ComputeWindowGG");
    switch(stft->type) {
    case RealStft :
      ComputeRealPhaseEnergy(coeffR,coeffI,realGG,imagGG,NULL,NULL,NULL,&energy);
      result[freqId/stft->fRate] 	= energy;
      break;
    case PhaseStft :
      ComputeRealPhaseEnergy(coeffR,coeffI,realGG,imagGG,&phase,&cosPhase,&sinPhase,NULL);
      result[freqId/stft->fRate] 	= phase;
      break;
    }
  }
}


/***********************/
/*
 *   Time is fixed 
 */
/***********************/

/****************************************************
 *
 * Very helpful function to set a real stft
 * at a given freq 'freqId', for all time.
 *
 ****************************************************/

static void StftRealOrPhaseSetTime(STFT stft,STFT stftComplex,int freqId)
{
  unsigned long timeId, fC;
  float realGG,imagGG,coeffR,coeffI;
  float energy,phase,cosPhase,sinPhase;
  float *real,*imag,*result;

  /* Checking argument */
  CheckStftRealOrPhaseComplex(stft,stftComplex);

  fC = freqId/stftComplex->fRate;

  /* Loop on all times */
  for (timeId = 0; timeId < stft->tRate*stft->nFrames; timeId += stft->tRate) {
    /* Get the complex data */
    GetStftData(stftComplex,timeId,&real,&imag);
    coeffR = real[fC];
    coeffI = imag[fC];
    if(!ComputeWindowGG(stft->windowShape,stft->windowSize,freqId,&realGG,&imagGG))
      Errorf("StftRealOrPhaseSetTime : error with ComputeWindowGG");
    /* Get the real data location */
    GetStftData(stft,timeId,&result,NULL);
    switch(stft->type) {
    case RealStft :
      ComputeRealPhaseEnergy(coeffR,coeffI,realGG,imagGG,NULL,NULL,NULL,&energy);
      result[freqId/stft->fRate] 	= energy;
      break;
    case PhaseStft :
      ComputeRealPhaseEnergy(coeffR,coeffI,realGG,imagGG,&phase,&cosPhase,&sinPhase,NULL);
      if(phase < 0.0 || phase >= 1.0) {
	Warningf("StftRealSetTime : phase %g",phase);
	phase      = phase-((int) phase) +3;
	phase      = phase-((int) phase);
      }
      result[freqId/stft->fRate] 	= phase;
      break;
    }
  }
} 



/**************************************************************
 *
 * 		TIME LOOP
 *
 * Computes the values of the ENERGY stft associated with 
 * a given COMPLEX stft , given that the signal associated
 * with this complex stft has changed between the time-indexes
 * [timeIdMin timeIdMax]. 
 *
 **************************************************************/


void UpdateStftRealOrPhase(STFT stft,STFT stftComplex,int timeIdMin,int timeIdMax)
{    
  int timeIdMinStft,timeIdMaxStft;
  int timeIdMinStft2,timeIdMaxStft2;
  int timeId;
  char flagTwoLoops;

  /* Checking arguments */
  CheckStftRealOrPhaseComplex(stft,stftComplex);

  ComputeUpdateLimits(stft,timeIdMin,timeIdMax,
		      &timeIdMinStft,&timeIdMaxStft,
		      &flagTwoLoops,&timeIdMinStft2,&timeIdMaxStft2);
    
  // First time Loop 
  for (timeId = timeIdMinStft; 
       timeId <= timeIdMaxStft; 
       timeId += stft->tRate) {
    /* We compute the real stftgrams only on the real grid */
    StftRealOrPhaseSetFreq(stft,stftComplex,timeId);
  }    
  // Second loop if needed
  if(flagTwoLoops) {
    for (timeId = timeIdMinStft2; 
	 timeId <= timeIdMaxStft2; 
	 timeId += stft->tRate) {
      StftRealOrPhaseSetFreq(stft,stftComplex,timeId);
    }
  }
  // Second time loop if needed
  stft->flagUpToDate = YES;
}


/* EOF */

