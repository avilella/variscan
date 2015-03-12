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



#include "pf_lib.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#ifndef NOT_USED_WITH_LW

extern void *Malloc(size_t size);
extern void *Calloc(int n, size_t size);
extern void Free(void * ptr);

#define malloc Malloc
#define calloc Calloc
#define free Free

#endif /* NOT_USED_WITH_LW */

typedef struct PartitionFunctionCell
{
  double q;
  int size;
  double *sTq;
  double *sTqLogT;
  double *logSTq;
  double *sTqLogT_sTq;
  double *log2STq;
  double *sTqLogT_sTq2;
  double *logSTqSTqLogT_sTq;
} *PartitionFunctionCell;

struct PartitionFunction
{
  /* + 1 for the terminating null byte */
  char method[PFMETHODSIZE+1];
  double aMin;
  int octaveNumber;
  int voiceNumber;
  int indexMax;
  int signalSize;
  int signalNumber;
  int dimension;
  int qNumber;
  PartitionFunctionCell *cellArray;
};

/*************************************************
 *
 * Stupid functions to compare double and used by qsort
 *
 *************************************************/
static int PFCompDouble(const double *d1,const double *d2)
{

  if(*d1<*d2)
    return -1;
  else if(*d1 == *d2)
    return 0;
  else
    return +1;
}

static int PFCellCompQ(const PartitionFunctionCell *pfc1,const PartitionFunctionCell *pfc2)
{
  return PFCompDouble(&(*pfc1)->q,&(*pfc2)->q);
}

/*************************************************
 *
 * DesAllocation of PartitionFunction(Cell)(Array)
 *
 *************************************************/

static void PFCellDelete(PartitionFunctionCell pfc)
{
  if( pfc == NULL ) return;

  /* We only use one memory allocation for the 7 arrays */
  free(pfc->sTq);
  free(pfc);
}

static void PFCellArrayDelete(PartitionFunctionCell *cellArray,int qNumber)
{
 int nq;

 if( cellArray == NULL ) return;

 assert(qNumber > 0);

 for(nq = 0; nq < qNumber; nq++)
   PFCellDelete(cellArray[nq]);
 
 free(cellArray);
}

void PFDelete(PartitionFunction pf)
{
  if( pf == NULL ) return;

  PFCellArrayDelete(pf->cellArray,pf->qNumber);

  free(pf);
}

/*************************************************
 *
 * Allocation of PartitionFunction(Cell)(Array)
 *
 *************************************************/

/* Returns an initialized cell (using q and size) in which the space 
   for the double *s is allocated and initialised to 0 
   Returns NULL if it fails or if size <= 0 */
static PartitionFunctionCell PFCellNew(double q,int size)
{
  PartitionFunctionCell pfc;

  if( size <= 0 ) return NULL;

  pfc = (PartitionFunctionCell) malloc(sizeof(struct PartitionFunctionCell));
  if( pfc == NULL ) return NULL;
  
  /* We use one memory allocation for the 7 arrays */
  pfc->sTq = (double*) calloc(7*size,sizeof(double));
  if( pfc->sTq == NULL ) 
    {
      free(pfc);
      return NULL;
    }

  pfc->sTqLogT = pfc->sTq + size;
  pfc->logSTq = pfc->sTq + 2*size;
  pfc->sTqLogT_sTq = pfc->sTq + 3*size;
  pfc->log2STq = pfc->sTq + 4*size;
  pfc->sTqLogT_sTq2 = pfc->sTq + 5*size;
  pfc->logSTqSTqLogT_sTq = pfc->sTq + 6*size;

  pfc->q = q;
  pfc->size = size;

  return pfc;
}
/* Return the size of a cell */
static int PFCellSizeOf(int size)
{
  return sizeof(struct PartitionFunctionCell) + 7*size*sizeof(double);
}

/* Returns a pointer to a new PartitionFunctionCell, copy of pfc 
   or NULL if it fails */
static PartitionFunctionCell PFCellCopy(const PartitionFunctionCell pfc)
{
  PartitionFunctionCell pfcNew;
  
  if(pfc == NULL) return NULL;
  
  pfcNew = PFCellNew(pfc->q,pfc->size);
  if(pfcNew == NULL) return NULL;
  
  /* This is correct because we use only one memory allocation for the 7 arrays
   */
  memcpy(pfcNew->sTq,pfc->sTq,7*pfc->size*sizeof(double));

  return pfcNew;
}

/* It returns a PartitionFunctionCell * pointing to qNumber initialized 
   cells (using size and qArray) in which the space for the double *s 
   is allocated and initialised to 0 */
static PartitionFunctionCell *PFCellArrayNew(int size,int qNumber,
					     double *qArray)
{
  int nq;
  PartitionFunctionCell *cellArray;

  assert( size > 0  && qNumber > 0 );
  
  cellArray = (PartitionFunctionCell *) 
    malloc(qNumber*sizeof(PartitionFunctionCell));
  if( cellArray == NULL ) return NULL;
  
  for(nq = 0; nq < qNumber; nq++)
    {
      cellArray[nq] = PFCellNew(qArray[nq],size);
      
      /* If we can't allocate one cell, we delete the one already allocated */
      /* and we return NULL */
      if( cellArray[nq] == NULL )
	{
	  for(nq--; nq >= 0; nq--)
	    PFCellDelete(cellArray[nq]);
	  free(cellArray);
	  return NULL;
	}
    }
  return cellArray;
}


static PartitionFunctionCell *PFCellArrayConcat(int qNumber1,const PartitionFunctionCell *cellArray1,int qNumber2,const PartitionFunctionCell *cellArray2)
{
  int nq;
  PartitionFunctionCell *cellArrayNew;
  
  cellArrayNew = (PartitionFunctionCell *) 
    malloc((qNumber1+qNumber2)*sizeof(PartitionFunctionCell));
  if(cellArrayNew == NULL) return NULL;

  for(nq=0;nq < qNumber1+qNumber2;nq++)
    {
      cellArrayNew[nq] = PFCellCopy( (nq < qNumber1) ? cellArray1[nq] : cellArray2[nq - qNumber1]);
      if(cellArrayNew[nq] == NULL)
	for(nq--;nq >= 0;nq--)
	  {
	    PFCellDelete(cellArrayNew[nq]);
	    free(cellArrayNew);
	    return NULL;
	  }
    }
  /* Now we have to sort */
  qsort((void *)cellArrayNew,qNumber1+qNumber2,sizeof(PartitionFunctionCell),
	(int (*) (const void *, const void *) ) &PFCellCompQ);

  return cellArrayNew;
}

/* Returns a pointer to a new PartitionFunctionCell *, copy of cellArray 
   or NULL if it fails */
static PartitionFunctionCell *PFCellArrayCopy(const PartitionFunctionCell *cellArray,int qNumber)
{
  int nq;
  PartitionFunctionCell *cellArrayNew;

  if( (cellArray == NULL) || (qNumber <= 0) ) return NULL;

  cellArrayNew = (PartitionFunctionCell *) 
    malloc(qNumber*sizeof(PartitionFunctionCell));
  if( cellArray == NULL ) 
    return NULL;

  for(nq=0;nq < qNumber;nq++)
    {
      cellArrayNew[nq] = PFCellCopy(cellArray[nq]);
      if(cellArrayNew[nq] == NULL)
	{
	  for(nq--;nq >= 0;nq--)
	    PFCellDelete(cellArrayNew[nq]);
	  free(cellArrayNew);
	  return NULL;
	}
    }
  
  return cellArrayNew;
}
/* Returns a new PartitionFunction initialized with non sense fields.
   Returns NULL if it fails. */
PartitionFunction PFNew(void)
{
  PartitionFunction pf;
  
  pf = (PartitionFunction) malloc(sizeof(struct PartitionFunction));
  if( pf == NULL ) return NULL;
  
  pf->method[0] = '\0';
  pf->aMin = -1.0;
  pf->octaveNumber = -1;
  pf->voiceNumber = -1;
  pf->indexMax = -2;
  pf->signalSize = -1;
  pf->signalNumber = -1;
  pf->dimension = -1;
  pf->qNumber = 0;
  pf->cellArray = NULL;

  return pf;
}

/* Returns a pointer to a new PartitionFunction, copy of pf 
   or NULL if it fails */
PartitionFunction PFCopy(const PartitionFunction pf)
{
  PartitionFunction pfNew;
  PartitionFunctionCell *cellArray;

  if(pf == NULL) return NULL;
  if( PFIsValid(pf) != PFYes )
    return NULL;

  if(pf->qNumber != 0)
    {
      cellArray = PFCellArrayCopy(pf->cellArray,pf->qNumber);
      if(cellArray == NULL)
	return NULL;
    }
  else cellArray = NULL;

  pfNew = PFNew();
  if(pfNew == NULL)
    {
      PFCellArrayDelete(cellArray,pf->qNumber);
      return NULL;
    }

  strcpy(pfNew->method,pf->method);
  pfNew->aMin = pf->aMin;
  pfNew->octaveNumber = pf->octaveNumber;
  pfNew->voiceNumber = pf->voiceNumber;
  pfNew->indexMax = pf->indexMax;
  pfNew->signalSize = pf->signalSize;
  pfNew->signalNumber = pf->signalNumber;
  pfNew->dimension = pf->dimension;
  pfNew->qNumber = pf->qNumber;
  pfNew->cellArray = cellArray;

  return pfNew;
}

/* Set all the arrays to 0
 */
static void PFCReset(PartitionFunctionCell pfc)
{
  int i;
  
  for(i=0;i<pfc->size;i++)
    {
      
      pfc->sTq[i] = 0.;
      pfc->sTqLogT[i] = 0.;
      pfc->logSTq[i] = 0.;
      pfc->sTqLogT_sTq[i] = 0.;
      pfc->log2STq[i] = 0.;
      pfc->sTqLogT_sTq2[i] = 0.;
      pfc->logSTqSTqLogT_sTq[i] = 0.;
    }
}

int PFReset(PartitionFunction pf)
{
  int nq;

  if(pf == NULL) return PFNo;
  if( PFIsValid(pf) != PFYes )
    return PFNo;
  
  for(nq=0;nq < pf->qNumber;nq++)
    PFCReset(pf->cellArray[nq]);

  pf->indexMax = -1;
  pf->signalNumber = 1;

  return PFYes;
}

/*************************************************
 *
 * functions to modify one field of a PartitionFunction
 *
 *************************************************/
int PFSetMethod(PartitionFunction pf,const char *method)
{
  if( pf == NULL ) return PFNo;
  if( method == NULL ) return PFNo;
  if( strlen(method) > PFMETHODSIZE)
    return PFNotValid;

  strcpy(pf->method,method);

  return PFYes;
}

int PFSetAMin(PartitionFunction pf,double aMin)
{
  if( pf == NULL ) return PFNo;
  if( aMin <= 0 ) return PFNotValid;

  pf->aMin = aMin;
  
  return PFYes;
}

static int PFSetOctaveNumber(PartitionFunction pf,int octaveNumber)
{
  if( pf == NULL ) return PFNo;
  if( octaveNumber <= 0) return PFNotValid;

  pf->octaveNumber = octaveNumber;
  
  return PFYes;
}

static int PFSetVoiceNumber(PartitionFunction pf,int voiceNumber)
{
  if( pf == NULL ) return PFNo;
  if( voiceNumber <= 0) return PFNotValid;

  pf->voiceNumber = voiceNumber;
  
  return PFYes;
}

static int PFSetIndexMax(PartitionFunction pf,int indexMax)
{
  if( pf == NULL ) return PFNo;
  if( indexMax < 0) return PFNotValid;

  pf->indexMax = indexMax;
  
  return PFYes;
}
 
int PFSetSignalSize(PartitionFunction pf,int signalSize)
{
  if( pf == NULL ) return PFNo;
  if( signalSize <= 0) return PFNotValid;

  pf->signalSize = signalSize;
  
  return PFYes;
}

static int PFSetSignalNumber(PartitionFunction pf,int signalNumber)
{
  if( pf == NULL ) return PFNo;
  if( signalNumber <= 0) return PFNotValid;

  pf->signalNumber = signalNumber;
  
  return PFYes;
}

int PFSetDimension(PartitionFunction pf,int dimension)
{
  if( pf == NULL ) return PFNo;
  if( dimension <= 0) return PFNotValid;

  pf->dimension = dimension;
  
  return PFYes;
}

int PFSetQList(PartitionFunction pf,int qNumber,double *qArray)
{
  int nq;
  PartitionFunctionCell *cellArray;

  if( pf == NULL ) return PFNo;
  if( (qNumber < 0) || (pf->octaveNumber <= 0) || (pf->voiceNumber <= 0)
      || ((qNumber == 0)&&(qArray != NULL)) )
    return PFNotValid;

  if( qNumber == 0 )
    {
      if(pf->cellArray != NULL)
	{
	  PFCellArrayDelete(pf->cellArray,pf->qNumber);
	  pf->cellArray = NULL;
	}
      pf->qNumber = 0;
    }
  else
    {
      /* We want the qs to be sorted in increasing order */
      qsort((void *)qArray,qNumber,sizeof(double),
	    (int (*) (const void *, const void *) ) &PFCompDouble);
 
      /* each value can appear only once */
      for(nq=0;nq < qNumber-1;nq++)
	if(qArray[nq] >= qArray[nq+1])
	  return PFNotValid;
  
      cellArray = PFCellArrayNew(pf->octaveNumber*pf->voiceNumber,qNumber,
				 qArray);
      if( cellArray == NULL) 
	return PFErrAlloc;
  
      if(pf->cellArray != NULL)
	PFCellArrayDelete(pf->cellArray,pf->qNumber);
      pf->cellArray = cellArray;

      pf->qNumber = qNumber;
    }
  
  pf->indexMax = -1;

  return PFYes;
}
/*************************************************
 *
 * functions to read one field of a PartitionFunction
 *
 *************************************************/
/* method must be able to contain at least PFMETHODSIZE+1 char */
void PFGetMethod(const PartitionFunction pf,char *method)
{
  strcpy(method,pf->method);
}

double PFGetAMin(const PartitionFunction pf)
{
  return pf->aMin;
}

int PFGetOctaveNumber(const PartitionFunction pf)
{
  return pf->octaveNumber;
}
int PFGetVoiceNumber(const PartitionFunction pf)
{
  return pf->voiceNumber;
}
int PFGetIndexMax(const PartitionFunction pf)
{
  return pf->indexMax;
}
int PFGetSignalSize(const PartitionFunction pf)
{
  return pf->signalSize;
}
int PFGetSignalNumber(const PartitionFunction pf)
{
  return pf->signalNumber;
}
int PFGetDimension(const PartitionFunction pf)
{
  return pf->dimension;
}
int PFGetQNumber(const PartitionFunction pf)
{
  return pf->qNumber;
}
/* One is supposed to have check qNumber before using thess functions 
   so that enough space is available in qArray */
void PFGetQListDouble(const PartitionFunction pf,double *qArray)
{
  int nq;
  
  for(nq=0;nq < pf->qNumber;nq++)
    qArray[nq] = pf->cellArray[nq]->q;
}
void PFGetQListFloat(const PartitionFunction pf,float *qArray)
{
  int nq;
  
  for(nq=0;nq < pf->qNumber;nq++)
    qArray[nq] = (float) pf->cellArray[nq]->q;
}
/* One is supposed to have check qNumber before using thess functions 
   so that indexQ is in [0,qNumber[ */
double PFGetQDouble(const PartitionFunction pf,int indexQ)
{
  if(indexQ < 0)
    return(-DBL_MAX);
  
  if(indexQ >= pf->qNumber)
    return(DBL_MAX);

  return( pf->cellArray[indexQ]->q);
}
float PFGetQFloat(const PartitionFunction pf,int indexQ)
{
  if(indexQ < 0)
    return(-FLT_MAX);
  
  if(indexQ >= pf->qNumber)
    return(FLT_MAX);

  return( (float) pf->cellArray[indexQ]->q);
}

/*************************************************
 *
 * functions to access computed partition functions
 *
 *************************************************/
/* It returns the size of the sTq sTqLogT... arrays 
   Very useful before calling PFAccesSig TQ | HQ | DQ.
   It returns 0 the list of q's is empty or -1 if pf is not valid */ 
int PFAccessSize(const PartitionFunction pf)
{
  if(PFIsValid(pf) != PFYes)
    return -1;
  if(pf->qNumber == 0)
    return 0;
  
  return pf->octaveNumber*pf->voiceNumber;
}
/* return the index of a q in the qList of pf 
   reurn -1 on failure */
int PFAccessIndQ(const PartitionFunction pf,double q)
{
  int nq;

  if( PFIsValid(pf) != PFYes )
    return -1;
  if(pf->qNumber == 0)
    return -1;
  
  for(nq=0;nq < pf->qNumber;nq++)
    if(fabs(pf->cellArray[nq]->q - q) <= PFQDIFFMAX)
      return nq;
  
  return -1;
}
/* Sets the float array to Tau(q).
   Returns PFYes on success.
   Mode is either PFEXTENSIVE or PFINTENSIVE.
   tq must be able to contain the result (the size returned by PFAccessSize)
   */
int PFAccessTQFloat(const PartitionFunction pf,int indexQ,int mode,float *tq)
{
  int i;
  int size;
  PartitionFunctionCell pfc;

  if(pf == NULL) return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber == 0)
      || (pf->qNumber <= indexQ) || (indexQ < 0) ) 
    return PFNotValid;
  
  pfc = pf->cellArray[indexQ];
  size = pfc->size;
  if( mode == PFEXTENSIVE )
    {
      for(i=0;i < size;i++)
	{
	  if(pfc->sTq[i] == 0)
	    tq[i] = 0.0;
	  else
	    tq[i] = (float) log(pfc->sTq[i]);
	}
    }
  else if( mode == PFINTENSIVE )
    {
      for(i=0;i <= pf->indexMax;i++)
	tq[i] = (float) ( pfc->logSTq[i]/(double) pf->signalNumber );
      for(;i < size;i++)
	tq[i] = 0.0;
    }
  else return PFNotValid;

  return PFYes;
}
/* Sets the float array to var[Tau(q)].
   Returns PFYes on success.
   vartq must be able to contain the result (the size returned by PFAccessSize)
   */
int PFAccessVarTQFloat(const PartitionFunction pf,int indexQ,float *vartq)
{
  int i;
  int size;
  double signalNumber;
  PartitionFunctionCell pfc;

  if(pf == NULL) return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber == 0) 
      || (pf->qNumber <= indexQ) || (indexQ < 0) || (pf->signalNumber < 2) ) 
    return PFNotValid;
  
  pfc = pf->cellArray[indexQ];
  size = pfc->size;
  signalNumber = (double) pf->signalNumber;

  for(i=0;i < size;i++)
    {
      for(i=0;i <= pf->indexMax;i++)
	vartq[i] = (float) ( (pfc->log2STq[i] - pfc->logSTq[i]*pfc->logSTq[i]/signalNumber)/(signalNumber - 1.0) );
      for(;i < size;i++)
	vartq[i] = 0.0;
    }
  
  return PFYes;
}
/* Sets the float array to H(q).
   Returns PFYes on success.
   Mode is either PFEXTENSIVE or PFINTENSIVE.
   hq must be able to contain the result (the size returned by PFAccessSize)
   */
int PFAccessHQFloat(const PartitionFunction pf,int indexQ,int mode,float *hq)
{
  int i;
  int size;
  PartitionFunctionCell pfc;

  if(pf == NULL) return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber == 0)
      || (pf->qNumber <= indexQ) || (indexQ < 0) ) 
    return PFNotValid;
  
  pfc = pf->cellArray[indexQ];
  size = pfc->size;
  if( mode == PFEXTENSIVE )
    {
      for(i=0;i < size;i++)
	{
	  if(pfc->sTq[i] == 0)
	    hq[i] = 0.0;
	  else
	    hq[i] = (float) ( pfc->sTqLogT[i] / pfc->sTq[i]);
	}
    }
  else if( mode == PFINTENSIVE )
    {
      for(i=0;i <= pf->indexMax;i++)
	hq[i] = (float) (pfc->sTqLogT_sTq[i] / (double) pf->signalNumber);
      for(;i < size;i++)
	hq[i] = 0.0;
    }
  else return PFNotValid;

  return PFYes;
}
/* Sets the float array to var[H(q)].
   Returns PFYes on success.
   varhq must be able to contain the result (the size returned by PFAccessSize)
   */
int PFAccessVarHQFloat(const PartitionFunction pf,int indexQ,float *varhq)
{
  int i;
  int size;
  double signalNumber;
  PartitionFunctionCell pfc;

  if(pf == NULL) return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber == 0) 
      || (pf->qNumber <= indexQ) || (indexQ < 0) || (pf->signalNumber < 2) ) 
    return PFNotValid;
  
  pfc = pf->cellArray[indexQ];
  size = pfc->size;
  signalNumber = (double) pf->signalNumber;

  for(i=0;i < size;i++)
    {
      for(i=0;i <= pf->indexMax;i++)
	varhq[i] = (float) ( (pfc->sTqLogT_sTq2[i] - pfc->sTqLogT_sTq[i]*pfc->sTqLogT_sTq[i]/signalNumber)/(signalNumber - 1.0) );
      for(;i < size;i++)
	varhq[i] = 0.0;
    }
  
  return PFYes;
}

/* Sets the float array to D(q).
   Returns PFYes on success.
   Mode is either PFEXTENSIVE or PFINTENSIVE.
   dq must be able to contain the result (the size returned by PFAccessSize) */
int PFAccessDQFloat(const PartitionFunction pf,int indexQ,int mode,float *dq)
{
  int i;
  int size;
  PartitionFunctionCell pfc;

  if(pf == NULL) return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber == 0)
      || (pf->qNumber <= indexQ) || (indexQ < 0) ) 
    return PFNotValid;
  
  pfc = pf->cellArray[indexQ];
  size = pfc->size;
  if( mode == PFEXTENSIVE )
    {
      for(i=0;i < size;i++)
	{
	  if(pfc->sTq[i] == 0)
	    dq[i] = 0.0;
	  else
	    dq[i] = (float) ( pfc->q*pfc->sTqLogT[i]/pfc->sTq[i] - log(pfc->sTq[i]) );
	}
    }
  else if( mode == PFINTENSIVE )
    {
      for(i=0;i <= pf->indexMax;i++)
	dq[i] = (float) ( (pfc->q*pfc->sTqLogT_sTq[i] - pfc->logSTq[i])/(double) pf->signalNumber );
      for(;i < size;i++)
	dq[i] = 0.0;
    }
  else return PFNotValid;

  return PFYes;
}

/* Sets the float array to var[D(q)].
   Returns PFYes on success.
   vardq must be able to contain the result (the size returned by PFAccessSize)
   */
int PFAccessVarDQFloat(const PartitionFunction pf,int indexQ,float *vardq)
{
  int i;
  int size;
  double signalNumber;
  double q;
  PartitionFunctionCell pfc;

  if(pf == NULL) return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber == 0) 
      || (pf->qNumber <= indexQ) || (indexQ < 0) || (pf->signalNumber < 2) ) 
    return PFNotValid;
  
  pfc = pf->cellArray[indexQ];
  size = pfc->size;
  signalNumber = (double) pf->signalNumber;
  q = pfc->q;

  for(i=0;i < size;i++)
    {
      for(i=0;i <= pf->indexMax;i++)
	vardq[i] = (float) ( (q*q*pfc->sTqLogT_sTq2[i] - 2*q*pfc->logSTqSTqLogT_sTq[i] + pfc->log2STq[i] - (q*pfc->sTqLogT_sTq[i] - pfc->logSTq[i])/signalNumber)/(signalNumber - 1.0) );
      for(;i < size;i++)
	vardq[i] = 0.0;
    }
  
  return PFYes;
}

/*************************************************
 *
 * Initialisation of PartitionFunction
 *
 *************************************************/

/* It initialiszes pf for the computation of a new partition function.
   The arguments must be meaningful.
   Returns PFYes if it succeeds. */
int PFInit(PartitionFunction pf,char *method,double aMin,
	   int octaveNumber,int voiceNumber,int signalSize,int dimension,
	   int qNumber,double *qArray)
{

  /* Some verifications */
  if( (pf == NULL) || (method == NULL) )
    return PFNo;

  if(PFSetMethod(pf,method) != PFYes)
    return PFNotValid;

  if(PFSetAMin(pf,aMin ) != PFYes)
    return PFNotValid;

  if(PFSetOctaveNumber(pf,octaveNumber) != PFYes)
    return PFNotValid;

  if(PFSetVoiceNumber(pf,voiceNumber) != PFYes)
    return PFNotValid;

  pf->indexMax = -1;

  if(PFSetSignalSize(pf,signalSize) != PFYes)
    return PFNotValid;

  pf->signalNumber = 1;

  if(PFSetDimension(pf,dimension) != PFYes)
    return PFNotValid;

  if(PFSetQList(pf,qNumber,qArray) != PFYes)
    return PFNotValid;


  return PFYes;
}

/* Returns PFYes if pf is valid (meaningful content).
   But does not check the cellArray */
static int _PFIsValid(PartitionFunction pf)
{
  if(pf == NULL) return PFNo;
  
  if( (strlen(pf->method) > PFMETHODSIZE) || (pf->aMin <= 0)
      || (pf->octaveNumber <= 0) || (pf->voiceNumber <= 0 )
      || (pf->indexMax < -1) 
      || (pf->indexMax >= pf->voiceNumber*pf->octaveNumber) 
      || (pf->signalSize <= 0) || (pf->signalNumber <= 0) 
      || (pf->dimension <= 0) || (pf->qNumber < 0) )
    return PFNotValid;

  return PFYes;
}

static int _PFCellArrayIsValid(PartitionFunctionCell *cellArray,int qNumber,
			       int size)
{
  int nq;

  if( cellArray == NULL ) return PFNotValid;

  for(nq=0;nq < qNumber;nq++)
    if(cellArray[nq]->size != size)
      return PFNotValid;
  
  /* The q have to be sorted in increasing order and each value
     can appear only once */
  for(nq=0;nq < qNumber-1;nq++)
    if(cellArray[nq]->q >= cellArray[nq+1]->q)
      return PFNotValid;

  return PFYes;
}

/* Returns PFYes if pf is valid (meaningful content).
   It should always be the case after PFInit() has been used once. */
int PFIsValid(PartitionFunction pf)
{
  if( (_PFIsValid(pf) != PFYes) )
    return PFNotValid;

  if(pf->qNumber == 0)
    {
      if(pf->cellArray != NULL)
	return PFNotValid;
      else 
	return PFYes;
    }
  
  if( _PFCellArrayIsValid(pf->cellArray,pf->qNumber,pf->octaveNumber*pf->voiceNumber) != PFYes )
    return PFNotValid;
      
  return PFYes;
}
/*************************************************
 *
 * Writing a PartitionFunction in ASCII or BINARY format
 *
 *************************************************/

/* The same as fprinf but return PFWriErr on an error */
#define PFFPRINTF(exp) if( fprintf exp < 0 ) return PFWriErr

/* The same as fwrite but return PFWriErr on an error */
#define PFFWRITE(exp,lengh) if( fwrite exp != (size_t) lengh ) return PFWriErr

/* All the string constant defined below 
   should be shorter than PFMESSMAXSIZE */
#define PFMESSMAXSIZE 50

/* Format used while writing double */
#define PFDOUBLEWRITEFORMAT " %.15G "

/* Format used while reading double */
#define PFDOUBLEREADFORMAT "%lG "

#define PFMessIdent "partition function 2.0"
#define PFMessAsciiFormat "ascii"
#define PFMessBinFormat "binary"
#define PFMessBigEndian "big endian"
#define PFMessLittleEndian "little endian"
#define PFMessMethod "method: "
#define PFMessAMin "first scale: "
#define PFMessOctaveNumber "octaves number: "
#define PFMessVoiceNumber "voices number: "
#define PFMessIndexMax "maximum index: "
#define PFMessSignalSize "source size: "
#define PFMessSignalNumber "sources number: "
#define PFMessDimension "dimension: "
#define PFMessQNumber "q number: "
#define PFMessQList "q list: "
#define PFMessQ "q: "
#define PFMessSTq "sTq: "
#define PFMessSTqLogT "sTqLogT: "
#define PFMessLogSTq "logSTq: "
#define PFMessSTqLogT_sTq "sTqLogT_sTq: "
#define PFMessLog2STq "log^2(sTq): "
#define PFMessSTqLogT_sTq2 "sTqLogT_sTq^2: "
#define PFMessLogSTqSTqLogT_sTq "LogSTqSTqLogT_sTq: "

static int PFCellWriteAsciiOneFunc(FILE *fp,const char message[],
				   double *t,int tSize)
{
  int i;
  
  assert( fp != NULL );
  assert( message != NULL );
  assert( t != NULL );
  assert( tSize > 0 );
    
  PFFPRINTF((fp,"%s",message));
  for(i=0;i < tSize;i++)
    PFFPRINTF((fp,PFDOUBLEWRITEFORMAT,t[i]));
  PFFPRINTF((fp,"\n"));
  
  return PFYes;
}
static int PFCellWriteAscii(FILE *fp,PartitionFunctionCell pfc)
{
  assert( fp != NULL && pfc != NULL );
  
  PFFPRINTF((fp,"%s"PFDOUBLEWRITEFORMAT"\n",PFMessQ,pfc->q)); 

  if( PFCellWriteAsciiOneFunc(fp,PFMessSTq,pfc->sTq,pfc->size) != PFYes )
    return PFWriErr;
  if( PFCellWriteAsciiOneFunc(fp,PFMessSTqLogT,pfc->sTqLogT,pfc->size) != PFYes )
    return PFWriErr;
  if( PFCellWriteAsciiOneFunc(fp,PFMessLogSTq,pfc->logSTq,pfc->size) != PFYes )
    return PFWriErr;
  if( PFCellWriteAsciiOneFunc(fp,PFMessSTqLogT_sTq,pfc->sTqLogT_sTq,pfc->size) != PFYes )
    return PFWriErr;
  if( PFCellWriteAsciiOneFunc(fp,PFMessLog2STq,pfc->log2STq,pfc->size) != PFYes )
    return PFWriErr;
  if( PFCellWriteAsciiOneFunc(fp,PFMessSTqLogT_sTq2,pfc->sTqLogT_sTq2,pfc->size) != PFYes )
    return PFWriErr;

  if( PFCellWriteAsciiOneFunc(fp,PFMessLogSTqSTqLogT_sTq,pfc->logSTqSTqLogT_sTq,pfc->size) != PFYes )
    return PFWriErr;

  return PFYes;
}

int PFWriteAscii(FILE *fp,PartitionFunction pf)
{
  int nq;

  if( PFIsValid(pf) != PFYes) return PFNotValid;

  if(fp == NULL) return PFNo;


  PFFPRINTF((fp,PFMessIdent"\n"PFMessAsciiFormat"\n")); 
  PFFPRINTF((fp,PFMessMethod"%s\n",pf->method)); 
  PFFPRINTF((fp,PFMessAMin PFDOUBLEWRITEFORMAT"\n",pf->aMin)); 
  PFFPRINTF((fp,PFMessOctaveNumber"%d\n",pf->octaveNumber)); 
  PFFPRINTF((fp,PFMessVoiceNumber"%d\n",pf->voiceNumber)); 
  PFFPRINTF((fp,PFMessIndexMax"%d\n",pf->indexMax)); 
  PFFPRINTF((fp,PFMessSignalSize"%d\n",pf->signalSize)); 
  PFFPRINTF((fp,PFMessSignalNumber"%d\n",pf->signalNumber)); 
  PFFPRINTF((fp,PFMessDimension"%d\n",pf->dimension)); 
  PFFPRINTF((fp,PFMessQNumber"%d\n",pf->qNumber)); 
    
  PFFPRINTF((fp,PFMessQList)); 
  for(nq=0;nq < pf->qNumber;nq++)
    PFFPRINTF((fp,PFDOUBLEWRITEFORMAT,pf->cellArray[nq]->q)); 
  PFFPRINTF((fp,"\n")); 
    
  for(nq=0;nq < pf->qNumber;nq++)
    if( PFCellWriteAscii(fp,pf->cellArray[nq]) != PFYes )
      return PFWriErr;

  return PFYes;
}

/* Test whether we are running on big endian or little endian */
static char PFIsLittleEndian(void) 
{ 
  short unsigned int i;
  char *c;
  
  i = 1;
  c = (char *) (&i);
  if (*c == 0) return(PFNo);
  else return(PFYes);
} 

static int PFWriteBinHeader(FILE *fp,PartitionFunction pf)
{
  
  PFFWRITE( ((void *)pf,
	     sizeof(struct PartitionFunction)-sizeof(PartitionFunctionCell *),
	     1,fp),1);

  return PFYes;
}

static int PFCellWriteBin(FILE *fp,PartitionFunctionCell pfc)
{
  assert( fp != NULL && pfc != NULL );
  assert( pfc->sTq != NULL );
  assert( pfc->size > 0 );
  
  PFFWRITE( ((void *) &(pfc->q),sizeof(double),1,fp),1);
  PFFWRITE( ((void *) &(pfc->size),sizeof(int),1,fp),1);

  PFFWRITE( ((void *) pfc->sTq,sizeof(double),7*pfc->size,fp),7*pfc->size);

  return PFYes;
}

int PFWriteBin(FILE *fp,PartitionFunction pf)
{
  int nq;

  if( PFIsValid(pf) != PFYes) return PFNotValid;

  if(fp == NULL) return PFNo;


  PFFPRINTF((fp,PFMessIdent"\n"PFMessBinFormat"\n")); 
  if( PFIsLittleEndian() == PFYes)
    {PFFPRINTF((fp,PFMessLittleEndian"\n"));}
  else 
    {PFFPRINTF((fp,PFMessBigEndian"\n"));}

  /* Let's write the header */
  if( PFWriteBinHeader(fp,pf) != PFYes)
    return PFWriErr;
  
  /* Let's write the cells */
  for(nq=0;nq < pf->qNumber;nq++)
    if( PFCellWriteBin(fp,pf->cellArray[nq]) != PFYes )
      return PFWriErr;

  return PFYes;
}
/*************************************************
 *
 * Reading a PartitionFunction in ASCII or BINARY format
 *
 *************************************************/

/* The same as fscanf but return PFErrFormat on an error */
#define PFFSCANF(exp) if( fscanf exp <= 0 ) return PFErrFormat

/* The same as fread but return PFReadErr on an error */
#define PFFREAD(exp,lengh) if( fread exp != (size_t) lengh ) return PFReadErr

/* It reads the q from the stream fp.
   Enough space is supposed to have been allocated for qArray */
static int PFReadAsciiDoubleList(FILE *fp,const char *message,int dSize,
				 double *d)
{
  int i;
  char format[2*PFMESSMAXSIZE+1];

  assert( fp != NULL && message != NULL && d != NULL );
  assert( dSize > 0 );

  /* We read the first double verifying it is after the corect message */
  format[0] = '\0';
  strcat(format,message);
  strcat(format,PFDOUBLEREADFORMAT);
  PFFSCANF((fp,format,d)); 
 
  for(i=1;i < dSize;i++)
    PFFSCANF((fp,PFDOUBLEREADFORMAT,d+i));

  return PFYes;
}
static int PFCellReadAscii(FILE *fp,PartitionFunctionCell pfc)
{
  double q;

  assert( fp != NULL );
  assert( pfc != NULL );
  
  if( PFReadAsciiDoubleList(fp,PFMessQ,1,&q) != PFYes )
    return PFErrFormat;
  
  if( q != pfc->q )
    return PFErrFormat;

  if( PFReadAsciiDoubleList(fp,PFMessSTq,pfc->size,pfc->sTq) != PFYes )
    return PFErrFormat;
  if( PFReadAsciiDoubleList(fp,PFMessSTqLogT,pfc->size,pfc->sTqLogT) != PFYes )
    return PFErrFormat;
  if( PFReadAsciiDoubleList(fp,PFMessLogSTq,pfc->size,pfc->logSTq) != PFYes )
    return PFErrFormat;
  if( PFReadAsciiDoubleList(fp,PFMessSTqLogT_sTq,pfc->size,pfc->sTqLogT_sTq) != PFYes )
    return PFErrFormat;
  if( PFReadAsciiDoubleList(fp,PFMessLog2STq,pfc->size,pfc->log2STq) != PFYes )
    return PFErrFormat;
  if( PFReadAsciiDoubleList(fp,PFMessSTqLogT_sTq2,pfc->size,pfc->sTqLogT_sTq2) != PFYes )
    return PFErrFormat;
  if( PFReadAsciiDoubleList(fp,PFMessLogSTqSTqLogT_sTq,pfc->size,pfc->logSTqSTqLogT_sTq) != PFYes )
    return PFErrFormat;

  return PFYes;
}

/* Reading function for the ASCII format */
static int PFReadAscii(FILE *fp,PartitionFunction pf)
{
  char methodLine[PFMESSMAXSIZE+PFMETHODSIZE+2];
  int nq;
  int err;
  double *qArray;

  assert( fp != NULL && pf != NULL );
  
  /* If pf->cellArray != NULL it means that we are writing
     on an already used PartitionFunction
     So we delete the cellArray */
  if( pf->cellArray != NULL )
    {
      PFCellArrayDelete(pf->cellArray,pf->qNumber);
      pf->cellArray = NULL;
    }
    
  if( fgets(methodLine,PFMESSMAXSIZE+PFMETHODSIZE+2,fp) == NULL)
    return PFReadErr;
  if( strncmp(methodLine,PFMessMethod,strlen(PFMessMethod)) != 0 )
    return PFErrFormat;
  /* The last character in methodLine is \n we convert it to \0 */
  methodLine[strlen(methodLine)-1] = '\0';
  strcpy(pf->method,methodLine+strlen(PFMessMethod));

  PFFSCANF((fp,PFMessAMin PFDOUBLEREADFORMAT"\n",&pf->aMin)); 
  PFFSCANF((fp,PFMessOctaveNumber"%d\n",&pf->octaveNumber)); 
  PFFSCANF((fp,PFMessVoiceNumber"%d\n",&pf->voiceNumber)); 
  PFFSCANF((fp,PFMessIndexMax"%d\n",&pf->indexMax)); 
  PFFSCANF((fp,PFMessSignalSize"%d\n",&pf->signalSize)); 
  PFFSCANF((fp,PFMessSignalNumber"%d\n",&pf->signalNumber)); 
  PFFSCANF((fp,PFMessDimension"%d\n",&pf->dimension)); 
  PFFSCANF((fp,PFMessQNumber"%d\n",&pf->qNumber)); 
  
  /* We verify that the values are meaningful */
  if( _PFIsValid(pf)  != PFYes )
    return PFNotValid;

  /* We allocate space for the qs */
  qArray = (double *) malloc(pf->qNumber*sizeof(double));
  if( qArray == NULL ) return PFErrAlloc;

  /* We read the qs */
  err = PFReadAsciiDoubleList(fp,PFMessQList,pf->qNumber,qArray);
  if( err != PFYes )
    {
      free(qArray);
      return err;
    }

  /* We initialize the cell array */
  pf->cellArray = PFCellArrayNew(pf->octaveNumber*pf->voiceNumber,
				 pf->qNumber,qArray);
  if( pf->cellArray == NULL) 
    {
      free(qArray);
      return PFErrAlloc;
    }
  
  
  /* We read the content of the cells */
  for(nq=0;nq < pf->qNumber;nq++)
    if( PFCellReadAscii(fp,pf->cellArray[nq]) != PFYes )
      {
	free(qArray);
	return PFErrFormat;
      }

  free(qArray);
  return PFYes;
}

/* Functions to convert Big Endian to Little Endian and Little to Big */
static void PFBigLittleOneVar(void *var,size_t sizevar)
{
  size_t i;
  unsigned char c;
  unsigned char *pvar;
  
  pvar = (unsigned char *) var;
  
  for(i=0;i<sizevar/2;i++)
    {
      c = *(pvar+i);
      *(pvar+i) = *(pvar+sizevar-1-i);
      *(pvar+sizevar-1-i) = c;
    }
}
static void PFBigLittleHeader(PartitionFunction pf)
{
  PFBigLittleOneVar(&(pf->aMin),sizeof(double));
  PFBigLittleOneVar(&(pf->octaveNumber),sizeof(int));
  PFBigLittleOneVar(&(pf->voiceNumber),sizeof(int));
  PFBigLittleOneVar(&(pf->indexMax),sizeof(int));
  PFBigLittleOneVar(&(pf->signalSize),sizeof(int));
  PFBigLittleOneVar(&(pf->signalNumber),sizeof(int));
  PFBigLittleOneVar(&(pf->dimension),sizeof(int));
  PFBigLittleOneVar(&(pf->qNumber),sizeof(int));
}

static void PFCellBigLittle(PartitionFunctionCell pfc)
{
  int i;
  
  PFBigLittleOneVar(&(pfc->q),sizeof(double));
  
  for(i=0;i< 7*pfc->size;i++)
    PFBigLittleOneVar(&(pfc->sTq[i]),sizeof(double));
}
/* ******************* */

static int PFReadBinHeader(FILE *fp,PartitionFunction pf,int flagSwap)
{
  
  PFFREAD( ((void *)pf,
	     sizeof(struct PartitionFunction)-sizeof(PartitionFunctionCell *),
	     1,fp),1);

  if(flagSwap == PFYes)
    PFBigLittleHeader(pf);
  
  return PFYes;
}

static int PFCellReadBin(FILE *fp,PartitionFunctionCell pfc,int flagSwap)
{
  int size;

  assert( fp != NULL && pfc != NULL );
  assert( pfc->sTq != NULL );
  assert( pfc->size > 0 );
  
  PFFREAD( ((void *) &(pfc->q),sizeof(double),1,fp),1);
  PFFREAD( ((void *) &size,sizeof(int),1,fp),1);
  
  if (flagSwap == PFYes)
    PFBigLittleOneVar((void *) &size,sizeof(int));

  if(pfc->size != size)
    return PFErrFormat;
  
  PFFREAD( ((void *) pfc->sTq,sizeof(double),7*pfc->size,fp),7*pfc->size);
  
  if (flagSwap == PFYes)
    PFCellBigLittle(pfc);
  
  return PFYes;
}

static int PFReadBin(FILE *fp,PartitionFunction pf)
{
  char endianLine[PFMESSMAXSIZE+2];
  int flagLittle,flagSwap;
  int nq;
  double *qArray;
  
  assert( fp != NULL && pf != NULL );
  
  if( fgets(endianLine,PFMESSMAXSIZE+2,fp) == NULL)
    return PFReadErr;
  /* The last character in endianLine is \n we convert it to \0 */
  endianLine[strlen(endianLine)-1] = '\0';

  if( strcmp(endianLine,PFMessLittleEndian) == 0 )
    flagLittle = PFYes;
  else if( strcmp(endianLine,PFMessBigEndian) == 0)
    flagLittle = PFNo;
  else
    return PFErrFormat;

  /* Is the endian convention in the file read the same as the computer
     we are working on?
     */
  if( flagLittle != PFIsLittleEndian() )
    flagSwap = PFYes;
  else
    flagSwap = PFNo;
    
  /* If pf->cellArray != NULL it means that we are writing
     on an already used PartitionFunction
     So we delete the cellArray */
  if( pf->cellArray != NULL )
    {
      PFCellArrayDelete(pf->cellArray,pf->qNumber);
      pf->cellArray = NULL;
    }
  
  
  if( PFReadBinHeader(fp,pf,flagSwap) != PFYes)
    return PFReadErr;
  
  /* We verify that the values are meaningful */
  if( _PFIsValid(pf)  != PFYes )
    return PFNotValid;

  /* We allocate space for the qArray */
  qArray = (double *) calloc(pf->qNumber,sizeof(double));
  if( qArray == NULL ) return PFErrAlloc;

  
  /* We initialize the cell array */
  pf->cellArray = PFCellArrayNew(pf->octaveNumber*pf->voiceNumber,
				 pf->qNumber,qArray);
  if( pf->cellArray == NULL) 
    {
      free(qArray);
      return PFErrAlloc;
    }
  
  /* We read the content of the cells */
  for(nq=0;nq < pf->qNumber;nq++)
    if( PFCellReadBin(fp,pf->cellArray[nq],flagSwap) != PFYes )
      {
	free(qArray);
	return PFReadErr;
      }

  free(qArray);
  return PFYes;
}

int PFRead(FILE *fp,PartitionFunction pf)
{
  char tempLine[PFMESSMAXSIZE+2];

  if(pf == NULL) return PFNo;
  
  if(fp == NULL) return PFNo;
  
  /* Are we dealing with a partition function file ? */
  if( fgets(tempLine,PFMESSMAXSIZE+2,fp) == NULL)
    return PFReadErr;
  /* The last character in templine is \n we convert it to \0
     before comparing strings */
  tempLine[strlen(tempLine)-1] = '\0';
  if( strcmp(tempLine,PFMessIdent) != 0 )
    return PFErrFormat;

  /* Is it an ASCII or a BINARY file ? */
  if( fgets(tempLine,PFMESSMAXSIZE+2,fp) == NULL)
    return PFReadErr;
  /* The last character in templine is \n we convert it to \0
     before comparing strings */
  tempLine[strlen(tempLine)-1] = '\0';
  if( strcmp(tempLine,PFMessAsciiFormat) == 0 )
    return PFReadAscii(fp,pf);
  else if( strcmp(tempLine,PFMessBinFormat) == 0 )
    return PFReadBin(fp,pf);
  else return PFErrFormat;
}


/*************************************************
 *
 * computation of the partition functions at one scale
 *
 *************************************************/

/* The ordering of data in PFComputeOneScaleF() is done using this function.
   One can change it using PFChangeQsort() especially if no sorting is wanted.
   The new sorting function must work the same as qsort() of <stdlib.h>
   */
void PFQNoSort(void *base,int nel,int size,
	       int (*compar)(const void *, const void *))
{
  return;
}
static void (*PFQsortData)(void *base,int  nel,int  size,
			   int (*compar)(const void *, const void *)) 
     = (void (*)( void *,int,int ,int (*)(const void *,const void *))) qsort;
     
void PFChangeQsort( void (*NewQsort)(void *,int ,int ,
				     int (*)(const void *, const void *)) )
{
  if(NewQsort == NULL)
    PFQsortData = ( void (*)(void *,int,int ,
			     int (*)(const void *,const void *)) ) qsort;
  else
    PFQsortData = NewQsort;
}

/* float version: the wavelet coefficients are given 
   in an array of float */
/* scales go from 0 to octaveNumber*voiceNumber-1 */
int PFComputeOneScaleF(PartitionFunction pf,int scale,const float *t,int tSize)
{
  int nq,i,imin;
  double tm;
  double q;
  double *tempAbsT,*tempTq,*tempLogT;
  PartitionFunctionCell tempPfc;

  /* Some verifications */
  if(pf == NULL) 
    return PFNo;
  if( (PFIsValid(pf) != PFYes) || (pf->qNumber <= 0) || (scale < 0) 
      || (scale >= pf->octaveNumber*pf->voiceNumber) || (tSize <= 0) )
    return PFNotValid;
  
  /* One is supposed to compute scales in increasing order */
  if( (scale - pf->indexMax) != 1 )
    return PFNotValid;

  /* Allocation of space for tempAbsT and tempMonome
     using only one call to malloc */
  tempAbsT = (double *) malloc(3*tSize*sizeof(double));
  if(tempAbsT == NULL) return PFNo;
  tempTq = tempAbsT + tSize;
  tempLogT = tempAbsT + 2*tSize;

  /* Computation of |t|, sorting of |t| and determination of imin 
     such that for i >= imin |t|(i) != 0 
     */
  for(i=0;i<tSize;i++)
    tempAbsT[i] = fabs((double) t[i]);
  PFQsortData((void *)tempAbsT,tSize,sizeof(double),
	      (int (*)(const void *,const void *))PFCompDouble);
  imin =0;
  while(tempAbsT[imin] == 0)
    imin++;

  /*  Printf("imin= %d, Tmin= %g, Tmax %g\n",imin,tempAbsT[imin],
      tempAbsT[tSize-1]);
      */

  for(nq=0; nq < pf->qNumber; nq++)
    {
      tempPfc = pf->cellArray[nq];
      q = tempPfc->q;
  
      /* Do we want T/tm to be >= 1 or <= 1 */
      if(q >= 0.)
	tm = tempAbsT[imin];
      else
	tm = tempAbsT[tSize-1];
      
      /* We compute Tq and Log(T/tm) */
      for(i=imin; i < tSize; i++)
	{
	  tempTq[i] = pow(tempAbsT[i],q);
	  tempLogT[i] = log(tempAbsT[i]/tm);
	}
      
      /* We compute sTq and sTqLogT */
      if(q >= 0.)
	{
	  for(i=imin; i < tSize; i++)
	    {
	      /* The arrays of a PartitionFunctionCell are obtained with
		 calloc, so there is no need to initialize them
		 */
	      tempPfc->sTq[scale] += tempTq[i];
	      tempPfc->sTqLogT[scale] += tempTq[i]*tempLogT[i];
	    }
	}
      else
	{
	  for(i=tSize-1; i >= 0; i--)
	    {
	      /* The arrays of a PartitionFunctionCell are obtained with
		 calloc, so there is no need to initialize them
		 */
	      tempPfc->sTq[scale] += tempTq[i];
	      tempPfc->sTqLogT[scale] += tempTq[i]*tempLogT[i];
	    }
	}
      /* sTqLogT = sTqLog(T/tm)+log(tm)*sTq
       */
      tempPfc->sTqLogT[scale] = 
	tempPfc->sTqLogT[scale] + log(tm)*tempPfc->sTq[scale];
      
      /* We compute LogSTq, sTqLogT_sTq, log2STq, sTqLogT_sTq2 
	 and logSTqSTqLogT_sTq
       */
      if(tempPfc->sTq[scale] != 0.0)
	{
	  tempPfc->logSTq[scale] = 
	    log(tempPfc->sTq[scale]/((double) (tSize-imin)));
	  tempPfc->log2STq[scale] = 
	    tempPfc->logSTq[scale]*tempPfc->logSTq[scale];

	  tempPfc->sTqLogT_sTq[scale] = 
	    tempPfc->sTqLogT[scale]/tempPfc->sTq[scale];
	  tempPfc->sTqLogT_sTq2[scale] = 
	    tempPfc->sTqLogT_sTq[scale]*tempPfc->sTqLogT_sTq[scale];
	  tempPfc->logSTqSTqLogT_sTq[scale] = 
	    tempPfc->logSTq[scale]*tempPfc->sTqLogT_sTq[scale];
	}
      else 
	{
	  tempPfc->logSTq[scale] = 0.0;
	  tempPfc->sTqLogT_sTq[scale] = 0.0;
	  tempPfc->log2STq[scale] = 0.0;
	  tempPfc->sTqLogT_sTq2[scale] = 0.0;
	  tempPfc->logSTqSTqLogT_sTq[scale] = 0.0;
	}

    }

  pf->indexMax = scale;
  free(tempAbsT);
  return PFYes;
}


/*************************************************
 *
 * Addition of two PartitionFunctions
 *
 *************************************************/

/* Comparaison of qList */
enum PFCompQlistCode { PFQListEqual, PFQListDifferent, PFQListMixed };
static int PFCompQList(const PartitionFunction pf1,const PartitionFunction pf2)
{
  int nq1,nq2;
  int flagEqual = PFNo;

  assert( PFIsValid(pf1) == PFYes );
  assert( PFIsValid(pf2) == PFYes );
  
  /* Are they strickly equal ? */
  if( pf1->qNumber == pf2->qNumber)
    {
      flagEqual = PFYes;
      for(nq1=0;nq1 < pf1->qNumber;nq1++)
	if(fabs(pf1->cellArray[nq1]->q - pf2->cellArray[nq1]->q) > PFQDIFFMAX)
	   {
	     flagEqual = PFNo;
	     break;
	   }
    }
  if(flagEqual == PFYes)
    return PFQListEqual;

  /* Are they strickly different ? */
  for(nq1=0;nq1 < pf1->qNumber;nq1++)
      for(nq2=0;nq2 < pf2->qNumber;nq2++)
	{
	  if(fabs(pf1->cellArray[nq1]->q - pf2->cellArray[nq2]->q) <= PFQDIFFMAX)
	    return PFQListMixed;
	}

  return PFQListDifferent;
}

/* Addition of two PartitionFunctionCell, the result is in pfc1.
   q's should be the same but pfc1->size may be bigger.
   ( it is assumed that the difference only comes from a different
   octaveNumber ) 
   */
static int PFCellAdd(PartitionFunctionCell pfc1,
		     const PartitionFunctionCell pfc2)
{
  int i;

  if( (pfc1->size < pfc2->size) || fabs(pfc1->q - pfc2->q) > PFQDIFFMAX ) 
    return PFNotCompatible;

  for(i=0;i < pfc2->size;i++)
    {
      pfc1->sTq[i] += pfc2->sTq[i];
      pfc1->sTqLogT[i] += pfc2->sTqLogT[i];
      pfc1->logSTq[i] += pfc2->logSTq[i];
      pfc1->sTqLogT_sTq[i] += pfc2->sTqLogT_sTq[i];
      pfc1->log2STq[i] += pfc2->log2STq[i];
      pfc1->sTqLogT_sTq2[i] += pfc2->sTqLogT_sTq2[i];
      pfc1->logSTqSTqLogT_sTq[i] += pfc2->logSTqSTqLogT_sTq[i];
      
    }    
  
  return PFYes;
}
/* The result will be in pf1 */
int PFStandardAddition(PartitionFunction pf1,const PartitionFunction pf2)
{
  int nq;
  int compQList;
  PartitionFunctionCell *cellArray;

  if( (PFIsValid(pf1) != PFYes) || (PFIsValid(pf2) != PFYes) )
    return PFNotValid;

  /* method, aMin, octaveNumber, voiceNumber, signalSize, dimension
     have to be equal for the addition to be meaningful */
  if( (strcmp(pf1->method,pf2->method) != 0) 
      || (fabs(pf1->aMin - pf2->aMin) > PFQDIFFMAX)
      || (pf1->octaveNumber != pf2->octaveNumber)
      || (pf1->voiceNumber != pf2->voiceNumber) 
      || (pf1->signalSize != pf2->signalSize) 
      || (pf1->dimension != pf2->dimension) )
    return PFNotCompatible;
  
  compQList = PFCompQList(pf1,pf2);

  /* If qList are mixed we can't add */
  if( compQList == PFQListMixed )
    return PFNotCompatible;
  /* If the qList are strickly different and all the other fields are 
     the same (except indexMax and qNumber) then the result is simply 
     the concatenation of the two cellArrays: we are adding new values of q
     */
  else if( compQList == PFQListDifferent )
    {
      if(pf1->signalNumber != pf2->signalNumber)
	return PFNotCompatible;
      
      cellArray = PFCellArrayConcat(pf1->qNumber,pf1->cellArray,
				    pf2->qNumber,pf2->cellArray);
      if(cellArray == NULL) return PFErrAlloc;

      PFCellArrayDelete(pf1->cellArray,pf1->qNumber);
      pf1->cellArray = cellArray;
      pf1->qNumber = pf1->qNumber + pf2->qNumber;
     
      if(pf1->indexMax > pf2->indexMax)
	pf1->indexMax = pf2->indexMax;
      
      return PFYes;
    }
  /* If the qList are strickly equal and all the other fields are 
     the same (except indexMax and signalNumber) then the result is
     the addition of the partition function for each q: we are adding 
     more statistic 
     */
  else if( compQList == PFQListEqual )
    {
      for(nq=0;nq < pf1->qNumber;nq++)
	if( PFCellAdd(pf1->cellArray[nq],pf2->cellArray[nq]) != PFYes)
	  return PFNotCompatible;
      pf1->signalNumber = pf1->signalNumber + pf2->signalNumber;

      if(pf1->indexMax > pf2->indexMax)
	pf1->indexMax = pf2->indexMax;

      return PFYes;
    }

  return PFNo;
}

int PFNonStandardAddition(PartitionFunction pf1,const PartitionFunction pf2)
{
  int compQList;
  int nq;
  
  if( (PFIsValid(pf1) != PFYes) || (PFIsValid(pf2) != PFYes) )
    return PFNotValid;

  compQList = PFCompQList(pf1,pf2);
  
  /* method, aMin, voiceNumber, dimension, qList must be equal for
     the addition to be meaningful.
     octaveNumber2 must be  smaller than octaveNumber1 */
  if( (strcmp(pf1->method,pf2->method) != 0) 
      || (fabs(pf1->aMin - pf2->aMin) > PFQDIFFMAX)
      || (pf1->octaveNumber < pf2->octaveNumber)
      || (pf1->voiceNumber != pf2->voiceNumber)
      || (pf1->dimension != pf2->dimension)
      || (compQList != PFQListEqual)
      )
    return PFNotCompatible;

  for(nq=0;nq < pf1->qNumber;nq++)
    if( PFCellAdd(pf1->cellArray[nq],pf2->cellArray[nq]) != PFYes)
      return PFNotCompatible;

  pf1->signalSize = pf1->signalSize + pf2->signalSize;
  pf1->signalNumber = pf1->signalNumber + pf2->signalNumber;
  

  if(pf1->indexMax > pf2->indexMax)
    pf1->indexMax = pf2->indexMax;
  
  return PFYes;
}
