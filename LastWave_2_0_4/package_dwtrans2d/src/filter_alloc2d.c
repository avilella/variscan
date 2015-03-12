/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'dwtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  E.Bacry, J.Fraleu, J.Kalifa, E. Le Pennec, */
/*                         W.L. Hwang , S.Mallat, S.Zhong                   */
/*      emails : lastwave@cmap.polytechnique.fr                             */
/*               fraleu@cmap.polytechnique.fr                               */
/*               kalifa@cmap.polytechnique.fr                               */
/*               lepennec@cmap.polytechnique.fr                             */
/*               mallat@cmap.polytechnique.fr                               */
/*               whwang@iis.sinica.edu.tw                                   */
/*               szhong@chelsea.princeton.edu                               */
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


#include "lastwave.h"
#include "dwtrans2d.h"
  


/*
 * (Des)Allocation functions
 */
 
FILTER2 NewFilter2(void)
     
{
  FILTER2 filter;

  if(!(filter = (FILTER2) (Malloc(sizeof(struct filter2)))))
    Errorf("Mem. alloc for FILTER failed\n");
   filter->size=0;
   filter->symmetry = 1.0;
  filter->name=NULL;
  return(filter);
}

void DeleteFilter2(FILTER2 filter)
     
{ if (filter) {
  if (filter->name)   Free(filter->name); filter->name=NULL;
  Free(filter); filter=NULL;
 }
}

void ClearFilter2(FILTER2 filter)

{ 
  if (filter == NULL) return;
  if (filter->name) Free(filter->name);
  filter->name=NULL;
  filter->size=0;
  filter->symmetry = 1.0;
}



/*******************************************************/
/* Read in a pair of filters from a file.              */
/* '#' is used for comments, '$' to end a filter       */
/* Blank lines are skipped, and so are leading blanks. */
/* Format:                                             */
/*                                                     */
/* H_size                                              */
/* H_shift                                             */
/* symmetry                                            */
/* coefficents of H                                    */
/* $                                                   */
/* G_size                                              */
/* G_shift                                             */
/* symmetry                                            */
/* coefficents of G                                    */
/* $                                                   */
/* K_size                                              */
/* K_shift                                             */
/* symmetry                                            */
/* coeff. of K                                         */
/* $                                                   */
/* factor ---- (H,K,G)->factor                         */
/* $                                                   */
/*******************************************************/

  
#define COMMENT '#'     /* Comments character      */
#define EO_FILTER '$'   /* End of filter character */

/* Macro to skip the comments in the filter files */
#define get_token \
do { \
 fscanf(fp,"%[^\n\r]\n\r",&input); \
 token = input; \
 while (token[0] == ' ') token++; \
} while (token[0] == '\n' || token[0] == '\r' || token[0] == COMMENT)
   
/* Associated variables */
static char input[STRSIZE],*token;


static int ReadFilter2File(FILE *fp,FILTER2 H,FILTER2 G,FILTER2 K)
{
 
  char *token; 
  int      j;

  get_token; 
 
  H->size = atoi(token);
  get_token;
  H->shift = atoi(token);
  get_token;
  H->symmetry = atof(token);

 

  for (j = 0; j < H->size; j++) {
    get_token;
    H->values[j] = atof(token);
    
  }

  /* Skip the extra coefficents. */
  for (; j < W2_FILT_SIZE; j++) {
    get_token;
    if (token[0] == EO_FILTER)
      break;
  }
   
  get_token;
  G->size = atoi(token);
  get_token;
  G->shift = atoi(token);
  get_token;
  G->symmetry = atof(token); 

 
  for (j = 0; j < G->size; j++) {
    get_token;
    G->values[j] = atof(token);
  }

  /* Skip the extra coefficents. */
  for (; j < W2_FILT_SIZE; j++) {
    get_token;
    if (token[0] == EO_FILTER)
      break;
  }

  get_token;
  K->size = atoi(token);
  get_token;
  K->shift = atoi(token);
  get_token;
  K->symmetry = atof(token); 

 
  for (j = 0; j < K->size; j++) {
    get_token;
    K->values[j] = atof(token);
  }

  /* Skip the extra coefficents. */
  for (; j < W2_FILT_SIZE; j++) {
    get_token;
    if (token[0] == EO_FILTER)
      break;
  }
 
  /* Read factors */
  j = 0;
  get_token;
  while (token[0] != EO_FILTER && j < W2_NFACT) 
    {
      H->factors[j]   = atof(token);
      K->factors[j]   = atof(token);
      G->factors[j++] = atof(token);
      get_token;
    }

  FClose(fp);
   return 1;

 
}


/**************************************************
 *
 * Dealing with the default filters
 *
 **************************************************/

static char defaultFilter2Name[40];

void SetDefaultFilter2Name(char *str)  
{
  if (strlen(defaultFilter2Name)>38) Errorf("SetDefaultFilter2Name() : Sorry filter name '%s' is too long",str);
  strcpy(defaultFilter2Name,str);
}

char  *GetDefaultFilter2Name(void)  
{
  return(defaultFilter2Name);
}

void C_DWt2f(char **argv)
     
{
  char *fname;
 
  argv= ParseArgv(argv,tSTR_,NULL,&fname,0);
 
  if (fname == NULL) {
    SetResultStr(GetDefaultFilter2Name());
    return;
  }
  
  SetDefaultFilter2Name(fname);
}

void SetDefaultFilter2(WTRANS2 wtrans2)  
{
  FILE *fp;
  char *FilterDir;
  char *fname;
  int res;
  
  if (wtrans2 == NULL) Errorf("SetDefaultFilter2() : Weird error");

  FilterDir = GetStrVariableLevel(levelFirst,"DWtrans2dFilterDirectory");

  fname = CharAlloc(strlen(FilterDir)+strlen(defaultFilter2Name)+5);
  TempPtr(fname);
  sprintf(fname,"%s/%s.1",FilterDir,defaultFilter2Name);

  if (!(fp = FOpen(fname, "r"))) Errorf("SetDefaultFilter2() : Sorry, unknown filter name '%s'.",defaultFilter2Name);

  res = ReadFilter2File(fp,wtrans2->filterh1,wtrans2->filterg1,wtrans2->filterk1);

  if (res) {
    if (wtrans2->filterh1->name != NULL) Free(wtrans2->filterh1->name);
    wtrans2->filterh1->name = CopyStr(fname);
    if (wtrans2->filterg1->name != NULL) Free(wtrans2->filterg1->name);  
    wtrans2->filterg1->name = CopyStr(fname);
    if (wtrans2->filterk1->name != NULL) Free(wtrans2->filterk1->name);
    wtrans2->filterk1->name = CopyStr(fname);
  } 
  else  Errorf("SetDefaultFilter2() : Sorry, do not know how to read filter file '%s'.",fname);
  
  sprintf(fname,"%s/%s.2",FilterDir,defaultFilter2Name);

  if (!(fp = FOpen(fname, "r"))) Errorf("SetDefaultFilter2() : Sorry, unknown filter name '%s'.",defaultFilter2Name);
  
  res=ReadFilter2File(fp,wtrans2->filterh2,wtrans2->filterg2,wtrans2->filterk2);
   
  if (res) {
    if (wtrans2->filterh2->name != NULL) Free(wtrans2->filterh2->name);
    wtrans2->filterh2->name = CopyStr(fname);
    if (wtrans2->filterg2->name != NULL) Free(wtrans2->filterg2->name);  
    wtrans2->filterg2->name = CopyStr(fname);
    if (wtrans2->filterk2->name != NULL) Free(wtrans2->filterk2->name);
    wtrans2->filterk2->name = CopyStr(fname);  
  }

  else  Errorf("SetDefaultFilter2() : Sorry, do not know how to read filter file '%s'.",fname);
}

void InitDFilter2(void)
{
  strcpy(defaultFilter2Name,"p3");
}
