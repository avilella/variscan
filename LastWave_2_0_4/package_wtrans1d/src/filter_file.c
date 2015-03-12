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
/*  filter.c         General Functions to deal with wavelet transform       */
/*                   filters                                                */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "wtrans1d.h"



/********************************************************/
/*         Some useful Functions and Constants             */
/*         for reading the filter files                 */
/********************************************************/

#define COMMENT '#'      /* Comments character      */
#define EO_FILTER '$'    /* End of filter character */

/* Macro to skip the comments in the filter files */
#define get_token \
do { \
 fscanf(fp,"%[^\n\r]\n\r",&input); \
 token = input; \
 while (token[0] == ' ') token++; \
} while (token[0] == '\n' || token[0] == '\r' || token[0] == COMMENT)
  
  
/* Associated variables */
static char input[STRSIZE],*token;


/********************************************************/
/* This function reads a 'filter' of type 'type' and    */
/* from the stream 'fp'.                                */
/*   '#' is used for comments, '$' to end a filter      */
/*    Blank lines are skipped, and so are leading blanks*/
/* The format of the file is :                          */
/*	    												*/
/* filter_size                                          */
/* filter_shift											*/
/* coefficients of filter    							*/
/* $                                                    */
/********************************************************/

static void FilterRead(FILTER filter,FILE * fp,int type)
{
  int j;
  
  /* Desallocation if needed */
  if (filter->Y != NULL) Free(filter->Y);
  
  /* Read filter size. */
  get_token;
  filter->size = atoi(token);

  /* Read filter shift. */
  get_token;
  filter->shift = atoi(token);

  /* Read filter. */
  filter->Y = FloatAlloc(filter->size);
  for (j = 0; j < filter->size; j++) {
    get_token;
    filter->Y[j] = atof(token);
    if(type == F_BIOR) filter->Y[j] *= sqrt(2.0) ;
  }

  /* Skip the extra coefficients if any. */
  while(!feof(fp) && token[0] != EO_FILTER) get_token;
}  


/********************************************************/
/*                                                      */
/* This function reads the one or two filters that are  */
/* contained in the file and put them in the filter     */
/* group variable 'fg'.                                 */
/* Then builds the other filters                        */
/*                                                      */
/* There are 3 cases of files :                         */
/*     1)  *.o : orthogonal filters (type is F_ORTH)    */ 
/*         Only one filter is coded in the file, it     */
/*         corresponds to H1 = H2 (no other filter)     */
/*     2)  *.b : biorthogonal filters (type is F_BIOR)  */
/*         Two filters are coded in the file, they      */
/*         corresponds to H1 and H2                     */
/*     3)  *.d : dyadic filters (type is F_DYAD)        */
/*         Two filters are coded in the file, they      */
/*         corresponds to H1 = H2 and G1 = G2           */
/*                                                      */
/* In each file:                                        */
/*   '#' is used for comments, '$' to end a filter      */
/*    Blank lines are skipped, and so are leading blanks*/
/*                                                      */
/* The format of the file is :                          */
/*	    						                        */
/* type 						                        */
/* F1_size                                              */
/* F1_shift					                            */
/* coefficients of F1					                */
/* $	(a F_ORTH filter ends here)    	        	    */
/* F2_size                                           	*/
/* F2_shift                                          	*/
/* coefficients of F2                                 	*/
/* $                                                   	*/
/* factors  (for a F_DYAD only)    			            */
/* $							                        */
/*                                                      */
/********************************************************/


void FilterGroupRead(char *filename, FILTERGROUP fg, int theType)
{
  FILE    *fp;
  int      j,type;
   
   
  /* Some Tests */
  if (!(fp = FOpen(filename, "r"))) 
     Errorf("FilterGroupRead() : filter %s does not exist", filename);

  
  /*************************************/
  /* read the type of the filtergroup  */
  /*************************************/
  get_token;
  type = atoi(token);
  if (theType != 0 &&
      ((theType == F_BIOR && type != F_BIOR && type != F_ORTH) ||
       (theType == F_ORTH && type != F_ORTH) ||
       (theType == F_DYAD && type != F_DYAD))){
      FClose(fp);
      Errorf("FilterGroupRead() : Filter does not correspond to recquested type");
  }
      

  /*************************************/
  /* Initialization of the filtergroup */
  /*************************************/
  switch(type) {
    case F_DYAD :
      if (fg->factors == NULL) fg->factors = FloatAlloc(NOCT+1);
      break;
    case F_ORTH :
      break;
    case F_BIOR :
      break;
    default :
      Errorf("FilterGroupRead() : filter %s has a bad type", filename);
  }
  fg->type = type;
  if (fg->filename != NULL) Free(fg->filename);
  fg->filename = CopyStr(filename);
  
  
  /**************************************/
  /* We read the first filter : H1      */
  /**************************************/
  FilterRead(fg->H1,fp,type);

  /* if orthogonal then we are done */
  if (type == F_ORTH) {
    FClose(fp);
    return;
  }
  
  /**************************************/
  /* We read the second filter          */
  /**************************************/
  
  if (type == F_BIOR) FilterRead(fg->H2,fp,type);
  else FilterRead(fg->G1,fp,type);
  
  /* if biorthogonal then we are done */
  if (type == F_BIOR) {
    FClose(fp);
    return;
  }
  
   
  /* If F_DYAD we read the factors */
  j = 0;
  get_token;
  while (token[0] != EO_FILTER && j < NOCT) {
    fg->factors[j++] = atof(token);
    get_token;
  }
  for (;j<NOCT;j++) fg->factors[j] = 1.0;
  
  FClose(fp);
}









