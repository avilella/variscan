/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 2000 Remi Gribonval, Emmanuel Bacry and Javier Abadia */
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
 * 	The HIGHRES Short Time Fourier Transform :
 *
 *		COMPUTATION and UPDATE
 */
/****************************************************/
#include "lastwave.h"

#include "atom.h"

/*#define DEBUG_HIGHRES*/

/*************************************************************/
/*
 *	COMPUTATION OF THE HIGH-RESOLUTION CORRELATION
 */
/*************************************************************/

extern void RRAtomInnerProduct(const ATOM atom1,const ATOM atom2,char flagForceNumeric,float *pReal);

/******************************/
/* 
 *	The function to compute 
 * 	the high-res correlation
 */
/******************************/
/*
 * We assume that the atom ENERGY and PHASE are already properly set
 * [firstp,lastp] is the range of timeId not affected by border effects
 * in the HIGHRES stft at the atom's windowSize
 */
static void SetAtomCoeff2HighRes(ATOM atom,STFT stftHighRes,STFT subStftReal,STFT subStftPhase)
{
  int timeId;
  
  static ATOM subAtom = NULL;
  int timeIdMinAtom,timeIdMaxAtom;
  int shiftMin,shiftMax;
  int timeIdMinSubAtoms,timeIdMaxSubAtoms;
  int tmp;

  int subTimeIdMin,subTimeIdMax;

  int subTRate;
  

  int subTimeId;
  int subFreqId;
  
  float *subCoeff2s,*subPhases;
  float subCoeff2,subPhase;
  float crossInner;
  
  /* Checking */
  CheckAtomReal(atom);
  if(!BorderTypeIsOK(stftHighRes->borderType))
    Errorf("SetAtomCoeff2HighRes : bad borderType %d",stftHighRes->borderType);
  CheckStftReal(subStftReal);
  CheckStftPhase(subStftPhase);
  if(subStftReal->flagUpToDate == NO || subStftPhase->flagUpToDate == NO)
    Errorf("SetAtomCoeff2HighRes: subStftReal or subStftPhase is out of date");
  // The atom MUST be on the time-frequency grid of subStft 
  // because we cannot recompute the sub-atom's energy
  timeId = (int) atom->timeId;
  if(atom->timeId != timeId || timeId % subStftReal->tRate != 0)
    Errorf("SetAtomCoeff2HighRes: timeId %g not on grid %d",atom->timeId,subStftReal->tRate);
  if(atom->freqId != (int)atom->freqId || ((int)atom->freqId) % subStftReal->fRate != 0)
    Errorf("SetAtomCoeff2HighRes: freqId %g not on grid %d",atom->freqId,subStftReal->fRate);
  
  /* Allocating (once) */
  if(subAtom == NULL) {
    subAtom = NewAtom();
  }
  else {
    ClearAtom(subAtom);
  }
  CopyFieldsTFContent(subStftReal,subAtom);
  subAtom->windowShape  = subStftReal->windowShape;
  subAtom->windowSize   = subStftReal->windowSize;
  subAtom->freqId    = subFreqId = atom->freqId;
  /* Case when the energy is ZERO */
  if (atom->coeff2 == 0.0) return;
  
  // The support of the main atom is [timeIdMinAtom,timeIdMaxAtom]
  ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,
		     &timeIdMinAtom,&timeIdMaxAtom);
  // A subAtom at 'subTimeId' has support 
  //[subTimeId+shiftMin,subTimeId+shiftMax]
  ComputeWindowSupport(subAtom->windowSize,subAtom->windowShape,0,
		     &shiftMin,&shiftMax);
  // This support is included in [timeIdMinAtom,timeIdMaxAtom] iff
  // subTimeId+shiftMin >= timeIdMinAtom
  //    and
  // subTimeId+shiftMax <= timeIdMaxAtom
  subTimeIdMin = timeIdMinAtom-shiftMin;
  subTimeIdMax = timeIdMaxAtom-shiftMax;

  // SubAtoms can only be on the grid at rate subTRate
  subTRate     = subAtom->windowSize;  //    subTRate = subStftReal->tRate;

  // Treat BORDER EFFECT : 
  // which subAtoms shall we actually look for
  switch(stftHighRes->borderType) {
    case BorderPad0 :
      // The subAtoms ??? CHANGE TODO ALL THIS BORDER EFFECT PART
      QuantizeRangeLarge(subTimeIdMin,subTimeIdMax,subTRate,
			 &subTimeIdMin,&subTimeIdMax);
      subTimeIdMin = MAX(subTimeIdMin,0);
      subTimeIdMax = MIN(subTimeIdMax,(subStftReal->nFrames*subTRate)-subTRate);

      // The union of the supports of the subAtoms is
      // [timeIdMinSubAtoms,timeIdMaxSubAtoms]
      ComputeWindowSupport(subAtom->windowSize,subAtom->windowShape,subTimeIdMin,
			 &timeIdMinSubAtoms,&tmp);
      ComputeWindowSupport(subAtom->windowSize,subAtom->windowShape,subTimeIdMax,
			 &tmp,&timeIdMaxSubAtoms);
      
      // The support [timeIdMinAtom,timeIdMaxAtom] 
      // of the atom must be ENTIRELY in [0 signalSize-1]
      if ((timeIdMinAtom < 0) || (timeIdMaxAtom > stftHighRes->signalSize-1)) {
	atom->coeff2 = 0.0;
	return;
      }
      // The support of all sub-atoms should satisfy the same
      if((timeIdMinSubAtoms < 0) ||
	 (timeIdMaxSubAtoms > subStftReal->signalSize-1)) {
	atom->coeff2 = 0.0;
	return;
      }
      break;
  default :
    Errorf("SetAtomCoeff2Highres : border type %s not treated yet",BorderType2Name(stftHighRes->borderType));
  }

  // Loop on the subAtoms timeIds : 
  // -initially we set atom->coeff2 to its maximum possible value
  // -we decrease it if necessary
  for(subTimeId = subTimeIdMin; 
      subTimeId <= subTimeIdMax; 
      subTimeId += subTRate) {
    /* The coefficient of the subAtom */
    GetStftData(subStftReal,subTimeId,&subCoeff2s,NULL);
    GetStftData(subStftPhase,subTimeId,&subPhases,NULL);
    subCoeff2 = subCoeff2s[subFreqId/subStftReal->fRate];
    subPhase  = subPhases[subFreqId/subStftPhase->fRate];
    
    // Properly setting the remaining fields of the sub-atom
    // to compute its
    // inner-product with the "big" one
    subAtom->timeId = subTimeId;
    if(!ComputeWindowGG(subAtom->windowShape,subAtom->windowSize,subAtom->freqId,&(subAtom->realGG),&(subAtom->imagGG)))
      Errorf("SetAtomCoeff2HighRes : error with ComputeWindowGG");

    subAtom->coeff2 = subCoeff2;
    // TODO : Is it costly ?? Do we need it ? Should we try something cheaper ?? 
    subAtom->cosPhase= cos(2*M_PI*subPhase);
    subAtom->sinPhase= sin(2*M_PI*subPhase);
    RRAtomInnerProduct(atom,subAtom,NO,&crossInner);

    // When the subAtom is orthogonal to the large one
    // it is NOT taken into account.
    if(crossInner == 0.0) {
      continue;
    }

    // If the sub-atom coefficient is zero !
    if(subCoeff2 == 0.0) {
      atom->coeff2 = 0.0;
      return;
    }

    // Case of a change of sign/phase
    if(crossInner < 0.0) {
      atom->coeff2 = 0.0;
      return;
    }

    // Taking the minimum encountered value
    if(atom->coeff2 > subCoeff2/(crossInner*crossInner))
      atom->coeff2 = subCoeff2/(crossInner*crossInner);
  }
}

/*************************************************/
/*
 *           Update of the REAL HIGHRES stft data,
 *           given a REAL ENERGY stft. 
 *           We loop on time, within a SPECIFIED TIME RANGE
 */
/*************************************************/

static void CheckStftHighResEnergy(STFT stftHighRes,
				   STFT stftReal,STFT stftPhase,
				   STFT subStftReal,STFT subStftPhase)
{
    /* Checking arguments */
    CheckStftHighRes(stftHighRes);
    CheckStftReal(stftReal);
    CheckStftPhase(stftPhase);
    CheckStftReal(subStftReal);
    CheckStftPhase(subStftPhase);
    if(stftReal->flagUpToDate == NO)
	Errorf("CheckStftHighResEnergy : stftReal is out of date");
    if(stftPhase->flagUpToDate == NO)
	Errorf("CheckStftHighResEnergy : stftPhase is out of date");
    if(subStftReal->flagUpToDate == NO)
	Errorf("CheckStftHighResEnergy : subStftReal is out of date");
    if(subStftPhase->flagUpToDate == NO)
	Errorf("CheckStftHighResEnergy : subStftPhase is out of date");

    /* Checking coherence of time-freq grids */
    CheckTFContentCompat(stftReal,stftHighRes);
    CheckTFContentCompat(stftPhase,stftHighRes);
    CheckTFContentCompat(subStftReal,stftHighRes);
    CheckTFContentCompat(subStftPhase,stftHighRes);
    if(stftReal->windowSize        != stftHighRes->windowSize ||
       stftPhase->windowSize        != stftHighRes->windowSize ||
       subStftReal->windowSize     != subStftPhase->windowSize)
	Errorf("CheckStftHighResEnergy : incompatible windowSize");
    if(stftReal->borderType     != stftHighRes->borderType  ||  
       stftPhase->borderType     != stftHighRes->borderType  ||  
       subStftReal->borderType  != stftHighRes->borderType  ||  
       subStftPhase->borderType  != stftHighRes->borderType  ||  
       stftReal->windowShape    != stftHighRes->windowShape ||
       stftPhase->windowShape    != stftHighRes->windowShape ||
       subStftReal->windowShape != stftHighRes->windowShape ||
       subStftPhase->windowShape != stftHighRes->windowShape)
	Errorf("CheckStftHighResEnergy : incompatible borderType or windowShape");

    /* Later to remove */
    if(subStftReal->windowSize > stftReal->windowSize)
      Errorf("CheckStftHighResEnergy : subStft windowSize %d >= %d !",subStftReal->windowSize,stftReal->windowSize);

    /* Remark : (sub)RateEnergy should ALWAYS divide rateHighRes */
    if(stftHighRes->fRate % stftReal->fRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible fRates %d %d",stftHighRes->fRate,stftReal->fRate);
    if(stftHighRes->tRate % stftReal->tRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible tRates %d %d",stftHighRes->tRate,stftReal->tRate);
    if(stftHighRes->fRate % subStftReal->fRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible sub fRates %d %d",stftHighRes->fRate,subStftReal->fRate);
    if(stftHighRes->tRate % subStftReal->tRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible sub tRates %d %d",stftHighRes->tRate,subStftReal->tRate);

    if(stftHighRes->fRate % stftPhase->fRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible fRates %d %d",stftHighRes->fRate,stftPhase->fRate);
    if(stftHighRes->tRate % stftPhase->tRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible tRates %d %d",stftHighRes->tRate,stftPhase->tRate);
    if(stftHighRes->fRate % subStftPhase->fRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible sub fRates %d %d",stftHighRes->fRate,subStftPhase->fRate);
    if(stftHighRes->tRate % subStftPhase->tRate != 0)
      Errorf("CheckStftHighResEnergy : incompatible sub tRates %d %d",stftHighRes->tRate,subStftPhase->tRate);
}



/***************************************************/
/*
 * Very helpful function to set a stft REAL HIGHRES part
 * at a given time 'timeId', for all frequencies.
 */
/***************************************************/
static void StftHighResSetFreq(STFT stftHighRes,
			       STFT stftReal,STFT stftPhase,
			       STFT subStftReal,STFT subStftPhase,
			       int timeId)
{
  static ATOM atom = NULL;
  int freqId;
  float *coeff2s,*phases,*coeff2sHighRes;
  float phase;
  /* Checkings */
  CheckStftHighResEnergy(stftHighRes,stftReal,stftPhase,subStftReal,subStftPhase);
  if(!INRANGE(0,timeId,stftHighRes->signalSize-1))
    Errorf("StftHighResSetFreq : bad timeId %d not in [0 %d]",timeId,stftHighRes->signalSize-1);

  /* Initializing (just once) */
  if(atom == NULL) {
    atom = NewAtom();
  } else {
    ClearAtom(atom);
  }
  CopyFieldsTFContent(stftHighRes,atom);
  atom->windowShape	= stftHighRes->windowShape;
  atom->windowSize = stftHighRes->windowSize;


  atom->timeId 	= timeId;
  atom->freqId  = 0;

  /* Getting the pointers to the data */
  GetStftData(stftReal,timeId,&coeff2s,NULL);
  GetStftData(stftHighRes,timeId,&coeff2sHighRes,NULL);
  GetStftData(stftPhase,timeId,&phases,NULL);

  /* Loop on all frequencies */
  for(freqId = 0;
      freqId < stftHighRes->fRate*stftHighRes->nSubBands; 
      freqId += stftHighRes->fRate) {
    // Setting the complex part of the atom
    atom->freqId  	 = freqId;
    // These fields are not used here
    atom->coeffR = 0.0;
    atom->coeffI = 0.0;
    // Convert to a real atom
    // Getting the energy and phase from the stftReal data
    if(!ComputeWindowGG(atom->windowShape,atom->windowSize,atom->freqId,&(atom->realGG),&(atom->imagGG)))
	Errorf("StftHighResSetFreq : error with ComputeWindowGG");

    atom->coeff2     = coeff2s[freqId/stftReal->fRate];
    phase            =   phases[freqId/stftReal->fRate];
    // DEBUG : is it too costly ?
    atom->cosPhase   = cos(2*M_PI*phase);
    atom->sinPhase   = sin(2*M_PI*phase);

    // Convert to HighResStft
    // The squared coefficient
    // Should take border type and stftHighRes->firstp/stftHighRes->lastp into effect ?
    SetAtomCoeff2HighRes(atom,stftHighRes,subStftReal,subStftPhase);

    // Store the computed data
    coeff2sHighRes[freqId/stftHighRes->fRate] = atom->coeff2;
  }
} 


/**************************************************************/
/*
 * Computes the values of the HighResStft stft associated with 
 * a given RealStft stft at the same windowSize and a RealStft stft
 * at a smaller windowSize (for High Time-Resolution) 
 * or (TODO) larger windowSize (for High Freq-Resolution). 
 * The signal associated with those RealStft stfts has changed
 * between the time-indexes [timeIdMin,timeIdMax].
 */
/**************************************************************/
void UpdateStftHighRes(STFT stftHighRes,
		       STFT stftReal,STFT stftPhase,
		       STFT subStftReal,STFT subStftPhase,
		       int timeIdMin,int timeIdMax)
{    
  int shiftMinAtom,shiftMaxAtom;
  int shiftMinSubAtom,shiftMaxSubAtom;
  int timeIdMinStft,timeIdMaxStft;
  //  int timeIdMinStft2,timeIdMaxStft2;
  //  char flagTwoLoops=NO;

  int timeId;
  
  /* Checking arguments */
  CheckStftHighResEnergy(stftHighRes,stftReal,stftPhase,subStftReal,subStftPhase);
  if((!INRANGE(0,timeIdMin,timeIdMax)) || 
     (!INRANGE(timeIdMin,timeIdMax,stftHighRes->signalSize-1)))
    Errorf("UpdateStftHighRes : bad time range [%d %d] not in [0 %d]",
	   timeIdMin,timeIdMax,stftHighRes->signalSize-1);
  
  
  // The support of a main atom at 'timeId' is
  // [timeId+shiftMinAtom,timeId+shiftMaxAtom]
  ComputeWindowSupport(stftHighRes->windowSize,stftHighRes->windowShape,0,
		     &shiftMinAtom,&shiftMaxAtom);
  // The support of a sub-atom at 'subTimeId' is
  // [subTimeId+shiftMinSubAtom,subTimeId+shiftMaxSubAtom]
  ComputeWindowSupport(subStftReal->windowSize,subStftReal->windowShape,0,
		     &shiftMinSubAtom,&shiftMaxSubAtom);

  // There are no boundary effects in the following region
  stftHighRes->firstp = shiftMaxSubAtom-shiftMinSubAtom-shiftMinAtom;
  stftHighRes->lastp  = stftHighRes->signalSize-1+shiftMinSubAtom-shiftMaxSubAtom-shiftMaxAtom;
  
  // The highres coefficient of a main atom can change
  // iff 
  //   timeId+shiftMaxAtom>=subTimeId+shiftMinSubAtom
  // where
  //   subTimeId+shiftMaxSubAtom>=timeIdMin
  // or
  //   timeId+shiftMinAtom<=subTimeId+shiftMaxSubAtom
  // where
  //   subTimeId+shiftMinSubAtom<=timeIdMax
  timeIdMinStft = timeIdMin-shiftMaxSubAtom
    +shiftMinSubAtom-shiftMaxAtom;
  timeIdMaxStft = timeIdMax-shiftMinSubAtom
    +shiftMaxSubAtom-shiftMinAtom;

  switch(stftHighRes->borderType) {
  case BorderPad0 :
    /* We consider only those on the  time-freq grid */
    QuantizeRangeLarge(timeIdMinStft,timeIdMaxStft,stftHighRes->tRate,
		       &timeIdMinStft,&timeIdMaxStft);
    timeIdMinStft = MAX(0,timeIdMinStft);
    timeIdMaxStft = MIN((stftHighRes->nFrames*stftHighRes->tRate)-stftHighRes->tRate,timeIdMaxStft);  
    break;
  default :
    Errorf("UpdateStftHighRes : border type %s not treated yet",BorderType2Name(stftHighRes->borderType));
  }

  // First time Loop 
  for (timeId = timeIdMinStft; 
       timeId <= timeIdMaxStft; 
       timeId += stftHighRes->tRate) {
    StftHighResSetFreq(stftHighRes,stftReal,stftPhase,subStftReal,subStftPhase,timeId);
  }
  stftHighRes->flagUpToDate = YES;
}

/* EOF */








