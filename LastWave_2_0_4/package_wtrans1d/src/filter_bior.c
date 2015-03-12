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
/*  filter_orth_bior.c   Functions to deal with the [bi]ortho filters       */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "wtrans1d.h"


/**************************************/
/* The Default (bi)orthogonal Filters */
/**************************************/

static char defaultFGName[] = "D1.o";
static FILTERGROUP defaultFG= NULL;


/***************************************/
/* Computes the quadrature filter G    */
/* associates to the low-pass filter H */
/***************************************/

static void QuadratureFilter(FILTER H, FILTER G)
{
  int Gleft,Gright;
  int k , kh, kg;

  /* Allocation of G */
  G->size = H->size;
  if (G->Y != NULL) Free(G->Y);
  G->Y = FloatAlloc(G->size);
  
  /* Let's do it */
  Gleft = 1 - RIGHT_FILT(H);
  Gright = 1 - LEFT_FILT(H);
  kg = -Gleft;
  G->shift= -(Gleft + Gright)/2;
  kh = ZERO(H);

  for(k = Gleft ; k <= Gright; k++) {
    if( k % 2) G->Y[k+kg] = H->Y[1-k+kh];
    else G->Y[k+kg] = -H->Y[1-k+kh];
  }
}

  
/***************************************/
/*      Conjugates a filter H          */
/***************************************/

static void ConjugateFilter(FILTER H)
{
  int size,k;
  float *temp = FloatAlloc(H->size);

  H->shift = - H->shift + (1 - H->size % 2);
  size = H->size;

  for(k = 0; k < size; k++)
    temp[k] = H->Y[size-1-k];
    
  for(k = 0; k < size; k++)
    H->Y[k] = temp[k];
  
  Free(temp);
}

/***********************************************************/
/* Builds all the filters that were not read from the file */
/***********************************************************/

static void BuildBiorFG(FILTERGROUP fg)
{
  QuadratureFilter(fg->H1,fg->G2);
  QuadratureFilter(fg->H2,fg->G1);
  ConjugateFilter(fg->H1);
  ConjugateFilter(fg->G1);
}

static void BuildOrthFG(FILTERGROUP fg)
{
  CopyFilter(fg->H1,fg->H2);
  BuildBiorFG(fg);
}


/***************************************************/
/*       Set a bior filter in the wtrans           */
/*       and change the default filter             */
/***************************************************/




/* for now on, this function is always called with s == 0 */
static void SetFracBiorFG(char *filename,WTRANS wtrans,float s)
{
  extern void BuildFracFilter(FILTERGROUP,float);	

  char file[250];
  char *dir;

  if (filename != NULL) {
    DeleteFilterGroup(defaultFG);
    strcpy(defaultFGName,filename);
  }
    
  if (filename != NULL || defaultFG == NULL) {
    /* We read the default filter */
    dir = GetStrVariableLevel(levelFirst,"Wtrans1dFilterDirectory");
    if (dir == NULL) Errorf("SetBiorFilters() : The Wtrans1dFilterDirectory variable is missing");
    sprintf(file,"%s/%s",dir,defaultFGName);
    Printf("Read [bi]orthogonal filter in '%s'...\n",file);
    defaultFG = NewFilterGroup();
    FilterGroupRead(file,defaultFG,F_BIOR);

    if (s != 0) {
	if (defaultFG->type != F_ORTH) {
	    BuildBiorFG(defaultFG);
	    Errorf("SetBiorFilters() : Fractional filters can be built only from orthogonal filters");
	}
/*	BuildFracFilter(defaultFG,s); ??*/
    }

    if (defaultFG->type == F_ORTH) BuildOrthFG(defaultFG);
    if (defaultFG->type == F_BIOR || defaultFG->type == F_BIOR_NO_SYM) BuildBiorFG(defaultFG); 
  }

  SetFGWtrans(wtrans,defaultFG);  
}

void SetBiorFG(char *filename,WTRANS wtrans)
{
  SetFracBiorFG(filename,wtrans,0);
}		

/**************************************************/
/* Command to change and load the default filters */
/**************************************************/

void C_OWtf(char **argv)
{
  char *filename;
  WTRANS wtrans;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tSTR,&filename,0);
  
  if (wtrans == NULL) wtrans = GetWtransCur();
    
  SetBiorFG(filename,wtrans);
}


/****************************/
/* Print a filter structure */
/****************************/

void PrintBiorFilter(FILTER filter)
{
  int Fleft,Fright;
  int n , n0;

  Fleft=LEFT_FILT(filter);
  Fright=RIGHT_FILT(filter); 
  n0 = ZERO(filter);

  Printf("size = %d, shift = %d\n",filter->size,filter->shift);

  for(n = Fleft; n <= Fright ;n++)
    Printf("  n = %2d, F[n] = %.8g\n",n , filter->Y[n+n0]);
}
