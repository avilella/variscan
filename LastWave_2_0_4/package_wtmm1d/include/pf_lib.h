/*..........................................................................*/
/*                                                                          */
/*      PARTITION FUNCTION 3.1                                              */
/*                                                                          */
/*      Copyright (C) 1998-2002 Benjamin Audit.                             */
/*      email : audit@ebi.ac.uk                                             */
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

#ifndef PARTITION_FUNCTION_H
#define PARTITION_FUNCTION_H

#include <stdio.h>

typedef struct PartitionFunction *PartitionFunction;

enum PFErrCode { PFNo, PFYes, PFWriErr, PFReadErr, PFErrFormat,
                 PFErrAlloc, PFNotValid, PFNotCompatible};

enum PFAccessMode {PFEXTENSIVE, PFINTENSIVE};

#define PFMETHODSIZE 100
#define PFQDIFFMAX 1e-5

/******************************************
 *     Functions in pf_lib.c   
 ******************************************/

/* Returns a new PartitionFunction initialized with non sense fields.
   Returns NULL if it fails. */
extern PartitionFunction PFNew(void);

/* Returns a pointer to a new PartitionFunction, copy of pf 
   or NULL if it fails */
extern PartitionFunction PFCopy(const PartitionFunction pf);

extern void PFDelete(PartitionFunction pf);

/* It initialiszes pf for the computation of a new partition function.
   The arguments must be meaningful.
   Returns PFYes if it succeeds. */
extern int PFInit(PartitionFunction pf,char *method,double aMin,
		  int octaveNumber,int voiceNumber,int signalSize,
		  int dimension,int qNumber,double *qArray);

/* Returns PFYes if pf is valid (meaningful content), PFNotValid otherwise.
   It should always be the case after PFInit() has been used once. */
extern int PFIsValid(PartitionFunction pf);

/* Computation of the partition functions at one scale.
   Scales go from 0 to octaveNumber*voiceNumber-1.
   Float version: the wavelet coefficients are given in an array of float.
   One is supposed to compute scales in increasing order. */
extern int PFComputeOneScaleF(PartitionFunction pf,int scale,
			      const float *t,int tSize);

/* The ordering of data in PFComputeOneScale() is done using qsort().
   One can change it using PFChangeQsort() especially if no sorting is wanted 
   The new sorting function must work the same as qsort() of <stdlib.h> .
   You can come back to the qsort() by calling PFChangeQsort(NULL). */
extern void PFChangeQsort(void (*NewQsort)(void *,int ,int ,int (*)(const void *, const void *)));
extern void PFQNoSort(void *base,int nel,int size,
		      int (*compar)(const void *, const void *));
/* Writing a PartitionFunction on a stream.
   pf is supposed to have been obtained with PFNew() and to have been 
   initialized with PFInit() (thus containing meaningful data).
   fp should be in writing mode */
extern int PFWriteAscii(FILE *fp,PartitionFunction pf);
extern int PFWriteBin(FILE *fp,PartitionFunction pf);

/* Reading a PartitionFunction from a stream.
   pf is supposed to have been obtained with PFNew() 
   (it may have already been used).
   fp should be in reading mode */
extern int PFRead(FILE *fp,PartitionFunction pf);

/* The result will be in pf1. */
extern int PFStandardAddition(PartitionFunction pf1,
			      const PartitionFunction pf2);

/* The result will be in pf1. */
extern int PFNonStandardAddition(PartitionFunction pf1,
				 const PartitionFunction pf2);

/* Set all the arrays to 0 */
extern int PFReset(PartitionFunction pf);

/*************************************************
 *
 * functions to modify one field of a PartitionFunction
 * (the field must be meaningful)
 *
 *************************************************/

/* method shouldn't be longer than PFMETHODSIZE */
int PFSetMethod(PartitionFunction pf,const char *method);

int PFSetAMin(PartitionFunction pf,double aMin);
/* The user is not allowed to modify octaveNumber */
/* The user is not allowed to modify voiceNumber */
/* The user is not allowed to modify indexMax */
int PFSetSignalSize(PartitionFunction pf,int signalSize);
/* The user is not allowed to modify signalNumber */
int PFSetDimension(PartitionFunction pf,int dimension);

/* qArray must contain qNumber double.
   Using this function erase the previous qList 
   and the results associated to it. */
int PFSetQList(PartitionFunction pf,int qNumber,double *qArray);

/*************************************************
 *
 * functions to read one field of a PartitionFunction
 *
 *************************************************/

/* method must be able to contain at least PFMETHODSIZE+1 char */
extern void PFGetMethod(const PartitionFunction pf,char *method);

extern double PFGetAMin(const PartitionFunction pf);
extern int PFGetOctaveNumber(const PartitionFunction pf);
extern int PFGetVoiceNumber(const PartitionFunction pf);
extern int PFGetIndexMax(const PartitionFunction pf);
extern int PFGetSignalSize(const PartitionFunction pf);
extern int PFGetSignalNumber(const PartitionFunction pf);
extern int PFGetDimension(const PartitionFunction pf);
extern int PFGetQNumber(const PartitionFunction pf);

/* One is supposed to have check qNumber before using these functions 
   so that enough space is available in qArray */
extern void PFGetQListDouble(const PartitionFunction pf,double *qArray);
extern void PFGetQListFloat(const PartitionFunction pf,float *qArray);

extern double PFGetQDouble(const PartitionFunction pf,int indexQ);
extern float PFGetQFloat(const PartitionFunction pf,int indexQ);

/*************************************************
 *
 * functions to access computed partition functions
 *
 *************************************************/

/* It returns the size of the sTq sTqLogT... arrays 
   Very useful before calling PFAccesSig TQ | HQ | DQ.
   It returns 0 the list of q's is empty or -1 if pf is not valid */ 
extern int PFAccessSize(const PartitionFunction pf);

/* returns the index of a q in the qList of pf 
   returns -1 on failure */
extern int PFAccessIndQ(const PartitionFunction pf,double q);

/* Sets the float array to T(q), var[T(q)], H(q), var[H(q)], D(q) or var[D(q)].
   Returns PFYes on success.
   indexQ is the index of q in the qList as returned by PFAccessIndQ.
   Mode is either PFEXTENSIVE or PFINTENSIVE.
   tq,hq,dq must be able to contain the result 
   (the size returned by PFAccessSize) 
   */
extern int PFAccessTQFloat(const PartitionFunction pf,int indexQ,int mode,
			   float *tq);
extern int PFAccessVarTQFloat(const PartitionFunction pf,int indexQ,
			      float *vartq);
extern int PFAccessHQFloat(const PartitionFunction pf,int indexQ,int mode,
			   float *hq);
extern int PFAccessVarHQFloat(const PartitionFunction pf,int indexQ,
			      float *varhq);
extern int PFAccessDQFloat(const PartitionFunction pf,int indexQ,int mode,
			   float *dq);
extern int PFAccessVarDQFloat(const PartitionFunction pf,int indexQ,
			      float *vardq);


#endif /* PARTITION_FUNCTION_H */

