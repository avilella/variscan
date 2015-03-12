/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'mp' 2.0                           */
/*                                                                          */
/*      Copyright (C) 2000 R.Gribonval, E.Bacry and J.Abadia.               */
/*      Copyright (C) 2001-2002 Remi Gribonval                              */
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



#ifndef MP_BOOK_H

#define MP_BOOK_H

#include "signals.h"
#include "atom.h"

/*********************************************/
/* MOLECULES = arrays of dim*nChannels atoms */
/*********************************************/
typedef struct molecule {
  ValueFields;
  /* The data : array of atoms. */
  unsigned short dim;
  unsigned char  nChannels;
  unsigned short sizeAlloc;
  ATOM *atoms;
  /* Coefficient with which the molecule was selected: the sum of the atom->coeff2 */
  float coeff2;
} Molecule, *MOLECULE;

/* 
 * The basic variables and functions for &molecule variable management 
 */
extern char *moleculeType;
extern TypeStruct tsMolecule;
extern int   tMOLECULE;
extern int   tMOLECULE_;
extern MOLECULE NewMolecule(void);
extern MOLECULE TNewMolecule(void);
extern MOLECULE DeleteMolecule(MOLECULE molecule); 
extern void     ClearMolecule(MOLECULE molecule);
/* Generate an error if the molecule is empty */
extern void   CheckMoleculeNotEmpty(const MOLECULE molecule);

/*
 * If 'sizeAlloc' is smaller than molecule->dim*molecule->nChannels, an error is generated.
 * Else the allocation size of the array of atoms is adjusted to 'sizeAlloc'.
 * -the newly allocated part of the array is initialized to NULL atoms;
 * -the previously allocated part is kept (molecule->dim and molecule->nChannels are not changed)
 */
extern void   SizeMolecule(MOLECULE molecule,unsigned int sizeAlloc);
#define MP_DEFAULT_MOLECULE_SIZE 16

/*
 * Get the <k>th atom of a <channel> in a molecule. 
 * Generates an error if <k> is bigger than molecule->dim or <channel> larger than molecule->nChannels
 */
extern ATOM   GetMoleculeAtom(const MOLECULE molecule,unsigned char channel,unsigned short k);

/*
 * Adds an atom to a MONOCHANNEL molecule, and increases the molecule->dim. 
 * If the molecule is not monochannel, an error is generated. The allocation size in the molecule
 * is automatically managed, and an error is generated if the atom 'TFContent' does not
 * match that of those previously contained in the molecule.
 */
extern void   AddAtom2Molecule(MOLECULE molecule,ATOM atom);

/*
 * Adds a new channel (the atoms of a monochannel molecule) to a molecule, and increases the molecule->nChannels. 
 * If the added molecule is not monochannel, or the molecule is empty, an error is generated.
 * An error is also generated if the channel 'TFContent' or 'dim' does not match that of the molecule.
 * The allocation size in the molecule is automatically managed.
 * WARNING : the atoms of the channel are not copied, a REFERENCE is added, 
 * so the channel should rather be deleted than modified.
 */
extern void   AddChannel2Molecule(MOLECULE molecule,MOLECULE channelMol);

extern MOLECULE CopyMolecule(const MOLECULE moleculeIn,MOLECULE moleculeOut);
extern MOLECULE CopyMoleculeChannel(const MOLECULE moleculeIn,unsigned char channel,MOLECULE moleculeOut);


extern void   PrintMolecule(const MOLECULE molecule,char flagShort);

/************************************/
/* BOOKS : arrays of words          */
/************************************/
/* The number of signals in a book that are accessible using the '<i>book' syntax */
#define NBookSignals 10

typedef struct book{
  /* The fields of the ATFCONTENT structure */
  ATFContentFields;
  char * name;	/* name of the book. TODO : explain ? */
  /* The signals that are accessible using the '<i>book' syntax */
  SIGNAL theSignals[NBookSignals]; 
  
  /* The content : */
  int size;        	   /* The number of molecules in book TODO : unsigned */
  unsigned long sizeAlloc; /* The allocation size */
  MOLECULE *molecules;           /* The array[size] of molecules */
} Book,*BOOK;

/* 
 * The basic variables and functions for &book variable management 
 */
extern char *bookType;
extern TypeStruct tsBook;
extern int   tBOOK;
extern int   tBOOK_;
extern BOOK NewBook(void);
extern BOOK TNewBook(void);
/* Delete the arrays of molecules. The rest is kept. */
extern void DeleteBookMolecules(BOOK book); 
extern void ClearBook(BOOK book); 
extern BOOK CopyBook(const BOOK bookIn,BOOK bookOut);
extern BOOK DeleteBook(BOOK book);

extern BOOK GetBookCur(void);

/*
 * If 'sizeAlloc' is smaller than book->size, an error is generated.
 * Else the allocation size of the array of molecules is adjusted
 * to 'sizeAlloc'.
 * -the newly allocated part of the array is initialized to NULL molecules;
 * -the previously allocated part is kept (book->size is not changed)
 */
extern void     SizeBook(BOOK book,unsigned long sizeAlloc);
#define MP_DEFAULT_BOOK_SIZE 128
extern void     AddMolecule2Book    (BOOK book,MOLECULE molecule);
extern void     DeleteMoleculeFromBook(BOOK book,unsigned long rank);
/* Get the <n>th molecule in a book. Generates an error if <n> is bigger than book->size. */
extern MOLECULE 	GetBookMolecule  	(const BOOK book,unsigned long n);

// TODO : renew this 
/* I/O */
/* to deal with obsolete formats */
extern void ReadBookOld(BOOK book,FILE *stream,unsigned long forceMaxFreqId,SIGNAL residualEnergy);
extern void WriteBook(const BOOK book,char flagBinary,FILE * stream);
extern void ReadBook(BOOK book,FILE *stream);

/*
 * Functions and variables related to books
 */

extern void 	CheckBook	(const BOOK book);
extern void 	CheckBookNotEmpty(const BOOK book);


/* Utilities */
extern void 	AddShiftedSignals(SIGNAL input,SIGNAL output,long shift);
extern void 	DefineGraphBook(void);

/*************************************/
/* DICTIONARIES AND SUB-DICTIONARIES */
/*************************************/
typedef struct subDict {
  struct subDictMethods *methods;// The GetMax and Update methods 
  char flagMain;                 // If YES, then the GetMax searches through this sub-dictionary, else it does not
  unsigned char channel;         // Indicates to which of the nChannels of a dictionary this sub-dictionary corresponds.
  // When dict->nChannels==1 this is always 0.
  // When there are several channels, each sub-dictionary is either associated to a particular channel
  // (0<=channel<dict->nChannels, and then it is necessarily an 'auxiliary' sub-dictionary)
  // or it is 'multichannel' (channel==dict->nChannels) which indicates that it sums over all channels
  // some measure of correlation.
  char flagUpToDate;             // If NO, then the sub-dictionary needs to be updated
  VALUE dataContainer;           // Points to the actual data of the sub-dictionary
  struct dict *dict;             // The dictionary of which the sub-dictionary is a part
} SubDict, *SUBDICT;

#define DICT_MAX_NCHANNELS 8

typedef struct dict {
  ATFContentFields;
  // The array of analyzed channel(s).
  unsigned char nChannels;
  unsigned char nChannelsAlloc;
  SIGNAL *channels;
  float signalEnergy;

  // The range [updateTimeIdMin, updateTimeIdMax] where an update of the 
  // sub-dictionaries has to take place because the channels have changed.
  // When an update has just taken place, it is the empty range
  // [dict->signalSize,0]
  unsigned long updateTimeIdMin;
  unsigned long updateTimeIdMax;
  MOLECULE removedMolecule;
  // The number of sub-dictionaries
  unsigned short size;
  // The allocation size for the array of sub-dictionaries
  unsigned short sizeAlloc;
  // The array of sub-dictionaries
  SUBDICT* subDicts;
} *DICT;

typedef struct subDictMethods {
  /* The GetMax Method : locates the maximum of a SUBDICT over a search domain which is
   * the intersection of domains specified by tokens in 'searchRange'. The function checks
   * that the provided SUBDICT is up to date (else an error is generated).
   *
   * ***
   * Example : for &stft sub-dictionaries, the following tokens are understood
   *           "causal", {"timeId" (<range>|<num>)}, {"time" (<range>|<num>)},
   *           {"freqId" (<range>|<num>)}, {"freq" (<range>|<num>)}
   * ***
   *         
   * TODO: syntax for getting help on the tokens!
   *
   * If the search domain is empty, we return NO, set *pMaxValue==0.0
   *  and the content of 'result' is unspecified.
   * If the search domain is non empty but the maximum is zero, we return YES, set *pMax==0.0
   *  and the content of 'result' is unspecified.
   * In any other case we return YES and set 'result', which can be either a &listv or a &molecule.
   *
   * -if 'result' is a &listv, we set it to a value that can be used as 'searchRange' in
   *  a future call of GetMax so that the search domain is non empty and as small as possible
   *  around the location of the maximum.
   *
   *  ***
   *  Example : for a &stft sub-dictionary, 'result' will be set to {{"timeId" <maxTimeId>} {"freqId" <maxFreqId>}}.
   *  ***
   *
   * -if 'result' is a &molecule, we set its content according to the maximum found  in the search domain
   */
  char (*GetMax)(SUBDICT subDict,LISTV searchRange,float *pMaxValue,VALUE result);
  
  /* The Update method : update the content of the sub-dictionary if necessary.
   * -do nothing if the subDict is already up to date (flagUpToDate==YES);
   * -do the 'minimal' amount of recomputation, taking into account 
   *   the fact that the analyzed channels have only changed between [updateTimeIdMin,updateTimeIdMax]
   *     of the 'parent' dict;
   * -take care of the update of auxiliary sub-dictionaries if necessary 
   */
  void (*Update)(SUBDICT subDict);
} SubDictMethods;

// 
// The basic variables and functions for &dict variable management 
//
extern char *dictType;
extern TypeStruct tsDict;
extern int  tDICT;
extern int  tDICT_;
extern DICT NewDict();
extern DICT TNewDict(void);
extern DICT DeleteDict();
extern void ClearDict();
extern void PrintDict(const DICT,char flagShort);
extern DICT GetDictCur(void);

// Function that generates an error if the dictionary does not contain any sub-dictionary
extern void CheckDictNotEmpty(const DICT);
// If 'sizeAlloc' is smaller than dict->size, an error is generated.
// Else the allocation size of the array of sub-dictionaries is adjusted :
// -the newly allocated part of the array is initialized to NULL sub-dictionaries;
// -the previously allocated part is kept (dict->size is not changed)
extern void SizeDict(DICT,unsigned short sizeAlloc);
// The default number of sub-dictionaries
#define MP_DEFAULT_DICT_SIZE 16

//
// Functions to access/add DICT data
//
// If dict->channels is NULL, an error is generated
extern SIGNAL  GetChannel(DICT,unsigned char channel);
// If the subDictType is unknown, or the parameters are irrelevant for the type, an error is generated.
// Else returns the first sub-dictionary that matches the type and parameters (NULL is none matches)
extern SUBDICT GetSubDict(DICT dict,unsigned char channel,char *subDictType,LISTV parameters);

// Change the dx,x0 of a dictionary : change dx for all signals
// as well as all sub-dictionaries.
extern void   SetDictDX(DICT dict,float dx);
extern void   SetDictX0(DICT dict,float x0);


//
// The main functionalities of a dictionary
//

// Shorthands for some methods on sub-dictionaries
#define UpdateSubDict(subDict)  ((void (*)(SUBDICT))((subDict)->methods->Update))((subDict))

#define GetMaxSubDict(subDict,searchRange,pMaxValue,result) \
       ((char (*)(SUBDICT,LISTV,float *,VALUE))((subDict)->methods->GetMax))((subDict),(searchRange),(pMaxValue),(VALUE)(result))

// Update all the 'main' sub-dictionaries, and the necessary 'aux' sub-dictionaries
// using the 'removedMolecule' if necessary, and resets [updateTimeIdMin,updateTimeIdMax].
// The state of the dictionary after a call to this function is as follows :
// 1/if there is no sub-dictionary of local maxima : all sub-dictionaries are up to date
// and [updaTimeIdMin,updateTimeIdMax] = [dict->signalSize-1,0] 
// (when a molecule is removed from the dictionary, updateTimeIdMin/Max decreases/increases)
// 2/if there is a sub-dictionary of local maxima  : this sub-dictionary is up to date,
// and the state of [updaTimeIdMin,updateTimeIdMax] depends on whether the update of the 
// sub-dictionary of local maxima involved an 'initialization' or not.
extern void   UpdateDict(DICT dict);
// Returns a MOLECULE corresponding to the maximum of a DICT over a search range.
// If the specified search range is empty or the  maximum value is zero, we return NULL.
// The returned MOLECULE is not optimized (interpolate,chirped,...).
// The maximum is looked for only in 'main' sub-dictionaries, which should be up to date
// (else an error is generated).
// An error is also generated if [updateTimeIdMin,updateTimeIdMax] is not empty in the case
// when there is no sub-dictionary of local maxima.
extern MOLECULE GetMaxDict(DICT dict,LISTV searchRange);

// Performs a series of optimizations on a molecule, using a dictionary.
extern void OptimizeMolecule(MOLECULE molecule,DICT dict,LISTV optimizations);

// Builds a molecule and removes it from the channels of a dictionary.
// The signalEnergy is updated. An error is generated if it does not decrease.
// The [updateTimeIdMin,updateTimeIdMax] of the dict is enlarged if necessary.
// All sub-dictionaries are marked as out of date.
// The molecule is memorized in the dict->removedMolecule field for possible
// use at the next UpdateDict step.
// If a molecule has alredy been memorized, an error is generated.
extern void   RemoveMoleculeFromDict(DICT dict,MOLECULE molecule);

//
// Some functions that are specific to STFT sub-dictionaries
//
// Get a sub-dictionary that contains a stft with given {type,windowShape,windowSize}
// If none exists, return NULL.
extern SUBDICT GetStftSubDict(DICT dict,unsigned char channel,char stftType,char windowShape,unsigned long windowSize,LISTV options);
// The GetMax/Update methods corresponding to Stft sub-dictionaries.
extern SubDictMethods StftMethods;

//
// Some functions that are specific to MAXIMADICT sub-dictionaries (see below)
//
// Get a sub-dictionary that contains local maxima
// If none exists, return NULL.
extern SUBDICT GetMaximaDictSubDict(DICT);
// The GetMax/Update methods corresponding to MAXIMADICT sub-dictionaries.
extern SubDictMethods MaximaDictMethods;

/************************************/
// DICTIONARIES OF LOCAL MAXIMA 
/************************************/
typedef struct maximaDict {
  ValueFields;
  // The number of books containing local maxima
  unsigned short size;
  // The allocation size for the array of books
  unsigned short sizeAlloc;
  // The array of books : each book contains molecules 
  // that are the local maxima of some sub-dictionary.
  BOOK* books;
  // The array of sub-dictionaries from which the books are built.
  // books[i] is initialized from subDicts[i].
  SUBDICT* subDicts;

  // The target number of local maxima (sum over all books of book->size)
  // that we want at each initialization from the sub-dictionaries.
  unsigned long nMaximaTarget;
  // The threshold that was applied to keep only (about) nMaximaTarget local maxima
  // at the initialization step.
  float threshold;
  // The current number of local maxima, which should (strictly) decrease each time
  // an update is performed.
  unsigned long nMaxima;

  // The molecule that was selected using the GetMax function. It is useful
  // at the update step because to 
  // TODO : precise that !!!
  MOLECULE  maxMolecule;
} MaximaDict, *MAXIMADICT;

// 
// The basic variables and functions for &maximadict variable management 
//
extern char *maximaDictType;
extern TypeStruct tsMaximaDict;
extern int  tMAXIMADICT;
extern int  tMAXIMADICT_;
extern MAXIMADICT NewMaximaDict();
extern MAXIMADICT TNewMaximaDict(void);
extern MAXIMADICT DeleteMaximaDict();
extern void       ClearMaximaDict();
extern void       PrintMaximaDict(const MAXIMADICT,char flagShort);
extern MAXIMADICT GetMaximaDictCur(void);

// Function that generates an error if the dictionary does not contain any sub-dictionary
extern void CheckMaximaDictNotEmpty(const MAXIMADICT);
// If 'sizeAlloc' is smaller than maximaDict->size, an error is generated.
// Else the allocation size of the arrays of books/sub-dictionaries are adjusted :
// -the newly allocated part of the array is initialized to NULL books/sub-dictionaries;
// -the previously allocated part is kept (maximaDict->size is not changed)
void SizeMaximaDict(MAXIMADICT maximaDict,unsigned short sizeAlloc);

// Add a sub-dictionary : 
// -adds the corresponding book.
// -adds a reference to the subDict
// -the sub-dictionary should be a 'main' one and becomes an 'auxiliary' one
// WARNING : should only be used with subDicts that already belong to a dictionary!
void AddSubDict2MaximaDict(MAXIMADICT maximaDict,SUBDICT subDict);

/* EOF */
#endif

