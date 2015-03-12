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
 * 	MOLECULE VARIABLES
 */
/**********************************/
char *moleculeType = "&mol";

/*
 * Answers to the different print messages
 */
 
void ShortPrintMolecule(MOLECULE molecule)
{
  Printf("<&mol[%d][%d];%p>\n",molecule->dim,molecule->nChannels,molecule);
}

char *ToStrMolecule(MOLECULE molecule, char flagShort)
{
  static char str[30];
  
  sprintf(str,"<&mol;%p>",molecule);
  return(str);
}


void PrintInfoMolecule(MOLECULE molecule)
{
  PrintMolecule(molecule,NO);
}

MOLECULE TNewMolecule(void)
{
  MOLECULE molecule;
  
  molecule = NewMolecule();
  TempValue(molecule);
  return(molecule);
}

/* ALLOCATION */
static void InitMolecule(MOLECULE molecule)
{
  if (molecule == NULL) Errorf("NULL argument for InitMolecule!");
  
  molecule->dim		= 0;
  molecule->nChannels	= 1;
  molecule->sizeAlloc	= 0;
  molecule->atoms	= NULL;

  molecule->coeff2      = 0.0;  
}

MOLECULE NewMolecule(void)
{
  MOLECULE molecule;
  
#ifdef DEBUGALLOC
  DebugType = "Molecule";
#endif
  
  molecule = (MOLECULE) Malloc(sizeof(Molecule));
  InitValue(molecule,&tsMolecule);
  InitMolecule(molecule);
  return(molecule);
}

MOLECULE DeleteMolecule(MOLECULE molecule)
{
  unsigned int i;

  if (molecule == NULL)  Errorf("NULL argument for DeleteMolecule!");
  if (molecule->nRef==0) Errorf("*** Danger : trying to delete a temporary molecule\n");

  RemoveRefValue(molecule);
  if(molecule->nRef > 0) return(NULL);
  
  if(molecule->atoms) {
    for(i = 0; i < molecule->dim*molecule->nChannels; i++) {
      if(molecule->atoms[i])
	molecule->atoms[i] = DeleteAtom(molecule->atoms[i]);
    }
    Free(molecule->atoms);
    molecule->atoms = NULL;
  }
  
#ifdef DEBUGALLOC
  DebugType = "Molecule";
#endif
  
  Free(molecule);
  return(NULL);
}

void ClearMolecule(MOLECULE molecule)
{
  unsigned int i;

  if (molecule == NULL) Errorf("NULL argument for ClearMolecule!");
  
  if(molecule->atoms) {
    for(i = 0; i < molecule->dim*molecule->nChannels; i++) {
      if(molecule->atoms[i])
	molecule->atoms[i] = DeleteAtom(molecule->atoms[i]);
    }
    molecule->dim = 0; // Note that we do NOT delete the array
    molecule->nChannels = 1;
  }
  molecule->coeff2 = 0.0;
}

// If 'sizeAlloc' is smaller than molecule->dim*molecule->nChannels, an error is generated.
// Else the allocation size of the array of atoms is adjusted to 'sizeAlloc'.
// -the newly allocated part of the array is initialized to NULL atoms;
// -the previously allocated part is kept (molecule->dim and molecule->nChannels are not changed)
void SizeMolecule(MOLECULE molecule,unsigned int sizeAlloc)
{
  unsigned int i;
  if(sizeAlloc<molecule->dim*molecule->nChannels) 
    Errorf("SizeMolecule : cannot (re)allocate less than the number of atoms");
  if(sizeAlloc==molecule->dim*molecule->nChannels) return;
  // Case of a first allocation
  if(molecule->sizeAlloc == 0) {
    molecule->atoms    = (ATOM*) Calloc(sizeAlloc,sizeof(ATOM));
    molecule->sizeAlloc = sizeAlloc;
  }
  // Case of a resize
  else {
    molecule->atoms          = (ATOM*) Realloc(molecule->atoms,sizeAlloc*sizeof(ATOM));
    // Initialize the newly allocated data, if necessary
    for(i = molecule->sizeAlloc; i < sizeAlloc; i++) 
      molecule->atoms[i] = NULL;
    molecule->sizeAlloc = sizeAlloc;
  }
}

/* METHODS */
void CheckMoleculeNotEmpty(const MOLECULE molecule)
{
  if(molecule == NULL)           Errorf("CheckMoleculeNotEmpty : NULL molecule");
  if(molecule->dim == 0)         Errorf("CheckMoleculeNotEmpty : molecule is of dimension 0");
  if(molecule->atoms[0] == NULL) Errorf("CheckMoleculeNotEmpty : first atom is NULL");
}

// Get the <k>th atom of a <channel> in a molecule. 
// Generates an error if <k> is bigger than molecule->dim or <channel> larger than molecule->nChannels
ATOM   GetMoleculeAtom(const MOLECULE molecule,unsigned char channel,unsigned short k)
{
  ATOM atom;
  CheckMoleculeNotEmpty(molecule);
  if(k>=molecule->dim)             Errorf("GetMoleculeAtom : atom number %d does not exist in this molecule",k);
  if(channel>=molecule->nChannels) Errorf("GetMoleculeAtom : channel number %d does not exist in this molecule",channel);
  /* Atoms are stacked as [k=0,channel=0][k=1,channel=0]...[k=dim-1,channel=0][k=0,channel=2]... */
  atom = molecule->atoms[channel*molecule->dim+k];
  if(atom==NULL) Errorf("GetMoleculeAtom : (Weired) atom number %d in channel %d is NULL",k,channel);
  return(atom);
}

// Adds an atom to a MONOCHANNEL molecule, and increases the molecule->dim. 
// If the molecule is not monochannel, an error is generated. The allocation size in the molecule
// is automatically managed, and an error is generated if the atom 'TFContent' does not
// match that of those previously contained in the molecule.
void AddAtom2Molecule(MOLECULE molecule,ATOM atom)
{
  ATOM firstAtom = NULL;
  // Checking the inputs
  if(molecule->nChannels>1)  Errorf("AddAtom2Molecule : molecule should be monochannel!");
  if(molecule->dim>0) {
    firstAtom = GetMoleculeAtom(molecule,0,0);
    CheckTFContentCompat(firstAtom,atom);
  }
  // Case where we have to resize the molecule
  if(molecule->sizeAlloc == molecule->dim) {
    if(molecule->sizeAlloc==0) SizeMolecule(molecule,MP_DEFAULT_MOLECULE_SIZE);
    else SizeMolecule(molecule,2*molecule->sizeAlloc); 
  }

  molecule->atoms[molecule->dim] = atom;
  molecule->dim++;
  molecule->coeff2 += atom->coeff2;
}

// Adds a new channel (the atoms of a monochannel molecule) to a molecule, and increases the molecule->nChannels. 
// If the added molecule is not monochannel, or the molecule is empty, an error is generated.
// An error is also generated if the channel 'TFContent' or 'dim' does not match that of the molecule.
// The allocation size in the molecule is automatically managed.
// WARNING : the atoms of the channel are not copied, a REFERENCE is added, 
// so the channel should rather be deleted than modified 
void AddChannel2Molecule(MOLECULE molecule,MOLECULE channelMol)
{
  unsigned short k;
  ATOM atom = NULL;
  /* Checking the input */
  if(channelMol->nChannels>1)  
    Errorf("AddChannel2Molecule : added 'channel' should be monochannel!");
  CheckMoleculeNotEmpty(molecule);
  if(molecule->dim!=channelMol->dim) 
    Errorf("AddChannel2Molecule : added channel and molecule should have the same dimension %d",molecule->dim);
  CheckTFContentCompat(GetMoleculeAtom(molecule,0,0),GetMoleculeAtom(channelMol,0,0));

  // Case where we have to resize the molecule
  if(molecule->sizeAlloc < molecule->dim*(molecule->nChannels+1)) {
    SizeMolecule(molecule,molecule->dim*(molecule->nChannels+1)); 
  }
  
  for(k = 0; k < molecule->dim; k++) {
    atom = GetMoleculeAtom(channelMol,0,k);
    molecule->atoms[molecule->nChannels*molecule->dim+k] = atom;
    AddRefValue(atom);
    molecule->coeff2 += atom->coeff2;
  }
  molecule->nChannels++;
}

MOLECULE CopyMolecule(const MOLECULE in,MOLECULE out) 
{
  unsigned int i;
  ATOM atom;

  if(in == NULL)  return(NULL);
  if(out == NULL) out = NewMolecule();
  if(in == out)   return(out);
  CheckMoleculeNotEmpty(in);
  ClearMolecule(out);
  SizeMolecule(out,in->dim*in->nChannels);
  for(i = 0; i < in->dim*in->nChannels; i++) {
    out->atoms[i] = CopyAtom(in->atoms[i],out->atoms[i]);
  }
  out->dim        = in->dim;
  out->nChannels  = in->nChannels;
  out->coeff2 	  = in->coeff2;
  return(out);
}

MOLECULE CopyMoleculeChannel(const MOLECULE in,unsigned char channel,MOLECULE out) 
{
  unsigned short k;
  ATOM atom;

  if(in == NULL)  return(NULL);
  CheckMoleculeNotEmpty(in);
  if(channel>=in->nChannels) Errorf("CopyMoleculeChannel : channel %d is too big",channel);
  if(out == NULL) out = NewMolecule();
  if(in == out)   return(out);
  
  ClearMolecule(out);
  /* Output is monochannel */
  SizeMolecule(out,in->dim);
  for(k = 0; k < in->dim; k++) {
    atom = GetMoleculeAtom(in,channel,k);
    AddAtom2Molecule(out,CopyAtom(atom,NULL));
  }
  return(out);
}

void PrintMolecule(const MOLECULE molecule,char flagShort)
{
  unsigned short k;
  unsigned char channel;
  ATOM atom;
  
  CheckMoleculeNotEmpty(molecule);
  
  // Short display of a molecule (e.g., during the pursuit)
  if(flagShort) {
    atom = GetMoleculeAtom(molecule,0,0);
    // Case of a molecule containing only one Gabor atom
    if(molecule->dim == 1) PrintAtom(atom,YES);
    // Case of a molecule containing a harmonic atom
    else {
      Printf("totalCoeff2 %g (s,t,f0,c) = (%d,%g,%g,%g)\n",
	     molecule->coeff2,atom->windowSize,
	     TimeId2Time(atom,atom->timeId),FreqId2Freq(atom,atom->freqId),ChirpId2Chirp(atom,atom->chirpId));
      Printf("   dim %d coeff2",molecule->dim);
      for(k = 0; k < molecule->dim; k++) {
	atom = GetMoleculeAtom(molecule,0,k);
	Printf("[%g]",atom->coeff2);
      }
      Printf("\n");
    }
  }
  else {
    Printf("Coeff2   : %g    (%g)\n",molecule->coeff2,sqrt(molecule->coeff2));
    for(k = 0; k < molecule->dim; k++) {
      atom = GetMoleculeAtom(molecule,0,k);
      if(molecule->dim>1) Printf("--%d\n",k);
      PrintAtom(atom,NO);
    }
  }
}

/*
 * The fields of a &mol
 */
static char *dimDoc = "{} {Returns the number of atoms in the &mol}";
static char *coeff2Doc = "{} {Returns the coeff2 of the &mol (it is the sum of those of its atoms)}";

void *GetDimMoleculeV(MOLECULE molecule, void **arg)
{
  /* Documentation */
  if (molecule == NULL) return(dimDoc);

  return(GetIntField(molecule->dim,arg));
}

static void *GetOptionsMoleculeV(MOLECULE molecule, void **arg)
{
  static char *opt[] = {NULL};

  return(opt);
}

static void *GetExtractInfoMoleculeV(MOLECULE molecule, void **arg)
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

  if (molecule->dim == 0) {
    SetErrorf("No extraction of atoms in an empty molecule");
    return(NULL);
  }
  extractInfo.xmax = molecule->dim-1;

  return(&extractInfo);
}

static char *atomDoc = "{[<n>]} {Gets the molecule <n> of a molecule}";
 
static void *GetMoleculeV(MOLECULE molecule, void **arg)
{
  char *field = ARG_G_GetField(arg);
  FSIList *fsiList;
  unsigned char channel = 0;

   /* doc */
  if (molecule == NULL) return(atomDoc);
  fsiList = ARG_G_GetFsiList(arg);

  if(fsiList==NULL) {
    SetErrorf("The syntax is mol[<n>]");
    return(NULL);
  }

  if (fsiList->nx != 1) {
    SetErrorf("Only a single index can be used");
    return(NULL);
  }

  ARG_G_SetResValue(arg,GetMoleculeAtom(molecule,channel,(int) FSI_FIRST(fsiList)));
  return(atomType);
}

static  void *SetMoleculeV(MOLECULE molecule, void **arg)
 {
  char *field = ARG_S_GetField(arg);
  FSIList *fsiList;

   /* doc */
  if (molecule == NULL) return(atomDoc);
  
  SetErrorf("Atoms from a molecule are read only");
  return(NULL);
 }


void *GetCoeff2MoleculeV(MOLECULE molecule, void **arg)
{
  /* Documentation */
  if (molecule == NULL) return(coeff2Doc);

  return(GetFloatField(molecule->coeff2,arg));
}

/*
 * The field list
 */
struct field fieldsMolecule[] = {
  "",GetMoleculeV,SetMoleculeV,GetOptionsMoleculeV,GetExtractInfoMoleculeV,  
  "dim",GetDimMoleculeV,NULL,NULL,NULL,
  "coeff2",GetCoeff2MoleculeV,NULL,NULL,NULL,
  NULL, NULL, NULL, NULL, NULL
};

/*
 * The type structure for MOLECULE
 */

TypeStruct tsMolecule = {

  "{{{&mol} {This type corresponds to subspaces spanned by a few atoms and is used in (Harmonic) Matching Pursuit decompositions.}}}",  /* Documentation */

  &moleculeType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteMolecule,     /* The Delete function */
  NewMolecule,     /* The New function */
  
  CopyMolecule,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrMolecule,       /* String conversion */
  ShortPrintMolecule,   /* The Print function : print the object when 'print' is called */
  PrintInfoMolecule,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsMolecule,      /* The list of fields */
};

/* EOF */

