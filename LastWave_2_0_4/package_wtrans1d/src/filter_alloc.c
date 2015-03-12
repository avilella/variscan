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



/****************************************************************************/
/*                                                                          */
/*  filter_alloc.c   Allocation of wavelet transform filter structures       */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"


/*********************************/
/* Return a new filter structure */
/*********************************/

FILTER NewFilter(void)
{
  FILTER filter;

  filter = (FILTER) (Malloc(sizeof(struct filter)));
    
  filter->Y = NULL; 
  filter->size = 0;
  filter->shift = 0;
  
  return(filter);
}


/******************************************/
/* Desallocate the whole FILTER structure */
/******************************************/

void DeleteFilter(FILTER filter)
{
  if (filter == NULL) return;

  if (filter->size != 0 && filter->Y != NULL) Free(filter->Y);
 
  Free(filter);
}

/******************************************/
/* Copy the whole FILTER structure        */
/******************************************/

void CopyFilter(FILTER in,FILTER out)
{
  /* Tests */
  if (in == NULL) return;
  if (out == NULL) Errorf("CopyFilter() : the out filter is NULL");
  
  if (out->Y != NULL && out->size != in->size) Free(out->Y);
  
  out->Y = FloatAlloc(in->size);
  out->size = in->size;
  out->shift = in->shift;
  
  memcpy(out->Y,in->Y,in->size*sizeof(float));
}


/**************************************/
/* Return a new filterGroup structure */
/**************************************/

FILTERGROUP NewFilterGroup(void)
{
  FILTERGROUP filterGroup;

  filterGroup = (FILTERGROUP) Malloc(sizeof(struct filterGroup));
    
  filterGroup->type = 0;
  filterGroup->filename = NULL;
  filterGroup->factors = NULL;
  filterGroup->H1 = NewFilter();
  filterGroup->H2 = NewFilter();
  filterGroup->G1 = NewFilter();
  filterGroup->G2 = NewFilter();
  filterGroup->nRef = 1;

  return(filterGroup);
}


/***********************************************/
/* Desallocate the whole FILTERGROUP structure */
/***********************************************/

void DeleteFilterGroup(FILTERGROUP fg)
{
  if (fg == NULL) return;
  
  fg->nRef--;
  if (fg->nRef != 0) return;
  
  if (fg->filename != NULL) Free(fg->filename);
  if (fg->factors != NULL) Free(fg->factors);
  if (fg->H1 != NULL) DeleteFilter(fg->H1);
  if (fg->H2 != NULL) DeleteFilter(fg->H2);
  if (fg->G1 != NULL) DeleteFilter(fg->G1);
  if (fg->G2 != NULL) DeleteFilter(fg->G2);
 
  Free(fg);
}

/************************************************/
/* Set the filter group of a wtrans             */
/************************************************/

void SetFGWtrans(WTRANS wtrans,FILTERGROUP fg)
{
  if (wtrans->fg != NULL) DeleteFilterGroup(wtrans->fg);
  wtrans->fg = fg;
 if (fg != NULL) fg->nRef++;
}


/***********************************/
/* Command To print a filter group */
/***********************************/

/* Print one filter */
static void PrintFilter(FILTER filter,int type)
{
  switch (type) {
    case F_DYAD:
      PrintDyadFilter(filter);
      break;
    case F_ORTH: case F_BIOR: case F_BIOR_NO_SYM:
      PrintBiorFilter(filter);
      break;
  }
}

/* The command */
void C_PrintFG(char **argv)
{
  WTRANS wtrans;
  char *str;
  FILTER f;
  int type;
  
  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tSTR_,NULL,&str,0);
 
  if (wtrans == NULL) wtrans = GetWtransCur();
  if (wtrans->fg == NULL) Errorf("No filter loaded in wtrans !");
  type = wtrans->fg->type;
    
  if (str == NULL) f = NULL;
  else if (!strcmp(str,"H1")) f = wtrans->fg->H1;
  else if (!strcmp(str,"H2")) f = wtrans->fg->H2;
  else if (!strcmp(str,"G1")) f = wtrans->fg->G1;
  else if (!strcmp(str,"G2")) f = wtrans->fg->G2;
  else Errorf("Invalid name '%s' for filter",str);

  Printf("Filter '%s' ",wtrans->fg->filename);
  switch (type) {
    case F_ORTH: Printf("(Orthogonal) "); break;
    case F_BIOR: Printf("(Biorthogonal) "); break;
    case F_BIOR_NO_SYM:Printf("(Biorthogonal - no symmetric) "); break;
    case F_DYAD: Printf("(Dyadic) "); break;
    default: Printf("(unknown type) ");
  }
  
  if (f != NULL) {
    Printf("-- %s\n",str);
    PrintFilter(f,wtrans->fg->type); 
  }
  else {
    Printf("-- H1\n");
    PrintFilter(wtrans->fg->H1,type); 
    Printf("\n-- G1\n");
    PrintFilter(wtrans->fg->G1,type); 
    if (type == F_BIOR || type == F_BIOR_NO_SYM) {
      Printf("\n-- H2\n");
      PrintFilter(wtrans->fg->H2,type); 
      Printf("\n-- G2\n");
      PrintFilter(wtrans->fg->G2,type);
    }
  } 
}

