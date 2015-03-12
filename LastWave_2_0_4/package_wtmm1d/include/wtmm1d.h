/*..........................................................................*/
/*                                                                          */
/*  L a s t W a v e    P a c k a g e 'wtmm1d' 1.2                           */
/*                                                                          */
/*      Copyright (C) 2000 Benjamin Audit.                                  */
/*      emails : audit@ebi.ac.uk                                            */
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


#ifndef WTMM1D_H
#define WTMM1D_H			 		 

#include "lastwave.h"
#include "extrema1d.h"

#include "pf_lib.h"

typedef struct LastWavePartitionFunction 
{
  /* The fields of the VALUE structure */
  ValueFields;

  PartitionFunction pf;
} *LWPARTFUNC;

/* PF types */
extern int tPF,tPF_;
extern char *partitionFunctionType;


/**** Functions in pf_alloc.c */
/* Create a new partitionFunction and returns it */
extern LWPARTFUNC NewPartitionFunction(void);
/* Desallocate the whole LWPARTFUNC structure */
extern void DeletePartitionFunction(LWPARTFUNC lwpf);


/* Functions in pf_functions.c */
extern int ComputePartFuncOnExtrep(LWPARTFUNC lwpf,EXTREP extrep,
				   int qNumber,double *qArray);
extern int ComputePartFuncOnWtrans(LWPARTFUNC lwpf,WTRANS wtrans,
				   int qNumber,double *qArray,int flagCausal);

#endif /* WTMM1D_H */
