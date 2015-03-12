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

/**********************************/
/*
 * 	DICT VARIABLES
 */
/**********************************/
char *dictType = "&dict";

/*
 * Answers to the different print messages
 */
void ShortPrintDict(DICT dict)
{
  Printf("<&dict;%p>\n",dict);
}

char *ToStrDict(DICT dict, char flagShort)
{
  static char str[30];
  
  sprintf(str,"<&dict;%p>",dict);
  return(str);
}

void PrintInfoDict(DICT dict)
{
  PrintDict(dict,NO);
}


DICT TNewDict(void)
{
  DICT dict;
  
  dict = NewDict();
  TempValue(dict);
  return(dict);
}

/*
 * Get the current dict
 * (generate an error if there is none)
 */
DICT GetDictCur(void)
{
  DICT dict;
  
  if(!ParseTypedValLevel_(levelCur,"objCur",NULL,(VALUE *)&dict,dictType)) Errorf1("");
  
  if (dict == NULL) Errorf1("");
  
  AddRefValue(dict);
  TempValue(dict);
  
  return(dict);
}


/*******************************************/
/*
 *	Basic data management for SUBDICT
 */				    
/*******************************************/
static void InitSubDict(SUBDICT subDict)
{
  subDict->methods       = NULL;
  subDict->flagMain      = NO;
  subDict->channel = 0;
  subDict->flagUpToDate  = NO;
  subDict->dataContainer = NULL;
  subDict->dict          = NULL;
}

SUBDICT NewSubDict()
{
  SUBDICT subDict;
  
#ifdef DEBUGALLOC
  DebugType = "SubDict";
#endif
  
  subDict = (SUBDICT) Malloc(sizeof(struct subDict));
  InitSubDict(subDict);
  return(subDict);
}

static void ClearSubDict(SUBDICT subDict)
{
  if (subDict == NULL)  Errorf("ClearSubDict : NULL subDict");
  if(subDict->dataContainer) {
    DeleteValue(subDict->dataContainer);
    subDict->dataContainer = NULL;
  }
  InitSubDict(subDict);
}

static SUBDICT DeleteSubDict(SUBDICT subDict)
{
  if (subDict == NULL)  Errorf("DeleteSubDict : NULL subDict");
  if(subDict->dataContainer) {
    DeleteValue(subDict->dataContainer);
    subDict->dataContainer = NULL;
  }
#ifdef DEBUGALLOC
  DebugType = "SubDict";
#endif
  Free(subDict);
  return(NULL);
}

/*******************************************/
/*
 *	Basic data management for DICT
 */				    
/*******************************************/
DICT NewDict()
{
  DICT dict;
  
#ifdef DEBUGALLOC
  DebugType = "Dict";
#endif
  
  dict = (DICT) Malloc(sizeof(struct dict));
  InitValue(dict,&tsDict);
  InitTFContent(dict);

  dict->nChannels      = 0;
  dict->nChannelsAlloc = 0;
  dict->channels        = NULL;
  dict->signalEnergy   = 0.0;

  dict->updateTimeIdMin = 1;
  dict->updateTimeIdMax = 0;
  dict->removedMolecule = NULL;

  dict->size = 0;
  dict->sizeAlloc = 0;
  dict->subDicts = NULL;
  return(dict);
}

DICT DeleteDict(DICT dict)
{
  unsigned short i;

  if (dict == NULL)  Errorf("DeleteDict : NULL dict");
  if (dict->nRef==0) Errorf("*** Danger : trying to delete a temporary dict\n");

  RemoveRefValue(dict);
  if (dict->nRef > 0) return(NULL);
  
  if(dict->channels) {
    for(i = 0; i < dict->nChannels; i++) {
      DeleteSignal(dict->channels[i]);
      dict->channels[i] = NULL;
    }
    Free(dict->channels);
    dict->channels = NULL;
  }
  if(dict->removedMolecule) dict->removedMolecule = DeleteMolecule(dict->removedMolecule);
  if(dict->subDicts) {
    for(i=0; i<= dict->size; i++) {if(dict->subDicts[i]) 
	dict->subDicts[i] = DeleteSubDict(dict->subDicts[i]);
    }
    Free(dict->subDicts);
    dict->subDicts = NULL;
  }  
  
#ifdef DEBUGALLOC
  DebugType = "Dict";
#endif
  
  Free(dict);
  return(NULL);
}

void ClearDict(DICT dict)
{
  unsigned short i;

  if(dict == NULL)  Errorf("ClearDict : NULL dict");
  InitTFContent(dict);

  if(dict->channels) {
    for(i = 0; i < dict->nChannels; i++) {
      DeleteSignal(dict->channels[i]);
      dict->channels[i] = NULL;
    }
    dict->nChannels       = 0;
    /* Note that we do NOT de-allocate the array of channels. */
    dict->signalEnergy    = 0.0;
    dict->updateTimeIdMin = 1;
    dict->updateTimeIdMin = 0;
  }
  if(dict->removedMolecule) dict->removedMolecule = DeleteMolecule(dict->removedMolecule);
  if(dict->subDicts) {
    for(i=0; i<= dict->size; i++) {	
      if(dict->subDicts[i])
	dict->subDicts[i] = DeleteSubDict(dict->subDicts[i]);
    }
    dict->size = 0;
  }  
  /* Note that we do NOT delete the array of sub-dictionaries, we keep it for later use  */
}

/* Prints the content of a dictionary, in long or short form */
void PrintDict(const DICT dict,char flagShort)
{
  unsigned short i;
  SUBDICT subDict;
  if(dict->channels == NULL) {
    Printf("Empty dict\n");
  } else {
    Printf("Channels : %d\n",dict->nChannels);
    PrintInfoValue(GetChannel(dict,0));
    Printf("Energy : %g\n",dict->signalEnergy);
    Printf("------------\n");
    Printf("Update range : ");
    if(dict->updateTimeIdMin <= dict->updateTimeIdMax)
      Printf("[%d %d]\n",dict->updateTimeIdMin,dict->updateTimeIdMax);
    else
      Printf("empty\n");
    if(dict->removedMolecule) {
      Printf("------------\n");
      Printf("Removed &mol :\n");
      PrintInfoValue(dict->removedMolecule);
    }
    for(i = 0; i < dict->size; i++) {
      subDict = dict->subDicts[i];
      Printf("------------------\n");
      Printf("Sub-dictionary[%d] : ",i);
      if(subDict->channel<dict->nChannels) Printf("channel %d\n",subDict->channel);
      else Printf("multichannel\n");
      if(subDict->flagMain) Printf("main\n");
      else Printf("auxiliary\n");
      PrintInfoValue(subDict->dataContainer);
    }
  }
}

/* Function that generates an error if the dictionary does not contain any sub-dictionary */
void CheckDictNotEmpty(const DICT dict)
{
  if(dict == NULL)    Errorf("CheckDictNotEmpty : NULL dict");
  if(dict->size == 0) Errorf("CheckDictNotEmpty : empty dict");
}

/*
 * If 'sizeAlloc' is smaller than dict->size, an error is generated.
 * Else the allocation size of the array of sub-dictionaries is adjusted :
 * -the newly allocated part of the array is initialized to NULL sub-dictionaries;
 * -the previously allocated part is kept (dict->size is not changed)
 */
void SizeDict(DICT dict,unsigned short sizeAlloc)
{
  unsigned short i;
  if(sizeAlloc<dict->size) Errorf("SizeDict : cannot (re)allocate less than the number of sub-dictionaries");
  if(sizeAlloc==dict->size) return;
  /* Case of an first allocation */
  if(dict->sizeAlloc == 0) {
    dict->subDicts=(SUBDICT *)Calloc(sizeAlloc,sizeof(SUBDICT));
    dict->sizeAlloc = sizeAlloc;
  }
  /* Case of a resize */
  if(dict->size==dict->sizeAlloc) {
    dict->subDicts=(SUBDICT *)Realloc(dict->subDicts,sizeAlloc*sizeof(SUBDICT));
    /* Initialize the newly allocated data, if necessary */
    for(i = dict->sizeAlloc; i < sizeAlloc; i++)
      dict->subDicts[i]=NULL;
    dict->sizeAlloc = sizeAlloc;
  }
}

/*
 * Takes care of the reallocation if necessary
 * WARNING : should only be used with newly created subDicts!
 */
SUBDICT PrivateAddSubDict(DICT dict,SUBDICT subDict) 
{
  if(subDict->dict!=NULL) Errorf("PrivateAddSubDict : (Weired) the sub-dictionary already belongs to a dictionary!");
  /* In case we need to allocate more room */
  if(dict->size==dict->sizeAlloc) {
    if(dict->sizeAlloc==0) SizeDict(dict,MP_DEFAULT_DICT_SIZE);
    else SizeDict(dict,2*dict->sizeAlloc);
  }
  subDict->dict = dict;
  dict->subDicts[dict->size]=subDict;
  dict->size++;
  return(subDict);
}

/*******************************************/
/*
 *	Accessing DICT data
 */				    
/*******************************************/
/* If dict->channels is NULL, an error is generated */
SIGNAL GetChannel(DICT dict,unsigned char channel) 
{
  if(dict->channels==NULL)            Errorf("GetChannel : channels is NULL");
  if(channel>=dict->nChannels) Errorf("GetChannel : bad channel number %d>%d",channel,dict->nChannels);
  return(dict->channels[channel]);
} 

/*
 * If the subDictType is unknown, or the parameters are irrelevant for the type, an error is generated.
 * Else returns the first sub-dictionary that matches the type and parameters (NULL is none matches)
 */
SUBDICT GetSubDict(DICT dict,unsigned char channel,char* subDictType,LISTV parameters)
{
  char type;
  char windowShape;
  unsigned long windowSize;
  LISTV options;
  float f;

  /* Case of sub-dictionary of local maxima */
  if(!strcmp(subDictType,maximaDictType)) {
    if(parameters!=NULL) Errorf("GetSubDict : irrelevant parameters for '%s' sub-dictionary",maximaDictType);
    if(channel!=dict->nChannels) Errorf("GetSubDict : '%s' sub-dictionary can only be multichannel",maximaDictType);
    return(GetMaximaDictSubDict(dict));
  }
  /* Case of Stft sub-dictionary */
  if(!strcmp(subDictType,stftType)) {
    if(parameters==NULL||!INRANGE(3,parameters->length,4)) Errorf("GetSubDict : bad parameters for '%s' sub-dictionary",stftType);
    type = Name2StftType(GetListvNthStr(parameters,0));
    windowShape = Name2WindowShape(GetListvNthStr(parameters,1));
    windowSize  = GetListvNthFloat(parameters,2);
    options= NULL;
    if(parameters->length==4) GetListvNth(parameters,3,(VALUE *)&options,&f);
    // TODO ??   return(GetStftSubDict(dict,channel,type,windowShape,windowSize,options));
    return(GetStftSubDict(dict,channel,type,windowShape,windowSize,NULL));
  }
  Errorf("GetSubDict : unknown sub-dictionary type '%s'",subDictType);
}

/*
 * Change the dx of a dictionary : change dx for dict->channels as well as all sub-dictionaries.
 */
void SetDictDX(DICT dict,float dx)
{
  unsigned char channel;
  SIGNAL signal;
  unsigned short i;
  SUBDICT subDict;
  STFT stft;
  MAXIMADICT maximaDict;
  //WTRANS wtrans;

  if(dict == NULL)         Errorf("SetDictDX : NULL dict");
  if(dict->channels==NULL) Errorf("SetDictDX : NULL dict->channels");
  if (dx <= 0)             Errorf("SetDictDX : dx %g <= 0\n",dx);
  
  /* First, the dx of all channels */
  for(channel=0;channel<dict->nChannels;channel++) {
    signal = GetChannel(dict,channel);
    signal->dx = dx;
  }
  /* Then, that of the sub-dictionaries */
  for(i = 0; i < dict->size; i++) {
    subDict = dict->subDicts[i];
    /* Case of STFT subDicts */
    if(GetTypeValue(subDict->dataContainer)==stftType) {
      stft=(STFT)(subDict->dataContainer);
      stft->dx = dx;
      continue;
    } 
    /* Case of MAXIMADICT subDicts */
    if(GetTypeValue(subDict->dataContainer)==maximaDictType) {
      maximaDict=(MAXIMADICT)subDict->dataContainer;
      Errorf("SetMaximaDictDX not implemented");
      //SetMaximaDictDX(maximaDict,dx); TODO !
      continue;
    }
    Errorf("SetDictDX : (Weired)");
  }
}

/* Change the dx of a dictionary : change x0 for dict->channels as well as all sub-dictionaries. */
void SetDictX0(DICT dict,float x0)
{
  unsigned char channel;
  SIGNAL signal;
  unsigned short i;
  SUBDICT subDict;
  STFT stft;
  MAXIMADICT maximaDict;
  //WTRANS wtrans;
  
  /* Checking arguments */
  if(dict == NULL)         Errorf("SetDictX0 : NULL dict");
  if(dict->channels==NULL) Errorf("SetDictX0 : NULL dict->channels");

  /* First, the x0 of all channels */
  for(channel=0;channel<dict->nChannels;channel++) {
    signal = GetChannel(dict,channel);
    signal->x0 = x0;
  }

  /* Then, that of the sub-dictionaries */
  for(i = 0; i < dict->size; i++) {
    subDict = dict->subDicts[i];
    /* Case of STFT subDicts */
    if(GetTypeValue(subDict->dataContainer)==stftType) {
      stft=(STFT)(subDict->dataContainer);
      stft->x0 = x0;
      continue;
    } 
    /* Case of MAXIMADICT subDicts */
    if(GetTypeValue(subDict->dataContainer)==maximaDictType) {
      maximaDict=(MAXIMADICT)subDict->dataContainer;
      Errorf("SetMaximaDictX0 not implemented");
      //SetMaximaDictX0(maximaDict,x0); TODO!
      continue;
    }
    Errorf("SetDictX0 : (Weired)");
  }
}

/*************************************************/
/*
 *	The main functionalities of a dictionary
 */				    
/*************************************************/
/*
 * Update all the 'main' sub-dictionaries, and the necessary 'aux' sub-dictionaries,
 * using the 'removedMolecule' if necessary, and resets [updateTimeIdMin,updateTimeIdMax].
 * The state of the dictionary after a call to this function is as follows :
 * 0/ removedMolecule = NULL.
 * 1/if there is no sub-dictionary of local maxima : all sub-dictionaries are up to date
 * and [updaTimeIdMin,updateTimeIdMax] = [dict->channels[0]->size-1,0] 
 * (when a molecule is removed from the dictionary, updateTimeIdMin/Max decreases/increases)
 * 2/if there is a sub-dictionary of local maxima  : this sub-dictionary is up to date,
 * and the state of [updaTimeIdMin,updateTimeIdMax] depends on whether the update of the 
 * sub-dictionary of local maxima involved an 'initialization' or not.
 */
void UpdateDict(DICT dict) {
  unsigned short i;
  SUBDICT subDict;

  /* If already up to date, do nothing */
  if(dict->updateTimeIdMin > dict->updateTimeIdMax) return;
  
  /* When there is a sub-dictionary of local maxima */
  if(subDict=GetMaximaDictSubDict(dict)) {
    /* This takes care of the proper management of [updateTimeIdMin,updateTimeIdMax] */
    UpdateSubDict(subDict);
  }
  /* When there is no sub-dictionary of local maxima */
  else {
    for(i = 0; i < dict->size; i++) {
      subDict = dict->subDicts[i];
      /* Display a clock for waiting */
      switch(i%4) {
      case 0 : Printf("\\"); Flush(); break;
      case 1 : Printf("|"); Flush(); break;
      case 2 : Printf("/"); Flush(); break;
      case 3 : Printf("-"); Flush(); break;
      }
      UpdateSubDict(subDict);
      Printf("\b");
    }
    dict->updateTimeIdMin = dict->channels[0]->size-1;
    dict->updateTimeIdMax = 0;
  }
  /* Deletes the removedMolecule if exists */
  if(dict->removedMolecule) dict->removedMolecule = DeleteMolecule(dict->removedMolecule);
}

/*
 * Returns a MOLECULE corresponding to the maximum of a DICT over a search range.
 * If the specified search range is empty or the  maximum value is zero, we return NULL.
 * The returned MOLECULE is not optimized (interpolate,chirped,...).
 * The maximum is looked for only in 'main' sub-dictionaries, which should be up to date
 * (else an error is generated).
 * An error is also generated if [updateTimeIdMin,updateTimeIdMax] is not empty in the case
 * when there is no sub-dictionary of local maxima.
 */
MOLECULE GetMaxDict(DICT dict,LISTV searchRange) 
{
  unsigned short i;
  SUBDICT subDict,subDictMax=NULL;
  static LISTV tmpParamMax = NULL;
  LISTV paramMax = NULL;
  char  maxFound = NO;
  float tmpMaxValue = 0.0,maxValue = 0.0;
  MOLECULE molecule = NULL;
  MAXIMADICT maximaDict;

  /* Allocate (once only) the list of parameters that will be used     */
  /* to locate quickly the &mol after the first exhaustive search */
  if(tmpParamMax == NULL)     tmpParamMax = NewListv();
  else                        ClearListv(tmpParamMax);

  /* First, locate the subdict that contains the maximum. */
  if((subDictMax=GetMaximaDictSubDict(dict))!=NULL) {
    /* 
     * If there is a sub-dictionary of local maxima, this is the only sub-dictionary where to look.
     * and there is a maximum as soon as it is not empty 
     */
    maximaDict = (MAXIMADICT)(subDictMax->dataContainer);
    if(maximaDict->nMaxima>0) {
      maxFound = YES;
    }
  } else {
    /*
     * Check that[updateTimeIdMin,updateTimeIdMax] is empty
     * (an individual check of each sub-dictionary being up to date will be done during the search for the max)
     */
    if(dict->updateTimeIdMin <= dict->updateTimeIdMax) 
      Errorf("GetMaxDict : dict should be updated first in [%d %d]",dict->updateTimeIdMin,dict->updateTimeIdMax);
    for(i = 0; i < dict->size; i++) {
      subDict = dict->subDicts[i];
      /* Skip auxiliary subDicts */
      if(subDict->flagMain==NO) continue;
      /* If the search range is empty for the subDict, we skip to the next subDict */
      if(GetMaxSubDict(subDict,searchRange,&tmpMaxValue,tmpParamMax)==NO) continue;
      /* The search range was non empty, hence we can compare the local maximum to the best we have so far */
      if(maxFound == NO || tmpMaxValue > maxValue) {
	maxValue = tmpMaxValue;
	paramMax = TNewListv();
	CopyListv(tmpParamMax,paramMax);
	subDictMax = subDict;
      }
      maxFound = YES;
    }
  } 

  /* If no maximum has been found (i.e. the search range was empty or the max was zero) we return NULL */
  if(maxFound == NO) return(NULL);

  /* Now we have to fill the maximum variable */
  molecule = TNewMolecule();
  GetMaxSubDict(subDictMax,paramMax,NULL,molecule);
  return(molecule);
}

// TODO : clean this up
void AddShiftedSignals(SIGNAL input,SIGNAL output,long shift)
{
  long iOut,iOutMin,iOutMax;
  
  /* Some Tests */
  if (output == input)   Errorf("AddShiftedSignals : you must have output != input\n");
  if (output->size == 0) Errorf("AddShiftedSignals : output should not be empty\n");
  
  iOutMin = MAX(shift,0);
  iOutMax = MIN(output->size,shift+input->size);
  
  for (iOut = iOutMin; iOut < iOutMax; iOut++)
    output->Y[iOut] += input->Y[iOut-shift];
}


static void SubstractShiftedSignals(SIGNAL input,SIGNAL output,long shift)
{
  long iOut,iOutMin,iOutMax;

  /* Some Tests */
  if (output == input)   Errorf("SubstractShiftedSignals : you must have output != input\n");
  if (output->size == 0) Errorf("SubstractShiftedSignals : output should not be empty\n");

  iOutMin = MAX(shift,0);
  iOutMax = MIN(output->size,shift+input->size);
  
  for (iOut = iOutMin; iOut < iOutMax; iOut++)
    output->Y[iOut] -= input->Y[iOut-shift];
}


/*
 * Builds an atom and removes it from a given channel of a dictionary.
 * The [updateTimeIdMin,updateTimeIdMax] of the dict is enlarged if necessary.
 * All sub-dictionaries are marked as out of date.
 */
static void RemoveAtomFromDictChannel(DICT dict,unsigned char channel,ATOM atom) 
{
  static ATOM copy = NULL;
  static SIGNAL atomSignal = NULL;
  // These are signed integers for they may be negative
  // TODO : change to unsigned ?
  int atomTimeIdMin,atomTimeIdMax;

  float shift;

  // TODO : What borderType do we really want ? Shall we store it in dict ?
  char borderType = BorderPad0;
  float real1,imag1,real2,imag2;
  /* Checking arguments */
  CheckDictNotEmpty(dict);
  CheckAtom(atom);

  /* Allocating (just once) */
  if(atomSignal == NULL) atomSignal = NewSignal();
  
  // DEBUG : COMPRENDRE POURQUOI CA ARRIVE
  copy = CopyAtom(atom,copy);
  SCAtomInnerProduct(GetChannel(dict,channel),copy,borderType,&(copy->coeffR),&(copy->coeffI));
  CastAtomReal(copy);
  if(fabsf((atom->coeffR-copy->coeffR)/atom->coeffR)>1e-5 || fabsf((atom->coeffI-copy->coeffI)/atom->coeffI)>1e-5) {
    //    Printf("FreqId %g Phase %g %g\n",atom->freqId,atom->cosPhase,atom->sinPhase);
    //    Printf("Given (%g,%g) (%g,%g,%g) [%g,%g]\n",atom->coeffR,atom->coeffI,atom->coeff2,atom->cosPhase,atom->sinPhase,atom->realGG,atom->imagGG);
    //    Printf("Computed (%g,%g) (%g,%g,%g) [%g,%g]\n",copy->coeffR,copy->coeffI,copy->coeff2,copy->cosPhase,copy->sinPhase,copy->realGG,copy->imagGG);
  }
  /* Build a real valued atom  */
  BuildAtom(atom,atomSignal,NULL,borderType,YES);
  /* Substract it to the dict->signal */
  // TODO : check the shift is integer! and clean this up anyway
  shift = (atomSignal->x0-atom->x0)/atom->dx;
  if (shift < 0) shift = (int) (shift-.5);
  else shift = (int) (shift+.5);
  SubstractShiftedSignals(atomSignal,GetChannel(dict,channel),shift);
    
  /* Compute the limits where the signal has changed */
  ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,&atomTimeIdMin,&atomTimeIdMax);
  dict->updateTimeIdMin = MIN(dict->updateTimeIdMin,atomTimeIdMin);
  dict->updateTimeIdMax = MAX(dict->updateTimeIdMax,atomTimeIdMax);

  // TODO : treat other border effects (pass it as an argument to the function ?)
  // TODO : clean this up !!!
  switch(borderType) {
  case BorderPad0 :
    if(atomTimeIdMin < 0 || dict->updateTimeIdMax >= dict->signalSize)
      Errorf("RemoveAtomFromDictChannels : signal updated out of its limits!");
    break;
  default :
    Errorf("RemoveAtomFromDictChannels : border type '%s' not treated yet",BorderType2Name(borderType));
  }
}

/*
 * Builds a molecule and removes it from the signal of a dictionary.
 * The signalEnergy is updated. An error is generated if it does not decrease.
 * The [updateTimeIdMin,updateTimeIdMax] of the dict is enlarged if necessary.
 * All sub-dictionaries are marked as out of date.
 * The molecule is memorized in the dict->removedMolecule field for possible
 * use at the next UpdateDict step.
 * If a molecule has already been memorized, an error is generated.
 */
void RemoveMoleculeFromDict(DICT dict,MOLECULE molecule) 
{
  unsigned char channel;
  unsigned short k;
  SIGNAL signal;
  ATOM atom;
  unsigned long i;
  float prevSigEnergy;

  SUBDICT subDict;
  // TODO : remove that when possible
  STFT stft;

  /* No other molecule should have been memorized */
  if(dict->removedMolecule) Errorf("RemoveMoleculeFromDict : a molecule has already been removed");

  
  for(channel = 0; channel < molecule->nChannels; channel++) {
    for(k = 0; k < molecule->dim; k++) {
      atom = GetMoleculeAtom(molecule,channel,k);
      RemoveAtomFromDictChannel(dict,channel,atom);
    }
  }

  /* Sub-dictionaries of the dict are now out of date */
  for(i = 0; i < dict->size; i++) {
    subDict = dict->subDicts[i];
    subDict->flagUpToDate = NO;
    // TODO : remove that when possible
    if(GetTypeValue(subDict->dataContainer)==stftType) {
      stft=(STFT)(subDict->dataContainer);
      stft->flagUpToDate=NO;
    }
  }

  /* Update the signalEnergy */
  // TODO fast update using [updateTimeIdMin updateTimeIdMax] ???
  prevSigEnergy = dict->signalEnergy;
  dict->signalEnergy = 0.0;
  for(channel=0; channel<dict->nChannels;channel++) {
    signal = GetChannel(dict,channel);
    for(i = 0; i < dict->signalSize; i++)
      dict->signalEnergy += signal->Y[i]*signal->Y[i];
  }
  /* The energy should strictly decrease */
  // TODO : remove when we are confident enough this will never happen
  if(isnan(dict->signalEnergy) || prevSigEnergy<dict->signalEnergy) {
    PrintInfoValue(molecule);
    Errorf("RemoveMoleculeFromDict : (WEIRED) increases the signalEnergy from %g to %g",prevSigEnergy,dict->signalEnergy);
  } 

  /* Memorize the molecule */
  dict->removedMolecule = molecule;
  AddRefValue(molecule);  

}

/*
 * If dict->channels is already set or the new signal(s) have inconsistent size/dx/x0, an error is generated
 * The [updateTimeIdMin,updateTimeIdMax] is set to [0,dict->signalSize-1]
 * The signalEnergy is initialized.
 */
void AddChannels(DICT dict,LISTV lv)
{
  unsigned char channel;
  char *type;
  SIGNAL signal;
  float f;

  unsigned long i;

  /* Basic checking */
  if(dict->nChannels>0)             Errorf("AddChannels : should clear the dict before");
  if(lv->length>DICT_MAX_NCHANNELS) Errorf("AddChannels : too many channels (the max is %d)",DICT_MAX_NCHANNELS);
  
  /* If we need to (re)allocate dict->channels */
  if(lv->length>dict->nChannelsAlloc) {
    if(dict->nChannelsAlloc==0) {
      dict->channels        = (SIGNAL*) Calloc(lv->length,sizeof(SIGNAL));
      dict->nChannelsAlloc = lv->length;
    } else {
      dict->channels = (SIGNAL*) Realloc(dict->channels,lv->length*sizeof(SIGNAL));
      dict->nChannelsAlloc = lv->length;
    }
  }  

  /* The signalEnergy is the sum of all signal energies */
  dict->signalEnergy = 0.0;
  for(channel=0; channel<lv->length; channel++) {
    type = GetListvNth(lv,channel,(VALUE *)&signal,&f);
    /* Basic checkings */
    if((type!=signalType && type!=signaliType) || signal==NULL || signal->size<=0 || signal->type!=YSIG) 
      Errorf("AddChannels : listv[%d] should contain a non empty YSIG signal",channel);
    /* All channels should have the same structure as the first channel */
    if(channel>0) {
      if(signal->size!=dict->signalSize)
	Errorf("AddChannels : bad size %d for listv[%d]!",signal->size,channel);
      if(signal->x0!=dict->x0) 
	Errorf("AddChannels : bad size %d for listv[%d]!",signal->x0,channel);
      if(signal->dx!=dict->dx) 
	Errorf("AddChannels : bad size %d for listv[%d]!",signal->dx,channel);
    }
    /* Now we can add a signal */
    dict->channels[channel] = NewSignal();
    CopySig(signal,dict->channels[channel]);
    dict->nChannels++;
    /* Update the signalEnergy */
    for(i = 0; i < signal->size; i++)
      dict->signalEnergy += signal->Y[i]*signal->Y[i];
    /* If this is the first channel we should initialize the TFContent */
    if(channel==0) {
      dict->x0 = signal->x0;
      dict->dx = signal->dx;
      dict->signalSize = signal->size;
    }
  }
  dict->updateTimeIdMin = 0;
  dict->updateTimeIdMax = dict->signalSize-1;
}

/*
 * Adds to a 'dict' a sub-dictionary that matches the type and parameters.
 * Id subDictType is unknown, or if the parameters are irrelevant for the type, an error is generated.
 *
 * If the sub-dictionary that we want to add already exists, it is simply kept, and is set to 'main'
 * if necessary. If auxiliary sub-dictionaries are needed, they are added too.
 * 
 * Nota : if the added sub-dictionary is a MAXIMADICT (i.e. it consists of local maxima
 * of other sub-dictionaries), it becomes the only 'main' sub-dictionary :
 *   -all previously existing 'main' sub-dictionaries are added to it and converted to 'auxiliary' 
 *   -all subsequently added sub-dictionaries are automatically added to the MAXIMADICT sub-dictionary.
 */
extern SUBDICT PrivateAddStftSubDict(DICT dict,unsigned char channel,char flagMain,char type,char windowShape,unsigned long windowSize,LISTV harmoOptions);
extern SUBDICT PrivateAddMaximaDictSubDict(DICT dict,unsigned long nMaximaTarget);

void AddSubDict(DICT dict,char* subDictType,LISTV parameters)
{
  char type;
  char windowShape;
  unsigned long windowSize;
  char *harmoOptionsType;
  LISTV harmoOptions;
  float f;
  unsigned long nMaximaTarget;
  SUBDICT subDict;
  SUBDICT subDictMaxima;
  MAXIMADICT maximaDict;

  unsigned char channel;
  /* Some channel(s) should have been set first */
  if(dict->nChannels==0) Errorf("AddSubDict : add channels(s) first!");

  /* For a monochannel dictionary we add monochannel sub-dictionaries */
  if(dict->nChannels==1)  channel=0;
  /* For a multichannel dictionary we add (joint) multichannel sub-dictionaries */
  else                    channel=dict->nChannels;

  /* Special case of a sub-dictionary of local maxima */
  if(!strcmp(subDictType,maximaDictType)) {
    if(parameters==NULL||parameters->length!=1) Errorf("AddSubDict : bad parameters for '%s' sub-dictionary",maximaDictType);
    nMaximaTarget = GetListvNthFloat(parameters,0);
    /* If it is already existing there is nothing to do */
    if(GetMaximaDictSubDict(dict)) return;
    PrivateAddMaximaDictSubDict(dict,nMaximaTarget);
    /* There is nothing more to do ! */
    return;
  }

  /* Case when such a subDict already exists */
  if((subDict=GetSubDict(dict,channel,subDictType,parameters))!=NULL) {
    /* If 'subDict' was 'auxiliary' 'dict' contains no sub-dict of local maxima we convert 'subDict' to 'main'. */
    if((subDictMaxima=GetMaximaDictSubDict(dict))==NULL) 
      subDict->flagMain = YES;
    /* Else we add it to the sub-dictionary of local maxima, and convert it to 'auxiliary' */
    else {
      maximaDict = (MAXIMADICT)(subDictMaxima->dataContainer);
      AddSubDict2MaximaDict(maximaDict,subDict);/* This takes care of switching to an 'auxiliary' subDict. */
    }
    return;
  }

  /* Case where the sub-dictionary did not exist : we have to create one depending on the type asked for */
  /* Case of Stft sub-dictionary */
  if(!strcmp(subDictType,stftType)) {
    if(parameters==NULL||!INRANGE(3,parameters->length,4)) Errorf("AddSubDict : bad parameters for '%s' sub-dictionary",stftType);
    type        = Name2StftType(GetListvNthStr(parameters,0));
    windowShape = Name2WindowShape(GetListvNthStr(parameters,1));
    windowSize  = GetListvNthFloat(parameters,2);
    harmoOptions= NULL;
    if(parameters->length==4) GetListvNth(parameters,3,(VALUE *)&harmoOptions,&f);
    subDict = PrivateAddStftSubDict(dict,channel,YES,type,windowShape,windowSize,harmoOptions);
  }
  else {
    Errorf("AddSubDict : unknown sub-dictionary type '%s'",subDictType);
  }
  
  /* If there is no sub-dictionary of local maxima, we have nothing more to do. */
  if((subDictMaxima=GetMaximaDictSubDict(dict))==NULL) return;
  /* Else we add the new sub-dict  to the sub-dict of local maxima (and convert it to 'auxiliary') */
  else {
    maximaDict = (MAXIMADICT)(subDictMaxima->dataContainer);
    AddSubDict2MaximaDict(maximaDict,subDict);/* This takes care of switching to an 'auxiliary' subDict. */
    return;
  }
}


/*
 *  Command to perform the main functions
 */
void C_SetDict(char **argv)
{
  DICT dict;
  char *action;
  char *subDictType;
  LISTV signalList;
  LISTV parameters;
  LISTV searchRange,optimizations;
  MOLECULE molecule;
  ATOM atom;
  unsigned char channel;

  argv = ParseArgv(argv,tWORD,&action,tDICT,&dict,-1);
  if(!strcmp(action,"channels")) {
    argv = ParseArgv(argv,tLISTV,&signalList,0);
    AddChannels(dict,signalList);
    return;
  }
  if(!strcmp(action,"add")) {
    argv = ParseArgv(argv,tSTR,&subDictType,tLISTV_,NULL,&parameters,0);
    AddSubDict(dict,subDictType,parameters);
    return;
  }
  if(!strcmp(action,"update")) {
    NoMoreArgs(argv);
    UpdateDict(dict);
    return;
  }
  if(!strcmp(action,"getmax")) {
    argv = ParseArgv(argv,tLISTV_,NULL,&searchRange,0);
    molecule = GetMaxDict(dict,searchRange);
    if(molecule==NULL)   SetResultValue(nullValue);
    else             SetResultValue(molecule);
    return;
  }

  if(!strcmp(action,"optmol")) {
    argv = ParseArgv(argv,tMOLECULE,&molecule,tLISTV_,NULL,&optimizations,0);
    OptimizeMolecule(molecule,dict,optimizations);
    return;
  }

  if(!strcmp(action,"rmmol")) {
    argv = ParseArgv(argv,tMOLECULE,&molecule,0);
    RemoveMoleculeFromDict(dict,molecule);
    return;
  }
  Errorf("C_SetDict : unknown action '%s'",action);
}


/*
 * The dict fields
 */
static char *dictNChannelsDoc = "{} {Gets the number of channels of a &dict.}";
static char *dictNChannelsAllocDoc = "{} {Gets the allocation size for the array of channels of a &dict.}";
static char *dictChannelsDoc = "{} {Gets a &listv containing the channels of a &dict.}";
static char *dictSignalEnergyDoc = "{} {Gets the energy of the signal of a &dict.}";
static char *dictMaximaDoc = "{} {Gets the &maximadict sub-dictionary of a &dict.}";
static char *dictStftDoc = "{} {Gets a &listv containing all the &stft sub-dictionaries of a &dict.}";
void *GetDictSubDictContainerV(DICT dict,void **arg)
{
  unsigned char channel;
  SUBDICT subDict;
  char *field = ARG_G_GetField(arg);
  LISTV lv;
  STFT stft;
  MAXIMADICT maximaDict;
  unsigned short i;
  /* Documentation */
  if (dict == NULL) {
    if(!strcmp(field,"channels")) return(dictChannelsDoc);
    if(!strcmp(field,"signalEnergy")) return(dictSignalEnergyDoc);
    if(!strcmp(field,"maximadict")) return(dictMaximaDoc);
    if(!strcmp(field,"stft")) return(dictStftDoc);
  }
  
  if(!strcmp(field,"channels")) {
    if(dict->channels==NULL)     return(GetValueField(nullValue,arg));
    lv = TNewListv();
    for(channel=0;channel<dict->nChannels;channel++)
      AppendValue2Listv(lv,(VALUE)GetChannel(dict,channel));
    return(GetValueField(lv,arg));
  }
  if(!strcmp(field,"signalEnergy")) {
    return(GetFloatField(dict->signalEnergy,arg));
  }
  if(!strcmp(field,"maximadict")) {
    subDict=GetSubDict(dict,dict->nChannels,maximaDictType,NULL);
    if(subDict==NULL)     return(GetValueField(nullValue,arg));
    maximaDict=(MAXIMADICT)(subDict->dataContainer);
    return(GetValueField(maximaDict,arg));
  }
  if(!strcmp(field,"stft")) {
    lv = TNewListv();
    for(i = 0; i < dict->size;i++) {
      subDict=dict->subDicts[i];
      stft=(STFT)(subDict->dataContainer);
      if(GetTypeValue(stft)!=stftType) continue;
      AppendValue2Listv(lv,(VALUE)stft);
    }
    return(GetValueField(lv,arg));
  }
  return(NULL);
}
/*
 * The field list
 */
struct field fieldsDict[] = {
  "channels",GetDictSubDictContainerV,NULL,NULL,NULL,
  "signalEnergy",GetDictSubDictContainerV,NULL,NULL,NULL,
  "maximadict",GetDictSubDictContainerV,NULL,NULL,NULL,
  "stft",GetDictSubDictContainerV,NULL,NULL,NULL,
  NULL, NULL, NULL, NULL, NULL
};

/*
 * The type structure for DICT
 */

TypeStruct tsDict = {

  "{{{&dict} {This type is the basic type for time-frequency dictionaries for Matching Pursuit decompositions.}}}",  /* Documentation */

  &dictType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteDict,     /* The Delete function */
  NewDict,     /* The New function */
  
  NULL,       /* The copy function */
  ClearDict,       /* The clear function */
  
  ToStrDict,       /* String conversion */
  ShortPrintDict,   /* The Print function : print the object when 'print' is called */
  PrintInfoDict,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsDict,      /* The list of fields */
};

/************************************************************************/
/*
 *    This part is specific to MAXIMADICT sub-dictionaries
 */
/************************************************************************/

/*
 * Setting/Getting a MAXIMADICT sub-dictionary
 */
/* Get a sub-dictionary that contains Local Maxima. If none exists, return NULL. */
SUBDICT GetMaximaDictSubDict(DICT dict)
{
  unsigned short i;
  SUBDICT subDict;
  
  /* Does such a subDict exist ? */
  for(i = 0; i < dict->size; i++) {
    subDict = dict->subDicts[i];
    if(GetTypeValue(subDict->dataContainer)!=maximaDictType) continue;
    return(subDict);
  }
  return(NULL);
}
/*
 * Adds to a 'dict' a sub-dictionary that will contain (about) 'nTargetMaxima' local maxima
 * of the other sub-dictionaries. (does NOT check whether it already exists)
 * WARNING : this is very special, because 
 * -it will contain local maxima associated to the previously existing 'main' sub-dictionaries;
 * -all previously existing 'main' sub-dictionaries become 'auxiliary;
 * -all sub-dictionaries that are added subsequently (through AddSubDict) are forced to be 'auxiliary'.
 */
SUBDICT PrivateAddMaximaDictSubDict(DICT dict,unsigned long nMaximaTarget)
{
  unsigned short i;
  SUBDICT subDict;
  MAXIMADICT maximaDict;
  
  /* 
   * Create a new maximaDict 
   * It will contain local maxima associated to all the previously
   * existing 'main' sub-dictionaries, which become 'auxiliary' ones.
   */
  maximaDict = NewMaximaDict();
  for(i = 0; i < dict->size; i++) {
    subDict = dict->subDicts[i];
    if(subDict->flagMain) AddSubDict2MaximaDict(maximaDict,subDict);
  }
  maximaDict->nMaximaTarget = nMaximaTarget;

  /* Now we can create a new sub-dictionary  */
  subDict = NewSubDict();
  subDict->methods = &MaximaDictMethods;
  /* It is ALWAYS the (ONLY) main sub-dictionary and MULTICHANNEL */
  subDict->flagMain = YES;
  subDict->channel = dict->nChannels;
  subDict->dataContainer = (VALUE)maximaDict;
  /* In the next update of the dictionary, the newly added MAXIMADICT sub-dictionary   */
  /* will have to be updated, so we need to to reset [updateTimeIdMin updateTimeIdMax] */
  dict->updateTimeIdMin = 0;
  dict->updateTimeIdMax = dict->signalSize-1;
  return(PrivateAddSubDict(dict,subDict));
}

//struct subDictMethods WtransMethods;
//struct subDictMethods FourierMethods;




/* EOF */


