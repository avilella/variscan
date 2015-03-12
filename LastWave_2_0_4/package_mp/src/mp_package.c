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



/****************************************************************************/
/*                                                                          */
/*  mp_package.c   Functions which deal with the Matching Pursuit package  */
/*                                                                          */
/****************************************************************************/

#include "lastwave.h"

#include "mp_book.h"


extern void C_SetDict(char **);
static CProc dictCommands[] = { 
  // Basic manipulation
    {"setdict",C_SetDict,"{{{channels <dict> {<signali1> ... <signaliN>}} {Sets the channels of a dictionary.}} \
{{add <dict> '&maximadict' <nmaxima>} {Adds a sub-dictionary of local maxima.}} \
{{add <dict> '&stft' [('real'|'harmo'|'highres')] <windowSize> [<windowShape>='gauss'] [{<freq0Min> <freq0max>}]} {Adds a &stft sub-dictionary.}} \
{{update <dict>} {Updates all sub-dictionaries to enable a new 'getmax'.}} \
{{getmax <dict> [ {['causal'] [{'time(Id)' <range>}] [{'freq(Id)' <range>}] [{'windowSize' <range>}]} ] [ {['interpolate'] ['chirp']} ]} {}} \
{{optmol <dict> <molecule> [ {['time'] ['freq'] ['chirp'] ['recompute']} ]} {Optimizes a molecule using a dictionary}} \
{{rmmol <dict> <molecule>} {}}}"},
  {NULL,NULL,NULL}
};

static CProcTable dictTable =   {dictCommands, "mp","Commands which deal with &dict : dictionaries of atoms"};

// Commands related with the MOLECULE and BOOK structure
extern void C_Book(char **);
extern void C_Mpr(char **);

// Notes 'package'
extern void C_Notes(char **);
extern void C_CreateProfile(char **);


static CProc bookCommands[] = {
 {"book",C_Book,
"{{{read [<book>=objCur] <filename>} {Reads a book from a file.}} \
 {{write [<book>=objCur] <filename> [-b]} {Writes a book to a file in ascii format (by default) or \
in binary (option -b).}} \
 {{readold [<book>=objCur] <filename> <forceMaxFreqId> <decay>} {Reads a book (and the decay signal) from a file in older format. You have to specify what was the MaxFreqId, you may have to make several trials and check the result using 'mpr'.}}}"},

// Reconstruction
  {"mpr",C_Mpr,
"{{{[<book>=objCur] <reconsSignal> [<maskSignal>] \
[-n <nMin> [<nMax>=<nMin>]] \
[-s <windowSizeMin> [<windowSizeMax>=<windowSizeMin>]>] \
[-t <timeMin> <timeMax>] [-T <timeIdMin> <timeIdMax>] \
[-f <freqMin> <freqMax>] [-F <freqIdMin> <freqIdMax>] \
[-c <chirpMin> <chirpMax>] [-C <chirpIdMin> <chirpIdMax>]} \
 {Builds the reconstructed signal from the molecules of a book. \
The command returns the number of molecules that were actually used. \
A masking signal can optionally be used to specify which molecules are \
used (the molecule number <n> is used in the reconstruction iff the \
<n>th sample in maskSignal is nonzero). \
Additionally, the reconstruction only uses the molecules whose \
fields lie within the range specified by the following options: \n\
-n : Specifies the molecule number range \n\
-s : Specifies the scale  range \n\
-t : Specifies the time range (real units) \n\
-f : Specifies the frequency range (real coordinate) \n\
-c : Specifies the chirp   range (real coordinates)\n\
-C : Specifies the chirpId range (id units).\n\
-T : Specifies the timeId  range (id units) \n\
-F : Specifies the freqId  range (id units)}}}"},

  {NULL,NULL,NULL}
};

static CProcTable bookTable = {bookCommands, "mp", "Commands which deal with &book and &mol variables"};


/* Inner-products between atoms */
extern void C_Inner(char **);
extern void C_AIP(char **);
#ifdef ATOM_ADVANCED
static CProc innerProdCommands[] = { 
  /* Inner-product */
    {"inner",C_Inner,
"{{{sig   <signal> <atom>} {Computes the inner product between a signal and a complex atom.}} \
 {{auto  <atom>} {Computes the inner product between a complex atom and its conjugate.}} \
 {{cc   <atom1> <atom2> [-n]} {Computes the inner product between two complex atoms, with a fast computation if possible. Option -n forces exact (slow) numeric computation.}} \
 {{rc   <atomR> <atomC> [-n]} {Computes the inner product between a real atom and a complex one, with a fast computation if possible. Option -n forces exact (slow) numeric computation.}} \
 {{rr   <atom1> <atom2> [-n]} {Computes the inner product between two real atoms, with a fast computation if possible. Option -n forces exact (slow) numeric computation.}}}"},
  {NULL,NULL,NULL}
};

static CProcTable innerProdTable = {innerProdCommands,"mp","Commands which deal with inner products of &atoms"};
#endif

static CProc noteCommands[] = {
{"notes",C_Notes,"{{{<book> ([-n <nMin> [<nMax>=<nMin>]] | [-s <output> [<attackDuration>] <noteList>])} {Gets a list with the notes from <book> usign molecules between index <nMin> and <nMax>. Options '-s' is used to synthesize in a signal the notes using sinusoids and a simple cosinusoidal attack pattern of <attackDuration> samples.}}}"},

{"profile",C_CreateProfile,"{{{<book> <signal> <n> <deltaFreq>} {Computes the energy profile at the location of the <n>th molecule of a book and puts it in a signal.}}}"},

  {NULL,NULL,NULL}
};

static CProcTable noteTable = {noteCommands, "mp", "Commands which deal with notes from a book"};

/***********************************
* 
* Loading/Adding the MP package
*
***********************************/
int tDICT, tDICT_;
int tMAXIMADICT, tMAXIMADICT_;
int tATOM, tATOM_;
int tMOLECULE,   tMOLECULE_;
int tBOOK, tBOOK_;

static void LoadMpPackage(void)
{
  // Adding Dict VARIABLES
  tDICT = AddVariableTypeValue(dictType, &tsDict, NULL);
  tDICT_ = tDICT+1;
  // Adding MaximaDict VARIABLES
  tMAXIMADICT = AddVariableTypeValue(maximaDictType, &tsMaximaDict, NULL);
  tMAXIMADICT_ = tMAXIMADICT+1;

  // Adding atom VARIABLES 
  tATOM = AddVariableTypeValue(atomType, &tsAtom, NULL);
  tATOM_ = tATOM+1;
  // Adding molecule VARIABLES 
  tMOLECULE = AddVariableTypeValue(moleculeType, &tsMolecule, NULL);
  tMOLECULE_ = tMOLECULE+1;
  // Adding book VARIABLES
  tBOOK = AddVariableTypeValue(bookType, &tsBook, NULL);
  tBOOK_ = tBOOK+1;

  AddCProcTable(&dictTable);
  AddCProcTable(&bookTable);
#ifdef ATOM_ADVANCED
  AddCProcTable(&innerProdTable);
#endif
  AddCProcTable(&noteTable);

  DefineGraphBook(); 
}

void DeclareMpPackage(void)
{
  DeclarePackage("mp",LoadMpPackage,2001,"2.0","R.Gribonval, E.Bacry and J. Abadia",
  "Package allowing to perform Matching Pursuit.");
}


/* EOF */

