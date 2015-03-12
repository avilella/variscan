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
#include "int_fsilist.h"

/**********************************/
/*
 * 	BOOK VARIABLES
 */
/**********************************/
char *bookType = "&book";
static char *defaultName="";

/*
 * Answers to the different print messages
 */

void ShortPrintBook(BOOK book)
{
  Printf("<&book[%d];%p>\n",book->size,book);
}

char *ToStrBook(BOOK book, char flagShort)
{
  static char str[30];
  
  if (book->name == defaultName || book->name == NULL) sprintf(str,"<&book;%p>",book);
  else sprintf(str,"<&book;%s>",book->name);
  return(str);
}

void PrintInfoBook(BOOK book)
{
  // TODO : finish that
  Printf("Book '%s'\n",book->name);
  Printf("  size %d (sizeAlloc %d)\n",book->size,book->sizeAlloc);
  Printf("  sampling rate: %.2f Hertz\n",1/book->dx);
  Printf("  signal size    : %d\n",book->signalSize);
}


/*
 * NumExtraction
 *
 * Signals (0m, 1m, ,...)
 */
static char *numdoc = "A book contains some signals that can be adressed using the syntax '<i>'. Among them, signal '0' is used by default for Matching Pursuit analysis.";
static void *NumExtractBook(BOOK book,void **arg)
{
  long n;
  char flagDot;
  
  /* doc */
  if (book == NULL) return(numdoc);

  n = ARG_NE_GetN(arg);
  flagDot = ARG_NE_GetFlagDot(arg);

  // Case of the extraction of a signal
  if(flagDot==NO) {
    if (n < 0 || n >= NBookSignals) {
      SetErrorf("'%d' is not a valid signal name in a book",n);
      return(NULL);
    }
    ARG_NE_SetResValue(arg,book->theSignals[n]);
    return(signalType);
  } else {
    SetErrorf("No signal '.%d' in a book",n);
    return(NULL);
  }
}

static void *GetOptionsBookV(BOOK book, void **arg)
{
  static char *opt[] = {NULL};
  return(opt);
}

static void *GetExtractInfoBookV(BOOK book, void **arg)
{
  char *field =  ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.nSignals = 1;
    extractInfo.dx = 1;
    extractInfo.xmin = 0;
    extractInfo.flags = EIIntIndex | EIErrorBound;
    flagInit = NO;
  }

  if(field==NULL || !strcmp(field,"")) {
    if (book->size == 0) {
      SetErrorf("No extraction of molecules in an empty book");
      return(NULL);
    }
    extractInfo.xmax = book->size-1;
    return(&extractInfo);
  }
  if(!strcmp(field,"sig")) {
    extractInfo.xmax = NBookSignals-1;
    return(&extractInfo);
  }

}

static char *sigdoc = "{[<n>]} {Gets the signal <n> of a book}";
static char *moldoc = "{[<n>]} {Gets the molecule <n> of a book}";
 
static void *GetBookV(BOOK book, void **arg)
{
  char *field = ARG_G_GetField(arg);
  FSIList *fsiList;
  SIGNAL sig;
  
  /* doc */
  if (book == NULL) {
    if (field==NULL || !strcmp(field,"")) return(moldoc);
    if (!strcmp(field,"sig")) return(sigdoc);
  }
  fsiList = ARG_G_GetFsiList(arg);

  if(fsiList==NULL) {
    if (field==NULL||!strcmp(field,""))    SetErrorf("The syntax is <book>[<n>]");
    if (!strcmp(field,"sig")) SetErrorf("The syntax is <book>.sig[<n>]");
    return(NULL);
  }

  if (fsiList->nx != 1) {
    SetErrorf("Only a single index can be used");
    return(NULL);
  }

  if(field==NULL || !strcmp(field,"")) {
    ARG_G_SetResValue(arg,GetBookMolecule(book,(int) FSI_FIRST(fsiList)));
    return(moleculeType);
  } 
  if (!strcmp(field,"sig")) {
    ARG_G_SetResValue(arg,book->theSignals[(int) FSI_FIRST(fsiList)]);
    return(signalType);
  } 
}

static  void *SetBookV(BOOK book, void **arg)
{
  char *field = ARG_S_GetField(arg);
  FSIList *fsiList;
  VALUE value = NULL;
  char *equal = NULL;
  MOLECULE molecule;
  VALUE *pValueRes = NULL;

   /* doc */
  if (book == NULL) {
    if (field==NULL || !strcmp(field,"")) return(moldoc);
    if (!strcmp(field,"sig")) return(sigdoc);
  }
  fsiList = ARG_S_GetFsiList(arg);

  if (field!=NULL && !strcmp(field,"sig")) {
    ARG_S_SetFsiList(arg,NULL);      
    return(SetSignalField(book->theSignals[(int) FSI_FIRST(fsiList)],arg));
  } 

  // No extraction is treated
  if(fsiList!=NULL) {
    SetErrorf("You must specify a field for the molecule");
    return(NULL);
  }
  equal = ARG_S_GetEqual(arg);
  value = ARG_S_GetRightValue(arg);
  // Case book += molecule
  if(*equal=='+' && value!=NULL && GetTypeValue(value)==moleculeType) {
    molecule = (MOLECULE) value;
    // Case of an empty book : we have to set the TFContent first
    if(book->size==0) CopyFieldsTFContent(GetMoleculeAtom(molecule,0,0),book);
    AddMolecule2Book(book,CopyMolecule(molecule,NULL));
    pValueRes  = ARG_S_GetResPValue(arg);
    *pValueRes = (VALUE)book;
    return(bookType);
  }
  return(NULL);
}



BOOK TNewBook(void)
{
  BOOK book = NewBook();
  TempValue(book);
  return(book);
}

/*
 * Get the current book
 * (generate an error if there is none)
 */
BOOK GetBookCur(void)
{
  BOOK book;
  if(!ParseTypedValLevel_(levelCur,"objCur",NULL,(VALUE *)&book,bookType)) Errorf1("");
  AddRefValue(book);
  TempValue(book);
  return(book);
}

/*******************************************/
/*
 *	Basic data management for BOOK
 */				    
/*******************************************/
static void InitBook(BOOK book)
{
  InitTFContent(book);

  book->size	  = 0;
  book->sizeAlloc = 0;
  book->molecules     = NULL;
}

BOOK NewBook()
{
  BOOK book;
  unsigned short i;
  
#ifdef DEBUGALLOC
  DebugType = "Book";
#endif
  
  book = (BOOK) Malloc(sizeof(Book));
  InitValue(book,&tsBook);
  InitBook(book);
   
  book->name = defaultName;
  
  for (i=0;i<NBookSignals;i++) book->theSignals[i] = NewSignal();
  
  return(book);    
}

// Delete the arrays of molecules. The rest is kept.
void   DeleteBookMolecules(BOOK book)
{
  unsigned long i;

  if(book == NULL) Errorf("DeleteBookMolecules : NULL book");
  // Delete all molecules 
  if(book->molecules) {
    for(i = 0; i < book->size; i++) 
      book->molecules[i] = DeleteMolecule(book->molecules[i]);
      // Note that we do not delete the array
  }

  book->size = 0;
}

void   ClearBook(BOOK book)
{
  if (book == NULL) Errorf("ClearBook : NULL book");
  // Deletes the content of the arrays of molecules (keeping the allocation of the array)
  DeleteBookMolecules(book);
  /* Inits the other fields */
  InitTFContent(book);
}

BOOK DeleteBook(BOOK book)
{
  unsigned short i;
  
  if (book == NULL) Errorf("DeleteBook : NULL book");
  if (book->nRef==0) Errorf("*** Danger : trying to delete a temporary book %s\n",book->name);
  
  RemoveRefValue(book);
  if (book->nRef > 0) return(NULL);
  
  DeleteBookMolecules(book);
  if(book->molecules) {
    Free(book->molecules);
    book->molecules = NULL;
  }
  /* Deallocates the name */
  if (book->name != NULL && book->name != defaultName) {
    Free(book->name);
    book->name = NULL;
  }
  book->name = defaultName;
  
  for (i=0;i<NBookSignals;i++) {
    DeleteSignal(book->theSignals[i]);
    book->theSignals[i] = NULL;
  }
  
#ifdef DEBUGALLOC
  DebugType = "Book";
#endif
  Free(book);	
  
  return(NULL);
}

// Copy the arrays of molecules. 
BOOK CopyBook(const BOOK bookIn,BOOK bookOut)
{
  MOLECULE moleculeOut=NULL;
  unsigned long n;

  /* Checking arguments */
  if(bookIn == NULL)    return(NULL);
  if(bookOut== NULL)    bookOut = NewBook();
  if(bookIn == bookOut) return(bookOut);

  CheckBook(bookIn);
  ClearBook(bookOut);
  /* Copying the 'tfContent' structure */
  CopyFieldsTFContent(bookIn,bookOut);
  // Copy the molecules : Set the right size first to make it faster ...
  SizeBook(bookOut,bookIn->size);
  for(n = 0; n < bookIn->size; n++) {
    moleculeOut = CopyMolecule(GetBookMolecule(bookIn,n),NULL);
    AddMolecule2Book(bookOut,moleculeOut);
  }
  return(bookOut);
}

// If 'sizeAlloc' is smaller than book->size, an error is generated.
// Else the allocation size of the array of molecules is adjusted
// to 'sizeAlloc'.
// -the newly allocated part of the array is initialized to NULL molecules;
// -the previously allocated part is kept (book->size is not changed)
void SizeBook(BOOK book,unsigned long sizeAlloc)
{
  unsigned long i;
  if(sizeAlloc<book->size) Errorf("SizeBook : cannot (re)allocate less than the number of molecules");
  if(sizeAlloc==book->size) return;
  // Case of an first allocation
  if(book->sizeAlloc == 0) {
    book->molecules           = (MOLECULE*) Calloc(sizeAlloc,sizeof(MOLECULE));
    book->sizeAlloc      = sizeAlloc;
  }
  // Case of a resize
  else {
    book->molecules           = (MOLECULE*) Realloc(book->molecules,sizeAlloc*sizeof(MOLECULE));
    // Initialize the newly allocated data, if necessary
    for(i = book->sizeAlloc; i < sizeAlloc; i++) 
      book->molecules[i] = NULL;
    book->sizeAlloc = sizeAlloc;
  }
}

/*--------------------------------------------------------------------------*/
/*
 * append a molecule into a book
 */
/*--------------------------------------------------------------------------*/

void AddMolecule2Book(BOOK book, MOLECULE molecule)
{
  /* checking the inputs */
  CheckBook(book);
  CheckMoleculeNotEmpty(molecule);
  CheckTFContentCompat(book,GetMoleculeAtom(molecule,0,0));

  // Case where we have to resize the book
  if(book->size == book->sizeAlloc) {
    if(book->sizeAlloc==0) SizeBook(book,MP_DEFAULT_BOOK_SIZE);
    else SizeBook(book,2*book->sizeAlloc); // Nota : it may be a bad strategy to double at each time !
  }
  
  // Now we can append the molecule
  book->molecules[book->size] = molecule;
  book->size++;
}

void DeleteMoleculeFromBook(BOOK book,unsigned long rank) 
{
  unsigned long n;
  CheckBookNotEmpty(book);
  if(!INRANGE(0,rank,book->size-1)) Errorf("DeleteMoleculeFromBook : rank %d is not in range [0 %d]",book->size-1);
  book->molecules[rank] = DeleteMolecule(book->molecules[rank]);
  // Piles up molecules to the left of the array
  // TODO : replace the pile up with
  // book->molecules[rank] = book->molecules[book->size-1]
  // book->molecules[book->size-1] = NULL;
  // book->size--;
  for(n = rank; n < book->size-1; n++) {
    book->molecules[n] = book->molecules[n+1];
  }
  book->molecules[book->size-1] = NULL;
  book->size--;
  // Note that we do NOT re-allocate to a smaller size
}
/* Get the n-th molecule , 0 <= n <= size-1 */
MOLECULE GetBookMolecule(const BOOK book,unsigned long n)
{
  // Some checkings
  CheckBookNotEmpty(book);
  if(n>=book->size)  Errorf("GetBookMolecule : Bad molecule number %d [%d %d]",n,0,book->size-1);
  return(book->molecules[n]);
}	





/* ALLOCATION */
void CheckBook(const BOOK book)
{
  CheckTFContent(book);
}


void CheckBookNotEmpty(const BOOK book)
{
  CheckBook(book);
  if(book->size == 0)    Errorf("CheckBookNotEmpty : empty book (run the pursuit first)");
}

/*
 * The fields of a book
 */
static char* nameDoc = "{[= <name>]} {Sets/Gets the name of a book}";
static char* sizeDoc = "{} {Gets the number of &mol in a book.}";
static char* sizeAllocDoc = "{[= <sizeAlloc>]} {Sets/Gets the allocation size for the array of &mol in a book. In case of a Set, <sizeAlloc> must be larger than book.size, else an error is generated. The previously allocated part is kept (book.size is not changed).}";

/*
 * 'name' field
 */
static void * GetNameBookV(BOOK book, void **arg)
{
  /* Documentation */
  if (book == NULL) return(nameDoc);
  return(GetStrField(book->name,arg));
}

static void * SetNameBookV(BOOK book, void **arg)
{
  /* doc */
  if (book == NULL) return(nameDoc);

  if (book->name==defaultName || book->name == NULL) {
    book->name=CharAlloc(1);
    book->name[0] = '\0';
  }
  return(SetStrField(&(book->name),arg));
}
  
static void *GetSizeBookV(BOOK book, void **arg)
{
  char *field = ARG_G_GetField(arg);
  
  /* Documentation */
  if (book == NULL) {
    if(!strcmp(field,"size"))      return(sizeDoc);
    if(!strcmp(field,"sizeAlloc")) return(sizeAllocDoc);
  }

  if(!strcmp(field,"size"))        return(GetIntField(book->size,arg));
  if(!strcmp(field,"sizeAlloc"))   return(GetIntField(book->sizeAlloc,arg));
}

static void * SetSizeAllocBookV(BOOK book, void **arg)
{
  int sizeAlloc;
  /* doc */
  if (book == NULL) return(sizeAllocDoc);
  // Init for += syntax
  sizeAlloc = book->sizeAlloc;
  if(SetIntField(&sizeAlloc,arg,FieldSPositive)==NULL) return(NULL);
  SizeBook(book,sizeAlloc);
}
  
/* 'tfContent' parameters */
static char *dxDoc = "{[= <dx>]} {Sets/Gets the abscissa step of the original signal of the book and all its atoms.}";
static char *x0Doc = "{[= <x0>]} {Sets/Gets the first abscissa of the original signal of the book and all its atoms.}";
void *SetDxBookV(BOOK book, void **arg)
{
  unsigned long n;
  MOLECULE molecule;
  unsigned short k;
  unsigned char channel;
  ATOM atom;
  /* Documentation */
  if (book == NULL) return(dxDoc);

  if(SetFloatField(&(book->dx),arg,FieldSPositive)==NULL) return(NULL);
  /* Update the atoms dx */
  for(n = 0; n < book->size; n++) {
    molecule = GetBookMolecule(book,n);
    for(channel = 0; channel < molecule->nChannels; channel++) {
      for(k = 0; k < molecule->dim; k++) {
	atom = GetMoleculeAtom(molecule,channel,k);
	atom->dx = book->dx;
      }
    }
  }
  return(numType);
}
 

void *SetX0BookV(BOOK book, void **arg)
{
  unsigned long n;
  MOLECULE molecule;
  unsigned short k;
  unsigned char channel;
  ATOM atom;

  /* Documentation */
  if (book == NULL) return(x0Doc);

  if(SetFloatField(&(book->x0),arg,0)==NULL) return(NULL);
  /* Update the atoms x0 */
  for(n = 0; n < book->size; n++) {
    molecule = GetBookMolecule(book,n);
    for(channel = 0; channel < molecule->nChannels; channel++) {
      for(k = 0; k < molecule->dim; k++) {
	atom = GetMoleculeAtom(molecule,channel,k);
	atom->x0 = book->x0;
      }
    }
  }
  return(numType);
}

/*
 * The extraction of signals that contain the fields of the molecules of a book
 */ 

static char* dimDoc        = "{} {Gets a &signal of size book.size containing the list of dimensions  of the molecules of a book (i.e. the number of atoms contained in each molecule). The dimension is larger than 1 only for books built using the Harmonic Matching Pursuit.}";
static char* wcoeff2Doc    = "{} {Gets a &signal of size book.size containing the list of 'coeff2' of the molecule in a book.}";
static char* windowSizeDoc = "{} {Gets a &signal of size book.size containing the list of 'windowSize' of the first atom of the molecules in a book.}";
static char* windowShapeDoc = "{} {Gets a &listv of size book.size containing the list of 'windowShape' of the first atom of the molecules of a book."
WindowShapeHelpString
"}";
static char* timeIdDoc     = "{} {Gets a &signal of size book.size containing the list of 'timeId' of the first atom of the molecules in a book..}";
static char* timeDoc       = "{} {Gets a &signal of size book.size containing the list of 'time' of the first atom of the molecules in a book.}";
static char* freqIdDoc     = "{} {Gets a &signal of size book.size containing the list of 'freqId' of the first atom of the molecules in a book.}";
static char* freqDoc       = "{} {Gets a &signal of size book.size containing the list of 'freq' of the first atom of the molecules in a book.}";
static char* chirpIdDoc    = "{} {Gets a &signal of size book.size containing the list of 'chirpId' of the first atom of the molecules in a book.}";
static char* chirpDoc      = "{} {Gets a &signal of size book.size containing the list of 'chirp' of the first atom of the molecules in a book.}";
static char* phaseDoc      = "{} {Gets a &signal of size book.size containing the list of 'phase' of the first atom of the molecules in a book.}";
static char* acoeff2Doc    = "{} {Gets a &signal of size book.size containing the list of 'coeff2' of the first atom of the molecules in a book.}";
static char* ggDoc         = "{} {Gets a &listv {real imag} of two &signal of size book.size containing the list of 'gg' of the first atom of the molecules in a book.}";

void *GetMoleculeFieldBookV(BOOK book, void **arg)
{
  char *field = ARG_G_GetField(arg);
  SIGNAL signal = NULL;
  SIGNAL signalIm = NULL;
  LISTV lv = NULL;
  unsigned long n;
  ATOM atom;
  float phase;

  /* Documentation */
  if (book == NULL) {
    if(!strcmp(field,"dim")) return(dimDoc);
    if(!strcmp(field,"wcoeff2")) return(wcoeff2Doc);
    if(!strcmp(field,"windowSize")) return(windowSizeDoc);
    if(!strcmp(field,"windowShape")) return(windowShapeDoc);
    if(!strcmp(field,"timeId")) return(timeIdDoc);
    if(!strcmp(field,"time")) return(timeDoc);
    if(!strcmp(field,"freqId")) return(freqIdDoc);
    if(!strcmp(field,"freq")) return(freqDoc);
    if(!strcmp(field,"chirpId")) return(chirpIdDoc);
    if(!strcmp(field,"chirp")) return(chirpDoc);
    if(!strcmp(field,"phase")) return(phaseDoc);
    if(!strcmp(field,"acoeff2")) return(acoeff2Doc);
    if(!strcmp(field,"gg")) return(ggDoc);
    Errorf("Weired : GetMoleculeFieldBooV : unknown field %s",field);
  }

  if(!strcmp(field,"windowShape")) {
    lv = TNewListv();
    for(n = 0; n < book->size; n++) {
      AppendStr2Listv(lv,WindowShape2Name(GetMoleculeAtom(GetBookMolecule(book,n),0,0)->windowShape));
    }
    return(GetValueField(lv,arg));
  }

  signal = TNewSignal();
  SizeSignal(signal,book->size,YSIG);
  signal->dx = 1;
  signal->x0 = 0;
  if(!strcmp(field,"dim")) {
    for(n = 0; n < book->size; n++) {
      signal->Y[n] = GetBookMolecule(book,n)->dim;
    }
  }
  if(!strcmp(field,"wcoeff2")) {
    for(n = 0; n < book->size; n++) {
      signal->Y[n] = GetBookMolecule(book,n)->coeff2;
    }
  }
  if(!strcmp(field,"windowSize")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0);
      signal->Y[n] = atom->windowSize;
    }
  }
  if(!strcmp(field,"timeId")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = atom->timeId;
    }
  }
  if(!strcmp(field,"time")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = TimeId2Time(book,atom->timeId);
    }
  }
  if(!strcmp(field,"freqId")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = atom->freqId;
    }
  }
  if(!strcmp(field,"freq")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = FreqId2Freq(book,atom->freqId);
    }
  }
  if(!strcmp(field,"chirpId")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = atom->chirpId;
    }
  }
  if(!strcmp(field,"chirp")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = ChirpId2Chirp(book,atom->chirpId);
    }
  }
  if(!strcmp(field,"phase")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      phase = atan2(atom->sinPhase,atom->cosPhase)/(2*M_PI);
      if (phase<0)     phase = 1+phase;
      if(phase >= 1.0) phase = phase-1.0;
      signal->Y[n] = phase;
    }
  }
  if(!strcmp(field,"acoeff2")) {
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = atom->coeff2;
    }
  }
  if(!strcmp(field,"gg")) {
    signalIm = TNewSignal();
    SizeSignal(signalIm,book->size,YSIG);
    signalIm->dx = 1;
    signalIm->x0 = 0;
    for(n = 0; n < book->size; n++) {
      atom = GetMoleculeAtom(GetBookMolecule(book,n),0,0); 
      signal->Y[n] = atom->realGG;
      signalIm->Y[n] = atom->imagGG;
    }
    lv = TNewListv();
    AppendValue2Listv(lv,(VALUE)signal);
    AppendValue2Listv(lv,(VALUE)signalIm);
    return(GetValueField(lv,arg));
  }

  return(GetValueField(signal,arg));
}
  
/*
 * The field list
 */
struct field fieldsBook[] = {
  "",GetBookV,SetBookV,GetOptionsBookV,GetExtractInfoBookV,  
  "sig",GetBookV,SetBookV,GetOptionsBookV,GetExtractInfoBookV,  
  "name",GetNameBookV,SetNameBookV,NULL,NULL,
  // The TFContent fields
  "dx",GetDxTFContentV,SetDxBookV,NULL,NULL,
  "x0",GetX0TFContentV,SetX0BookV,NULL,NULL,
  "signalSize",GetSignalSizeTFContentV,NULL,NULL,NULL,
  "freqIdNyquist",GetFreqIdNyquistTFContentV,NULL,NULL,NULL,
  // The array fields
  "size",GetSizeBookV,NULL,NULL,NULL,
  "sizeAlloc",GetSizeBookV,SetSizeAllocBookV,NULL,NULL,
  // The molecule fields
  "dim",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "wcoeff2",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "windowSize",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "windowShape",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "timeId",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "time",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "freqId",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "freq",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "chirpId",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "chirp",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "acoeff2",GetMoleculeFieldBookV,NULL,NULL,NULL,
  "phase",GetMoleculeFieldBookV,NULL,NULL,NULL,
#ifdef ATOM_ADVANCED
  "gg",GetMoleculeFieldBookV,NULL,NULL,NULL,
#endif // ATOM_ADVANCED
  NULL, NULL, NULL, NULL, NULL
};

/*
 * The type structure for BOOK
 */

TypeStruct tsBook = {

  "{{{&book} {This type is the basic type for storing the result of Matching Pursuit decompositions as an array of &mol's. \n \
- Operator + : book+molecule, appends a molecule at the end of the book.}}}",  /* Documentation */

  &bookType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteBook,     /* The Delete function */
  NewBook,     /* The New function */
  
  CopyBook,       /* The copy function */
  ClearBook,       /* The clear function */
  
  ToStrBook,       /* String conversion */
  ShortPrintBook,   /* The Print function : print the object when 'print' is called */
  PrintInfoBook,   /* The PrintInfo function : called by 'info' */

  NumExtractBook,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsBook,      /* The list of fields */
};


/* EOF */

