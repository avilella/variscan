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



/*****************************************************/
/*
 *	Functions to get a stft BEST LOCATION
 */
/****************************************************/
#include "lastwave.h"

#include "stft.h"

/****************************************************/
/*
 * Utilities to deal with the time-frequency ranges
 */
/****************************************************/

/* 
 * We provide a range [..] and we return the larger range [..] that is
 * included in it and whose extremities are on the grid defined by 'rate'
 */
void QuantizeRangeLarge(float min,float max,int rate,
			int *pMin,int *pMax)
{
    /* Min */
    *pMin = (((int)floor(min))/rate)*rate;
    if(*pMin < min)
	*pMin += rate;
    /* Max */
    *pMax = (((int)ceil(max))/rate)*rate;
    if(*pMax > max)
	*pMax -= rate;
}

void C_TestQRL(char ** argv) {
  float min,max;
  int rate,qmin,qmax;

  argv = ParseArgv(argv,tFLOAT,&min,tFLOAT,&max,tINT,&rate,0);
  QuantizeRangeLarge(min,max,rate,&qmin,&qmax);
  Printf("[%g %g] ->%d-> [%d %d]\n",min,max,rate,qmin,qmax);
}

/* DESCRIPTION : 
 *   Locates the maximum of a STFT over a time-frequency search domain [timeIdMin timeIdMax]x[freqIdMin freqIdMax].
 *   If flagCausal==YES the search domain is further restricted to timeId in [firstp lastp]
 *
 *   If the search range is empty, we return NO, set *pMax==0.0
 *     and the content of *pMaxTimeId,*pMaxFreqId is unspecified
 *   If the search range is non empty but the maximum is zero, we return YES, set *pMax==0.0
 *     and the content of *pMaxTimeId,*pMaxFreqId is unspecified
 *   In any other case we return YES
 *    and (*pMaxTimeId,*pMaxFreqId) is the time-frequency location (in 'sample coordinates') of the maximum
 *    and *pMax is the value at the maximum 
 *
 *  HINTS : the stft MUST be up to date
 */
char GetStftMax(const STFT stft,char flagCausal,
		int timeIdMin,int timeIdMax,
		int freqIdMin,int freqIdMax, 
		int *pMaxTimeId,int *pMaxFreqId,float *pMax)
{	
  /* The quantized ranges */
  int thisFreqIdMin,thisFreqIdMax;
  int thisTimeIdMin,thisTimeIdMax;
  
  int timeId, freqId;
  int i;
  
  float *re,*im;
  float *coeff2s;
  float norm2,coeff2;
  
  int   maxTimeId,maxFreqId;
  float max;
  
  /* Checking arguments  */
  CheckStft(stft);
  if(stft->flagUpToDate == NO)
    Errorf("GetStftMax : stft is out of date [%d]",stft->windowSize);
  if(pMaxTimeId == NULL || pMaxFreqId == NULL || pMax == NULL)
    Errorf("GetStftMax : NULL output");
  
  /* Determining the quantized range [ .. ] of timeId/freqId */
  
  /* Time range */
  if(flagCausal) {
    thisTimeIdMin = MAX(stft->firstp,timeIdMin);
    thisTimeIdMax = MIN(stft->lastp,timeIdMax);
  }
  else {
    thisTimeIdMin = timeIdMin;
    thisTimeIdMax = timeIdMax;
  }
  QuantizeRangeLarge(thisTimeIdMin,thisTimeIdMax,stft->tRate,
		     &thisTimeIdMin,&thisTimeIdMax);
  thisTimeIdMin = MAX(thisTimeIdMin,0);
  thisTimeIdMax = MIN(thisTimeIdMax,(stft->nFrames*stft->tRate)-stft->tRate);

  // Freq range
  QuantizeRangeLarge(MAX(stft->freqIdMin,freqIdMin),
		     MIN(stft->freqIdMax,freqIdMax),
		     stft->fRate,&thisFreqIdMin,&thisFreqIdMax);
  thisFreqIdMin = MAX(thisFreqIdMin,0);
  thisFreqIdMax = MIN(thisFreqIdMax,stft->fRate*(stft->nSubBands-1));
  
  /* Initializing */
  maxTimeId = 0;
  maxFreqId = 0;
  max       = -1.0;
  
  /* Loop on time (real grid) */
  for(timeId = thisTimeIdMin;
      timeId <= thisTimeIdMax; 
      timeId += stft->tRate) {
    /* DEBUG : Checking */
    if(!INRANGE(timeIdMin,timeId,timeIdMax))
      Errorf("GetStftMax : (Weird) timeId range error");
    /* Get the corresponding data */
    switch(stft->type) {
    case ComplexStft :
      GetStftData(stft,timeId,&re,&im);
      break;
    case RealStft :
    case PhaseStft :
    case HighResStft :
    case HarmoStft :
      GetStftData(stft,timeId,&coeff2s,NULL);
      break;
    default :
      Errorf("GetStftMax : (Weird) unknown type %d",stft->type);
    }
    /* Loop on the frequency (real grid) */
    for (freqId = thisFreqIdMin;
	 freqId <= thisFreqIdMax; 
	 freqId += stft->fRate) {
      /* DEBUG : Checking */
      if(!INRANGE(freqIdMin,freqId,freqIdMax))
	Errorf("GetStftMax : (Weird) freqId range error");
      /* Updating the maximum location if necessary */
      i = freqId/stft->fRate;
      /* DEBUG : Double Checking !!!! */
      if(!INRANGE(0,i,stft->nSubBands-1))
	Errorf("GetStftMax : (Weird) bad i %d");
      
      switch(stft->type){
      case ComplexStft :
	norm2 = re[i]*re[i]+im[i]*im[i];
	if (norm2 > max)  {
	  maxTimeId	= timeId;
	  maxFreqId	= freqId;
	  max	        = norm2;
	}
	break;
      case RealStft :
      case PhaseStft :
      case HighResStft :
	coeff2 = coeff2s[i];
	if (coeff2 > max)  {
	  maxTimeId	= timeId;
	  maxFreqId	= freqId;
	  max	        = coeff2;
	}
	break;
      case HarmoStft :
	/* DEBUG : Checking */
	if(!INRANGE(stft->freqIdMin,freqId,stft->freqIdMax))
	  Errorf("GetStftMax : (Weird) freqId range error");
	coeff2 = coeff2s[i-stft->freqIdMin/stft->fRate];
	if (coeff2 > max)  {
	  maxTimeId	= timeId;
	  maxFreqId	= freqId;
	  max	        = coeff2;
	}
	break;
      default :
	Errorf("GetStftMax : (Weird) unknown type %d",stft->type);
      }
    }
  }
  
  /* Set the max location */
  *pMaxTimeId 	= maxTimeId;
  *pMaxFreqId 	= maxFreqId;
  
  if(max < 0.0 && max != -1.0)
    Errorf("GetStftMax : (Weird Error) would return %g !",max);
  if(max == -1.0) {
    *pMax = 0.0;
    return(NO);
  } else {
    *pMax = max;
    return(YES);
  }
}



/******************************/
/* The corresponding commands */
/******************************/

/* REMI : routine permettant de traiter une stftreal ou complex */
void C_GetStftMax(char **argv)
{
  STFT stft;
  char opt;
  char flagCausal = NO;
  /* Ranges to specify */
  float timeMin,timeMax,freqMin,freqMax;
  int timeIdMin,timeIdMax;
  int freqIdMin,freqIdMax;
  
  int   maxTimeId;
  int   maxFreqId;
  float	max;
 
  LISTV lv;

  /* Reading stft */
  argv = ParseArgv(argv,tSTFT_,NULL,&stft,-1);
  if(stft == NULL)
    stft = GetStftCur();
  
  /* Default range */
  timeIdMin = 0;
  timeIdMax = stft->signalSize-1;
  freqIdMin = 0;
  freqIdMax = GABOR_NYQUIST_FREQID;
  
  /* Reading options */
  while( (opt = ParseOption(&argv)) ) {
    switch(opt) {
    case 'c' :
      flagCausal = YES;
      break;
    case 't' :
      argv = ParseArgv(argv,tFLOAT,&timeMin,tFLOAT,&timeMax,-1);
      timeIdMin = ceil(Time2TimeId(stft,timeMin));
      timeIdMax = floor(Time2TimeId(stft,timeMax));
      break;
    case 'f' :
      argv = ParseArgv(argv,tFLOAT,&freqMin,tFLOAT,&freqMax,-1);
      freqIdMin = ceil(Freq2FreqId(stft,freqMin));
      freqIdMax = floor(Freq2FreqId(stft,freqMax));
      break;
    case 'T' :
      argv = ParseArgv(argv,tINT,&timeIdMin,tINT,&timeIdMax,-1);
      break;
    case 'F' :
      argv = ParseArgv(argv,tINT,&freqIdMin,tINT,&freqIdMax,-1);
      break;
    default : 
      ErrorOption(opt);
    }
  }
  NoMoreArgs(argv);
  
  // Get the best location
  GetStftMax(stft,flagCausal,timeIdMin,timeIdMax,freqIdMin,freqIdMax,
	     &maxTimeId,&maxFreqId,&max);
  
  lv = TNewListv();
  AppendFloat2Listv(lv,max);
  if (max != 0.0 && max != -1.0) {
    AppendInt2Listv(lv,maxTimeId);
    AppendInt2Listv(lv,maxFreqId);
  }
  SetResultValue(lv);
}

/* EOF */

