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
/*  mp_graph.c   Functions for displaying books                             */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "mp_book.h"


/* The GOBJECT structure for displaying a BOOK */
typedef struct graphBook {

  GObjectFields; 
  
  /* The book to be displayed */
  BOOK book;

  /* The limits on which atoms to display */
  unsigned long nMin,nMax;                      /* minimum and maximum ranks of molecules that are displayed */
  unsigned long windowSizeMin, windowSizeMax;   /* 'windowSize' range */
  float chirpIdMin,chirpIdMax;  /* 'chirpId' range */

  /* Some flags changing the color-code */
  char flagFund;                /* 
				 * the way an harmonic molecule is displayed :
				 * see help in function _SetGraphBook
				 */
  char flagDb;                  /* switches decibel display */
  float exponent;               /* dynamic range (dB) / exponent ... */
  
  //  char flagLogFreq;
  //  float freqSwitch;

  /* The colormap used */
  unsigned long cm;

} GraphBook, *GRAPHBOOK;



/* The corresponding class */      
GCLASS theGraphBookClass = NULL;

enum {
  FundAll=NO,
  FundMax,
  FundSum
};

/* Some default values */
#define DEFAULT_GRAPH_BOOK_FLAGFUND     FundAll
#define DEFAULT_GRAPH_BOOK_FLAGDB       YES
#define DEFAULT_GRAPH_BOOK_EXPONENT     30.0
//#define DEFAULT_GRAPH_BOOK_FLAGLOGFREQ  NO
//#define DEFAULT_GRAPH_BOOK_FREQSWITCH   300.0


/* Initialization of the GraphBook structure */
static void _InitGraphBook(GOBJECT o)
{
  GRAPHBOOK graph;

  graph = (GRAPHBOOK) o;

  graph->bgColor = invisibleColor;

  graph->rectType = SmallRect;
  
  graph->book = NULL;

  graph->nMin = 0;
  graph->nMax = 0;

  graph->windowSizeMin = STFT_MIN_WINDOWSIZE;
  graph->windowSizeMax = STFT_MAX_WINDOWSIZE;

  graph->chirpIdMin = 0.0;
  graph->chirpIdMax = 0.0;

  graph->flagFund   = DEFAULT_GRAPH_BOOK_FLAGFUND;
    
  graph->flagDb   = DEFAULT_GRAPH_BOOK_FLAGDB;
  graph->exponent = DEFAULT_GRAPH_BOOK_EXPONENT;

  //  graph->flagLogFreq = DEFAULT_GRAPH_BOOK_FLAGLOGFREQ;
  //  graph->freqSwitch  = DEFAULT_GRAPH_BOOK_FREQSWITCH;

  graph->book = NULL;
  
  graph->cm   = GetColorMapCur();  
}


/* Deleting the content of a GraphBook */
static void _DeleteContentGraphBook(GOBJECT o)
{
  GRAPHBOOK graph;

  graph = (GRAPHBOOK) o;

  if (graph->book != NULL) graph->book = DeleteBook(graph->book);
}

/* A utility that tells whether an atom is to be displayed in a graph */
static char IsVisible(GRAPHBOOK graph,ATOM atom)
{
  if (!INRANGE(graph->windowSizeMin,atom->windowSize,graph->windowSizeMax))  return(NO);
  if (!INRANGE(graph->chirpIdMin,atom->chirpId,graph->chirpIdMax)) return(NO);

  return(YES);
}


/* The setg method */
static int _SetGraphBook (GOBJECT o, char *field, char**argv)
{
  GRAPHBOOK graph;
  BOOK book;
  MOLECULE molecule;
  ATOM atom=NULL;
  char  *str;

  int   imin,imax;
  float fmin,fmax;
  float t,f,timeId,freqId;
  int   flag;
  
  unsigned long n,nClosest;
  unsigned short k,kClosest;
  int timeIdMin,timeIdMax;
  float freqIdMin,freqIdMax;
  float dist,distClosest;

  LISTV lv;
  
  /* The help command */
  if (o == NULL) {
    SetResultStr("{{{graph [<book>]} {Gets/Sets the book to be displayed by the GraphBook. (The '-cgraph' field is equivalent to that field).}} \
{{cm [<colormap>]} {Sets/Gets the colormap that will be used to display the book.}} \
{{scale [<windowSizeMin> <windowSizeMax>]} {Sets/Gets the windowSize range of the molecules that are displayed.}} \
{{chirp [<chirpMin> <chirpMax>]} {Sets/Gets the chirp range of the molecules that are displayed.}} \
{{chirpId [<chirpIdMin> <chirpIdMax>]} {Sets/Gets the chirpId range of the molecules that are displayed.}} \
{{n [<nMin> <nMax>]} {Sets/Gets the minimum and maximum ranks of the molecules that are displayed.}} "
"{{fund <flag>} {Sets/Gets the fundamental flag which can be 'all','max' or 'sum'. This flag allows to display all the atoms within each molecule (<flag>='all') or \
only the most energetic atom with its energy (<flag>='max') or with the total energy of the molecule (<flag>='sum').}} "
" {{db [<flagOnOff>]} {Sets/Gets the decibel-display flag. If it is on, the energy of the molecules is displayed in decibel.}} \
{{expo [<exponent>]} {Sets/Gets the exponent used for display. If '-db' is off, then the energy to the <exponent> of the molecules is displayed. If '-db' is on, then the same quantity is displayed in decibel.}} \
{{?closest <time> <freq>} {Gets the rank of the atom that is currently displayed and that is the closest to position <time> <freq>. It returns either 'null' if it fails or a listv {<n> <k>} where 0<=n<book.size is the rank of the molecule  and 0<=k<molecule.dim is the atom number in the molecule.}}}");
    return(YES);
  }
   
      
  graph = (GRAPHBOOK) o;
  book = graph->book;

  
  /* the 'graph' and 'cgraph' fields */
  if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) {
    if (*argv == NULL) {
      SetResultValue(graph->book); 
      return(YES);
    }
    argv = ParseArgv(argv,tBOOK,&book,0);
    CheckBookNotEmpty(book);
    if (graph->book != NULL) DeleteBook(graph->book);
    graph->book = book;
    AddRefValue(book);

    graph->nMin = 0;
    graph->nMax = book->size-1;

    graph->windowSizeMin = STFT_MIN_WINDOWSIZE;
    graph->windowSizeMax  = STFT_MAX_WINDOWSIZE;
    graph->chirpIdMax = GABOR_MAX_CHIRPID;
    graph->chirpIdMin = -graph->chirpIdMax;

    o->rx = TimeId2Time(book,-.5);
    o->rw = TimeId2Time(book,book->signalSize-.5);
    o->rw-=o->rx;
    o->ry = FreqId2Freq(book,-.5);
    o->rh = FreqId2Freq(book,GABOR_NYQUIST_FREQID+.5);
    o->rh-=o->ry;
    UpdateGlobalRectGObject(o);    
    return(YES);
  }

  /* The octave field */
  if (!strcmp(field,"s")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendInt2Listv(lv,graph->windowSizeMin);
      AppendInt2Listv(lv,graph->windowSizeMax);
      return(YES);  
    }    
    argv = ParseArgv(argv,tINT,&imin,tINT,&imax,0);
    imin = MAX(imin,STFT_MIN_WINDOWSIZE);
    imax = MIN(imax,STFT_MAX_WINDOWSIZE);
    if (imin>imax) Errorf("_SetGraphBook : Option '-scale' : Minimum windowSize value '%d' cannot be greater than maximum windowSize value '%d'",imin,imax);
    graph->windowSizeMin = imin;
    graph->windowSizeMax = imax;
    return(YES);
  }
 
  /* The chirpId field */
  if (!strcmp(field,"chirpId")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,graph->chirpIdMin);
      AppendFloat2Listv(lv,graph->chirpIdMax);
      return(YES);  
    }    
    argv = ParseArgv(argv,tFLOAT,&fmin,tFLOAT,&fmax,0);
    fmin = MAX(fmin,-GABOR_MAX_CHIRPID);
    fmax = MIN(fmax,GABOR_MAX_CHIRPID);
    if (fmin > fmax)
      Errorf("_SetGraphBook : Option '-chirpId' : Minimum chirpId value '%g' cannot be greater than maximum chirpId value '%g'",fmin,fmax);
    graph->chirpIdMin = fmin;
    graph->chirpIdMax = fmax;
    return(YES);
  }
 
  /* The chirp field */
  if (!strcmp(field,"chirp")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,ChirpId2Chirp(graph->book,graph->chirpIdMin));
      AppendFloat2Listv(lv,ChirpId2Chirp(graph->book,graph->chirpIdMax));
      return(YES);  
    }    
    argv = ParseArgv(argv,tFLOAT,&fmin,tFLOAT,&fmax,0);
    fmin = MAX(Chirp2ChirpId(graph->book,fmin),-GABOR_MAX_CHIRPID);
    fmax = MIN(Chirp2ChirpId(graph->book,fmax),GABOR_MAX_CHIRPID);
    if (fmin > fmax)
      Errorf("_SetGraphBook : Option '-chirp' : Minimum chirp value '%g' cannot be greater than maximum chirp value '%g'",fmin,fmax);
    graph->chirpIdMin = fmin;
    graph->chirpIdMax = fmax;
    return(YES);
  }
 
  /* The closest field */
  if (!strcmp(field,"?closest")) {
     argv = ParseArgv(argv,tFLOAT,&t,tFLOAT,&f,0);
     timeId = Time2TimeId(book,t);
     freqId = Freq2FreqId(book,f);
     /* Some inits */
     distClosest  = -1.0;

     /* Look for all the molecules of which some atom is displayed in this graph */
     for (n = MAX(0,graph->nMin); n <= MIN(book->size-1,graph->nMax); n++) {
       molecule = GetBookMolecule(book,n);
       for (k=0;k<molecule->dim;k++) {
         atom = GetMoleculeAtom(molecule,0,k);
         if (!IsVisible(graph,atom)) continue;
         /* 
          * Detect if (timeId,freqId) is indeed INSIDE some atom.
          */
         ComputeWindowSupport(atom->windowSize,atom->windowShape,atom->timeId,&timeIdMin,&timeIdMax);
         freqIdMin = atom->freqId-(GABOR_MAX_FREQID/atom->windowSize)/(2.0);
         freqIdMax = atom->freqId+(GABOR_MAX_FREQID/atom->windowSize)/(2.0);
         if(INRANGE(timeIdMin,timeId,timeIdMax) && INRANGE(freqIdMin,freqId,freqIdMax)) {
           distClosest = 0.0;
           nClosest    = n;
           kClosest    = k;
           break;
         }
         /* When not 'inside' the atom, compute the distance to (timeId,freqId) */
         dist = pow(fabs(timeId-atom->timeId),2.)+pow(fabs(freqId-atom->freqId),2.);
         /* Update the location of the closest molecule, if necessary */
         if (distClosest < 0 || distClosest > dist) {
            distClosest = dist;
            nClosest    = n;
            kClosest    = k;
          }
       }
       /* Case when (timeId,freqId) is 'inside' some displayed atom */
       if (distClosest == 0.0) break;
    }
    if (distClosest < 0) {
      SetResultValue(nullValue);
    }
    else {
      lv = TNewListv();
      SetResultValue(lv);
      AppendInt2Listv(lv,nClosest);
      AppendInt2Listv(lv,kClosest);
    }     
    return(YES);
  }

  /* The n field */
  if (!strcmp(field,"n")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendInt2Listv(lv,graph->nMin);
      AppendInt2Listv(lv,graph->nMax);
      return(YES);  
    }    
    argv = ParseArgv(argv,tINT,&imin,tINT,&imax,0);
    imin = MAX(imin,0);
    imax = MIN(imax,book->size-1);    
    if (imin>imax) Errorf("_SetGraphBook : Option '-n' : Minimum n-value '%d' cannot be greater than maximum n-value '%d'",imin,imax);
    graph->nMin = imin;
    graph->nMax = imax;
    return(YES);
  }

   /* The fund field */
  if (!strcmp(field,"fund")) {
    if (*argv == NULL) {
      switch(graph->flagFund) {
      case FundAll :
        SetResultStr("all");
        break;
      case FundMax :
        SetResultStr("max");
        break;
      case FundSum :
        SetResultStr("sum");
        break;
      default :
        Errorf("???? bad flagFund");
      }
      return(YES);  
    }    

    argv = ParseArgv(argv,tSTR,&str,0);
    if(!strcmp(str,"all")) {
     graph->flagFund = FundAll;
     return(YES);
    } 
    if(!strcmp(str,"max")) {
     graph->flagFund = FundMax;
     return(YES);
    }
    if(!strcmp(str,"sum")) {
     graph->flagFund = FundSum;
     return(YES);
    }
    Errorf("_SetGraphBook : Option -fund : bad value '%s'",str);
    return(NO);
  }

  /* The db field */
  if (!strcmp(field,"db")) {
    if (*argv == NULL) {
      SetResultInt((int) graph->flagDb);
      return(YES);  
    }    
    argv = ParseArgv(argv,tINT,&flag,0);
    graph->flagDb = (flag!=0);
    return(YES);
  }

  /* The expo field */
  if (!strcmp(field,"expo")) {
    if (*argv == NULL) {
      SetResultFloat(graph->exponent);
      return(YES);  
    }    
    argv = ParseArgv(argv,tFLOAT,&(graph->exponent),0);
    return(YES);
  }

//  /* The logFreq field */
//  if (!strcmp(field,"logfreq")) {
//    if (*argv == NULL) {
//      SetResultInt((int) graph->flagLogFreq);
//      return(YES);  
//    }    
//    argv = ParseArgv(argv,tINT,&flag,0);
//    graph->flagLogFreq = (flag!=0);
//    return(YES);
//  }

//  /* The expo field */
//  if (!strcmp(field,"freqswitch")) {
//    if (*argv == NULL) {
//      SetResultFloat(graph->freqSwitch);
//      return(YES);  
//    }    
//    argv = ParseArgv(argv,tFLOAT,&(graph->freqSwitch),0);
//    return(YES);
//  }

  /* The colormap field */
  if (!strcmp(field,"cm")) {
    if (*argv == NULL) {
      SetResultStr(GetColorMapName(graph->cm));
      return(YES);  
    }    
    argv = ParseArgv(argv,tCOLORMAP,&(graph->cm),0);
    return(YES);
  }

  /* The 'rect' field */
  if (!strcmp(field,"rect")) {
    NoMoreArgs(argv);

    if (book == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0); 
      return(YES);
    }
    o->rx = TimeId2Time(book,-.5);
    o->rw = TimeId2Time(book,book->signalSize-.5);
    o->rw-=o->rx;
    o->ry = FreqId2Freq(book,-.5);
    o->rh = FreqId2Freq(book,GABOR_NYQUIST_FREQID+.5);
    o->rh-=o->ry;
    UpdateGlobalRectGObject(o);    
    lv = TNewListv();
    SetResultValue(lv);
    
    AppendFloat2Listv(lv,o->rx);
    AppendFloat2Listv(lv,o->ry);
    AppendFloat2Listv(lv,o->rw);
    AppendFloat2Listv(lv,o->rh);
    return(YES);
  }

  return(NO);
}


/*********************************************************************
 * Functions to fill the display area with the Wigner distribution
 *********************************************************************/


/*
 * The Wigner-Ville distribution of a Gaussian atom is
 *  GWV(2*(u/2^o)) x GWV(2*(2*pi*\sigma^2* k*2^o/GABOR_MAX_FREQID))
 *
 *  GWV(x) = e^{-x^2/4\sigma^2}
 */
float *GaussianWignerVille(int sizeGWV)
{
    float *GWV;
    float c = 1/(4*theGaussianSigma2*sizeGWV*sizeGWV);
    int i;
        
    /* Allocation */
    if ((GWV = FloatAlloc(sizeGWV)) == NULL) 
        Errorf("GaussianWignerVilleTime : Mem. Alloc. failed!");
        
    /* Computation of e^{-x^2/\sigma^2}, x = i/sizeGWV*/
    for (i = 0; i < sizeGWV; i++) 
    {
        GWV[i] = exp(-c*i*i);
    }
    return(GWV);
}

/* 
 * The Wigner-Ville distribution of a FoF atom is
 *  FWV(2*(u/2^o)) x GWV(2*(2*pi*\sigma^2* k*2^o/GABOR_MAX_FREQID))
 *
 *  FWV(x) = ???
 */
float *FoFWignerVille(int sizeFWV)
{
    float *FWV;
    float a,expon,beta,max;
    int i;
        
    /* Allocation */
    if ((FWV = FloatAlloc(sizeFWV)) == NULL) 
        Errorf("FoFWignerVilleTime : Mem. Alloc. failed!");
    
    /* Damping factor */
    a = log(decayFoF);
    
    /* scale */
    expon = a/sizeFWV; 
    beta = betaFoF/sizeFWV;        
    
    max=0.0;

    /* Computation of FoF window */
    for (i = 0; i <= M_PI/beta; i++) 
    {
        FWV[i] = 0.5*(1-cos(beta*i))*exp(-expon*i);
	if (FWV[i] > max) max=FWV[i];
    }
    for (; i<sizeFWV; i++)
    {
	FWV[i] = exp(-expon*i);
	if (FWV[i] > max) max=FWV[i];
    }

    /* Normalizing to the maximum */
    for (i = 0; i <sizeFWV; i++) 
    {
        FWV[i]=FWV[i]/max;
    }
    return(FWV);
}

/* 
 * Add the Wigner-Ville time-frequency distribution
 * of an atom to a time-frequency 'map' of nRow*nCol
 */
void DisplayGaborAtom(GOBJECT obj,float *map,ATOM atom,int atomK,int mapx,int mapy,int nRow,int nCol)
{
  GRAPHBOOK graph;
  BOOK book;

  /* The templates of the Wigner-Ville distributions */
  static int sizeG = 256;
  static float *GWV = NULL;
  static float *FWV = NULL;

  float timeMin,timeWidth,freqMin,freqWidth;
  float nTimeIdPerCol,nFreqIdPerRow;
  
  char flagAsym;

  int x,y,xcenter,ycenter,dx,dy;
  int xcurrent;
  int xmin,xmax,ymin,ymax;
  
  float iWV;
  int  t,f,k;

  float timeWV,freqWV;
  

  /* Some inits */
  graph = (GRAPHBOOK) obj;
  book = graph->book;
  if (book == NULL) return;
  CheckBookNotEmpty(book);
  flagAsym = GetFlagAsymWindowShape(atom->windowShape);

  /* Is this atom to be displayed in this graph ? */
  if(!IsVisible(graph,atom)) return;

  /* Initialize the templates of Wigner-Ville distribution */
  if(flagAsym) {
    if(FWV == NULL) {
      FWV = FoFWignerVille(sizeG);
    }
    if(GWV == NULL) {
      GWV = GaussianWignerVille(sizeG);
    }
  }
  else {
    if(GWV == NULL) {
      GWV = GaussianWignerVille(sizeG);
    }
  }
 
  /* 
   * Get the pixel coordinates (xcenter,ycenter)
   * of the 'center' of the atom, in map[] :
   */
  /* the coordinates in the gobject */
  Local2Global(obj,TimeId2Time(book,atom->timeId),FreqId2Freq(book,atom->freqId),&xcenter,&ycenter);
  /* change the origin to the corner of the map */
  xcenter -=mapx;
  ycenter -=mapy;

  /*
   * Get the pixel rectangle (xmin,xmax,ymin,ymax) corresponding to the atom
   * as well as the conversion ratios between pixel and (time/freq)Id scales
   */
  /* (time/freq)(Min/Width) <=> size of the map in (time/freq) units */
  Global2LocalRect(obj,mapx,mapy,nCol,nRow,
		   &timeMin,&freqMin,&timeWidth,&freqWidth,NormalRect);
  /* Conversion ratios */
  nTimeIdPerCol = (Time2TimeId(book,timeMin+timeWidth)-Time2TimeId(book,timeMin))/nCol;
  nFreqIdPerRow = (Freq2FreqId(book,freqMin+freqWidth)-Freq2FreqId(book,freqMin))/nRow;

  /* The rectangle for a Gabor atom */
  /* dx and dy are HALF the width and heighth of the box (in pixels) */
  if (!flagAsym) {
    dx = atom->windowSize/(2*nTimeIdPerCol);
  }
  // Except for asymmetric atoms !
  else {
    dx = atom->windowSize/nTimeIdPerCol;
  }
  dy = (GABOR_MAX_FREQID/(4*M_PI*theGaussianSigma2*atom->windowSize))/nFreqIdPerRow;
  /* Now the box itself */
  xmax = xcenter+dx;
  if (!flagAsym) {
    xmin = xcenter-dx;
  }
  else {
    xmin = xcenter;
  }
  ymax = ycenter+dy;
  ymin = ycenter-dy;
  
  /* Is there anything to be displayed ?
   * i.e. does the box intersect the pixmap ? */
  if (xmax < 0 || ymax < 0 || xmin >= nCol || ymin >= nRow) return;

  /* We do not print outside the pixmap */
  xmin = MAX(0,xmin);
  xmax = MIN(nCol-1,xmax);
  ymin = MAX(0,ymin);
  ymax = MIN(nRow-1,ymax);

  /* 
   * Fill the pixmap
   */
  /* Case of a Gabor atom */
  /* columns x = 'time' */
  for (x = xmin; x <= xmax; x++) {
    /* 
     * Compute the index in the template of Wigner-Ville distrib. 
     */
    /* current distance in pixels from the 'time-center' of the atom */
    iWV = abs(x-xcenter);
    /* conversion in timeId units */
    iWV *= nTimeIdPerCol; 

    /* conversion in percentage of the time-support of the template WV */
    /* For symetric atoms, we have two equal sides of size 2^o/2 */
    if (!flagAsym) {
      iWV /= atom->windowSize/2;
    }
    /* For asymetric ones, only one side of size 2^o */
    else {
      iWV /= atom->windowSize;
    }

    /* conversion in index in the template WV */
    t = sizeG*iWV;
    
    /* indexes outside the tabulated template WV 
     * are expected to be be very small and are not displayed
     */
    if (t>=sizeG) continue;
    
    /* 
     * It is time to use the template WV to get
     * the amplitude of the time factor of the WV.
     */
    if(flagAsym) {
      timeWV = FWV[t];
    }
    else {
      timeWV = GWV[t];
    }

    /* Now, we take into account the CHIRP to compute
     * (in pixel coordinates) the instantaneous frequency 
     * f = atomtime+atomchirp*(delta t)
     * with the 'delta t' corresponding to the current value of 'x'
     */
    Local2Global(obj,
                 TimeId2Time(book,atom->timeId+(x-xcenter)*nTimeIdPerCol),
                 FreqId2Freq(book,atom->freqId+atom->chirpId*(x-xcenter)*nTimeIdPerCol),&xcurrent,&ycenter); 
    xcurrent -=mapx;
    ycenter  -=mapy;  
    
    /* compute the local frequency box */
    ymax = ycenter+dy;
    ymin = ycenter-dy;
    ymin = MAX(0,ymin);
    ymax = MIN(nRow-1,ymax);
        
    /* loop on frequency/row within the local box */
    for (y = ymin; y <= ymax; y++) {
      /* 
       * Compute the index in the template of Wigner-Ville distrib. 
       */
      /* current distance in pixels from the 'frequency-center' of the atom */
      iWV = abs(y-ycenter);
      /* conversion in freqId units */
      iWV *= nFreqIdPerRow; 
      /* conversion in percentage of the freq-support of the template WV */
      iWV *= (4*M_PI*theGaussianSigma2*atom->windowSize)/(GABOR_MAX_FREQID);
      /* conversion in index in the template WV */
      f = sizeG*iWV;
            
      /* indexes outside the tabulated template WV 
       * are expected to be be very small and are not displayed
       */
      if (f>=sizeG) continue;
            
      /* 
       * It is time to use the template WV to get
       * the amplitude of the freq factor of the WV.
       */
      freqWV = GWV[f];
      /* Adding contribution at the right position in 'map' */
      k = y*nCol+x;
      map[k] += atom->coeff2*timeWV*freqWV;
    }
  }
  return;
}



/* The drawing procedure */
static void _DrawGraphBook (WINDOW win, GOBJECT obj, int x, int y,int w,int h)
{
  GRAPHBOOK graph;
  BOOK book;

  unsigned short kMin,kMax;
  int nColors;

  int nRow,nCol;
  int color;

  static float *map         = NULL;
  static int   mapAllocSize = 0;

  float maxMap,maxPartialEnergy,moleculeEnergy;

  unsigned long n;
  MOLECULE molecule;
  
  ATOM atom;

  int i,j;
  unsigned long k;


  /* Some inits */
  graph = (GRAPHBOOK) obj;
  book  = graph->book;
  if (book == NULL) return;
  CheckBookNotEmpty(book);

  /* Get the number of colors of the colormap */
  nColors = ColorMapSize(graph->cm);

  /* Allocation */
  nCol = w;
  nRow = h;
  WInitPixMap(nRow,nCol);
  
  /* Allocate a 'local map' of amplitudes and initialize with zeros */
  if(map != NULL && mapAllocSize < nRow*nCol) {
    Free(map);
    map          = NULL;
    mapAllocSize = 0;
  }
  if(map == NULL) {
    if ((map = FloatAlloc(nRow*nCol)) == NULL) 
        Errorf("_DrawGraphBook : Mem. Alloc. failed!");
    mapAllocSize = nRow*nCol;
  }
  for(k = 0; k < mapAllocSize; k++) map[k] = 0;

  /*
   * Do the plot in the local map
   */
  maxMap = -1;

  /* Draw each molecule within the graph's limits */
  for(n = graph->nMin; n <= MIN(book->size-1,graph->nMax); n++)  {
    molecule = GetBookMolecule(book,n);
    switch(graph->flagFund) {
      /* Case when we display all partials within a certain range */
    case FundAll :
      kMin = 0;
      kMax = molecule->dim-1;
      break;
      /* Case when we display only the most energetic partial */
    case FundMax :
    case FundSum :
      maxPartialEnergy = -1;
      moleculeEnergy       = 0;    
      /*
       * We locate the most energetic partial and compute
       * maxPartialEnergy    = the energy of this partial
       * moleculeEnergy          = the total energy of the molecule
       */
      for(k = 0; k < molecule->dim; k++) {
        atom = GetMoleculeAtom(molecule,0,k);
        moleculeEnergy += atom->coeff2;
        if (maxPartialEnergy < atom->coeff2) {
          maxPartialEnergy = MAX(maxPartialEnergy,atom->coeff2);
          kMin = kMax = k;
        }
      }
      break;
    }
      
    /* Now we do display the desired partials */
    for(k = kMin; k <= kMax; k++) {
      atom = GetMoleculeAtom(molecule,0,k);
      /*
       * Case 'FundSum' : 
       * the most energetic with the total energy of the molecule 
       */
      if (graph->flagFund == FundSum) {
	/* We temporarily modify the coeff2 and memorize the right one
           in maxPartialEnergy */
        maxPartialEnergy = atom->coeff2;
        atom->coeff2 = moleculeEnergy;
      }
             
      maxMap = MAX (maxMap,atom->coeff2);
          
      DisplayGaborAtom(obj,map,atom,k,x,y,nRow,nCol);
      
      /* We restore atom->coeff2 if necessary */
      if (graph->flagFund == FundSum) atom->coeff2 = maxPartialEnergy;

    }
  }
  
  /* Normalize and replace the map by its log (dB) or power, pixel by pixel */
  if(graph->flagDb) {
    for(k = 0; k < nCol*nRow; k++) {
      if(map[k] != 0.0) {
	/* Normalize */
	map[k] /= maxMap;
	/* Take decibels */
	map[k] = 10*log10(map[k]);
	/* Scale the dynamics with 'exponent' */
	map[k] /= graph->exponent;
	map[k] += 1;
	map[k] = MAX(map[k],0);
      }
    }
  }    
  else {
    for(k = 0; k < nCol*nRow; k++) { 
       if(map[k] != 0.0) {
	/* Normalize */
	map[k] /= maxMap;
	map[k] = pow(map[k],graph->exponent);
      }
    }
  }
  
  /* Puts the 'local map' into the pixmap, using the color-code */
  /* rows = 'freq' */
  for(i = 0; i < nRow; i++) {
    /* columns =  'time' */
    for(j = 0; j < nCol; j++) {
      k = i*nCol+j;
      color = (int) (nColors * map[k]);
      color = (color>=nColors ? nColors-1 : color);
      color = (color<0 ? 0 : color);
      color += graph->cm;
          
      WSetPixelPixMap(i,j,color);
    }
  }
  WDisplayPixMap(win,x,y);   
}

/* Defining the GraphBook gclass */  
void DefineGraphBook(void)
{
  theGraphBookClass          = NewGClass("GraphBook",theGObjectClass,"mp"); 
  theGraphBookClass->nbBytes = sizeof(GraphBook);
  theGraphBookClass->init    = _InitGraphBook;
  theGraphBookClass->deleteContent = _DeleteContentGraphBook;
  theGraphBookClass->set     = _SetGraphBook;
  theGraphBookClass->draw    = _DrawGraphBook;   
  theGraphBookClass->varType = bookType;
  theGraphBookClass->flags  &= ~(GClassMoveResize+GClassLocalCoor);
  theGraphBookClass->info    = "Graphic Class that allows to display Book";
}


/* EOF */

