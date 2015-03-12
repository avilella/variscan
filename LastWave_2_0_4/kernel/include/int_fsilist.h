/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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


#ifndef INT_FSILIST_H

#define INT_FSILIST_H

/****************************************************************************/
/*                                                                          */
/*  int_listfsi.h                 */
/*                                                                          */
/****************************************************************************/


/* Include signals.h and images.h if not yet included */
#ifndef SIGNALS_H
#include "signals.h"
#endif
#ifndef IMAGES_H
#include "images.h"
#endif

/*
 *
 * A listfsi structure is only used temporarily by LastWave.
 * It is used in 2 ways
 *
 *   1) To store (temporarily) a list of floats/signals/images (fsi)
 *      for signals and images building facilities
 *      
 *   2) To store (temporarily) a list of signals/floats (on two rows)
 *      used for extraction facilities.
 *
 *
 */

/* 
 * An element of a fsi could be either a float, a signal or an image 
 * OR a row separator.
 */
 
enum {
  FSIFloat,
  FSISignal,
  FSIImage,
  FSIRange,
  FSISeparator
};

/* The row separator */
#define FSISEPARATOR ';'

/* The structure of a fsi */
typedef struct fsi { 
  int size;            /* the number of floats in this fsi */
  float *array; /* The array of floats */
  char type;    /* One of FSIFloat, FSISignal, FSIImage, FSISeparator */
  union {
    SIGNAL s;
    RANGE r;
#ifdef IMAGES_H
    IMAGE i;
#endif    
    float f;
  } val;
} Fsi;


/* Macro to get the kth value of an fsi */
#define FSIArray(fsi,k)  \
  (fsi->array != NULL ? fsi->array[k] : RangeVal(fsi->val.r,k))
 
/* 
 * The list of fsi structure
 */

#define NFSI 60

typedef struct fsiList {

  /* the number of fsi structures in the list */
  int size;   
  
  /* a flag that indicates whether the extraction indices has been specified using a 2 column image */
  char flagImage;
  
  /* 
   * Two useful fields that can be used in 2 different ways depending on 1) or 2).
   *
   * In case of 1) they are used to store the total size of the image that will
   * be built.
   *
   * In case of 2) the list is made of two rows representing a signal each.
   * These two fields then represent the size of each signal.
   */
  int nx,ny;

  /* 
   * in case of 2) these are used to store the number of the indexes of the two signals
   * that are within the range specified by the extractInfo structure
   */
  int nx1,ny1; 
   
  /* Number of row separators encountered in the list : IS IT USEFUL???? */
  int nSep;
  
  /* the contents themselves */
  Fsi fsi[NFSI];
  
  /* the options */
  unsigned long options;
  
  /* The extractInfo */
  struct extractInfo *ei;
  
} FSIList;


/* 
 * Types for the flagType variable that indicates what type should be the result
 */

#define FloatType 1 
#define SignalType 2
#define ImageType 4
#define StringType 8
#define ListvType 16
#define OtherType 32
#define NullType 64
#define RangeType 128
#define AnyType 255



/*
 * The information on a variable content to be able to perform extraction
 */

/* Possible values for the flag field of extractInfo */
#define EIIntIndex 1
#define EIErrorBound 2

typedef struct extractInfo {

  /* 
   * The min and max range for the indexes
   * If min > max then not to be used
   */
  float xmin,xmax,dx; 
  float ymin,ymax,dy;
  SIGNAL xsignal;

  /* the flags  */
  char flags;

  /* Expecting 1 or two signals ? */
  unsigned char nSignals;
  
} ExtractInfo;


/* 
 * The flags to test the options
 */

#define FSIOption1 (1)
#define FSIOption2 (1<<1)
#define FSIOption3 (1<<2)
#define FSIOption4 (1<<3)
#define FSIOption5 (1<<4)
#define FSIOption6 (1<<5)
#define FSIOption7 (1<<6)
#define FSIOption8 (1<<7)
#define FSIOption9 (1<<8)
#define FSIOption10 (1<<9)
#define FSIOption11 (1<<10)
#define FSIOption12 (1<<11)
#define FSIOption13 (1<<12)
#define FSIOption14 (1<<13)
#define FSIOption15 (1<<14)
#define FSIOption16 (1<<15)


/*
 * Helpful macros for looping on FSIList
 */
extern long __borderTempLong__;
#define BPER(i,n) (__borderTempLong__ = (i)%(n), __borderTempLong__ >= 0 ? __borderTempLong__ : __borderTempLong__+(n))
#define BCONST(i,n) ((i) < 0 ? 0 : ((i)>= (n) ? (n)-1 : (i)))
#define BMIR1(i,n) (__borderTempLong__=BPER((i),2*(n)),__borderTempLong__>=(n) ? 2*(n)-1-__borderTempLong__ : __borderTempLong__)
#define BMIR(i,n) (__borderTempLong__=BPER((i),2*(n)-2),__borderTempLong__>=(n) ? 2*(n)-2-__borderTempLong__ : __borderTempLong__)
#define FSI_DECL Fsi *fsi; int _k,_n,_k1,_i; float _f; char _flagBreak=NO
    

#define FSI_FOR_START1(list) { \
  for (_k=0,_n=0;_n<list->size && !_flagBreak;_n++) { \
    fsi = &(list->fsi[_n])
#define FSI_FOR_START2(list) \
    for (_k1=0;_k1<fsi->size && !_flagBreak;_k1++) { \
      _f = FSIArray(fsi,_k1); \
      _i= (int) _f
#define FSI_FOR_START(list) \
  FSI_FOR_START1(list); \
  FSI_FOR_START2(list)

#define FSI_BREAK _flagBreak=YES; continue

#define FSI_FOR_END2 _k++;}
#define FSI_FOR_END1 }}
#define FSI_FOR_END \
  FSI_FOR_END2; \
  FSI_FOR_END1

#define FSI_DECL_COL Fsi *fsic; int _ic,_nc,_ns,_kc1; float _fc
#define FSI_DECL_ROW Fsi *fsir; int _ir,_nr,_kr1; float _fr

#define FSI_FOR_INIT(list) \
  for (_ns=0;_ns<list->size;_ns++) {if ((list->fsi[_ns]).type == FSISeparator) break;}

#define FSI_FOR_START_COL(list) { \
  for (_nc=_ns+1;_nc<list->size;_nc++) { \
    fsic = &(list->fsi[_nc]); \
    for (_kc1=0;_kc1<fsic->size;_kc1++) { \
      _fc = FSIArray(fsic,_kc1); \
      _ic = (int) _fc
  
#define FSI_FOR_END_COL }}}

#define FSI_FOR_START_ROW(list) { \
  for (_nr=0;_nr<_ns;_nr++) { \
    fsir = &(list->fsi[_nr]); \
    for (_kr1=0;_kr1<fsir->size;_kr1++) { \
      _fr = FSIArray(fsir,_kr1); \
      _ir = (int) _fr
  
#define FSI_FOR_END_ROW }}}

#define FSI_FOR_START_IMAGE(list) { \
      fsic = &(fsiList->fsi[_ns+1]); \
      FSI_FOR_START_ROW(fsiList); \
      _ic = (int) FSIArray(fsic,_kr1)
  
#define FSI_FOR_END_IMAGE FSI_FOR_END_ROW; }
  
#define FSI_FIRST(fsiList) ((fsiList)->fsi[0].array[0]) 
#define FSI_SECOND(fsiList) ((fsiList)->fsi[0].size == 2 ? (fsiList)->fsi[0].array[1] : \
                            (fsiList)->fsi[1].array[0])
 
 
/* 
 * Macros to set/get the default parameters for expression evaluation
 */

typedef struct exprDefParam {

  /* The X for the XY operator */
  VALUE vcX;
  
  /* Variables to remember the default signal X values */
  int _theSignalSize;
  double _theSignalDx,_theSignalX0;
  
  /* Variables to remember the default image size */
  int _theImageNRow;
  int _theImageNCol;
  
  /* Variables to remember the default range (vector) values */
  float _vmin,_vmax,_vdx;
  SIGNAL _vxsignal;
  
  /* The current level */
  LEVEL _theLevel;
  
  /* Should we do not care about spaces ? */
  char flagSpace;

  /* Should we accept empty signals/images ? */
  char flagEmptySI;

  /* Accepted type for the elements of a listv */
  unsigned char listvElemType;

  /* Should we perform substitutions */
  char flagSubst;

  /* For finding the compiled version of a word in case there is a [] */
  int nWord;
  int nPos;
  int nScript;
  
} ExprDefParam;


/*
 * Different macros to manipulate it
 */

#define InitDefaults(def,level,flagSubst1,flagFloat1) {(def)->flagEmptySI = YES;(def)->flagSubst = flagSubst1; (def)->flagSpace = YES; \
(def)->_theSignalSize = (def)->_theImageNRow = (def)->_theImageNCol = -1; (def)->vcX = NULL; \
(def)->_theSignalDx = (def)->_theSignalX0 = 0; (def)->_vmin = 0; (def)->_vdx = 1; (def)->_vxsignal = NULL; (def)->_vmax = -1; \
(def)->_theLevel = level; (def)->nWord=-1; (def)->nScript = -1;(def)->nPos = -1;(def)->listvElemType=AnyType;}

#define DefaultLevel(def) (def)->_theLevel  

#define SkipSpace(v) while (*(v) == ' ' || *(v) == '\t' || *(v) == '\n' || *(v) == '\r') (v)++
#define ParamSkipSpace(v) if (defParam->flagSpace) {SkipSpace(v);}


#define SetDefaultX(def,x) { \
  (def)->vcX = x; \
}

#define DefaultX(def) (def)->vcX
 
/* For vectors */

#define DefaultVectVar \
  float __vmin, __vdx, __vmax; SIGNAL __vxsignal  
 
#define SetDefaultVector(def,vmin1,vdx1,vmax1,vxsignal1,flagForce) { \
  if ((def)->_vmin > (def)->_vmax || (flagForce)) { \
    (def)->_vmin = vmin1; \
    (def)->_vdx = vdx1; \
    (def)->_vmax = vmax1; \
    (def)->_vxsignal = vxsignal1; \
  }; \
}

#define SaveDefaultVector(def) { \
  __vmin = (def)->_vmin; \
  __vdx = (def)->_vdx; \
  __vmax = (def)->_vmax; \
  __vxsignal = (def)->_vxsignal; \
}

#define RestoreDefaultVector(def) { \
  (def)->_vmin = __vmin; \
  (def)->_vdx = __vdx; \
  (def)->_vmax = __vmax; \
  (def)->_vxsignal = __vxsignal; \
}

#define DefaultVMin(def) (def)->_vmin  
#define DefaultVDx(def) (def)->_vdx  
#define DefaultVMax(def) (def)->_vmax

#define NoDefaultVector(def) ((def)->_vmin > (def)->_vmax)


/* For Signals */

#define SetDefaultSizeX0Dx(def,size,x0,dx,flagForce)  { \
 if ((def)->_theSignalSize == -1 || (flagForce)) { \
   (def)->_theSignalSize = size; \
   if (dx >= 0) { \
     (def)->_theSignalDx = dx;\
     (def)->_theSignalX0 =x0;\
   } \
   else { \
     (def)->_theSignalDx = -(dx);\
     (def)->_theSignalX0 =((size)-1)*(dx)+(x0);\
   } \
 }; \
}
  
#define SetDefaultSig(def,sig,flagForce)  { \
  SetDefaultSizeX0Dx(def,(sig)->size,(sig)->x0,(sig)->dx,flagForce); \
}

#define NoDefaultSig(def) ((def)->_theSignalSize == -1)
#define ForceNoDefaultSig(def) {{(def)->_theSignalSize=-1;}}
#define DefaultSigSize(def) (def)->_theSignalSize
#define DefaultSigDx(def) (def)->_theSignalDx
#define DefaultSigX0(def) (def)->_theSignalX0


/* For Images */

#define SetDefaultNColNRow(def,ncol,nrow,flagForce) { \
 if ((def)->_theImageNRow == -1 || (flagForce)) { \
   (def)->_theImageNRow = nrow; \
   (def)->_theImageNCol = ncol; \
 }; \
}
  
#define SetDefaultIm(def,im,flagForce) { \
  SetDefaultNColNRow(def,(im)->ncol,(im)->nrow,flagForce); \
}

#define NoDefaultIm(def) ((def)->_theImageNRow == -1)
#define ForceNoDefaultIm(def) {{(def)->_theImageNRow=-1;}}
#define DefaultImNRow(def) (def)->_theImageNRow
#define DefaultImNCol(def) (def)->_theImageNCol


/*
 * Some externs
 */  
extern FSIList *SFIListExpr(char* begin, char** left, ExtractInfo *ei, ExprDefParam *defParam);
extern unsigned char ExpandFSIList(FSIList *list, float *resFlt, VALUE *resVC, unsigned char flagType);
extern void DeleteFSIList(FSIList *list);
extern FSIList *NewSFIList(void);
extern char FSIReadExtractOption(char *begin, char **left, char **options, unsigned long *optionFlag);


  

#endif
