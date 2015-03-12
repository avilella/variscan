
#ifndef OWTRANS2D_H
#define OWTRANS2D_H


#include "images.h"


extern char *owtrans2Type;
extern TypeStruct tsOWtrans2;


/*
 * The different structures for 2d filters
 */
 
typedef struct ofilter2 {

  int size, firstIndex, center;
  float *coeff;
  
} OFilter2, *OFILTER2;

typedef struct ofilterSet {

  int symmetric;
  OFILTER2 analysisLow, analysisHigh, synthesisLow, synthesisHigh;

} OFilterSet, *OFILTERSET;

typedef struct owavelet2 {

  OFILTER2 analysisLow, analysisHigh;    /* H and G */
  OFILTER2 synthesisLow, synthesisHigh;  /* H~ and G~ */

  int symmetric;  /* TRUE if filter set is symmetric */
  
  int npad;
  
  char name[15];

} OWavelet2, *OWAVELET2;


extern char defaultO2WaveletName[];
extern void InitOWavelet2(void);
extern void OWt2f(char *str);


/*
 * The 2d orthogonal wavelet transform structure
 */
 
typedef struct owtrans2
{
  ValueFields;

  char *name;           /* The name of the wavelet transform */

  IMAGE original;       /* Original image */
  int noct;             /* The number of octaves in the wtrans */
  IMAGE * subimage;     /* There are 3*noct+1 images if noct != 0 and 0 otherwise */
  IMAGE *workimage;     /* Nine images for the user */
    
  OWAVELET2 wavelet;    /* The wavelet used */

  int hsize, vsize;     /* The number of cols and rows of the image which was analyzed */
 
  struct coeffset ** coeff; /* Coeff used for quantization and coding */
 

} OWtrans2, *OWTRANS2;


extern OWTRANS2 NewOWtrans2(void);
extern void ClearOWtrans2(OWTRANS2 ow2trans2);
extern OWTRANS2 CopyOWtrans2(OWTRANS2 in,OWTRANS2 out);
extern void DeleteOWtrans2(OWTRANS2 owtrans2);
extern void  DeleteCoeffOWtrans2 (OWTRANS2 owtrans2);
extern void SetWaveletOWtrans2 (OWTRANS2 wtrans,char * nameWavelet);
extern void SetNOctOWtrans2(OWTRANS2 wtrans,int noct,int ncol,int nrow);
extern OWTRANS2 GetOWtrans2Cur(void);
extern IMAGE GetOWtrans2Image(OWTRANS2 wtrans, int oct, int orient);
 extern void CheckOWtrans2(OWTRANS2 wtrans);



extern int tOWTRANS2,tOWTRANS2_; 
extern char *owtrans2Type;

extern void OWt2d (OWTRANS2 wtrans, int noct);
extern void OWt2r (OWTRANS2 wtrans, IMAGE image);
extern OWAVELET2  NewOWavelet2(char * nameFilterset);





#endif


