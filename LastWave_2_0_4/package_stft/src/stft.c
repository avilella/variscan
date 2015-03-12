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
 * 	The Short Time Fourier Transform :
 *		Parsing of variables
 *		Allocation and Desallocations of structures
 *
 */
/****************************************************/
#include "lastwave.h"

#include "stft.h"

/********************************/
/*
 *	STFT variables
 */
/********************************/
char *stftType = "&stft";
extern TypeStruct tsStft;


/*
 * Answers to the different print messages
 */
 
void ShortPrintStft(STFT stft)
{
  //  if (stft->name == NULL)
    Printf("<&stft[%d,%d];%p>\n",stft->nFrames,stft->nSubBands,stft);
    //  else
    //    Printf("<'%s';&stft[%d,%d];%p>\n",stft->name,stft->nFrames,stft->nSubBands,stft);
}

char *ToStrStft(STFT stft, char flagShort)
{
  static char str[30];
  
  sprintf(str,"<&stft;%p>",stft);
  return(str);
}


void PrintInfoStft(STFT stft)
{
  PrintStft(stft,NO);
}

/*
 * NumExtraction
 *
 * Vertical slices (<timeId>stft, ...)
 * Horizontal slices (<freqId>stft ...)
 */
static char *numdoc = "The syntax <timeId>stft corresponds to vertical slices of the &stft. The syntax .<freqId>stft addresses horizontal slices. A slice is either a &listv {<real> <imag>} if the type of the &stft is complex, or just a &signal <real> which may contain either the energy, the phase, or the highres correlation.";

static void *NumExtractStft(STFT stft,void **arg)
{
  long n;
  char flagDot;
  
  int timeId,freqId;
  SIGNAL signalR,signalI;
  float *real,*imag;
  LISTV lv = NULL;

  /* doc */
  if (stft == NULL) return(numdoc);

  n = ARG_NE_GetN(arg);
  flagDot = ARG_NE_GetFlagDot(arg);

  /* Horizontal slice at as fixed frequency */
  if(flagDot) {
    CheckStftNotEmpty(stft);
    freqId = n;
    /* Checking */
    if(freqId%stft->fRate != 0 || 
       !INRANGE(0,freqId/stft->fRate,stft->nSubBands-1) ||
       !INRANGE(stft->freqIdMin,freqId,stft->freqIdMax)) {
	SetErrorf("Bad freqId %d : rate %d length %d min %d max %d",freqId,stft->fRate,stft->nSubBands,stft->freqIdMin,stft->freqIdMax);
	return(NULL);
    }
      /* Checking */
    switch(stft->type) {
    case ComplexStft:
      signalR = TNewSignal();
      signalI = TNewSignal();
      SizeSignal(signalR,stft->nFrames,YSIG);
      SizeSignal(signalI,stft->nFrames,YSIG);
      signalR->x0      =signalI->x0      =stft->x0;
      signalR->dx      =signalI->dx      = TimeId2Time(stft,stft->tRate);
      signalR->firstp  =signalI->firstp  = stft->firstp;
      signalR->lastp   =signalI->lastp   = stft->lastp;
      /* Getting the data */
      for(timeId = 0;
	  timeId < stft->tRate*stft->nFrames;
	  timeId += stft->tRate) {
	GetStftData(stft,timeId,&real,&imag);
	signalR->Y[timeId/stft->tRate] = real[(freqId-stft->freqIdMin)/stft->fRate];
	signalI->Y[timeId/stft->tRate] = imag[(freqId-stft->freqIdMin)/stft->fRate];
      }
      lv = TNewListv();
      AppendValue2Listv(lv,(VALUE)signalR);
      AppendValue2Listv(lv,(VALUE)signalI);
      ARG_NE_SetResValue(arg,lv);
      return(listvType);
    case RealStft: 
    case PhaseStft: 
    case HighResStft:
    case HarmoStft:
      signalR = TNewSignal();
      SizeSignal(signalR,stft->nFrames,YSIG);
      signalR->x0 = stft->x0;
      signalR->dx = TimeId2Time(stft,stft->tRate);
      signalR->firstp  = stft->firstp;
      signalR->lastp   = stft->lastp;
      /* Getting the data */
      for(timeId = 0; 
	  timeId < stft->tRate*stft->nFrames; 
	  timeId += stft->tRate) {
	GetStftData(stft,timeId,&real,NULL);
	signalR->Y[timeId/stft->tRate] = real[(freqId-stft->freqIdMin)/stft->fRate];
      }
      ARG_NE_SetResValue(arg,signalR);
      return(signalType);
    }
  } else {
    /* Vertical slice at as fixed time */
    CheckStftNotEmpty(stft);
    timeId = n;
    switch(stft->type) {
    case ComplexStft:
      /* Getting the data */
      GetStftData(stft,timeId,&real,&imag);
      /* Allocation */
      signalR = TNewSignal();
      signalI = TNewSignal();
      SizeSignal(signalR,stft->nSubBands,YSIG);
      SizeSignal(signalI,stft->nSubBands,YSIG);
      signalR->x0 = signalI->x0 = 0.0;
      signalR->dx = signalI->dx = FreqId2Freq(stft,stft->fRate);
      signalR->firstp  = signalI->firstp  = 0;
      signalR->lastp   = signalI->lastp   = stft->nSubBands-1;
      /* Copying */
      memcpy(signalR->Y,real,stft->nSubBands*sizeof(float));
      memcpy(signalI->Y,imag,stft->nSubBands*sizeof(float));
      lv = TNewListv();
      AppendValue2Listv(lv,(VALUE)signalR);
      AppendValue2Listv(lv,(VALUE)signalI);
      ARG_NE_SetResValue(arg,lv);
      return(listvType);
    case RealStft:
    case PhaseStft:
    case HighResStft:
    case HarmoStft:
      /* Getting the data */
      GetStftData(stft,timeId,&real,NULL);
      /* Allocation */
      signalR = TNewSignal();
      SizeSignal(signalR,((stft->freqIdMax-stft->freqIdMin)/stft->fRate)+1,YSIG);
      signalR->x0      = FreqId2Freq(stft,stft->freqIdMin);
      signalR->dx      = FreqId2Freq(stft,stft->fRate);
      signalR->firstp  = 0;
      signalR->lastp   = stft->nSubBands-1;
      /* Copying */
      memcpy(signalR->Y,real,(((stft->freqIdMax-stft->freqIdMin)/stft->fRate)+1)*sizeof(float));
      ARG_NE_SetResValue(arg,signalR);
      return(signalType);
    }
  }
}



STFT TNewStft(void)
{
  STFT stft;
  
  stft = NewStft();
  TempValue(stft);
  return(stft);
}

/*
 * Get the current stft
 * (generate an error if there is none)
 */

STFT GetStftCur(void)
{
  STFT stft;

  if(!ParseTypedValLevel_(levelCur,"objCur",NULL,(VALUE *)&stft,stftType)) 
    Errorf1("");
  
  AddRefValue(stft);
  TempValue(stft);
  
  return(stft);
}


static void CheckSameStftStruct(const STFT stft1,const STFT stft2)
{
  if(stft1==NULL||stft2==NULL)
    Errorf("CheckSameStftStruct : NULL input");
  CheckTFContentCompat(stft1,stft2);
  if(stft1->tRate != stft2->tRate ||
     stft1->nFrames != stft2->nFrames ||
     stft1->fRate != stft2->fRate ||
     stft1->nSubBands != stft2->nSubBands ||
     stft1->borderType != stft2->borderType ||
     stft1->firstp != stft2->firstp ||
     stft1->lastp != stft2->lastp ||
     stft1->freqIdMin != stft2->freqIdMin ||
     stft1->freqIdMax != stft2->freqIdMax ||
     stft1->windowShape != stft2->windowShape ||
     stft1->windowSize != stft2->windowSize ||
     stft1->dimHarmo != stft2->dimHarmo ||
     stft1->iFMO != stft2->iFMO ||
     stft1->type != stft2->type) {
    PrintStft(stft1,NO);
    PrintStft(stft2,NO);
    Errorf("CheckSameStftStruct : struct is different!");
  }
}

char* StftOper2Name(char oper)
{
  switch(oper) {
  case STFT_ADD:
    return("+");
  case STFT_SUBSTRACT:
    return("-");
  case STFT_MULTIPLY:
    return("*");
  case STFT_DIVIDE:
    return("/");
  case STFT_CONJUGATE:
    return("conjugate");
  case STFT_LN:
    return("ln");
  case STFT_LOG:
    return("log");
  case STFT_LOG2:
    return("log2");
  default :
    Errorf("StftOper2Name : (Weird) unknown oper %s",oper);
    return("");
  }
}

char Name2StftOper(char *name)
{
  if(name==NULL)
    Errorf("Name2StftOper : NULL input");
  if(!strcmp(name,"+"))
    return(STFT_ADD);
  if(!strcmp(name,"-"))
    return(STFT_SUBSTRACT);
  if(!strcmp(name,"*"))
    return(STFT_MULTIPLY);
  if(!strcmp(name,"/"))
    return(STFT_DIVIDE);
  if(!strcmp(name,"conjugate"))
    return(STFT_CONJUGATE);
  if(!strcmp(name,"ln"))
    return(STFT_LN);
  if(!strcmp(name,"log"))
    return(STFT_LOG);
  if(!strcmp(name,"log2"))
    return(STFT_LOG2);
  Errorf("Name2StftOper : unknown operation '%s'",name);
  return(-1);
}


// Given a stft and a time range [timeIdMin,timeIdMax] where a
// signal has changed, computes the time range where to update the stft
void  ComputeUpdateLimits(STFT stft,int timeIdMin,int timeIdMax,
			  int *pTimeIdMin,int *pTimeIdMax,
			  char *pFlagTwoLoops,int *pTimeIdMin1,int *pTimeIdMax1)
{
  int shiftMin,shiftMax;
  // Check ...
  CheckStftNotEmpty(stft);
  switch(stft->borderType) {
    // The simple border effects
  case BorderPad0 :
  case BorderPad : 
  case BorderMirror :
    if((!INRANGE(0,timeIdMin,timeIdMax)) || 
       (!INRANGE(timeIdMin,timeIdMax,stft->signalSize-1)))
      Errorf("ComputeUpdateLimits : [%d %d] not in [0 %d]",
	     timeIdMin,timeIdMax,stft->signalSize-1);
    break;
  case BorderPeriodic :
    if((!INRANGE(0,timeIdMin,MIN(timeIdMax,stft->signalSize-1))) || 
       (!INRANGE(timeIdMin,timeIdMax,2*stft->signalSize-1)))
      Errorf("ComputeUpdateLimits : [%d %d] not in [0 %d]",
	     timeIdMin,timeIdMax,stft->signalSize-1);
    break;
  default : 
    Errorf("ComputeUpdateLimits : border '%s' not treated yet",BorderType2Name(stft->borderType));
  }

  // By default there is only one loop
  *pFlagTwoLoops = NO;

  // The time support of the window centered at timeId is
  // [timeId+shiftMin,timeId+shiftMax]
  ComputeWindowSupport(stft->windowSize,stft->windowShape,0.0,
		       &shiftMin,&shiftMax);
  // The inner product has changed iff this support intersects 
  // the portion of the signal that has changed. This depends on the border type
  switch(stft->borderType) {
    // The simple border effects
  case BorderPad0 :
  case BorderPad : 
  case BorderMirror :
    *pTimeIdMin     = timeIdMin-shiftMax;
    *pTimeIdMax     = timeIdMax-shiftMin;
    break;
  case BorderPeriodic :
    // Case when all changes are in the first 'copy' of the signal
    if(timeIdMax <= stft->signalSize-1) {
      *pTimeIdMin      = timeIdMin-shiftMax;
      *pTimeIdMax      = timeIdMax-shiftMin;
      break;
    }
    // Otherwise : the first part that need to be updated
    *pFlagTwoLoops = YES;
    *pTimeIdMin    = 0-shiftMax;
    *pTimeIdMax    = (timeIdMax%stft->signalSize)-shiftMin;
    *pTimeIdMin1   = timeIdMin-shiftMax;
    *pTimeIdMax1   = stft->signalSize-1-shiftMin;
    break;
  default :
      Errorf("ComputeUpdateLimits : border type %s not treated yet",BorderType2Name(stft->borderType));
  }

  /* We consider only the locations on the  time-freq grid */
  QuantizeRangeLarge(*pTimeIdMin,*pTimeIdMax,stft->tRate,
		     pTimeIdMin,pTimeIdMax);	
  *pTimeIdMin     = MAX(0,*pTimeIdMin);
  *pTimeIdMax     = MIN((stft->nFrames*stft->tRate)-stft->tRate,*pTimeIdMax);

  if(*pFlagTwoLoops) {
    QuantizeRangeLarge(*pTimeIdMin1,*pTimeIdMax1,stft->tRate,
		       pTimeIdMin1,pTimeIdMax1);	
    *pTimeIdMin1     = MAX(0,*pTimeIdMin1);
    *pTimeIdMax1     = MIN((stft->nFrames*stft->tRate)-stft->tRate,*pTimeIdMax1);
  }
}


void AddStft(STFT stftResult,STFT stftAdded,unsigned long timeIdMin,unsigned long timeIdMax)
{
  int timeIdMinStft,timeIdMaxStft;
  int timeIdMinStft2,timeIdMaxStft2;
  char flagTwoLoops;

  unsigned long timeId;
  unsigned long f;
  float *real1,*real2,*imag1=NULL,*imag2;

  CheckStftNotEmpty(stftResult);
  CheckStftNotEmpty(stftAdded);
  CheckSameStftStruct(stftResult,stftAdded);
  ComputeUpdateLimits(stftResult,timeIdMin,timeIdMax,
		      &timeIdMinStft,&timeIdMaxStft,
		      &flagTwoLoops,&timeIdMinStft2,&timeIdMaxStft2);
  // First time Loop 
  for (timeId = timeIdMinStft; 
       timeId <= timeIdMaxStft; 
       timeId += stftResult->tRate) {
    if(stftResult->type==ComplexStft) {
      GetStftData(stftResult,timeId,&real1,&imag1);
      GetStftData(stftAdded,timeId,&real2,&imag2);
    } else {
      GetStftData(stftResult,timeId,&real1,NULL);
      GetStftData(stftAdded,timeId,&real2,NULL);
    }
    for(f = 0; f < stftResult->nSubBands; f++) real1[f]+=real2[f];
    if(imag1) {
      for(f = 0; f < stftResult->nSubBands; f++) imag1[f]+=imag2[f];
    }
  }
  // Second loop if needed
  if(flagTwoLoops) Errorf("????");
} 

void ZeroStft(STFT stft,unsigned long timeIdMin,unsigned long timeIdMax)
{
  int timeIdMinStft,timeIdMaxStft;
  int timeIdMinStft2,timeIdMaxStft2;
  char flagTwoLoops;

  unsigned long timeId;
  unsigned long f;
  float *real1,*imag1=NULL;

  CheckStftNotEmpty(stft);
  ComputeUpdateLimits(stft,timeIdMin,timeIdMax,
		      &timeIdMinStft,&timeIdMaxStft,
		      &flagTwoLoops,&timeIdMinStft2,&timeIdMaxStft2);
  // First time Loop 
  for (timeId = timeIdMinStft; timeId <= timeIdMaxStft; timeId += stft->tRate) {
    if(stft->type==ComplexStft) {
      GetStftData(stft,timeId,&real1,&imag1);
    } else {
      GetStftData(stft,timeId,&real1,NULL);
    }
    for(f = 0; f < stft->nSubBands; f++) real1[f]=0.0;
    if(imag1!=NULL) {
      for(f = 0; f < stft->nSubBands; f++) imag1[f]=0.0;
    }
  }
  // Second loop if needed
  if(flagTwoLoops) Errorf("????");
} 

void StftOper(const STFT stftIn1,const STFT stftIn2,STFT stftOut,char oper)
{
  int i;
  float re1,im1,re2,im2,norm2;
  static double log2 = -1;

  if(log2 == -1) log2 = log(2);

  CheckStftNotEmpty(stftIn1);
  if(stftIn2 != NULL) {
    CheckStftNotEmpty(stftIn2);
    CheckSameStftStruct(stftIn1,stftIn2);
  }
  CheckStft(stftOut);

  if(!StftOperIsOK(oper))
    Errorf("StftOper : unknown arithmetic operation");

  // If the output is different from the inputs we size it
  if(stftOut != stftIn1 && stftOut != stftIn2) {
    CopyFieldsTFContent(stftIn1,stftOut);
    // TODO : deal with time/freqRedund ... should be computed from tRate ?
    SizeStft(stftOut,stftIn1->windowShape,stftIn1->windowSize,
	     STFT_DEFAULT_TIMEREDUND,STFT_DEFAULT_FREQREDUND,
	     stftIn1->borderType,stftIn1->type,
	     stftIn1->freqIdMin,stftIn1->freqIdMax,
	     stftIn1->iFMO,stftIn1->dimHarmo);
  }
  // DEBUG
  CheckSameStftStruct(stftIn1,stftOut);

  switch(stftIn1->type) {
  case ComplexStft:
    switch(oper) {
    case STFT_ADD: //Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	stftOut->real->Y[i] = stftIn1->real->Y[i]+stftIn2->real->Y[i];
	stftOut->imag->Y[i] = stftIn1->imag->Y[i]+stftIn2->imag->Y[i];
      }
      break;
    case STFT_SUBSTRACT: //Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	stftOut->real->Y[i] = stftIn1->real->Y[i]-stftIn2->real->Y[i];
	stftOut->imag->Y[i] = stftIn1->imag->Y[i]-stftIn2->imag->Y[i];
      }
      break;
    case STFT_MULTIPLY: //Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	re2 = stftIn2->real->Y[i];
	im1 = stftIn1->imag->Y[i];
	im2 = stftIn2->imag->Y[i];
	stftOut->real->Y[i] = re1*re2-im1*im2;
	stftOut->imag->Y[i] = re1*im2+im1*re2;
      }
      break;
    case STFT_DIVIDE: //Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	re2 = stftIn2->real->Y[i];
	im1 = stftIn1->imag->Y[i];
	im2 = stftIn2->imag->Y[i];
	norm2 = re2*re2+im2*im2;
	if(norm2 == 0) {
	  Warningf("StftOper : division by zero");
	  stftOut->real->Y[i] = FLT_MAX/2;
	  stftOut->imag->Y[i] = FLT_MAX/2;
	}
	else {
	  stftOut->real->Y[i] = (re1*re2+im1*im2)/norm2;
	  stftOut->imag->Y[i] = (-re1*im2+im1*re2)/norm2;
	}
      }
      break;
    case STFT_CONJUGATE:
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	stftOut->real->Y[i] = stftIn1->real->Y[i];
	stftOut->imag->Y[i] = -stftIn1->imag->Y[i];
      }
      break;
    case STFT_LN: //Natural logarithm
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	im1 = stftIn1->imag->Y[i];
	norm2 = re1*re1+im1*im1;
	if(norm2 == 0.0) {
	  Warningf("StftOper : log of zero");
	  stftOut->real->Y[i]  = -FLT_MAX/2;
	  stftOut->imag->Y[i] = 0.0;
	}
	else {
	  stftOut->real->Y[i] = 0.5*log(norm2);
	  stftOut->imag->Y[i] = atan2(im1,re1)/(2*M_PI);
	}
      }
      break;
    case STFT_LOG: //Base 10
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	im1 = stftIn1->imag->Y[i];
	norm2 = re1*re1+im1*im1;
	if(norm2 == 0.0) {
	  Warningf("StftOper : log10 of zero");
	  stftOut->real->Y[i]  = -FLT_MAX/2;
	  stftOut->imag->Y[i] = 0.0;
	}
	else {
	  stftOut->real->Y[i] = 0.5*log10(norm2);
	  stftOut->imag->Y[i] = atan2(im1,re1)/(2*M_PI);
	}
      }
      break;
    case STFT_LOG2: //Base 2
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	im1 = stftIn1->imag->Y[i];
	norm2 = re1*re1+im1*im1;
	if(norm2 == 0.0) {
	  Warningf("StftOper : log2 of zero");
	  stftOut->real->Y[i]  = -FLT_MAX/2;
	  stftOut->imag->Y[i] = 0.0;
	}
	else {
	  stftOut->real->Y[i] = 0.5*log(norm2)/log2;
	  stftOut->imag->Y[i] = atan2(im1,re1)/(2*M_PI);
	}
      }
      break;
    default :
      Errorf("StftOper : operation '%s' not treated fro '%s' stft",StftOper2Name(oper),StftType2Name(stftIn1->type));
      break;
    }
    break;
  case RealStft:
  case PhaseStft:
  case HighResStft:
  case HarmoStft:
    switch(oper) {
    case STFT_ADD://Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	stftOut->real->Y[i] = stftIn1->real->Y[i]+stftIn2->real->Y[i];
      }
      break;
    case STFT_SUBSTRACT://Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	stftOut->real->Y[i] = stftIn1->real->Y[i]-stftIn2->real->Y[i];
      }
      break;
    case STFT_MULTIPLY://Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	stftOut->real->Y[i] = stftIn1->real->Y[i]*stftIn2->real->Y[i];
      }
      break;
    case STFT_DIVIDE://Should work even if stftOut==stftIn1 or stftIn2
      CheckStftNotEmpty(stftIn2);
      for(i = 0; i < stftIn1->real->size; i++) {
	if(stftIn2->real->Y[i] == 0) {
	  Warningf("StftOper : division by zero");
	  stftOut->real->Y[i] = FLT_MAX/2;
	}
	else {
	  stftOut->real->Y[i] = stftIn1->real->Y[i]/stftIn2->real->Y[i];
	}
      }
      break;
    case STFT_LN: //Natural logarithm
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	if(re1 <= 0.0) {
	  Warningf("StftOper : log of %g",re1);
	  stftOut->real->Y[i]  = -FLT_MAX/2;
	}
	else {
	  stftOut->real->Y[i] = log(re1);
	}
      }
      break;
    case STFT_LOG: //Base 10 logarithm
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	if(re1 <= 0.0) {
	  Warningf("StftOper : log10 of %g",re1);
	  stftOut->real->Y[i]  = -FLT_MAX/2;
	}
	else {
	  stftOut->real->Y[i] = log10(re1);
	}
      }
      break;
    case STFT_LOG2: //Base 2 logarithm
      if(stftIn2 != NULL)
	Errorf("StftOper : stftIn2 should be NULL for '%s' operation",StftOper2Name(oper));
      for(i = 0; i < stftIn1->real->size; i++) {
	re1 = stftIn1->real->Y[i];
	if(re1 <= 0.0) {
	  Warningf("StftOper : log2 of %g",re1);
	  stftOut->real->Y[i]  = -FLT_MAX/2;
	}
	else {
	  stftOut->real->Y[i] = log(re1)/log(2);
	}
      }
      break;
    default :
      Errorf("StftOper : operation '%s' not treated for '%s' stft",StftOper2Name(oper),StftType2Name(stftIn1->type));
      break;
    }
    break;
  default : 
    Errorf("StftOper : stft type '%s' not treated yet",StftType2Name(stftIn1->type));
  }
  stftOut->flagUpToDate = YES;
}


/* COMMAND */
void C_Stft(char **argv)
{
  STFT stft = NULL;
  STFT stft1 = NULL;
  STFT stft2 = NULL;
  STREAM stream;
  char *action;
  char opt;
  
  char flagShortPrint = NO;
  char flagHeader = YES;
  char *filename;
  
  // To get tabulated data from the package
  SIGNAL signal,signal1,real,imag;
  char *windowShapeName;
  unsigned long windowSize;
  LISTV listv = NULL;

  /* Parsing command */
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if(!strcmp(action,"window")) {
    argv = ParseArgv(argv,tSTR,&windowShapeName,tINT,&windowSize,0);
    GetTabWindow(Name2WindowShape(windowShapeName),windowSize,&real);
    signal = TNewSignal();
    CopySig(real,signal);
    SetResultValue(signal);
    return;
  }
  if(!strcmp(action,"exp")) {
    NoMoreArgs(argv);
    GetTabExponentials(&real,&imag);
    signal= TNewSignal();
    signal1= TNewSignal();
    CopySig(real,signal);
    CopySig(imag,signal1);
    listv = TNewListv();
    AppendValue2Listv(listv,(VALUE)signal);
    AppendValue2Listv(listv,(VALUE)signal1);
    SetResultValue(listv);
    return;
  }
  if(!strcmp(action,"gg")) {
    argv = ParseArgv(argv,tSTR,&windowShapeName,tINT,&windowSize,0);
    GetTabGG(Name2WindowShape(windowShapeName),windowSize,&real,&imag);
    signal= TNewSignal();
    signal1= TNewSignal();
    CopySig(real,signal);
    CopySig(imag,signal1);
    listv = TNewListv();
    AppendValue2Listv(listv,(VALUE)signal);
    AppendValue2Listv(listv,(VALUE)signal1);
    SetResultValue(listv);
    return;
  }

  if(!strcmp(action,"write")) {
    argv = ParseArgv(argv,tSTFT,&stft,-1);
    argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
    if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
    else filename = NULL;
    while((opt = ParseOption(&argv))) { 
      switch(opt) {
      case 'h': flagHeader = NO; break;
      default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    if (stream == NULL) WriteStftFile(stft,filename,flagHeader);
    else WriteStftStream(stft,stream,flagHeader);
    return;
  }
  
  if(!strcmp(action,"+") || !strcmp(action,"-") || 
     !strcmp(action,"*") || !strcmp(action,"/")) {
    argv = ParseArgv(argv,tSTFT,&stft,tSTFT,&stft1,tSTFT_,NULL,&stft2,0);
    // If there are only two arguments
    if (stft2 == NULL) stft2 = stft;
    StftOper(stft,stft1,stft2,Name2StftOper(action));
    return;
  }

  if(!strcmp(action,"conjugate") || !strcmp(action,"ln") ||
     !strcmp(action,"log") || !strcmp(action,"log2")) {
    argv = ParseArgv(argv,tSTFT,&stft,tSTFT_,NULL,&stft1,0);
    // If there is only one argument
    if (stft1 == NULL) stft1 = stft;
    StftOper(stft,NULL,stft1,Name2StftOper(action));
    return;
  }
  Printf("Unknown action '%s'\n",action);
  ErrorUsage();
}

/************************************/
/*
 *	Type conversions 
 */				    
/************************************/
char * StftType2Name(char stftType)
{
  if(!StftTypeIsOK(stftType))
    Errorf("StftType2Name : bad stftType %d",stftType);
  
  switch(stftType) {
  case ComplexStft: return("complex");
  case RealStft:    return("real");
  case PhaseStft:   return("phase");
  case HighResStft: return("highres");
  case HarmoStft:   return("harmo");
  default : Errorf("StftType2Name : (Weird) bad type %d",stftType);
  }
  return("");
}


char Name2StftType(char *stftType)
{
  if (!strcmp(stftType,"complex")) return(ComplexStft);
  if (!strcmp(stftType,"real")) return(RealStft);
  if (!strcmp(stftType,"phase")) return(PhaseStft);
  if (!strcmp(stftType,"highres")) return(HighResStft);
  if (!strcmp(stftType,"harmo")) return(HarmoStft);
  Errorf("Name2StftType : (Weird) bad stft name %s",stftType);
  // This should never be reached but the compiler may want to be sure the fnuction returns a value
  return(LastStftType);
}


/************************************/
/*
 *	ALLOCATIONS
 *
 */				    
/************************************/
static void _MyCheckHarmo(const STFT stft)
{
  if(!INRANGE(STFT_MIN_WINDOWSIZE,stft->windowSize,stft->signalSize))
    Errorf("CheckHarmo : bad windowSize %d not in [4 %d]",stft->windowSize,stft->signalSize);
  
  /* Checking frequency grid */
  if(stft->fRate*(stft->nSubBands-1) != GABOR_NYQUIST_FREQID)
    Errorf("CheckHarmo : bad freq grid %d * %d != %d for windowSize %d",
	   stft->fRate,stft->nSubBands-1,GABOR_NYQUIST_FREQID,stft->windowSize);
  
  /* The freq0Id-range must be on the grid */
  if(stft->freqIdMin % stft->fRate != 0 || stft->freqIdMax % stft->fRate != 0)
    Errorf("CheckHarmo : bad f0 range [%d %d] not on the grid %d",stft->freqIdMin,stft->freqIdMax,stft->fRate);
  
  if(stft->freqIdMin < stft->iFMO*stft->fRate)
    Errorf("CheckHarmo ; bad freq0IdMin %d < %d",stft->freqIdMin,stft->iFMO*stft->fRate);
  if(stft->freqIdMin <= stft->freqIdMax && stft->real == NULL)
    Errorf("CheckHarmo : NULL data for range [%d %d]",stft->freqIdMin,stft->freqIdMax);
  if(stft->freqIdMin > stft->freqIdMax && stft->real != NULL)
    Errorf("CheckHarmo : NON NULL data for range [%d %d]",stft->freqIdMin,stft->freqIdMax);
  if(stft->freqIdMax > stft->fRate*(stft->nSubBands-1))
    Errorf("CheckHarmo ; bad freq0IdMax %d > %d",stft->freqIdMax,stft->fRate*(stft->nSubBands-1));
}

void CheckStft(const STFT stft)
{
  // Checking of fields independently of one another
  if (!BorderTypeIsOK(stft->borderType))
    Errorf("CheckStft : unknown border type %d",stft->borderType); 
  if (!WindowShapeIsOK(stft->windowShape)) 
    Errorf("CheckStft : unknown windowShape %d",stft->windowShape); 
  if(stft->tRate <= 0 || stft->fRate <= 0) 
    Errorf("CheckStft : bad rates %d %d",stft->tRate,stft->fRate); 
  if(stft->tRate*stft->nFrames < stft->signalSize) 
    Errorf("CheckStft : bad time grid %d * %d < %d",stft->tRate,stft->nFrames,stft->signalSize);

  if(stft->iFMO <= 0)
    Errorf("CheckStft : bad iFMO %d for HarmoStft",stft->iFMO);
  if(stft->dimHarmo <= 0)
    Errorf("CheckStft : bad dimHarmo %d for HarmoStft",stft->dimHarmo);
  
  // Checking of coherence of fields
  /* Case of an empty stft */
  if(stft->signalSize == 0) {
    if(stft->windowSize > 0)
      Errorf("CheckStft %d %s : empty stft is at windowSize %d!",
	     stft->windowSize,StftType2Name(stft->type),stft->windowSize);
    if(stft->real != NULL || stft->imag != NULL)
      Errorf("CheckStft %d %s : non NULL data for empty stft",
	     stft->windowSize,StftType2Name(stft->type));
    if(stft->flagUpToDate)
      Errorf("CheckStft %d %s : empty stft is up to date!");
  }
  /* Case of non empty one */
  else {
    // Temporary during development
    if(stft->type == HarmoStft) {    
      _MyCheckHarmo(stft);
      return;
    }
    if(stft->windowSize <= 0)
      Errorf("CheckStft %d %s : bad windowSize %d <= 0",
	     stft->windowSize,StftType2Name(stft->type),stft->windowSize);
    /* Window size must be smaller than signal size ! */
    if(!INRANGE(2,stft->windowSize,stft->signalSize))
      Errorf("CheckStft %d %s : bad windowSize %d compared to signalSize %d",
	     stft->windowSize,StftType2Name(stft->type),stft->windowSize,stft->signalSize);
    /* TODO ; remove ??? */
    if(stft->windowSize > GABOR_MAX_FREQID)
      Errorf("CheckStft %d %s : bad windowSize %d compared to GABOR_MAX_FREQID %d",
	     stft->windowSize,StftType2Name(stft->type),stft->windowSize,GABOR_MAX_FREQID);
  }
  
  /* Checking types */
  if(!StftTypeIsOK(stft->type))
    Errorf("CheckStft %d %s : unknown  type %d",stft->windowSize,StftType2Name(stft->type),stft->type);
  
  
  if(stft->fRate*(stft->nSubBands-1) != GABOR_NYQUIST_FREQID)
    Errorf("CheckStft %d %s : bad freq grid %d * %d != %d = Nyquist",stft->windowSize,StftType2Name(stft->type),stft->fRate,stft->nSubBands-1,GABOR_NYQUIST_FREQID);
  /* Type dependent checkings */
  if(stft->signalSize != 0) {
    switch(stft->type) {
    case ComplexStft:
      if(stft->real == NULL || stft->imag == NULL)
	Errorf("CheckStft %d %s : NULL complex data",stft->windowSize,StftType2Name(stft->type));
      break;
    case RealStft:
    case PhaseStft:
    case HighResStft:
    case HarmoStft:
      if(stft->real == NULL)
	Errorf("CheckStft %d %s : NULL real data",stft->windowSize,StftType2Name(stft->type));
      if(stft->imag != NULL)
	Errorf("CheckStft %d %s : NON NULL imag data",stft->windowSize,StftType2Name(stft->type));
      break;
    default :
      Errorf("CheckStft : (Weird) unknown type %d",stft->type);
    }
  }
}

void CheckStftNotEmpty(const STFT stft)
{
  CheckStft(stft);
  if(stft->signalSize == 0)  Errorf("CheckStftNotEmpty : empty stft");
}

void CheckStftGridCompat(const STFT stftGrid,const STFT stftSubGrid)
{
  CheckTFContentCompat(stftGrid,stftSubGrid);
  if(stftGrid->windowSize     != stftSubGrid->windowSize ||
     stftGrid->borderType != stftSubGrid->borderType ||
     stftGrid->windowShape != stftSubGrid->windowShape)
    Errorf("CheckStftGridCompat : Weird error");
  if(stftGrid->tRate % stftSubGrid->tRate != 0)
    Errorf("CheckStftGridCompat : incompatible tRates %d %d",stftGrid->tRate,stftSubGrid->tRate);
  if(stftGrid->fRate % stftSubGrid->fRate != 0)
    Errorf("CheckStftGridCompat : incompatible fRates %d %d",stftGrid->fRate,stftSubGrid->fRate);
}

void CheckStftComplex(const STFT stft)
{
  CheckStftNotEmpty(stft);
  if(stft->type != ComplexStft)
    Errorf("CheckStftComplex : stft is not complex");
}

void CheckStftReal(const STFT stft)
{
  CheckStftNotEmpty(stft);
  if(stft->type != RealStft)
    Errorf("CheckStftReal : stft is not energy");
}

void CheckStftPhase(const STFT stft)
{
  CheckStftNotEmpty(stft);
  if(stft->type != PhaseStft)
    Errorf("CheckStftPhase : stft is not energy");
}

void CheckStftHighRes(const STFT stft)
{
  CheckStftNotEmpty(stft);
  if(stft->type != HighResStft)
    Errorf("CheckStftHighRes : stft is not highres");
}

void CheckStftHarmo(const STFT stft)
{
  CheckStftNotEmpty(stft);
  if(stft->type != HarmoStft)
    Errorf("CheckStftHarmo : stft is not harmo");
}

void CheckStftUpToDate(const STFT stft)
{
  CheckStftNotEmpty(stft);
  if(stft->flagUpToDate == NO)
    Errorf("CheckStftUpToDate : stft is out of date");
}


/* Allocation/Desallocation of STFT */
static void InitStft(STFT stft)
{	
  InitTFContent(stft);

  stft->tRate      = 1;
  stft->nFrames    = 0;
  stft->fRate      = 1;
  stft->nSubBands    = 0;
  stft->borderType = STFT_DEFAULT_BORDERTYPE;
  stft->firstp     = 0;
  stft->lastp      = -1;
  stft->freqIdMin  = 0;
  stft->freqIdMax  = 0;
  stft->windowShape = STFT_DEFAULT_WINDOWSHAPE;
  stft->windowSize 	   = 0;
  stft->flagUpToDate 	= NO;
  
  /* Default type for a stft : complex, zero-padded, gabor window */
  stft->type		= STFT_DEFAULT_STFTTYPE;
  /* The complex part */
  stft->real 		= NULL;
  stft->imag 		= NULL;

  /* The harmo part */
  stft->iFMO = 1;
  stft->dimHarmo             = 1;
}

STFT NewStft(void)
{
  STFT stft;
  
#ifdef DEBUGALLOC
  DebugType = "Stft";
#endif
  
  stft = (STFT) Malloc(sizeof(Stft));
  InitValue(stft,&tsStft);
  InitStft(stft);
  return(stft);
}

void ClearStft(STFT stft)
{
  if(stft == NULL)
    Errorf("ClearStft : NULL stft");
  
  if(stft->real) {
    DeleteSignal(stft->real);
    stft->real = NULL;
  }
  if(stft->imag) {
    DeleteSignal(stft->imag);
    stft->imag = NULL;
  }

  InitStft(stft);
}

STFT DeleteStft(STFT stft)
{
  if(stft == NULL)
    Errorf("DeleteStft : NULL stft");
  
  if (stft->nRef==0) {
    Errorf("*** Danger : trying to delete a temporary stft\n");
  }
  
  RemoveRefValue(stft);
  if (stft->nRef > 0) 
    return(NULL);
  
  if(stft->real) {
    DeleteSignal(stft->real);
    stft->real = NULL;
  }
  if(stft->imag) {
    DeleteSignal(stft->imag);
    stft->imag = NULL;
  }
  
#ifdef DEBUGALLOC
  DebugType = "Stft";
#endif
  
  Free(stft);
  return(NULL);
}

void TouchStft(STFT stft)
{
  if(stft == NULL)
    Errorf("TouchStft : NULL stft");
  stft->flagUpToDate = NO;
}



void PrintStft(const STFT stft,char flagShort)
{
  /* Checking argument */
  CheckStft(stft);
  
  if(flagShort) {
    Printf("[%3d]    %-7d %-7d %-7d %-7d %s %s\n",
	   stft->windowSize,
	   stft->tRate,stft->nFrames,stft->fRate,stft->nSubBands,
	   BorderType2Name(stft->borderType),WindowShape2Name(stft->windowShape),StftType2Name(stft->type));
  }
  else {
    /* Case of an empty stft */
    if(stft->windowSize <= 1) {
      Printf("  empty\n");
      return;
    }
    Printf("  window        : %s %d\n",
	   WindowShape2Name(stft->windowShape),stft->windowSize);
    Printf("  type          : %s\n",StftType2Name(stft->type));
    Printf("  size          : [%d frames x %d subbands]\n",stft->nFrames,stft->nSubBands);
    Printf("  tRate,fRate   : %-4d %-4d\n",stft->tRate,stft->fRate);
    if(stft->type==HarmoStft) {
      Printf("  range of f0   : [%g %g] ([%d %d])\n",
	     FreqId2Freq(stft,stft->freqIdMin),FreqId2Freq(stft,stft->freqIdMax),stft->freqIdMin,stft->freqIdMax);
      Printf("  dimension     : %d\n",stft->dimHarmo);
      Printf("  quasi-ortho   : %d\n",stft->iFMO);
    }
    Printf("Analyzed signal information\n");
    Printf("  border     : %s\n",BorderType2Name(stft->borderType));
    Printf("  signalSize : %d\n",stft->signalSize);
    Printf("  x0         : %g\n",stft->x0);
    Printf("  dx         : %g [Sampling Frequency = %g]\n",stft->dx,1/stft->dx);
    Printf("  firstp     : %d (%g)\n",stft->firstp,TimeId2Time(stft,stft->firstp));
    Printf("  lastp      : %d (%g)\n",stft->lastp,TimeId2Time(stft,stft->lastp));
  }
}

/*********************************************************/
/*
 *	Various functions to set a stft field
 */
/*********************************************************/





/*
 * Compute the frequency 'box' [freqIdMin,freqIdMax] on which 
 * one can look for partial 0<=k<stft->dimHarmo corresponding to 
 * the fundamental frequency freq0Id.
 */
char HarmoPartialBox(STFT stft,int k,int freq0Id, 
		     int* pk_f0IdMin,int* pk_f0IdMax)
{  
  float A = 0;
  float df;
  /* Checking arguments */
  CheckStftHarmo(stft); 
  if(k<0 || k >= stft->dimHarmo)
    Errorf("HarmoPartialBox : k %d is out of range [0, %d[",k,stft->dimHarmo);

  *pk_f0IdMax = (k+1)*freq0Id;
  *pk_f0IdMin = (k+1)*freq0Id;
  
  /* Not really used for now since A = 0 */
  df = (int) ((A/(stft->windowSize*stft->dx))*stft->dx*GABOR_MAX_FREQID+.5);
  
  *pk_f0IdMin -= df; 
  *pk_f0IdMax += df;
  
  if(*pk_f0IdMin > *pk_f0IdMax) {
    Errorf("k_f0IdMin > k_f0IdMax\n");
  }
  
  if(*pk_f0IdMin >= stft->fRate*stft->nSubBands) {
    //#define DEBUGPARTIAL
#ifdef DEBUGPARTIAL
    Printf("Bad Box (1): s=%d k=%d f0=%d",stft->windowSize,k,freq0Id);
    Printf("box=[%d %d] rate=%d length=%d\n",*pk_f0IdMin,*pk_f0IdMax,stft->fRate,stft->nSubBands);
#endif
    return(NO);
  }
  
  if(*pk_f0IdMax > stft->fRate*(stft->nSubBands-1)) {
#ifdef DEBUGPARTIAL
    Printf("Bad Box (2): s=%d k=%d f0=%d",stft->windowSize,k,freq0Id);
    Printf("box=[%d %d] rate=%d length=%d\n",*pk_f0IdMin,*pk_f0IdMax,stft->fRate,stft->nSubBands);
#endif
    return(NO);
  }
#ifdef DEBUGPARTIAL
  if(k != 0) {
    Printf("Good Box (2): o=%d k=%d f0=%d",stft->windowSize,k,freq0Id);
    Printf("box=[%d %d] rate=%d length=%d\n",*pk_f0IdMin,*pk_f0IdMax,stft->fRate,stft->nSubBands);
  }
#endif
  return(YES);
  
  /* Second version NEVER REACHED, NOT FINALIZED */
  *pk_f0IdMin = (k+1)*freq0Id-stft->fRate*((freq0Id/stft->fRate - stft->iFMO)/2);
  
  if(*pk_f0IdMin >= stft->fRate*stft->nSubBands) 
    return(NO);
  
  *pk_f0IdMax = (k+1)*freq0Id+stft->fRate*((freq0Id/stft->fRate - stft->iFMO)/2);
  *pk_f0IdMax = MIN(*pk_f0IdMax-stft->fRate,stft->fRate*(stft->nSubBands-1));
  
  if(*pk_f0IdMin > *pk_f0IdMax) 
    Errorf("k_f0IdMin > k_f0IdMax\n");
  
  return(YES);
  
}

/**********************************************************************/
/*
 * Compute a time-freq grid at a given windowSize, given two numbers
 * 'timeRedund' and 'freqRedund' that specify whether there is subsampling
 * or not at the given scale.
 */
/**********************************************************************/
void SizeStftGrid(STFT stft,int windowSize,int timeRedund,int freqRedund)
{
  CheckTFContent(stft);
  if(stft->signalSize == 0)  Errorf("SizeStftGrid : bad signalSize %d <= 0",stft->signalSize);
  /* Is it necessary ? */
  if(!INRANGE(STFT_MIN_WINDOWSIZE,windowSize,STFT_MAX_WINDOWSIZE) || (windowSize > GABOR_MAX_FREQID))
    Errorf("SizeStftGrid : Bad windowSize %d not in [%d %d] or larger that %d",windowSize,STFT_MIN_WINDOWSIZE,STFT_MAX_WINDOWSIZE,GABOR_MAX_FREQID);
  // TODO : understand or remove ?
  if(!INRANGE(1,timeRedund,GABOR_MAX_FREQID))
    Errorf("SizeStftGrid : Bad timeRedund %d for GABOR_MAX_FREQID = %d",timeRedund,GABOR_MAX_FREQID);
  if(!INRANGE(1,freqRedund,GABOR_MAX_FREQID))
    Errorf("SizeStftGrid : Bad freqRedund %d for GABOR_MAX_FREQID = %d",freqRedund,GABOR_MAX_FREQID);

  // In the time domain ... use of timeRedund
  if (windowSize <= timeRedund)    stft->tRate 	= 1;
  else                             stft->tRate 	= windowSize/(timeRedund);

  // In the frequency domain ... use of freqRedund !
  if (windowSize > GABOR_MAX_FREQID/freqRedund)
    stft->fRate 	= 1;
  else
    stft->fRate 	= 2*GABOR_MAX_FREQID/((freqRedund)*windowSize);

  stft->nFrames	= stft->signalSize/(stft->tRate);
  if((stft->nFrames)*(stft->tRate)< stft->signalSize)
    (stft->nFrames) += 1;

  stft->nSubBands 	= GABOR_NYQUIST_FREQID/(stft->fRate)+1;
  stft->windowSize = windowSize;
}

/***********************************************/
/*
 * Setting a stft time-frequency grid
 * and allocating the data arrays.
 * Keeps the ATFCONTENT fields
 * Resets it to STFT_DEFAULT_BORDERTYPE and STFT_DEFAULT_WINDOWSHAPE
 */
/***********************************************/
void SizeStft(STFT stft,char windowShape,int windowSize,
	      int timeRedund,int freqRedund,
	      char borderType,char stftType,
	      float freq0IdMin,float freq0IdMax,
	      int iFMO,int dimHarmo)
{
  struct aTFContent tfContent;
  // Used for the grid
  int shiftMin,shiftMax;
  // Only for HarmoStft type
  int f0IdMin, f0IdMax;
  int i_f0Min,i_f0Max;
  int size;
  int octave;
  
  // Checking arguments
  if(!WindowShapeIsOK(windowShape))   Errorf("SizeStft : Bad windowShape %d\n",windowShape);
  // TODO REMOVE THIS CONSTRAINT !!!!
  octave = (int) (log(windowSize)/log(2.0)+0.5);
  if((1<<octave) != windowSize)       Errorf("SizeStft : windowSize %d is not a power of two!",windowSize);

  if(!BorderTypeIsOK(borderType))     Errorf("SizeStft : unknown border type %d",borderType);
  // Checkings specific to HarmoStft type
  if(stftType == HarmoStft) {
    if(freq0IdMin <= 0.0)
      Errorf("SizeStft : bad freq0IdMin %g <= 0.0",freq0IdMin);
    if(freq0IdMin > freq0IdMax)
      Errorf("SizeStft : bad freq0 range [%g %g]!",freq0IdMin,freq0IdMax);
    if(freq0IdMax > GABOR_NYQUIST_FREQID)
      Errorf("SizeStft : bad freq0IdMax %g > %d",freq0IdMax,GABOR_NYQUIST_FREQID);
    if(iFMO <= 0)
      Errorf("SizeStft : bad iFMO %d",iFMO);
    if(dimHarmo <= 0)
      Errorf("SizeStft : bad dimHarmo %d",dimHarmo);
  }
  
  
  // Clearing the signals if necessary
  CopyFieldsTFContent(stft,&tfContent);
  ClearStft(stft);
  CopyFieldsTFContent(&tfContent,stft);
  
  // Setting the time freq grid 
  SizeStftGrid(stft,windowSize,timeRedund,freqRedund);

  // The window and border now
  stft->windowShape = windowShape;
  stft->borderType = borderType;
  
  /* 
   * Determines which are the first and last timeId in a stft
   * that are not affected by left/right side effects
   */   
  
  /* 
   * The time support of a window centered at timeId is 
   * [timeId+shiftMin,timeId+shiftMax]
   * It is affected by border effects only if this support
   * intersects the outside of [0,signalSize-1], hence
   * lastp + shiftmax = signalSize-1
   * firstp+ shiftmin = 0
   */
  ComputeWindowSupport(stft->windowSize,stft->windowShape,0.0,
		       &shiftMin,&shiftMax);
  
  stft->firstp = MAX(0,-shiftMin);
  stft->lastp  = MIN(stft->signalSize-1,stft->signalSize-1 - shiftMax);
  TouchStft(stft);

  stft->freqIdMin = 0;
  stft->freqIdMax = GABOR_NYQUIST_FREQID;

  switch(stftType) {
  case ComplexStft:
    stft->type = stftType;
    if(stft->real == NULL)
      stft->real = NewSignal();
    if(stft->imag == NULL)
      stft->imag = NewSignal();
    SizeSignal(stft->real,stft->nFrames*stft->nSubBands,YSIG); 
    SizeSignal(stft->imag,stft->nFrames*stft->nSubBands,YSIG); 
    ZeroSig(stft->real);
    ZeroSig(stft->imag);
    stft->real->dx = stft->fRate;
    stft->imag->dx = stft->fRate;
    break;
  case RealStft:
  case PhaseStft:
  case HighResStft:
    stft->type = stftType;
    if(stft->real == NULL)
      stft->real = NewSignal();
    SizeSignal(stft->real,stft->nFrames*stft->nSubBands,YSIG); 
    ZeroSig(stft->real);
    stft->real->dx = stft->fRate;
    // We should also put the right firstp,lastp here
    // but this depends on depthHighRes !
    break;
  case HarmoStft:
    stft->type =  HarmoStft;
    
    // Compute the actual range of fundamental frequencies
    /* ***************************************************************/
    /*
     *                  Orthogonality condition : 
     *
     *  Given the freq0Id range [freq0IdMin,freq0IdMax], and the <windowSize>,
     *  what is the region of allowed indexes ON THE GRID i_f0=freq0Id/fRate,
     *  knowing that there is a LOWER BOUND i_f0 >= iFMO (iF0Min"Ortho")
     *  for these indexes in order to keep the partials at frequencies 
     *  k*freq0Id and (k+1)*freq0Id QUASI ORTHOGONAL.
     *  -> i_f0Min and i_f0Max depend on <windowSize> 
     */
    /* ***************************************************************/
    
    /* the f0 range */
    QuantizeRangeLarge(freq0IdMin,freq0IdMax,stft->fRate,
		       &f0IdMin,&f0IdMax);      
    i_f0Min = f0IdMin/stft->fRate;
    i_f0Max = f0IdMax/stft->fRate;
    
    i_f0Min = MAX(iFMO,i_f0Min);
    if(i_f0Min*stft->fRate < freq0IdMin) {
      i_f0Min++;
    }
    i_f0Max = MIN(stft->nSubBands-1,i_f0Max);
    
    /* This is now the freq0Id-range */
    stft->freqIdMin = i_f0Min*stft->fRate;
    stft->freqIdMax = i_f0Max*stft->fRate;
    stft->iFMO      = iFMO;
    stft->dimHarmo  = dimHarmo;
    
    /* If the range is not empty we allocate the data accordingly */
    if(stft->freqIdMin <= stft->freqIdMax) {
      stft->real = NewSignal();
      size = stft->nFrames*(((stft->freqIdMax-stft->freqIdMin)/stft->fRate)+1);
      SizeSignal(stft->real,size,YSIG); 
      ZeroSig(stft->real);
      stft->real->x0 = stft->freqIdMin;
      stft->real->dx = stft->fRate;
    }
    else {
      stft->real = NULL;
      Warningf("SizeStft : empty 'harmo' stft at windowSize=%d",windowSize);
    }
    
    break;
  default : 
    Errorf("SizeStft : unknown stftType %d",stftType);
  }
}   


/******************************************
 *
 *  Functions to access the real and imaginary
 *  inner product of a STFT structure
 *  corresponding to a given timeId (for all frequencies)
 *
 ******************************************/
void GetStftData(STFT stft,float timeId,float* *pReal,float* *pImag)
{
  int tId;
  
  /* Checking argument */
  CheckStft(stft);
  if(stft->real == NULL)
    Errorf("GetStftData : empty stft->real");
  if(pReal == NULL)
    Errorf("GetStftData : NULL output");
  if(pImag != NULL && stft->imag == NULL) 
    Errorf("GetStftData : trying to get imag in real stft");

  /* Some tests */
  if(timeId != (int) timeId) 
    Errorf("GetStftData : Bad timeId %g not integer",timeId);
  tId = timeId;
  
  if (tId % stft->tRate != 0 || tId < 0 || tId/stft->tRate >= stft->nFrames) 
    Errorf("GetStftData : Bad timeId %d : rate %d range [0 %d]\n",
	   tId,stft->tRate,stft->nFrames);
  
  /* Getting the data */
  *pReal = stft->real->Y+(tId/stft->tRate)*(((stft->freqIdMax-stft->freqIdMin)/stft->fRate)+1);
  if(pImag != NULL)
    *pImag = stft->imag->Y+(tId/stft->tRate)*stft->nSubBands;
} 

/*
 * The GENERAL command for computing a stft
 */
 
void C_Stftd(char **argv)
{
  // The signal to be analyzed
  SIGNAL signal;
  struct aTFContent tfContent;
  // The output stft 
  STFT stft = NULL;

  // Some intermediate STFT variables
  STFT stftTmp        = NULL;
  STFT stftComplex    = NULL;
  STFT stftReal       = NULL;
  STFT stftPhase      = NULL;
  // Parameters for the grid

  int timeRedund,freqRedund;
  // The window
  char windowShape;
  int  windowSize;
  // Border effects
  char borderType;

  // The limits of the update
  int  timeIdMin,timeIdMax;
  
  char *name;
  char opt;
  char stftType;

  
  // Read the main arguments
  argv = ParseArgv(argv,tSTFT_,NULL,&stft,tSIGNALI,&signal,-1);
  if(stft == NULL) 
    stft = GetStftCur();
  // The signal size
  tfContent.signalSize = signal->size;
  tfContent.x0 = signal->x0;
  tfContent.dx = signal->dx;

  // The minimum and maximum timeId
  timeIdMin = 0;
  timeIdMax = tfContent.signalSize-1;
  
  // Default Option values
  windowShape = STFT_DEFAULT_WINDOWSHAPE;
  stftType    = STFT_DEFAULT_STFTTYPE;
  timeRedund  = STFT_DEFAULT_TIMEREDUND;
  freqRedund  = STFT_DEFAULT_FREQREDUND;
  borderType  = STFT_DEFAULT_BORDERTYPE;

  // Try to read the windowShape
  ParseArgv(argv,tSTR_,NULL,&name,-1);
  if(name != NULL) {
    windowShape = Name2WindowShape(name);
    argv++;
  }
  // The windowSize
  argv = ParseArgv(argv,tINT,&windowSize,-1);

  // Try to read the type
  ParseArgv(argv,tSTR_,NULL,&name,-1);
  if(name != NULL) {
    if(!strcmp(name,"complex") || !strcmp(name,"real")    || !strcmp(name,"phase")) {
      stftType = Name2StftType(name);
      argv++;
    }
  }

  // Reading additional options
  while(( opt = ParseOption(&argv))) {
    switch(opt) {
      // Border type
    case 'b' :
      argv = ParseArgv(argv,tSTR,&name,-1);
      borderType = Name2BorderType(name);
      break;
      // Time redundancy factor
    case 'T' :
      argv = ParseArgv(argv,tINT,&timeRedund,-1);
      if(timeRedund<=0) Errorf("Bad timeRedund %d",timeRedund);
      break;
      // Frequency redundancy factor
    case 'F' :
      argv = ParseArgv(argv,tINT,&freqRedund,-1);
      if(freqRedund<=0) Errorf("Bad freqRedund %d",freqRedund);
      break;
    default : ErrorOption(opt);
    }
  }
  NoMoreArgs(argv);

  //
  // Computation of a Complex spectrogram 
  //
  
  // -If we are computing a complex spectrogram 
  // we don't need any temporary stft structure
  if(stftType == ComplexStft) 
    stftTmp = stft;
  // -Otherwise we need one to store the (main) complex spectrogram.
  else
    stftTmp = NewStft();
  CopyFieldsTFContent(&tfContent,stftTmp);
  SizeStft(stftTmp,windowShape,windowSize,timeRedund,freqRedund,
	   borderType,ComplexStft,0,0,0,0);

  // Computing the complex spectrogram
  UpdateStftComplex(stftTmp,signal,timeIdMin,timeIdMax);
  // Are we done ?
  if(stftType == ComplexStft) {
    return;
  } else
    stftComplex = stftTmp;
  
  //
  // Computation of a Real spectrogram 
  //
  
  // -If we are computing a real or phase spectrogram
  // we don't need any temporary stft structure
  if(stftType == RealStft || stftType == PhaseStft) 
    stftTmp = stft;
  // -Otherwise we need one to store the real spectrogram.
  else
    stftTmp = NewStft();
  CopyFieldsTFContent(&tfContent,stftTmp);
  // If we compute ONLY the phase
  if(stftType == PhaseStft)
    SizeStft(stftTmp,windowShape,windowSize,timeRedund,freqRedund,borderType,
	     PhaseStft,0,0,0,0);
  // Else we need the RealStft in any case
  else
    SizeStft(stftTmp,windowShape,windowSize,timeRedund,freqRedund,borderType,
	     RealStft,0,0,0,0);

  // Compute the real or phase stft
  UpdateStftRealOrPhase(stftTmp,stftComplex,timeIdMin,timeIdMax); 
  // Are we done ?
  if (stftType == RealStft || stftType == PhaseStft) {
    // We delete the stftComplex 
    stftComplex = DeleteStft(stftComplex);
    return;
  }
  else
    stftReal = stftTmp;

}

/*
 * The TFContent fields : x0,dx,signalSize,freqIdNyquist
 */
static char *dxDoc = "{[= <dx>]} {Sets/Gets the abscissa step of the signal which has been analyzed.}";

void *GetDxTFContentV(ATFCONTENT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(dxDoc);
  
  return(GetFloatField(val->dx,arg));
}

void *SetDxTFContentV(ATFCONTENT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(dxDoc);

  return(SetFloatField(&(val->dx),arg,FieldSPositive));
}

static char *x0Doc = "{[= <x0>]} {Sets/Gets the first abscissa of the signal which has been analyzed.}";
void *GetX0TFContentV(ATFCONTENT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(x0Doc);
  return(GetFloatField(val->x0,arg));
}

void *SetX0TFContentV(ATFCONTENT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(x0Doc);
  return(SetFloatField(&(val->x0),arg,0));
}

static char *signalSizeDoc = "{} {Gets the size of the original signal of the time-frequency transform.}";
void *GetSignalSizeTFContentV(ATFCONTENT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(signalSizeDoc);
  return(GetIntField(val->signalSize,arg));
}

static char *freqIdNyquistDoc = "{} {Gets the Nyquist frequency in sample coordinates.}";

/* 
 * This is a bit special ... val is not used : 'freqIdNyquist' is a builtin constant 
 * that is useful for index<->real frequency conversions
*/
void *GetFreqIdNyquistTFContentV(VALUE val, void **arg)
{
  char *field = ARG_G_GetField(arg);
  /* Documentation */
  if (val == NULL)  return(freqIdNyquistDoc);
  return(GetIntField(GABOR_NYQUIST_FREQID,arg));
}

/*
 * The window fields : windowShape,windowSize
 */
static char *windowShapeDoc = "{} {Gets the windowShape of a Short Time Fourier Transform. "
WindowShapeHelpString
". To see the shape of a window, you can build it using the 'stft window ...' command.}";
static char *windowSizeDoc = "{} {Gets the windowSize of a Short Time Fourier Transform, i.e. the number of samples of its window. So far only powers of 2 are allowed.  To see the shape of a window, you can build it using the 'stft window ...' command.}";

void *GetWindowShapeStftV(STFT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(windowShapeDoc);
  return(GetStrField(WindowShape2Name(val->windowShape),arg));
}

void *GetWindowSizeStftV(STFT val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(windowSizeDoc);
  return(GetIntField(val->windowSize,arg));
}

static char *gridDoc = "{} {Gets stft 'grid' i.e. returns a &listv {timeRate timeLength freqRate freqLength}.}";
void *GetGridStftV(STFT stft,void **arg)
{
  LISTV lv;
  if(stft == NULL) return(gridDoc);

  lv = TNewListv();
  AppendInt2Listv(lv,stft->tRate);
  AppendInt2Listv(lv,stft->nFrames);
  AppendInt2Listv(lv,stft->fRate);
  AppendInt2Listv(lv,stft->nSubBands);
  return(GetValueField(lv,arg));
}

static char *typeDoc = "{} {Gets stft type (it is either 'complex' for short time fourier transform, 'real', 'phase' or 'highres'}";
void *GetTypeStftV(STFT val,void **arg)
{
  if(val == NULL) return(typeDoc);
  return(GetStrField(StftType2Name(val->type),arg));
}

static char *borderDoc = "{} {Returns stft <border type>"
BorderTypeHelpString
".}";
static char *firstpDoc = "{} {Gets stft <firstp>}";
static char *lastpDoc = "{} {Gets stft <lastp>}";

void *GetBorderStftV(STFT stft,void **arg)
{
  if(stft == NULL) return(borderDoc);
  return(GetStrField(BorderType2Name(stft->borderType),arg));
}

void *GetFirstpStftV(STFT stft,void **arg)
{
  if(stft == NULL) return(firstpDoc);
  return(GetIntField(stft->firstp,arg));
}

void *GetLastpStftV(STFT stft,void **arg)
{
  if(stft == NULL) return(lastpDoc);
  return(GetIntField(stft->lastp,arg));
}
  
static char *dataDoc = "{} {Returns a signal containing the stft real data or a listv {real imag} containing its complex data}";
void *GetDataStftV(STFT stft,void **arg)
{
  SIGNAL signalR = NULL,signalI = NULL;
  LISTV lv = NULL;

  /* Documentation */
  if(stft == NULL) return(dataDoc);
  
  /* Case of an empty stft */
  if(stft->real==NULL) {
    return(GetValueField(nullValue,arg));
  }
  switch(stft->type) {
  case ComplexStft :
    lv = TNewListv();
    signalR = TNewSignal();
    signalI = TNewSignal();
    CopySig(stft->real,signalR);
    CopySig(stft->imag,signalI);
    signalR->x0 = FreqId2Freq(stft,stft->freqIdMin);
    signalR->dx = FreqId2Freq(stft,stft->fRate);
    signalI->x0 = FreqId2Freq(stft,stft->freqIdMin);
    signalI->dx = FreqId2Freq(stft,stft->fRate);
    AppendValue2Listv(lv,(VALUE)signalR);
    AppendValue2Listv(lv,(VALUE)signalI);
    return(GetValueField(lv,arg));
  case RealStft: 
  case PhaseStft: 
  case HighResStft:
  case HarmoStft:
    signalR = TNewSignal();
    CopySig(stft->real,signalR);
    signalR->x0 = FreqId2Freq(stft,stft->freqIdMin);
    signalR->dx = FreqId2Freq(stft,stft->fRate);
    return(GetValueField(signalR,arg));
  }
}

/*
 * The field list
 */
struct field fieldsStft[] = {
  "dx",GetDxTFContentV,SetDxTFContentV,NULL,NULL,
  "x0",GetX0TFContentV,SetX0TFContentV,NULL,NULL,
  "signalSize",GetSignalSizeTFContentV,NULL,NULL,NULL,
  "freqIdNyquist",GetFreqIdNyquistTFContentV,NULL,NULL,NULL,
  "windowShape",GetWindowShapeStftV,NULL,NULL,NULL,
  "windowSize",GetWindowSizeStftV,NULL,NULL,NULL,
  "grid",GetGridStftV,NULL,NULL,NULL,
  "border",GetBorderStftV,NULL,NULL,NULL,
  "firstp",GetFirstpStftV,NULL,NULL,NULL,
  "lastp",GetLastpStftV,NULL,NULL,NULL,
  "type",GetTypeStftV,NULL,NULL,NULL,
  "sig",GetDataStftV,NULL,NULL,NULL,
  NULL, NULL, NULL, NULL, NULL
};

/*
 * The type structure for STFT
 */

TypeStruct tsStft = {

  "{{{&stft} {This type is the basic type for Short Time Fourier transforms and related time-frequency transforms.}}}",  /* Documentation */

  &stftType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteStft,     /* The Delete function */
  NewStft,     /* The New function */
  
  NULL,       /* The copy function */
  ClearStft,       /* The clear function */
  
  ToStrStft,       /* String conversion */
  ShortPrintStft,   /* The Print function : print the object when 'print' is called */
  PrintInfoStft,   /* The PrintInfo function : called by 'info' */

  NumExtractStft,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsStft,      /* The list of fields */
};

/* EOF */

