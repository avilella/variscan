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
/*  filter_dyad.c    Functions to deal with dyadic wavelet                  */
/*                   transform filters                                      */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "wtrans1d.h"




/**************************************/
/* The Default dyadic Filters         */
/**************************************/

static char defaultFGName[] = "p1.d";
static FILTERGROUP defaultFG= NULL;


/***********************************************************/
/* Builds all the filters that were not read from the file */
/***********************************************************/

/* Everything has been read from the file !! */
static void BuildDyadFG(FILTERGROUP fg)
{
  CopyFilter(fg->H1,fg->H2);
  CopyFilter(fg->G1,fg->G2);
}

/***************************************************/
/*       Set a dyad filter in the wtrans           */
/*       and change the default filter             */
/***************************************************/

void SetDyadFG(char *filename,WTRANS wtrans)
{
  char file[250];
  char *dir;

  if (filename != NULL) {
    DeleteFilterGroup(defaultFG);
    strcpy(defaultFGName,filename);
  }
    
  if (filename != NULL || defaultFG == NULL) {
    /* We read the default filter */
    dir = GetStrVariableLevel(levelFirst,"Wtrans1dFilterDirectory");
    if (dir == NULL) Errorf("SetDyadFG() : The Wtrans1dFilterDirectory variable is missing");
    sprintf(file,"%s/%s",dir,defaultFGName);
    defaultFG = NewFilterGroup();
    Printf("Read dyadic filter in '%s'...\n",file);
    FilterGroupRead(file,defaultFG,F_DYAD);

    BuildDyadFG(defaultFG);	
  }	

  SetFGWtrans(wtrans,defaultFG);
}


/**************************************************/
/* Command to change and load the default filters */
/**************************************************/

void C_DWtf(char **argv)
{
  char *filename;
  WTRANS wtrans;
  
  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tSTR,&filename,0);
  
  if (wtrans == NULL) wtrans = GetWtransCur();
  
  SetDyadFG(filename,wtrans);
}


/****************************/
/* Print a filter structure */
/****************************/

void PrintDyadFilter(FILTER filter)
{ 
/* ??????? 
  int Fleft,Fright;
  int n , n0;

  Fleft=LEFT_FILT(filter);
  Fright=RIGHT_FILT(filter); 
  n0 = ZERO(filter);

  Printf("size = %d, shift = %d\n",filter->size,filter->shift);

  for(n = Fleft; n <= Fright ;n++)
    Printf("  n = %2d, F[n] = %.8g\n",n , filter->Y[n+n0]);
*/
}

