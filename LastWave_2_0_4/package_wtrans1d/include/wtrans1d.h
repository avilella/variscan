/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
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




/**************************************************************/
/*                                                            */
/* 	 wtrans1d.h :    The wtrans structure                       */
/*                                                            */
/**************************************************************/

#ifndef WTRANS_H

#define WTRANS_H


/**********************/
/* Some constants ... */
/**********************/

/* The different types of wavelet transform */
#define W_ORTH  1    /* Orthogonal WT */
#define W_DYAD  2    /* Dyadic WT     */
#define W_CONT  3    /* Continuous WT */
#define W_OTHER 4    /* Anything else */


/* The Different types of border effects for (bi)orthogonal decomposition */
#define B_PERIODIC     1
#define B_ANTISYMETRIC 2


/***********************************
 *
 *       wtransform structure       
 *
 ************************************/

#include "signals.h"
#include "filter1d.h"

#define NOCT 20       /* max number of octaves  */
#define NVOICE 20     /* max number of voices  */

typedef struct wtrans{

  /* The fields of the VALUE structure */
  ValueFields;

  char *name;             /* The name of the wtrans */
  
  int type;               /* Type of the wtransform */
  int border;             /* Border effect type for (bi)orthogonal decomposition */

  float x0,dx;            /* The x0 and dx of the original signal */
  int size;               /* The size of the original signal */
  
  SIGNAL A[NOCT][NVOICE];             /* Approximation signals. */
                          /* A[0][0] is the original signal */
  						  /* A[oct][voice] corresponds to scale  2^(oct+voice/nVoice) */
  						  /* oct varies in [0,noct] and voice in [0,nvoice-1] */
  SIGNAL D[NOCT][NVOICE];             /* Difference signals. Same coding as above. */
                          /* The D[0][?] are never used by wavelet decomposition */
  
  int  nOct;		      /* Total # of octaves in decomposition 	*/
  int  nVoice;		      /* Total # of voices in decomposition */

  FILTERGROUP fg;         /* The group of filters */
  
  char *wName;
  
  float aMin;
  float exponent; 
  
  struct extrep *extrep;     	  /* ptr to the extrema representation 	*/

} *WTRANS;
 

/* Wtrans types */
extern char *wtransType;
extern int tWTRANS, tWTRANS_;

/* Create a new wtrans structure and returns it  */
extern WTRANS NewWtrans(void);              

/* Desallocate the whole WTRANS structure */
extern void DeleteWtrans(WTRANS wtrans);

/* Copy a WTRANS into another */
extern WTRANS CopyWtrans(WTRANS in,WTRANS out);
extern void CopyFieldsWtrans(WTRANS in , WTRANS out);

/* Get the current wavelet transform */
extern WTRANS GetWtransCur(void);

/* Compute the Min and the max of a wtrans */
extern void MinMaxWtrans(WTRANS wtrans,float x1, float x2,float *pvMin,float *pvMax,int flagCausal);

/* Threshold on a wtrans */
extern void ThreshWtrans(WTRANS wtrans_in,WTRANS wtrans_out,float threshold,
                         float x_left,float x_right,float alpha,int oct_min,int oct_max);

/* Check a wtrans is not empty */
extern void CheckWtrans(WTRANS wtrans);

/* Managing the filters of a wtrans */
extern void SetFGWtrans(WTRANS wtrans,FILTERGROUP fg);
extern void SetBiorFG(char *filename,WTRANS wtrans);
extern void SetDyadFG(char *filename,WTRANS wtrans);


/* Read/write wtrans */
extern void WriteBinWtransStream (WTRANS wtrans,STREAM s,char flagBinary);
extern void WriteBinWtransFile (WTRANS wtrans,char *filename,char flagBinary);
extern void ReadBinWtransStream (WTRANS wtrans,STREAM s);
extern void ReadBinWtransFile (WTRANS wtrans,char *filename);

/* Compute wtransforms */
extern void DWtd(WTRANS wtrans, int num_oct, int flag_remember);
extern void DWtr(WTRANS wtrans,SIGNAL signal);
extern void OWtd(WTRANS wtrans,int num_oct);
extern void OWtr(WTRANS wtrans, SIGNAL signal);

/* Display a wtrans */
extern void DefineGraphWtrans(void);

#include "wt1d.h"

#endif







