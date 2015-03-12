/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'mp' 2.0                           */
/*                                                                          */
/*      Copyright (C) 2000 Remi Gribonval, Emmanuel Bacry and Javier Abadia.*/
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

/*************************************/
/*
 *	MAXIMADICT VARIABLES
 */
/*************************************/
char *maximaDictType = "&maximadict";

/*
 * Answers to the different print messages
 */
void ShortPrintMaximaDict(MAXIMADICT maximaDict)
{
  Printf("<&maximadict;%p>\n",maximaDict);
}

char *ToStrMaximaDict(MAXIMADICT maximaDict, char flagShort)
{
  static char str[30];
  
  sprintf(str,"<&maximadict;%p>",maximaDict);
  return(str);
}

void PrintInfoMaximaDict(MAXIMADICT maximaDict)
{
  PrintMaximaDict(maximaDict,NO);
}


MAXIMADICT TNewMaximaDict(void)
{
  MAXIMADICT maximaDict;
  
  maximaDict = NewMaximaDict();
  TempValue(maximaDict);
  return(maximaDict);
}

/*
 * Get the current maximaDict
 * (generate an error if there is none)
 */
MAXIMADICT GetMaximaDictCur(void)
{
  MAXIMADICT maximaDict;
  
  if(!ParseTypedValLevel_(levelCur,"objCur",NULL,(VALUE *)&maximaDict,maximaDictType)) Errorf1("");
  
  if (maximaDict == NULL) Errorf1("");
  
  AddRefValue(maximaDict);
  TempValue(maximaDict);
  
  return(maximaDict);
}

/*******************************************/
/*
 *	Basic data management for MAXIMADICT
 */				    
/*******************************************/
MAXIMADICT NewMaximaDict()
{
  MAXIMADICT maximaDict;
  
#ifdef DEBUGALLOC
  DebugType = "MaximaDict";
#endif
  
  maximaDict = (MAXIMADICT) Malloc(sizeof(struct maximaDict));
  InitValue(maximaDict,&tsMaximaDict);

  maximaDict->size = 0;
  maximaDict->sizeAlloc = 0;
  maximaDict->books = NULL;
  maximaDict->subDicts = NULL;

  maximaDict->nMaximaTarget = 0;
  maximaDict->nMaxima       = 0;
  maximaDict->threshold     = 0.0;

  maximaDict->maxMolecule       = NULL;
  return(maximaDict);
}

MAXIMADICT DeleteMaximaDict(MAXIMADICT maximaDict)
{
  unsigned short i;

  if (maximaDict == NULL)  Errorf("DeleteMaximaDict : NULL maximaDict");
  if (maximaDict->nRef==0) Errorf("*** Danger : trying to delete a temporary maximaDict\n");

  RemoveRefValue(maximaDict);
  if (maximaDict->nRef > 0) return(NULL);
  
  if(maximaDict->books) {
    for(i=0; i<= maximaDict->size; i++) {if(maximaDict->books[i]) 
	maximaDict->books[i] = DeleteBook(maximaDict->books[i]);
    }
    Free(maximaDict->books);
    maximaDict->books = NULL;
  }  
  // We do not delete the subDicts because we did not add a reference to them
  if(maximaDict->subDicts) {
    Free(maximaDict->subDicts);
    maximaDict->subDicts = NULL;
  }  
  maximaDict->size = 0;
  maximaDict->sizeAlloc = 0;

  maximaDict->nMaximaTarget = 0;
  maximaDict->nMaxima       = 0;
  maximaDict->threshold     = 0.0;

  if(maximaDict->maxMolecule) maximaDict->maxMolecule = DeleteMolecule(maximaDict->maxMolecule);

#ifdef DEBUGALLOC
  DebugType = "MaximaDict";
#endif
  
  Free(maximaDict);
  return(NULL);
}

void ClearMaximaDict(MAXIMADICT maximaDict)
{
  unsigned short i;

  if (maximaDict == NULL)  Errorf("ClearMaximaDict : NULL maximaDict");
  
  if(maximaDict->books) {
    for(i=0; i<= maximaDict->size; i++) {if(maximaDict->books[i]) 
	maximaDict->books[i] = DeleteBook(maximaDict->books[i]);
    }
    // Note : we do not delete the array, we keep it for a later use
  }  
  if(maximaDict->subDicts) {
    for(i=0; i<= maximaDict->size; i++) {
      // We do not delete subDicts because they were not allocated here and were not added a reference.
      maximaDict->subDicts[i] = NULL;
    }
    // Note : we do not delete the array, we keep it for a later use
  }  
  maximaDict->size = 0;

  maximaDict->nMaximaTarget = 0;
  maximaDict->nMaxima       = 0;
  maximaDict->threshold     = 0.0;

  if(maximaDict->maxMolecule) maximaDict->maxMolecule = DeleteMolecule(maximaDict->maxMolecule);
}

// Prints the content of a maximaDictionary, in long or short form
// TODO : improve that
void PrintMaximaDict(const MAXIMADICT maximaDict,char flagShort)
{
  unsigned short i;
  BOOK book;
  Printf("size      = %d (sizeAlloc = %d)\n",maximaDict->size,maximaDict->sizeAlloc);
  Printf("nMaxima   = %d (target = %d)\n",maximaDict->nMaxima,maximaDict->nMaximaTarget);
  Printf("threshold = %g\n",maximaDict->threshold);
  Printf("maxmolecule :\n");
  if(maximaDict->maxMolecule) PrintMolecule(maximaDict->maxMolecule,YES);
  else Printf("NULL\n");
  for(i = 0; i < maximaDict->size; i++) {
    book = maximaDict->books[i];
    Printf("book[%d] size=%d\n",i,book->size);
  }
}

// Function that generates an error if the maximaDictionary does not contain any sub-dictionary
void CheckMaximaDictNotEmpty(const MAXIMADICT maximaDict)
{
  if(maximaDict == NULL)    Errorf("CheckMaximaDictNotEmpty : NULL maximaDict");
  if(maximaDict->size == 0) Errorf("CheckMaximaDictNotEmpty : empty maximaDict");
}

// If 'sizeAlloc' is smaller than maximaDict->size, an error is generated.
// Else the allocation size of the arrays of books/sub-dictionaries are adjusted :
// -the newly allocated part of the array is initialized to NULL books/sub-dictionaries;
// -the previously allocated part is kept (maximaDict->size is not changed)
void SizeMaximaDict(MAXIMADICT maximaDict,unsigned short sizeAlloc)
{
  unsigned short i;
  if(sizeAlloc<maximaDict->size) Errorf("SizeMaximaDict : cannot (re)allocate less than the number of books/sub-dictionaries");
  if(sizeAlloc==maximaDict->size) return;
  // Case of an first allocation
  if(maximaDict->sizeAlloc == 0) {
    maximaDict->books     =(BOOK *)    Calloc(sizeAlloc,sizeof(BOOK));
    maximaDict->subDicts  =(SUBDICT *) Calloc(sizeAlloc,sizeof(SUBDICT));
    maximaDict->sizeAlloc = sizeAlloc;
  }
  // Case of a resize
  else {
    maximaDict->books    =(BOOK *)    Realloc(maximaDict->books,sizeAlloc*sizeof(BOOK));
    maximaDict->subDicts =(SUBDICT *) Realloc(maximaDict->subDicts,sizeAlloc*sizeof(SUBDICT));
    // Initialize the newly allocated data, if necessary
    for(i = maximaDict->sizeAlloc; i < sizeAlloc; i++) {
      maximaDict->books[i]   =NULL;
      maximaDict->subDicts[i]=NULL;
    }
    maximaDict->sizeAlloc = sizeAlloc;
  }
}

// Add a sub-dictionary and creates the corresponding book for storing its local maxima.
// If the sub-dictionary is not a 'main' one an error is generated.
// The sub-dictionary becomes an 'auxiliary' one.
// WARNING : should only be used with subDicts that already belong to a dictionary!
void AddSubDict2MaximaDict(MAXIMADICT maximaDict,SUBDICT subDict) 
{
  if(subDict->dict==NULL)   Errorf("AddSubDict2MaximaDict : (Weired) the sub-dictionary does not belong to any dictionary!");
  if(subDict->flagMain==NO) Errorf("AddSubDict2MaximaDict : (Weired) the sub-dictionary is not a 'main' one");
  if(maximaDict->threshold!= 0.0 || maximaDict->nMaxima!=0 || maximaDict->maxMolecule!=NULL)
    Errorf("AddSubDict2MaximaDict : (Weired) maximaDict content has already been initialized %g %d %p",
	   maximaDict->threshold,maximaDict->nMaxima,maximaDict->maxMolecule);
  // In case we need to allocate more room
  if(maximaDict->size==maximaDict->sizeAlloc) {
    if(maximaDict->sizeAlloc==0) SizeMaximaDict(maximaDict,MP_DEFAULT_DICT_SIZE);
    else                         SizeMaximaDict(maximaDict,2*maximaDict->sizeAlloc);
  }
  maximaDict->books[maximaDict->size] = NewBook();
  maximaDict->subDicts[maximaDict->size]=subDict; // We do not add a reference because subDicts don't have ValueFields!
  subDict->flagMain = NO;
  maximaDict->size++;
}

/**************************************/
/* 
 * MANAGEMENT OF A LIST OF COEFF2S 
 */
/**************************************/
#define MAXCOEFFS_SIZEMIN 1<<13

typedef struct maxCoeffs {
  unsigned long size;
  unsigned long sizeAlloc;
  float *coeff2s;
  float coeff2Min;
  float coeff2Max;
} *MAXCOEFFS;

static MAXCOEFFS NewMaxCoeffs(void)
{
  MAXCOEFFS maxCoeffs = NULL;
  
#ifdef DEBUGALLOC
  DebugType = "MaxCoeffs";
#endif
  
  maxCoeffs = (MAXCOEFFS) Malloc(sizeof(struct maxCoeffs));
  maxCoeffs->size = maxCoeffs->sizeAlloc = 0;
  maxCoeffs->coeff2s = NULL;
  maxCoeffs->coeff2Min = 0.0;
  maxCoeffs->coeff2Max = 0.0;
  return(maxCoeffs);
}

static MAXCOEFFS DeleteMaxCoeffs(MAXCOEFFS maxCoeffs)
{
  if(maxCoeffs == NULL) Errorf("DeleteMaxCoeffs : NULL input");
  if(maxCoeffs->coeff2s) {
    Free(maxCoeffs->coeff2s);
    maxCoeffs->coeff2s = NULL;
    maxCoeffs->size = maxCoeffs->sizeAlloc = 0;
  }
  
#ifdef DEBUGALLOC
  DebugType = "MaxCoeffs";
#endif
  Free(maxCoeffs);
  return(NULL);
}

static void ClearMaxCoeffs(MAXCOEFFS maxCoeffs)
{
  if(maxCoeffs == NULL) Errorf("ClearMaxCoeffs : NULL input");
  maxCoeffs->size = 0;
  maxCoeffs->coeff2Min = 0.0;
  maxCoeffs->coeff2Max = 0.0;
  // Note that we do NOT de-allocate the array and do not initialize it
}

static void DoubleSizeMaxCoeffs(MAXCOEFFS maxCoeffs)
{
  if(maxCoeffs == NULL)  Errorf("DoubleSizeMaxCoeffs : NULL input");
  
  // Case of a first allocation
  if(maxCoeffs->sizeAlloc == 0) {
    maxCoeffs->coeff2s   = FloatAlloc(MAXCOEFFS_SIZEMIN);
    maxCoeffs->sizeAlloc = MAXCOEFFS_SIZEMIN;
  } 
  // Case of a resize
  else {
    /* Reallocation */
    maxCoeffs->coeff2s = (float *)Realloc(maxCoeffs->coeff2s,2*maxCoeffs->sizeAlloc*sizeof(float));
    /* Update the allocated size */
    maxCoeffs->sizeAlloc *= 2;
  }
}

static void AppendCoeff2(MAXCOEFFS maxCoeffs,float coeff2)
{
  // Case when we need more space to store the coeff2s
  if(maxCoeffs->size == maxCoeffs->sizeAlloc) DoubleSizeMaxCoeffs(maxCoeffs);
  
  /* Appending */
  maxCoeffs->coeff2s[maxCoeffs->size] = coeff2;
  maxCoeffs->size++;
  
  /* Updating the bounds : first appended coeff2 or ... */
  if (maxCoeffs->size == 1 || coeff2 < maxCoeffs->coeff2Min) 
    maxCoeffs->coeff2Min = coeff2;
  if (coeff2 > maxCoeffs->coeff2Max) 
    maxCoeffs->coeff2Max = coeff2;
}

/**********************************************************/
/* 
 *	FIND THE RIGHT THRESHOLD IN ORDER TO GET NMAXIMA
 */
/**********************************************************/

/* Quick-Sort procedure */
static int   CmpFloat(const void* pArg1, const void* pArg2)
{
  float arg1,arg2;
  
  arg1 = *(float *)pArg1;
  arg2 = *(float *)pArg2;
  if(arg1 > arg2)
    return(-1);
  if(arg1 < arg2)
    return(1);
  return(0);
}

// Returns the threshold such that "exactly" 'nMaximaTarget' coeff2s are above 
// or equal to it. If the total number of coeff2s is too small, the threshold is 
// simply the minimum value of the coeff2s.
static float QSortFindThreshold(MAXCOEFFS maxCoeffs,unsigned long nMaximaTarget)
{
  qsort(maxCoeffs->coeff2s,maxCoeffs->size,sizeof(float),&CmpFloat);
  return(maxCoeffs->coeff2s[MIN(nMaximaTarget,maxCoeffs->size)-1]);
}

static unsigned long CountMaximaAboveThreshold(MAXCOEFFS maxCoeffs,float threshold)
{
  unsigned long i;
  unsigned long n = 0;
  for(i = 0; i < maxCoeffs->size; i++) {
    if(maxCoeffs->coeff2s[i] >= threshold)  n++;
  }
  return(n);
}

// Returns a threshold such that "approximately" 'nMaximaTarget' coeff2s are above 
// or equal to it. If the total number of coeff2s is too small, the threshold is 
// simply the minimum value of the coeff2s.
// By "approximately" it is meant that 
//   nMaximaTarget/MAXCOEFF2_TOLERANCE <= nMaxima <= nMaximaTarget*MAXCOEFF2_TOLERANCE
#define MAXCOEFF2_TOLERANCE 1.2
static float DichotomyFindThreshold(MAXCOEFFS maxCoeffs,unsigned long nMaximaTarget)
{
  // The target range of 
  unsigned long nMaximaTargetMin = nMaximaTarget/MAXCOEFF2_TOLERANCE;
  unsigned long nMaximaTargetMax = nMaximaTarget*MAXCOEFF2_TOLERANCE;
  // The minimum threshold and the corresponding number of maxima
  float	thresholdMin	= 0.0;
  unsigned long nMaximaMin	= maxCoeffs->size;
  // The maximum threshold and the corresponding number of maxima
  float	thresholdMax	= maxCoeffs->coeff2Max;
  unsigned long nMaximaMax	= 0;
  // The 'average' threshold and the corresponding number of maxima
  float	thresholdAverage;
  unsigned long nMaximaAverage;
  
  // For debug display only : count the number of iterations in the dichotomy
  unsigned long nSteps = 0;
  
  // In case there are not enough maxima available
  if(nMaximaMin < nMaximaTarget) return(thresholdMin); // TODO : replace nMaximaTarget with nMaximaTargetMax
  // There are enough maxima available, we have to find the right threshold
  // We loop until one of the extremities of the range [nMaximaMax,nMaximaMin] 
  // is in the range [nMaximaTargetMin,nMaximaTargetMax]
  while((nMaximaMax<nMaximaTargetMin) && (nMaximaMin>nMaximaTargetMax)) {
    // Count maxima above the 'average' threshold
    // TODO : look at the shape of decay of maxima to change the 'AVERAGE' 
    // (example : threshold = sqrt(thresholdMin*thresholdMax) ???)
    thresholdAverage = (thresholdMin+thresholdMax)/2;
    nMaximaAverage   = CountMaximaAboveThreshold(maxCoeffs,thresholdAverage);
    // Case 1 : the 'average' threshold is fine
    if(INRANGE(nMaximaTargetMin,nMaximaAverage,nMaximaTargetMax)) 
      return(thresholdAverage); 
    // Case 2 : the 'average' threshold is too large
    if(nMaximaAverage < nMaximaTargetMin) {
      thresholdMax = thresholdAverage;
      nMaximaMax   = nMaximaAverage;
    }
    // Case 3 : the 'average' threshold is too small
    if(nMaximaAverage > nMaximaTargetMax) {
      thresholdMin = thresholdAverage;
      nMaximaMin   = nMaximaAverage;
    }
  }
  // This code should never be reached !
  Errorf("Should return a value");
  return(0.0);
}

/* 
 * Find the local maximas from a STFT that are above a threshold
 * If STFT is not up to date, an error is generated.
 * If book!=NULL, appends to it the molecules corresponding to the found maxima if .
 * If maxCoeffs!=NULL, appends to it the value of the local maximum.
 * and thresholds them to append them in a book.
 * The molecules are neither interpolated nor chirped.
 */
static void AppendMaximaFromStftSubDict(SUBDICT subDict,char flagCausal,char flagTimeMaxima,char flagFreqMaxima,
					float threshold,BOOK book,MAXCOEFFS maxCoeffs)
{
  STFT   stft;
  MOLECULE molecule;
  // TODO : change to unsigned long when prototype of QuantizeRangeLarge is changed
  int timeIdMin,timeIdMax;
  unsigned long timeId;
  float *prevCoeffs,*coeffs,*nextCoeffs;
  float prevCoeff2,coeff2,nextCoeff2;
  unsigned long freqId,f;
  float belowCoeff2,aboveCoeff2;

  // TODO : replace with simple explicit arguments to GetMaxStftSubDict ???
  static LISTV searchRange = NULL;
  static LISTV lvTime = NULL;
  static LISTV lvFreq = NULL;
  if(searchRange==NULL) {
    searchRange = NewListv();
    lvTime = NewListv();
    lvFreq = NewListv();
    AppendStr2Listv(lvTime,"timeId");AppendInt2Listv(lvTime,0);
    AppendStr2Listv(lvFreq,"freqId");AppendInt2Listv(lvFreq,0);
    AppendValue2Listv(searchRange,(VALUE)lvTime);
    AppendValue2Listv(searchRange,(VALUE)lvFreq);
  }
  // Basic checking :
  // TODO : place in calling generic function
  if(subDict->flagUpToDate==NO)
    Errorf("AppendMaximaFromStftSubDict : subDict is out of date");
  stft = (STFT)(subDict->dataContainer);
  // TODO : remove when there is a calling generic function
  if(GetTypeValue(stft)!=stftType)
    Errorf("AppendMaximaFromStftSubDict : subDict is not of type '%s' but '%s'",stftType,GetTypeValue(stft));

  if(stft->type!=RealStft && stft->type!=HighResStft && stft->type!=HarmoStft)
    Errorf("AppendMaximaFromStftSubDict : cannot treat a stft of type '%s'",StftType2Name(stft->type));

  // Initializing the book if necessary
  if(book != NULL) {
    ClearBook(book);
    CopyFieldsTFContent(stft,book);
  }
  // The time range search 
  // TODO : simplify ?
  if(flagCausal) QuantizeRangeLarge(stft->firstp,stft->lastp,stft->tRate,&timeIdMin,&timeIdMax);
  else           QuantizeRangeLarge(0,stft->signalSize,stft->tRate,&timeIdMin,&timeIdMax);
  timeIdMin = MAX(timeIdMin,0);
  timeIdMax = MIN(timeIdMax,stft->tRate*(stft->nFrames-1));
  // Loop on time
  for(timeId = timeIdMin; timeId <= timeIdMax;timeId += stft->tRate) {
    // Get the stft 'vertical' data
    prevCoeffs    = nextCoeffs    = NULL;
    GetStftData(stft,timeId,&coeffs,NULL);
    // When not at the border, get the previous and next 'vertical' slices of data
    if(timeId != timeIdMin && timeId != timeIdMax) {
      GetStftData(stft,timeId-stft->tRate,&prevCoeffs,NULL);
      GetStftData(stft,timeId+stft->tRate,&nextCoeffs,NULL);
    }
    // Loop on frequency
    for(freqId  = stft->freqIdMin; freqId <= stft->freqIdMax; freqId += stft->fRate) {
      // Get the 'center' coeff2 at location (timeId,freqId)
      // TODO : GetStftValue(timeId,freqId) ??
      f  = (freqId-stft->freqIdMin)/stft->fRate;
      coeff2 = coeffs[f];
      // Case where we look for maxima over time and we are not at the (time) border
      if(flagTimeMaxima && prevCoeffs != NULL && nextCoeffs != NULL) {
	prevCoeff2 = prevCoeffs[f];
	nextCoeff2 = nextCoeffs[f];
	// Detection ! Should be a local maximum, and above the threshold
	if (coeff2 >= threshold && prevCoeff2 <= coeff2 && coeff2 > nextCoeff2) {
	  if(maxCoeffs) AppendCoeff2(maxCoeffs,coeff2);
	  if(book) {
	    SetListvNthFloat(lvTime,1,timeId);
	    SetListvNthFloat(lvFreq,1,freqId);
	    molecule = NewMolecule();
	    GetMaxSubDict(subDict,searchRange,NULL,(VALUE)molecule);
	    AddMolecule2Book(book,molecule);
	  }
	  // We don't want to add the molecule a second time if it is also a freq maximum!
	  continue;
	}
      }
      // Case where we look for maxima over frequency and we are not at the (freq) border
      if (flagFreqMaxima && freqId != stft->freqIdMin && freqId != stft->freqIdMax) {
	belowCoeff2 = coeffs[f-1];
	aboveCoeff2 = coeffs[f+1];
	// Detection ! Should be a local maximum, and above the threshold
	if (coeff2 >= threshold && belowCoeff2 <= coeff2 && coeff2 > aboveCoeff2) {
	  if(maxCoeffs) AppendCoeff2(maxCoeffs,coeff2);
	  if(book) {
	    SetListvNthFloat(lvTime,1,timeId);
	    SetListvNthFloat(lvFreq,1,freqId);
	    molecule = NewMolecule();
	    GetMaxSubDict(subDict,searchRange,NULL,(VALUE)molecule);
	    AddMolecule2Book(book,molecule);
	  }
	  // We don't want to add the molecule a second time if it is also a ?? maximum!
	  continue;
	}
      }
    }
  }
}

// TODO : void AppendMaximaFromSubDict(SUBDICT subdict,char flagCausal,char flagMaxType,float threshold,BOOK book,MAXCOEFFS maxCoeffs)
// with flagMaxType = DictMaxTime|DictMaxFreq|DictMaxScale| ....

// Inits a maximaDict : clears all local maxima and finds new ones from 
// the sub-dictionaries, which should be up to date (else an error is generated).
static void PrivateInitMaximaDict(MAXIMADICT maximaDict) {
  unsigned short i;
  BOOK book;
  SUBDICT subDict;
  static MAXCOEFFS maxCoeffs = NULL;

  // Initialize maxCoeffs
  if(maxCoeffs == NULL)  maxCoeffs = NewMaxCoeffs();
  else                   ClearMaxCoeffs(maxCoeffs);

  // First loop to get the values of the local maxima
  for(i = 0; i < maximaDict->size; i++) {
    subDict = maximaDict->subDicts[i];
    // TODO : check if we want both time and freq maxima, and if flagCausal==YES
    AppendMaximaFromStftSubDict(subDict,YES,YES,YES,0.0,NULL,maxCoeffs);
  }
  // Find the right threshold
  //  maximaDict->threshold = QSortFindThreshold(maxCoeffs,maximaDict->nMaximaTarget);
  maximaDict->threshold = DichotomyFindThreshold(maxCoeffs,maximaDict->nMaximaTarget);

  // Second loop to add the maxima greater than the threshold to the book
  maximaDict->nMaxima = 0;
  for(i = 0; i < maximaDict->size; i++) {
    subDict = maximaDict->subDicts[i];
    book    = maximaDict->books[i];
    // TODO : check if we want both time and freq maxima, and if flagCausal==YES
    AppendMaximaFromStftSubDict(subDict,YES,YES,YES,maximaDict->threshold,book,NULL);
    maximaDict->nMaxima += book->size;
    // TODO : perform optimization of molecules here if we want to
  }
}



/************************************************************************/
/*
 *    Implementation of the SUBDICT methods 
 *    for MAXIMADICT sub-dictionaries
 *  TODO : help/doc on these functions
 */
/************************************************************************/
char GetMaxMaximaDictSubDict(SUBDICT subDict,LISTV searchRange,float *pMaxValue,VALUE result) 
{
  MAXIMADICT maximaDict = (MAXIMADICT)(subDict->dataContainer);
  unsigned short i;
  BOOK   book;
  unsigned long n;
  MOLECULE maxMolecule;
  MOLECULE molecule;
  //ATOM   atom;

  // Checking arguments
  if(subDict->flagUpToDate==NO) Errorf("GetMaxMaximaDictSubDict : subDict is out of date!");
  if(searchRange) Errorf("GetMaxMaximaDictSubDict : searchRange is non NULL");
  if(pMaxValue) Errorf("GetMaxMaximaDictSubDict : pMaxValue is non NULL");
  if(GetTypeValue(result)!=moleculeType) Errorf("GetMaxMaximaDictSubDict : result is not a &mol");
  
  // Looks for the maximum over the maximaDict
  maxMolecule = NULL;
  // If the search range was empty we return NO
  for(i = 0; i < maximaDict->size; i++) {
    book = maximaDict->books[i];
    for(n = 0; n < book->size; n++) {
      molecule = GetBookMolecule(book,n);
      //atom = GetMoleculeAtom(molecule,0);
      // TODO : Skip molecules which first atom is not in the search range
      //if(!INRANGE(windowSizeMin,atom->windowSize,windowSizeMax)) continue;
      //if(!INRANGE(timeIdMin,atom->timeId,timeIdMax)) continue;
      //if(!INRANGE(freqIdMin,atom->freqId,freqIdMax)) continue;
      // Case where we encounter the first atom have 
      // Case where we encounter a better molecule
      if(maxMolecule==NULL || molecule->coeff2 > maxMolecule->coeff2) {
	maxMolecule = molecule;
	continue;
      }
    }
  }
  if(maxMolecule==NULL) return(NO);

  // DEBUG : TODO : remove when this can no longer occur!
  if(isnan(maxMolecule->coeff2) || maxMolecule->coeff2 <= 0.0)  Errorf("GetMaxMaximaDictSubDict : maxMolecule coeff2=%g!",maxMolecule->coeff2);

  // Fill the result
  CopyMolecule(maxMolecule,(MOLECULE)result);
  // Memorizes the maxMolecule
  maximaDict->maxMolecule = maxMolecule;
  AddRefValue(maxMolecule);
  return(YES);
}

/******************************************************/
/*
 * 	UPDATING THE MAXIMA MOLECULES
 */
/******************************************************/
/*
 * Function to monitor the update of an atom by itself
 */
static int CompareAtoms(ATOM atomReal,ATOM atomComplex)
{
  if(atomReal->windowSize==atomComplex->windowSize &&
     (int) atomReal->timeId == (int) atomComplex->timeId &&
     (int) atomReal->freqId == (int) atomComplex->freqId)
    return(YES);
  else
    return(NO);
}


/*
 * Updating an atom, given a chosen atom. 
 */
extern void RCAtomInnerProduct(const ATOM atomR,const ATOM atomC,char flagForceNumeric,float *pReal,float *pImag);

void UpdateMaximaAtom(ATOM atom,ATOM optAtom)
{
  float re,im;
  
  /* Checkings */
  CheckAtomReal(atom);
  CheckAtomReal(optAtom);
  
  /*
   * Inner-product between the two atoms
   * The updated one is considered complex
   * The other is real and normalized      
   */
  RCAtomInnerProduct(optAtom,atom,NO,&re,&im);
  
  /* Case of atom ORTHOGONAL to atom : no update is needed */
  if (re == 0 && im == 0) {
    CastAtomReal(atom);
    return;
  }
  
  /* Case when we need to update */
  
  /* Update atom complex inner-product */
  atom->coeffR -= sqrt(optAtom->coeff2)*re;
  atom->coeffI -= sqrt(optAtom->coeff2)*im;
  
  /* Keeping the extreme frequencies REAL */
  if(atom->freqId == 0 || atom->freqId == GABOR_NYQUIST_FREQID)
    atom->coeffI = 0.0;
  
  /*
   * Update atom phase,coeff2,... 
   */
  CastAtomReal(atom);
  
  /* WHEN WE UPDATE AN ATOM BY ITSELF, IT SHOULD BE ALWAYS DELETED */
  if(CompareAtoms(optAtom,atom)) {
    atom->coeffR	= atom->coeffI 	= 0.0;
    atom->coeff2	= 0.0;
  }
}

/*
 * Updating a molecule given a chosen optimized molecule to remove
 */
void UpdateMaximaMolecule(MOLECULE molecule,MOLECULE optMolecule)
{
  ATOM atom,optAtom;
  unsigned short kUpdate,k;
  unsigned char channel;
  CheckMoleculeNotEmpty(molecule);
  CheckMoleculeNotEmpty(optMolecule);
  if(molecule->nChannels!=optMolecule->nChannels)
    Errorf("UpdateMaximaMolecule : bad nChannels correspondance");

  molecule->coeff2 = 0.0;
  
  /* Loop on the atoms to update */
  for(channel = 0; channel < molecule->nChannels; channel ++) {
    for(kUpdate = 0; kUpdate < molecule->dim; kUpdate++) {
      atom = GetMoleculeAtom(molecule,channel,kUpdate);
      /* Loop on the chosen atoms */
      for(k = 0; k < optMolecule->dim; k++) {
	optAtom	 = GetMoleculeAtom(optMolecule,channel,k);
	/* Updating the atoms inner-product,phase and coeff2 */
	UpdateMaximaAtom(atom,optAtom);
	/* Updating accordingly the molecule coeff2 */
	molecule->coeff2 += atom->coeff2;
      }
    }
  }
}

void UpdateMaximaDictSubDict(SUBDICT subDict)
{
  MAXIMADICT maximaDict = (MAXIMADICT)(subDict->dataContainer);
  unsigned short i;
  DICT dict = subDict->dict;
  BOOK book;
  unsigned long n;

  MOLECULE molecule;

  // If already up to date, do nothing
  if(subDict->flagUpToDate) return;

  // Some checking
  if(GetTypeValue(maximaDict)!=maximaDictType)
    Errorf("UpdateMaximaDictSubDict (Weired) : data is not maximaDict!");

  // If there is a removedMolecule (which may have been optimized) we have to update 
  // the books of local maximas well as the maxMolecule.
  if(dict->removedMolecule) {
    // DEBUG
    //    Printf("%d",maximaDict->nMaxima);
    for(i = 0; i < maximaDict->size; i++) {
      book = maximaDict->books[i];
      // The increase of 'n' is only done when the molecule is not deleted
      for(n = 0; n < book->size; ) {
	molecule = GetBookMolecule(book,n);
	// Case where this is the molecule selected through GetMax (before optimizations):
	// it has to be deleted.
	if(molecule == maximaDict->maxMolecule) {
	  DeleteMoleculeFromBook(book,n);
	  maximaDict->maxMolecule = DeleteMolecule(maximaDict->maxMolecule);
	  maximaDict->nMaxima--;
	  // Do not increase 'n'
	  continue;
	}
	UpdateMaximaMolecule(molecule,dict->removedMolecule);
	// Case when we should we delete the current molecule : don't increase 'n'
	if (molecule->coeff2 < maximaDict->threshold) {
	  DeleteMoleculeFromBook(book,n);
	  maximaDict->nMaxima--;
	  continue;
	}
	n++;
      }
    }
    // DEBUG
    //    Printf("->%d\n",maximaDict->nMaxima);
  }

  // If the maximaDict is empty, we have to (re)initialize
  if(maximaDict->nMaxima==0) {
    // First, update the necessary sub-dictionaries
    for(i = 0; i < maximaDict->size; i++) {
      // Display a clock for waiting
      switch(i%4) {
      case 0 : Printf("\\"); Flush(); break;
      case 1 : Printf("|"); Flush(); break;
      case 2 : Printf("/"); Flush(); break;
      case 3 : Printf("-"); Flush(); break;
      }
      UpdateSubDict(maximaDict->subDicts[i]);
      Printf("\b");
    }
    // Then, compute the local maxima
    PrivateInitMaximaDict(maximaDict);
    // DEBUG
    //    Printf("Init %d\n",maximaDict->nMaxima);
    // Now all sub-dictionaries are up to date
    dict->updateTimeIdMin = dict->signalSize;
    dict->updateTimeIdMax = 0;
  }

  subDict->flagUpToDate = YES;
  // DEBUG :
  if(maximaDict->maxMolecule) Errorf("UpdateMaximaDictSubDict : maxMolecule not deleted!");
}

SubDictMethods MaximaDictMethods = {
  &GetMaxMaximaDictSubDict,
  &UpdateMaximaDictSubDict
};

/*
 * The maximaDict fields
 */
static char *maxDictBookDoc          = "{} {Gets a &listv containing all the &book of local maxima of a &maximadict.}";
static char *maxDictThresholdDoc     = "{} {Gets the threshold of a &maximadict.}";
static char *maxDictNMaximaDoc       = "{} {Gets the total number of maxima of a &maximadict.}";
void *GetMaximaDictFieldsV(MAXIMADICT maximaDict,void **arg)
{
  char *field = ARG_G_GetField(arg);
  LISTV lv;
  BOOK book;
  unsigned short i;
  /* Documentation */
  if (maximaDict == NULL) {
    if(!strcmp(field,"book"))   return(maxDictBookDoc);
    if(!strcmp(field,"thresh")) return(maxDictThresholdDoc);
    if(!strcmp(field,"nmax"))   return(maxDictNMaximaDoc);
  }
  
  if(!strcmp(field,"book")) {
    lv = TNewListv();
    for(i = 0; i < maximaDict->size;i++) {
      book=(BOOK)(maximaDict->books[i]);
      AppendValue2Listv(lv,(VALUE)book);
    }
    return(GetValueField(lv,arg));
  }
  if(!strcmp(field,"thresh")) {
    return(GetFloatField(maximaDict->threshold,arg));
  }
  if(!strcmp(field,"nmax")) {
    return(GetFloatField(maximaDict->nMaxima,arg));
  }
}

struct field fieldsMaximaDict[] = {
  "book", GetMaximaDictFieldsV, NULL, NULL, NULL,
  "thresh", GetMaximaDictFieldsV, NULL, NULL, NULL,
  "nmax", GetMaximaDictFieldsV, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL
};

/*
 * The type structure for MAXIMADICT
 */

TypeStruct tsMaximaDict = {

  "{{{&maximadict} {This type is the basic type for local maxima of time-frequency dictionaries for Matching Pursuit decompositions.}}}",  /* Documentation */

  &maximaDictType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteMaximaDict,     /* The Delete function */
  NewMaximaDict,     /* The New function */
  
  NULL,       /* The copy function */
  ClearMaximaDict,       /* The clear function */
  
  ToStrMaximaDict,       /* String conversion */
  ShortPrintMaximaDict,   /* The Print function : print the object when 'print' is called */
  PrintInfoMaximaDict,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsMaximaDict,      /* The list of fields */
};

/* EOF */

