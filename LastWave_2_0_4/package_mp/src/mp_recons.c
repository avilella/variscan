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


/*****************************************************/
/*
 *	Functions to BUILD a BOOK (Matching Pursuit)
 */
/****************************************************/
#include "lastwave.h"

#include "mp_book.h"

/******************************************************************/
/*                Reconstruction                                  */
/******************************************************************/
/* flagFundamentalRange = YES : 
 *   if the fundamental is not in range, we skip all the partials
 * flagFundamentalRange = NO : 
 *   we check each partial separately.
 */
void Mpr(BOOK book,SIGNAL reconsSignal,long *pTimeIdReconsMin,long *pTimeIdReconsMax,
	 SIGNAL maskSignal,
	 char flagFundamentalRange,
	 unsigned long windowSizeMin,unsigned long windowSizeMax,
	 float timeIdMin, float timeIdMax,
	 float freqIdMin, float freqIdMax,
	 float chirpIdMin,float chirpIdMax,
	 unsigned long nMin,unsigned long nMax)
{
  static SIGNAL atomSignal = NULL;
  MOLECULE molecule;
  ATOM atom;
  unsigned short k;
  unsigned long n;
  char flagSkipHarmo = NO;
  // TODO : decide what to do with that
  char borderType = BorderPad0;
  float shift;

  unsigned long count = 0;
  // TODO ; change to long when prototype of ComputeWindowSupport is changed
  int atomTimeIdMin,atomTimeIdMax;

  // Checking
  CheckBookNotEmpty(book);
  if((maskSignal != NULL) && (maskSignal->size != book->size)) Errorf("Mpr : bad size of the mask");
  // Allocations
  if(atomSignal == NULL) {
    atomSignal = NewSignal();
  }
  
  // Init the signal
  SizeSignal(reconsSignal,book->signalSize,YSIG);
  ZeroSig(reconsSignal);
  reconsSignal->dx = book->dx;
  reconsSignal->x0 = book->x0;
  
  // So far nothing has been built :
  *pTimeIdReconsMin = book->signalSize;
  *pTimeIdReconsMax = 0;  
  // Loop on molecules
  for(n = nMin; n <= MIN(book->size-1,nMax); n++) {
    molecule = GetBookMolecule(book,n);
    // Skip masked molecules      
    if(maskSignal != NULL && maskSignal->Y[n] == 0.0)    continue;
    flagSkipHarmo = NO;
    // Loop on atoms in molecule
    for(k = 0; k < molecule->dim; k++) {
      atom = GetMoleculeAtom(molecule,0,k);
      if (flagSkipHarmo ||
	  !INRANGE(windowSizeMin,atom->windowSize,windowSizeMax) ||
	  !INRANGE(timeIdMin, atom->timeId,timeIdMax) ||
	  !INRANGE(freqIdMin, atom->freqId,freqIdMax) ||
	  !INRANGE(chirpIdMin, atom->chirpId,chirpIdMax)) {
	if(flagFundamentalRange && k == 0) flagSkipHarmo = YES;
	continue;
      }
      // Display a clock for waiting
      switch(count%4) {
      case 0 : Printf("\\"); Flush(); break;
      case 1 : Printf("|"); Flush(); break;
      case 2 : Printf("/"); Flush(); break;
      case 3 : Printf("-"); Flush(); break;
      }
      
      // Build a real normalized atom
      BuildAtom(atom,atomSignal,NULL,borderType,YES); 
      // Add it to the output signal
      shift = (atomSignal->x0-atom->x0)/atom->dx;
      if (shift < 0) shift = (int) (shift-.5);
      else shift = (int) (shift+.5);
      AddShiftedSignals(atomSignal,reconsSignal,shift);

      // Compute the limits where the output signal has changed
      ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,&atomTimeIdMin,&atomTimeIdMax);
      *pTimeIdReconsMin = MIN(*pTimeIdReconsMin,atomTimeIdMin);
      *pTimeIdReconsMax = MAX(*pTimeIdReconsMax,atomTimeIdMax);
      /* Border effect : only ZERO border is treated for the moment */
      switch(borderType) {
      case BorderPad0 :
	if(*pTimeIdReconsMin < 0 || *pTimeIdReconsMax > book->signalSize-1)
	  Errorf("Mpr : signal updated out of its limits!");
	break;
      default :
	Errorf("Mpr : border type '%s' not treated yet",BorderType2Name(borderType));
      }
      // Deal with the 'clock'
      Printf("\b");
      count++;
    }
  }
}


void C_Mpr(char **argv)
{
  SIGNAL reconsSignal;
  long timeIdReconsMin,timeIdReconsMax;
  SIGNAL maskSignal;
  BOOK book;
  char opt;
  
  /* The rangs */
  unsigned long nMin,nMax;
  
  unsigned long windowSizeMin, windowSizeMax;
  float timeIdMin,timeIdMax;
  float freqIdMin,freqIdMax;
  float chirpIdMin,chirpIdMax;
  
  /* Which ranges have been selected */
  int flagFundamentalRange = NO;
  LISTV lv;
  /* Reading arguments */
  argv = ParseArgv(argv,tBOOK_,NULL,&book,tSIGNAL,&reconsSignal,tSIGNALI_,NULL,&maskSignal,-1);
  if(book == NULL)
    book = GetBookCur();
  
  /* Checking arguments */
  CheckBookNotEmpty(book);
  
  /* default values */
  nMin = 0;
  nMax = book->size-1;
  windowSizeMin	= STFT_MIN_WINDOWSIZE;
  windowSizeMax	= STFT_MAX_WINDOWSIZE;
  timeIdMin 	= 0;
  timeIdMax 	= book->signalSize;
  freqIdMin 	= 0;
  freqIdMax 	= GABOR_NYQUIST_FREQID;
  chirpIdMax 	= GABOR_MAX_CHIRPID;
  chirpIdMin	= -chirpIdMax;

  
  /* Looking for options */
  while((opt = ParseOption(&argv))) {
    switch(opt) {
      /* Do we reconstruct an harmonic structure 
       * if the fundamental is not in range ? */
    case 'h' :
      flagFundamentalRange = YES;
      break;
    case 'n':
      argv = ParseArgv(argv,tINT,&nMin,tINT_,0,&nMax,-1);
      if(nMax == 0) nMax = nMin;
      if(nMin > nMax) Errorf("<nMin> is greater than <nMax>");
      break;
    case 's':
      argv = ParseArgv(argv,tINT,&windowSizeMin,tINT_,0,&windowSizeMax,-1);
      if(windowSizeMax == 0) windowSizeMax = windowSizeMin;
      if(windowSizeMin > windowSizeMax) Errorf("<windowSizeMin> is greater than <windowSizeMax>");
      break;
    case 'T':
      argv = ParseArgv(argv,tFLOAT,&timeIdMin,tFLOAT,&timeIdMax,-1);
      if(timeIdMin > timeIdMax) Errorf("<timeIdMin> is greater than <timeIdMax>");
      break;
    case 'F':
      argv = ParseArgv(argv,tFLOAT,&freqIdMin,tFLOAT,&freqIdMax,-1);
      if(freqIdMin > freqIdMax) Errorf("<freqIdMin> is greater than <freqIdMax>");
      break;
    case 'C':
      argv = ParseArgv(argv,tFLOAT,&chirpIdMin,tFLOAT,&chirpIdMax,-1);
      if(chirpIdMin > chirpIdMax) Errorf("<chirpIdMin> is greater than <chirpIdMax>");
      break;
    case 't':
      argv = ParseArgv(argv,tFLOAT,&timeIdMin,tFLOAT,&timeIdMax,-1);
      if(timeIdMin > timeIdMax) Errorf("<timeMin> is greater than <timeMax>");
      timeIdMin = Time2TimeId(book,timeIdMin);
      timeIdMax = Time2TimeId(book,timeIdMax);
      break;
    case 'f':
      argv = ParseArgv(argv,tFLOAT,&freqIdMin,tFLOAT,&freqIdMax,-1);
      if(freqIdMin > freqIdMax) Errorf("<freqMin> is greater than <freqMax>");
      freqIdMin = Freq2FreqId(book,freqIdMin);
      freqIdMax = Freq2FreqId(book,freqIdMax);
      break;
    case 'c':
      argv = ParseArgv(argv,tFLOAT,&chirpIdMin,tFLOAT,&chirpIdMax,-1);
      if(chirpIdMin > chirpIdMax) Errorf("<chirpMin> is greater than <chirpMax>");
      chirpIdMin = Chirp2ChirpId(book,chirpIdMin);
      chirpIdMax = Chirp2ChirpId(book,chirpIdMax);
      break;
    default : 
      ErrorOption(opt);
    }
  }
  NoMoreArgs(argv);
  
  if (timeIdMin < 0) 
    Errorf("'timeIdMin' should be greater than 0\n");
  if (timeIdMax > book->signalSize) 
    Errorf("'timeIdMax' is too big\n");

  /* Reconstructing */
  Mpr(book,reconsSignal,&timeIdReconsMin,&timeIdReconsMax,maskSignal,
      flagFundamentalRange,
      windowSizeMin,windowSizeMax,
      timeIdMin,timeIdMax,
      freqIdMin,freqIdMax,
      chirpIdMin,chirpIdMax,
      nMin,nMax);
  lv = TNewListv();
  AppendInt2Listv(lv,timeIdReconsMin);
  AppendInt2Listv(lv,timeIdReconsMax);
  SetResultValue(lv);
}

/* EOF */

