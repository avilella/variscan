
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
/*  ext_compute.c    Functions which allow the computation                  */
/*                   of the extrema of a wavelet transform                  */
/*                                                                          */
/****************************************************************************/




#include "lastwave.h"
#include "extrema1d.h"


#define EPSILON 1e-6

/* It returns the index of the first point outside the plateau */
/* min amd max are set to the MIN and the MAX on the plateau*/

static int Plateau(float *Y,int size,int ideb,float *min,float *max,float eps)

{
  int i;

  *min = Y[ideb];
  *max = Y[ideb];

  for (i=ideb;i<size;i++) 
    { 
      if (Y[i]>*max)
        {
          if ((Y[i]-*min)>eps) return i;
          else *max = Y[i];
        }
      else if (Y[i]<*min) 
        {
          if ((*max - Y[i])>eps) return i;
          else *min = Y[i];
        }
    }
  return i;
}



/*************************************************************/
/* It tests whether the value 'valueI' is an extremum or not */
/* knowing the previous value ('prevValueI') and the next    */
/* value ('nextValueI').                                     */
/* It returns YES or NO                                      */
/*************************************************************/


static int PlateauIsMaxima(float prevValueI,float valueI,float nextValueI,float min,float max,float threshold)
{
  int derI,derNextI;
  
  derI =  SIGN(valueI - prevValueI);
  derNextI = SIGN(nextValueI - valueI);
  /* If the plateau is around zero it can't be an extremum*/
  if (min * max <= 0) return (NO);
  else {
    if ((derI != derNextI) &&
        ((((derI ==  1) || (derNextI == -1)) && (valueI > 0)) ||
         (((derI == -1) || (derNextI ==  1)) && (valueI < 0))))
      {
        if (derI != 0 && derNextI != 0) {
          return(YES);
        }
      }
    return(NO);
  }
}



/***************************************************/
/* Compute the extrema of the signal 'signal'      */
/* which corresponds to a detail at scale 'scale'  */
/* and put them in 'extlis'                        */
/* If flagInterpol is YES then we interpolate      */
/*    to compute the extrema                       */
/* If flagCausal is YES, it computes only the      */
/*    extrema not affected by border effect        */
/* It returns the number of extrema detected       */
/***************************************************/


static int ComputeExtlis(WTRANS wtrans,SIGNAL signal,EXTLIS extlis,int scale,int flagCausal,int flagInterpol,float
threshold)
{
  EXT ext;
  int i,ifin,iext;
  float a,b,c, valueI, prevValueI, nextValueI, nextNextValueI;
  float min,max;
  int firstI, lastI;
  int firstExt;
  
  if (flagCausal == NO){
    firstI = 1;
    lastI = signal->size - 1;
  }
  else {
    firstI = signal->firstp+1;
    lastI = signal->lastp;
  }
  
  firstExt = YES;
  i=firstI;
  
  while (i < lastI-2) {
    
    ifin = Plateau(signal->Y,signal->size,i,&min,&max,threshold);
    iext = (i + ifin - 1 )/2; 
    prevValueI = signal->Y[i-1];
    if (max>=0) valueI=max;
    else valueI = min;
    nextValueI = signal->Y[ifin];
    nextNextValueI = signal->Y[ifin+1];
    
    
    if (PlateauIsMaxima(prevValueI,valueI,nextValueI,min,max,threshold) == YES) {
      
      if(firstExt == YES) { 
        ext = NewExt();
        extlis->first = ext;
        ext->extlis = extlis;
        firstExt = NO;
      }
      else {
        ext->next = NewExt();
        ext->extlis = ext->next->extlis = extlis;
        ext->next->previous = ext;
        ext = ext->next;
      }
      extlis->size++;
      ext->scale = scale;
      
      
      /* We compute the position and value of the extremum depending
         on the value of 'flagInterpol' */
      a = (nextValueI + prevValueI)/2. - valueI;
      if (flagInterpol == YES)  {
        b = (nextValueI - prevValueI)/2.;
        c = valueI;
        if (a == 0) {
          ext->abscissa = iext;
          ext->ordinate = valueI;
        }
        else {
          ext->abscissa = (-b/(2*a)+i);
          ext->ordinate = c-b*b/(4*a);
        }
        if (ext->abscissa <= i-1 || ext->abscissa >= ifin) {
          ext->abscissa = iext;
          ext->ordinate = valueI;
        }
      }
      /* No interpolation */
      else {
        ext->abscissa = iext;
        ext->ordinate = valueI;
      }
      
      ext->index =  iext;
      ext->abscissa *= signal->dx;    
      ext->abscissa += signal->x0;
      
    }
    
    i=ifin;
  }
  
  if (extlis->size != 0) {
    extlis->end = ext;
    extlis->end->next = NULL;
  }
  else extlis->end = extlis->first = NULL;
  
  
  return(extlis->size);
}






/**************************************************************/
/* This function computes the extrema representation of       */
/* the wavelet transform 'wtrans'.                            */
/* For description of the different flags, see function above */
/* It returns the number of extrema detected                  */
/**************************************************************/

void InitExtrep(WTRANS wtrans,EXTREP extrep)
{ 
  ClearExtrep(extrep);

  extrep->dx = wtrans->dx;
  extrep->x0 = wtrans->x0;
  extrep->size = wtrans->size ;

  extrep->nVoice = wtrans->nVoice;
  extrep->nOct = wtrans->nOct;
  extrep->aMin = wtrans->aMin;
  extrep->exponent = wtrans->exponent;
  
  if (wtrans->wName != NULL) extrep->wName = CopyStr(wtrans->wName);
}

int ComputeExtOctVoice(WTRANS wtrans,EXTREP extrep,int flagCausal,int flagInterpol,float epsilon,int o,int v,float *pThreshold)
{
  int nb;
  float NL2;
  int i;
  
  
  /* Computation of the threshold */
  
  NL2 =0;
  
  for (i=wtrans->D[o][v]->firstp;i<=wtrans->D[o][v]->lastp;i++)
    {
      /* L2 norm of the wavelet coef. at scale o,v*/
      NL2=NL2+wtrans->D[o][v]->Y[i]*wtrans->D[o][v]->Y[i];
    }
  *pThreshold =sqrt(NL2)*epsilon;
   
   /* Extrema Computation*/
   
   nb = ComputeExtlis(wtrans,wtrans->D[o][v],
                                 extrep->D[o][v],
                         v+(o-1)*wtrans->nVoice,
                         flagCausal,
                         flagInterpol,
                         *pThreshold);
   
   return(nb);
}
                          
int ComputeExtrep(WTRANS wtrans,EXTREP extrep,int flagCausal,int flagInterpol,float epsilon)
{
  int i,j;
  int nb = 0;
  float threshold;

  /* Initialization of the extrep */
  InitExtrep(wtrans,extrep);
  
  /* Computation */
  for (i=1;i<=wtrans->nOct;i++) 
    for (j=0;j<wtrans->nVoice;j++) {
      nb += ComputeExtOctVoice(wtrans,extrep,flagCausal,flagInterpol,epsilon,i,j,&threshold);
    }
  return(nb);
}

/* 'extrema' command */
void C_ComputeExtrep(char **argv)
{
  WTRANS wtrans;
  EXTREP extrep;
  int flagInterpol;
  int flagCausal,flagChain;
  int nb;
  float epsilon = EPSILON;
  char opt;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,tEXTREP_,NULL,&extrep,-1);

  if (wtrans == NULL) {
    if (extrep != NULL) ErrorUsage();
    wtrans = GetWtransCur();
  }
  if (extrep == NULL) {
    extrep = wtrans->extrep;
    if (extrep == NULL) ErrorUsage();
  }

  CheckWtrans(wtrans);
  
  flagInterpol = YES;       /* options initialization */
  flagCausal = NO;
  flagChain = YES;
  
  while(opt = ParseOption(&argv)) { 

   switch(opt) {
    case 'i':
      flagInterpol = NO;
      break;
    case 'c':
      flagCausal = YES;
      break;
    case 'C':
      flagChain = NO;
      break;
    case 'e':
      argv = ParseArgv(argv,tFLOAT,&epsilon,-1);
      if (epsilon<=0) ErrorUsage1();
      break;
    default:
     ErrorOption(opt);
   }
 }    
 NoMoreArgs(argv);

  nb = ComputeExtrep(wtrans,extrep, flagCausal,flagInterpol,epsilon);

  if (flagChain) {
    Chain(extrep,10);
    ChainDelete(extrep);
  }

  SetResultInt(nb);
}




