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

#include "mp_book.h"

//#define DEBUG_SCALE 1


/*******************************************************/
/*
 *	FUNCTIONS WHICH PERFORM AN INTERPOLATION
 *	IN ORDER TO ADJUST THE FIELDS OF A GIVEN ATOM
 */
/*******************************************************/

/* 
 * For timeId/freqId interpolation
 * (resolution = 0  => integers) 
 * (resolution > 0  => precision = pow(2,-resolution)
 * (resolution < 0  => infinite precision)
 */
static float AtomTimeResolution  =  0.0;
static float AtomFreqResolution  = -1.0;
/*
 * For chirp interpolation 
 * (resolution < 0  => infinite precision)
 * (resolution >= 0  => precision = pow(2,-resolution)/scale^2 where 's' = scale = TODO : precise!
 */
static float AtomChirpResolution = -1.0;

static float QuantizeParameter(float parameter,float resolution)
{
  float precision;
  if (resolution >= 0) {
    precision = pow(2.0,-(float) resolution);
    if (parameter >= 0) return(((int) (parameter/precision+0.5))*precision);
    else          return(((int) (parameter/precision-0.5))*precision);
  }
  return(parameter);
}

/*
 * The various types of parameters that can be estimated
 */
enum {
  TimeParameter,
  FreqParameter,
  ChirpParameter,
  ScaleParameter
};

/*
 * Fits a parabola y(x) = (a/2)*k^2+b*k+c with x = k*rate
 * If the parabola is indeed a line, an error is generated.
 *
 * Input  : 3 points (-rate,yM) (0,yC) (rate,yP) and 'rate'
 * Output : parameters a,b,c and local extremum x.
 */
static void ParabolaInterpolate(float yM,float yC,float yP,float rate,float *pA, float *pB, float *pC,float *pX)
{
  float a,b,c;
  /* The parameters of the parabola */
  *pA = a = (yP-2*yC+yM);
  *pB = b = (yP-yM)/2;
  *pC = c = yC;
  /* The parabola should not be a line ! */
  if(a == 0.0) Errorf("ParabolaInterpolate : second derivative is zero");
  /* The location of the extremum */
  *pX = -rate*b/a;
}

/* 
 * Same as above, but generates an error if the parabola (or flat line)
 * has no maximum strictly within the interval [-rate,rate]
 */
static void ParabolaInterpolateMax(float yM,float yC,float yP,float rate,float *pX)
{
  float a,b,c;
  /* Checking that the maximum will be within the range [-rate,rate] */
  if (yC < yM || yP > yC) Errorf("ParabolaInterpolateMax : no strict maximum with data %g %g %g\n",yM,yC,yP);
  /* Case of a flat line */
  if (yC == yM && yC == yP) {
    *pX = 0.0;
    return;
  }
  ParabolaInterpolate(yM,yC,yP,rate,&a,&b,&c,pX);
}

/*
 * Simple conversion of a complex number from cartesian coordinates z= real+i*imag
 * to 'logpolar' coordinates   z=exp(logAmp+i*arg)  with -M_PI/2 <= ang <= 3*M+PI/2
 */
static void Cartesian2LogPolar(float re,float im,float *logAmp,float *arg)
{
  /* Cannot deal with z==0 */
  if(re==0.0&&im==0.0)   Errorf("Cartesian2LogPolar : (Weird) (re,im) = (0,0) !");

  *logAmp = 0.5*log(re*re+im*im);
  /* Case of pure imaginary number */
  if(re == 0.0) {
    if(im > 0) *arg = M_PI/2;
    else       *arg = -M_PI/2;
  }
  /* Other cases */
  else {
    *arg = atan2(im,re);
    if(re < 0.0) {
      *arg += M_PI;
    }
  }
}  

/*
 * Simple function that gets the (complex or real) values of the inner product 
 * of three 'neighboring' time-frequency atoms.
 * Arguments : 
 *  atom     : the atom which neighbors we look at
 *  dict     : the dictionary used to get/compute the values
 *  channel  : the channel of the dictionary we should use
 *  type     : either ComplexStft or RealStft
 *  neighbor : either TIME_NEIGHBOR or FREQ_NEIGHBOR
 * Output    :
 *  the real (and imaginary) parts of the three inner products
 *  the distance 'rate' between neighbors (in timeId or freqId units)
 * Return    :
 *  'YES' in most cases
 *  'NO'  iff one of the neighbors 
 *  -is strictly beyond the Nyquist frequency OR
 *  -is strictly below the Zero frequency     OR
 *  -has support not completely inside [0,signalSize-1]
 */ 
#define TIME_NEIGHBOR 0
#define FREQ_NEIGHBOR 1
static char GetNeighborsData(ATOM atom,DICT dict,unsigned char channel,char type,char neighbor,unsigned long *pRate,
			     float *coeffRM,float *coeffRC,float *coeffRP,
			     float *coeffIM,float *coeffIC,float *coeffIP)
{
  char flagRecompute = NO;
  SUBDICT subDict = NULL;
  STFT    stft    = NULL;
  SIGNAL  signal  = NULL;
  
  /* Atom fields */
  long timeId;
  long freqId;
  /* Support of the atom */
  int timeIdMin,timeIdMax;

  /* Utilities */
  long tM,tC,tP,fM,fC,fP;
  float *pArrayRM,*pArrayRC,*pArrayRP;
  float *pArrayIM,*pArrayIC,*pArrayIP;
  static ATOM atomM = NULL,atomC = NULL,atomP = NULL;
  
  timeId = atom->timeId;
  freqId = atom->freqId;

  if((subDict = GetStftSubDict(dict,channel,type,atom->windowShape,atom->windowSize,NULL))==NULL)
    Errorf("GetNeighborsData : stft is not available at windowSize=%d windowShape='%s'",atom->windowSize,WindowShape2Name(atom->windowShape));
  stft = (STFT)(subDict->dataContainer);


  /* Checking the range */
  CheckAtom(atom);
  if(timeId > stft->tRate*(stft->nFrames-1))
    Errorf("GetNeighborsData (Weired) : bad timeId %g > %d",timeId,stft->tRate*(stft->nFrames-1));
  if(freqId>GABOR_NYQUIST_FREQID)
    Errorf("GetNeighborsData (Weired) : bad freqId %g > %d",freqId,GABOR_NYQUIST_FREQID);

  /* Cases where we have to compute the inner products */
  if(atom->chirpId != 0.0)
    flagRecompute = YES;   /* the atom is chirped */
  if(subDict->flagUpToDate == NO) {
    flagRecompute = YES;   /* the content of the stft is out of date */
  } else {
    if(((float)freqId)!=atom->freqId || freqId%stft->fRate != 0)
      flagRecompute = YES; /* the frequency is not on the grid of the stft */
    if(((float)timeId)!=atom->timeId || timeId%stft->tRate != 0)
      flagRecompute = YES; /* the time is not on the grid of the stft */
  }

  /* Setting the neighbors we want */
  if(flagRecompute == NO) {
    /* Case when we get the coeffs from the stft */
    switch(neighbor) {
    case FREQ_NEIGHBOR :
      tM  = tC = tP = timeId;
      fM  = freqId-stft->fRate;
      fC  = freqId;
      fP  = freqId+stft->fRate;
      *pRate = stft->fRate;
      /* Cases where the neighbors are 'out' */
      if(fM<0 || fP>GABOR_NYQUIST_FREQID) return(NO);    
      break;
    case TIME_NEIGHBOR :
      tM  = timeId-stft->tRate;
      tC  = timeId;
      tP  = timeId+stft->tRate;
      fM  = fC = fP = freqId;
      *pRate = stft->tRate;
      /* Cases where the neighbors are 'out' */
      ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,&timeIdMin,&timeIdMax);
      if(timeIdMin-stft->tRate<0 || timeIdMax+stft->tRate>=atom->signalSize) return(NO);    
      break;
    default :
      Errorf("GetNeighborsData : Weired error (neighbor type=%d)",neighbor);
      break;
    }
  } else {
    /* Case when we compute the coeffs */
    atomM = CopyAtom(atom,atomM);
    atomC = CopyAtom(atom,atomC);
    atomP = CopyAtom(atom,atomP);
    /* 
     * This ensures compliance of atomM and atomP with CheckAtom in SCAtomInnerProduct
     * if atomM->freqId==0 or atomP->freqId==GABOR_NYQUIST_FREQID
     */
    atomM->coeffI = atomP->coeffI = 0.0; 
    switch(neighbor) {
    case FREQ_NEIGHBOR :
      atomM->freqId -= stft->fRate;
      atomP->freqId += stft->fRate;
      *pRate = stft->fRate;
      /* Cases where the neighbors are 'out' */
      if(atomM->freqId<0 || atomP->freqId>GABOR_NYQUIST_FREQID)  return(NO);    
      break;
    case TIME_NEIGHBOR :
      atomM->timeId -= stft->tRate;
      atomP->timeId += stft->tRate;
      *pRate = stft->tRate;
      /* Cases where the neighbors are 'out' */
      ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,&timeIdMin,&timeIdMax);
      if(timeIdMin-stft->tRate<0 || timeIdMax+stft->tRate>=atom->signalSize) return(NO);    
      break;
    default : 
      Errorf("GetNeighborsData : Weired error (neighbor type=%d)",neighbor);
      break;
    }
  }    

  /* Actually get/compute the coeffs */
  if(flagRecompute == NO) {
    switch(type) {
    case ComplexStft :
      GetStftData(stft,tM,&pArrayRM,&pArrayIM);
      GetStftData(stft,tC,&pArrayRC,&pArrayIC);
      GetStftData(stft,tP,&pArrayRP,&pArrayIP);
      *coeffRM = pArrayRM[fM/stft->fRate];    
      *coeffRC = pArrayRC[fC/stft->fRate];
      *coeffRP = pArrayRP[fP/stft->fRate];    
      *coeffIM = pArrayIM[fM/stft->fRate];    
      *coeffIC = pArrayIC[fC/stft->fRate];
      *coeffIP = pArrayIP[fP/stft->fRate];    
      break;
    case RealStft : 
      GetStftData(stft,tM,&pArrayRM,NULL);
      GetStftData(stft,tC,&pArrayRC,NULL);
      GetStftData(stft,tP,&pArrayRP,NULL);
      *coeffRM = pArrayRM[fM/stft->fRate];    
      *coeffRC = pArrayRC[fC/stft->fRate];
      *coeffRP = pArrayRP[fP/stft->fRate];    
      break;
    default :
      Errorf("GetNeighborsData : Weired error (stft type)");
      break;
    }
  }
  else {
    signal = GetChannel(dict,channel);
    SCAtomInnerProduct(signal,atomM,stft->borderType,&(atomM->coeffR),&(atomM->coeffI));
    SCAtomInnerProduct(signal,atomC,stft->borderType,&(atomC->coeffR),&(atomC->coeffI));
    SCAtomInnerProduct(signal,atomP,stft->borderType,&(atomP->coeffR),&(atomP->coeffI));
    switch(type) {
    case ComplexStft :
      *coeffRM = atomM->coeffR;
      *coeffRC = atomC->coeffR;
      *coeffRP = atomP->coeffR;
      *coeffIM = atomM->coeffI;
      *coeffIC = atomC->coeffI;
      *coeffIP = atomP->coeffI;
      break;
    case RealStft :
      CastAtomReal(atomM);
      CastAtomReal(atomC);
      CastAtomReal(atomP);
      *coeffRM = atomM->coeff2;
      *coeffRC = atomC->coeff2;
      *coeffRP = atomP->coeff2;
      break;
    default :
      Errorf("GetNeighborsData : Weired error (stft type)");
      break;
    }
  }
  return(YES);
}

/*
 * The main procedure to estimate the chirpId and windowSize of a chirplet
 * using parabolic interpolation of the second derivatives of the phase and log-amplitude 
 * of a spectrogram. Cf the article
 * "Fast Matching Pursuit with a multiscale dictionary of Gaussian chirps",
 * IEEE Trans. Signal Proc., Vol. 49, No. 5, May 2001
 * Input  : 
 * Output :
 * Return :
 *   YES (in most cases) : the estimation was successful and compatible with the underlying model
 *   NO if the model was not compatible with the data (the content of the output is unspecified)
 */
static char EstimateChirp(const ATOM atom,float aLogAmp,float aPhiWrapped,unsigned long rate,float *pChirpId,float *pWindowSize)
{
  float scale,scale2,binSize;
  float der2LogAmp,der2Phi;
  int nMin,nMax;

  float estimChirpId;

  // TODO : Errorf or what ?
  if(atom->windowShape != GaussWindowShape) {
    Warningf("EstimateChirp : only valid for Gaussian windows, not '%s'",WindowShape2Name(atom->windowShape));
  }

  scale   = atom->windowSize;
  scale2  = theGaussianSigma2*scale*scale;
  binSize = (2*M_PI*rate)/GABOR_MAX_FREQID;
  if(scale2 >= 2*M_PI/(binSize*binSize))  
    Errorf("EstimateChirp (Weired) : multiple unwrapping of the phase may be possible");

  der2LogAmp     = aLogAmp/(binSize*binSize);
  /* Testing if the 'ridge' model is valid : see Eq. (18) in the paper */
  if(der2LogAmp >= 0 || der2LogAmp < -scale2) {
    //Warningf("EstimateChirp : model is incompatible with logAmp");
    return(NO);
  }

  /* 
   * 'Unwrapping' the phase : given the second derivative der2Phi of the phase of a spectrogram along frequency, we compute
   * the value of 'der2Phi+2*n*pi/(binSize*binSize)' which is compatible with the chirp model : see Eq. (17) in paper.
   * If no such value is compatible, then the model does not allow an estimate.
   */
  der2Phi = aPhiWrapped/(binSize*binSize);
  if(fabs(der2Phi) > scale2/2) {
    nMin = ceil ((-der2Phi-scale2/2)*binSize*binSize/(2*M_PI));
    nMax = floor((-der2Phi+scale2/2)*binSize*binSize/(2*M_PI));
    /* Testing if the 'ridge' model is valid : is there at least one compatible unwrapping ? */
    if(nMin != nMax) {
      //Warningf("EstimateChirp : model is incompatible with phase unwrapping [%d %d]",nMin,nMax);
      return(NO);
    } 
    der2Phi += 2*M_PI*nMin/(binSize*binSize);
  }

  /* The estimation */
  estimChirpId       = -der2Phi/(der2LogAmp*der2LogAmp+der2Phi+der2Phi);
  *pChirpId    = estimChirpId*GABOR_MAX_FREQID/(2*M_PI);

#ifdef DEBUG_SCALE
  estimInverseScale2 = -(1/scale2)-der2LogAmp/(der2LogAmp*der2LogAmp+der2Phi*der2Phi);
  // DEBUG/INFO for future estimate ??
  if(estimInverseScale2 <= 0.0)
    Warningf("EstimateChirp : estimated scale2=1/(%g) is negative or infinite !",estimInverseScale2);
  estimScale2        = 1/estimInverseScale2;

  *pWindowSize = sqrt(estimScale2/theGaussianSigma2);
  // DEBUG
  if(estimInverseScale2 > 0.0)
    Printf("s = %g -> %d ",*pWindowSize,1<<((int)(0.5+(log(*pWindowSize)/log(2.0)))));
#endif
  return(YES);
}

/*
 * Estimation of the fields of an atom using three neighbors and interpolation.
 * Output : one or more estimated parameters, depending on paramType.
 * Return : YES if the estimation was successful, NO in the other case (the content of the estimated parameters is then unspecified)
 */
static char EstimateAtomParameter(ATOM atom,DICT dict,unsigned char channel,unsigned char paramType,float *pParamVal,float *pAuxParamVal)
{
  unsigned long rate;
  float coeff2M,coeff2C,coeff2P;
  float realM,realC,realP,imagM,imagC,imagP;

  float phaseM,phaseC,phaseP;
  float logAmpM,logAmpC,logAmpP;

  float aPhi,aLogAmp,b,c;
  float kPhi;


  float dTimeId,dFreqId,dChirpId,windowSize;
  float freqId,chirpId;

  /* Checking */
  CheckAtom(atom);

  switch(paramType) {
    /***************************************/
    /* Interpolation on the time parameter */
    /***************************************/
  case TimeParameter :
    /* We get the value on three points to interpolate */
    if(GetNeighborsData(atom,dict,channel,RealStft,TIME_NEIGHBOR,&rate,&coeff2M,&coeff2C,&coeff2P,NULL,NULL,NULL)==NO) {
      //Warningf("TimeInterpolation : no neighbor available");
      return(NO);
    }
    if (coeff2C <= coeff2M || coeff2C <= coeff2P) { 
      //Warningf("TimeInterpolation : not a strict maximum %g %g %g!",coeff2M,coeff2C,coeff2P);
      return(NO);
    } 
    ParabolaInterpolateMax(coeff2M,coeff2C,coeff2P,rate,&dTimeId); 
    *pParamVal = atom->timeId + dTimeId;
    return(YES);
    /***************************************/
    /* Interpolation on the freq parameter */
    /***************************************/
  case FreqParameter :
    /* We get the value on three points to interpolate */
    if(GetNeighborsData(atom,dict,channel,RealStft,FREQ_NEIGHBOR,&rate,&coeff2M,&coeff2C,&coeff2P,NULL,NULL,NULL)==NO) {
      //Warningf("FreqInterpolation : no neighbor available");
      return(NO);
    }
    if (coeff2C <= coeff2M || coeff2C <= coeff2P) { 
      //Warningf("FreqInterpolation : not a strict maximum %g %g %g!",coeff2M,coeff2C,coeff2P);
      return(NO);
    }
    ParabolaInterpolateMax(coeff2M,coeff2C,coeff2P,rate,&dFreqId);
    *pParamVal = atom->freqId + dFreqId;
    return(YES);
    /***************************** **********/
    /* Interpolation on the chirp parameter */
    /****************************** *********/
  case ChirpParameter :
    /* TODO : enable GetNeihborsData from already chirped atom */
    if(GetNeighborsData(atom,dict,channel,ComplexStft,FREQ_NEIGHBOR,&rate,&realM,&realC,&realP,&imagM,&imagC,&imagP)==NO) {
      //Warningf("ChirpInterpolation : no neighbor available");
      return(NO);
    }
    /* Conversion to log-amplitude and phase */
    Cartesian2LogPolar(realM,imagM,&logAmpM,&phaseM);
    Cartesian2LogPolar(realC,imagC,&logAmpC,&phaseC);
    Cartesian2LogPolar(realP,imagP,&logAmpP,&phaseP);
    ParabolaInterpolate(logAmpM,logAmpC,logAmpP,rate,&aLogAmp,&b,&c,&dFreqId);
    ParabolaInterpolate(phaseM,phaseC,phaseP,rate,&aPhi,&b,&c,&kPhi);
    /* Perform estimation based on the interpolated parabolas */
    if(EstimateChirp(atom,aLogAmp,aPhi,rate,&dChirpId,&windowSize)==NO) {
      //Warningf("ChirpInterpolation : incompatible model");
      return(NO);
    }
    freqId  = atom->freqId +dFreqId;
    chirpId = atom->chirpId+dChirpId;
    /* Check whether the estimated chirplet is aliased */
    if(!INRANGE(0,freqId-chirpId*atom->windowSize/2,GABOR_NYQUIST_FREQID) 
       || !INRANGE(0,freqId+chirpId*atom->windowSize/2,GABOR_NYQUIST_FREQID)) {
      //Warningf("ChirpInterpolation : the estimated chirplet would be aliased");
      return(NO);
    }
    *pAuxParamVal = freqId;
    *pParamVal    = chirpId;
    return(YES);
  default :
    Errorf("EstimateAtomParameter (Weired) : bad parameter type %d",paramType);
  }
}


/* Performs a series of global optimizations on a molecule, using a dictionary. */
void OptimizeMolecule(MOLECULE molecule,DICT dict,LISTV optimizations)
{
  unsigned short k;
  unsigned char channel;
  ATOM atom;

  /* Optimizations options */
  unsigned short i;
  char *type=NULL;
  VALUE value=NULL;
  float f;
  STRVALUE str=NULL;
  LISTV  lv=NULL;
  char* rangeName=NULL;
  RANGE range=NULL;
  
  // TODO : get rid of that ?
  char borderType      = BorderPad0;
  float coeffR,coeffI;
  
  SIGNAL signal = NULL;
  
  char flagTouched = NO;
  unsigned int nAtoms,nAtomsAux;
  float timeId,freqId,chirpId;
  float meanTimeId,meanFreqId,meanFreq0Id,meanFreqKId,meanChirpId;
  
  if(optimizations==NULL) return;
  
  /* Now, perform the optimizations that are asked for */
  i=0;
  while(i<optimizations->length) {
    type = GetListvNth(optimizations,i,&value,&f);
    // All optimizations options so far are &strings
    if(type!=strType) 
      Errorf("OptimizeMolecule : Weired");

    str = CastValue(value,STRVALUE);
    /* 
     * Interpolation of the time : jointly across partials and channels
     */
    if(!strcmp(str->str,"time")) {
      /* First, estimate the average time over partials and channels */
      meanTimeId = 0.0;
      nAtoms = 0;
      for(k = 0; k < molecule->dim; k++) {
	channel = 0;
	//    for(channel = 0; channel < molecule->nChannels; channel++) {
	atom = GetMoleculeAtom(molecule,channel,k);
	if(EstimateAtomParameter(atom,dict,channel,TimeParameter,&timeId,NULL)) {
	  meanTimeId+=timeId;
	  nAtoms++;
	} 
      }
      /* Then, set the new time of all these atoms */
      if(nAtoms>0) {
	meanTimeId /= nAtoms;
	meanTimeId  = QuantizeParameter(meanTimeId,AtomTimeResolution);
	for(k = 0; k < molecule->dim; k++) {
	  channel = 0;
	  //    for(channel = 0; channel < molecule->nChannels; channel++) {
	  atom = GetMoleculeAtom(molecule,channel,k);
	  atom->timeId = meanTimeId;
	  flagTouched = YES;
	}
      }
      i++;continue;
    }
    /*
     * Interpolation of the frequency : jointly over channels
     * for each partial
     */
    if(!strcmp(str->str,"freq")) {
      /* The estimate is different for each partial */
      for(k = 0; k < molecule->dim; k++) {
	/* First, estimate over the channels for the current partial */
	meanFreqId = 0.0;
	nAtoms     = 0;
	channel    = 0;
	//    for(channel = 0; channel < molecule->nChannels; channel++) {
	atom = GetMoleculeAtom(molecule,channel,k);
	if(EstimateAtomParameter(atom,dict,channel,FreqParameter,&freqId,NULL)) {
	  meanFreqId+=freqId;
	  nAtoms++;
	}  
	/* Then. set the new frequency for all channels */
	if(nAtoms>0) {
	  meanFreqId /= nAtoms;
	  meanFreqId  = QuantizeParameter(meanFreqId,AtomFreqResolution);
	  channel = 0;
	  //    for(channel = 0; channel < molecule->nChannels; channel++) {
	  atom = GetMoleculeAtom(molecule,channel,k);
	  atom->freqId = meanFreqId;
	  flagTouched = YES;
	}
      }
      i++;continue;
    }	
    /*
     * Interpolation of the chirp : jointly across partials and channels
     * the frequency is estimated jointly across channels for each partial
     */
    if(!strcmp(str->str,"chirp")) {
      /* First, estimate the average chirp over partials and channels */
      meanChirpId = 0.0;
      nAtoms = 0;
      for(k = 0; k < molecule->dim; k++) {
	/* Estimate the average frequency over channels for the current partial */
	meanFreqId = 0.0;
	nAtomsAux = 0;
	channel = 0;
	//    for(channel = 0; channel < molecule->nChannels; channel++) {
	atom = GetMoleculeAtom(molecule,channel,k);
	if(EstimateAtomParameter(atom,dict,channel,ChirpParameter,&chirpId,&freqId)) {
	  meanChirpId += chirpId;
	  meanFreqId  += freqId;
	  nAtoms++;
	  nAtomsAux++;
	} 
	
	/* Then, set the new frequency of all channels of the current partial */
	if(nAtomsAux>0) {
	  meanFreqId /= nAtomsAux;
	  meanFreqId  = QuantizeParameter(meanFreqId,AtomFreqResolution);
	  channel = 0;
	  //    for(channel = 0; channel < molecule->nChannels; channel++) {
	  atom = GetMoleculeAtom(molecule,channel,k);
	  atom->freqId = meanFreqId;
	  flagTouched = YES;
	}
      }
      
      /* Eventually, set the new chirp of all channels of all partials */
      if(nAtoms>0) {
	meanChirpId /= nAtoms;
	meanChirpId  = QuantizeParameter(meanChirpId,AtomChirpResolution);
	for(k = 0; k < molecule->dim; k++) {
	  channel = 0;
	  //    for(channel = 0; channel < molecule->nChannels; channel++) {
	  atom = GetMoleculeAtom(molecule,channel,k);
	  /* The first partial */
	  if(k == 0) {
	    meanFreq0Id   = atom->freqId;
	    atom->chirpId = meanChirpId;
	  } else {
	    /* The other partials */
	    if(meanFreq0Id==0.0) {
	      Warningf("meanFreq0Id==%g",meanFreq0Id);
	      atom->chirpId = meanChirpId*(k+1);
	    } else {
	      meanFreqKId = atom->freqId;
	      atom->chirpId = meanChirpId*meanFreqKId/meanFreq0Id;
	    }
	  }
	  flagTouched = YES;
	}
      }
      i++;continue;
    } 
    
    /* 
     * Recompute all the coefficients
     */
    if(!strcmp(str->str,"recompute")) {
      flagTouched = YES;
      i++;continue;
    }

    // DEBUG : shall we do that or just continue ?
    Errorf("OptimizeMolecule : unknown optimization '%s'",str->str);  
  }

  /* Actually recompute all coefficients */
  molecule->coeff2 = 0.0;
  for(k = 0; k < molecule->dim; k++) {
    channel = 0;
    //    for(channel = 0; channel < molecule->nChannels; channel++) {
    atom = GetMoleculeAtom(molecule,channel,k);
    signal = GetChannel(dict,channel);
    SCAtomInnerProduct(signal,atom,borderType,&coeffR,&coeffI);
    atom->coeffR = coeffR;
    atom->coeffI = coeffI;
    CastAtomReal(atom);
    molecule->coeff2 += atom->coeff2;
  } 
}

// OLD STUFF
/* We increase the octave as long as it increases the coeff2 */
static void EstimateScale(ATOM atom,DICT dict,unsigned char channel)
{
  SUBDICT subDict = NULL;
  STFT    stft    = NULL;
  SIGNAL  signal  = NULL;
  // TODO : other border types
  char borderType = BorderPad0;
  ATOM copy = NULL;
  int atomTimeIdMin,atomTimeIdMax;
  /* Checking */
  CheckAtom(atom);

  signal = GetChannel(dict,channel);
  /* First we recompute the inner product */
  SCAtomInnerProduct(signal,atom,borderType,&(atom->coeffR),&(atom->coeffI));
  CastAtomReal(atom);

  copy = CopyAtom(atom,copy);
  /*
   * We try to increase the size of the atom as long as 
   * -it is possible
   * -it increases its coeff2
   */
  while(1) {
    /* Case where the window size is too big */
    if(copy->windowSize > STFT_MAX_WINDOWSIZE) break;
    ComputeWindowSupport(copy->windowSize,copy->windowShape,copy->timeId,&atomTimeIdMin,&atomTimeIdMax);
    if(atomTimeIdMin < 0 || atomTimeIdMax >= dict->signalSize) break;
    /* Case where the window size is not available */
    if((subDict = GetStftSubDict(dict,channel,ComplexStft,copy->windowShape,copy->windowSize,NULL))==NULL) break;

    stft  = (STFT)subDict->dataContainer;
    // TODO : use flagCausal ?
    if(!INRANGE(stft->firstp,copy->timeId,stft->lastp)) break;
    /* Compute the coeff2 */
    SCAtomInnerProduct(GetChannel(dict,channel),copy,borderType,&(copy->coeffR),&(copy->coeffI));
    CastAtomReal(copy);
    /* If increasing the windowSize 'increased' the coeff2, we shall try increasing again */
    if(copy->coeff2 >= atom->coeff2) {
      atom = CopyAtom(copy,atom);
    }
    copy->windowSize *= 2;
  }
}

/* Performs a series of optimizations on an atom/a molecule, using a dictionary. */
static void OptimizeAtom(ATOM atom,DICT dict,LISTV optimizations) 
{
  /* Optimizations options */
  unsigned short i;
  char *type=NULL;
  VALUE value=NULL;
  float f;
  STRVALUE str=NULL;
  LISTV  lv=NULL;
  char* rangeName=NULL;
  RANGE range=NULL;

  unsigned char channel;// TODO : pass as an argument of OptimizeAtom!
  // TODO : get rid of that ?
  char borderType      = BorderPad0;
  float coeffR,coeffI;

  SIGNAL signal = NULL;
  static ATOM copy = NULL;

  float timeId,freqId,chirpId;

  if(dict->nChannels > 1) Errorf("OptimizeAtom : cannot deal with multichannel dictionaries");

  if(optimizations==NULL) return;
  /* Now, perform the optimizations that are asked for */
  // NOTE THAT THIS WILL CHANGE A LOT IN THE NEAR FUTURE!
  i=0;
  while(i<optimizations->length) {
    type = GetListvNth(optimizations,i,&value,&f);
    // All optimizations options so far are &strings
    if(type==strType) {
      str = CastValue(value,STRVALUE);
      /* Interpolation in Time and Frequency if asked */
      if(!strcmp(str->str,"freqtime")) {
	// MEANINGLESS!!!
	channel = 0;
	// TODO : when result is NO, take it into account ????
	if(EstimateAtomParameter(atom,dict,channel,FreqParameter,&freqId,NULL)==NO) freqId = atom->freqId;
	if(EstimateAtomParameter(atom,dict,channel,TimeParameter,&timeId,NULL)==NO) timeId = atom->timeId;
	// TODO : move this to another task
	timeId = QuantizeParameter(timeId,AtomTimeResolution);
	freqId = QuantizeParameter(freqId,AtomFreqResolution);

	/* Set the interpolated time and frequency */
	signal = GetChannel(dict,channel);
	copy = CopyAtom(atom,copy);
	copy->freqId = freqId; 
	copy->timeId = timeId;
	/* 
	 * This ensures compliance of copy with CheckAtom in SCAtomInnerProduct
	 * if copy->freqId==0 or copy->freqId==GABOR_NYQUIST_FREQID
	 */
	copy->coeffI = 0.0; 
	/* Recompute the inner-product */
	// TODO : move from here ??
	SCAtomInnerProduct(signal,copy,borderType,&(copy->coeffR),&(copy->coeffI));
	CastAtomReal(copy);
	if (copy->coeff2 > atom->coeff2) 
	  atom = CopyAtom(copy,atom);
	else {
	  // DEBUG
	  Warningf("Bad Interpolation");
	  PrintAtom(atom,NO);
	  PrintAtom(copy,NO);
	  SCAtomInnerProduct(signal,atom,borderType,&(atom->coeffR),&(atom->coeffI));  
	  CastAtomReal(atom);
	}
	i++;continue;
      } 
      /* Interpolation in Frequency and Chirp (and scale) if asked */
      if(!strcmp(str->str,"chirp")) {
	// MEANINGLESS!!!
	channel = 0;
	// TODO : when result is NO, take it into account
	// WARNING : this modifies atom->freqId too !
	if(EstimateAtomParameter(atom,dict,channel,ChirpParameter,&chirpId,&freqId)==NO) {
	  freqId  = atom->freqId;
	  chirpId = atom->chirpId;
	}
	QuantizeParameter(freqId,AtomFreqResolution);
	QuantizeParameter(chirpId,AtomChirpResolution);
	atom->freqId  = freqId;
	atom->chirpId = chirpId;
	// TODO : separate chirp/scale
	// DEBUG
#ifdef DEBUG_SCALE
	Printf("s : %d ",atom->windowSize);
#endif
	EstimateScale(atom,dict,channel);
#ifdef DEBUG_SCALE
	// DEBUG
	Printf("-> %d\n",atom->windowSize);
#endif
	i++;continue;
      } 

      /* Recompute the coefficient if asked */
      // TODO : do that automatically exactly when needed without asking ?
      if(!strcmp(str->str,"recompute")) {
	// MEANINGLESS!!!
	channel = 0;
	signal = GetChannel(dict,channel);
	SCAtomInnerProduct(signal,atom,borderType,&coeffR,&coeffI);
	atom->coeffR = coeffR;
	atom->coeffI = coeffI;
      } 
    }
    i++;continue;
  }

  CastAtomReal(atom);
}

void OldOptimizeMolecule(MOLECULE molecule,DICT dict,LISTV optimizations) 
{
  unsigned short k;
  unsigned char channel = 0;
  ATOM atom;

  if(optimizations==NULL) return;
  // TODO : joint optimization over all harmonics (for chirp of harmonic atoms ...)
  // TODO : joint optimization over channels

  // Optimize and recompute the molecule->coeff2
  molecule->coeff2 = 0.0;
  for(k = 0; k < molecule->dim; k++) {
    atom = GetMoleculeAtom(molecule,channel,k);
    OptimizeAtom(atom,dict,optimizations);
    molecule->coeff2 += atom->coeff2;
  }
}

/* EOF */

