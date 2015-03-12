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



/* javi: dealing with notes */
typedef struct note{
    float onset;
    float end;
    float pitch;
} *NOTE;

extern void   	Notes(BOOK book,int nMin,int nMax,NOTE *note);
extern int	SearchNote(BOOK book,ATOM mainAtom,int numAtom,int *flagNotes,float *pp, float * bb, float * ee);

extern void 	ComputeEnergyProfile(BOOK book,ATOM mainAtom,SIGNAL energyProfile);
extern int	MarkAtoms(BOOK book,ATOM mainAtom,int numAtom,int *flagNotes,float * begin,float end);
extern void 	NotesSynthesis(NOTE *note,SIGNAL synthesis,BOOK book,int attack);
extern void 	AddAtomEnergyEnvelope(SIGNAL energyEnvelope,ATOM atom,ATOM mainAtom);
extern void	GetBestPartial(MOLECULE molecule,ATOM bestPartial);
extern void	GetFundamental(MOLECULE molecule,ATOM bestPartial);
extern void 	GetNearestPartial(MOLECULE molecule,ATOM mainAtom,ATOM nearestPartial);
extern void 	GetBeginEndNote(SIGNAL energyProfile,ATOM atom,float energyTh,float *beginNote,float *endNote,float *endMark);
extern float 	GetEnergyMax(SIGNAL energyProfile,ATOM atom);
extern NOTE 	NewNote(void); 
extern void 	DeleteNote(NOTE note); 

// The parameters for the note detection algorithm
static float minNoteDuration = 0.03; /* Delta t_min : minimum duration of the notes */
static float stopCoeff2Ratio = 0.01; /* ?? When to stop in the book */
static float maxFundRatio    = 0.03; /* ?? Minimum energy of the fundamental (compared to the most energetic one) */
static float thresholdNoteDB = -14;  /* theta_beg = theta_end :
					Used to find begining and end of a note */
static float freqSigmaFact = 8;      /* Controls the bandwidth (in Hertz)
				      * of a FoF at octave 12 (windowSize 4096)
				      * and sampling frequency 16000 */
static float markRatio = 0.00002;    /* Controls how different from the
				      * fundamental frequency the frequency
				      * of a marked atom can be */
static float deltaFreq = 5;


NOTE NewNote(void)  
{
  NOTE notes;

#ifdef DEBUGALLOC
  DebugType = "Note";
#endif
  notes = (NOTE) Malloc(sizeof(struct note));
  notes->onset = 0.0;
  notes->end = 0.0;
  notes->pitch = 0.0;
	 
  return(notes);
} 

void DeleteNote(NOTE n)
{
#ifdef DEBUGALLOC
  DebugType = "Note";
#endif
  Free(n);
}


// COMMAND
void C_Notes(char **argv)    		
{
  int nMax,nMin;
  BOOK book = NULL;
  NOTE note[500],*note1;
  char flagSynthesis;
  int i;
  char opt;
  SIGNAL synSig;
  char **list,**list1;
  int attack;
  
  LISTV reslist;
  LISTV res;

  argv = ParseArgv(argv,tBOOK,&book,-1);
  CheckBookNotEmpty(book);
  
  /* Default values */
  nMin = 0;
  nMax = book->size-1;
  flagSynthesis = NO;

  /* Reading options */
  while(( opt = ParseOption(&argv))) {
    switch(opt) {
    case 'n' : /* nMin nMax */
      argv = ParseArgv(argv,tINT,&nMin,tINT_,-1,&nMax,-1);
      if(nMax == -1)
	nMax = nMin;
      break;
    case 's' : /* synthesis of the notes */
      argv = ParseArgv(argv,tSIGNAL,&synSig,tINT_,1000,&attack,tLIST,&list,-1);
      flagSynthesis = YES;
      break;
    default : 
      ErrorOption(opt);
    }
  }
  NoMoreArgs(argv);

/*    Printf("nMin:%d , nMax:%d\n",nMin,nMax); */
  
  // Synthesis if asked
  if (flagSynthesis) {
    note1 = note;
    while (*list) {
      *note1 = NewNote();
      ParseList(*list,&list1);
      ParseArgv(list1,tFLOAT,&((*note1)->pitch),tFLOAT,&((*note1)->onset),tFLOAT,&((*note1)->end),0);
      note1++;
      list++;
    }
    *note1 = NULL;
    NotesSynthesis(note,synSig,book,attack);
    for (i=0;note[i]!=NULL;i++) {
      DeleteNote(note[i]);
    }
    return;
  }

  // Computing the notes from the molecules of the book
  Notes(book,nMin,nMax,note); 

  // Setting the resulting list
  reslist = TNewListv();
  for (i=0;note[i]!=NULL;i++) {
    res = TNewListv();
    AppendFloat2Listv(res,note[i]->pitch);
    AppendFloat2Listv(res,note[i]->onset);
    AppendFloat2Listv(res,note[i]->end);
    AppendValue2Listv(reslist,(VALUE)res);
    DeleteNote(note[i]);
  }
}

static     int *blackList = NULL;

void Notes(BOOK book,int nMin,int nMax,NOTE *note)
{
  unsigned long n;
  int i;
  int minOctave;
  int flagNote = NO;
  float maxMoleculeCoeff2,maxFundCoeff2;
  static MOLECULE molecule = NULL;
  static ATOM fundamental = NULL;
  int noteNum;
  float pp,ee,bb;

  noteNum = 0;
  note[0] = NULL;

  CheckBookNotEmpty(book);

  /* Initializing */
  if (fundamental == NULL) {
    fundamental = NewAtom();
    molecule = NewMolecule();
  }
  if (blackList != NULL) Free(blackList);
  blackList = IntAlloc(book->size);
  for (i = 0;i < book->size;i++) *(blackList + i) = 0;

  minOctave = floor(log(floor(minNoteDuration/(book->dx)))/log(2));
  Printf("minOctave:%d\n",minOctave);

  molecule = GetBookMolecule(book,0);
  GetFundamental(molecule,fundamental); 

  maxMoleculeCoeff2 = molecule->coeff2;
  maxFundCoeff2 = fundamental->coeff2;
 
  /* Main loop to search notes */
  for (n=nMin; n<=nMax; n++) {
    if (*(blackList+n-1)) continue;
    molecule = GetBookMolecule(book,n);
    // Condition to stop the notes search
    if ( molecule->coeff2 < maxMoleculeCoeff2*stopCoeff2Ratio ) break;   
    // Else we take the fundamental in the current molecule
    GetFundamental(molecule,fundamental);
    // We don't take into account small atoms
    /* ????? bof */
    if (fundamental->windowSize <= 1<<minOctave) continue;
    if (fundamental->coeff2 < maxFundCoeff2*maxFundRatio) continue;
    /* ShortPrintAtom(fundamental); */
    Printf("ATOM:%d ->\n",n);
    flagNote = SearchNote(book,fundamental,n,blackList,&pp,&bb,&ee);
    if (!flagNote) continue;
    if (ee-bb < minNoteDuration ) {
      Printf("\tToo small!\n");
      continue;
    }
    Printf(" NOTE:%d (pitch:%f, begin:%f, end:%f)\n",noteNum+1,pp,bb,ee);
        
    note[noteNum] = NewNote();
    note[noteNum]->onset = bb;
    note[noteNum]->end   = ee;
    note[noteNum]->pitch = pp;
    noteNum++;
    note[noteNum] = NULL;
  }
}


void NotesSynthesis(NOTE *note,SIGNAL synthesis,BOOK book,int attack)
{ 
  int i,j;
  int begin,end;
  float dx;
    
  SizeSignal(synthesis,book->signalSize,YSIG);
  ZeroSig(synthesis);
  synthesis->dx = book->dx;
  synthesis->x0 = book->x0;
  dx = synthesis->dx;
  
  for (i=0;note[i]!=NULL;i++) {
    begin 	= (int) ((note[i]->onset)/dx);
    end 	= (int) ((note[i]->end)/dx);
    for (j=begin;j<=begin+attack;j++) {
      synthesis->Y[j] += (1-cos(0.5*M_PI*(j-begin)/((float) attack)))*sin(2*M_PI*(note[i]->pitch)*j*dx);
    }
    for (;j<=end-attack;j++) {
      synthesis->Y[j] += sin(2*M_PI*(note[i]->pitch)*j*dx);
    }
    for (;j<=end;j++) {
      synthesis->Y[j] += (1-cos(.5*M_PI*(end-j)/((float) attack)))*sin(2*M_PI*(note[i]->pitch)*j*dx);
    }
  }
}



int SearchNote(BOOK book,ATOM mainAtom,int numAtom,int *flagNotes,float *pp, float * bb, float * ee)
{
  static SIGNAL energyProfile = NULL;
  
  int foundAtoms;
  float pitchNote,pitchNoteId;
  float beginNote,endNote;
  float beginNoteId,endNoteId,endMarkId;
    
  /* Init the energy profile signal */
  if (energyProfile == NULL) energyProfile = NewSignal();

  /* Compute the energy profile */
  ComputeEnergyProfile(book,mainAtom,energyProfile);
    
  /* we find the onset and the end of the note taking into account a threshold of energy */
  GetBeginEndNote(energyProfile,mainAtom,thresholdNoteDB,&beginNoteId,&endNoteId,&endMarkId);
  
  pitchNote =  FreqId2Freq(mainAtom,mainAtom->freqId);
  pitchNoteId = mainAtom->freqId;
  beginNote =  mainAtom->dx*beginNoteId;
  endNote   =  mainAtom->dx*endNoteId;
  
  /*    Printf("\tNOTE:( pitch: %f(%d), begin: %f(%d), end: %f(%d) )\n",pitchNote,(int)(mainAtom->freqId),beginNote,(int)(beginNoteId),endNote,(int)(endNoteId)); */
  *pp = pitchNote;
  *bb = beginNote;
  *ee = MAX(endNote,beginNote+mainAtom->dx*0.5*mainAtom->windowSize);
  
    
  /* look for atoms which seem to belong to the same note */ 
  foundAtoms = MarkAtoms(book,mainAtom,numAtom,flagNotes,&beginNoteId,endMarkId);
  Printf("foundAtoms:%d\n",foundAtoms);   
  
  if (foundAtoms > 0) { 
    Printf("\n");
    return(YES);
  }
  else {
    Printf("\t\tNOT A NOTE!\n");
    return(NO);
  }
}

void ComputeEnergyProfile(BOOK book,ATOM mainAtom,SIGNAL energyProfile)
{
  static ATOM partial = NULL;
  static MOLECULE moleculeSearch = NULL; 
  unsigned long n;
  int i;
  float energyMax;
  
  // Allocation (once)
  if (partial == NULL) {
    partial    = NewAtom();
    moleculeSearch = NewMolecule();
  }
  
  // Initialize the energyProfile
  SizeSignal(energyProfile,book->signalSize,YSIG);
  energyProfile->dx = book->dx;
  energyProfile->x0 = book->x0;
  for (i = 0; i <energyProfile->size; i++) energyProfile->Y[i] = 1e-7;
  
  /* Now we add the envelope of the others atoms with a frequency correction */
  for (n = 0; n<book->size; n++) {
    moleculeSearch = GetBookMolecule(book,n);
    /* we take the best partial in this molecule  */
    GetNearestPartial(moleculeSearch,mainAtom,partial);
    /* we add the envelope of the best partial */
    AddAtomEnergyEnvelope(energyProfile,partial,mainAtom);	
  }

  /* linear -> dB */
  energyMax = GetEnergyMax(energyProfile,mainAtom);
  for (i = 0; i <energyProfile->size; i++)
    energyProfile->Y[i] = 10*log10(energyProfile->Y[i]/energyMax);
}
  
/* COMMAND */
void C_CreateProfile(char **argv)
{
  BOOK book;
  SIGNAL signal;
  unsigned long n;
  MOLECULE molecule;
  static ATOM atom = NULL;
  float delta;
  float begin,end,end1;

  if (atom == NULL) {
    atom    = NewAtom();
  }
  
  argv = ParseArgv(argv,tBOOK,&book,tSIGNAL,&signal,tINT,&n,tFLOAT_,deltaFreq,&delta,0);

  deltaFreq = delta;
  
  molecule = GetBookMolecule(book,n);
  GetFundamental(molecule,atom);

  ComputeEnergyProfile(book,atom,signal);
  GetBeginEndNote(signal,atom,thresholdNoteDB,&begin,&end,&end1);//
  /*signal->Y[(int) (begin+.5)] = 10; 
    signal->Y[(int) (end+.5)] = 10; */
}


int MarkAtoms(BOOK book,ATOM mainAtom,int numAtom,int *flagNotes,float * beginNoteId,float endNoteId)
{
  static ATOM partial = NULL;
  static MOLECULE moleculeSearch = NULL;   
  static ATOM fund = NULL;   
  
  unsigned long n;
  int timesFreq;
  int foundAtoms;
  int partialSize;
  float mainFreq,partialFreq,partialFreqId;
  float diffFreq,freqFactor;
  float sigma2;
  
  if(numAtom <0) Errorf("numAtom %d < 0",numAtom);
  /* initialization */
  if (partial == NULL) {
    partial = NewAtom();
    fund = NewAtom();
    moleculeSearch = NewMolecule();
  }    
  
  foundAtoms = 0;   
  for (n = numAtom; n<book->size; n++) {
    /*	   if (*(flagNotes +n-1)) continue;  this atom belongs to another note yet */
    moleculeSearch = GetBookMolecule(book,n);
        
    /* we take the best partial in this molecule  */
    GetNearestPartial(moleculeSearch,mainAtom,partial);
    GetFundamental(moleculeSearch,fund);
    if (fund->freqId != partial->freqId) continue;
    
    /*Printf("%d,mainFreq:%f,partialFreq:%f",n,mainAtom->freqId,partial->freqId);*/	
      
    /* we try to mend the octave errors */
    partialFreqId = partial->freqId;
    if (partialFreqId > mainAtom->freqId) {
      timesFreq = floor(partialFreqId/mainAtom->freqId);
      if (timesFreq > 1) {
	if (partialFreqId-timesFreq*mainAtom->freqId < mainAtom->freqId/2)
	  partialFreqId -= (timesFreq-1)*mainAtom->freqId;
	else partialFreqId -= timesFreq*mainAtom->freqId;	
	/* Printf(",times:%d, diffFreqId:%f\n",timesFreq,mainAtom->freqId-partial->freqId);*/
      }
      else {
	if (partialFreqId - mainAtom->freqId > mainAtom->freqId/2) {
	  partialFreqId -= mainAtom->freqId;
	  /* Printf(",times:1, diffFreqId:%f\n",mainAtom->freqId-partial->freqId);*/
	}
	/* else Printf("\n");*/
      }
    }
    /* else Printf("\n"); */
    
    
    /* Should we mark it ? */
    partialSize = partial->windowSize;
    if (partial->timeId > *beginNoteId-partialSize/4 && partial->timeId < endNoteId) {
      mainFreq 	  = FreqId2Freq(mainAtom,mainAtom->freqId);
      partialFreq = FreqId2Freq(mainAtom,partialFreqId);
      
      
      sigma2 = (4096/partial->windowSize)*freqSigmaFact/(16000.0*book->dx);
      sigma2 = sigma2*sigma2;
      diffFreq = mainFreq-partialFreq;
      freqFactor = exp(-(diffFreq*diffFreq)/sigma2);
      if (freqFactor > markRatio) {
	foundAtoms++;
	/* if (partial->timeId < *beginNoteId) *beginNoteId = partial->timeId; */
	if (*(flagNotes +n-1)) continue;
	Printf("\t\t%d:(s:%d,dif:%f,factor:%f)MARKED\n",n,partial->windowSize,diffFreq,freqFactor);
	*(flagNotes +n-1) = 1;
      }
    }
  }
  return(foundAtoms);
}


float GetEnergyMax(SIGNAL energyProfile,ATOM atom)
{
  float maxima;
  int i;
  //static SIGNAL atomEnvelope = NULL;
  //float (*f)(SIGNAL,float);
  
  /*  if (atomEnvelope == NULL) atomEnvelope = NewSignal();
  SizeSignal(atomEnvelope,atom->windowSize,YSIG);
  GetWindowShapeFunc(atom->windowShape,&f);
  (*f)(atomEnvelope,0.0);
  maxima = -1;
  for (i=0;;i++) {
    while(maxima < atomEnvelope->Y[i]) maxima = atomEnvelope->Y[i];
  }
  maxima = energyProfile->Y[(int) (i+atom->timeId)];
  
  return(maxima);
*/

  maxima = 0.0;
  for (i = (int) atom->timeId ; 
       i < atom->timeId + atom->windowSize/4 ;
       i++) {
    if ( energyProfile->Y[i] > maxima) maxima = energyProfile->Y[i];
  }
  return(maxima);
  /*
    maxima = 0.0;
    
    for (i = 0; i < energyProfile->size ; i++) {
	  if ( energyProfile->Y[i] > maxima) maxima = energyProfile->Y[i];
    }
    return(maxima);
  */
}
    

void GetBeginEndNote(SIGNAL energyProfile,ATOM atom,float energyTh,float *beginNote,float *endNote,float *endMark)
{
  float energy;
  int timeIdBegin,timeIdEnd;
  int originSearch;
  float maxima;
  int i;
  
  maxima = -1e10;
  for (i= (int) atom->timeId ; i < atom->timeId+atom->windowSize/4; i++) {
    if (energyProfile->Y[i] > maxima) {originSearch = i; maxima = energyProfile->Y[i];}
  }
  timeIdBegin = originSearch;
  
  /* loop to find the beginning of the current note */
  /*  originSearch = (int)(atom->timeId + (atom->windowSize)/4);  
  timeIdBegin = originSearch + 1; 
*/

  do {
    timeIdBegin -= 1;
    energy = energyProfile->Y[timeIdBegin];
    /* Printf("energy1:%f (%d) > %f \n",energy,timeIdBegin,energyTh); */
  } while ( (energy > energyTh) && (timeIdBegin > 0) );
  
  *beginNote = timeIdBegin;
  
  /* loop to find the end */
  timeIdEnd = originSearch - 1;
  
  do {
    timeIdEnd += 1;
    energy = energyProfile->Y[timeIdEnd];
    /* Printf("energy2:%f (%d) > %f \n",energy,timeIdEnd,energyTh); */
  } while ( (energy > energyTh) && (timeIdEnd < energyProfile->size -1) );
  
  *endNote = timeIdEnd;
  *endMark = timeIdEnd;
}	

void AddAtomEnergyEnvelope(SIGNAL energyEnvelope,ATOM atom,ATOM mainAtom)
{
  static SIGNAL atomEnvelope = NULL;
  static SIGNAL tempSignal = NULL;   
  
  int i,time;
  float freqAtom,mainFreq;
  int atomSize;
  int windowShape;
  float diffFreq;
  float sigma2;
  float coeff2;
  float (*f)(SIGNAL,float);
  float factor,freqFactor;
  float fr;
  
  if (atomEnvelope == NULL) {
    tempSignal = NewSignal();
    atomEnvelope = NewSignal();
  }
  windowShape = atom->windowShape;
  atomSize = atom->windowSize;
  
  /* we use always the normalized coeff2 by the main atom of the note */
  coeff2 = atom->coeff2/mainAtom->coeff2;
  
  SizeSignal(atomEnvelope,atomSize,YSIG);
  CopyFieldsSig(energyEnvelope,atomEnvelope);
  
  /* Convertion of freqId to Hertz */
  mainFreq = FreqId2Freq(atom,mainAtom->freqId);
  freqAtom = FreqId2Freq(atom,atom->freqId);
  
  
  /*sigma2 = (20*(15-atom->octave)); */
  sigma2 = (4096/atom->windowSize)*freqSigmaFact/(16000.0*atom->dx);
  sigma2 = sigma2*sigma2;
    

/* Printf("%f - %f = %f ---->freqFactor: %f (%f)\n",mainFreq,freqAtom,diffFreq,freqFactor,sigma2); */
/* Printf("   coeff2: %f freqFactor: %f --> factor: %f\n",coeff2,freqFactor,factor); */

  /* Setting the right enveloppe size */	
  
  SizeSignal(tempSignal,atomSize,YSIG);
  atomEnvelope  = tempSignal;
  GetWindowShapeFunc(windowShape,&f);
  (*f)(atomEnvelope,0.0);
  
    /* adding the atom envelope to the energy profile */
  freqFactor = 0;
  for (fr = mainFreq-deltaFreq; fr<= mainFreq+deltaFreq;fr+=.1) {
    diffFreq = (fr-freqAtom);
    freqFactor += exp(-(diffFreq*diffFreq)/sigma2);
    factor = coeff2*freqFactor;
  }
  if (factor < 0.01) return;
  
  for (i=0; i<atomSize; i++) {
    time = (int) i + atom->timeId;
    energyEnvelope->Y[time] += (factor*atomEnvelope->Y[i]*atomEnvelope->Y[i]);
  } 
}

void GetNearestPartial(MOLECULE molecule,ATOM mainAtom,ATOM nearestPartial)
{
  int k;
  int kMax;
  int partial;
  float freqId,mainFreqId,diffFreqId;
  float minDiffFreqId;
  ATOM atom = NULL;
  
  kMax = molecule->dim;
  mainFreqId = mainAtom->freqId;
  minDiffFreqId = 1e5;
    
  for (k=1; k<= kMax; k++) {
    atom = GetMoleculeAtom(molecule,0,k-1);
    if (atom == NULL) continue;
    freqId = atom->freqId;
    diffFreqId = fabs(mainFreqId - freqId);

    if (diffFreqId < minDiffFreqId) {
      minDiffFreqId = diffFreqId;
      CopyAtom(atom,nearestPartial);
      partial = k;
    }
    else break;
  }
}

void GetFundamental(MOLECULE molecule,ATOM fundamental)	
{
  int k;
  int kMax;
  ATOM atom1 = NULL;
  float maxCoeff2;
        
  kMax = molecule->dim;
  if (kMax > 1) {
    maxCoeff2 = -1;
    for (k=0; k< kMax; k++) {
      atom1 = GetMoleculeAtom(molecule,0,k);
      if (atom1 == NULL) continue;
      if (atom1->coeff2 > maxCoeff2) {
	CopyAtom(atom1,fundamental);
	maxCoeff2 = atom1->coeff2;
      }
    }
    if (maxCoeff2 == -1) Errorf("GetFundamental() : Weird !");
  }
  else {
    atom1 = GetMoleculeAtom(molecule,0,0);
    CopyAtom(atom1,fundamental);
  }
}


/*

int MarkAtoms(BOOK book,ATOM mainAtom,int numAtom,int *flagNotes,float * beginNoteId,float endNoteId)
{
static ATOM partial = NULL;
static MOLECULE moleculeSearch = NULL;   

int n;
int timesFreq;
int findedAtoms;
int partialSize;
float mainFreq,partialFreq;
float diffFreq,freqFactor;
float sigma2;
    
if (partial == NULL) {
partial = NewAtom();
moleculeSearch = NewMolecule();
}    
     
findedAtoms = 0;   
for (n = numAtom; n<book->size; n++)
{
	moleculeSearch = GetBookMolecule(book,n);
        
	GetNearestPartial(moleculeSearch,mainAtom,partial);
	if (partial->freqId > mainAtom->freqId){
	    timesFreq = floor((partial->freqId)/(mainAtom->freqId));
	    if (timesFreq > 1){
		if ((partial->freqId - (timesFreq*mainAtom->freqId)) < mainAtom->freqId/2)
		    partial->freqId -= ((timesFreq-1)*mainAtom->freqId);
		else partial->freqId -= (timesFreq*mainAtom->freqId);	
	    }
	    else{
		
		if ((partial->freqId - mainAtom->freqId) > (mainAtom->freqId/2)){
		    partial->freqId -= mainAtom->freqId;
		}
	    }
	}
	partialSize = partial->windowSize;
	if ( (partial->timeId > (*beginNoteId-partialSize/4))&&(partial->timeId < endNoteId ) )
	{
	    mainFreq 	= FreqId2Freq(mainAtom,mainAtom->freqId);
	    partialFreq = FreqId2Freq(mainAtom,partial->freqId);
	    sigma2 = (18*(15-partial->octave));
	    diffFreq = (mainFreq-partialFreq);
	    if ( partial->octave < 11)
		diffFreq /= ( 12 - partial->octave );   
	    freqFactor = exp(-(diffFreq*diffFreq)/sigma2);
	    if (freqFactor > 0.005)
	    {
		findedAtoms++;
		if (*(flagNotes +n-1)) continue;
		Printf("\t\t%d:(o:%d,dif:%f,factor:%f)MARKED\n",n,partial->octave,diffFreq,freqFactor);
		*(flagNotes +n-1) = 1;
	    }
	}
    }
    return(findedAtoms);
}

void GetBestPartial(MOLECULE molecule,ATOM bestPartial)
{
    int k;
    int kMax;
    int partialMax;
    float coeff2,maxCoeff2;
    ATOM atom = NULL;
    
    kMax = molecule->dim;
    maxCoeff2 = 0.0;
    partialMax = 0;
    for (k=1; k<= kMax; k++)
    {
	atom = GetMoleculeAtom(molecule,0,k-1);
	if (atom == NULL) continue;
	coeff2 = atom->coeff2;
	if (coeff2 > maxCoeff2) 
	{
	    CopyAtom(atom,bestPartial);
	    maxCoeff2 = coeff2;
	    partialMax = k;
	}
    }

}


void GetFundamental(MOLECULE molecule,ATOM fundamental)	
{
  int k;
  int kMax;
  ATOM atom1 = NULL;
  ATOM atom2 = NULL;
  
  kMax = molecule->dim;
  if (kMax > 1) {
    for (k=1; k<= kMax; k++) {
      atom1 = GetMoleculeAtom(molecule,0,k-1);
      atom2 = GetMoleculeAtom(molecule,0,k);
      if ((atom1 == NULL)||(atom2 == NULL)) Errorf("GetFundamental");
      if ( atom1->coeff2 > 0.1*atom2->coeff2)  {
	CopyAtom(atom1,fundamental);
	break;
      }
    }
  } else {
    atom1 = GetMoleculeAtom(molecule,0,0);
    CopyAtom(atom1,fundamental);
  }
}
*/


/* EOF */


