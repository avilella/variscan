/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'extrema1d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1999-2002 Benjamin Audit, Emmanuel Bacry,             */
/*                         Jean Francois Muzy, Cedric Vaillant              */
/*      emails : muzy@crpp.u-bordeaux.fr                                    */
/*               audit@crpp.u-bordeaux.fr                                   */
/*               vaillant@crpp.u-bordeaux.fr                                */
/*               lastwave@cmap.polytechnique.fr                             */
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
/*    ext_chain.c       chain propagation                                   */
/*                                                                          */
/****************************************************************************/




#include "lastwave.h"
#include "extrema1d.h"



/**** Propagate the prevExtlis with extlis */

static void ChainExtlis(EXTREP extrep,EXTLIS prevExtlis, EXTLIS extlis,float delta)
{
  float dist,oldDist,distMin;
  EXT extTemp, ext;
  EXT extNearest;
  int flagNearest;


  /** If no coarser extrema then we don t do any thing **/
  if (extlis->size == 0) return;

  /** Loop on the extrema of prevExtlis **/
  extNearest = extlis->first;
  for (ext = prevExtlis->first; ext != NULL; ext = ext->next)
    {	
      /* if the extrema hasn't been chained then forget it! */
      if  (extrep->D[1][0] != prevExtlis && ext->finer == NULL)
         continue;

      oldDist = 9999999;
      dist = oldDist;
      distMin = 9999999; /* change function of delta */
      flagNearest = NO;
 
      /** Loop on the ext of prevExtlis: we are looking for the **/
      /** nearest coarser extremum. We begin to search not from    **/
      /** beginning of extlis but, from the nearest coarser **/
      /** extrema of the last extremum				   **/
      for(extTemp = extNearest;extTemp != NULL && oldDist >= dist;
 	      extTemp = extTemp->next) {	
    	oldDist = dist;
        dist = fabs(extTemp->abscissa - ext->abscissa);
        if (dist <= distMin) {
          flagNearest = YES;
		  distMin = dist;
		  extNearest = extTemp;
	    }
	  }
      /** If we have found a nearest extremum then we chain except on very special cases ... */
     if (flagNearest == YES) {
        /* We chain only if ... */
        if (extNearest->finer == NULL ||
            distMin < fabs(extNearest->abscissa-extNearest->finer->abscissa)) {
              if (extNearest->finer != NULL) extNearest->finer->coarser = NULL;
  	      ext->coarser = extNearest;
              extNearest->finer = ext;
	  }
          else /* We don't chain */
	    ext->coarser = NULL;
      }
      else
        ext->coarser = NULL;

     /** We go back a little bit **/
     if (extNearest->previous != NULL)
        extNearest = extNearest->previous;
     if (extNearest->previous != NULL)
        extNearest = extNearest->previous;
  }
}

void Chain(EXTREP extrep,float delta)
{
  int o,v;
  EXTLIS extlis,prevExtlis;
  EXT ext;

  /** We first delete an eventual former propagation ***/
  for (o=1;o<=extrep->nOct;o++) 
    for (v=0;v<extrep->nVoice;v++) {
      extlis = extrep->D[o][v];
      for (ext = extlis->first; ext != NULL;ext = ext->next) {
        ext->coarser = NULL;
	    ext->finer = NULL;
	  }
    }

  /** We propagate each extrema list on the next one **/
  prevExtlis = extrep->D[1][0];
  for (o=1;o<=extrep->nOct;o++) 
    for (v=0;v<extrep->nVoice;v++) {
      extlis = extrep->D[o][v];
      if (o==1 && v== 0) continue;
      ChainExtlis(extrep,prevExtlis, extlis,delta);
      prevExtlis = extlis;
    }
}


void C_Chain(char **argv)
{
  EXTREP extrep;
  float delta = 10;
  char opt;
  char flagDelete;
  
  argv = ParseArgv(argv,tEXTREP_,NULL,&extrep,-1);
  if (extrep == NULL) extrep = GetExtrepCur();
  
  CheckExtrep(extrep);

  flagDelete = YES;
  while(opt = ParseOption(&argv)) { 

   switch(opt) {
    case 'D':
      argv = ParseArgv(argv,tFLOAT,&delta,-1);
      break;
    case 'd':
     flagDelete = NO;
     break;
    default:
     ErrorOption(opt);
   }
 }    
 NoMoreArgs(argv);

 Chain(extrep,delta);
 if (flagDelete) ChainDelete(extrep);
 
 SetResultInt(extrep->D[1][0]->size);
}



/******************************************************************/
/*** Delete extrema which are not part of a chain **/
/******************************************************************/

int ChainDelete(EXTREP extrep)
{
  EXTLIS extlis;
  EXT ext,ext1;
  int nb = 0;
        int o,v;
        
        for(o=1;o<=extrep->nOct;o++)
                for(v=0;v<extrep->nVoice;v++) {
                        extlis = extrep->D[o][v];
                        if (o==1 && v==0) continue;
      ext = extlis->first;
      while (ext) {
        if (ext->finer == NULL || ext->ordinate*ext->finer->ordinate < 0) {
          if (ext->finer != NULL) {
            ext->finer->coarser = NULL;
            ext->finer = NULL;
          }
          ext1 = ext->next;
          RemoveDeleteChain(ext);
          nb++;
          ext = ext1;
          }
        else 
          ext = ext->next;
      }
    }
  return(nb);
}




void C_ChainDelete(char **argv)
{
  EXTREP extrep;
  
  argv = ParseArgv(argv,tEXTREP_,NULL,&extrep,0);
  if (extrep == NULL) extrep = GetExtrepCur();
 
  CheckExtrep(extrep);

 SetResultInt(ChainDelete(extrep));
}


/***************************************************/
/** Replace all the extrema  ordinates by the max **/
/** ordinate on the extrema line (finer scale)    **/
/** It takes into account a factor for computing  **/
/** the max value                                 **/
/***************************************************/



void ChainMax(EXTREP extrep,double expo)
{
  EXTLIS extlisFinest;
  EXT extFinest, ext;
  float maxValue, maxValueN, newMaxValueN;

  extlisFinest = extrep->D[1][0];
  if(extlisFinest->size == 0)
    Errorf("No extrema on the finest scale!");

  for(extFinest = extlisFinest->first; extFinest != NULL;
      extFinest = extFinest->next)

     {
        maxValue = -1;
        maxValueN = -1;

        for(ext = extFinest; ext != NULL; ext = ext->coarser) {
   
           newMaxValueN = fabs(ext->ordinate * 
           		extrep->aMin*pow(2.0,expo*ext->scale/extrep->nVoice));
           if (newMaxValueN < maxValueN)
	      			ext->ordinate = maxValue;
	   			 else {
	      			maxValue = ext->ordinate;
              maxValueN = newMaxValueN;
	      	 }
        }
     }
}

void C_ChainMax(char **argv)
{
  EXTREP extrep;
  float expo;

  argv = ParseArgv(argv,tEXTREP_,NULL,&extrep,tFLOAT_,0.,&expo,0);
  if (extrep == NULL) extrep = GetExtrepCur();
 
  CheckExtrep(extrep);

  ChainMax(extrep,(double) expo);
}


/*********************************/
/* Convert an extlis in a signal */
/*********************************/

void ExtlisToSig (EXTREP extrep,EXTLIS extlis,SIGNAL sig)
{
  int i;
  EXT ext;
  
  SizeSignal(sig,extlis->size,XYSIG);
  
  for(i=0,ext = extlis->first;ext != NULL;ext = ext->next,i++) 
    { 
      sig->X[i] = ext->abscissa /*extrep->dx+extrep->x0*/;
      sig->Y[i] = ext->ordinate;
    }
}

void C_ExtlisToSig (char **argv)
{
  EXT ext;
  EXTLIS extlis;
  EXTREP extrep;
  SIGNAL sig;
 
  argv = ParseArgv(argv,tEXT,&ext,tSIGNAL,&sig,0);
  
  extlis = ext->extlis;
  extrep = extlis->extrep;
  ExtlisToSig(extrep,extlis,sig);
}

