/*..........................................................................*/
/*                                                                          */
/*  L a s t W a v e    P a c k a g e 'wtmm1d' 1.1.1                         */
/*                                                                          */
/*      Copyright (C) 2000 Benjamin Audit.                                  */
/*      emails : audit@crpp.u-bordeaux.fr                                   */
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

#include "wtmm1d.h"

static void ExtlisToTabFloat(EXTLIS extlis,float *t)
{
  int i;
  EXT ext;
  
  if(extlis->size <= 0)
    Errorf("ExtlisToTabFloat(): extlis->size <= 0 !");
  
  for(i=0,ext = extlis->first;ext != NULL;ext = ext->next,i++) 
    t[i] = ext->ordinate;
  
  return;
}

int ComputePartFuncOnExtrep(LWPARTFUNC lwpf,EXTREP extrep,int qNumber,double *qArray)
{
  EXTLIS extlis;
  int res;
  int o,v;
  float *t = NULL;
  int tSize = 0;
  char method[PFMETHODSIZE+1];

  CheckExtrep(extrep);
  
  if( (strlen(extrep->wName)+strlen("_max")) > PFMETHODSIZE)
    Errorf("extrep->wName is too long");

  strcpy(method,extrep->wName);
  strcat(method,"_max");
  
  res = PFInit(lwpf->pf,method,(double) extrep->aMin,extrep->nOct,extrep->nVoice,extrep->size,1,qNumber,qArray);
  switch(res)
    {
    case PFYes:
      break;
    case PFNotValid:
      Errorf("One of the parameter isn\'t valid");
      break;
    default:
      Errorf("serious error. (maybe one of the pointer lwpf->pf or extrep->wName was NULL)");
    }
  
  for(o=1;o <= extrep->nOct;o++)
    for(v=0;v < extrep->nVoice;v++)
      {
	extlis = extrep->D[o][v];
	
	if(extlis->size < 0)
	  Errorf("ComputePartFuncOnExtrep(): extlis->size < 0 !!!!!!!");
	else if(extlis->size == 0)
	  return (o-1)*extrep->nVoice + v;
	else if(extlis->size > tSize)
	  {
	    if(t != NULL) Free(t);
	    t = FloatAlloc(extlis->size);
	    tSize = extlis->size;
	  }
	      
	ExtlisToTabFloat(extlis,t);
	
	res = PFComputeOneScaleF(lwpf->pf,(o-1)*extrep->nVoice + v,t,extlis->size);
	switch(res)
	  {
	  case PFYes:
	    break;
	  case PFNotValid:
	    Errorf("ComputePartFuncOnExtrep(): One of the parameter isn\'t valid");
	    break;
	  default:
	    Errorf("ComputePartFuncOnExtrep(): serious error. (maybe the lwpf->pf pointer was NULL)");
	  }
      }
  
  if(t != NULL) Free(t);
  return (o-2)*extrep->nVoice + v;
}

int ComputePartFuncOnWtrans(LWPARTFUNC lwpf,WTRANS wtrans,int qNumber,double *qArray,int flagCausal)
{
  SIGNAL signal;
  int res;
  int o,v;
  int sizeUseful;

  CheckWtrans(wtrans);

  res = PFInit(lwpf->pf,wtrans->wName,(double) wtrans->aMin,wtrans->nOct,wtrans->nVoice,wtrans->A[0][0]->size,1,qNumber,qArray);
  switch(res)
    {
    case PFYes:
      break;
    case PFNotValid:
      Errorf("One of the parameter isn\'t valid");
      break;
    default:
      Errorf("serious error. (maybe one of the pointer lwpf->pf or wtrans->wName was NULL)");
    }
  
  for(o=1;o <= wtrans->nOct;o++)
    for(v=0;v < wtrans->nVoice;v++)
      {
	signal = wtrans->D[o][v];
	
	if(signal->size <= 0)
	  Errorf("ComputePartFuncOnWtrans(): signal->size <= 0 !!!!!!!");
	
	if(flagCausal == YES)
	  {
	    sizeUseful = signal->lastp - signal->firstp + 1;
	    if(sizeUseful <= 0)
	      return (o-1)*wtrans->nVoice + v;
	    
	    res = PFComputeOneScaleF(lwpf->pf,(o-1)*wtrans->nVoice + v,(signal->Y + signal->firstp),sizeUseful);
	  }
	else if(flagCausal == NO)
	  {
	    res = PFComputeOneScaleF(lwpf->pf,(o-1)*wtrans->nVoice + v,signal->Y,signal->size);
	  }
	else Errorf("ComputePartFuncOnWtrans(): Unexpected value for flagCausal");
	
	switch(res)
	  {
	  case PFYes:
	    break;
	  case PFNotValid:
	    Errorf("ComputePartFuncOnWtrans(): One of the parameter isn\'t valid");
	    break;
	  default:
	    Errorf("ComputePartFuncOnWtrans(): serious error. (maybe the lwpf->pf pointer was NULL)");
	  }
      }
  
  return (o-2)*wtrans->nVoice + v;
}



