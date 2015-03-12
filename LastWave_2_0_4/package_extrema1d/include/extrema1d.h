/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'extrema1d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1999-2002 Emmanuel Bacry                              */
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





#ifndef EXT_H

#define EXT_H


/**************************************/
/*    EXT : Extremum structure        */
/**************************************/


typedef struct ext {

  /* The fields of the VALUE structure */
  ValueFields;
  
  float abscissa;               /* Abscissa of the extremum */
  int index;
  int scale;                    /* scale at which the extremum is detected */
  float ordinate;               /* amplitude of the extremum */
  struct ext *previous, *next;  /* previous and next extrema at a 
				   fixed scale when varying the abscissa */
  struct ext *coarser, *finer;  /* coarser and finer extrema when varying
				   the scale */

  int type;
  
  struct extlis *extlis;
} *EXT;

extern char *extType;

extern int tEXT, tEXT_;
extern TypeStruct tsExt;


/*****************************************************/
/* Allocation Functions on EXTREMA  (EXT_ALLOC.c)    */
/*****************************************************/

extern EXT NewExt(void);
extern void DeleteExt(EXT ext);
extern EXT CopyExt(EXT in,EXT out);
extern void RemoveDeleteExt(EXT);
extern void RemoveDeleteChain(EXT);


/**************************************/
/* EXTLIS : Extremum list structure  */
/* (extrema of the wavelet transform  */
/* at a fixed scale)                  */
/**************************************/


typedef struct extlis {
  int size;    /* Number of extrema in the list */
  EXT first;   /* First extremum in the list */
  EXT end;     /* Last extremum in the list */
  struct extrep *extrep;
} *EXTLIS;


/*****************************************************/
/* Allocation Functions on EXTLIS  (EXT_ALLOC.c)     */
/*****************************************************/

extern EXTLIS NewExtlis(void); 
extern void ClearExtlis(EXTLIS extlis);
extern void DeleteExtlis(EXTLIS extlis);



/**************************************/
/* EXTREP: Extrema representation     */
/* structure (all the extrema of a    */
/* wavelet transform)                 */
/**************************************/

#include "wtrans1d.h"


typedef struct extrep {

  /* The fields of the VALUE structure */
  ValueFields;
  
  char *name;
  int size;                  /* size of the initial signal */
  float dx;                  /* scale of the initial signal */
  float x0;                  /* shift of the initial signal */
  int nOct;                  /* number of octave */
  int nVoice;                /* number of voice */
  SIGNAL coarse;             /* coarse signal */
  FILTERGROUP fg;            /* The group of filters */
  char *wName;
  
  EXTLIS D[NOCT][NVOICE];  /* extrema list for each octave and each voice */
  							
  float aMin;                /* wavelet transform finest scale       */
  struct wtrans *wtrans;  
  float exponent;
} *EXTREP;



extern char *extrepType;
extern TypeStruct tsExtrep;
extern int tEXTREP, tEXTREP_;

/*****************************************************/
/* Allocation Functions on EXTREP  (EXT_ALLOC.c)     */
/*****************************************************/

extern EXTREP NewExtrep(void); 
extern void ClearExtrep(EXTREP extrep);
extern void DeleteExtrep(EXTREP extrep);
extern void CheckExtrep(EXTREP extrep);

extern EXTREP GetExtrepCur(void);

extern void CopyExtrep(EXTREP extrep1,EXTREP extrep2,int flagCut,float xMin,float xMax);

extern void Chain(EXTREP extrep,float delta);
extern int ChainDelete(EXTREP extrep);
extern void ChainMax(EXTREP extrep,double expo);

extern void WriteExtrepStream (EXTREP extrep,STREAM s,int flagHeader);
extern void WriteExtrep (EXTREP extrep,char filename[],int flagHeader);
extern void ReadExtrepStream (EXTREP extrep,STREAM s);
extern void ReadExtrep(EXTREP extrep,char *filename);
extern int InsertExt(EXTREP extrep,EXT ext);

extern void DefineGraphExtrep(void);

extern void InitExtrep(WTRANS wtrans,EXTREP extrep);
extern int ComputeExtOctVoice(WTRANS wtrans,EXTREP extrep, int flagCausal,int flagInterpol,float epsilon,int o,int v,float *pThreshold);
extern int ComputeExtrep(WTRANS wtrans,EXTREP extrep, int flagCausal,int flagInterpol,float epsilon);




#endif

