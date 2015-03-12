/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry and J.Abadia           */
/*      email  : remi.gribonval@inria.fr                                    */
/*               lastwave@cmapx.polytechnique.fr                            */
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



#ifndef STFT_H

#define STFT_H

#include "tabulate.h"

/************************************************************/
/*
 * 	Header file for the definition of time-frequency structures : 
 * 
 *		Short Time Fourier Transform
 */
/************************************************************/


// Remi 24/07/2001 
/*
 * This is an abstract structure for the elements of any time-frequency
 * structure (ex: atom,stft,harmo,dict,book ...)
 *
 * Any such structure must start with the ATFCONTENT structure fields, this 
 * allows time-frequency type structures to be used as such and benefit from 
 * common functions (typically, time-frequency unit conversion).
 */
#define GABOR_MAX_FREQID     (1<<16)
#define GABOR_NYQUIST_FREQID (GABOR_MAX_FREQID/2)
#define GABOR_MAX_CHIRPID    (GABOR_MAX_FREQID/4)

#define ATFContentFields \
	ValueFields; \
	float x0; \
	float dx; \
	unsigned long signalSize

typedef struct aTFContent {
  ATFContentFields;
} *ATFCONTENT;

extern float LW_TFConvertUnit(const void *content,float value,char unitType,char conversionType);

//The time-frequency unit types
enum {
	LW_TF_TimeUnit = 1,
	LW_TF_FreqUnit,
	LW_TF_ChirpUnit,
	LW_TF_LastUnit
};
//The conversion types
enum {
	LW_TF_Id2RealConvert = 1,
	LW_TF_Real2IdConvert,
	LW_TF_LastConvert
};

//Shorthands for unit conversions
#define Time2TimeId(a,b)	LW_TFConvertUnit((ATFCONTENT)(a),(b),LW_TF_TimeUnit,LW_TF_Real2IdConvert) 
#define TimeId2Time(a,b)	LW_TFConvertUnit((ATFCONTENT)(a),(b),LW_TF_TimeUnit,LW_TF_Id2RealConvert) 
#define Freq2FreqId(a,b)	LW_TFConvertUnit((ATFCONTENT)(a),(b),LW_TF_FreqUnit,LW_TF_Real2IdConvert) 
#define FreqId2Freq(a,b)	LW_TFConvertUnit((ATFCONTENT)(a),(b),LW_TF_FreqUnit,LW_TF_Id2RealConvert) 
#define Chirp2ChirpId(a,b)	LW_TFConvertUnit((ATFCONTENT)(a),(b),LW_TF_ChirpUnit,LW_TF_Real2IdConvert) 
#define ChirpId2Chirp(a,b)	LW_TFConvertUnit((ATFCONTENT)(a),(b),LW_TF_ChirpUnit,LW_TF_Id2RealConvert) 

#define FreqNyquist(a)          FreqId2Freq((ATFCONTENT)(a),GABOR_NYQUIST_FREQID)

#define InitTFContent(content) \
	if((content) == NULL) \
	Errorf("InitTFContent : NULL content"); \
    (content)->x0		    = 0.0; \
    (content)->dx		    = 1.0; \
    (content)->signalSize = 0

#define CheckTFContent(content) \
	if((content) == NULL) \
	Errorf("CheckTFContent : NULL content"); \
    if((content)->dx <= 0.0) \
	Errorf("CheckTFContent : bad dx %g",(content)->dx); \

#define CopyFieldsTFContent(in,out) \
	if((in) == NULL || (out) == NULL) \
	Errorf("CopyTFContent : NULL in or out"); \
	if((ATFCONTENT)(in)==(ATFCONTENT)(out)) return; \
	(out)->x0			= (in)->x0; \
	(out)->dx			= (in)->dx; \
	(out)->signalSize	= (in)->signalSize

#define CheckTFContentCompat(content,subContent) \
	if((content) == NULL || (subContent) == NULL) \
	Errorf("CheckTFContentCompat : NULL in or out"); \
        if(((content)->x0         != (subContent)->x0) || \
	   ((content)->dx         != (subContent)->dx) || \
           ((content)->signalSize != (subContent)->signalSize)) \
	Errorf("CheckTFContentCompat : Weird error")

extern void * GetDxTFContentV(ATFCONTENT val, void **arg);
extern void * SetDxTFContentV(ATFCONTENT val, void **arg);
extern void * GetX0TFContentV(ATFCONTENT val, void **arg);
extern void * SetX0TFContentV(ATFCONTENT val, void **arg);
extern void * GetSignalSizeTFContentV(ATFCONTENT val, void **arg);
extern void * GetFreqIdNyquistTFContentV(VALUE val, void **arg);


/* SOME DEFAULT VALUES FOR STFT */
/* Min and Max possible window size for a stft */
#define STFT_MIN_WINDOWSIZE     1<<2
#define STFT_MAX_WINDOWSIZE     (1<<14)

/* Default type for a stft : complex, zero-padded, gabor window */
#define STFT_DEFAULT_STFTTYPE        ComplexStft
#define STFT_DEFAULT_TIMEREDUND      (1<<2)                  // One quarter of window
#define STFT_DEFAULT_FREQREDUND      (1<<1)
#define STFT_DEFAULT_BORDERTYPE      BorderPad0
#define STFT_DEFAULT_WINDOWSHAPE     GaussWindowShape        //Gaussian window
#define STFT_DEFAULT_WINDOWSIZE      STFT_MIN_WINDOWSIZE


/* Default values for harmo */
#define STFTHARMO_DEFAULT_DIMHARMO 10
#define STFTHARMO_DEFAULT_IFMO     5

/*
 * The different types of stfts :
 *  	ComplexStft : Complex valued window fourier transform
 *     	RealStft    : Squared inner products with optimized real fourier atoms
 *     	PhaseStft   : Phase of optimized real fourier atoms
 *	HighResStft : High resolution correlation
 *      HarmoStft   : Harmonic atoms
 *      StereoStft  : Stereo atoms
 */
 
enum {
  ComplexStft,
  RealStft,
  PhaseStft,
  HighResStft,
  HarmoStft,
  LastStftType
};

#define     StftTypeIsOK(stftType)	(INRANGE(0,(stftType),LastStftType-1))
extern char *StftType2Name(char stftType);
extern char Name2StftType(char *stftType);
#define StftTypeHelpString "The various stft types can be 'complex','real','phase', 'highres' (in which case you must specify a '<depthHighRes>') and 'harmo' (in which case a '<dim>' and '<freq0Min> <freq0Max>' must be specified)."


/************************************************************************/
/*
 *   The structure   of   a stft (SHORT TIME FOURIER TRANSFORM):
 */
/*
 * 'borderType' (see "signals.h" for the list of possible border types): 
 *
 *	Is the spectrogram computed assuming the signal is 
 *	zero-padded,periodized,symetric or antisymetric ?
 *
 * 'firstp'     :
 *      timeId of the first point not affected by left side effect
 * 'lastp'      :
 *      timeId of the last point not affected by right side effect
 *
 * 'freqIdMin', 'freqIdMax' : ????
 *
 * 'windowShape','windowSize' :
 *	Specifies the analyzing window. See "tabulate.h" for the list
 *      of analyzing windows.
 *
 */
/*	'type'	     :
 *	Either a COMPLEX spectrogram, an ENERGY MAP, a PHASE MAP,
 *      a HIGHRES spectrogram or an HARMO
 *
 *	'real'
 *	'imag' :
 *   	'coeff2s' 
 *      'phase'   :
 *	are made up of 'nFrames' appended signals of size 'nSubBands'
 *	corresponding to the different times. 
 *      Each signal represents 
 *      -real/imag : a spectrum real/imaginary part along frequencies,
 *	These signals are used to store the real and imaginary parts of
 *	the inner products of the analyzed signal with the
 *      COMPLEX atoms.
 *	-coeff2s   : a spectral power density.
 *	This signal is used to store the correlation between
 *	the analyzed signal and the REAL valued atoms, that is to say :
 *	 -the squared energy of the REAL VALUED atom,
 *	  when the low resolution is used (RealStft)
 *	 -the high-resolution correlation C(s,g)^2 between the analyzed
 *	  signal and the REAL VALUED atom in the case of a HighResStft.
 *
 *	tRate*nFrames     = signalSize
 *	fRate*(nSubBands-1) = GABOR_NYQUIST_FREQID
 */
/***********************************************************************/
typedef struct stft {
   // The fields of the ATFCONTENT structure
  ATFContentFields;
  int tRate;
  unsigned int nFrames;
  int fRate;
  unsigned short nSubBands;
  char   borderType;
  int   firstp;
  int   lastp;
  int freqIdMin;
  int freqIdMax;
  char windowShape;
  int  windowSize;
  char flagUpToDate;

  // Specific fields : the type of stft */
  char type;
  // The complex or real data 
  SIGNAL real;
  SIGNAL imag;

  // The harmo parameters
  int dimHarmo;
  int iFMO;
  
} Stft, *STFT;

/* Dealing with STFT variables */
extern char 	*stftType;

extern STFT 	TNewStft(void);

extern STFT 	GetStftCur(void);

extern int 	tSTFT_;
extern int 	tSTFT;


/* STFT Allocations */
extern void 	CheckStft(const STFT stft);
extern void 	CheckStftComplex(const STFT stft);
extern void 	CheckStftReal(const STFT stft);
extern void     CheckStftPhase(const STFT stft);
extern void 	CheckStftHighRes(const STFT stft);
extern void 	CheckStftHarmo(const STFT stft);
extern void 	CheckStftNotEmpty(const STFT stft);
extern void 	CheckStftUpToDate(const STFT stft);
extern void     CheckStftGridCompat(const STFT stftGrid,const STFT stftSubGrid);

extern STFT 	NewStft(void);
extern void 	ClearStft(STFT stft);
extern STFT 	DeleteStft(STFT stft);

/* STFT I/O */
extern void 	PrintStft(const STFT stft,char flagShort);
extern void     WriteStftFile(STFT stft,char *filename,int flagHeader);
extern void     WriteStftStream(STFT stft,STREAM s,char flagHeader);

/* STFT settings : the grid, allocation size ... */
extern char     ComputeWindowGG(char windowShape,unsigned long windowSize,float freqId,float *pRealGG,float *pImagGG);

extern void     ComputeWindowSupport(int windowSize,char windowShape,
				     float timeId,
				     int *pTimeIdMin,int *pTimeIdMax);

extern void     TouchStft(STFT stft);
extern void	SizeStft(STFT stft,char windowShape,int windowSize,
			 int timeRedund,int freqRedund,
			 char borderType,char stftType,
			 float freq0IdMin,float freq0IdMax,
			 int iFMO,int dimHarmo);

/* STFT data managing */

/* Getting stft complex/real data */
extern void 	GetStftData(STFT stft,float timeId,float* *pReal,float* *pImag);
extern char 	GetStftMax(const STFT stft,char flagCausal,
			   int timeIdMin,int timeIdMax,
			   int freqIdMin,int freqIdMax,
			   int *pMaxTimeId,int *pMaxFreqId,float *pMax);

/******************************/
/*
 *	Utilities
 */
/******************************/
// To perform arithmetic operations on stfts
enum {
  STFT_ADD,
  STFT_SUBSTRACT,
  STFT_MULTIPLY,
  STFT_DIVIDE,
  STFT_LN,
  STFT_LOG,
  STFT_LOG2,
  STFT_CONJUGATE,
  //  STFT_REAL,
  //  STFT_IMAG,
  //  STFT_MOD,
  //  STFT_ARG,
  STFT_LASTOPER
};

#define StftOperIsOK(oper) (INRANGE(0,(oper),STFT_LASTOPER-1))

extern char* StftOper2Name(char oper);
extern char  Name2StftOper(char *name);

extern void     StftOper(const STFT stftIn1,const STFT stftIn2,STFT stftOut,char oper);

extern void     ExtractSignal(SIGNAL input,char borderType,SIGNAL output,int outSize,int index1);

/* Update */
extern void     ZeroStft(STFT stft,unsigned long timeIdMin,unsigned long timeIdMax);
extern void     AddStft(STFT stftResult,STFT stftAdded,unsigned long timeIdMin,unsigned long timeIdMax);

extern void     ComputeRealPhaseEnergy(float coeffR,float coeffI,float realGG,float imagGG,
				       float *pPhase,float *pCosPhase,float *pSinPhase,
				       float *pEnergy);


extern void	UpdateStftComplex(STFT stftComplex,SIGNAL signal,int timeIdMin,int timeIdMax);
extern void     UpdateStftRealOrPhase(STFT stft,STFT stftComplex,int timeIdMin,int timeIdMax);
extern void 	UpdateStftHarmo(STFT stftHarmo,STFT stftReal,int timeIdMin,int timeIdMax);
extern void UpdateStftHighRes(STFT stftHighRes,STFT stftReal,STFT stftPhase,STFT subStftReal,STFT subStftPhase,int timeIdMin,int timeIdMax);

extern char     HarmoPartialBox(STFT stftHarmo,int k,int freq0Id,
				int* pKFreq0IdMin,int* pKFreq0IdMax);


extern void     ComputeUpdateLimits(STFT stft,int timeIdMin,int timeIdMax,
				    int *pTimeIdMin,int *pTimeIdMax,
				    char *pFlagTwoLoops,int *pTimeIdMin1,int *pTimeIdMax1);

extern void 	QuantizeRangeLarge(float min,float max,int rate,
				   int *pMin,int *pMax);

extern void 	DefineGraphStft(void);

// Some functions to access tabulated data
#define CCATOM_PRECISION 1e-5
extern void   GetTabWindow(char windowShape,unsigned long windowSize,SIGNAL *windowPtr);
extern void   GetTabExponentials(SIGNAL *expikRptr,SIGNAL *expikIptr);
extern void   GetTabGG(char windowShape, unsigned long windowSize,SIGNAL *GGRptr,SIGNAL *GGIptr);
#endif

/* EOF */

