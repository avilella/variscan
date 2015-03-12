/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0.1                     */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
/*      email : lastwave@cmap.polytechnique.fr                              */
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



/**************************************************************/
/*                                                            */
/* 	 signal.h :    The signal structure                       */
/*                                                            */
/**************************************************************/



#ifndef SIGNALS_H

#define SIGNALS_H

/******************************************************************************
 * There are three types of signals. 
 *
 * 1) XYSIG is made of two arrays X and Y 
 *    X is supposed to be made of increasing numbers
 *
 * 2) YSIG is supposed to have uniformed samples. 
 *    So X is not used. Instead, we use x0 (i.e., first x value) and 
 *    dx (i.e., the sample rate). 
 *
 *******************************************************************************/


#define XYSIG 1
#define YSIG 2


/******************************
 *
 *    The SIGNAL structure    
 *
 ******************************/

typedef struct signal {

 /* The fields of the VALUE structure */
  ValueFields;

  char *name;	            /* name of the signal */

  char type;			    /* either XYSIG or YSIG */

  float *X;			        /* the X array */
  float *Y;			        /* the Y array */

  unsigned int sizeMallocX;	/* size of the allocation for X */
  unsigned int sizeMallocY;	/* size of the allocation for Y */
  unsigned int size;        /* size of the signal */
  
  float x0;			        /* first x value (for type YSIG) */
  float dx;			        /* sample rate (for type YSIG) */
	
  int firstp;               /* index of the first point not affected by left side effect */
  int lastp;                /* index of the last point not affected by right side effect */
  
  float param;            /* distance between two successive uncorrelated points */
  
} *SIGNAL;


extern void DefineGraphSignal(void);


/******************************************/
/*     Functions in SIGNAL_ALLOC.c         */
/******************************************/

/*
 * The signal types
 */
extern char *signalType,*signaliType;
extern int tSIGNAL, tSIGNAL_;
extern int tSIGNALI,tSIGNALI_;


/* Setting a signal (managing extraction) */
struct fsiList;
extern char *SetSignalField_(SIGNAL sigLeft,char *field, struct fsiList *fsiList, float fltRight, VALUE val,char *equal, char *fieldName);
extern void *SetSignalField(SIGNAL sig,void **arg);
extern void *GetSignalField_(SIGNAL signal, void **arg);
extern void *GetSignalExtractField(SIGNAL signal, void **arg);


/* fields */
extern int SetSizeSignal(SIGNAL sig, int size);
extern int SetDxSignal(SIGNAL sig,float dx);
extern int SetX0Signal(SIGNAL sig,float x0);
extern int SetNameSignal(SIGNAL signal, char *name);
extern int SetXYSignal(SIGNAL sig,int xy);
extern int SetSizeAllocXSignal(SIGNAL sig,int size);
extern int SetSizeAllocYSignal(SIGNAL sig,int size);


/* (Des)Allocation of signals */
extern SIGNAL NewSignal(void);				/* Create a new signal structure and returns it  */
extern SIGNAL TNewSignal(void);				/* Create a new signal structure make it temporary and returns it  */
extern void DeleteSignal(SIGNAL signal);	/* Desalloc SIGNAL 'signal' */
extern void ClearSignal(SIGNAL signal);		/* Initializes 'signal' */
extern void SizeSignal(SIGNAL signal, int size, int type); /* Change Allocation to fit the size */


/*
 *
 * Functions that manage X, Y and index values f a signal 
 *
 */

extern float XSig(SIGNAL sig,int index);          /* Get xvalue to an index */
extern int ISig(SIGNAL signal,float xValue);      /* Get index to an x value */
extern float XYSig(SIGNAL signal,int index,char which); /* Get index of X or Y */

/*
 * The main routine that can be called from outside to get an index
 * associated to an x value ('xValue') whatever the type of the signal is.
 */ 
extern float X2FIndexSig(SIGNAL signal, float xValue);
 
/* The different interpolation mode */
typedef enum {
  InterNone = 0,
  InterLinear
} InterMode;

/* The different border effects */
typedef enum {
  BorderNone = 0,
  BorderPer,
  BorderMir,
  BorderMir1,
  BorderCon,
  Border0
} BorderType;

/*
 * The main routine that returns a Y value given an X value 
 * given an interpolation mode and a border type.
 */
extern float X2YSig(SIGNAL signal,float x, InterMode im, BorderType bt, char flagIndex);



/*
 * Copy functions 
 */
extern void CopyFieldsSig(SIGNAL in,SIGNAL out); /* Copy the fields of a signal */
extern SIGNAL CopySig(SIGNAL in, SIGNAL out);      /* Copy a signal into another  */
extern void CopySigXX(SIGNAL inX,SIGNAL outY,char *type); 


/*
 * Parsing functions 
 */
extern char ParseSignalILevel_(LEVEL level, char *arg, SIGNAL defVal, SIGNAL *sig);
extern char ParseSignalI_(char *arg, SIGNAL defVal, SIGNAL *sig);
extern void ParseSignalILevel(LEVEL level, char *arg, SIGNAL *sig);
extern void ParseSignalI(char *arg, SIGNAL *sig);
extern char ParseSignal_(char *arg, SIGNAL defVal, SIGNAL *sig);
extern void ParseSignal(char *arg, SIGNAL *sig);
extern char ParseSignalLevel_(LEVEL level,char *arg, SIGNAL defVal, SIGNAL *sig);
extern void ParseSignalLevel(LEVEL level, char *arg, SIGNAL *sig);
 

/******************************************/
/*     Functions in SIGNAL_FUNCTIONS.c         */
/******************************************/

/* Put a signal to 0 */
extern void ZeroSig(SIGNAL sig);

/* Compute the Min and the max of a signal */
extern void MinMaxSig(SIGNAL signal,float *pxMin,float *pxMax,float *pyMin,float *pyMax,
               int *piyMin,int *piyMax,int flagCausal);

extern void ThreshSig(SIGNAL in, SIGNAL out, int flagX, int flagY,int flagMin, float min, int flagMax,float max);
extern void SortArrays(float *x, float *y, int n);
extern void SortSig(SIGNAL signal);
extern void MergeSig(SIGNAL in1, SIGNAL in2, SIGNAL out, int flag);
extern void PaddSig(SIGNAL sig,SIGNAL sigOut,int borderType,int newSize);
extern void ExtractSig(SIGNAL sig,SIGNAL sigOut, int borderType, int firstPoint,int newSize);
extern float GetNthMoment(SIGNAL signal, int n, float *pNthMoment,int flagCausal, int flagCentered);
extern float GetAbsMoment(SIGNAL signal, float f1, float *pMoment,int flagCausal, int flagCentered);
extern float GetCorrelation(SIGNAL signal1,SIGNAL signal2,int flagCausal);
extern void HistoSig(SIGNAL input, SIGNAL output, int n,float xmin, float xmax,float ymin, float ymax, SIGNAL weight,int flagCausal);
extern void LineFitSig(SIGNAL signal,float *pA,float *pSigA,float *pB,float *pSigB,int iMin,int iMax);
extern float GetLpNormSig(SIGNAL signal, float p,int flagCausal);
extern float Urand(void);
extern float Grand(float sigma);
extern void ConvSig(SIGNAL in, SIGNAL filter, SIGNAL out, int borderType,int method);
extern void  DirectConvolution (SIGNAL signal1,SIGNAL signal2, SIGNAL signalOut, int borderType, float x_begin_conv, float x_end_conv);
extern void  FFTConvolution (SIGNAL signal1, SIGNAL signal2, SIGNAL out, int borderType, float x_begin_conv, float x_end_conv);
 
extern void Fft(SIGNAL inReal,SIGNAL inImag,SIGNAL outReal,SIGNAL outImag,int fftSign,char flagShift);

/******************************************/
/*     Functions in SIGNAL_FILE.c         */
/******************************************/

extern void WriteSigRawStream(SIGNAL signal,STREAM s, char binaryCoding);               
extern void WriteSigRawFile(SIGNAL signal,char *filename, char binaryCoding);
extern void WriteSigFile(SIGNAL signal,char *filename, char flagBinary, char *mode,int flagHeader);
extern void WriteSigStream(SIGNAL signal,STREAM s, char flagBinary, char *mode,int flagHeader);

/* Old functions : not to be used */
extern void WriteBinSigStream(SIGNAL signal,STREAM stream,char flagHeader,char * mode);
extern void WriteBinSigFile(SIGNAL signal,char *filename,char flagHeader,char * mode);

extern char ReadInfoSigStream(STREAM s, SIGNAL siginfo, char *header, char *flagBinary, char *binaryCoding, int *nColumns);
extern char ReadInfoSigFile(char *filename, SIGNAL siginfo, char *header, char *binaryMode, char *binaryCoding, int *nColumns);
extern void ReadSigRawStream(SIGNAL signal,STREAM s, int firstIndex,int sizeToRead, char binaryCoding);
extern void ReadSigRawFile(SIGNAL signal,char *filename,  int firstIndex,int sizeToRead, char binaryCoding);
extern void ReadSigFile(SIGNAL signal,char *filename,  int firstIndex,int sizeToRead, int xcol,int ycol);
extern void ReadSigStream(SIGNAL signal,STREAM s, int firstIndex,int sizeToRead, int xcol,int ycol);

/* Old functions : not to be used */
extern void ReadBinSigFile(SIGNAL signal,char *filename,int firstIndex,int sizeToRead);
extern void ReadBinSigStream(SIGNAL signal,STREAM stream,int firstIndex,int sizeToRead);




/* Include the convolution file */
#include "cv.h"

#define BorderPad CV_PADDING
#define BorderPad0 CV_0_PADDING
#define BorderPeriodic CV_PERIODIC
#define BorderMirror CV_MIRROR



#endif
