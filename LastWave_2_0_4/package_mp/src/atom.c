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


/***********************************************************************************/
/*
 * 	Allocation and Desallocations of atom, management of the '&atom' variable.
 */
/************************************************************************************/

#include "lastwave.h"
#include "int_fsilist.h"
#include "atom.h"

/**********************************/
/*
 * 	ATOM VARIABLES
 */
/**********************************/

char *atomType = "&atom";
/*
 * Answers to the different print messages
 */
 
void ShortPrintAtom(ATOM atom)
{
  PrintAtom(atom,YES);
}

char *ToStrAtom(ATOM atom, char flagShort)
{
  static char str[30];
  sprintf(str,"<&atom;%p>",atom);
  return(str);
}

void PrintInfoAtom(ATOM atom)
{
  PrintAtom(atom,NO);
}

ATOM TNewAtom(void)
{
  ATOM atom;
  
  atom = NewAtom();
  TempValue(atom);
  return(atom);
}

/*******************************************************************/
/*
 * 	Conversion between "sample" coordinates
 *	and "physical" coordinates  using sample-frequency and GABOR_MAX_FREQID
 */
/*******************************************************************/
float LW_TFConvertUnit(const void *content,float value,char unitType,char convertType)
{
  ATFCONTENT tfContent = (ATFCONTENT) content;
  
  // Some checkings
  if(content == NULL)
    Errorf("LW_TFConvertUnit : NULL content");
  if(convertType!= LW_TF_Id2RealConvert && convertType != LW_TF_Real2IdConvert)
    Errorf("LW_TFConvertUnit : (Weird) unknown conversion type %d",convertType);
  if(tfContent->dx <= 0.0)
    Errorf("LW_TFConvertUnit : bad dx %g",tfContent->dx);
  // Conversion
  switch(unitType) {
    // Time
  case LW_TF_TimeUnit :
    if(convertType == LW_TF_Id2RealConvert) 
      return(value*tfContent->dx+tfContent->x0);
    else
      return((value-tfContent->x0)/tfContent->dx);
    // Frequency
  case LW_TF_FreqUnit :
    if(convertType == LW_TF_Id2RealConvert) 
      return(value/(tfContent->dx*GABOR_MAX_FREQID));
    else
      return(value*GABOR_MAX_FREQID*tfContent->dx);
    // Chirp
  case LW_TF_ChirpUnit :
    if(convertType == LW_TF_Id2RealConvert) 
      return(value/(tfContent->dx*tfContent->dx*GABOR_MAX_FREQID));
    else
      return(value*GABOR_MAX_FREQID*tfContent->dx*tfContent->dx);
  default : 
    Errorf("LW_TF_ConvertUnit : (Weird) unknown unit type %d",unitType);
  }
  // Should never be reached but ensures that the function returns a value
  return(0);
}

/**************************************/
/* 
 *	ALLOCATIONS
 */
/**************************************/

/***********************************/
/*
 *           Checkings 
 */
/***********************************/

void CheckAtom(const ATOM atom)
{
  CheckTFContent(atom);
  // Window 
  if(!INRANGE(STFT_MIN_WINDOWSIZE,atom->windowSize,STFT_MAX_WINDOWSIZE))
    Errorf("CheckAtom : bad windowSize %d",atom->windowSize);
  if(atom->windowSize>atom->signalSize)
    Errorf("CheckAtom : bad windowSize %d compared to signalSize %d",atom->windowSize,atom->signalSize);
  if(!WindowShapeIsOK(atom->windowShape))
    Errorf("CheckAtom : bad windowShape %d",atom->windowShape);
  // Time location
  if(atom->timeId != (int) atom->timeId)  Errorf("CheckAtom : timeId is not int (not implemented yet)");      
  if(!INRANGE(0,atom->timeId,atom->signalSize-1))
    Errorf("CheckAtom : bad timeId %g not in [0 %d]",atom->timeId,atom->signalSize-1);
  // Freq location : TODO remove this restriction to allow pitch shifting ?
  // Or restrict to GABOR_NYQUIST_FREQID after changing the "AutoInnerProd" function ?
  if(!INRANGE(0,atom->freqId,GABOR_MAX_FREQID))
    Errorf("CheckAtom : bad freqId %g not in [0 %d]",atom->freqId,GABOR_MAX_FREQID);
  // Chirp `location'
  if(!INRANGE(-GABOR_MAX_CHIRPID,atom->chirpId,GABOR_MAX_CHIRPID))    
    Errorf("CheckAtom : bad |chirpId|=|%g| > %d",atom->chirpId,GABOR_MAX_CHIRPID);

  // Case of DC/Nyquist atoms
  if(atom->freqId == 0.0 || atom->freqId == GABOR_NYQUIST_FREQID) {
    if(atom->chirpId != 0.0)
      Errorf("CheckAtom : DC/Nyquist (freqId %g) is chirped at %g",atom->freqId,atom->chirpId);
    if(atom->coeffI != 0.0)
      Errorf("CheckAtom : DC/Nyquist (freqId %g) has coeffI %g",atom->freqId,atom->coeffI);
  }

  if(atom->coeff2 < 0) Errorf("CheckAtom : coeff2 %g < 0!",atom->coeff2);
  if(atom->realGG*atom->realGG+atom->imagGG*atom->imagGG>1)
    Errorf("CheckAtom : bad GG (%g,%g)",atom->realGG,atom->imagGG);
}

void CheckAtomReal(const ATOM atom)
{
  CheckAtom(atom);
  if(atom->flagGGIsSet == NO)
    Errorf("CheckAtomReal : atom GG is not set");
}

/*  Allocation/ Initialization of ATOM */
static void InitAtom(ATOM atom)
{
  InitTFContent(atom);

  // Window shape
  atom->windowShape = STFT_DEFAULT_WINDOWSHAPE;
  atom->windowSize  = STFT_DEFAULT_WINDOWSIZE;  
  // Time-freq location
  atom->timeId  = 0.0;
  atom->freqId  = 0.0;
  atom->chirpId = 0.0;
  
  // This is the right value of gg when freqId==0
  atom->realGG   = 1.0;
  atom->imagGG   = 0.0;
  atom->flagGGIsSet = NO;

  // The complex coeff is zero
  atom->coeffR      = 0.0;
  atom->coeffI      = 0.0;
  // The squared amplitude and phase 
  atom->coeff2   = 0.0;
  atom->cosPhase = 1.0;
  atom->sinPhase = 0.0;

}

ATOM NewAtom(void)
{
  ATOM atom;
  
#ifdef DEBUGALLOC
  DebugType = "Atom";
#endif
  
  atom = (ATOM) Malloc(sizeof(Atom));
  InitValue(atom,&tsAtom); 
  InitAtom(atom);
  return(atom);
}

void ClearAtom(ATOM atom)
{
  InitAtom(atom);
}

ATOM DeleteAtom(ATOM atom)
{
  if(atom == NULL) Errorf("DeleteAtom : NULL atom");
  
  if (atom->nRef==0) Errorf("DeleteAtom : Weird Error : trying to delete a temporary atom\n");
  
  RemoveRefValue(atom);
  if (atom->nRef > 0) return(NULL);
  
#ifdef DEBUGALLOC
  DebugType = "Atom";
#endif
  
  Free(atom);
  return(NULL);
}

// Makes a copy of all fields of an atom
ATOM CopyAtom(const ATOM in,ATOM out)
{
  if(in == NULL) return(NULL);
  if(out == NULL) out = NewAtom();
  if(in == out) return(out);

  CopyFieldsTFContent(in,out);
  out->windowShape = in->windowShape;
  out->windowSize  = in->windowSize;  

  out->timeId   = in->timeId;
  out->freqId   = in->freqId;
  out->chirpId  = in->chirpId;
  
  out->realGG     = in->realGG;
  out->imagGG     = in->imagGG;  
  out->flagGGIsSet  = in->flagGGIsSet;

  out->coeffR = in->coeffR;
  out->coeffI = in->coeffI;
  out->coeff2   = in->coeff2;
  out->cosPhase = in->cosPhase;
  out->sinPhase = in->sinPhase;

  return(out);
}

/* Printing utility */
void PrintAtom(const ATOM atom,char flagShort)
{
  CheckAtom(atom);
  if(flagShort) {
    Printf("(s,t,f,c) = (%d,%g,%g,%g), ",
	   atom->windowSize,TimeId2Time(atom,atom->timeId),FreqId2Freq(atom,atom->freqId),ChirpId2Chirp(atom,atom->chirpId));
    if(atom->flagGGIsSet)
      Printf("coeff2=%g phase=(%g,%g)\n",atom->coeff2,atom->cosPhase,atom->sinPhase);
    else
      Printf("coeff (%g,%g)",atom->coeffR,atom->coeffI);
  }
  else {
    Printf("Scale  : %g (%d)\n",atom->dx*atom->windowSize,atom->windowSize,atom->dx);
    Printf("Time   : %g (%g)\n",TimeId2Time(atom,atom->timeId),atom->timeId);
    Printf("Freq   : %g (%g)\n",FreqId2Freq(atom,atom->freqId),atom->freqId);
    Printf("Chirp  : %g (%g)\n",ChirpId2Chirp(atom,atom->chirpId),atom->chirpId);
    if(atom->flagGGIsSet) {
#ifdef ATOM_ADVANCED
    Printf("  correlation with complex conjugate  (%g,%g)\n",atom->realGG,atom->imagGG);
#endif // ATOM_ADVANCED
      Printf("Coeff2 : %g\n",atom->coeff2);
      Printf("Phase  : (%g,%g)\n",atom->cosPhase,atom->sinPhase);
    } 
    Printf("Coeff  : (%g,%g)\n",atom->coeffR,atom->coeffI);
  }
}

/* returns YES iff the time-support of the atoms intersect
 * and [*pTimeIdMin,*pTimeIdMax] is the intersection of the supports
 */
char AtomsIntersect(const ATOM atom1,const ATOM atom2,long *pTimeIdMin,long *pTimeIdMax)
{
  int timeIdMin1,timeIdMin2,timeIdMax1,timeIdMax2;
  long timeIdMin,timeIdMax;
  
  /* Checking arguments */
  CheckAtom(atom1);
  CheckAtom(atom2);
  
  /* Computing supports and their intersection */
  ComputeWindowSupport(atom1->windowSize,atom1->windowShape,atom1->timeId,&timeIdMin1,&timeIdMax1);
  ComputeWindowSupport(atom2->windowSize,atom2->windowShape,atom2->timeId,&timeIdMin2,&timeIdMax2);
  
  timeIdMin = MAX(timeIdMin1,timeIdMin2);
  timeIdMax = MIN(timeIdMax1,timeIdMax2);
  
  if(pTimeIdMin != NULL)
    *pTimeIdMin = timeIdMin;
  if(pTimeIdMax != NULL)
    *pTimeIdMax = timeIdMax;
  
  return(timeIdMin<=timeIdMax);
}

/*
 *
 * Computes (realGG,imagGG) : The real and imaginary parts
 * of the integral of the atom with its conjugate
 *
 * We guarantee that if the atom has symmetries one has imagGG = 0
 * Namely : if the chirp is zero and the window is symmetric (e.g., Gaussian)
 *
 */
static void SetAtomGG(ATOM atom)
{
  static ATOM copy = NULL;
  
  /* The signals to build the atom in */
  static SIGNAL  atomSignalR = NULL;
  static SIGNAL  atomSignalI = NULL;
  
  float realGG,imagGG;
  
  /* Utility */
  char flagAsym = NO;
  /* Checkings */
  CheckAtom(atom);
  // Case where the GG is tabulated
  if(atom->chirpId == 0.0 && atom->freqId == (int) atom->freqId) {
    if(ComputeWindowGG(atom->windowShape,atom->windowSize,atom->freqId,&realGG,&imagGG)) {
      atom->realGG = realGG;
      atom->imagGG = imagGG;
      atom->flagGGIsSet = YES;
      return;
    }
  }
  // Else, do a numeric or a fast analytic computation 
  AutoAtomInnerProduct(atom,NO,&realGG,&imagGG);
  atom->realGG = realGG;
  atom->imagGG = imagGG;
  atom->flagGGIsSet = YES;
}

void TouchAtomGG(ATOM atom)
{
  atom->flagGGIsSet = NO;
}



// TODO : explain 
void ComputeRealPhaseEnergy(float coeffR,float coeffI,float realGG,float imagGG,
			    float *pPhase,float *pCosPhase,float *pSinPhase,
			    float *pEnergy)
{
  float innerProd2;
  float real,imag;
  float norm;

  // Basic checking
  if(realGG*realGG+imagGG*imagGG>1) Errorf("ComputeRealPhaseEnergy : (Weired) bad GG (%g %g)",realGG,imagGG);
  if(pPhase==NULL && (pCosPhase!=NULL || pSinPhase!=NULL))
    Errorf("ComputeRealPhaseEnergy : bad phase pointers");
  if(pPhase!=NULL && (pCosPhase==NULL || pSinPhase==NULL))
    Errorf("ComputeRealPhaseEnergy : bad phase pointers");

  // The squared magnitude of the complex innerproduct
  innerProd2 	= coeffR*coeffR+coeffI*coeffI;

  // The simplest case : when the result is zero!
  if (innerProd2 == 0)  {
    if(pEnergy) 
      *pEnergy 	= 0.0;
    if(pPhase) {
      *pCosPhase= 1.0;
      *pSinPhase= 0.0;
      *pPhase 	= 0.0;
    }
    return;
  }
  
  // The special case where (realGG,imagGG)==(1,0)
  // corresponds to freqId = 0,Nyquist : 
  // -> no phase, just a sign
  if(realGG==1.0) {
    // Checking :
    if(imagGG != 0.0 || coeffI != 0.0) Errorf("ComputeRealPhaseEnergy : non zero imaginary part when realGG==1.0!");

    // Setting energy
    if(pEnergy)
      *pEnergy = innerProd2;
    // Setting phase
    if(pPhase) {
      if (coeffR >= 0) {
	*pCosPhase = 1.0;
	*pSinPhase = 0.0;
	*pPhase	 = 0.0;
      } else {
	*pCosPhase = -1.0;
	*pSinPhase = 0.0;
	*pPhase	 = 0.5;
      }
    }
    return;
  } else {
    /* Cf. PhD thesis R. Gribonval */
    /* Computing energy */
    if(pEnergy) {
      *pEnergy    = innerProd2-realGG*(coeffR*coeffR-coeffI*coeffI)+imagGG*2*coeffR*coeffI;
      *pEnergy   *= 2;
      *pEnergy   /= 1-(realGG*realGG+imagGG*imagGG);
      // DEBUG 
      if(*pEnergy<0) Errorf("ComputeRealPhaseEnergy : (Weired) negative energy!");
    }
    /* Computing phase */
    if(pPhase) {
      real = (1-realGG)*coeffR + imagGG*coeffI;
      imag = (1+realGG)*coeffI + imagGG*coeffR;
    
      norm = sqrt(real*real+imag*imag);
      *pCosPhase  = real/norm;
      *pSinPhase  = imag/norm;

      *pPhase = atan2(imag,real)/(2*M_PI);
      if (*pPhase<0)
	*pPhase = 1+*pPhase;
      if(*pPhase >= 1.0)
	*pPhase = *pPhase-1.0;
    }
  }
}

void CastAtomReal(ATOM atom)
{
  float phase;
  CheckAtom(atom);

  SetAtomGG(atom);
  ComputeRealPhaseEnergy(atom->coeffR,atom->coeffI,atom->realGG,atom->imagGG,
			 &phase,&(atom->cosPhase),&(atom->sinPhase),&(atom->coeff2));
  // DEBUG
  if(isnan(atom->coeff2) || atom->coeff2 < 0.0) Errorf("CastAtomReal : atom coeff2=%g!",atom->coeff2);
}

/*********************************************
 *      The main function which builds atoms 
 *********************************************
 * Build either (depending whether atomI!=NULL or not)
 * a/ a COMPLEX NORMALIZED atom (coeff2,coeffR,coeffI,cosPhase,sinPhase are not taken into account).
 * b/ a REAL atom (coeff2,cosPhase,sinPhase are used) 
 * The 'dx' and 'x0' fields of the output signals are properly set.
 *
 * If 'flagAtomSizeOnly'==YES, the output signal(s) have 'windowSize' samples corresponding to the 'support' of the atom,
 *   (the first sample is zero)
 * Else, the output signal(s) have 'signalSize' samples.
 *
 * The function built is the product of
 * -an amplitude 'sqrt(coeff2)', for REAL atoms (the amplitude is one for COMPLEX ones).
 * -a normalized window g_s(t-timeId), with shape and size determined by 'windowShape' and 'windowSize'
 * -a modulation :
 *  COMPLEX : exp(i*2*PI*(freqId*(t-timeId)/GABOR_MAX_FREQID+(chirpId/2)*(t-timeId)^2/GABOR_MAX_FREQID))
 *  or
 *  REAL      cos(2*PI*(phi+freqId*(t-timeId)/GABOR_MAX_FREQID+(chirpId/2)*(t-timeId)^2/GABOR_MAX_FREQID))
 *
 * WARNING: BORDERTYPE IS NOT  PROPERLY TREATED YET !!!!!
 ***************************************************************************/

void BuildAtom(const ATOM atom,SIGNAL atomR,SIGNAL atomI,char borderType,char flagAtomSizeOnly)
{
  /* Parameters of the atom (we make a copy to get shorter notations) */
  float timeId;
  float freqId;
  float chirpId;
  float coeff2,cosPhase,sinPhase,phase;

  /* Tabulated data */
  SIGNAL expikR 	= NULL; 
  SIGNAL expikI 	= NULL;
  SIGNAL window 	= NULL;

  static SIGNAL tempSignal = NULL;
  unsigned long reconsSize;
  // TODO : change to (unsigned) long when prototype of ComputeWindowSupport is changed ?
  int   timeIdMin,timeIdMax;

  unsigned long i;
  long index;
  double phaseArgument;
  
  float ishift;
  
  float factor;
  float norm2;
  
  /* Checking */
  CheckAtom(atom);
  if(atomR == NULL)               Errorf("BuildAtom : NULL atomR (Weird Error)");
  if(!BorderTypeIsOK(borderType)) Errorf("BuildAtom : unknown borderType %d",borderType);
  
  /* We copy the atom parameters to get shorter notation */
  timeId  = atom->timeId;
  freqId  = atom->freqId;
  chirpId = atom->chirpId;

  /* Complex atoms are normalized, real atoms use coeff2 and cosPhase/sinPhase */
  if(atomI==NULL) {
    coeff2    = atom->coeff2;
    cosPhase  = atom->cosPhase;
    sinPhase  = atom->sinPhase;
    phase = atan2(atom->sinPhase,atom->cosPhase)/(2*M_PI);
    if (phase<0)     phase = 1+phase;
    if(phase >= 1.0) phase = phase-1.0;
  } else {
    coeff2    = 1.0;
    cosPhase  = 1.0;
    sinPhase  = 0.0;
    phase     = 0.0;
  }

  /* Determine the size of the output signal(s) and allocate it/them */
  if (flagAtomSizeOnly == YES)     reconsSize = atom->windowSize;
  else                             reconsSize = atom->signalSize;
  SizeSignal(atomR,reconsSize,YSIG);
  ZeroSig(atomR);
  if(atomI != NULL) {
    SizeSignal(atomI,reconsSize,YSIG);
    ZeroSig(atomI);
  }
  
  /* Compute the support of the atom [timeIdMin,timeIdMax] 
   * i.e. the set of indexes (in sample coordinates of the output signal) that have to be filled.
   */
  ComputeWindowSupport(atom->windowSize,atom->windowShape,timeId,&timeIdMin,&timeIdMax);
  /* Add the first point of the atom, which value is zero */
  // TODO : remove!
  timeIdMin -=1;
  
  /* Set the dx/x0 fields of the output signals */
  if(flagAtomSizeOnly == YES) {
    /* The output signals have a time offset with respect to the input signal */
    atomR->x0 = atom->x0 + timeIdMin*atom->dx;
    atomR->dx = atom->dx;
    if(atomI != NULL){
      atomI->x0   = atomR->x0;
      atomI->dx   = atomR->dx;
    }
    /* TODO : EXPLAIN !!!! */
    ishift  = timeIdMin-timeId;
    /* The whole output signal has to be filled without any border effect */
    timeIdMin	= 0;
    timeIdMax	= reconsSize-1;
  }
  else {
    /* The output signals have the same time offset as the input signal  */
    atomR->x0 = atom->x0;
    atomR->dx = atom->dx;
    if(atomI != NULL){
      atomI->x0 = atomR->x0;
      atomI->dx = atom->dx;
    }
    /* TODO : EXPLAIN !!!! */
    ishift 	= -timeId;
    /* TODO : treat the border effects */
    switch(borderType) {
    case BorderPad0 :
      timeIdMin	= MAX(0,timeIdMin);
      timeIdMax	= MIN(reconsSize-1,timeIdMax);
      break;
    default :
      Errorf("BuildAtom : border %s not treated yet",BorderType2Name(borderType));
    }
  }
  
  /*
   * Now, we start actually building the atom
   */
  /* First, the window of the atom */
  GetTabWindow(atom->windowShape,atom->windowSize,&window);
  for(i=timeIdMin; i <= timeIdMax; i++)  atomR->Y[i] = window->Y[i-timeIdMin];
  if (atomI != NULL) {
    for(i=timeIdMin;i<=timeIdMax;i++)    atomI->Y[i] = atomR->Y[i];
  }
  /* Then, multiply by the modulation, including the chirp */
  /* We can use the tabulated exponentials only when
   * - freqId is an integer [because freqId*(i+ishift) must be integer for all i]
   * - phase is either 0 or 0.5 modulo 1
   * - chirpId is an *even* integer        [because 0.5*chirpId*(i+ishift)^2 must be integer for all i]
   */
  if (freqId != (int) freqId || ((phase != 0) && (phase != 0.5)) || chirpId/2 == (int) (chirpId/2)) {
    /* Modulating without tabulated data */
    for(i=timeIdMin;i<=timeIdMax;i++) {
      /* TODO : EXPLAIN the use of ishift */
      phaseArgument = 2*M_PI*(phase+ (freqId*(i+ishift))/(GABOR_MAX_FREQID)
			      +0.5*(chirpId*(i+ishift)*(i+ishift))/(GABOR_MAX_FREQID));
      atomR->Y[i] *= cos(phaseArgument);
      if(atomI != NULL)
	atomI->Y[i] *= sin(phaseArgument);
    }
  } else {
    /* Modulating with tabulated data */
    GetTabExponentials(&expikR,&expikI);

    /* Computations are modulo 'GABOR_MAX_FREQID', with positive integers */
    ishift   = GABOR_MAX_FREQID + ((int) ishift)%(GABOR_MAX_FREQID);
    for(i=timeIdMin;i<=timeIdMax;i++)  {	
      /* TODO : EXPLAIN the use of ishift */
      // Is there a problem here with phase?
      //index = ((int) (phase*GABOR_MAX_FREQID
      //+ freqId*(i+ishift)
      //+ 0.5*chirpId*(i+ishift)*(i+ishift)))	%(GABOR_MAX_FREQID);
      index  = ((int)freqId)*(i+(int)ishift);
      index += ((int)(chirpId/2))*(i+(int)ishift)*(i+(int)ishift);
      if(phase==0.5)
	index += GABOR_MAX_FREQID/2;
      index = index%GABOR_MAX_FREQID;
      index = (index+GABOR_MAX_FREQID)%GABOR_MAX_FREQID;
      if(!INRANGE(0,index,expikR->size-1)) Errorf("BuildAtom : (Weird) internal accessing expikR[%d]",index);
      if(i>=atomR->size)      Errorf("BuildAtom : (Weird) internal atomR");
      atomR->Y[i] *= expikR->Y[index];
      if(atomI != NULL)
	atomI->Y[i] *= expikI->Y[index];
    }
  }
  
  /*
   * If the atom we built is complex, we are done
   */
  if(atomI!=NULL) return;
  else {
    /* Compute the norm of the real atom */
    norm2 = 0.0;
    for(i=timeIdMin;i<=timeIdMax;i++) norm2 += atomR->Y[i]*atomR->Y[i];  
    /* Normalize the atom and set its amplitude */
    factor = sqrt(coeff2/norm2);
    for(i=timeIdMin;i<=timeIdMax;i++) atomR->Y[i] *= factor;
  }
}


/*
 * The functions to Get the atom fields
 */

/*
 * The window fields : windowShape,windowSize,
 */
static char *windowShapeDoc = "{[= <windowShape>]} {Sets/Gets the windowShape of an atom. "
WindowShapeHelpString
"}";
static char *windowSizeDoc = "{[= <windowSize>]} {Sets/Gets the windowSize of an atom, i.e. the number of samples of its window. So far only powers of 2 are allowed.}";

void *GetWindowShapeAtomV(ATOM atom, void **arg)
{
  /* Documentation */
  if (atom == NULL) return(windowShapeDoc);
  return(GetStrField(WindowShape2Name(atom->windowShape),arg));
}

void *GetWindowSizeAtomV(ATOM atom, void **arg)
{
  /* Documentation */
  if (atom == NULL) return(windowSizeDoc);
  return(GetIntField(atom->windowSize,arg));
}

void *SetWindowShapeAtomV(ATOM atom, void **arg)
{
  static char *windowShapeName = NULL;
  char windowShape;
  /* Documentation */
  if (atom == NULL) return(windowShapeDoc);

  // Allocation (once only) of the resulting string
  if(windowShapeName == NULL) {
    windowShapeName = CharAlloc(1);
    windowShapeName[0] = '\0';
  }
  if(SetStrField(&windowShapeName,arg)==NULL) return(NULL);
  windowShape = Name2WindowShape(windowShapeName);
  // If the windowShape changed we have to recompute the 'GG' of the atom
  // to keep coherent
  if(atom->windowShape == windowShape) return(strType);
  atom->windowShape = windowShape;
  SetAtomGG(atom);
  // DEBUG TODO : remove!
  if(atom->realGG*atom->realGG+atom->imagGG*atom->imagGG>1) {
    PrintInfoValue(atom);
    Errorf("SetWindowShapeAtomV : (Weired) bad GG (%g %g)",atom->realGG,atom->imagGG);
  }
  return(strType);
}

void *SetWindowSizeAtomV(ATOM atom, void **arg)
{
  int windowSize;
  /* Documentation */
  if (atom == NULL) return(windowShapeDoc);
  // Init (for += syntax for example)
  windowSize = atom->windowSize;
  if(SetIntField(&windowSize,arg,FieldSPositive)==NULL) return(NULL);

  /* Some checkings */
  if(!INRANGE(STFT_MIN_WINDOWSIZE,windowSize,STFT_MAX_WINDOWSIZE) || windowSize > atom->signalSize) {
    SetErrorf("Cannot set atom.windowSize=%d. Should be between %d and %d and at most atom.signalSize (%d)",
	      windowSize,STFT_MIN_WINDOWSIZE,STFT_MAX_WINDOWSIZE,atom->signalSize);
    return(NULL);
  }
  // If the windowSize changed we have to recompute the 'GG' of the atom
  // to keep coherent
  if(atom->windowSize == windowSize) return(numType);
  atom->windowSize = windowSize;
  SetAtomGG(atom);
  // DEBUG TODO : remove!
  if(atom->realGG*atom->realGG+atom->imagGG*atom->imagGG>1) {
    PrintInfoValue(atom);
    Errorf("SetWindowSizeAtomV : (Weired) bad GG (%g %g)",atom->realGG,atom->imagGG);
  }
  return(numType);
}


/*
 * The dt/df fields
 */
static char *dtDoc = "{} {Gets the time spread of an atom in seconds.}";
static char *dfDoc = "{} {Gets the frequency spread of an atom in Hertz.}";
static char *supportDoc = "{} {Gets the time support {timeMin timeMax} of an atom in seconds}"; 
static char *supportIdDoc = "{} {Gets the time support {timeIdMin timeIdMax} of an atom in sample coordinates}"; 

void *GetDtDfAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);
  LISTV lv;
  int timeIdMin,timeIdMax;
  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"dt")) return(dtDoc);
    if(!strcmp(field,"df")) return(dfDoc);
    if(!strcmp(field,"support")) return(supportDoc);
    if(!strcmp(field,"supportId")) return(supportIdDoc);
  }
  if(!strcmp(field,"dt"))  
    return(GetFloatField(0.5*(TimeId2Time(atom,atom->windowSize)-TimeId2Time(atom,0)),arg));
  if(!strcmp(field,"df"))
    return(GetFloatField(0.5*FreqId2Freq(atom,((float)GABOR_MAX_FREQID)/(2*M_PI*theGaussianSigma2*(float)atom->windowSize)),arg));
  if(!strcmp(field,"support") || !strcmp(field,"supportId")) {
    lv = TNewListv();
    ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,
			 &timeIdMin,&timeIdMax);
    if(!strcmp(field,"supportId")) {
      AppendInt2Listv(lv,timeIdMin);
      AppendInt2Listv(lv,timeIdMax);
    } else {
      AppendFloat2Listv(lv,TimeId2Time(atom,timeIdMin));
      AppendFloat2Listv(lv,TimeId2Time(atom,timeIdMax));
    }
    return(GetValueField(lv,arg));
  }
}

/*
 * The time(Id) fields
 */
static char *timeIdDoc = "{[= <timeId>]} {Sets/Gets the time center of an atom in sample coordinates, i.e. an index 0 <= timeId < atom.signalSize.}";
static char *timeDoc = "{[= <time>]} {Sets/Gets the time center of an atom in real coordinates, i.e. the real time in seconds.}";

void *GetTimeAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);
 
  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"time")) return(timeDoc);
    if(!strcmp(field,"timeId")) return(timeIdDoc);
  }
  if(!strcmp(field,"time"))  
    return(GetFloatField(TimeId2Time(atom,atom->timeId),arg));
  if(!strcmp(field,"timeId"))
    return(GetFloatField(atom->timeId,arg));
}

void *SetTimeAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);
  char flagId = NO;

  float time,timeId;
  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"time")) return(timeDoc);
    if(!strcmp(field,"timeId")) return(timeIdDoc);
  }
  if(!strcmp(field,"timeId"))    flagId = YES;
  else if(!strcmp(field,"time")) flagId = NO;
  else Errorf("SetTimeAtomV : Weird field %s",field);

  if(flagId) {
    // Init for += syntax
    timeId = atom->timeId;
    if(SetFloatField(&timeId,arg,FieldPositive)==NULL) return(NULL);
  }
  else {
    // Init for += syntax
    time = TimeId2Time(atom,atom->timeId);
    if(SetFloatField(&time,arg,0)==NULL) return(NULL);
    timeId = Time2TimeId(atom,time);
  } 

  /* Some checkings */
  if(!INRANGE(0,timeId,atom->signalSize-1)) {
    if(flagId)
      SetErrorf("Cannot set atom.timeId=%g because atom.signalSize-1=%d",timeId,atom->signalSize-1);
    else 
      SetErrorf("Cannot set atom.time=%g (should be in [0,%g])",time,TimeId2Time(atom,atom->signalSize-1));
    return(NULL);
  }
  atom->timeId = timeId;
  return(numType);
}
  

/*
 * The freq(Id) fields
 */
static char *freqIdDoc = "{[= <freqId>]} {Sets/Gets the frequency center of an atom in sample coordinates, i.e. an index 0 <= freqId <= atom.freqIdNyquist.}";

static char *freqDoc = "{[= <freq>]} {Sets/Gets the frequency center of an atom in real coordinates, i.e. the real frequency in Hertz.}";

void *GetFreqAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);

  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"freq")) return(freqDoc);
    if(!strcmp(field,"freqId")) return(freqIdDoc);
  }
  if(!strcmp(field,"freq")) return(GetFloatField(FreqId2Freq(atom,atom->freqId),arg));
  if(!strcmp(field,"freqId")) return(GetFloatField(atom->freqId,arg));
}

void *SetFreqAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);
  char flagId = NO;

  float freq,freqId;
  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"freq")) return(freqDoc);
    if(!strcmp(field,"freqId")) return(freqIdDoc);
  }
  if(!strcmp(field,"freqId"))    flagId = YES;
  else if(!strcmp(field,"freq")) flagId = NO;
  else Errorf("SetFreqAtomV : Weird field %s",field);

  if(flagId) {
    // Init for += syntax
    freqId = atom->freqId;
    if(SetFloatField(&freqId,arg,FieldPositive)==NULL) return(NULL);
  }
  else {
    // Init for += syntax
    freq   = FreqId2Freq(atom,atom->freqId);
    if(SetFloatField(&freq,arg,FieldPositive)==NULL) return(NULL);
    freqId = Freq2FreqId(atom,freq);
  }


  /* Some checkings */
  if(freqId > GABOR_NYQUIST_FREQID) {
    if(flagId)
      SetErrorf("Cannot set atom.freqId=%g because atom.freqIdNyquist=%d",freqId,GABOR_NYQUIST_FREQID);
    else
      SetErrorf("Cannot set atom.freq=%g (should be in [0,%g]",freq,FreqNyquist(atom));
    return(NULL);
  }
  if(atom->coeffI != 0.0 && (freqId == 0 || freqId == GABOR_NYQUIST_FREQID)) {
    if(flagId)
       SetErrorf("Cannot set atom.freqId=0 or %d (Nyquist) because atom.coeffI=%g (try setting atom.coeffI=0 first)",GABOR_NYQUIST_FREQID,atom->coeffI);
    else 
      SetErrorf("Cannot set atom.freq=0 or %g (Nyquist) because atom.coeffI=%g (try setting atom.coeffI=0 first)",atom->coeffI,FreqNyquist(atom));
    return(NULL);
  }
  // If the freqId changed we have to recompute the 'GG' of the atom
  // to keep coherent
  if(atom->freqId == freqId) return(numType);
  atom->freqId = freqId;
  SetAtomGG(atom);
  // DEBUG TODO : remove!
  if(atom->realGG*atom->realGG+atom->imagGG*atom->imagGG>1) {
    PrintInfoValue(atom);
    Errorf("SetFreqAtomV : (Weired) bad GG (%g %g)",atom->realGG,atom->imagGG);
  }
  return(numType);
}
  

/*
 * The chirp(Id) fields
 */
static char *chirpIdDoc = "{[= <chirpId>]} {Sets/Gets the chirp rate of an atom in sample coordinates, i.e. an index chirpId with |chirpId| <= atom.freqIdNyquist/2.}";

static char *chirpDoc = "{[= <chirp>]} {Sets/Gets the chirp rate of an atom in real coordinates, i.e. the real frequency slope in Hertz per second.}";

void *GetChirpAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);

  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"chirp")) return(chirpDoc);
    if(!strcmp(field,"chirpId")) return(chirpIdDoc);
  }
  if(!strcmp(field,"chirp"))  
    return(GetFloatField(ChirpId2Chirp(atom,atom->chirpId),arg));
  if(!strcmp(field,"chirpId"))
    return(GetFloatField(atom->chirpId,arg));
}

void *SetChirpAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);
  char flagId = NO;

  float chirp,chirpId;
  /* Documentation */
  if (atom == NULL){
    if(!strcmp(field,"chirp")) return(chirpDoc);
    if(!strcmp(field,"chirpId")) return(chirpIdDoc);
  }
  if(!strcmp(field,"chirpId"))    flagId = YES;
  else if(!strcmp(field,"chirp")) flagId = NO;
  else Errorf("SetChirpAtomV : Weird field %s",field);

  if(flagId) {
    // Init for += syntax
    chirpId = atom->chirpId;
    if(SetFloatField(&chirpId,arg,0)==NULL) return(NULL);
  }
  else {
    // Init for += syntax
    chirp = ChirpId2Chirp(atom,atom->chirpId);
    if(SetFloatField(&chirp,arg,0)==NULL) return(NULL);
    chirpId = Chirp2ChirpId(atom,chirp);
  } 

  /* Some checkings */
  if(!INRANGE(-GABOR_MAX_CHIRPID,chirpId,GABOR_MAX_CHIRPID)) {
    if(flagId)
      SetErrorf("Cannot set atom.chirpId=%g (its absolute value should be at most %g)",chirpId,GABOR_MAX_CHIRPID);
    else
      SetErrorf("Cannot set atom.chirp=%g (its absolute value should be at most %g)",chirp,ChirpId2Chirp(atom,GABOR_MAX_CHIRPID));
    return(NULL);
  }
  // If the chirpId changed we have to recompute the 'GG' of the atom
  // to keep coherent
  if(atom->chirpId == chirpId) return(numType);
  atom->chirpId = chirpId;
  SetAtomGG(atom);
  // DEBUG TODO : remove!
  if(atom->realGG*atom->realGG+atom->imagGG*atom->imagGG>1) {
    PrintInfoValue(atom);
    Errorf("SetChirpAtomV : (Weired) bad GG (%g %g)",atom->realGG,atom->imagGG);
  }
  return(numType);
}
  
#ifdef ATOM_ADVANCED
/*
 * The inner-product <g,_g> between a (normalized) complex atom
 * and its complex conjugate.
 */
static char *ggDoc = "{} {Gets a listv {real imag} that corresponds to the (complex) inner-product <g,_g> between a (normalized) complex atom 'g' and its complex conjugate '_g'. The value of <g,_g> depends on the windowShape, windowSize, frequency and chirp parameters. It is used to compute the energy ||P_{g,_g}s||^2 of the projection of a (real valued) signal 's' on the subspace spanned by 'g' and '_g' from the complex inner product <s,g>. WARNING : this is a read-only field. If you type 'atom.gg[0]=1' it will be accepted but what will be performed is similar to 'l=atom.gg; l[0]=1'.}";

void *GetGGAtomV(ATOM atom, void **arg)
{
  LISTV lv;
  /* Documentation */
  if (atom == NULL) return(ggDoc);
  lv = TNewListv();
  AppendFloat2Listv(lv,atom->realGG);
  AppendFloat2Listv(lv,atom->imagGG);
  return(GetValueField(lv,arg));
}
#endif //ATOM_ADVANCED

/*
 * The coeff fields : coeff2,phase 
 */
static char *coeff2Doc = "{[= <coeff2>]} {Sets/Gets the atom squared coefficient.}";
static char *phaseDoc = "{[= <phase>]} {Sets/Gets the atom phase (which is defined modulo 1).}";
static char *coeffDoc = "{} {Gets the atom complex coefficient.}";

static float _ComputePhase(float cosPhase,float sinPhase)
{
  float phase;

  phase = atan2(sinPhase,cosPhase)/(2*M_PI);
  if (phase<0)     phase = 1+phase;
  if(phase >= 1.0) phase = phase-1.0;
  return(phase);
}

void *GetCoeffAtomV(ATOM atom, void **arg)
{
  char *field = ARG_G_GetField(arg);
  float phase;
  LISTV lv;
  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"coeff2")) return(coeff2Doc);
    if(!strcmp(field,"phase"))  return(phaseDoc);
    if(!strcmp(field,"coeff"))  return(coeffDoc);
  }

  CheckAtomReal(atom);
  if(!strcmp(field,"coeff2"))  return(GetFloatField(atom->coeff2,arg));
  if(!strcmp(field,"phase")) {
    phase = _ComputePhase(atom->cosPhase,atom->sinPhase);
    return(GetFloatField(phase,arg));
  }
  if(!strcmp(field,"coeff"))  {
    lv = TNewListv();
    AppendFloat2Listv(lv,atom->coeffR);
    AppendFloat2Listv(lv,atom->coeffI);
    return(GetValueField((VALUE)lv,arg));
  }
}

void *SetCoeffAtomV(ATOM atom, void **arg)
{
  char *field = ARG_G_GetField(arg);
  float phase;
  /* Documentation */
  if (atom == NULL) {
    if(!strcmp(field,"coeff2")) return(coeff2Doc);
    if(!strcmp(field,"phase"))  return(phaseDoc);
  }

  if(!strcmp(field,"coeff2")) return(SetFloatField(&(atom->coeff2),arg,FieldSPositive));
  if(!strcmp(field,"phase")) {
    // Init for += syntax
    phase = _ComputePhase(atom->cosPhase,atom->sinPhase);
    if(SetFloatField(&phase,arg,0)==NULL) return(NULL);
    if((atom->freqId == 0 || atom->freqId == GABOR_NYQUIST_FREQID)
       && 2*phase != (int) 2*phase) {
      SetErrorf("Cannot set <phase>=%g : should be 0 or 0.5 (modulo 1) when atom.freq==0 or Nyquist. Try changing atom.freq(Id) first.",phase);
      return(NULL);
    }
    atom->cosPhase = cos(2*M_PI*(phase-(int)phase));
    atom->sinPhase = sin(2*M_PI*(phase-(int)phase));
    // DEBUG
    Printf("%g %g %g\n",phase,atom->cosPhase,atom->sinPhase);
    return(numType);
  }
}

/*
 * A virtual field to build the atom in
 */
static char *buildcDoc = "{[*opt,...] [:]} {Gets a pair {real imag} of signals where the normalized complex atom has been built.}";
static char *buildrDoc = "{[*opt,...] [:]} {Gets a signal where the real atom has been built.}";
static void *GetExtractBuildAtomV(ATOM atom,void **arg)
{
  char *field = ARG_G_GetField(arg);
  SIGNAL signalR = NULL;
  SIGNAL signalI = NULL;
  char borderType = Name2BorderType("pad0");// TODO an EXTRACT OPTION
  char flagAtomSizeOnly = NO; // TODO AN EXTRACT OPTION

  LISTV lv;

  if(atom==NULL) {
    if(!strcmp(field,"buildc")) return(buildcDoc);
    if(!strcmp(field,"buildr")) return(buildrDoc);
  }

  // Temporary allocation of the output signal(s)
  signalR = TNewSignal();
  if(!strcmp(field,"buildc")) signalI = TNewSignal();
  // Building the atom
  BuildAtom(atom,signalR,signalI,borderType,flagAtomSizeOnly);
  // Setting the output
  if(!strcmp(field,"buildc")) {
    lv = TNewListv();
    AppendValue2Listv(lv,(VALUE)signalR);
    AppendValue2Listv(lv,(VALUE)signalI);
    return(GetValueField(lv,arg));
  } else {
    return(GetValueField(signalR,arg));
  }
}

static char *buildExtractDoc = "{*sizeonly,*bperiodic,...} {*sizeonly : the signal(s) where the atom will be built will have exactly the size of the atom. Otherwise the atom.signalSize will be used.}";
static char *buildExtractOptions[] = {"*bperiodic","*bmirror","*bmirror1","*bconst","*b0","*sizeonly",NULL};

static void *GetExtractOptionsAtomV(ATOM atom, void **arg)
{
  char *field;
  
  field = ARG_G_GetField(arg);

  // There is no extraction on the atom itself
  if(field == NULL) return(NULL);

   /* doc */
  if (atom == NULL) {
    if (!strcmp(field,"buildr")) return(buildExtractDoc);
    if (!strcmp(field,"buildc")) return(buildExtractDoc);
    // There is no extraction on atom itself nor on other fields
    return(NULL);
  }

  if (!strcmp(field,"buildr")) return(buildExtractOptions);  
  if (!strcmp(field,"buildc")) return(buildExtractOptions);  
  return(NULL);
}


/*
 * Function to get the ExtractInfo for fields 'coeff' and 'build' (and 'gg' ?)
 */

static void *GetExtractInfoAtomV(ATOM atom, void **arg)
{
  static ExtractInfo extractInfo;
  
  char *field = ARG_EI_GetField(arg);

  unsigned long *options = ARG_EI_GetPOptions(arg);

  // No extraction is available on atom itself
  if(field==NULL) return(NULL);

  if(!strcmp(field,"buildr") || !strcmp(field,"buildc")) {
    SetErrorf("ExtractInfo for build not finalized");return(NULL);
    extractInfo.nSignals = 1;
    extractInfo.xmin = 0;
    extractInfo.xmax = 1;
    extractInfo.dx = 1;
    extractInfo.xsignal = NULL;
    extractInfo.flags = EIErrorBound;
    return(&extractInfo);
  } 

  return(NULL);
}

/*
 * The field list
 */
struct field fieldsAtom[] = {
  "dx",GetDxTFContentV,SetDxTFContentV,NULL,NULL,
  "x0",GetX0TFContentV,SetX0TFContentV,NULL,NULL,
  "signalSize",GetSignalSizeTFContentV,NULL,NULL,NULL,
  "freqIdNyquist",GetFreqIdNyquistTFContentV,NULL,NULL,NULL,
  "windowShape",GetWindowShapeAtomV,SetWindowShapeAtomV,NULL,NULL,
  "windowSize",GetWindowSizeAtomV,SetWindowSizeAtomV,NULL,NULL,
  "timeId",GetTimeAtomV,SetTimeAtomV,NULL,NULL,
  "time",GetTimeAtomV,SetTimeAtomV,NULL,NULL,
  "freqId",GetFreqAtomV,SetFreqAtomV,NULL,NULL,
  "freq",GetFreqAtomV,SetFreqAtomV,NULL,NULL,
  "chirp",GetChirpAtomV,SetChirpAtomV,NULL,NULL,
  "chirpId",GetChirpAtomV,SetChirpAtomV,NULL,NULL,
  "coeff2",GetCoeffAtomV,SetCoeffAtomV,NULL,NULL,
  "phase",GetCoeffAtomV,SetCoeffAtomV,NULL,NULL,
  "coeff",GetCoeffAtomV,NULL,NULL,NULL,
  "buildr",GetExtractBuildAtomV,NULL,GetExtractOptionsAtomV,GetExtractInfoAtomV,
  "buildc",GetExtractBuildAtomV,NULL,GetExtractOptionsAtomV,GetExtractInfoAtomV,
  //  "build",GetExtractBuildAtomV,NULL,NULL,NULL,
  "dt",GetDtDfAtomV,NULL,NULL,NULL,
  "df",GetDtDfAtomV,NULL,NULL,NULL,
  "support",GetDtDfAtomV,NULL,NULL,NULL,
  "supportId",GetDtDfAtomV,NULL,NULL,NULL,
#ifdef ATOM_ADVANCED
  "gg",GetGGAtomV,NULL,NULL,NULL,
#endif // ATOM_ADVANCED
  NULL, NULL, NULL, NULL, NULL
};

/*
 * The type structure for ATOM
 */

TypeStruct tsAtom = {

  "{{{&atom} {This type is the basic type for time-frequency atoms that are used in Short Time Fourier Transform and Matching Pursuit decompositions.}}}",  /* Documentation */

  &atomType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteAtom,     /* The Delete function */
  NewAtom,     /* The New function */
  
  CopyAtom,       /* The copy function */
  ClearAtom,       /* The clear function */
  
  ToStrAtom,       /* String conversion */
  ShortPrintAtom,   /* The Print function : print the object when 'print' is called */
  PrintInfoAtom,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsAtom,      /* The list of fields */
};
 
/* EOF */


