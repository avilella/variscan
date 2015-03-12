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


#include "lastwave.h"
#include "atom.h"

/***********************************************/
/* Inner-products between time-frequency atoms */
/***********************************************/

// External functions that perform approximate analytic computation of the inner product
// between two complex atoms for some windowShapes
extern void CCAtomAnalyticGauss(const ATOM atom1,const ATOM atom2,float *pReal,float *pImag);
extern void CCAtomAnalyticFoF(const ATOM atom1,const ATOM atom2,float *pReal,float *pImag);

// The EXACT inner-product between two complex atoms, with numerical computations (it involves building the atoms).
//  The values of coeffR/coeffI/coeff2/realGG/imagGG/cosPhase/sinPhase 
//  do not influence the result.
// TODO : optimize it (it is NOT AT ALL OPTIMIZED!!!)
static void CCAtomNumericInnerProduct(const ATOM atom1,const ATOM atom2,float *pReal,float *pImag)
{
  static SIGNAL atomR1 = NULL;
  static SIGNAL atomI1 = NULL;
  static SIGNAL atomR2 = NULL;
  static SIGNAL atomI2 = NULL;

  // TODO : other border types ?
  char borderType = STFT_DEFAULT_BORDERTYPE;
  long freqId1,freqId2;

  long timeIdMin,timeIdMax;
  int timeIdMin1,timeIdMin2;
  int timeIdMax1,timeIdMax2;

  int i;
  int i1,i2;

  /* Allocate the signals to build the two atoms in (once only) */
  if(atomR1 == NULL) {
    atomR1 = NewSignal();
    atomI1 = NewSignal();
    atomR2 = NewSignal();
    atomI2 = NewSignal();
  }

  /* 
   * Build the atoms with a unit norm in a complex signal.
   * The signals have exactly windowSize points.
   * The firstp point has a value of zero.
   */
  BuildAtom(atom1,atomR1,atomI1,borderType,YES);
  BuildAtom(atom2,atomR2,atomI2,borderType,YES);

  /* 
   * Compute the inner product :
   * a/ [timeIdMin timeIdMax] is the support
   *   (in 'absolute' sample coordinates) of the
   *   intersection of the supports of the atoms
   * b/ timeIdMin<j> is the index (in 'absolute' 
   *   sample coordinates) of the first non zero point in atom<j>
   */
  *pReal = *pImag = 0.0;
  AtomsIntersect(atom1,atom2,&timeIdMin,&timeIdMax);
  ComputeWindowSupport(atom1->windowSize,atom1->windowShape,atom1->timeId,&timeIdMin1,&timeIdMax1);
  ComputeWindowSupport(atom2->windowSize,atom2->windowShape,atom2->timeId,&timeIdMin2,&timeIdMax2);

  for(i = timeIdMin; i <= timeIdMax; i++) {
    /* 
     *Checked correct on the 22/02/2001 by R. Gribonval
     * The indexes in the atom windows : 
     * because we always have atom[0] = 0.0,
     *   i=timeIdMin1 corresponds to i1=1 
     *   i=timeIdMin2 corresponds to i2=1 
     * hence the expression of i1,i2
     * By construction we will (should?) always have
     *  1 <= i1,i2 <= atom(1|2)Size-1.
     */
    i1 = i-timeIdMin1+1;
    i2 = i-timeIdMin2+1;
      
    /* We multiply atom1[i] by Conjugate(atom2[i]) */
    *pReal += atomR1->Y[i1]*atomR2->Y[i2]+atomI1->Y[i1]*atomI2->Y[i2];
    *pImag += atomI1->Y[i1]*atomR2->Y[i2]-atomR1->Y[i1]*atomI2->Y[i2];
  }

  // If the atoms are indeed 'real' (i.e. unmodulated) 
  freqId1 = (long)atom1->freqId;
  freqId2 = (long)atom2->freqId;
  if((freqId1 == atom1->freqId) && (freqId2 == atom2->freqId)
     && (freqId1%GABOR_NYQUIST_FREQID)==0
     && (freqId2%GABOR_NYQUIST_FREQID)==0
     && atom1->chirpId == 0.0 && atom2->chirpId == 0.0)
    *pImag = 0.0;
  return;
}

/**********************************************/
/*
 * The inner product between two COMPLEX atoms.
 * The values of coeffR/coeffI/coeff2/realGG/imagGG/cosPhase/sinPhase 
 * do not influence the result.
 * - Some fast Analytic formulas are implemented in files 'atom_<window>innerprod.c'
 *  and used when possible ('gauss' and 'asym' only for the moment)
 * - Numerical evaluation is done for other shapes or when it is faster (small scales)
 * TODO : optimize the switch between analytic and numeric.
*/
/**********************************************/
static void CCAtomInnerProduct(const ATOM atom1,const ATOM atom2,char flagForceNumeric,float *pReal,float *pImag)
{
  /* Case of non-overlapping atoms */
  if(AtomsIntersect(atom1,atom2,NULL,NULL)==NO) {
    *pReal = *pImag = 0.0;
    return;
  }
  
  // Case where we want to force exact computation
  if(flagForceNumeric) {
    CCAtomNumericInnerProduct(atom1,atom2,pReal,pImag); 
    return;
  }

  /* Choosing the computation method adapted to the shape of the atoms */
  switch(atom1->windowShape) {
  case GaussWindowShape :
    // Case of two Gaussian atoms with large enough windows
    // TODO : We may have to optimize the windowSize switch
    if (atom2->windowShape == GaussWindowShape && atom1->windowSize > 16 && atom2->windowSize > 16)  {	
      CCAtomAnalyticGauss(atom1,atom2,pReal,pImag);
      return;
    }
    /* Numerical formula is better */
    CCAtomNumericInnerProduct(atom1,atom2,pReal,pImag); 
    return;    
  case FoFWindowShape :		
    // Case of two FoF atoms with large enough windows
    // TODO : We may have to optimize the windowSize switch
    /* ???? VOIR CE QUI EST PLUS RAPIDE ET JUSTE !! (PB FoF NUMERIQUE) */
    if (atom2->windowShape == FoFWindowShape && atom1->chirpId == 0.0 && atom2->chirpId == 0.0 &&
	atom1->windowSize > 32 && atom2->windowSize > 32)  {	
      CCAtomAnalyticFoF(atom1,atom2,pReal,pImag);  
      return;
    }
    /* Numerical formula */
    CCAtomNumericInnerProduct(atom1,atom2,pReal,pImag); 
    return;
  default :
    CCAtomNumericInnerProduct(atom1,atom2,pReal,pImag); 
    return;
  }
}

/* 
 * After going through this function, CheckAtom(atom) would generate an error because 
 * 'atom' does not satisfy 0<=freqId<=GABOR_NYQUIST_FREQID
 */
static void ConjugateAtom(ATOM atom) 
{
  CheckAtom(atom);
  atom->freqId = 2*GABOR_NYQUIST_FREQID-atom->freqId;
  if(atom->freqId >= 2*GABOR_NYQUIST_FREQID)
    atom->freqId -= 2*GABOR_NYQUIST_FREQID;
  atom->chirpId = -atom->chirpId;
}


/*************************************************************/
/* "Auto" inner-product of an atom with its conjugate.	     */
/*	<g_r+,g_r->					     */
/* The values of coeffR/coeffI/coeff2/realGG/imagGG/         */
/* cosPhase/sinPhase do not influence the result.            */
/*************************************************************/
void AutoAtomInnerProduct(const ATOM atom,char flagForceNumeric,float *pReal,float *pImag)
{
  static ATOM conjugate = NULL;

  CheckAtom(atom);

  conjugate = CopyAtom(atom,conjugate);
  ConjugateAtom(conjugate);
  /* Inner product of complex atom with its conjugate */
  CCAtomInnerProduct(atom,conjugate,flagForceNumeric,pReal,pImag);
  /* Fix the cases when the imaginary part is surely zero : 
   * it is the case for symmetric windows like GAUSSIAN  */
  /* TODO should simply test symmetry */
  if(atom->chirpId == 0.0 && GetFlagAsymWindowShape(atom->windowShape)==NO)
    *pImag = 0.0;
}

/*************************************************************************************/
/*
 * The inner product between two atoms.
 * (a REAL atom and a COMPLEX one)
 * The values of coeffR/coeffI/coeff2/realGG/imagGG/ do not influence the result.
 * Only the (cosPhase,sinPhase) of the first atom do.
 */
/****************************************************/
/*
 * <g_r,g_c>  =  <g'_r,g_c> / ||g'_r|| 
 *
 * where g'_r = 1/2(exp i phi <g'_r+,g_c> + exp -i phi <g'_r-,g_c>)
 * with g'_r+ and g'_r- complex atoms 
 * and  ||g'_r||^2 = 1/2 (1+1\Re{\exp{2 i \phi}<g'_r+,g'_r->})
 */
void RCAtomInnerProduct(const ATOM atomR,const ATOM atomC,char flagForceNumeric,float *pReal,float *pImag)
{
  static ATOM copyR = NULL;
  float cosPhase,sinPhase;
  long freqId;

  float x1,y1,x2,y2;
  float x,y;

  float realGG,imagGG;
  float cos2Phase,sin2Phase;
  
  float norm2;
  float factor;
  
  /* Checking */
  CheckAtomReal(atomR);
  CheckAtom(atomC);
  CheckTFContentCompat(atomR,atomC);

  /* Test if the supports intersect */
  if(AtomsIntersect(atomR,atomC,NULL,NULL)==NO)  {
    *pReal = *pImag = 0.0;
    return;
  }
  
  /* The inner-product before normalization 
   * <g'_r,g_c> = 1/2(exp i phi <g_r+,g_c> + exp -i phi <g_r-,g_c>) 
   */
  cosPhase = atomR->cosPhase;
  sinPhase = atomR->sinPhase;
  /* We must copy at least one of the two atoms to avoid bugs
   * in the conjugation process if atomR == atomC
   */
  copyR = CopyAtom(atomR,copyR);
  /* Inner product of the real (positive frequency) atom with the complex atom */
  CCAtomInnerProduct(copyR,atomC,flagForceNumeric,&x1,&y1);
  /* Multiply by \exp{i*phase}  */
  x = x1*cosPhase-y1*sinPhase;
  y = x1*sinPhase+y1*cosPhase;

  /* Taking the conjugate of the real atom */
  ConjugateAtom(copyR);
  /* Inner product of the real (negative frequency) atom with the complex atom */
  CCAtomInnerProduct(copyR,atomC,flagForceNumeric,&x2,&y2);
  /* Multiply by \exp{-i*phase} */
  x += x2*cosPhase+y2*sinPhase;
  y += x2*sinPhase-y2*cosPhase;

  /* Dividing by 2 */
  *pReal = .5*x;
  *pImag = .5*y;

  /* <g'_r+,g'_r-> */
  AutoAtomInnerProduct(atomR,flagForceNumeric,&realGG,&imagGG);
  
  /* ||g'_r||^2 = 1/2 (1+\Re{\exp{2 i \phi}<g'_r+,g'_r->}) */
  cos2Phase	= cosPhase*cosPhase-sinPhase*sinPhase;
  sin2Phase	= 2*sinPhase*cosPhase;
  norm2 	= (1+cos2Phase*realGG-sin2Phase*imagGG)/2;
  
  /* Normalizing */
  factor	= sqrt(norm2);
  *pReal 	/= factor;
  *pImag 	/= factor;

  /* Case where the complex atom is not modulated */
  freqId = (long)atomC->freqId;
  if((freqId == atomC->freqId) && (freqId%GABOR_NYQUIST_FREQID)==0 && atomC->chirpId == 0.0) *pImag = 0.0;


}


/*************************************************************************************/
/*
 * The inner product between two REAL atoms (if not, an error is generated).
 * The values of coeffR/coeffI/coeff2/realGG/imagGG/ do not influence the result.
 * Only the (cosPhase,sinPhase) of the two atoms do.
 */
/****************************************************/
/*
 * <g_rn1,g_rn2> = 1/2(e^-i\phi2 <g_rn1,g_r2+> + e^+i\phi2 <g_rn1,g_r2->)/||g_r2|| 
 *
 * where ||g_r2||^2 = 1/2(1+\Re{\exp 2i\phi2 <g_r2+,g_r2->
 */

void RRAtomInnerProduct(const ATOM atom1,const ATOM atom2,char flagForceNumeric,float *pReal)
{
  static ATOM copy2 = NULL;
  float cosPhase,sinPhase;
  
  float x1,y1,x2,y2;
  
  float realGG,imagGG;
  float cos2Phase,sin2Phase;

  float norm2;
  float factor;
    
  /* Checking */
  CheckAtomReal(atom1);
  CheckAtomReal(atom2);
  CheckTFContentCompat(atom1,atom2);
  
  /* Test if the supports intersect */
  if(AtomsIntersect(atom1,atom2,NULL,NULL)==NO)  {
    *pReal = 0.0;
    return;
  }

  /* The inner-product before normalization of the second atom, i.e. betweeen
   * the first real atom and the second complex atom
   */
  cosPhase = atom2->cosPhase;
  sinPhase = atom2->sinPhase;
 /* We must copy at least one of the two atoms to avoid bugs
   * in the conjugation process if atomR == atomC
   */
  copy2 = CopyAtom(atom2,copy2);
  RCAtomInnerProduct(atom1,copy2,flagForceNumeric,&x1,&y1);
  /* Multiplying by \exp{-i\phi2}, we only care about the real part because in the end the imaginary part will be zero */
  *pReal  = x1*cosPhase+y1*sinPhase;

  /* Now the second term with the conjugate of the complex atom */
  ConjugateAtom(copy2);
  RCAtomInnerProduct(atom1,copy2,flagForceNumeric,&x2,&y2);
  /* Multiplying by \exp{+i\phi2} and adding */
  *pReal += x2*cosPhase-y2*sinPhase;
  
 /* Dividing by 2 */
  *pReal /= 2;
  
  if(*pReal == 0.0)
    return;
  
  
  /* We normalize the second real atom */
  /* ||g_r2||^2 = 1/2 (1+\Re{\exp{2 i \phi2}<g_r2+,g_r2->}) */
  
  /* we use atom2 and not copy2 because we need
   * <g_r2+,g_r2-> and not <g_r2-,g_r2+>
   * (copy2 is now the conjugate of atom2) 
   */
  AutoAtomInnerProduct(atom2,flagForceNumeric,&realGG,&imagGG);
  
  cos2Phase	= cosPhase*cosPhase-sinPhase*sinPhase;
  sin2Phase	= 2*sinPhase*cosPhase;
  norm2 	= (1+cos2Phase*realGG-sin2Phase*imagGG)/2;
  
  /* Normalizing */
  factor	= sqrt(norm2);
  *pReal 	/= factor;
}

/*************************************************************************************/
/*
 * The inner product between a SIGNAL and a COMPLEX atom
 * (if not, an error is generated).
 * The values of coeffR/coeffI/coeff2/realGG/imagGG/cosPhase/sinPhase
 * of the atom do not influence the result.
 */
// If the frequency is either 0 or Nyquist the atom takes indeed real
// values and we garantee that the imaginary part of the inner product is zero.
/****************************************************/
void SCAtomInnerProduct(const SIGNAL signal,const ATOM atom,char borderType,float *pReal,float *pImag)
{
  float real,imag;

  static SIGNAL atomR = NULL;
  static SIGNAL atomI = NULL;
  static SIGNAL signalExtract = NULL;

  int timeIdMin,timeIdMax,i;

  // Checking 
  if(signal == NULL) Errorf("SCAtomInnerProduct : NULL signal");
  CheckAtom(atom);
  if(!BorderTypeIsOK(borderType)) Errorf("SCAtomInnerProduct : bad borderType '%d'",borderType);

  // Initialization
  real = imag = 0.0;
  /* Are the supports of the signal and the atom overlapping ? */
  ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,&timeIdMin,&timeIdMax);
  switch(borderType) {
  case BorderPad0 :      
    if ((timeIdMax < 0) || (timeIdMin > signal->size-1))
      return;
    break;
  default : 
    Errorf("SCAtomInnerProduct : borderType '%s' not treated yet",BorderType2Name(borderType));
    break;
  }

  /* Allocation (just once) */
  if(atomR == NULL) {
    atomR = NewSignal();
    atomI = NewSignal();
    signalExtract = NewSignal();
  }

  /* 
   * Build the atom with a unit norm in a complex signal.
   * The signal has exactly windowSize points.
   * The firstp point has a value of zero.
   */
  BuildAtom(atom,atomR,atomI,borderType,YES);

  /* 
   * Extract the corresponding piece of 'signal'
   * The first point must coincide with the first point
   * of the window of the atom, which is timeIdMin-1.
   */
  ExtractSig(signal,signalExtract,borderType,timeIdMin-1,atomR->size);

  /* Compute the real part of the inner product */
  for(i=0; i< atomR->size; i++) 
    real += signalExtract->Y[i]*atomR->Y[i];
  /* Compute the imaginary part if necessary (i.e. if nonzero) */
  if(atom->freqId != 0 && atom->freqId != GABOR_NYQUIST_FREQID) 
    for(i=0; i< atomR->size; i++) 
      imag -= signalExtract->Y[i]*atomI->Y[i];

  *pReal=real;
  *pImag=imag;
}


/*****************************************************************************/
/*                            TESTS  !!!!                                    */
/*****************************************************************************/

void C_Inner(char **argv)
{
    ATOM atom1=NULL,atom2=NULL;
    SIGNAL signal=NULL;
    char flagForceNumeric = NO;
    char *action;
    char *name;
    char opt;

    LISTV lv;
    /* Result */
    int borderType = STFT_DEFAULT_BORDERTYPE;
    float re,im;

    /* Parsing */
    argv = ParseArgv(argv,tWORD,&action,-1);

    /*
     * Complex-signal
     */    
    if(!strcmp(action,"sig")) {
      argv = ParseArgv(argv,tSIGNAL,&signal,tATOM,&atom1,-1);
      while( (opt = ParseOption(&argv))) {
	switch(opt) {
	case 'b' :
	  argv = ParseArgv(argv,tSTR,&name,-1);
	  borderType = Name2BorderType(name);
	  break;
	default :
	  ErrorOption(opt);
	}
      }
      NoMoreArgs(argv);
      SCAtomInnerProduct(signal,atom1,borderType,&re,&im);
      lv = TNewListv();
      AppendFloat2Listv(lv,re);
      AppendFloat2Listv(lv,im);
      SetResultValue(lv);
      return;
    }

    /*
     * "Auto" inner-product with the conjugate 
     */
    if(!strcmp(action,"auto")) {
      argv = ParseArgv(argv,tATOM,&atom1,0);
      AutoAtomInnerProduct(atom1,NO,&re,&im);
      lv = TNewListv();
      AppendFloat2Listv(lv,re);
      AppendFloat2Listv(lv,im);
      SetResultValue(lv);
      return;
    }
    
    /*
     * Inner product between two atoms 
     */
    argv = ParseArgv(argv,tATOM,&atom1,tATOM,&atom2,-1);
    while( (opt = ParseOption(&argv))) {
      switch(opt) {
      case 'n' :
	flagForceNumeric = YES;
	NoMoreArgs(argv);
	break;
      default :
	ErrorOption(opt);
      }
    }
    /*
     * Complex-Complex
     */
    if(!strcmp(action,"cc"))  {
      CheckTFContentCompat(atom1,atom2);
      CCAtomInnerProduct(atom1,atom2,flagForceNumeric,&re,&im);
      lv = TNewListv();
      AppendFloat2Listv(lv,re);
      AppendFloat2Listv(lv,im);
      SetResultValue(lv);
      return;
    }
    /* 
     * Real-complex 
     */
    if(!strcmp(action,"rc")) {
      RCAtomInnerProduct(atom1,atom2,flagForceNumeric,&re,&im);
      lv = TNewListv();
      AppendFloat2Listv(lv,re);
      AppendFloat2Listv(lv,im);
      SetResultValue(lv);
      return;
    }
    /*
     * Real-real normalized
     */
    if(!strcmp(action,"rr")) {
      RRAtomInnerProduct(atom1,atom2,flagForceNumeric,&re);
      SetResultFloat(re);
      return;
    }
    Printf("Unknown action '%s'",action);
    ErrorUsage();
}

/* EOF */

