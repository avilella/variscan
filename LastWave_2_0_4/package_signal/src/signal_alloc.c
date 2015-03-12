/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0.2                     */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
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
/*  signal_alloc.c   Functions which deal with the dynamical                */
/*                   allocation of memory for SIGNAL's                      */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "signals.h"
#include "int_fsilist.h"



/* Default name for a signal */
static char defaultName[] = "";

/* Types of a signal */
char *signalType = "&signal";
char *signaliType = "&signali";


/*
 * Print a signal when it is a result
 */

#define YSigPrintLength 6
#define XYSigPrintLength 3
 
void PrintSignal(SIGNAL signal)
{
  int n;
  
  if (signal->size == 0) {
    Printf("<size=%d>",signal->size);
    return;
  }
  for (n = 0; n < signal->size; n++) {
    Printf("% 7g  %g\n", XSig(signal,n),signal->Y[n]);
  }
}

/*
 * String conversion
 */

char *ToStrSignal(SIGNAL signal, char flagShort)
{
  static char strShort[50];
  int n, nAlloc;
  char *str;
  
  if (flagShort) {
    if (signal->type == YSIG) sprintf(strShort,"<size=%d>",signal->size);
    else sprintf(strShort,"<XY,size=%d>",signal->size);
    return(strShort);
  }

  if (signal->size == 0) {
    sprintf(strShort,"<size=%d>",signal->size);
    return(strShort);
  }

  nAlloc = 300;
  str = CharAlloc(nAlloc);
  TempPtr(str);
  if (signal->type == YSIG) {
    if (signal->size < YSigPrintLength) sprintf(str,"<");
    else {
      sprintf(str,"<size=%d",signal->size);
      if (signal->dx == 1 && signal->x0 == 0) strcat(str,";");
      else strcat(str,",");
    }
    if (signal->dx != 1 || signal->x0 != 0) {
      sprintf(strShort,"x0=%g,dx=%g;",signal->x0,signal->dx);
      strcat(str,strShort);
    }
    for (n=0;n<MIN(YSigPrintLength,signal->size);n++) {
      if (strlen(str) > nAlloc-40) {
        nAlloc+=300;
        str = CharAlloc(nAlloc);
        TempPtr(str);
      }
      sprintf(strShort,"%g",signal->Y[n]);
      strcat(str,strShort);
      if (n!=MIN(YSigPrintLength,signal->size)-1) strcat(str,",");
    }
    if (signal->size > YSigPrintLength) strcat(str,",...>");
    else strcat(str,">");
    return(str);
  }
  else {
    if (signal->size < XYSigPrintLength) sprintf(str,"<");
    else sprintf(str,"<size=%d;",signal->size);
    for (n=0;n<MIN(XYSigPrintLength,signal->size);n++) {
      if (strlen(str) > nAlloc-40) {
        nAlloc+=300;
        str = CharAlloc(nAlloc);
        TempPtr(str);
      }
      sprintf(strShort,"(%g/%g)",signal->X[n],signal->Y[n]);
      strcat(str,strShort);
      if (n!=MIN(XYSigPrintLength,signal->size)-1) strcat(str,",");
    }
    if (signal->size > XYSigPrintLength) strcat(str,",...>");
    else strcat(str,">");
    return(str);
  }
}

/*
 * Print the info of a signal
 */
void PrintInfoSignal(SIGNAL signal)
{
  if (signal->type == XYSIG) Printf("   XY Signal\n");
  else  Printf("   Y Signal\n");

  Printf("   size    :  %d  [%dx %dy]\n",signal->size,signal->sizeMallocX,signal->sizeMallocY);
  
  if (signal->type == YSIG) {
    Printf("   x0      :  %g \n",signal->x0);
    Printf("   dx      :  %g \n",signal->dx);
  }

  Printf("   firstp  :  %d (%g)\n",signal->firstp,XSig(signal,signal->firstp));
  Printf("   lastp   :  %d (%g)\n",signal->lastp,XSig(signal,signal->lastp));
}


/*
 * The different extract options 
 */
static char *extractOptionsSig[] = {"*nolimit","*bperiodic","*bmirror","*bmirror1","*bconst","*b0","*x","*xlin",NULL};
static char *indexextractOptions[] = {"*nolimit", NULL};
enum {
  FSIOptSigNoLimit = FSIOption1,
  FSIOptSigBPer = FSIOption2,
  FSIOptSigBMir = FSIOption3,
  FSIOptSigBMir1 = FSIOption4,
  FSIOptSigBCon = FSIOption5,
  FSIOptSigB0 = FSIOption6,
  FSIOptSigX = FSIOption7,
  FSIOptSigXLin = FSIOption8
};


/* 
 * Set a signal using explicit parameters
 */
char *SetSignalField_(SIGNAL sigLeft,char *field, FSIList *fsiList, float fltRight, VALUE value,char *equal, char *fieldName)
{
  FSI_DECL;
  int i,j,k,size,_j,_iold;
  char *type;
  SIGNAL sigRight,sig;
  LISTV lv;
  float *arrayLeft;
  char flagY;
  RANGE rg;
  
  /* Get the type of the right value  */
  if (value == NULL) type = numType;
  else {
    value = ValueOf(value);
    type = GetTypeValue(value);
  }

  if (type == signaliType && field != NULL && !strcmp("X",field) && fsiList == NULL) {
    sigRight = CastValue(value,SIGNAL);
    if (sigLeft->size != sigRight->size) {
      SetErrorf("Size on both sides should be the same (left size = %d and right size = %d)",sigLeft->size,sigRight->size);
      return(NULL);
    }
    if (sigLeft->type == YSIG) SizeSignal(sigLeft,sigLeft->size,XYSIG);
    for (i=0;i<sigLeft->size;i++) sigLeft->X[i] = sigRight->Y[i];
    return(signaliType);
  }
  
  /* Conversion of range to signal */
  if (type == rangeType && (field == NULL || strcmp("X",field) || fsiList != NULL)) {
    rg = CastValue(value,RANGE);
    sig = TNewSignal();
    SizeSignal(sig,rg->size,YSIG);
    sig->Y[0] = rg->first;
    for (i=1;i<rg->size;i++) sig->Y[i] = sig->Y[i-1]+rg->step;
    value = (VALUE) sig;
    type = signaliType;
  }
  
  /* Case s.X = range */
  if (type == rangeType) {
    rg = CastValue(value,RANGE);
    if (rg->size != sigLeft->size) {
      SetErrorf("Size on both sides should be the same (left size = %d and right size = %d)",sigLeft->size,rg->size);
      return(NULL);
    }
    if (rg->step <= 0) {
      SetErrorf("Step of right handside range should be strictly positive");
      return(NULL);
    }
    rg = CastValue(value,RANGE);
    if (sigLeft->type == XYSIG) sigLeft->type = YSIG;
    sigLeft->dx = rg->step;
    sigLeft->x0 = rg->first;
    return(signaliType);
  }
  
  if (type != numType && type != signaliType && type != signalType && type != listvType) {
    SetErrorf("Expect a signal (and not a '%s') on right handside",type);
    return(NULL);
  }

  if (type == numType) {
    if (value != NULL) fltRight = CastValue(value,NUMVALUE)->f;
    sigRight = NULL;
  }
  else if (type == signaliType || type == signalType) {
    sigRight = CastValue(value,SIGNAL);
  }
  else lv = CastValue(value,LISTV);

  
  /*************************
   *
   * Case value operator is *= += -= /= ^= %=
   *
   *************************/
   
  if (*equal != '=' && *equal != ':') {

    if (type == listvType) {
      SetErrorf("Expect a signal or a number (and not a '%s') on right handside",type);
      return(NULL);
    }

    if (sigLeft->size  == 0) {
      SetErrorf("Expect a non empty signal on the left handside");
      return(NULL);
    }
    
    if (sigRight && (fsiList && sigRight->size != fsiList->nx || !fsiList && sigRight->size != sigLeft->size)) {
      SetErrorf("Signals should have the same size");
      return(NULL);
    }

    if (field != NULL && !strcmp(field,"X")) {
      if (sigLeft->type == YSIG) {
        SetErrorf("Sorry, this functionality (for YSIG) is not implemented yet");
        return(NULL);
      }
      arrayLeft = sigLeft->X;
    }
    else arrayLeft = sigLeft->Y;
    
    switch (*equal) {
    case '+' :  
      if (fsiList == NULL || fsiList->nx == 0) {
        if (sigRight == NULL) for (i=0;i<sigLeft->size;i++) arrayLeft[i]+=fltRight;
        else for (i=0;i<sigLeft->size;i++) arrayLeft[i]+=sigRight->Y[i];
      }
      else {
        if (sigRight == NULL) {FSI_FOR_START(fsiList);arrayLeft[_i]+=fltRight;FSI_FOR_END;}
        else {FSI_FOR_START(fsiList); arrayLeft[_i]+=sigRight->Y[_k];FSI_FOR_END}
      }      
      break; 
    case '-' :  
      if (fsiList == NULL || fsiList->nx == 0) {
        if (sigRight == NULL) for (i=0;i<sigLeft->size;i++) arrayLeft[i]-=fltRight;
        else for (i=0;i<sigLeft->size;i++) arrayLeft[i]-=sigRight->Y[i];
      }
      else {
        if (sigRight == NULL) {FSI_FOR_START(fsiList);arrayLeft[_i]-=fltRight;FSI_FOR_END;}
        else {FSI_FOR_START(fsiList); arrayLeft[_i]-=sigRight->Y[_k];FSI_FOR_END}
      }      
      break; 
    case '*' :  
      if (fsiList == NULL || fsiList->nx == 0) {
        if (sigRight == NULL) for (i=0;i<sigLeft->size;i++) arrayLeft[i]*=fltRight;
        else for (i=0;i<sigLeft->size;i++) arrayLeft[i]*=sigRight->Y[i];
      }
      else {
        if (sigRight == NULL) {FSI_FOR_START(fsiList);arrayLeft[_i]*=fltRight;FSI_FOR_END;}
        else {FSI_FOR_START(fsiList); arrayLeft[_i]*=sigRight->Y[_k];FSI_FOR_END}
      }      
      break; 
    case '^' :  
      if (fsiList == NULL || fsiList->nx == 0) {
        if (sigRight == NULL) for (i=0;i<sigLeft->size;i++) arrayLeft[i]=pow(fabs(arrayLeft[i]),fltRight);
        else for (i=0;i<sigLeft->size;i++) arrayLeft[i]=pow(fabs(arrayLeft[i]),sigRight->Y[i]);
      }
      else {
        if (sigRight == NULL) {FSI_FOR_START(fsiList);arrayLeft[_i]=pow(fabs(arrayLeft[_i]),fltRight);FSI_FOR_END;}
        else {FSI_FOR_START(fsiList); arrayLeft[_i]=pow(fabs(arrayLeft[_i]),sigRight->Y[_k]);FSI_FOR_END}
      }      
      break; 
    case '%' :  
      if (fsiList == NULL || fsiList->nx == 0) {
        if (sigRight == NULL) for (i=0;i<sigLeft->size;i++) arrayLeft[i]=((int) arrayLeft[i]) % ((int) fltRight);
        else for (i=0;i<sigLeft->size;i++) arrayLeft[i]=((int) arrayLeft[i]) % ((int) sigRight->Y[i]);
      }
      else {
        if (sigRight == NULL) {FSI_FOR_START(fsiList);arrayLeft[_i]=((int) arrayLeft[_i]) % ((int) fltRight);FSI_FOR_END;}
        else {FSI_FOR_START(fsiList); arrayLeft[_i]=((int) arrayLeft[_i]) % ((int) sigRight->Y[_k]);FSI_FOR_END}
      }      
      break; 
    case '/' :  
      if (sigRight == NULL && fltRight == 0) {      
        SetErrorf("Division by 0");
        return(NULL);
      }
      if (fsiList == NULL || fsiList->nx == 0) {
        if (sigRight == NULL) for (i=0;i<sigLeft->size;i++) arrayLeft[i]/=fltRight;
        else for (i=0;i<sigLeft->size;i++) {
          if (sigRight->Y[i] == 0) {     
            SetErrorf("Division by 0");
            return(NULL);
          }
          arrayLeft[i]/=sigRight->Y[i];
        }
      }
      else {
        if (sigRight == NULL) {FSI_FOR_START(fsiList);arrayLeft[_i]/=fltRight;FSI_FOR_END;}
        else {
          FSI_FOR_START(fsiList); 
          if (sigRight->Y[_k] == 0) {     
            SetErrorf("Division by 0");
            return(NULL);
          }
          arrayLeft[_i]/=sigRight->Y[_k];
          FSI_FOR_END;
        }
      }    
      break; 
    default :   break;
  	}
  	return(signaliType);
  }


  /*************************
   *
   * Case s[rang1...rangeN] = sig or float
   *
   *************************/

  if (*equal == '=' && type != listvType) {
    /* Case of s=sigRight */
    if (fsiList == NULL) {
      if (sigRight) CopySig(sigRight,sigLeft);
      else {
        SizeSignal(sigLeft,1,YSIG);
        sigLeft->Y[0] = fltRight;
      }
      return(signaliType);
    }      
      
    /* Case of s[]=sigRight */
    if (fsiList->nx==0) {
      if (sigRight) CopySig(sigRight,sigLeft);
      else {
        SizeSignal(sigLeft,1,YSIG);
        sigLeft->Y[0] = fltRight;
      }
      return(signaliType);
    }      
    
    if (sigLeft->size  == 0) {
      SetErrorf("Expect a non empty signal on the left handside");
      return(NULL);
    }
   
    if (field != NULL && !strcmp(field,"X")) {
      if (sigLeft->type == YSIG) {
        SetErrorf("Sorry, this functionality (for YSIG) is not implemented yet");
        return(NULL);
      }
      arrayLeft = sigLeft->X;
    }
    else arrayLeft = sigLeft->Y;
  
    /* Case s[1]=2 */
    if (type == numType) {
      if (fsiList->nx != 1) {
        SetErrorf("Size of both handsides should match (left size = %d, right size =1)",fsiList->nx);
        return(NULL);
      }
      arrayLeft[(int) FSIArray((&(fsiList->fsi[0])),0)]=fltRight;
      return(signaliType);
    }
    

    /* Case of s[...]=sigRight */
    if (sigRight->size != fsiList->nx) {
      SetErrorf("Size of both handsides should match (left size = %d, right size = %d)",fsiList->nx,sigRight->size);
      return(NULL);
    }
    FSI_FOR_START(fsiList); 
    arrayLeft[_i] = sigRight->Y[_k];
    FSI_FOR_END;
    return(signaliType);
  }

  /*************************
   *
   * Case of s[range1,...,rangeN] = {sig1,...,sigN} or s[range1,...,rangeN] := {sig}
   *
   *************************/

  if (type == listvType) {
    
    if (fsiList == NULL) {
      SetErrorf("Cannot set a signal with a listv");
      return(NULL);
    }
      
    if (*equal == '=' && lv->length != fsiList->size) {
      SetErrorf("Length of extraction list should be the same as length of &listv");
      return(NULL);
    }

    if (*equal == ':' && lv->length != 1) {
      SetErrorf("Length of right handside &listv is expected to be 1");
      return(NULL);
    }
    
    if (field != NULL && !strcmp(field,"X")) {
      SetErrorf("Sorry, this functionality is not implemented yet");
      return(NULL);
    }
    
    if (field != NULL && !strcmp(field,"Y")) flagY = YES;
    else flagY = NO;
       
    /* We first need to compute the new size */
    size = 0;
    for (j=0;j<lv->length;j++) {
      if (lv->values[j] != NULL && GetTypeValue(lv->values[j]) != signaliType && GetTypeValue(lv->values[j]) != signalType ) {
        SetErrorf("The &listv should contain only numbers or signals");
        return(NULL);
      }
      size += (lv->values[j] == NULL ? 1 : CastValue(lv->values[j],SIGNAL)->size);
    }
    if (*equal == ':') size *= fsiList->size;
    
    /* Then we go */
    size = sigLeft->size + size -fsiList->nx;
    sig = NewSignal();
    SizeSignal(sig,size,sigLeft->type);
  
    /* Then we perform the set */
    j = 0; /* Index in the sigLeft string */
    k = 0; /* Index in the sig string */
    _iold = -1;
    FSI_FOR_START1(fsiList);
    if (*equal == ':') GetListvNth(lv,0,(VALUE *) &sigRight,&fltRight);
    else GetListvNth(lv,_n,(VALUE *) &sigRight,&fltRight);
    switch(fsi->type) {
    case FSIFloat : 
      _j = (int) fsi->val.f;
      if (_j<=_iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        DeleteSignal(sig);
        return(NULL);
      }
      _iold = _j;
      memcpy(sig->Y+k,sigLeft->Y+j,(_j-j)*sizeof(float));
      if (!flagY && sigLeft->type == XYSIG) memcpy(sig->X+k,sigLeft->X+j,(_j-j)*sizeof(float));
      k+=_j-j;
      if (sigRight) {
        if (sigRight->size != 0) {
          memcpy(sig->Y+k,sigRight->Y,sizeof(float)*sigRight->size);
          if (!flagY && sig->type == XYSIG) {
            if (sigRight->type == XYSIG) memcpy(sig->X+k,sigRight->X,sizeof(float)*sigRight->size);
            else for (i=0;i<sigRight->size;i++) sig->X[k+i]= sigLeft->X[(_j-1<0 ? 0 : _j-1)];
          }
          k+=sigRight->size;
        }
      }
      else {sig->Y[k]=fltRight;k++;}
      j=_j+1;
      break;
    case FSIRange : 
      _j = (int) fsi->val.r->first;
      if (_j<=_iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        DeleteSignal(sig);
        return(NULL);
      }
      if (fsi->val.r->step != 1) {
        SetErrorf("Sorry, only ranges with a step of 1 are valid indexes for this operation");
        DeleteSignal(sig);
        return(NULL);
      }
      _iold = _j+fsi->val.r->size-1;
      memcpy(sig->Y+k,sigLeft->Y+j,(_j-j)*sizeof(float));
      if (!flagY && sigLeft->type == XYSIG) memcpy(sig->X+k,sigLeft->X+j,(_j-j)*sizeof(float));
      k+=_j-j;
      if (sigRight) {
        if (sigRight->size != 0) {
          memcpy(sig->Y+k,sigRight->Y,sizeof(float)*sigRight->size);
          if (!flagY && sig->type == XYSIG) {
            if (sigRight->type == XYSIG) memcpy(sig->X+k,sigRight->X,sizeof(float)*sigRight->size);
            else for (i=0;i<sigRight->size;i++) sig->X[k+i]= sigLeft->X[(_j-1<0 ? 0 : _j-1)];
          }
          k+=sigRight->size;
        }
      }
      else {sig->Y[k]=fltRight;k++;}
      j=_j+fsi->val.r->size;
      break;
    case FSISignal :
      SetErrorf("Sorry, only ranges or numbers are valid indexes for this operation");
      DeleteSignal(sig);
      return(NULL);
    }
    FSI_FOR_END1;

    memcpy(sig->Y+k,sigLeft->Y+j,sizeof(float)*(sigLeft->size-j));  
    if (!flagY && sigLeft->type == XYSIG) memcpy(sig->X+k,sigLeft->X+j,sizeof(float)*(sigLeft->size-j));
    CopySig(sig,sigLeft);
    DeleteSignal(sig);    
    return(signaliType);
  }

  /* 
   * Case of s[range1,...,rangeN] := sig or float (i.e., NOT SAME SIZE)
   */   

  if (*equal == ':') {
    
    if (fsiList == NULL) {
      SetErrorf("Inadequate ':=' syntax");
      return(NULL);
    }
    /* Case of s[range1,...,rangeN] := float */
    if (type == numType) {
      FSI_FOR_START(fsiList);
      sigLeft->Y[_i]=fltRight;
      FSI_FOR_END;
      return(signaliType);
    }
        
    if (sigLeft->type == XYSIG) {
      SetErrorf("Sorry, cannot perform sig[...]:=sig1, where sig is an XYSig");
      return(NULL);
    }

    /* If float then we create signal of size 1 */
    if (sigRight == NULL) {
      sigRight = NewSignal();
      SizeSignal(sigRight,1,YSIG);
      sigRight->Y[0] = fltRight;
    }    
      
    /* We start by creating a new signal which is a copy of sig */
    sig = NewSignal();
    CopySig(sigLeft,sig);
      
    /* We must reallocate sig */
    SizeSignal(sigLeft,sig->size - fsiList->nx + fsiList->nx*sigRight->size,sig->type);
  
    /* Then we peform the set */
    j = 0; /* Index in sig */
    k = 0; /* Index in sigLeft */
    _iold = -1;
    FSI_FOR_START(fsiList);
      /* We check the fsiList is sorted */
      if (_i <= _iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        DeleteSignal(sig);
        if ((VALUE) sigRight != value) DeleteSignal(sigRight);
        return(NULL);
      }
      _iold = _i;
      memcpy(sigLeft->Y+k,sig->Y+j,(_i-j)*sizeof(float));        
      if (!flagY && sigLeft->type == XYSIG) memcpy(sigLeft->X+k,sig->X+j,(_i-j)*sizeof(float));        
      k+=_i-j;
      memcpy(sigLeft->Y+k,sigRight->Y,sigRight->size*sizeof(float));
      k+=sigRight->size;
      j=_i+1;
    FSI_FOR_END;
    memcpy(sigLeft->Y+k,sig->Y+j,(sig->size-j)*sizeof(float));
    if (!flagY && sigLeft->type == XYSIG) memcpy(sigLeft->X+k,sig->X+j,(sig->size-j)*sizeof(float));        
    DeleteSignal(sig);
    if ((VALUE) sigRight != value) DeleteSignal(sigRight);
    return(signaliType);
            
  }

  SetErrorf("Weird error");
  return(NULL);
}


/*
 * Basic routine to deal with setting fields of a signal
 */
void *SetSignalField(SIGNAL sig,void **arg)
{
  char *field = ARG_S_GetField(arg);
  FSIList *fsiList = ARG_S_GetFsiList(arg);
  char *type = ARG_S_GetRightType(arg);    
  float flt = ARG_S_GetRightFloat(arg);   
  VALUE value = ARG_S_GetRightValue(arg);
  char *equal = ARG_S_GetEqual(arg);

  if (SetSignalField_(sig,NULL,fsiList,flt,value,equal,NULL) == NULL) return(NULL);
  ARG_S_SetResValue(arg,(VALUE) sig);
  return(signaliType);
}  



/* 
 * Routine to deal with setting of signals
 */

static char *doc = "{[*opt,...] [:]= (<float> | <range> | <signal> | <listv>)} {Get/Set the signal values}"; 
static char *xdoc = "{[*opt,...] [:]= (<float> | <range> | <signal> | <listv>)} {Get/Set the X field of a signal}"; 
static char *ydoc = "{[*opt,...] [:]= (<float> | <range> | <signal> | <listv>)} {Get/set the Y field of a signal}"; 
static char *indexdoc = "{[[*nolimit],<xValue>]} {Get the index corresponding to a given <xValue>}";

static void *SetExtractSignalV(SIGNAL sig,void **arg)
{
  char *field;
  FSIList *fsiList;
  char *type;    
  float flt;   
  char *equal;
  VALUE value;
  
  field = ARG_S_GetField(arg);

   /* doc */
  if (sig == NULL) {
    if (field == NULL || !strcmp(field,"")) return(doc);
    if (!strcmp(field,"X")) return(xdoc);
    if (!strcmp(field,"Y")) return(ydoc);
  }

  fsiList = ARG_S_GetFsiList(arg);
  type = ARG_S_GetRightType(arg);    
  flt = ARG_S_GetRightFloat(arg);   
  value = ARG_S_GetRightValue(arg);
  equal = ARG_S_GetEqual(arg);

  if (SetSignalField_(sig,field,fsiList,flt,value,equal,NULL) == NULL) return(NULL);
  ARG_S_SetResValue(arg,(VALUE) sig);
  return(signaliType);
}
 

/* 
 * Routine to deal with getting of signals managing extraction
 */
void *GetSignalField_(SIGNAL signal, void **arg)
{
  char *field;
  FSIList *fsiList;
  float *pFlt;
  char **pStr;
  VALUE *pValue;
  
  int max;
  FSI_DECL;
  BorderType bt;
  SIGNAL sigResult;
  float xmax,xmin;
  ExtractInfo *ei;
  InterMode im;
  char flagIndex;
  float firstFlt;
  int n,first,last;
  char flagX,flagY;
  RANGE rg;
  int i;

  field = ARG_G_GetField(arg);
  fsiList = ARG_G_GetFsiList(arg);
  pFlt = ARG_G_GetResPFloat(arg);
  pStr = ARG_G_GetResPStr(arg);
  pValue = ARG_G_GetResPValue(arg);

  if (field == NULL && fsiList == NULL) {
    *pValue = (VALUE) signal;
    return(signaliType);
  }
  
  if (field != NULL && !strcmp(field,"index")) {
    if (fsiList == NULL) {
      SetErrorf("Field 'index' needs extraction : index[]");
      return(NULL);
    }
      
    /* 1 float in between the [] */
    if (fsiList->nx != 1) {
      SetErrorf("index[] expects 1 xValue only");
      return(NULL);
    }
    i = ISig(signal, FSIArray((&(fsiList->fsi[0])),0));
    if (signal->type==YSIG) {
      i = (i<0 ? 0 : i);
      i = (i>=signal->size ? signal->size-1 : i);
    }
    return(GetFloatField(i,arg));
  }
  
  
  /* case of an empty extraction */
  if (fsiList != NULL && fsiList->nx == 0) {
    *pValue = (VALUE) TNewSignal();
    return(signalType);
  }
  
  if (fsiList == NULL && field != NULL && !strcmp(field,"X")) {
    if (signal->type == XYSIG) {
      sigResult = TNewSignal();
      SizeSignal(sigResult,signal->size,YSIG);
      CopySigXX(signal,sigResult,"XY");
      *pValue = (VALUE) sigResult;
      return(signaliType);
    }
    rg = TNewRange();
    rg->first = signal->x0;
    rg->step = signal->dx;
    rg->size = signal->size;
    *pValue = (VALUE) rg;
    return(rangeType);
  }

  if (fsiList == NULL && field != NULL && !strcmp(field,"Y")) {
    sigResult = TNewSignal();
    SizeSignal(sigResult,signal->size,YSIG);
    CopySigXX(signal,sigResult,"YY");
    *pValue = (VALUE) sigResult;
    return(signaliType);
  }

  flagX = flagY = 0;
  if (field) {
    if (!strcmp(field,"X")) flagX = 1;
    else flagY = 1;
  }
  
  /* Get extractInfo */
  ei = fsiList->ei;
  xmax = ei->xmax;
  xmin = ei->xmin;
  max = signal->size;

  if (fsiList->options & FSIOptSigBPer) bt = BorderPer;
  else if (fsiList->options & FSIOptSigBMir) bt = BorderMir;
  else if (fsiList->options & FSIOptSigBMir1) bt = BorderMir1;
  else if (fsiList->options & FSIOptSigBCon) bt = BorderCon;
  else if (fsiList->options & FSIOptSigB0) bt = Border0;
  else bt = BorderNone;

  if (fsiList->options & FSIOptSigXLin) im = InterLinear;
  else im = InterNone;
 
  if (fsiList->options & (FSIOptSigX | FSIOptSigXLin)) flagIndex = NO;
  else flagIndex = YES;
  
  /*
   * Case the result will be a simple float
   */
  if (bt == BorderNone && fsiList->nx1 == 1 || bt != BorderNone && fsiList->nx == 1) {

    if (fsiList->options & FSIOptSigNoLimit) {
      FSI_FOR_START(fsiList); 
      if (_f<xmin || _f > xmax) continue;
      break;
      FSI_FOR_END;
    }
    else  _f =  FSIArray((&(fsiList->fsi[0])),0);

    if (field == NULL || !strcmp(field,"Y")) *pFlt = X2YSig(signal,_f,im,bt,flagIndex);
    else if (flagIndex) *pFlt = XSig(signal,(int) _f);
    else *pFlt = _f;

    return(numType);
  }

  /* 
   * Allocation of the signal result 
   */
  sigResult = TNewSignal();
  if (field != NULL || signal->type == XYSIG && !flagIndex && fsiList->size == 1 && fsiList->fsi[0].type == FSIRange) n = YSIG;
  else n = signal->type;
  if (bt != BorderNone) SizeSignal(sigResult,fsiList->nx,n);
  else SizeSignal(sigResult,fsiList->nx1,n);

  /* ????? Utiliser memcpy dans le cas de signaux XY que l'on extrait avec *x */
  
  /* The loop in the case of *nolimit */
  if (fsiList->options & FSIOptSigNoLimit) {
    FSI_FOR_START1(fsiList); 
  
    if (fsi->type == FSIRange && 
        (flagIndex && fsi->val.r->step == 1 || 
         (field == NULL && signal->type == YSIG && fsi->val.r->step == signal->dx  &&
          (fsi->val.r->first-signal->x0)/signal->dx == (int) ((fsi->val.r->first-signal->x0)/signal->dx) ))) {
         
      if (flagIndex) {
        first = (int) RangeFirst(fsi->val.r);
        last = (int) RangeLast(fsi->val.r);
        if (_k == 0) firstFlt = MAX(first,0);
      }
      else {
        first = (int) ((RangeFirst(fsi->val.r)-signal->x0)/signal->dx);
        last = (int) ((RangeLast(fsi->val.r)-signal->x0)/signal->dx);
        if (_k == 0) firstFlt = XSig(signal,MAX(first,0));
      }
      
      /* MACINTOSH "scary" bug keeps from writing just instead of the 2 lines below  
      if (signal->size<first || last <0) continue; */
      
      n = signal->size;
      if (n<=first || last <0) continue; 
      
      if (field == NULL) {
        memcpy(sigResult->Y+_k,signal->Y+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));
        if (sigResult->type == XYSIG) 
          memcpy(sigResult->X+_k,signal->X+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));
      }
      else if (!strcmp(field,"Y"))
        memcpy(sigResult->Y+_k,signal->Y+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));
      else
        memcpy(sigResult->Y+_k,signal->X+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));

      _k+=MIN(last,signal->size-1)-MAX(first,0)+1;
    }
    else {        
      FSI_FOR_START2(fsiList);
      if (_f<xmin || _f > xmax) continue;
      if (field == NULL || !strcmp(field,"Y")) sigResult->Y[_k] = X2YSig(signal,_f,im,bt,flagIndex);    
      else if (flagIndex) sigResult->Y[_k] = XSig(signal,(int) _f);
      else sigResult->Y[_k] = _f;
      if (sigResult->type == XYSIG) {
        if (flagIndex) sigResult->X[_k] = signal->X[(int) _f];
        else sigResult->X[_k] = _f;
      }
      if (_k == 0) firstFlt = _f;
      FSI_FOR_END2;
    }
    FSI_FOR_END1;
  }

  /* The loop in the generic case */
  else {
    FSI_FOR_START1(fsiList); 
    if (fsi->type == FSIRange) {
      if (flagIndex) {
        first = (int) RangeFirst(fsi->val.r);
        last = (int) RangeLast(fsi->val.r);
        if (_k == 0) firstFlt = first;
      }
      else {
        first = (int) ((RangeFirst(fsi->val.r)-signal->x0)/signal->dx);
        last = (int) ((RangeLast(fsi->val.r)-signal->x0)/signal->dx);
        if (_k == 0) firstFlt = XSig(signal,first);
      }
    }
    if (fsi->type == FSIRange && INRANGE(0,first,signal->size-1) && INRANGE(0,last,signal->size-1) &&
        (flagIndex && fsi->val.r->step == 1 || 
         (field == NULL && signal->type == YSIG && fsi->val.r->step == signal->dx  &&
          (fsi->val.r->first-signal->x0)/signal->dx == (int) ((fsi->val.r->first-signal->x0)/signal->dx) ))) {
         
      if (field == NULL) {
        memcpy(sigResult->Y+_k,signal->Y+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));
        if (sigResult->type == XYSIG) 
          memcpy(sigResult->X+_k,signal->X+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));
      }
      else if (!strcmp(field,"Y"))
        memcpy(sigResult->Y+_k,signal->Y+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));
      else
        memcpy(sigResult->Y+_k,signal->X+MAX(first,0),(MIN(last,signal->size-1)-MAX(first,0)+1)*sizeof(float));

      _k+=MIN(last,signal->size-1)-MAX(first,0)+1;
    }
    else {        
      FSI_FOR_START2(fsiList); 
      if (field == NULL || !strcmp(field,"Y")) sigResult->Y[_k] = X2YSig(signal,_f,im,bt,flagIndex);    
      else if (flagIndex) sigResult->Y[_k] = XSig(signal,(int) _f);
      else sigResult->Y[_k] = _f;
      if (sigResult->type == XYSIG) {
        if (flagIndex) sigResult->X[_k] = signal->X[(int) _f];
        else sigResult->X[_k] = _f;
      }
      if (_k == 0) firstFlt = _f;
      FSI_FOR_END2;
    }
    FSI_FOR_END1;
  }

  /* Setting the fields of the Y-signal */
  if (field == NULL) {
    if (sigResult->type == YSIG) {
      if (flagIndex) {
        if (fsiList->fsi[0].type == FSIRange) {
          if (fsiList->fsi[0].val.r->step > 0) {
            sigResult->x0 = signal->dx*((int) firstFlt)+signal->x0;
            sigResult->dx = signal->dx*fsiList->fsi[0].val.r->step;
          }
          else {
            sigResult->x0 = 0;
            sigResult->dx = -signal->dx*fsiList->fsi[0].val.r->step;
          }
        }
        else {
          sigResult->dx = signal->dx;
          sigResult->x0 = XSig(signal,(int) FSIArray((&(fsiList->fsi[0])),0));
        }
      }
      else {
        if (fsiList->fsi[0].type == FSIRange) {
          if (fsiList->fsi[0].val.r->step > 0) {
            sigResult->x0 = firstFlt;
            sigResult->dx = fsiList->fsi[0].val.r->step;
          }
          else {
            sigResult->x0 = 0;
            sigResult->dx = -fsiList->fsi[0].val.r->step;
          }
        }
        else {
          sigResult->dx = 1;
          sigResult->x0 = FSIArray((&(fsiList->fsi[0])),0);
        }
      }
    }
  }
        
  *pValue = (VALUE) sigResult;
        
  return(signaliType);
}

void *GetSignalExtractField(SIGNAL sig, void **arg)
{
  char *field = ARG_G_GetField(arg);
  void *res;
  
  ARG_G_SetField(arg,NULL);
  
  res = GetSignalField_(sig, arg);
  
  ARG_G_SetField(arg,field);

  return(res);
}

static void *GetExtractSignalV(VALUE value, void **arg)
{
  char *field = ARG_G_GetField(arg);

  /* doc */
  if (value == NULL) {
    if (field == NULL || !strcmp(field,"")) return(doc);
    if (!strcmp(field,"index")) return(indexdoc);
    if (!strcmp(field,"X")) return(xdoc);
    if (!strcmp(field,"Y")) return(ydoc);
  }
  
  return(GetSignalField_((SIGNAL) value, arg));
}


/*
 * Get the options for extraction (called for field NULL, X or Y only)
 */


static char *optionDoc = "{{*nolimit,*b0,*bconst,*bmirror,*bmirror1,*bperiodic,*x,*xlin} \
{*nolimit : indexes can be out of range} \
{*b0 : border effect with 0 value} \
{*bconst : border effect with constant values (last signal value for right handside and first signal value for left handside)} \
{*bperiodic : periodic border effect)} \
{*bmirror1 : mirror+periodic border effect (first and last points are repeated)} \
{*bmirror : mirror+periodic border effect (first and last points are NOT repeated))} \
{*x : index values are replaced by x-values. Interpolation is piece-wise constant} \
{*xlin : index values are replaced by x-values. Interpolation piece-wise linear}\
}";
static char *xoptionDoc = "{{*nolimit,*b0,*bconst,*bmirror,*bmirror1,*bperiodic,*x,*xlin} \
{*nolimit : indexes can be out of range} \
{*b0 : border effect with 0 value} \
{*bconst : border effect with constant values (last signal value for right handside and first signal value for left handside)} \
{*bperiodic : periodic border effect)} \
{*bmirror1 : mirror+periodic border effect (first and last points are repeated)} \
{*bmirror : mirror+periodic border effect (first and last points are NOT repeated))} \
{*x : index values are replaced by x-values. Interplation is piece-wise constant} \
{*xlin : index values are replaced by x-values. Interplation piece-wise linear}\
}";
static char *yoptionDoc = "{{*nolimit,*b0,*bconst,*bmirror,*bmirror1,*bperiodic,*x,*xlin} \
{*nolimit : indexes can be out of range} \
{*b0 : border effect with 0 value} \
{*bconst : border effect with constant values (last signal value for right handside and first signal value for left handside)} \
{*bperiodic : periodic border effect)} \
{*bmirror1 : mirror+periodic border effect (first and last points are repeated)} \
{*bmirror : mirror+periodic border effect (first and last points are NOT repeated))} \
{*x : index values are replaced by x-values. Interplation is piece-wise constant} \
{*xlin : index values are replaced by x-values. Interplation piece-wise linear}\
}";
static char *indexoptionDoc = "{{*nolimit} {*nolimit : indexes can be out of range}}";

static void *GetExtractOptionsSignalV(VALUE value, void **arg)
{
  SIGNAL signal;
  char *field = ARG_EO_GetField(arg);

  
   /* doc */
  if (value == NULL) {
    if (field == NULL || !strcmp(field,"")) return(optionDoc);
    if (!strcmp(field,"index")) return(indexoptionDoc);
    if (!strcmp(field,"X")) return(xoptionDoc);
    if (!strcmp(field,"Y")) return(yoptionDoc);
  }

  signal = (SIGNAL) value;

  if (field != NULL && !strcmp(field,"X") && signal->type == YSIG) return(NULL);

  if (field != NULL && !strcmp(field,"index")) return(indexextractOptions);
  
  return(extractOptionsSig);
}


/*
 * Function to get the ExtractInfo for fields NULL, X or Y
 */

static void *GetExtractInfoSignalV(VALUE value, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  SIGNAL signal = (SIGNAL) value;
  char *field = ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);

  if (signal->size == 0) {
    SetErrorf("No extraction on emty signal");
    return(NULL);
  }
  
  
  if (field != NULL && !strcmp(field,"index")) {
    extractInfo.nSignals = 1;
    if (signal->type == YSIG) {
      extractInfo.xmax = signal->dx*(signal->size-1)+signal->x0;
      extractInfo.dx = signal->dx;
      extractInfo.xmin = signal->x0;
    }
    else {
      extractInfo.xmax = signal->X[signal->size-1];
      extractInfo.dx = -1;
      extractInfo.xsignal = signal;
      extractInfo.xmin = signal->X[0];
    }
    if (*options & FSIOptSigNoLimit) extractInfo.flags = 0;
    else extractInfo.flags = EIErrorBound;
    return(&extractInfo);
  }
        
  /* Some checkings */
  if (signal->type == XYSIG && *options & (FSIOptSigBPer | FSIOptSigBMir | FSIOptSigBMir1)) {
    SetErrorf("Cannot use any of the options *bperiodic, *bmirror or *bmirror1 for XYSignals");
    return(NULL);
  }

  /* Some checkings */
  if (signal->type == XYSIG && *options & (FSIOptSigBCon | FSIOptSigB0) && !(*options & (FSIOptSigX | FSIOptSigXLin))) {
    SetErrorf("Cannot use any of the options *bconst or *b0 for XYSignals without *x or *xlin");
    return(NULL);
  }

  /* Some checkings */
  if (*options & FSIOptSigXLin && *options & (FSIOptSigBPer | FSIOptSigBMir | FSIOptSigBMir1)) {
    SetErrorf("Cannot use any of the options *bperiodic, *bmirror or *bmirror1 with *xlin");
    return(NULL);
  }

  /* If *bperiodic,... then *nolimit must be off */
  if (*options & (FSIOptSigBPer | FSIOptSigBMir | FSIOptSigBMir1 | FSIOptSigB0 | FSIOptSigBCon)) *options &= ~FSIOptSigNoLimit;
      
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.nSignals = 1;
    flagInit = NO;
  }
  
  if (*options & (FSIOptSigX | FSIOptSigXLin)) {
    if (signal->type == YSIG) {
      extractInfo.xmax = signal->dx*(signal->size-1)+signal->x0;
      extractInfo.dx = signal->dx;
      extractInfo.xmin = signal->x0;
    }
    else {
      extractInfo.xmax = signal->X[signal->size-1];
      extractInfo.dx = -1;
      extractInfo.xsignal = signal;
      extractInfo.xmin = signal->X[0];
    }
  }
  else {
    extractInfo.xmax = signal->size-1;
    extractInfo.dx = 1;
    extractInfo.xmin = 0;
  }        
      
  /* '*nolimit' option : set some flags */
  if (*options & (FSIOptSigX | FSIOptSigXLin)) extractInfo.flags = 0;
  else extractInfo.flags = EIIntIndex;
  if (!(*options & (FSIOptSigNoLimit | FSIOptSigBPer | FSIOptSigBMir | FSIOptSigBMir1 | FSIOptSigBCon | FSIOptSigB0))) extractInfo.flags |= EIErrorBound;
      
  return(&extractInfo);
}

/*
 * 'size' field
 */

static char *sizeDoc = "{[= <size>]} {Sets/Gets the size of a signal. In a case of a Set no initialization is performed. Moreover, if the asked size \
is smaller than the allocation size no additional allocation is performed.}";


static void * GetSizeSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(sizeDoc);
  
  return(GetIntField(((SIGNAL) value)->size,arg));
}

int SetSizeSignal(SIGNAL sig, int size)
{
  if (size < 0) {
    SetErrorf("SetSizeSignal() : size must be positive");
    return(0);
  }
  SizeSignal(sig,size,sig->type);
  return(1);
}

static void * SetSizeSignalV(VALUE value, void **arg)
{
 SIGNAL signal = (SIGNAL) value;
 int size;
  
 /* doc */
 if (value == NULL) return(sizeDoc);

 size = signal->size;
 if (SetIntField(&size,arg,FieldPositive)==NULL) return(NULL);
 if (SetSizeSignal(signal,size)==0) return(NULL);
 return(numType);

}

/*
 * 'dx' field
 */

static char *dxDoc = "{[= <dx>]} {Sets/Gets the dx of a Y-signal}";

static void * GetDxSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(dxDoc);
  
  return(GetFloatField(((SIGNAL) value)->dx,arg));
}

int SetDxSignal(SIGNAL sig,float dx)
{
  if (dx <= 0) {
    SetErrorf("SetDxSignal() : Bad value %g for 'dx' field",dx);
    return(0);
  }
  sig->dx=dx;
  return(1);
}

static void *SetDxSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(dxDoc);

 return(SetFloatField(&(((SIGNAL) value)->dx),arg,FieldSPositive));
}

/*
 * 'x0' field
 */

static char *x0Doc = "{[= <x0>]} {Sets/Gets the x0 of a Y-signal}";

static void * GetX0SignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(x0Doc);
  
  return(GetFloatField(((SIGNAL) value)->x0,arg));
}

int SetX0Signal(SIGNAL sig,float x0)
{
  sig->x0=x0;
  return(1);
}

static void * SetX0SignalV(VALUE value, void **arg)
{
     /* doc */
  if (value == NULL) return(x0Doc);

 return(SetFloatField(&(((SIGNAL) value)->x0),arg,0));
}

/*
 * 'name' field
 */

static char *nameDoc = "{[= <name>]} {Sets/Gets the name of a signal}";

static void * GetNameSignalV(SIGNAL signal, void **arg)
{
  /* Documentation */
  if (signal == NULL) return(nameDoc);
  
  return(GetStrField(signal->name,arg));
}

/* Set the name of a signal */
int SetNameSignal(SIGNAL signal, char *name)
{
  if (signal->name != defaultName && signal->name != NULL) {
    Free(signal->name);
    signal->name = NULL;
  }
  if (name == NULL) signal->name = defaultName;
  else signal->name = CopyStr(name);
  return(1);
}


static void * SetNameSignalV(SIGNAL signal, void **arg)
{
     /* doc */
  if (signal == NULL) return(nameDoc);

  if (signal->name==defaultName) {
    signal->name=CharAlloc(1);
    signal->name[0] = '\0';
  }
  return(SetStrField(&(signal->name),arg));
}


/*
 * 'xy' field
 */

static char *xyDoc = "{[= (0|1)]} {Sets/Gets 'xy' flag signal. If 0 it means that the signal is a Y-signal, otherwise, it is a XY-signal.}";

static void * GetXYSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(xyDoc);
  
  return(GetIntField(((SIGNAL) value)->type==XYSIG,arg));
}

int SetXYSignal(SIGNAL sig,int xy)
{
  if (xy == 0) sig->type = YSIG;
  else {
    SizeSignal(sig,sig->size,XYSIG);
  }
  return(1);
}

static void * SetXYSignalV(VALUE value, void **arg)
{
  int xy;
    
  /* doc */
  if (value == NULL) return(xyDoc);
  
  xy = (((SIGNAL) value)->type == XYSIG);

  if (SetIntField(&xy,arg,FieldPositive)==NULL) return(NULL);
  
  if (SetXYSignal(((SIGNAL) value),xy)==0) return(NULL);

  return(numType);
}

/*
 * 'sizeAllocX' field
 */

static char *SizeAllocXDoc = "{[= <sizeAllocX>]} {Gets/Sets the allocation size of the X array of a signal.}";

static void * GetSizeAllocXSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(SizeAllocXDoc);
  
  return(GetIntField(((SIGNAL) value)->sizeMallocX,arg));
}

int SetSizeAllocXSignal(SIGNAL s,int size)
{  
  if (s->type == XYSIG && size < s->size) {
    SetErrorf("SetSizeAllocXSignal() : Cannot set the 'sizeAllocX' field to an integer smaller than the 'size' field");
    return(0);
  }
  
  if (s->X != NULL) Free(s->X);
  s->X = NULL;
  if (size != 0) s->X = FloatAlloc(size);
  s->sizeMallocX = size;
  return(1);
}

static void * SetSizeAllocXSignalV(VALUE value, void **arg)
{
  int size;
  SIGNAL s;
    
     /* doc */
  if (value == NULL) return(SizeAllocXDoc);

  s = (SIGNAL) value;
  size = s->sizeMallocX;

  if (SetIntField(&size,arg,FieldPositive)==NULL) return(NULL);
    
  if (SetSizeAllocXSignal(s,size)==0) return(NULL);
  
  return(numType);
}

/*
 * 'sizeAllocY' field
 */

static char *SizeAllocYDoc = "{[= <sizeAllocY>]} {Gets the allocation size of the X array of a signal.}";

static void * GetSizeAllocYSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(SizeAllocYDoc);
  
  return(GetIntField(((SIGNAL) value)->sizeMallocY,arg));
}

int SetSizeAllocYSignal(SIGNAL s,int size)
{
  if ( size < s->size) {
    SetErrorf("SetSizeAllocYSignal() : Cannot set the 'sizeAllocY' field to an integer smaller than the 'size' field");
    return(0);
  }
  
  if (s->Y != NULL) Free(s->Y);
  s->Y = NULL;
  if (size != 0) s->Y = FloatAlloc(size);
  s->sizeMallocY = size;
  return(1);
}

static void * SetSizeAllocYSignalV(VALUE value, void **arg)
{
  int size;
  SIGNAL s;
    
     /* doc */
  if (value == NULL) return(SizeAllocYDoc);

  s = (SIGNAL) value;
  size = s->sizeMallocY;

  if (SetIntField(&size,arg,FieldPositive)==NULL) return(NULL);

  if (SetSizeAllocYSignal(s,size)==0) return(NULL);
  
  return(numType);
}


/*
 * 'firstp' field
 */

static char *firstpDoc = "{[= <firstp>]} {Sets/Gets the 'firstp' field of a signal ('firstp' is the index number used for storing the first index affected by border effects).}";
static void * GetFirstpSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(firstpDoc);
  
  return(GetIntField(((SIGNAL) value)->firstp,arg));
}

static void * SetFirstpSignalV(VALUE value, void **arg)
{
     /* doc */
  if (value == NULL) return(firstpDoc);

 return(SetIntField(&(((SIGNAL) value)->firstp),arg,FieldPositive));
}

/*
 * 'lastp' field
 */

static char *lastpDoc = "{[= <lastp>]} {Sets/Gets the 'lastp' field of a signal ('lastp' is the index number used for storing the last index affected by border effects).}";

static void * GetLastpSignalV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(lastpDoc);
  
  return(GetIntField(((SIGNAL) value)->lastp,arg));
}

static void * SetLastpSignalV(VALUE value, void **arg)
{
     /* doc */
  if (value == NULL) return(lastpDoc);

  return(SetIntField(&(((SIGNAL) value)->lastp),arg,FieldPositive));
}

/*
 * 'tolistv' field
 */

static char *tolistvDoc = "{} {Gets a listv made of the y-values of the signal}";

static void * GetTolistvSignalV(SIGNAL sig, void **arg)
{
  LISTV lv;
  int i;
  
  /* Documentation */
  if (sig == NULL) return(tolistvDoc);
  
  lv = TNewListv();
  AppendFloatArray2Listv(lv,sig->Y,sig->size);

  return(GetValueField(lv,arg));
}



/*
 * Function to get the type of a signal
 */
static char * GetTypeSignal(VALUE value)
{
  SIGNAL s = (SIGNAL) value;
  
  if (s->size == 0) return(signalType);
  return(signaliType);
}


/*
 * Function for allocating a signal
 */
extern int flagOn;
SIGNAL NewSignal(void)
{
  extern TypeStruct tsSignal;
  SIGNAL signal;

#ifdef DEBUGALLOC
DebugType = "Signal";
#endif
  
  signal = (SIGNAL) (Malloc(sizeof(struct signal)));

  InitValue(signal,&tsSignal);
  
  signal->X = NULL;
  signal->Y = NULL;
  signal->sizeMallocX = signal->sizeMallocY =0;
  signal->size = 0;
  signal->x0 = 0;
  signal->dx = 1;
  signal->name = defaultName;
   
  signal->firstp = 0;
  signal->lastp = 0;
  signal->param = 1.;

  signal->type = YSIG;

  if (flagOn) Printf("** New Signal %p\n",signal); 

  return (signal);
}

/* Allocating a temporary signal */
SIGNAL TNewSignal(void)
{
  SIGNAL signal;
  
  signal = NewSignal();
  TempValue(signal);
  return(signal);
}



/*
 * Initialization of 'signal' and desallocation  
 * of the array of float signal->Y          
 */
 
void ClearSignal(SIGNAL signal)
{
  if (signal->X) {
    Free(signal->X);
    signal->X=NULL;
  }
  if (signal->Y) {
    Free(signal->Y);
    signal->Y=NULL;
  }
  signal->size = 0;
  signal->sizeMallocX = signal->sizeMallocY = 0;
  
}
    
/*
 *
 * Very important procedure which has to be called each time one wants to use a signal.
 * It manages allocation.
 * It asks the signal 'signal' to be able to store a signal of type 'type'
 * and size 'size'.
 *
 * In any case it initializes all the fields except dx and x0
 *
 */

void SizeSignal(SIGNAL signal,int size,int type)
{
  if (size < 0) Errorf("SizeSignal() : asked size is negative '%d'",size);
  
  signal->type = type;
  
  if (size == 0) {
    signal->lastp = signal->firstp = signal->size = 0;
    signal->param = 1;
    return;
  }
    
    
  switch(type) {

  	case YSIG: 
  	  if (signal->sizeMallocX != 0) {
  		  Free(signal->X);
  		  signal->X = NULL;
  		  signal->sizeMallocX = 0;
  	  }
  	  if (signal->sizeMallocY < size) {
      	  if (signal->sizeMallocY != 0) Free(signal->Y);
      	  signal->Y = NULL;
      	  signal->sizeMallocY = 0;
      	  signal->size = 0;
      	  signal->Y = FloatAlloc(size);
      	  signal->sizeMallocY = size;
       }
       break;

    case XYSIG: 
      if (signal->sizeMallocX < size) {
      	if (signal->sizeMallocX != 0) Free(signal->X);
      	signal->X = NULL;
     	signal->sizeMallocX = 0;
      	signal->X = FloatAlloc(size);
      	signal->sizeMallocX = size;
      }
      if (signal->sizeMallocY < size) {
      	if (signal->sizeMallocY != 0) Free(signal->Y);
    	signal->Y = NULL;
      	signal->sizeMallocY = 0;
      	signal->size = 0;
      	signal->Y = FloatAlloc(size);
      	signal->sizeMallocY = size;
      }
      break;
      
     default: Errorf("SizeSignal() : Bad Signal type");
   }

  signal->lastp = size-1;
  signal->firstp = 0;
  signal->size = size;
  signal->param = 1;
}


/*
 * Function for Deleting a signal
 */
void DeleteSignal(SIGNAL signal)
{
  if (signal)
    {
    if (signal->nRef==0) {
      Warningf("DeleteSignal() : Trying to delete a temporary signal\n");
      return;
    }
    RemoveRefValue(signal);
    if (signal->nRef > 0) return;
    
    if (signal->X) Free(signal->X);
    if (signal->Y) Free(signal->Y);
    if (signal->name != defaultName) Free(signal->name);
    if (flagOn) Printf("** Delete Signal %p\n",signal); 

#ifdef DEBUGALLOC
DebugType = "Signal";
#endif
     Free(signal);
    };
}


/*****************************************************************************
 *
 *  Functions to access index, Y or X values of signals           
 *
 *****************************************************************************/

/* 
 * Dichotomic recursive algorithm to find an x value within a signal
 * (used only in the case of an XYSIG signal  
 */

/* Only called by the DichX routine */
static int DichX_(SIGNAL signal,float x,int iMin,int iMax)
{
  int i;
  
  if (iMin == iMax) return(iMin);
  
  if (iMax-iMin == 1) {
    if (signal->X[iMax] == x) return(iMax);
    return(iMin);
  }

  i = (iMax+iMin)/2;
  if (signal->X[i] > x)  return(DichX_(signal,x,iMin,i));
  return(DichX_(signal,x,i,iMax));  
} 

/* 
 * Get the index associated to a x-value within a signal 
 * (Only for XYSIG signal)
 * returns -1 if x is too low
 */		
static int DichX(SIGNAL signal,float x)
{
  if (signal->size == 0) Errorf("DichX() : signal is empty");

  if (signal->X[0] > x) return(-1);

  if (signal->size == 1) return(0);

  if (signal->X[1] > x) return(0);

  if (signal->size == 2) return(1);
  
  if (signal->X[signal->size-1] <= x) return(signal->size-1);

  
  return(DichX_(signal,x,0,signal->size-1));
}

/*
 * The main routine that can be called from outside to get an index
 * associated to an x value ('xValue') whatever the type of the signal is.
 * This routines behaves differently depending on the type of signal :
 *   - YSIG  : the index is a float number that can be out of range and that
 *             is computed only using the dx and x0 fields of the signal
 *   - XYSIG : the index is an integer that can be out of range (-1)
 *             it is the  index i such that x[i] <= xvalue <  x[i+1]
 */ 
float X2FIndexSig(SIGNAL signal, float xValue)
{
  if (signal->type == XYSIG) return(DichX(signal,xValue));
  return((xValue-signal->x0)/signal->dx);
}


/*
 * The main routine that returns a Y value given an x value 
 * given an interpolation mode and a border type.
 * If flagindex is YES then x corresponds to a float index
 * otherwise, it is an x-value
 */
float X2YSig(SIGNAL signal,float x, InterMode im, BorderType bt, char flagIndex)
{
  int i;
  float index;

  
  if (flagIndex) index = x;
  else index = X2FIndexSig(signal,x);
  
  /*
   * We first deal with the border type
   */

  switch (signal->type) {
    
    /* case of a YSignal */
    case YSIG :
      
      /* Manage border effects */
      switch (bt) {
        case BorderNone :
          break;
        case BorderPer : 
          index = fmod(index,signal->size);
          if (index < 0) index+=signal->size;
          break;
        case BorderMir1 : 
          index = fmod(index,2*signal->size);
          if (index < 0) index+=2*signal->size;
          if (index>=signal->size) index = 2*signal->size-1-index;
          break;
        case BorderMir : 
          index = fmod(index,2*signal->size-2);
          if (index < 0) index+=2*signal->size-2;
          if (index>=signal->size) index = 2*signal->size-2-index;
          break;
        case BorderCon :
          if (index < 0) return(signal->Y[0]);
          if (index>=signal->size) return(signal->Y[signal->size-1]);
          break;
        case Border0 :
          if (index < 0 || index>=signal->size) return(0);
          break;
          
        default : Errorf("X2YSig() : Weird");
      }  
      break;
      
    /* case of a XYSignal */
    case XYSIG :

      /* Manage border effects */
      switch (bt) {
        case BorderNone :
          break;
        case BorderCon :
          if (index < 0) return(signal->Y[0]);
          if (index>=signal->size) return(signal->Y[signal->size-1]);
          break;
        case Border0 :
          if (index < 0 || index>=signal->size) return(0);
          break;
          
        default : Errorf("X2YSig() : Only 'BorderNone', 'Border0' or 'BorderCon' are available for XY signals");
      }
      break;
      
  }

  /*
   * Then the Y value 
   */

  i = (int) floor(index);
  if (i <0 || i > signal->size-1) Errorf("X2YSig() : Weird 1");
  
  switch(im) {
    case InterNone : return(signal->Y[i]);
    case InterLinear : 
      if (i == signal->size-1) return(signal->Y[i]);
      if (i+1> signal->size-1) Errorf("X2YSig() : Weird 2");
      switch (signal->type) {
        case YSIG : return((signal->Y[i+1]-signal->Y[i])*(index-i)+signal->Y[i]);
        case XYSIG :
          if (signal->X[i+1]==signal->X[i]) return(signal->Y[i]);
          return((signal->Y[i+1]-signal->Y[i])*(x-signal->X[i])/(signal->X[i+1]-signal->X[i])+signal->Y[i]);
      }
    default : Errorf("YSig1() : Weird 2");
  }
}

/* Returns the index corresponding to an xValue (always in range [0,signal->size-1]) */
int ISig(SIGNAL signal, float xValue)
{
  int i;
  
  if (signal->type == XYSIG) return(DichX(signal,xValue));

  i = (int) ((xValue-signal->x0)/signal->dx);

  if (i < 0) i = 0;
  else if (i >= signal->size) i = signal->size-1;

  return(i);
}

/* Return the xValue to an index */
float XSig(SIGNAL signal,int index)
{
  if (signal->type == XYSIG)
  	return(signal->X[index]);
  else
    return(signal->x0+signal->dx*index);
}

/* Return either the x or the y value corresponding to an index */

float XYSig(SIGNAL signal,int index,char which)
{
  switch(which) {
    case 'Y': 
      return(signal->Y[index]);
      break;
    case 'X':
      return(XSig(signal,index));
      break;
    default:
      Errorf("Bad value of 'which' in 'XYSig' function");
  }
} 




/********************************
 *
 *  Copy procedures            
 *
 ********************************/
 
/* Copy the signal in in the signal out */

void CopyFieldsSig(SIGNAL in,SIGNAL out)
{
	out->firstp = in->firstp;
	out->lastp = in->lastp;
	out->type = in->type;
	out->size = in->size;
	out->dx = in->dx;
	out->x0 = in->x0;
	out->param = in->param;
}

SIGNAL CopySig(SIGNAL in, SIGNAL out)
{
  if (in == out) return(out);
    
  if (out == NULL) out = NewSignal();  
  
  SizeSignal(out,in->size,in->type);
  
  CopyFieldsSig(in,out);
  
  memcpy(out->Y,in->Y,in->size*sizeof(float));
  if (in->type == XYSIG) memcpy(out->X,in->X,in->size*sizeof(float));
  
  return(out);
}

void CopySigXX(SIGNAL in,SIGNAL out,char *type)
{
  int i;
  
  if (out == in) Errorf("CopySigXX() : The two signals must be different");
  
  if (!strcmp(type,"YY")) {  
    SizeSignal(out,in->size,YSIG);
    memcpy(out->Y,in->Y,sizeof(float)*in->size);
  }
  
  else if (!strcmp(type,"XY")) {  
    SizeSignal(out,in->size,YSIG);
    if (in->type == XYSIG) memcpy(out->Y,in->X,sizeof(float)*in->size);
    else for(i=0;i<in->size;i++) out->Y[i] = XSig(in,i);
  }
  
  else if (!strcmp(type,"YX")) {  
    SizeSignal(out,in->size,XYSIG);
    memcpy(out->X,in->Y,sizeof(float)*in->size);
  }

  else if (!strcmp(type,"XX")) {  
    SizeSignal(out,in->size,XYSIG);
    if (in->type == XYSIG) memcpy(out->X,in->X,sizeof(float)*in->size);
    else for(i=0;i<in->size;i++) out->X[i] = XSig(in,i);
  }
  
  else Errorf("CopySigXX() : Bad type '%s'",type);
}


/***********************************************************************************
 *
 *  Parsing a signal
 *
 ***********************************************************************************/

/*
 * Parse an output Signal
 */

char ParseSignalLevel_(LEVEL level, char *arg, SIGNAL defVal, SIGNAL *sig)
{
  char *type;
  float f;
  
  type = TTEvalExpressionLevel_(level,arg,&f,(VALUE *) sig,SignalType,NO,NO,AnyType,YES);

  if (type == NULL) {
    *sig = defVal;
    if (defVal != NULL) (*sig)->nRef++;
    return(NO);
  }
  
  return(YES);
}

char ParseSignal_(char *arg, SIGNAL defVal, SIGNAL *sig)
{
  return(ParseSignalLevel_(levelCur,arg,defVal,sig));
}

void ParseSignalLevel(LEVEL level,char *arg, SIGNAL *sig)
{
  if (ParseSignalLevel_(level,arg,NULL,sig) == NO) Errorf1("");
}

void ParseSignal(char *arg, SIGNAL *sig)
{
  ParseSignalLevel(levelCur,arg,sig);
}

/*
 * Parse an input Signal
 */

char ParseSignalILevel_(LEVEL level, char *arg, SIGNAL defVal, SIGNAL *sig)
{  
  char *type;
  float f;
  
  type = TTEvalExpressionLevel_(level,arg,&f,(VALUE *) sig,SignalType,NO,NO,AnyType,NO);

  if (type == NULL || (*sig)->size == 0) {
    *sig = defVal;
    if (defVal != NULL) {
      if (defVal->size == 0) Errorf("ParseSignalILevel_() : default signal is empty");
      (*sig)->nRef++;
    }
    return(NO);
  }    
  
  return(YES);
}


char ParseSignalI_(char *arg, SIGNAL defVal, SIGNAL *sig)
{
  return(ParseSignalILevel_(levelCur,arg,defVal,sig));
}

void ParseSignalILevel(LEVEL level, char *arg, SIGNAL *sig)
{
  if (ParseSignalILevel_(level,arg,NULL,sig) == NO) Errorf1("");
}

void ParseSignalI(char *arg, SIGNAL *sig)
{
  ParseSignalILevel(levelCur,arg,sig);
}


/*
 * The field list
 */
struct field fieldsSignal[] = {

  "", GetExtractSignalV, SetExtractSignalV, GetExtractOptionsSignalV, GetExtractInfoSignalV,
  "X", GetExtractSignalV, SetExtractSignalV, GetExtractOptionsSignalV, GetExtractInfoSignalV,
  "Y", GetExtractSignalV, SetExtractSignalV, GetExtractOptionsSignalV, GetExtractInfoSignalV,
  "index", GetExtractSignalV, NULL, GetExtractOptionsSignalV, GetExtractInfoSignalV,
  "size", GetSizeSignalV, SetSizeSignalV, NULL, NULL,
  "dx", GetDxSignalV, SetDxSignalV, NULL, NULL,
  "x0", GetX0SignalV, SetX0SignalV, NULL, NULL,
  "name", GetNameSignalV, SetNameSignalV, NULL, NULL,
  "xy", GetXYSignalV, SetXYSignalV, NULL, NULL,
  "sizeAllocX", GetSizeAllocXSignalV, SetSizeAllocXSignalV, NULL, NULL,
  "sizeAllocY", GetSizeAllocYSignalV, SetSizeAllocYSignalV, NULL, NULL,
  "firstp", GetFirstpSignalV, SetFirstpSignalV, NULL, NULL,
  "lastp", GetLastpSignalV, SetLastpSignalV, NULL, NULL,
  "tolistv", GetTolistvSignalV, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};


/*
 * The type structure for SIGNAL
 */

TypeStruct tsSignal = {

  "{{{&signal} {This type is the basic type for signals. Uniformly sampled signals (i.e., Y-signals) \
can be built using the <value1,...,valueN> syntax. The values can be either a float, a signal, a range or a listv of floats, \
signals and ranges. The different operators are \n \
- +,-,*,/ (and +=,-=,*=,/=) : regular operators \n \
- ==,!=,<=,>=,<,> : regular tests \n \
- x^f (and ^=) : each value of |x| is taken to the popwer f \n \
- x*^n : each value of x to the power n where n is a positive integer \n\
- ~ : transposition operator (returns a single columnimage) \n\
- //,% : integer division and remainder \n \
- is,isnot : test if 2 signals correspond or not to the same C object \n \
- sinh,sin,cosh,cos,tanh,tan,acos,asin,atan : trigonometric operators \n \
- min,max : if 1 argument, returns the min or max value of a signal, if 2 arguments returns \
the signal made of the min/max of each value. \n\
- log2,log,ln,sqrt,abs,exp,ceil,floor,round,frac,int : other math functions \n \
- der,prim : derivative and primitive of a signal  \n \
- sum : computes the sum of all signal values \n \
- mean : same as sum but divides by the total number of points\n \
- any : returns 1 if at least one of the values is different from 0\n \
- all : returns 1 if all of the values are different from 0\n \
- find : returns a signal made of indices which correspond to non 0 values \n\
- YSIG Constructors : <...>,Zero,One,I,Grand,Urand \n \
- XYSIG Constructors : XY(xsignal,ysignal). In the ysig expression you can use the 'X' notation which refers to xsig.}} \
{{&signali} {This type corresponds to non empty signals.}}}",  /* Documentation */

  &signalType,       /* The basic (unique) type name */
  GetTypeSignal,     /* The GetType function */                       
  
  DeleteSignal,     /* The Delete function */
  NewSignal,     /* The New function */
  
  CopySig,       /* The copy function */
  ClearSignal,       /* The clear function */
  
  ToStrSignal,       /* String conversion */
  PrintSignal,   /* The Print function : print the object when 'print' is called */
  PrintInfoSignal,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsSignal,      /* The list of fields */
};


