/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'mp' 2.0                           */
/*                                                                          */
/*      Copyright (C) 2002 Remi Gribonval.                                  */
/*      email  : remi.gribonval@inria.fr                                    */
/*      email  : lastwave@cmap.polytechnique.fr                             */
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

extern SUBDICT NewSubDict();
extern SUBDICT PrivateAddSubDict(DICT dict,SUBDICT subDict);
/************************************************************************/
/*
 *    This part is specific to STFT sub-dictionaries
 * TODO : help on these functions
 */
/************************************************************************/

/*
 * Setting/Getting a STFT sub-dictionary
 */

// Get a sub-dictionary that contains a stft with given {type,windowShape,windowSize}
// If none exists, return NULL.
// TODO : add a listv parameter that will contain Harmo/HighRes additional parameters
SUBDICT GetStftSubDict(DICT dict,unsigned char channel,char type,char windowShape,unsigned long windowSize,LISTV options) 
{
  unsigned short i;
  SUBDICT subDict;
  STFT stft;
  // Basic checking
  if(options!=NULL && type!=HarmoStft && type!=HighResStft) 
    Errorf("GetStftSubDict : non NULL options for '%s' &stft",StftType2Name(type));

  /* Does such a subDict exist ? */
  for(i = 0; i < dict->size; i++) {
    subDict = dict->subDicts[i];
    if(subDict->channel!=channel) continue;
    if(GetTypeValue(subDict->dataContainer)!=stftType) continue;
    stft = (STFT)(subDict->dataContainer);
    if(stft->type != type) continue;
    if(stft->windowShape != windowShape) continue;
    if(stft->windowSize != windowSize) continue;
    // TODO : treat case where there are additional options
    if(options != NULL) {
      switch(type) {
      case HarmoStft :
      case HighResStft :
	Errorf("GetStftSubDict : options not treated for '%s' sub-dictionaries yet",StftType2Name(type));
      }
    }
    return(subDict);
  }
  return(NULL);
}

// Add a stft sub-dictionary (does NOT check whether it already exists)
SUBDICT PrivateAddStftSubDict(DICT dict,unsigned char channel,char flagMain,
			      char type,char windowShape,unsigned long windowSize,LISTV harmoOptions) 
{
  STFT stft;
  SUBDICT subDict;
  unsigned short i;
  unsigned char n;
  // Some default values for the moment
  char borderType = BorderPad0;
  char depthHighRes = 0;
  unsigned short timeRedund=STFT_DEFAULT_TIMEREDUND ,freqRedund = STFT_DEFAULT_FREQREDUND;
  char flagHarmonic = NO;
  float freq0Min = 0,freq0Max = 0;
  float freq0IdMin = 0,freq0IdMax = 0;
  unsigned short dim = STFTHARMO_DEFAULT_DIMHARMO;
  unsigned char iFMO = STFTHARMO_DEFAULT_IFMO;

  // Some basic checking
  // Basic checking 
  if(channel>dict->nChannels)  Errorf("PrivateAddStftSubDict : channel is too big");

  if(harmoOptions==NULL&&type==HarmoStft) Errorf("PrivateAddStftSubDict : NULL harmoOptions for 'harmo' &stft");
  if(harmoOptions!=NULL) {
    if(type!=HarmoStft) Errorf("PrivateAddStftSubDict : non NULL harmoOptions for '%s' &stft",StftType2Name(type));
    if(GetTypeValue(harmoOptions)!=listvType || harmoOptions->length<2)
      Errorf("PrivateAddStftSubDict : bad harmoOptions",StftType2Name(type));
  }

  // Do we need some auxiliary sub-dicts first ? 
  // It is better to put them first in the list if they do not exist already !
  switch(type) {
  case ComplexStft : 
    if(channel==dict->nChannels)  
      Errorf("PrivateAddStftSubDict : multichannel ComplexStft do not exist!");
    break;    
  case PhaseStft :
    if(channel==dict->nChannels)  
      Errorf("PrivateAddStftSubDict : multichannel PhaseStft do not exist!");
    if(GetStftSubDict(dict,channel,ComplexStft,windowShape,windowSize,NULL)==NULL)
      PrivateAddStftSubDict(dict,channel,NO,ComplexStft,windowShape,windowSize,NULL); 
    break;
  case RealStft : 
    // Case of a monochannel RealStft
    if(channel<dict->nChannels) {
      if(GetStftSubDict(dict,channel,ComplexStft,windowShape,windowSize,NULL)==NULL)
	PrivateAddStftSubDict(dict,channel,NO,ComplexStft,windowShape,windowSize,NULL); 
    } else {
      // Case of multichannel RealStft (the sum of monochannel ones)
      for(n = 0; n < dict->nChannels; n++) {
	if(GetStftSubDict(dict,n,RealStft,windowShape,windowSize,NULL)==NULL)
	  PrivateAddStftSubDict(dict,n,NO,RealStft,windowShape,windowSize,NULL); 
      }
    }
    break;
  case HarmoStft :
    if(dict->nChannels>1 && channel!=dict->nChannels) 
      Errorf("PrivateAddStftSubDict (Weired) : monochannel HarmoStft do not exist in multichannel dictionaries!");
    if(GetStftSubDict(dict,channel,RealStft,windowShape,windowSize,NULL)==NULL) 
      // When we add a harmonic sub-dictionary as a main dictionary, a plain Gabor
      // sub-dictionary with the same window should also be included as a main sub-dictionary
      PrivateAddStftSubDict(dict,channel,flagMain,RealStft,windowShape,windowSize,NULL); 
    break;
  case HighResStft :
    Errorf("PrivateAddStftSubDict : highres not implemented (depthHighRes not parameter...");
    if(channel<dict->nChannels) {
      if(GetStftSubDict(dict,channel,RealStft,windowShape,windowSize/2,NULL)==NULL)
	PrivateAddStftSubDict(dict,channel,NO,RealStft,windowShape,windowSize/2,NULL); 
      if(GetStftSubDict(dict,channel,RealStft,windowShape,windowSize,NULL)==NULL)
	PrivateAddStftSubDict(dict,channel,NO,RealStft,windowShape,windowSize,NULL); 
      if(GetStftSubDict(dict,channel,PhaseStft,windowShape,windowSize/2,NULL)==NULL)
	PrivateAddStftSubDict(dict,channel,NO,PhaseStft,windowShape,windowSize/2,NULL); 
      if(GetStftSubDict(dict,channel,PhaseStft,windowShape,windowSize,NULL)==NULL)
	PrivateAddStftSubDict(dict,channel,NO,PhaseStft,windowShape,windowSize,NULL); 
    }
    // TODO : decide what should be done about multichannel HighRes ?
    break;
  default : 
    Errorf("Stft of type '%s' cannot be added to a &dict",StftType2Name(type));
  }

  // Now it is time to add the stft we asked for
  stft = NewStft();
  CopyFieldsTFContent(dict,stft);
  switch(type) {
  case ComplexStft :
  case RealStft:
  case PhaseStft:
    SizeStft(stft,windowShape,windowSize,timeRedund,freqRedund,borderType,type,0,0,0,0);
    break;
  case HarmoStft:
    // First we should parse the harmonic options TODO: parse dim and iFMO too if given!
    if(harmoOptions->length!=2) Errorf("PrivateAddStftSubDict : bad harmo options");
    freq0Min = GetListvNthFloat(harmoOptions,0);
    freq0Max = GetListvNthFloat(harmoOptions,1);
    freq0IdMin = Freq2FreqId(stft,freq0Min);
    freq0IdMax = Freq2FreqId(stft,freq0Max);
    SizeStft(stft,windowShape,windowSize,timeRedund,freqRedund,borderType,type,freq0IdMin,freq0IdMax,iFMO,dim);
    // There are cases where, due to the orthogonality condition between partials of harmonic atoms, the 
    // 'harmo' stft is empty. Then we don't add it!
    // TODO : clean that ?
    if(stft->real==NULL) {
#ifdef DEBUG_HARMO
      Warningf("AddStftSubDict : deleting empty 'harmo' stft");
#endif // DEBUG_HARMO
      stft = DeleteStft(stft);
      return(NULL);
    }
    break;
  case HighResStft:
    SizeStft(stft,windowShape,windowSize,timeRedund,freqRedund,borderType,type,0,0,0,0);
    break;
  }

  // Now we can create a new sub-dictionary with 'stft' as data content
  subDict = NewSubDict();
  subDict->methods = &StftMethods;
  subDict->flagMain = flagMain;
  subDict->channel = channel;
  subDict->dataContainer = (VALUE)stft;
  // In the next update of the dictionary, the newly added STFT sub-dictionary
  // will have to be updated, so we need to to reset [updateTimeIdMin updateTimeIdMax]
  dict->updateTimeIdMin = 0;
  dict->updateTimeIdMax = dict->signalSize-1;
  return(PrivateAddSubDict(dict,subDict));
}

void UpdateStftSubDict(SUBDICT subDict)
{
  STFT stft = (STFT)(subDict->dataContainer);
  DICT dict = subDict->dict;
  SUBDICT subDictAux;
  STFT stftAux = NULL,stftAux1 = NULL,stftAux2 = NULL,stftAux3 = NULL;
  unsigned char n;
  short scaleFactorHighRes = 2;

  // Some checking
  if(GetTypeValue(stft)!=stftType)
    Errorf("UpdateStftSubDict (Weired) : data is not of type '%s'!",stftType);
  
  // If already up to date, do nothing
  if(subDict->flagUpToDate) return;
  
  // If we have something to do, it depends on the type of the stft
  switch(stft->type) {
  case ComplexStft :
    Printf("[C]");
    UpdateStftComplex(stft,GetChannel(dict,subDict->channel),dict->updateTimeIdMin,dict->updateTimeIdMax);
    Printf("\b\b\b");
    break;
  case PhaseStft:
    // Update the auxiliary sub-dictionaries first
    if((subDictAux = GetStftSubDict(dict,subDict->channel,ComplexStft,stft->windowShape,stft->windowSize,NULL))==NULL)
      Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary ComplexStft for PhaseStft"); 
    UpdateSubDict(subDictAux);
    stftAux = (STFT)subDictAux->dataContainer;
    // Update the asked sub-dictionary now
    Printf("[R]");
    UpdateStftRealOrPhase(stft,stftAux,dict->updateTimeIdMin,dict->updateTimeIdMax);
    Printf("\b\b\b");
    break;
  case RealStft:
    // Case of monochannel RealStft
    if(subDict->channel<dict->nChannels) {
      // Update the auxiliary sub-dictionaries first
      if((subDictAux = GetStftSubDict(dict,subDict->channel,ComplexStft,stft->windowShape,stft->windowSize,NULL))==NULL)
	Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary ComplexStft for RealStft"); 
      UpdateSubDict(subDictAux);
      stftAux = (STFT)subDictAux->dataContainer;
      // Update the asked sub-dictionary now
      Printf("[R]");
      UpdateStftRealOrPhase(stft,stftAux,dict->updateTimeIdMin,dict->updateTimeIdMax);
      Printf("\b\b\b");
    } else {
      // Case of multichannel RealStft
      ZeroStft(stft,dict->updateTimeIdMin,dict->updateTimeIdMax);
      for(n = 0; n < dict->nChannels; n++) {
	// Update the auxiliary sub-dictionaries first
	if((subDictAux = GetStftSubDict(dict,n,RealStft,stft->windowShape,stft->windowSize,NULL))==NULL)
	  Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary RealStft[%d] for Multichannel RealStft",n); 
	UpdateSubDict(subDictAux);
	stftAux = (STFT)subDictAux->dataContainer;
	// Update the asked sub-dictionary now
	Printf("[MR]");
	AddStft(stft,stftAux,dict->updateTimeIdMin,dict->updateTimeIdMax);
	Printf("\b\b\b\b");
      }
    }
    break;
  case HarmoStft :
    if(dict->nChannels>1 && subDict->channel!=dict->nChannels) 
      Errorf("UpdateStftSubDict (Weired) : monochannel HarmoStft should not exist in multichannel dictionaries!");
    // Update the auxiliary sub-dictionaries first
    if((subDictAux = GetStftSubDict(dict,subDict->channel,RealStft,stft->windowShape,stft->windowSize,NULL))==NULL)
      Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary RealStft for HarmoStft"); 
    UpdateSubDict(subDictAux);
    stftAux = (STFT)subDictAux->dataContainer;
    // Update the asked sub-dictionary now
    Printf("[H]");
    UpdateStftHarmo(stft,stftAux,dict->updateTimeIdMin,dict->updateTimeIdMax);
    Printf("\b\b\b");
    break;
  case HighResStft :
    Errorf("UpdateStftSubDict : HighResStft not implemented!");
    // Update the auxiliary sub-dictionaries first
    if((subDictAux = GetStftSubDict(dict,subDict->channel,RealStft,stft->windowShape,stft->windowSize,NULL))==NULL)
      Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary RealStft for HighResStft"); 
    UpdateSubDict(subDictAux);
    stftAux = (STFT)subDictAux->dataContainer;
    if((subDictAux = GetStftSubDict(dict,subDict->channel,PhaseStft,stft->windowShape,stft->windowSize,NULL))==NULL)
      Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary PhaseStft for HighResStft"); 
    UpdateSubDict(subDictAux);
    stftAux1 = (STFT)subDictAux->dataContainer;
    if((subDictAux = GetStftSubDict(dict,subDict->channel,RealStft,stft->windowShape,stft->windowSize/scaleFactorHighRes,NULL))==NULL)
      Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary RealStft for HighResStft"); 
    UpdateSubDict(subDictAux);
    stftAux2 = (STFT)subDictAux->dataContainer;
    if((subDictAux = GetStftSubDict(dict,subDict->channel,PhaseStft,stft->windowShape,stft->windowSize/scaleFactorHighRes,NULL))==NULL)
      Errorf("UpdateStftSubDict (Weired) : missing auxiliary sub-dictionary PhaseStft for HighResStft"); 
    UpdateSubDict(subDictAux);
    stftAux3 = (STFT)subDictAux->dataContainer;
    // Update the asked sub-dictionary now
    Printf("[h]");
    UpdateStftHighRes(stft,stftAux,stftAux1,stftAux2,stftAux3,dict->updateTimeIdMin,dict->updateTimeIdMax);
    Printf("\b\b\b");
    break;
  default : Errorf("UpdateStftSubDict : type %s not implemented",StftType2Name(stft->type));
  }
  subDict->flagUpToDate = YES;
  // TODO : remove that when possible
  stft->flagUpToDate=YES;
}

char GetMaxStftSubDict(SUBDICT subDict,LISTV searchRange,float *pMaxValue,VALUE result) 
{
  // Search range options
  unsigned short i;
  VALUE value=NULL;
  STRVALUE str=NULL;
  LISTV  lv=NULL;
  char* rangeName=NULL;
  char *type=NULL;
  RANGE range=NULL;
  float min,max;
  float f;

  // The search limits
  char flagCausal;
  long windowSizeMin,windowSizeMax;
  long timeIdMin,timeIdMax;
  long freqIdMin,freqIdMax;

  // The location of the maximum
  float maxValue;
  // TODO : change to unsigned 
  int maxTimeId = 0;
  int maxFreqId = 0;
  int maxFreq0Id= 0;
  int k_freqIdMin,k_freqIdMax;
  int tmp1=0,tmp2=0;

  // Some auxiliary variables
  MOLECULE molecule=NULL;
  static MOLECULE channelMol = NULL;
  ATOM atom        =NULL;
  ATOM channelAtom =NULL;
  unsigned char channel;
  unsigned short k;
  float *innerProdR=NULL,*innerProdI=NULL;
  DICT dict;
  SUBDICT subDictAux = NULL;
  STFT stftComplex = NULL;
  STFT stftReal    = NULL;

  STFT stft = (STFT)(subDict->dataContainer);
  // Checking arguments
  if(subDict->flagUpToDate==NO) Errorf("GetMaxStftSubDict : subDict is out of date!");

  // Default search range
  flagCausal  = NO;
  windowSizeMin = STFT_MIN_WINDOWSIZE; windowSizeMax = STFT_MAX_WINDOWSIZE;
  timeIdMin = 0; timeIdMax = stft->signalSize-1;
  freqIdMin = 0; freqIdMax = GABOR_NYQUIST_FREQID;

  // Limiting the search range if necessary
  if(searchRange != NULL) {
    i = 0;
    while(i < searchRange->length) {
      type = GetListvNth(searchRange,i,&value,&f);
      // All search range options are &strings or &listv
      if(type==strType) {
	str = CastValue(value,STRVALUE);
	if(!strcmp(str->str,"causal")) flagCausal=YES;
	i++;continue;
      } 
      if(type!=listvType) {
	i++;continue;
      }
      lv=CastValue(value,LISTV);
      if(lv->length!=2) {
	i++;continue;
      }
      // Get the range name and the range itself
      rangeName = GetListvNthStr(lv,0);
      type = GetListvNth(lv,1,(VALUE*)&value,&f);
      if(type==rangeType) {
	range = CastValue(value,RANGE);
	if(range->size >=2) Errorf("GetMaxStftSubDict : range size should be at most 2");
	min = RangeMin(range);max=RangeMax(range);
      } else if(type == numType) {
	min = max = f;
      } else Errorf("GetMaxStftSubDict : expect a &range or &num, not a '%s'",type);
      // Treat the known fields
      if(!strcmp(rangeName,"windowSize")) {
	windowSizeMin = MAX(windowSizeMin,min);windowSizeMax = MIN(windowSizeMax,max);
	if(!INRANGE(windowSizeMin,stft->windowSize,windowSizeMax)) return(NO);
	i++;continue;
      }
      if(!strcmp(rangeName,"timeId")) {
	timeIdMin = MAX(timeIdMin,min);timeIdMax = MIN(timeIdMax,max);
	if(timeIdMin>timeIdMax) return(NO);
	i++;continue;
      }
      if(!strcmp(rangeName,"time")) {
	timeIdMin = MAX(timeIdMin,Time2TimeId(stft,min));timeIdMax = MIN(timeIdMax,Time2TimeId(stft,max));
	if(timeIdMin>timeIdMax) return(NO);
	i++;continue;
      }
      if(!strcmp(rangeName,"freqId")) {
	freqIdMin = MAX(freqIdMin,min);freqIdMax = MIN(freqIdMax,max);
	if(freqIdMin>freqIdMax) return(NO);
	i++;continue;
      }
      if(!strcmp(rangeName,"freq")) {
	freqIdMin = MAX(freqIdMin,Freq2FreqId(stft,min));freqIdMax = MIN(freqIdMax,Freq2FreqId(stft,max));
	if(freqIdMin>freqIdMax) return(NO);
	i++;continue;
      }
      i++;continue;
    }
  }

  // Looks for the maximum over the stft
  if(pMaxValue) *pMaxValue=0.0;
  // If the search range was empty we return NO
  if(GetStftMax(stft,flagCausal,timeIdMin,timeIdMax,freqIdMin,freqIdMax,&maxTimeId,&maxFreqId,&maxValue)==NO) return(NO);
  if(pMaxValue) *pMaxValue=maxValue;
  
  // If the result is a &listv we should fill it
  if(GetTypeValue(result)==listvType) {
    ClearListv(CastValue(result,LISTV));
    lv = TNewListv();
    AppendStr2Listv(lv,"timeId");AppendInt2Listv(lv,maxTimeId);
    AppendValue2Listv(CastValue(result,LISTV),(VALUE)lv);
    lv = TNewListv();
    AppendStr2Listv(lv,"freqId");AppendInt2Listv(lv,maxFreqId);
    AppendValue2Listv(CastValue(result,LISTV),(VALUE)lv);
    return(YES);
  } 

  // If the result is a &mol we should fill it
  dict = subDict->dict;
  if(GetTypeValue(result)==moleculeType) {
    molecule = (MOLECULE)result;
    ClearMolecule(molecule);
    SizeMolecule(molecule,stft->dimHarmo*dict->nChannels);
    // What we know so far as a 'freqId' is indeed the fundamental frequency.
    maxFreq0Id = maxFreqId;
    /* First, we fill the molecule with the first channel */
    // Next we will loop (using 'maxFreqId') on the frequency of the partials
    for(k = 0; k < stft->dimHarmo; k++) {
      atom = NewAtom();
      // First, set the time-frequency content
      CopyFieldsTFContent(stft,atom);
      atom->windowShape= stft->windowShape;
      atom->windowSize = stft->windowSize;
      atom->timeId 	= maxTimeId;
      if(stft->type==HarmoStft) {
	// Determining the box around (k+1)*freq0Id where to look for a partial
	// if it is empty we return the molecule immediately without adding a new (empty!) atom.
	if(HarmoPartialBox(stft,k,maxFreq0Id,&k_freqIdMin,&k_freqIdMax) == NO) {
	  if(k==0) Errorf("GetMaxStftSubDict : (Weird) NULL first partial");
	  DeleteAtom(atom);
	  return(YES);
	}
	// Get the best partial in the box, using the (multichannel) 'real' stft of the dictionary 
	// with the corresponding window
	if(dict->nChannels>1) channel = dict->nChannels;
	else                  channel = 0;
	if((subDictAux = GetStftSubDict(subDict->dict,channel,RealStft,stft->windowShape,stft->windowSize,NULL))==NULL)
	  Errorf("GetMaxStftSubDict (Weired) : missing auxiliary sub-dictionary RealStft for setting harmonic molecule"); 
	if(subDictAux->flagUpToDate == NO) Errorf("GetMaxStftSubDict : stftReal is out of date");
	stftReal = (STFT)subDictAux->dataContainer;
	GetStftMax(stftReal,flagCausal,maxTimeId,maxTimeId,k_freqIdMin,k_freqIdMax,&tmp1,&tmp2,&f);
	// maxFreqId is the freqId of the partial 'atom'
	maxFreqId = tmp2;
      } 
      atom->freqId = maxFreqId;

      // Then, get the complex inner product (for the first channel) using the 'complex' stft of the dictionary 
      // with the corresponding window
      channel = 0;
      if((subDictAux = GetStftSubDict(subDict->dict,channel,ComplexStft,stft->windowShape,stft->windowSize,NULL))==NULL)
	Errorf("GetMaxStftSubDict (Weired) : missing auxiliary sub-dictionary ComplexStft for setting atom"); 
      if(subDictAux->flagUpToDate == NO)
	Errorf("GetMaxStftSubDict : stftComplex is out of date");
      stftComplex = (STFT)(subDictAux->dataContainer);  
      GetStftData(stftComplex,maxTimeId,&innerProdR,&innerProdI);
      atom->coeffR= innerProdR[maxFreqId/stftComplex->fRate];
      atom->coeffI= innerProdI[maxFreqId/stftComplex->fRate];
      //	  //	  Printf("channel0 %g\n",innerProdR[maxFreqId/stftComplex->fRate]);
      // Get the phase and real coeff2
      CastAtomReal(atom);
      // Compute the HighRes coefficient if asked
      if(stft->type==HighResStft) {
	//	SetRealAtom(atom,subDict->dict,subdict->dict->signal,
	//		    flagOptimize,
	//		    NO,YES);
	Errorf("GetMaxStftSubDict : Highres atom not re-implemented");
      }
      // Now we add the atom and increase the actual dimension
      AddAtom2Molecule(molecule,atom);
    }
    /* Now, we fill the other channels if necessary */
    if(dict->nChannels > 1) {
      if(channelMol==NULL) channelMol = NewMolecule();
      SizeMolecule(channelMol,molecule->dim);
      for(channel=1; channel<dict->nChannels; channel++) {
	ClearMolecule(channelMol);
	for(k = 0; k < molecule->dim; k++) {
	  atom        = GetMoleculeAtom(molecule,0,k);
	  channelAtom = CopyAtom(atom,channelAtom);
	  /* Get the complex inner product */
	  if((subDictAux = GetStftSubDict(subDict->dict,channel,ComplexStft,stft->windowShape,stft->windowSize,NULL))==NULL)
	    Errorf("GetMaxStftSubDict (Weired) : missing auxiliary sub-dictionary ComplexStft for setting atom"); 
	  if(subDictAux->flagUpToDate == NO)
	    Errorf("GetMaxStftSubDict : stftComplex is out of date");
	  stftComplex = (STFT)(subDictAux->dataContainer);  
	  GetStftData(stftComplex,channelAtom->timeId,&innerProdR,&innerProdI);
	  channelAtom->coeffR= innerProdR[((long)(channelAtom->freqId+0.5))/stftComplex->fRate];
	  channelAtom->coeffI= innerProdI[((long)(channelAtom->freqId+0.5))/stftComplex->fRate];
	  /* Get the phase and real coeff2 */
	  CastAtomReal(channelAtom);
	  /* Compute the HighRes coefficient if asked */
	  if(stft->type==HighResStft) {
	    //	SetRealAtom(atom,subDict->dict,subdict->dict->signal,
	    //		    flagOptimize,
	    //		    NO,YES);
	    Errorf("GetMaxStftSubDict : Highres atom not re-implemented");
	  }
	  // Now we add the channelAtom to the channelMolecule */
	  AddAtom2Molecule(channelMol,channelAtom);
	}
	AddChannel2Molecule(molecule,channelMol);
      }
    }
    return(YES);
  }
  Errorf("GetMaxStftSubDict : Weired error!");
}

SubDictMethods StftMethods = {
  &GetMaxStftSubDict,
  &UpdateStftSubDict
};

