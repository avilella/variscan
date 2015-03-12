/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry, Stephane Mallat             */
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
/* 	 filter.h :    The filter and filterGroup structures      */
/*                                                            */
/**************************************************************/



#ifndef FILTER1D_H

#define FILTER1D_H

/********************************************************************/
/***************************** FILTERS ******************************/
/********************************************************************/


/************************************/
/*       Filter structure           */
/************************************/

typedef struct filter{
  int size;					/* filter size */
  int shift;				/* filter shift */
  float *Y;	                /* filter values */
} *FILTER;


/*****************************************************/
/* Allocation Functions on FILTERS  (FILTER_ALLOC.c) */
/*****************************************************/

/* Create a new filter structure and returns it  */
extern FILTER NewFilter(void);              

/* Desallocate the whole FILTER structure */
extern void DeleteFilter(FILTER filter);

/* Copy a filter into another */
extern void CopyFilter(FILTER in,FILTER out);



/********************************************************************/
/************************* FILTER GROUPS ****************************/
/********************************************************************/


/************************************/
/* Different types of filter groups */
/************************************/

#define F_DYAD 1
#define F_BIOR 2
#define F_ORTH 3
#define F_BIOR_NO_SYM 4

/****************************************/
/*      FilterGroup structure           */
/****************************************/

typedef struct filterGroup{
  int type;           /* type of the filters */
  char *filename;     /* full name of file where the filters are */
  float *factors;     /* factors for renormalization (only for F_DYAD) */
  FILTER H1;          /* The 4 filters */
  FILTER G1;                
  FILTER H2;                
  FILTER G2;          
  int nRef;          /* reference count number */      
} *FILTERGROUP;


/********************************************************/
/* Allocation Functions on FILTERGROUP (FILTER_ALLOC.c) */
/********************************************************/

extern FILTER NewFilter(void);
extern void DeleteFilter(FILTER filter);
extern void CopyFilter(FILTER in,FILTER out);


/* Create a new filterGroup structure and returns it  */
extern FILTERGROUP NewFilterGroup(void);              

/* Desallocate the whole FILTERGROUP structure */
extern void DeleteFilterGroup (FILTERGROUP fg);


/*****************************************************/
/* IO Functions on FILTERGROUP  (FILTER_FILE.c)      */
/*****************************************************/

extern void FilterGroupRead(char *filename, FILTERGROUP fg,int type);


/*****************************************/
/*      Print filters                    */
/*****************************************/

void PrintBiorFilter(FILTER filter); /*in FILTER_BIOR.c */
void PrintDyadFilter(FILTER filter); /*in FILTER_DYAD.c */


/****************************************************************************/
/*                                                                          */
/*  filter_biorth.h   Macros for dealing with border effects of filters     */
/*                                                                          */
/****************************************************************************/


/* F is a filter */
/* Abscissa of the left-most coeff of the filter */
#define LEFT_FILT(F) 	(-((F->size-1)>>1)-F->shift)

/* Abscissa of the right-most non-zero coeff of the filter */
#define RIGHT_FILT(F) 	(((F->size)>>1)-F->shift)

/* Index of the zero abscissa of the filter */
#define ZERO(F)         (((F->size-1)>>1)+F->shift)




#endif






