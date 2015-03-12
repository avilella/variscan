/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 1                           */
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




/*********************************************************
 *
 * LISTV contents
 *
 *********************************************************/


#include "lastwave.h"
#include <stdarg.h>
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"



/* Type of a listv */
char *listvType = "&listv";


/*
 * Answers to the different print messages
 */
 
#define LVPrintLength 30

void PrintListv(LISTV lv)
{
  int n;
  
  Printf("%s {\n",listvType);
  for (n=0;n<lv->length;n++) {
    Printf("% 4d :  ",n);
    if (lv->values[n] == NULL) Printf("%g\n",lv->f[n]);
    else {
      Printf("%s\n",ToStrValue(lv->values[n],NO));
    }
  }
  Printf("}\n");
}

char *ToStrListv(LISTV lv,char flagShort)
{
  static char strShort[30];
  char *str,*str1,*str2;
  int n;
  int nAlloc;
  
  if (flagShort) {
    sprintf(strShort,"{length=%d}",lv->length);
    return(strShort);
  }
  
  nAlloc = 300;
  str = CharAlloc(nAlloc);
  TempPtr(str);
  if (LVPrintLength < lv->length) sprintf(str,"{length=%d; ",lv->length);
  else sprintf(str,"{");
  
  for (n=0;n<MIN(lv->length,LVPrintLength);n++) {
    if (lv->values[n] == NULL) {
      sprintf(strShort,"%g",lv->f[n]);
      str1 = strShort;
    }
    else str1 = ToStrValue(lv->values[n],YES);
    if (strlen(str)+strlen(str1) > nAlloc-20) {
      nAlloc += MAX(300,strlen(str1)+20);
      str2 = CharAlloc(nAlloc);
      TempPtr(str2);
      strcpy(str2,str);
      str = str2;
    }
    strcat(str,str1);    
    if (n != MIN(lv->length,LVPrintLength)-1) strcat(str," ");
  }
  if (LVPrintLength < lv->length) strcat(str,"...");
  strcat(str,"}");
  
  return(str);
}

void PrintInfoListv(LISTV lv)
{
  Printf("   length =  %d   [nAlloc=%d]\n",lv->length,lv->nAlloc);
}

/* 
 * Subroutine of SetListvField (just below)
 */
char *SetListv_(LISTV *plv,void **arg)
{
  extern void *SetFieldArg(VALUE value, void **arg);
  char *field = ARG_S_GetField(arg);
  FSIList *fsiList = ARG_S_GetFsiList(arg);
  char *type = ARG_S_GetRightType(arg);      
  float flt = ARG_S_GetRightFloat(arg);   
  VALUE val = ARG_S_GetRightValue(arg);
  char *equal = ARG_S_GetEqual(arg);
  VALUE *pValueRes = ARG_S_GetResPValue(arg);
  FSI_DECL;
  LISTV lv = *plv;
  LISTV lv1;
  int j,k,_iold;
      
  /*************************
   *
   * Case lv = lv1
   *
   *************************/
  if (fsiList == NULL && *equal == '=') {
    if (type != listvType) {
      SetErrorf("Right handside should be a listv");
      return(NULL);
    }
    lv1 = (LISTV) val;
    CopyListv(lv1,*plv);
    return(listvType);
  }
   
  /*************************
   *
   * Case lv[range] *= val
   *
   *************************/

   if (*equal != '=' && *equal != ':' && fsiList != NULL) {
     ARG_S_SetField(arg,NULL);
     ARG_S_SetFsiList(arg,NULL);
     FSI_FOR_START(fsiList);
     if (lv->values[_i] == NULL) {
       if (SetFloatField(&(lv->f[_i]),arg,0)==NULL) return(NULL);
     }
     else {
       if (SetFieldArg(lv->values[_i],arg) == NULL) return(NULL);      
     }
     FSI_FOR_END;
     return(listvType);
   }

  /*************************
   *
   * Case lv += val
   *
   *************************/
   if (*equal != '=' && *equal != ':') {
     
     switch(*equal) {

     case '+':
       if (val == NULL) AppendFloat2Listv(lv,flt);
       else if (GetTypeValue(val) != listvType) AppendValue2Listv(lv,val);
       else AppendListv2Listv(lv,CastValue(val,LISTV));
       return(listvType);

     case '*':
       if (val != NULL) {
         if (GetTypeValue(val) != numType) {
           SetErrorf("The right handside must be an integer");
           return(NULL);
         }
         flt = CastValue(val,NUMVALUE)->f;
       }
       if ((int) flt != flt) { 
         SetErrorf("The right handside must be an integer and not a float");
         return(NULL);
       }
       if (flt<0) { 
         SetErrorf("The right handside must be a positive integer");
         return(NULL);
       }
       lv1 = NewListv();
       CopyListv(lv,lv1);
       SetLengthListv(lv,0);
       MultListv(lv1,(int) flt, lv);
       DeleteListv(lv1);
       return(listvType);
     
     default : 
       SetErrorf("Inadequate syntax '%s'",equal);
       return(NULL);  
     }

   }

  /*************************
   *
   * Case lv[range]= val
   *
   *************************/
   if (*equal == '=') {
   
     /* case val is not a listv */
     if (type != listvType) {
       if (fsiList->nx != 1) {
         SetErrorf("Size of both handsides should match (left size = %d, right size =1)",fsiList->nx);
         return(NULL);
       }
       if (val == NULL) SetListvNthFloat(lv,(int) FSIArray((&(fsiList->fsi[0])),0),flt);
       else SetListvNthValue(lv,(int) FSIArray((&(fsiList->fsi[0])),0),val);
       return(listvType);
     }

     /* case val it is a listv */
     lv1 = CastValue(val,LISTV);
     if (fsiList->nx != lv1->length) {
       SetErrorf("Size of both handsides should match (left size = %d, right size = %d)",fsiList->nx,lv1->length);
       return(NULL);
     }
     FSI_FOR_START(fsiList);
     if (lv1->values[_k] != NULL) SetListvNthValue(lv,_i,lv1->values[_k]);
     else SetListvNthFloat(lv,_i,lv1->f[_k]);
     FSI_FOR_END;
     return(listvType);
   }  
 
   /*************************
   *
   * Case lv[range] := {val}
   *
   *************************/

   if (val == NULL || GetTypeValue(val)!=listvType) {
     SetErrorf("The syntax 'listv :=' should be followed by another 'listv'");
     return(NULL);
   }
   lv1 = CastValue(val,LISTV);
   if (lv1->length > 1) {
     SetErrorf("The syntax is 'listv := ' should be followed by another 'listv' of length 1 or 0");
     return(NULL);
   }   
   if (fsiList == NULL) {
     SetErrorf("Inadequate ':=' syntax");
     return(NULL);
   }
   
   if (lv1->length == 1) {
     if (lv1->values[0] == NULL) {
       FSI_FOR_START(fsiList);
       SetListvNthFloat(lv,_i,lv1->f[0]);
       FSI_FOR_END;
     }
     else {
       FSI_FOR_START(fsiList);
       SetListvNthValue(lv,_i,lv1->values[0]);
       FSI_FOR_END;
     }
     return(listvType);
   }

   /* We start by creating a new listv which is a copy of lv */
   lv1 = NewListv();
   CopyListv(lv,lv1);
   SetLengthListv(lv,lv1->length-fsiList->nx); 
  
    /* Then we peform the set */
    j = 0; /* Index in lv1 */
    k = 0; /* Index in lv */
    _iold = -1;
    FSI_FOR_START(fsiList);
      /* We check the fsiList is sorted */
      if (_i <= _iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        DeleteListv(lv1);
        return(NULL);
      }
      _iold = _i;
      memcpy(lv->values+k,lv1->values+j,(_i-j)*sizeof(VALUE));        
      memcpy(lv->f+k,lv1->f+j,(_i-j)*sizeof(float));        
      k+=_i-j;
      j=_i+1;
    FSI_FOR_END;
    memcpy(lv->values+k,lv1->values+j,(lv1->length-j)*sizeof(VALUE));
    memcpy(lv->f+k,lv1->f+j,(lv1->length-j)*sizeof(float));
    DeleteListv(lv1);
    return(listvType);
}



/*
 * Basic routine to deal with setting Listv fields
 */
void *SetListvField(LISTV *plv,void **arg)
{
  VALUE *pValueRes = ARG_S_GetResPValue(arg);
  char *field = ARG_S_GetField(arg);
  void *res;
  
  ARG_S_SetField(arg,NULL);
  res = SetListv_(plv,arg);
  ARG_S_SetField(arg,field);
  
  if (res == NULL) return(NULL);
  
  if (*pValueRes == NULL) *pValueRes = (VALUE) *plv;
  return(listvType);
}  



/* 
 * Routine to deal with setting of listv
 */

static char *doc = "{[*opt,...] [:]= (<value> | <listv>)} {Get/Set the listv elements}"; 
 
static void *SetExtractListvV(LISTV lv,void **arg)
{
  VALUE *pValueRes;

  /* doc */
  if (lv == NULL) return(doc);

  pValueRes = ARG_S_GetResPValue(arg);

  if (SetListv_(&lv,arg) == NULL) return(NULL);
  if (*pValueRes == NULL) *pValueRes = (VALUE) lv;
  return(listvType);
}


/*
 * The different extract options 
 */

static char *extractOptionsListv[] = {"*nolimit","*bperiodic","*bmirror","*bmirror1",NULL};
enum {
  FSIOptListvNoLimit = FSIOption1,
  FSIOptListvBPer = FSIOption2,
  FSIOptListvBMir = FSIOption3,
  FSIOptListvBMir1 = FSIOption4,
};

static char *doc1 = "{[*opt,...] [:]= (<value> | <listv>)} {Get/Set the element of a listv}"; 
 
 /* 
 * The function that manages extraction of a listv
 */

static void *GetExtractListvV(VALUE val, void **arg)
{
  LISTV lv;
  FSIList *fsiList;
  float *pFlt;
  char **pStr;
  VALUE *pValue;

  int max;
  FSI_DECL;
  BorderType bt;
  char *type;
  LISTV lvResult;

  /* Doc */
  if (val == NULL) return(doc1);

  lv = (LISTV) val;
  fsiList = ARG_G_GetFsiList(arg);
  pFlt = ARG_G_GetResPFloat(arg);
  pStr = ARG_G_GetResPStr(arg);
  pValue = ARG_G_GetResPValue(arg);

  /* case of an empty extraction */
  if (fsiList != NULL && fsiList->nx == 0) {
    *pValue = (VALUE) TNewListv();
    return(listvType);
  }

  if (fsiList->options & FSIOptListvBPer) bt = BorderPer;
  else if (fsiList->options & FSIOptListvBMir) bt = BorderMir;
  else if (fsiList->options & FSIOptListvBMir1) bt = BorderMir1;
  else bt = BorderNone;

  /* Get the max size */
  max = lv->length;

  /*
   * Case the result will be a simple val or a float
   */
  if (fsiList->options & (FSIOptListvBPer | FSIOptListvBMir | FSIOptListvBMir1) && fsiList->nx == 1 ||
      !(fsiList->options & (FSIOptListvBPer | FSIOptListvBMir | FSIOptListvBMir1)) && fsiList->nx1 == 1) {

    _i = (int) FSIArray((&(fsiList->fsi[0])),0);

    /* The loop in the case of *nolimit */
    if (fsiList->options & FSIOptListvNoLimit) {
      FSI_FOR_START(fsiList); 
      if (_i<0 || _i > max-1) continue;
      type=GetListvNth(lv,_i,pValue,pFlt);
      break;       
      FSI_FOR_END;
    }
    
    /* Other cases */
    else {
      switch (bt) {
      case BorderPer : type=GetListvNth(lv,BPER(_i,max),pValue,pFlt); break;
      case BorderMir : type=GetListvNth(lv,BMIR(_i,max),pValue,pFlt); break;
      case BorderMir1 : type=GetListvNth(lv,BMIR1(_i,max),pValue,pFlt); break;
      default : type=GetListvNth(lv,_i,pValue,pFlt);
      }
    }
    
    if (*pValue) {
      AddRefValue(*pValue);
      TempValue(*pValue);
      return(type);
    }
    else return(numType);
  }
  
  /*
   * Case the result will be a listv
   */

  /* Allocation of the result */
  lvResult = TNewListv();
  if (fsiList->options & (FSIOptListvBPer | FSIOptListvBMir | FSIOptListvBMir1)) SetLengthListv(lvResult,fsiList->nx);
  else SetLengthListv(lvResult,fsiList->nx1);
  lvResult->length = 0;

  /* The loop in the case of *nolimit */
  if (fsiList->options & FSIOptListvNoLimit) {
    FSI_FOR_START(fsiList); 
    if (_i<0 || _i > max-1) continue;
    GetListvNth(lv,_i,pValue,pFlt);
    FSI_FOR_END;
  }
  /* The loop in the generic case */
  else {
    FSI_FOR_START(fsiList); 
    switch (bt) {
    case BorderPer : GetListvNth(lv,BPER(_i,max),pValue,pFlt); break;
    case BorderMir : GetListvNth(lv,BMIR(_i,max),pValue,pFlt); break;
    case BorderMir1 : GetListvNth(lv,BMIR1(_i,max),pValue,pFlt); break;
    default : GetListvNth(lv,_i,pValue,pFlt);
    if (*pValue) AppendValue2Listv(lvResult,*pValue);
    else AppendFloat2Listv(lvResult,*pFlt);          
    }
    FSI_FOR_END;
  }
  
  *pValue = (VALUE) lvResult;
        
  return(listvType);
}

/*
 * Get the options for extraction (called for field NULL only)
 */

static char *optionDoc = "{{*nolimit,*bmirror,*bmirror1,*bperiodic} \
{*nolimit : indexes can be out of range} \
{*bperiodic : periodic border effect)} \
{*bmirror : mirror+periodic border effect (first and last elements are repeated)} \
{*bmirror1 : mirror+periodic border effect (first and last elements are NOT repeated)}\
}";

static void *GetExtractOptionsListvV(VALUE val, void **arg)
{
  /* doc */
  if (val == NULL) return(optionDoc);

  return(extractOptionsListv);
}


/*
 * Function to get the ExtractInfo for fields NULL
 */

static void *GetExtractInfoListvV(VALUE val, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  
  LISTV lv = (LISTV) val;
  char *field = ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);
    
  /* If *bperiodic,... then *nolimit must be off */
  if (*options & (FSIOptListvBPer | FSIOptListvBMir | FSIOptListvBMir1)) *options &= ~FSIOptListvNoLimit;
      
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.xmin = 0;
    extractInfo.dx = 1;
    extractInfo.nSignals = 1;
    flagInit = NO;
  }
      
  /* Get the maximum index */
  if (lv->length == 0) {
    SetErrorf("No extraction on empty listv");
    return(NULL);
  }
  else extractInfo.xmax = lv->length-1;
 
  /* '*nolimit' option : set some flags */
  if (*options & (FSIOptListvBPer | FSIOptListvBMir | FSIOptListvBMir1 | FSIOptListvNoLimit)) extractInfo.flags = EIIntIndex;
  else extractInfo.flags = EIErrorBound | EIIntIndex;
     
  return(&extractInfo);
}

/*
 * 'length/size' field
 */

static char *lengthDoc = "{[= <length>]} {Sets/Gets the length/size of a listv}";

static void * GetLengthListvV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(lengthDoc);
  
  return(GetIntField(((LISTV) val)->length,arg));
}

/* Set the length of a listv (perform allocation) */
#define LISTVINCRLENGTH 10   /* If n element are required, we allocate n+LISTVINCRLENGTH */
#define LISTVMINLENGTH 10    /* The minimum number of elements of a listv */
void SetLengthListv(LISTV lv, int length)
{
  int i,l;
  
  if (length <= lv->nAlloc) {
    for (i=lv->length; i<length;i++) {
      if (lv->values[i] != NULL) {
        DeleteValue(lv->values[i]);
        lv->values[i] = NULL;
      }
      lv->f[i] = 0;
    }
  }
  else if (lv->nAlloc != 0) {
    l = length+LISTVINCRLENGTH;
    lv->f = Realloc(lv->f,sizeof(float)*l);
    lv->values = Realloc(lv->values,sizeof(VALUE)*l);
    for (i=lv->length; i<l;i++) {
      lv->values[i] = NULL;
      lv->f[i] = 0;
    }
    lv->nAlloc = l;
  }
  else {
    if (length < LISTVMINLENGTH) l = LISTVMINLENGTH;
    else l = length + LISTVINCRLENGTH;
    lv->f = Calloc(l,sizeof(float));
    lv->values = Calloc(l,sizeof(VALUE));
    lv->nAlloc = l;
  } 
  
  lv->length = length;
}

static void * SetLengthListvV(VALUE val, void **arg)
{
 LISTV lv;
 int length;

  /* doc */
  if (val == NULL) return(lengthDoc);

 lv = (LISTV) val;
 length = lv->length;
  
 if (SetIntField(&length,arg,FieldPositive)==NULL) return(NULL);
 
 SetLengthListv(lv,length);
}

/*
 * 'nAlloc' field
 */

static char *nallocDoc = "{} {Gets the allocation size of a listv}";

static void * GetNAllocListvV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(nallocDoc);
  
  return(GetIntField(((LISTV) val)->nAlloc,arg));
}


/*
 * (Des)Allocation of a Listv
 */


/* Allocate a new listv */
LISTV NewListv(void)
{
  LISTV lv;
  
#ifdef DEBUGALLOC
DebugType = "Listv";
#endif

  lv = Malloc(sizeof(struct listvValue));
  InitValue(lv,&tsListv);
  
  lv->length = 0;
  lv->nAlloc = 0;
  lv->values = NULL;
  lv->f = NULL;
  
  return(lv);
}

LISTV TNewListv(void)
{
  LISTV lv;
  
  lv = NewListv();
  
  TempValue( lv);
  
  return(lv);
}


/* Delete a listv */
void DeleteListv(LISTV lv)
{
  int i;
  
  RemoveRefValue( lv);
  if (lv->nRef>0) return;

  if (lv->f != NULL) Free(lv->f);
  
  if (lv->values != NULL) {
   
    for (i=0;i<lv->length;i++) {
      if(lv->values[i] != NULL) DeleteValue(lv->values[i]);
    }
  
    Free(lv->values);
  }

#ifdef DEBUGALLOC
DebugType = "Listv";
#endif
  
  Free(lv);
}

/* Get the nth element wich should be a number */
float GetListvNthFloat(LISTV lv,int n)
{
  if (n>=lv->length || n< 0) Errorf("GetListvNthFloat() : index out of range (%d)",n);
  
  if (lv->values[n] == NULL) return(lv->f[n]);

  if (GetTypeValue(lv->values[n]) != numType) Errorf("GetListvNthFloat() : Bad element of type '%s'",GetTypeValue(lv->values[n]));
  return(CastValue(lv->values[n],NUMVALUE)->f);
}

char *GetListvNth(LISTV lv,int n,VALUE *val,float *f)
{  
  char *type;
  
  if (n>=lv->length || n< 0) Errorf("GetListvNth() : index out of range (%d)",n);
  
  if (lv->values[n] == NULL) {
    *val = NULL;
    *f = lv->f[n];
    return(numType);
  }
  
  *val = lv->values[n];
  
  type = GetTypeValue(*val);
  
  if (type == numType) *f = ((NUMVALUE) (*val))->f;
  
  return(type);
}


char *GetListvNthStr(LISTV lv,int n)
{
  if (n>=lv->length || n< 0) Errorf("GetListvNthStr() : index out of range (%d)",n);
  
  if (lv->values[n] == NULL) Errorf("GetListvNthStr() : expect a string, not a number");
  if (GetTypeValue(lv->values[n]) != strType) Errorf("GetListvNthStr() : expect a string, not a '%s'",GetTypeValue(lv->values[n]));
  
  return(CastValue(lv->values[n],STRVALUE)->str);
}

void SetListvNthFloat(LISTV lv,int n,float f)
{
  if (n>=lv->length || n < 0) Errorf("GetListvNth() : index out of range (%d)",n);
  
  if (lv->values[n] != NULL) {
    DeleteValue(lv->values[n]);
    lv->values[n] = NULL;
  }
  
  lv->f[n] = f;
  return;
}

void SetListvNthValue(LISTV lv,int n,VALUE val)
{
  extern char TestRecursiveListv(LISTV lv, LISTV lv1);

  if (n>=lv->length || n < 0) Errorf("GetListvNth() : index out of range (%d)",n);
  
  if (GetTypeValue(val) == listvType && TestRecursiveListv(lv,CastValue(val,LISTV)))
    Errorf("Sorry, recursive list are not allowed");
    
  if (lv->values[n] != NULL) {
    DeleteValue(lv->values[n]);
    lv->values[n] = NULL;
  }
  
  lv->values[n] = val;
  AddRefValue(val);
  
  return;
}

int AreEqualListv(LISTV lv1,LISTV lv2)
{
  int i;
  
  if (lv1->length != lv2->length) return(NO);
  
  for (i=0;i<lv1->length;i++) {
    if (lv1->values[i] != lv2->values[i]) return(NO);
    if (lv1->values[i] == NULL) {
      if (lv1->f[i] != lv2->f[i]) return(NO);
    }
    else {
      if (strType == GetTypeValue((VALUE) lv1->values[i])) {
        if (strcmp(CastValue(lv1->values[i],STRVALUE)->str,CastValue(lv2->values[i],STRVALUE)->str)) return(NO);
      }
    }
  }
  
  return(YES);
}

/* Copy a listv */
LISTV CopyListv(LISTV in,LISTV out)
{
  int i;
  
  if (in == out) return(out);
  
  if (out == NULL) out = NewListv();
  
  SetLengthListv(out,in->length);

  for (i=0;i<in->length;i++) {
    if (out->values[i] != NULL) DeleteValue(out->values[i]);
    out->values[i] = in->values[i];
    if (in->values[i]) AddRefValue(in->values[i]);
    else out->f[i] = in->f[i];
  }
  
  return(out);
}

/* Clear a listv */
void ClearListv(LISTV lv)
{
  int i;
  
  if (lv->f != NULL) Free(lv->f);
  
  if (lv->values != NULL) {
   
    for (i=0;i<lv->length;i++) {
      if(lv->values[i] != NULL) DeleteValue(lv->values[i]);
    }
  
    Free(lv->values);
  }

  lv->length = 0;
  lv->nAlloc = 0;
  lv->values = NULL;
  lv->f = NULL;
}

/* Concat 2 listv's */
void ConcatListv(LISTV lv1,LISTV lv2,LISTV lv3)
{
  int i;
  int l1 = lv1->length;
  int l2 = lv2->length;
  int l3 = l1+l2;
  
  SetLengthListv(lv3,l3);

  /* We first copy l2 */
  for (i=l1;i<l3;i++) {
    if (lv2->values[i-l1]) AddRefValue(lv2->values[i-l1]);
    else lv3->f[i] = lv2->f[i-l1];
    if (lv3->values[i] != NULL) DeleteValue(lv3->values[i]);
    lv3->values[i] = lv2->values[i-l1];
  }

  /* Then if necessary we copy l1 */
  if (lv1 != lv3) {
    for (i=0;i<l1;i++) {
      if (lv1->values[i]) AddRefValue(lv1->values[i]);
      else lv3->f[i] = lv1->f[i];
      if (lv3->values[i] != NULL) DeleteValue(lv3->values[i]);
      lv3->values[i] = lv1->values[i];
    }
  } 
}

/* Multiply a listv with an integer */
void MultListv(LISTV lv1, int n, LISTV lv3)
{
  int i,j,k,j0;
  int l1 = lv1->length;
  int l3 = l1*n;
  
/*  if (lv1 == lv3) Errorf("MultListv() : the two listv should be different"); */
  
  SetLengthListv(lv3,l3);

  if (n == 0) return;
  
  if (lv1 == lv3) j0 = 1;
  else j0 = 0;  
  for (j=j0,k=j0*l1;j<n;j++) {
    for (i=0;i<l1;i++,k++) {
      if (lv1->values[i]) AddRefValue(lv1->values[i]);
      else lv3->f[k] = lv1->f[i];
      if (lv3->values[k] != NULL) DeleteValue(lv3->values[k]);
      lv3->values[k] = lv1->values[i];
    }
  }
}

/* Test if lv1 can be added as an element of lv */
char TestRecursiveListv(LISTV lv, LISTV lv1)
{
  int i;
    
  if (lv == lv1) return(YES);
  
  for (i=0;i<lv1->length;i++) {
    if (lv1->values[i]==NULL || GetTypeValue(lv1->values[i]) != listvType) continue;
    if (TestRecursiveListv(lv,CastValue(lv1->values[i],LISTV))) return(YES);
  }
  return(NO);
}

/* Append a Variable Content at the end of a listv */
void AppendValue2Listv (LISTV lv, VALUE val)
{
  if (GetTypeValue(val) == listvType && TestRecursiveListv(lv,CastValue(val,LISTV)))
    Errorf("Sorry, recursive list are not allowed");

  SetLengthListv(lv,lv->length+1);
  val = ValueOf(val);
  lv->values[lv->length-1] = val;
  AddRefValue(val);
}

/* Append a float at the end of a listv */
void AppendFloat2Listv (LISTV lv, float f)
{
  SetLengthListv(lv,lv->length+1);
  lv->values[lv->length-1] = NULL;
  lv->f[lv->length-1] = f;
}

/* Append a float array at the end of a listv */
void AppendFloatArray2Listv (LISTV lv, float *f, int n)
{
  int i,j;
  
  if (n <= 0) return;
  
  j=lv->length;
  SetLengthListv(lv,lv->length+n);
  for (i=0;i<n;i++,j++) {
    lv->values[j] = NULL;
    lv->f[j] = f[i];
  }
}

/* Append an int the end of a listv */
void AppendInt2Listv (LISTV lv, int i)
{
  SetLengthListv(lv,lv->length+1);
  lv->values[lv->length-1] = NULL;
  lv->f[lv->length-1] = i;
}

/* Append a float array at the end of a listv */
void AppendIntArray2Listv (LISTV lv, int *i, int n)
{
  int i1,j1;
  
  j1=lv->length;
  SetLengthListv(lv,lv->length+n);
  for (i1=0;i1<n;i1++,j1++) {
    lv->values[j1] = NULL;
    lv->f[j1] = i[i1];
  }
}

/* Append a string to the end of a listv */
void AppendStr2Listv (LISTV lv, char *str)
{
  STRVALUE sc;
  
  sc = NewStrValue();
  SetStrValue(sc,str);
  SetLengthListv(lv,lv->length+1);
  lv->values[lv->length-1] = (VALUE) sc;
}

/* The result is supposed to contain a list and we just want to append a string */
void AppendStr2Listvf(LISTV lv, char *format,...)
{
Errorf("AppendStr2Listvf pas fait");
}

/* Append a listv to the end of a listv */
void AppendListv2Listv (LISTV lv, LISTV lv1)
{
  int i,j;
  int l1=lv1->length;  
  
  j = lv->length;
  SetLengthListv(lv,lv->length+lv1->length);
  for (i=0;i<l1;i++) {
    if (lv1->values[i] == NULL) {
      lv->values[j] = NULL;
      lv->f[j] = lv1->f[i];
    }
    else {
      if (GetTypeValue(lv1->values[i]) == listvType && TestRecursiveListv(lv,CastValue(lv1->values[i],LISTV))) Errorf("Sorry, recursive list are not allowed");
      lv->values[j] = lv1->values[i];
      AddRefValue(lv1->values[i]);
    }
    j++;
  }
}

extern void ApplyProc2Listv(PROC proc, LISTV lv);

static char _theFlagInverse=0;

static int qsortcmpstr(const void *s1,const void *s2)
{
  switch(_theFlagInverse) {
  case 0:
    return(strcmp((*((STRVALUE *) s1))->str,(*((STRVALUE *) s2))->str));
  default :
    return(strcmp((*((STRVALUE *) s2))->str,(*((STRVALUE *) s1))->str));
  }
}

static int qsortcmpnum(const void *s1,const void *s2)
{
  switch(_theFlagInverse) {
  case 0: 
    return((*((float *) s1)) >= (*((float *) s2)) ? 1 : -1);
  default :
    return((*((float *) s2)) >= (*((float *) s1)) ? 1 : -1);
  }
}

static PROC _theProc=NULL;
static LISTV _theLv= NULL;

static int qsortcmpvc(const void *s1,const void *s2)
{
  _theLv->values[0] = (*((VALUE *) s1));
  _theLv->values[1] = (*((VALUE *) s2));
  
   ApplyProc2Listv(_theProc,_theLv);
   
  switch(_theFlagInverse) {
  case 0:
    return(GetResultInt());
  default :
    return(-GetResultInt());
  }
}

LISTV SortListv(LISTV lv, PROC proc,char flagInverse)
{
  VALUE *vcarray;
  float *farray;
  int i;
  LISTV lv1;
  
  if (lv->length == 0) return(lv);
  
  _theFlagInverse = flagInverse;
  
  lv1 = TNewListv();
  CopyListv(lv,lv1);
  lv = lv1;
    
  if (proc != NULL) {
    vcarray = TMalloc(sizeof(VALUE)*lv->length);
    for (i=0;i<lv->length;i++) {
      vcarray[i] = lv->values[i];
      if (lv->values[i] != NULL && GetTrueTypeValue(lv->values[i]) == numType) {
        lv->f[i] = ((NUMVALUE) (lv->values[i]))->f;
        DeleteValue(lv->values[i]);
        lv->values[i] = NULL;
      }
      if (lv->values[i] == NULL) {
        vcarray[i] = (VALUE) TNewNumValue();
        ((NUMVALUE) (vcarray[i]))->f = lv->f[i];
      }
      else {
        vcarray[i] = lv->values[i];
      }
    }      
    _theProc = proc;
    if (_theLv == NULL) {
      _theLv = NewListv();
      SetLengthListv(_theLv,2);
    }
    qsort(vcarray,lv->length,sizeof(VALUE),&qsortcmpvc);
    for (i=0;i<lv->length;i++) {
      if (GetTypeValue(vcarray[i]) == numType) {
        lv->f[i] = CastValue(vcarray[i],NUMVALUE)->f;
        lv->values[i] = NULL;
      }
      else lv->values[i] = vcarray[i];
    }
    return(lv);
  }  
    
  
  if (lv->values[0] == NULL) {
    farray = TMalloc(sizeof(float)*lv->length);
    for (i=0;i<lv->length;i++) {
      if (lv->values[i] != NULL) break;
      farray[i] = lv->f[i];
    }
    if (i<lv->length) Errorf("Don't know how to sort");
    qsort(farray,lv->length,sizeof(float),&qsortcmpnum);
    for (i=0;i<lv->length;i++) {
      lv->f[i] = farray[i];
    }
    return(lv);
  }
  
  if (GetTypeValue(lv->values[0]) == strType) {
    vcarray = TMalloc(sizeof(VALUE)*lv->length);
    for (i=0;i<lv->length;i++) {
      if (lv->values[i] == NULL) break;
      if (GetTypeValue(lv->values[i]) != strType) break;
      vcarray[i] = lv->values[i];
    }
    if (i<lv->length) Errorf("Don't know how to sort");
    qsort(vcarray,lv->length,sizeof(VALUE),&qsortcmpstr);
    for (i=0;i<lv->length;i++) {
      lv->values[i] = vcarray[i];
    }
    return(lv);
  }
  SetErrorf("SortListv() : Don't know how to sort this listv");
  return(NULL);
}

void PrintListvColumn(LISTV lv,int colSize)
{
  int i,l;
  char *str;
  static NUMVALUE nc = NULL;
  
  if (lv->length == 0) return;
  
  if (nc == NULL) nc = NewNumValue();
    
  /* Computing the size of the columns if 'colSize' is not specified */
  if (colSize <= 0) {
    l = 0;
    for (i=0;i<lv->length;i++) {
      if (lv->values[i] != NULL) {
        if (GetTypeValue(lv->values[i]) == strType) str = ToStrValue(lv->values[i],NO);
        else str = ToStrValue(lv->values[i],YES);
      }
      else {
        nc->f = lv->f[i];
        str = ToStrValue(nc,YES);
      }
      if (*str == '\'' && str[strlen(str)-1] == '\'') {
        str[strlen(str)-1] = '\0';
        str++;
      }
      l = MAX(l,strlen(str));
    }
    colSize = l+2;
  }
  
  /* Then just print !! */
  PrintStrColumn(NULL,colSize);

  for (i=0;i<lv->length;i++) {
    if (lv->values[i] != NULL) {
      if (GetTypeValue(lv->values[i]) == strType) str = ToStrValue(lv->values[i],NO);
      else str = ToStrValue(lv->values[i],YES);
    }
    else {
      nc->f = lv->f[i];
      str = ToStrValue(nc,YES);
    }
    if (*str == '\'' && str[strlen(str)-1] == '\'') {
      str[strlen(str)-1] = '\0';
      str++;
    }
    PrintStrColumn(str,0);
  }
  
  Printf("\n");
}
 

void C_Listv(char **argv)
{
  char *action;
  PROC proc;
  LISTV lv,lv1,lv2;
  float f;
  VALUE val;
  int j,n;
  char flagInverse,opt;
  
  argv = ParseArgv(argv,tWORD,&action,tLISTV,&lv,-1);
  
  if (!strcmp(action,"map")) {
    argv = ParseArgv(argv,tPROC,&proc,0);
    lv1 = TNewListv();
    lv2 = TNewListv();
    SetLengthListv(lv2,1);
    for (j=0;j<lv->length;j++) {
      GetListvNth(lv,j,&val,&f);
      if (val) SetListvNthValue(lv2,0,val);
      else SetListvNthFloat(lv2,0,f);
      ApplyProc2Listv(proc,lv2);
      if (toplevelCur->resultType != NULL)
        AppendValue2Listv(lv1,GetResultValue());
    }
    SetResultValue(lv1);
    return;
  }

  /* sort action */
  if (!strcmp(action,"sort")) {
    argv = ParseArgv(argv,tPROC_,NULL,&proc,-1);
    flagInverse = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'i': flagInverse = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    lv = SortListv(lv,proc,flagInverse);
    if (lv == NULL) Errorf1("");
    SetResultValue(lv);  
    return;  
  }
 
  /* niceprint action */
  if (!strcmp(action,"niceprint")) {
    argv = ParseArgv(argv,tINT_,0,&n,0);
    PrintListvColumn(lv,n); 
    return;
  }
  
  Errorf("Bad action '%s'",action);
}



     
/*
 * The field list
 */
struct field fieldsListv[] = {

  "", GetExtractListvV, SetExtractListvV, GetExtractOptionsListvV, GetExtractInfoListvV,
  "length", GetLengthListvV, SetLengthListvV, NULL, NULL,
  "size", GetLengthListvV, SetLengthListvV, NULL, NULL,
  "nAlloc", GetNAllocListvV, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for LISTV
 */

TypeStruct tsListv = {

  "{{{&listv} {This type corresponds to list of values. \n \
- ==,!= : test equality of all the elements \n \
- Operator * : listv*n, repetition of the listv n times \n \
- Operator + : listv+listv, Appending 2 listv \n \
- Operator + : listv+value, (where value is not a listv), adding the value at the end of the listv  \n \
- Constructor : syntax {value1 ... valueN}.}}}",  /* Documentation */

  &listvType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteListv,     /* The Delete function */
  NewListv,     /* The New function */

  CopyListv,       /* The copy function */
  ClearListv,       /* The clear function */
  
  ToStrListv,       /* String conversion */
  PrintListv,   /* The Print function : print the object when 'print' is called */
  PrintInfoListv,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsListv,      /* The list of fields */
};
