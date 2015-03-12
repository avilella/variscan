/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1998 Remi Gribonval.                                  */
/*      email  : remi.gribonval@inria.fr                                    */
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
 *	Computing and Updating a harmo
 *
 ****************************************************/
#include "lastwave.h"

#include "stft.h"

//#define HARMO_DEBUG
//#define HARMO_DEBUG1

/********************************************************/
/*   Get the best k-th partial-coeff2 among the harmo   */
/********************************************************/
float GetStftRealMaxPartialCoeff2(STFT stftReal,STFT harmo,int timeId,int freq0Id,int k)
{
  int maxTimeId,maxFreqId;
  int kFreqIdMin,kFreqIdMax;
  float max;
  
  /* Some checkings */
  CheckStftReal(stftReal);
  if(stftReal->flagUpToDate == NO)
    Errorf("GetStftRealMaxPartialCoeff2 : stft out of date");
  CheckStftHarmo(harmo);
  CheckTFContentCompat(stftReal,harmo);
  if(stftReal->windowSize     != harmo->windowSize ||  stftReal->borderType != harmo->borderType ||  stftReal->windowShape!= harmo->windowShape)
    Errorf("GetStftRealMaxPartialCoeff2 : (Weird) bad stftReal/harmo correspondance");
  
  if(!INRANGE(0,timeId,harmo->signalSize-1))
    Errorf("GetStftRealMaxPartialCoeff2 : bad timeId %d not in [0 %d]",timeId,harmo->signalSize-1);
  
  /* Determining the box */
  if(HarmoPartialBox(harmo,k,freq0Id,&kFreqIdMin,&kFreqIdMax) == NO) {
    return(0.0);
  }
  if(GetStftMax(stftReal,NO,timeId,timeId,kFreqIdMin,kFreqIdMax,&maxTimeId,&maxFreqId,&max)==NO)
    Errorf("GetStftRealMaxPartialCoeff2 ; empty range");
  return(max);
}

static void StftHarmoSetFreq(STFT harmo,STFT stftReal,int timeId)
{
  static int   dimHarmo = 0;
  static float *partialCoeff2s = NULL;
  
  float* coeff2s = NULL;
  
  int freq0Id,f;
  int k;
  int kFreq0IdMin,kFreq0IdMax;
  
  float partialCoeff2;
  
  
  
  /* Checkings */
  CheckStftHarmo(harmo);
  CheckStftReal(stftReal);
  if(stftReal->flagUpToDate == NO)
    Errorf("StftHarmoSetFreq : stftReal out of date");
  CheckStftGridCompat(harmo,stftReal);
  if(!INRANGE(0,timeId,harmo->signalSize-1))
    Errorf("StftHarmoSetFreq() : bad timeId %d not in [0 %d]",timeId,harmo->signalSize-1);
  
  /* Initializing the partialCoeff2s size and allocation */
  if(partialCoeff2s != NULL) {
    if(dimHarmo < harmo->dimHarmo) {
      Free(partialCoeff2s);
      partialCoeff2s = FloatAlloc(harmo->dimHarmo);
      dimHarmo = harmo->dimHarmo;
    }
  }
  else {
    partialCoeff2s = FloatAlloc(harmo->dimHarmo);
    dimHarmo = harmo->dimHarmo;
  }
  
  /* Getting the location of the coeff2s to update */
  GetStftData(harmo,timeId,&coeff2s,NULL);
  
  // Loop on fundamental frequencies, 
  for(freq0Id  = harmo->freqIdMin;
      freq0Id <= harmo->freqIdMax;
      freq0Id += harmo->fRate) { 
    f = (freq0Id-harmo->freqIdMin)/harmo->fRate;
    /* Re-initializing the harmonic structure coeff2 */
    coeff2s[f] = 0.0;
    
    // First, the case of the fundamental
    k = 0;
    if(HarmoPartialBox(harmo,k,freq0Id,&kFreq0IdMin,&kFreq0IdMax) == NO) 
      Errorf("F0 Box Empty");
    // Get its coeff2
    partialCoeff2     = GetStftRealMaxPartialCoeff2(stftReal,harmo,
						    timeId,freq0Id,k);
    partialCoeff2s[k] = partialCoeff2;
    coeff2s[f] 	      = partialCoeff2;
    
    // If the fundamental is nonzero, treat the upper partials
    if(partialCoeff2 != 0.0) {
      // Loop on partial numbers
      //      for(k=1; k < harmo->dimHarmo; k++) {
      for(k=1; k < 3; k++) {
	// Get the coeff2 of the partial
	partialCoeff2 = GetStftRealMaxPartialCoeff2(stftReal,harmo,
						    timeId,freq0Id,k);
	partialCoeff2s[k] = partialCoeff2;
	coeff2s[f]       += partialCoeff2;
      }
    }
    
    /* 
     * We have at our disposal the amplitudes 
     * of the partials, if we need to select
     * something to avoid octave errors
     * ...
     */
  }
}
/**************************************************************
 *
 * 		TIME LOOP
 *
 * Computes the values of a HARMO given a REAL stft at the 
 * same windowSize, given that the signal associated
 * with this REAL stft has changed between the time-indexes
 * [timeIdMin timeIdMax]. 
 *
 **************************************************************/


void UpdateStftHarmo(STFT stftHarmo,STFT stftReal,int timeIdMin,int timeIdMax)
{	
  int  timeIdMinStft,timeIdMaxStft;
  int  timeIdMinStft2,timeIdMaxStft2;
  char flagTwoLoops;
  
  int  timeId;
  
  /* Checking arguments */
  CheckStftHarmo(stftHarmo);
  CheckStftReal(stftReal);
  if(stftReal->flagUpToDate == NO)
    Errorf("UpdateStftHarmo : stftReal out of date");
  CheckStftGridCompat(stftHarmo,stftReal);
  
  ComputeUpdateLimits(stftHarmo,timeIdMin,timeIdMax,
		      &timeIdMinStft,&timeIdMaxStft,
		      &flagTwoLoops,&timeIdMinStft2,&timeIdMaxStft2);
  
  // First time loop
  for (timeId = timeIdMinStft;
       timeId <= timeIdMaxStft;
       timeId += stftHarmo->tRate) {
    /* We compute the harmo for each timeId */
    StftHarmoSetFreq(stftHarmo,stftReal,timeId);
  }
  // Second loop if needed
  if(flagTwoLoops) {
    for (timeId = timeIdMinStft2;
	 timeId <= timeIdMaxStft2;
	 timeId += stftHarmo->tRate) {
      /* We compute the harmo for each timeId */
      StftHarmoSetFreq(stftHarmo,stftReal,timeId);
    }
  }
  stftHarmo->flagUpToDate = YES;
}


/* EOF */


