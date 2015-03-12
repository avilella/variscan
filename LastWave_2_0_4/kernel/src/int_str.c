/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 4                           */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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




/*****************************************************************
 *
 *  The String content
 *
 *****************************************************************/  


#include "lastwave.h"
#include <stdarg.h>
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"


 
/* Type of a string */
char *strType = "&string";

/* Types used only for specifying procedure arguments types */
char *listType = "&list";
char *wordType = "&word";
char *wordlistType = "&wordlist";
char *valType = "&val";
char *valobjType = "&valobj";


/*
 * Answers to the different print messages
 */

#define StrShortPrintLength 10
#define StrPrintLength 200
 
void PrintStrContent(STRVALUE sc)
{
  Printf("'%s'\n",sc->str);
}

static void _PrintStr(char *strOut, char *str,int length)
{
  int n;
  char c;
  
  if (strlen(str)<length) sprintf(strOut,"'%s'",str);
  else {
    n = MIN(strlen(str),length-2);
    c = str[n];
    str[n] = '\0'; 
    sprintf(strOut,"'%s...'",str);
    str[n] = c;
  }
}

char *ToStrStrContent(STRVALUE sc, char flagShort)
{
  static char str[StrPrintLength];
  
  if (flagShort) _PrintStr(str,sc->str,StrShortPrintLength);
  else _PrintStr(str,sc->str,StrPrintLength);
  
  return(str);
}

void PrintInfoStrContent(STRVALUE sc)
{
  Printf("   length =  %d\n",strlen(sc->str));
}

/*
 * The different extract options 
 */

static char *extractOptionsStr[] = {"*nolimit","*bperiodic","*bmirror","*bmirror1","*list",NULL};
enum {
  FSIOptStrNoLimit = FSIOption1,
  FSIOptStrBPer = FSIOption2,
  FSIOptStrBMir = FSIOption3,
  FSIOptStrBMir1 = FSIOption4,
  FSIOptStrList = FSIOption5
};
 

/* 
 * The function that manages setting a string using explicit parameters
 */
char *SetStr_(char **strLeft,FSIList *fsiList, float flt, VALUE val,char *equal, char *fieldName)
{
  FSI_DECL;
  int _j,j,k,_iold;
  char *strRight,*str,*type;
  int lRight,lLeft,length;
  LISTV lv;
  static STRVALUE sc1 = NULL;
  static STRVALUE sc2 = NULL;
  
  
  /* Get the type of the right value  */
  if (val == NULL) type = numType;
  else {
    type = GetTypeValue(val);
    if (type == numType) flt = CastValue(val,NUMVALUE)->f;
  }
  

  /*************************
   *
   * Case right value is a float
   *
   *   s *= 3
   *
   *************************/
   
  if (type == numType) {
  
    if (*equal != '*') { 
      SetErrorf("The '<string> %s <number>' syntax is not adequate",equal);
      return(NULL);
    }
    if (fsiList != NULL) { 
      SetErrorf("The '%s' syntax is not adequate on a string[range]",equal);
      return(NULL);
    }
    if (flt != (int) flt) {
      SetErrorf("Right handside of assignement should be an integer");
      return(NULL);
    }
    
    if (sc1 == NULL) {
      sc1 = NewNullStrValue();
      sc2 = NewNullStrValue();
    }
    sc1->str = *strLeft;
    InitStrValue(sc2);
    MultStrValue(sc1, (int) flt,sc2);
    Free(*strLeft);
    *strLeft = sc2->str;
    sc1->str = NULL;
    sc2->str = NULL;
    return(strType);
  }

  /*************************
   *
   * Case right value is a string
   *
   *************************/
  
  if (type == strType) {
   
    /* Get the right handside string and its length */
    strRight = CastValue(val,STRVALUE)->str;
    lRight = strlen(strRight);
    
    /* Get left handside length */
    lLeft = strlen(*strLeft);
    
    /*
     * Case simple assignement 'strLeft... op strRight'
     */
    
    /* 'strLeft op strRight' or 'strLeft[] op strRight' */
    if (fsiList == NULL || fsiList->nx == 0) {
      if (*equal == '=') { /* 'strLeft = strRight' or 'strLeft[] = strRight' */
        if (lLeft >= lRight) strcpy(*strLeft,strRight);
        else {
          Free(*strLeft);
          *strLeft = CharAlloc(MAX(lRight,MinStringSize)+1);
          strcpy(*strLeft,strRight);
        }
      }
      else if (*equal == '+') { /* 'strLeft += strRight' or 'strLeft[] += strRight' */
        str = CharAlloc(MAX(lRight+lLeft,MinStringSize)+1);
        strcpy(str,*strLeft);
        strcpy(str+lLeft,strRight);
        Free(*strLeft);
        *strLeft = str;
      }
      else {
        SetErrorf("The '%s' syntax is not adequate",equal);
        return(NULL);
      }
      return(strType);
    }

    if (*equal != ':' && *equal != '=') {
      SetErrorf("The '%s' syntax is not adequate for operating on strings",equal);
      return(NULL);
    }
        
    /* 
     * Case of an extraction strLeft[1:3,7]=strRight (i.e., SAME SIZE)
     */   
    if (*equal == '=') {
      if (lRight != fsiList->nx) {
        SetErrorf("String lengths do not match");
        return(NULL);
      }
      _j = 0;
      FSI_FOR_START(fsiList); 
      (*strLeft)[_i] = strRight[_j++];
      FSI_FOR_END;
      return(strType);
    } 
  
  
    /* 
     * Case of an extraction s[1:3,7] := "ab" (i.e., NOT SAME SIZE)
     */   
 
    /* In the case s[1,2,6,9] := "a" it is particularly simple */
    if (fsiList->nx == fsiList->size && lRight == 1) {
      FSI_FOR_START(fsiList);
      (*strLeft)[_i]= *strRight;
      FSI_FOR_END;
      return(strType);
    }
  
    /* We first need to compute the new length */
    length = strlen(*strLeft) - fsiList->nx + fsiList->nx*lRight;
    str = CharAlloc(MAX(length,MinStringSize)+1);
  
    /* Then we perform the set */
    j = 0; /* Index in the *strLeft string */
    k = 0; /* Index in the str string */
    _iold = -1;
    FSI_FOR_START(fsiList);
      /* We check the fsiList is sorted */
      if (_i <= _iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        Free(str);
        return(NULL);
      }
      _iold = _i;
      strncpy(str+k,*strLeft+j,_i-j);
      k+=_i-j;
      strcpy(str+k,strRight);
      k+=lRight;
      j=_i+1;
    FSI_FOR_END;
    strcat(str+k,*strLeft+j);
    Free(*strLeft);
    *strLeft = str;
    return(strType);
  }
    
    
  /*************************
   *
   * Case val is a listv
   *
   *
   *************************/

  else if (type == listvType) {

    lv = CastValue(val,LISTV);
    
    if (fsiList == NULL) {
      SetErrorf("Syntax 'string = listv' not valid");
      return(NULL);      
    }
    
    /* Must be of the type s[1:3,4,2:4] = {"a" "bb" "c"} or s[1:3,4,2:4] := {"a"} */
    if (*equal == '=' && lv->length != fsiList->size) {
      SetErrorf("Length of extraction list should be the same as length of &listv");
      return(NULL);
    }
    if (*equal == ':' && lv->length != 1) {
      SetErrorf("Length of right handside &listv should be 1");
      return(NULL);
    }
       
    /* We first need to compute the length of the listv */
    lRight = 0;
    for (j=0;j<lv->length;j++) {
      if (lv->values[j] == NULL || GetTypeValue(lv->values[j]) != strType) {
        SetErrorf("The &listv should contain only &string elements");
        return(NULL);
      }
      lRight += strlen(CastValue(lv->values[j],STRVALUE)->str);
    }
    if (*equal ==':') lRight*=fsiList->size;

    /* Then we go */
    lRight += strlen(*strLeft)-fsiList->nx;
    str = CharAlloc(MAX(lRight,MinStringSize)+1);
  
    /* Then we perform the set */
    j = 0; /* Index in the *strLeft string */
    k = 0; /* Index in the str string */
    _iold = -1;
    FSI_FOR_START1(fsiList);
    if (*equal == ':') strRight = CastValue(lv->values[0],STRVALUE)->str;
    else strRight = CastValue(lv->values[_n],STRVALUE)->str;
    switch(fsi->type) {
    case FSIFloat : 
      _j = (int) fsi->val.f;
      if (_j<=_iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        Free(str);
        return(NULL);
      }
      _iold = _j;
      strncpy(str+k,*strLeft+j,_j-j);
      k+=_j-j;
      strcpy(str+k,strRight);
      k+=strlen(strRight);
      j=_j+1;
      break;
    case FSIRange : 
      _j = (int) fsi->val.r->first;
      if (_j<=_iold) {
        SetErrorf("Sorry, this operation with non sorted indexes is not implemented");
        Free(str);
        return(NULL);
      }
      if (fsi->val.r->step != 1) {
        SetErrorf("Sorry, only ranges with a step of 1 are valid indexes for this operation");
        Free(str);
        return(NULL);
      }
      _iold = _j+fsi->val.r->size-1;
      strncpy(str+k,*strLeft+j,_j-j);
      k+=_j-j;
      strcpy(str+k,strRight);
      k+=strlen(strRight);
      j=_j+fsi->val.r->size;
      break;
    case FSISignal :
      SetErrorf("Sorry, only ranges or numbers are valid indexes for this operation");
      Free(str);
      return(NULL);
    }
    FSI_FOR_END1;
  
    strcat(str+k,*strLeft+j);
    
    Free(*strLeft);
    *strLeft = str;
    return(strType);
  }
  
  else {  
    if (fieldName == NULL) SetErrorf("Cannot assign '%s' to string",GetTypeValue(val));
    else SetErrorf("Cannot assign field '%s' with non string argument (type is '%s')",GetTypeValue(val));
    return(NULL);
  }

}

/*
 * Basic routine to deal with setting a string field (with eventual extraction)
 */

void *SetStrField(char **pstr, void **arg)
{
   char *field = NULL;
   FSIList *fsiList = ARG_S_GetFsiList(arg);
   char *type = ARG_S_GetRightType(arg);      
   float flt = ARG_S_GetRightFloat(arg);   
   VALUE val = ARG_S_GetRightValue(arg);
  char *equal = ARG_S_GetEqual(arg);
   char **pstrRes = ARG_S_GetResPStr(arg);
   
   if (SetStr_(pstr,fsiList,flt,val,equal,field) == NULL) return(NULL);
   *pstrRes = *pstr;
   return(strType);
}

/* 
 * Routine to deal with setting of strings
 */
static char *doc = "{[*opt,...] [:]= (<string> | <listv>)} {Get/Set characters}"; 

static void *SetExtractStrV(VALUE val,void **arg)
{
   char *field;
   FSIList *fsiList;
   char *type;
   float flt;
  char *equal;
   char **pstrRes;
   VALUE val1;
   STRVALUE sc;   

  /* Doc */
  if (val == NULL) return(doc);

  sc = (STRVALUE) val;

  field = ARG_S_GetField(arg);
  fsiList = ARG_S_GetFsiList(arg);
  type = ARG_S_GetRightType(arg);      
  flt = ARG_S_GetRightFloat(arg);   
  val1 = ARG_S_GetRightValue(arg);
  equal = ARG_S_GetEqual(arg);
  pstrRes = ARG_S_GetResPStr(arg);
   
  if (SetStr_(&(sc->str),fsiList,flt,val1,equal,field) == NULL) return(NULL);
  *pstrRes = sc->str;
  if (sc->list) {
    DeleteList(sc->list);
    sc->list = NULL;
  }
  return(strType);
}

/*
 * Basic routine to deal with getting a string field
 */

static void *GetStrFieldExtract_(char *str, void **arg,STRVALUE sc)
{
  char **list;
  int max,sizeList;
  FSI_DECL;
  BorderType bt;
  
  char *str1,*str2,*str0;
  FSIList *fsiList;
  float *pFlt;
  char **pStr;
  char **list1,**list2;
  VALUE *pValue;  
  
  fsiList = ARG_G_GetFsiList(arg);
  pFlt = ARG_G_GetResPFloat(arg);
  pStr = ARG_G_GetResPStr(arg);
  pValue = ARG_G_GetResPValue(arg);  
 
  /* case of no extraction and no field */
  if (fsiList == NULL) {
    *pStr = str;
    return(strType);
  }
  
  /* case of an empty extraction */
  if (fsiList != NULL && fsiList->nx == 0) {
    *pValue = (VALUE) TNewStrValue();
    return(strType);
  }
  
  if (fsiList->options & FSIOptStrBPer) bt = BorderPer;
  else if (fsiList->options & FSIOptStrBMir) bt = BorderMir;
  else if (fsiList->options & FSIOptStrBMir1) bt = BorderMir1;
  else bt = BorderNone;

  /*
   * Case the '*list' option is off 
   */
  if (!(fsiList->options & FSIOptStrList)) {

    /* Allocation of the result */
    if (!(fsiList->options & (FSIOptStrBPer | FSIOptStrBMir | FSIOptStrBMir1))) *pStr = StrValueStrAlloc(fsiList->nx1);
    else *pStr = StrValueStrAlloc(fsiList->nx);
        
    /* Get the max size */
    max = strlen(str);

    /* The loop in the case of *nolimit */
    if (fsiList->options & FSIOptStrNoLimit) {
      FSI_FOR_START1(fsiList); 
      if (fsi->type == FSIRange && fsi->val.r->step == 1) {
        if (RangeFirst(fsi->val.r) >= max || RangeLast(fsi->val.r) < 0) continue;
        memcpy((*pStr)+_k,str+(int) (MAX(RangeFirst(fsi->val.r),0)),((int) (MIN(RangeLast(fsi->val.r),max-1)-MAX(RangeFirst(fsi->val.r),0)+1))*sizeof(char));
        _k+= (int) (MIN(RangeLast(fsi->val.r),max-1)-MAX(RangeFirst(fsi->val.r),0)+1);
      }
      else {
        FSI_FOR_START2(fsiList); 
        if (_i<0 || _i > max-1) continue;
        (*pStr)[_k] = str[_i];       
        FSI_FOR_END2;
      }
      FSI_FOR_END1;
    }
    /* The loop in the generic case */
    else {
      /* ????? AMELIORATION : utiliser memcpy pour les effets de bords aussi */
      FSI_FOR_START1(fsiList); 
      if (fsi->type == FSIRange && fsi->val.r->step == 1 && RangeFirst(fsi->val.r) >= 0 && RangeLast(fsi->val.r) < max) {
        memcpy((*pStr)+_k,str+(int) (RangeFirst(fsi->val.r)),((int) (RangeLast(fsi->val.r)-RangeFirst(fsi->val.r)+1))*sizeof(char));
        _k+= (int) (MIN(RangeLast(fsi->val.r),max-1)-MAX(RangeFirst(fsi->val.r),0)+1);
      }
      else {
        FSI_FOR_START2(fsiList); 
        switch (bt) {
        case BorderPer : (*pStr)[_k] = str[BPER(_i,max)]; break;
        case BorderMir : (*pStr)[_k] = str[BMIR(_i,max)]; break;
        case BorderMir1 : (*pStr)[_k] = str[BMIR1(_i,max)]; break;
        default : (*pStr)[_k] = str[_i];
        }
        FSI_FOR_END2;
      }
      FSI_FOR_END1;
    }
    

    (*pStr)[_k] = '\0';
        
    /* We want to return a STRVALUE so that we do not need to temp the string */
    *pValue = (VALUE) NewNullStrValue();
    ((STRVALUE) *pValue)->str = *pStr;
    TempValue(*pValue);
    *pStr = NULL;
    return(strType);
  }


  /*
   * Case the '*list' option is  on 
   */
  else {
    /* Get the list and its size */
    if (sc == NULL) {
      SetErrorf("*list option not yet implemented in this case... sorry!");
      return(NULL);
    }

    list = GetListFromStrValue(sc);
    if (list == NULL) return(NULL);
    max = GetListSize(list);  
        
    /* 
     * First we need to know how much room the result needs 
     */
    sizeList = 0;

    /* The loop in the case of *nolimit */
    if (fsiList->options & FSIOptStrNoLimit) {
      FSI_FOR_START(fsiList); 
      if (_i < 0 || _i>max-1) continue; 
      sizeList += strlen(list[_i])+5;
      FSI_FOR_END;
    }
    /* The loop in the generic case */
    else {
      FSI_FOR_START(fsiList); 
      switch (bt) {
      case BorderPer : sizeList += strlen(list[BPER(_i,max)])+5; break;
      case BorderMir : sizeList += strlen(list[BMIR(_i,max)])+5; break;
      case BorderMir1 : sizeList += strlen(list[BMIR1(_i,max)])+5; break;
      default : sizeList += strlen(list[_i])+5;
      }
      FSI_FOR_END;
    }

    /* 
     * Allocation of the result 
     */
    sizeList = MAX(MinStringSize,sizeList)+1;
    str = CharAlloc(sizeList);
    str[0] = '\0';
    if (fsiList->nx1 >= 1) {
      str2 = str1 = CharAlloc(sizeList);
      str1[0] = '\0';
      list2 = list1 = Malloc(sizeof(char *)*(_k+1));
      *list1 = NULL;
    }
    else list2 = list1 = NULL;
        
    /* 
     * Then we set the result ?????? se souvenir de la liste !! + faire optmisation si pas de *nolimit 
     */

    /* The loop in the case of *nolimit */
    FSI_FOR_START(fsiList); 
      if (fsiList->options & FSIOptStrNoLimit) {
        if (_i < 0 || _i>max-1) continue; 
        str0 = list[_i];
      }    
      else {
        switch (bt) {
        case BorderPer : str0 = list[BPER(_i,max)]; break;
        case BorderMir : str0 = list[BMIR(_i,max)]; break;
        case BorderMir1 : str0 = list[BMIR1(_i,max)]; break;
        default : str0 = list[_i];
        }
      }
      if (list2 != list1) strcat(str," ");
      strcat(str,str0);
      if (list2 != NULL) {
        strcpy(str2,str0);
        *list2 = str2;
        list2++;
        *list2 = NULL;
        str2 += strlen(str2)+1;
      }
    FSI_FOR_END;

    /* We want to return a STRVALUE so that we do not need to temp the string */
    *pValue = (VALUE) NewNullStrValue();
    ((STRVALUE) *pValue)->str = str;
    ((STRVALUE) *pValue)->list = list1;
    TempValue(*pValue);
    return(strType);
  }
}

void *GetStrFieldExtract(char *str, void **arg)
{
  return(GetStrFieldExtract_(str, arg,NULL));
}

void *GetStrField(char *str, void **arg)
{
  char *field =  ARG_S_GetField(arg);
  FSIList *fsiList = ARG_S_GetFsiList(arg);
  void *res;
  
  ARG_S_SetField(arg,NULL);
  ARG_S_SetFsiList(arg,NULL);
 
  res = GetStrFieldExtract(str,arg);

  ARG_S_SetField(arg,field);
  ARG_S_SetFsiList(arg,fsiList);
  
  return(res);
}


/*
 * Get and extraction
 */
      
static void *GetExtractStrV(VALUE val, void **arg)
{
  STRVALUE sc;
  char *str;
  
   /* Doc */
  if (val == NULL) return(doc);
 
  sc = (STRVALUE) val;
  str = sc->str;

  return(GetStrFieldExtract_(str,arg,sc));
}


/*
 * Get the options for extraction (called for field NULL only)
 */

static char *optionDoc = "{{*list,*nolimit,*bconst,*bmirror,*bmirror1,*bperiodic} \
{*list : the string is considered as a list and all the extractions are performed using the list representation} \
{*nolimit : indexes can be out of range} \
{*bconst : border effect with same characters (last character for right handside and first character for left handside)} \
{*bperiodic : periodic border effect)} \
{*bmirror1 : mirror+periodic border effect (first and last points are repeated)} \
{*bmirror : mirror+periodic border effect (first and last points are NOT repeated)}\
}";

static void *GetExtractOptionsStrV(VALUE val, void **arg)
{
  /* Doc */
  if (val == NULL) return(optionDoc);

  return(extractOptionsStr);
}

/*
 * Function to get the ExtractInfo for fields NULL
 */

static void *GetExtractInfoStrV(VALUE val, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  char **list;
  STRVALUE sc = (STRVALUE) val;
  char *str = sc->str;
  char *field = ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);

  if (field != NULL) return(NULL);
            
  /* If *bperiodic,... then *nolimit must be off */
  if (*options & (FSIOptStrBPer | FSIOptStrBMir | FSIOptStrBMir1)) *options &= ~FSIOptStrNoLimit;
      
  /* Init of the extraction info */
    extractInfo.xmin = 0; /* ??????? */
  if (flagInit) {
    extractInfo.xmin = 0;
    extractInfo.dx = 1;
    extractInfo.nSignals = 1;
    flagInit = NO;
  }
      
  /* '*list' is off : Get the maximum index */
  if (!(*options & FSIOptStrList)) {
    extractInfo.xmax = strlen(str);
    if (extractInfo.xmax != 0) extractInfo.xmax--;
  }
      
  /* '*list' is on  : Get the maximum index */      
  else {
    list = GetListFromStrValue(sc);
    if (list == NULL) return(NULL);
    extractInfo.xmax = GetListSize(list);
    if (extractInfo.xmax != 0) extractInfo.xmax--;
  }
 
  /* '*nolimit' option : set some flags */
  if (*options & (FSIOptStrBPer | FSIOptStrBMir | FSIOptStrBMir1 | FSIOptStrNoLimit)) extractInfo.flags = EIIntIndex;
  else extractInfo.flags = EIErrorBound | EIIntIndex;
     
  return(&extractInfo);
}


/*
 * 'length' field
 */

static char *lengthDoc = "{} {Get the number of characters of the string}";
static void * GetLengthStrV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(lengthDoc);
  
  return(GetIntField(strlen(((STRVALUE) val)->str),arg));
}

/*
 * 'llength' field
 */
static char *llengthDoc = "{} {Get the number of elements of the string considered as a list}";
static void * GetLLengthStrV(VALUE val, void **arg)
{
  char **list;
  
  /* Documentation */
  if (val == NULL) return(llengthDoc);

  list = GetListFromStrValue((STRVALUE) val);
  if (list == NULL) return(NULL);

  return(GetIntField(GetListSize(list),arg));
}

/*
 * 'tonum' field
 */
static char *tonumDoc = "{} {Performs a conversion to a number if possible. Otherwise it returns 'null'}";
static void * GetToNumStrV(VALUE val, void **arg)
{
  float f;
  char *end;
  
  /* Documentation */
  if (val == NULL) return(tonumDoc);

  f = strtod(((STRVALUE) val)->str,&end);
  
  if (*end != '\0') return(GetValueField(nullValue,arg));
    
  return(GetFloatField(f,arg));
}




/*
 * Desallocation of a string content
 */
 
void DeleteStrValue(STRVALUE sc)
{
  RemoveRefValue(sc);
  if (sc->nRef>0) return;
  if (sc->str != NULL) {
    Free(sc->str);
    sc->str = NULL;
  }
  if (sc->list != NULL) {
    DeleteList(sc->list);
    sc->list = NULL;
  }

#ifdef DEBUGALLOC
DebugType = "StrValue";
#endif
  Free(sc);
}   


/*
 * Alloc space for storing a string of size 'n' in a str content 
 */
char * StrValueStrAlloc(int n)
{
  return(CharAlloc((MAX(MinStringSize,n)+1)));
}

/*
 * Init Null string Content
 */
void InitNullStrValue(STRVALUE sc) 
{
  extern TypeStruct tsStr;
  
  InitValue(sc,&tsStr);
  
  sc->str = NULL;
  sc->list = NULL;
}

/*
 * Init string Content
 */
void InitStrValue(STRVALUE sc) 
{
  InitNullStrValue(sc);
  
  sc->str = Malloc((MinStringSize+1)*sizeof(char));
  sc->str[0] = '\0';
}

 
/*
 * Allocation of a string content with nothing in it
 */
 
STRVALUE NewNullStrValue(void)
{
  STRVALUE sc;

#ifdef DEBUGALLOC
DebugType = "StrValue";
#endif
  
  sc = Malloc(sizeof(struct strValue));
  
  
  InitNullStrValue(sc);
  
  return(sc);
}

/*
 * Allocation of a string content with "" in it
 */
 
STRVALUE NewStrValue(void)
{
  STRVALUE sc;

#ifdef DEBUGALLOC
DebugType = "StrValue";
#endif
  
  sc = Malloc(sizeof(struct strValue));
  
  InitStrValue(sc);
  
  return(sc);
}

STRVALUE TNewStrValue(void)
{
  STRVALUE sc = NewStrValue();
  
  TempValue( sc);
  
  return(sc);
}


void CopyStrValue(STRVALUE in, STRVALUE out)
{
  if (strlen(in->str) <= MinStringSize || strlen(in->str)<=strlen(out->str)) strcpy(out->str,in->str);
  else {
    Free(out->str);
    out->str = CopyStr(in->str);
  }
  
  if (out->list) {
    DeleteList(out->list);
    out->list = NULL;
  }
  if (in->list) out->list = CopyList(in->list);  
}


/* 
 * Set a string content with a string (make a copy)
 */

void SetStrValue(STRVALUE sc, char *str)
{ 
  if (sc->str == NULL) sc->str = str;
  else if (strlen(str) <= MinStringSize || strlen(str)<=strlen(sc->str)) strcpy(sc->str,str);
  else {
    Free(sc->str);
    sc->str = CopyStr(str);
  }
  
  if (sc->list) {
    DeleteList(sc->list);
    sc->list = NULL;
  }  
}

/*
 * Get a string from a string content
 */
 
char *GetStrFromStrValue(STRVALUE sc)
{ 
  return(sc->str);
}


/*
 * Get a list from a str content
 */

extern int ParseListBegEnd(char *theLine, char ***beg, char ***end);
 
char **GetListFromStrValue(STRVALUE sc)
{
  int n;
  char **beg,**end;
    
  if (sc->list) return(sc->list);

  n = ParseListBegEnd(sc->str,&beg,&end);
  if (n == -1) return(NULL);
  
  sc->list = BegEndStr2List(beg,end);

  return(sc->list);
}

/* 
 * Concat two string contents
 */
void ConcatStrValue(STRVALUE sc1,STRVALUE sc2,STRVALUE sc3)
{
  char *str;
  int l3,l1,l2;
  
  l1 = strlen(sc1->str);
  l2 = strlen(sc2->str);
  l3 = l1+l2;
  l3 = MAX(l3,MinStringSize);
  
  str = CharAlloc(l3+1);
  
  strcpy(str,sc1->str);
  strcpy(str+l1,sc2->str);
  
  if (sc3->str) Free(sc3->str);

  sc3->str = str;
}


/* Multiply a string with a number */
void MultStrValue(STRVALUE sc1, int n, STRVALUE sc3)
{
  char *str,*str1;
  int j;
  int l3,l1 = strlen(sc1->str);

  if (n == 0) {
    SetStrValue(sc3,"");
    return;
  }
  
  l3 = l1*n;
  l3 = MAX(l3,MinStringSize);

  str = CharAlloc(l3+1);
  
  for (j=0,str1 = str;j<n;j++,str1+=l1) strcpy(str1,sc1->str);

  if (sc3->str) Free(sc3->str);

  sc3->str = str;
}


/* 
 * Getting the string value of a variable named 'name' at a given level 'level'
 *    (generate an error if not the right type)
 */ 

char * GetStrVariableLevel(LEVEL level,char *name)
{
  VARIABLE var;
  VALUE val;
  STRVALUE sc;
  
  var = GetVariableLevel(level,name);
  val = ValueOf((VALUE) var);
  if (GetTypeValue(val) != strType) Errorf("GetStrVariableLevel() : Bad variable type '%s'",GetTypeValue(val));
  sc = (STRVALUE) val;
  
  return(GetStrFromStrValue(sc));
} 

char * GetStrVariable(char *name)
{
  return(GetStrVariableLevel(levelCur,name));
}

/* 
 * Setting a str content variable at a given level (creates it if it does not exist)
 * and sets it with string 'value'
 */ 
 
void SetStrVariableLevel(LEVEL level,char *name,char *value)
{
  VARIABLE v;
  HASHTABLE t;
  char *left,flag,*type;
  VALUE *pcont;
  
  while (level->levelVar != level) level = level->levelVar;
  t = level->theVariables;
  
  /* Get the variable (eventually creates it) */
  v = GetVariableHT(&t, YES, name, &left, &flag);
  if (v==NULL || flag != 0 || *left != '\0') Errorf1("");

  /* Get a pointer to the content and the type */
  pcont =  GetVariablePContent(v, NO);
  type = GetTypeValue(*pcont);
  
  /* Check overwriting */
  if (!DoesTypeOverwrite(type,strType)) 
    Errorf("SetStrVariableLevel() : Cannot overwrite variable '%s' of type '%s' with '%s' typed value",name,type,strType);

  /* If the content is a strType can we use it ? */
  if (type == strType && (*pcont)->nRef == 1) {
    SetStrValue((STRVALUE) (*pcont),value);
    return;
  } 
  
  /* Delete the content */
  DeleteValue(*pcont);

  /* Create it */
  *pcont = (VALUE) NewStrValue();
  
  /* Set it */
  SetStrValue((STRVALUE) (*pcont),value);
}

void SetStrVariable(char *name,char *value)
{
  SetStrVariableLevel(levelCur,name,value);
}  


/*
 * Setting a string variable using a printf format (max length is 10000 !!)
 */

void SetStrVariablef(char *name,char *format, ...)
{
  va_list ap;
  char str[10000];
   
  va_start(ap,format);
  vsprintf(str,format,ap);
  va_end(ap);

  SetStrVariable(name,str);
}


/*
 * Setting a string variable with a list
 */

void SetStrVariableListLevel(LEVEL level,char *name,char **list,char flagBracket)
{
  VARIABLE v;
  HASHTABLE t;
  char *left,flag,*type;
  VALUE *pcont,value;
  STRVALUE sc;
  int size,l;
  char **list1 = list;
  
  while (level->levelVar != level) level = level->levelVar;
  t = level->theVariables;
  
  /* Get the variable (eventually creates it) */
  v = GetVariableHT(&t, YES, name, &left, &flag);
  if (v==NULL || flag != 0 || *left != '\0') Errorf1("");

  /* Get a pointer to the content and the type */
  pcont =  GetVariablePContent(v, NO);
  value = *pcont;
  type = GetTypeValue(value);
  
  /* Check overwriting */
  if (!DoesTypeOverwrite(type,listType)) 
    Errorf("SetStrVariableListLevel() : Cannot overwrite variable '%s' of type '%s' with '%s' typed value",name,type,listType);

  /* If the content is a strType can we use it ? */
  if (type == strType && (*pcont)->nRef == 1) {}
  else {
    /* Delete the content */
    DeleteValue(*pcont);
    /* Create it */
    *pcont = (VALUE) NewStrValue();
    value = *pcont;
  }

  sc = (STRVALUE) value;
  
  
  /* Computing the size the result will take */
  l = 0;
  size = 0;
  list1 = list;
  while(*list1 != NULL) {
    size += strlen(*list1) + 3;
    list1++;
  }
   
  /* Allocation ? */
  if (size > l && size > MinStringSize-1) {
    Free(sc->str);
    sc->str = StrValueStrAlloc(size);
  }
  if (sc->list) {
    Free(sc->list);
    sc->list = NULL;
  }

  /* Setting the result ???????? optimiser */
  sc->str[0] = '\0';
  list1 = list;
  while(*list1 != NULL) {
    if (!flagBracket || **list1 != '\0' && !IsList(*list1)) strcat(sc->str,*list1);
    else {
      strcat(sc->str,"{");
      strcat(sc->str,*list1);
      strcat(sc->str,"}");
    }
    if (*(list1+1) != NULL) strcat(sc->str," ");
    list1++;
  }
  sc->list = CopyList(list);
  
}


void SetStrVariableList(char *name,char **list, char flagBracket)
{
  SetStrVariableListLevel(levelCur,name,list,YES);
}


 
/******************************************
 * 
 * The string command and string regexp
 *
 ******************************************/




/*
 * Subroutine pour le matching d'une chaine par une regexp
 *
 * Cherche a matcher le debut de la chaine et renvoie 
 * la chaine matchee la plus grande possible !
 *
 * renvoie le dernier caractere non matche
 */

static char *start;
static char *end;
 
static char *MatchStr_(char *str,char *expr)
{
  int i;
  char *expr1,*expr0,*str1,*res;
  static char tok[100];
  static char c;
  char flag0Repeat,flag1Repeat,flag01Repeat,flagInclude;
  char *start1,*end1;
  
  while (1) {

    switch(*expr) {

    /* end of string */
    case '$' :  
      if (str[0] != '\0') return(NULL);
      if (expr[1] != '\0') Errorf("MatchStr_ : the character '$' must be at the end of the regexp");
      return(str);
    

    /* Case of a '*' character */
    case '*' : 
      switch (expr[1]) {
      case '\0' : return(str+strlen(str));        
      default :
        i = strlen(str);
        while(1) {
          start1 = start;
          end1 = end;
          res = MatchStr_(str+i,expr+1);
          if (res) return(res);
          start = start1;
          end = end1;
          if (i == 0) return(NULL);
          i--;
        }    
      }

    /* Case of a '+' character */
    case '+' : 
      switch (expr[1]) {
      case '\0' : 
        if (*str == '\0') return(NULL);
        return(str);
      default : 
        i = strlen(str);
        while(1) {
          res = MatchStr_(str+i,expr+1);
          start1 = start;
          end1 = end;
          if (res) return(res);
          start = start1;
          end = end1;
          if (i == 1) return(NULL);
          i--;
        }    
      }

    /* Case of a "[", "[*", "[^" and "[*^" string */
    case '[' :
    
      /* Get the flags */
      expr0 = expr;     /* expr0 --> '[' */
      expr++;
      flag0Repeat = flag1Repeat = flag01Repeat = NO;
      switch (*expr) {
      case '*' : flag0Repeat = YES; expr++; break;
      case '#' : flag01Repeat = YES; expr++; break;
      case '+' : flag1Repeat = YES; expr++; break;
      }
      if (*expr == '^') {flagInclude = NO; expr++;}
      else flagInclude = YES;
      
      /* Get the set of characters */
      expr1 = expr;     /* expr1 --> first character */
      if (*expr == ']') expr++;
      while (*expr != ']' && *expr != '\0') expr++;  /* expr --> ] */
      if (*expr == '\0') Errorf("MatchStr_() : Missing matching '[' in regexp '...%s'",expr0);
      if (expr-expr1>90) Errorf("MatchStr_() : regexp between [] is too long");
      strncpy(tok,expr1,expr-expr1);
      tok[expr-expr1] = '\0';
      expr++;
      
      if (*str == '\0') {
        if (flag0Repeat || flag01Repeat) continue;
        return(NULL);
      }
      
      if (!flag0Repeat && !flag01Repeat) {
        if (flagInclude) {
          if (strchr(tok,*str) == NULL) return(NULL);          
        }
        else {
          if (strchr(tok,*str) != NULL) return(NULL);          
        }
        str++;
        if (!flag1Repeat) continue;
        flag0Repeat = YES;
      }

      /* We must look for all the characters it could match */
      str1 = str;
      if (flag0Repeat) {
        if (flagInclude) while(*str1 != '\0' && strchr(tok,*str1)) str1++;
        else while(*str1 != '\0' && !strchr(tok,*str1)) str1++;
      }
      else {
        if (flagInclude && *str1 != '\0' && strchr(tok,*str1)) str1++;
        else if (!flagInclude && *str1 != '\0' && !strchr(tok,*str1)) str1++;
      }
        
      /* Then call recursively */
      while (str<=str1) {
        start1 = start;
        end1 = end;
        res = MatchStr_(str1,expr);
        if (res) return(res);
        start = start1;
        end = end1;
        str1--;
      }
      return(NULL);

    /* Case of a '?' character */
    case '?' :
      if (str[0] == '\0') return(NULL);
      str++;
      expr++;
      continue;

    /* Case of a '|' character */
    case '|' :
      if (start == NULL) start = str;
      else if (end == NULL) {
        end = str;
      }
      else Errorf("MatchStr_ : Too many '|' (there must be at most 2 of them"); 
      expr++;
      continue;
    

    /* ! */
    case '!' :
      expr++;
      while (*str != '\0' && *expr != '\0' && *expr != '!') {
        if (*expr == '\\' && expr[1] == '!') expr++;
        if (*str != *expr) return(NULL);
        expr++;
        str++;
        continue;
      }
      if (*expr == '\0') Errorf("MatchStr_ : bad regexp, missing a '!'");
      if (*expr != '!') return(NULL);
      expr++;
      continue;

    /* End of expr */
    case '\0' :
      return(str);    
    
    /* Match a character */    
    default :
      if (*str != *expr) return(NULL);
      str++;
      expr++;
      continue;
    }
    
  }
}

LISTV MatchStrN(char *str,char *expr, int nOcc)
{
  char *res,*str0;
  LISTV lv;
  RANGE rg;
  char *expr0;
  
  expr0 = expr;
    
  if (*expr == '^') expr++;

  str0 = str;
  lv = TNewListv();

  while (1) {
    if (*str == '\0') return(lv);

    /* To go a little faster */
    if (*expr0 != '|' && *expr0 != '^' && *expr != '\0' && *expr != '[' && *expr != '*' && *expr != '?'  && *expr != '+' && *expr != '$') {
      if (*expr == '!') res = strchr(str,expr[1]);
      else res = strchr(str,*expr);
      if (res == NULL) return(lv);
      str = res;
    }

    start = end = NULL;
    res = MatchStr_(str,expr);
    if (start == NULL) start = str;
    if (end == NULL) end = res;
    if (end == start+1 || end == start) {
      AppendFloat2Listv(lv,start-str0);
      nOcc--;
    }
    else if (end>start) {
      rg = TNewRange();
      rg->first = start-str0;
      rg->size = end-start;
      AppendValue2Listv(lv,(VALUE) rg);
      nOcc--;
    }
    if (nOcc == 0) return(lv);    
    if (str[1] == '\0' || *expr0 == '^') return(lv);
    if (res == NULL || res <= str) res = str+1;
    str = res;
  }
}    

char MatchStr(char *str,char *expr)
{
  char *str1;
  char *f;
  
  SetTempAlloc();
  str1 = CharAlloc(strlen(expr)+2); /* ????? bof */
  TempPtr(str1);
  strcpy(str1,expr);
  str1[strlen(expr)]='$';
  str1[strlen(expr)+1]='\0';
  f = MatchStr_(str,str1);
  ClearTempAlloc();
  return(f!=NULL);  
}

  
LISTV FindStr(char *str, char *subStr, int nOcc)
{
  LISTV lv;
  char *str0;
  int n;
  RANGE rg;
  
  str0 = str;
  n = strlen(subStr);
  lv = NewListv();
  
  while (1) {
    
    while (1) {
      str = strchr(str,*subStr);
      if (str == NULL) break;
      if (!strncmp(str,subStr,strlen(subStr))) break;
      str++;
    }

    if (str==NULL) return(lv);

    if (n == 1) {
      AppendFloat2Listv(lv,str-str0);
      nOcc--;
    }
    else {
      rg = NewRange();
      rg->first = str-str0;
      rg->size = n;
      AppendValue2Listv(lv, (VALUE) rg);
      DeleteValue(rg);
      nOcc--;
    }
    
    if (nOcc == 0) return(lv);
    
    str += n;
    
  }
}


/************************************************
 *
 * Useful functions on strings
 *
 ************************************************/

/*
 * Managing allocations of strings
 */

/* Delete a string */ 
void DeleteStr(char *str)
{
  Free(str);
}

/* Copy a string */ 
char *CopyStr(char *str)
{
  char *strOut;
  
  if (str == NULL) Errorf("CopyStr() : cannot copy NULL string");
  
  strOut = CharAlloc(strlen(str)+1);
  strcpy(strOut,str);
  return(strOut);
}

/* Copy a string and make it temporary */ 
char *TCopyStr(char *str)
{
  char *strOut = CopyStr(str);
  TempStr(strOut);
  return(strOut);
}


 /* Make a string temporary */
void TempStr(char *str)
{
  TempPtr(str);
}

/* ??? */
#define LINELENGTH 80

 void PrintStrColumn(char *str,int size)
{
  static int countLine;
  static int terminalLength = LINELENGTH;
  static int tab = 0;
  int length;
  
  if (str == NULL) {
    if (size <= 0) Errorf("PrintStrColumn() : size must be strictly positive");
    tab = size;
    countLine = 0;
    return;
  }
  
  if (tab == 0) Errorf("PrintStrColumn() : you should initialize the call first");

  length = tab * (1 + (int) strlen(str) / tab);
  if (countLine+length >= terminalLength) {
     Printf("\n");
     countLine = 0;
  }
  countLine += length;
    
  Printf("%s",str);
  length -= strlen(str);
  while (length--!=0) Printf(" ");
}


/************************************************
 *
 * Command on strings
 *
 ************************************************/

void C_Str(char **argv)
{
  char *action,*str,*str1,*str3;
  int i,n;
  char c;
  LISTV lv;
  float f;
  extern float GetListvNthFloat(LISTV lv,int n);

  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if (!strcmp(action,"2ascii")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    lv = TNewListv();
    for (i=0;i<strlen(str);i++) AppendInt2Listv(lv,(int) str[i]);
    SetResultValue(lv);
    return;
  } 

  /* ascii2 action */
  else if (!strcmp(action,"ascii2")) {
    argv = ParseArgv(argv,tLISTV,&lv,0);
    for (i=0;i<lv->length;i++) {
      f = GetListvNthFloat(lv,i);
      if (f != (int) f || f < 0 || f >=256) 
        Errorf("Bad ascii code '%g'",f);
      AppendResultf("%c",(char) f);
    }
    return;
  } 

  /* match action */
  else if (!strcmp(action,"match")) {
    argv = ParseArgv(argv,tSTR,&str,tSTR,&str1,tINT_,-1,&n,0);  
    lv = MatchStrN(str,str1,n);
    SetResultValue(lv);
    return;
  }
  
  /* substr action */
  else if (!strcmp(action,"substr")) {
    argv = ParseArgv(argv,tSTR,&str,tSTR,&str1,tINT_,-1,&n,0);
    lv = FindStr(str,str1,n);
    TempValue( lv);
    SetResultValue(lv);
    return;
  }

  /* inter action */
  else if (!strcmp(action,"inter")) {
    argv = ParseArgv(argv,tSTR,&str,tSTR,&str1,0);
    str3 = str;
    while (*str == *str1 && *str != '\0') {str++; str1++;}
    c = *str;
    *str = '\0';
    SetResultStr(str3);
    *str = c;
    return;
  }
    
  else Errorf("Unknown action '%s'",action);
}
  
 
 
/******************************************************
 *
 * Managing lists
 *
 ******************************************************/
 
 

int GetMaxSizeWord(char **beg,char **end, int n)
{
  return((end[n]-beg[n])+5);
}

static char *theStrList = NULL;
static int theMaxSizeAlloc = 0;
static int theNWords = 0;
static int curNWord = 0;
static char *curStrList = 0;

void StartAppendStr2StringList(char *strlist, int nWords, int maxSizeAlloc)
{
  curStrList = theStrList = strlist;
  curNWord = 0;
  theNWords = nWords;
  theMaxSizeAlloc = maxSizeAlloc;
  *theStrList = '\0';
}

void EndAppendStr2StringList(void)
{
  curStrList = theStrList = NULL;
  curNWord  = 0;
  theNWords = 0;
}

void AppendStr2StringList(char *str)
{
  char flagBraces;
  
  /* If not first word then ' ' */
  if (curNWord != 0) {
    if ((curStrList-theStrList+1)+1 >= theMaxSizeAlloc) {
      Errorf("AppendStr2StringList() : You exceeded the allocation for your list");
    }
    *curStrList = ' ';
    curStrList++;
  }

  /* Should we put braces around the result ? */
/*  if (theNWords == 1) flagBraces = NO;
  else if (*str == '\0') flagBraces = YES;
  else flagBraces = IsList(str);
  */
  flagBraces = NO;
  
  /* Let's do it */
  if (!flagBraces) {
    if ((curStrList-theStrList+1)+strlen(str) >= theMaxSizeAlloc) {
      Errorf("AppendStr2StringList() : You exceeded the allocation for your list");
    }
    strcpy(curStrList,str);
    curStrList += strlen(str);
  }
  else {
    if ((curStrList-theStrList+1)+strlen(str)+2 >= theMaxSizeAlloc) {
      Errorf("AppendStr2StringList() : You exceeded the allocation for your list");
    }
    *curStrList = '{';
    curStrList++;
    strcpy(curStrList,str);
    curStrList += strlen(str);
    *curStrList = '}';
    curStrList++;
  }
  
  *curStrList = '\0';
  
  curNWord++;
}


/* Delete a list */
void DeleteList(char **list)
{
  if (list == NULL) Errorf("DeleteList() : NULL argument");
  if (*list) Free(*list);
  Free(list);
}


/* A simplifier dans la copie */
/* Copy a list */
char **CopyList(char **list)
{
  char **listOut;
  int totalSize;
  int nElem,i;
  char flagList;

  if (list == NULL) Errorf("CopyList() : cannot copy NULL list");
  
  /* Case of an empty list */
  if (*list == NULL) {
    listOut = Malloc(sizeof(char *));
    *listOut = NULL;
    return(listOut);
  }
   
  /* Counting the number of elements of thelist AND checking if it is a list */ 
  nElem = 0;
  flagList = YES;
  while (list[nElem] != NULL) {
    if (nElem != 0 && flagList && strlen(list[nElem-1])+list[nElem-1]+1 != list[nElem]) flagList = NO;
    nElem++;
  }
  
  /* Total size allocation of the list */
  if (flagList) {
    totalSize = list[nElem-1]-list[0] + strlen(list[nElem-1])+1;
    if (totalSize < 0) Errorf("CopyList() : Weired bug");
  }
  else {
    nElem = 0;
    totalSize = 0;
    while (list[nElem] != NULL) {
      totalSize += strlen(list[nElem])+1;
      nElem++;
    }
  }
  
  /* Let's allocate */
  listOut = Malloc((nElem+1)*sizeof(char*));
  *listOut = Malloc(totalSize*sizeof(char));
  
  /* We have to set the listOut[i] */
  strcpy(listOut[0],list[0]);
  for (i=1;i<nElem;i++) {
    if (flagList) listOut[i] = (list[i]-list[i-1])+listOut[i-1];
    else listOut[i] = listOut[i-1]+strlen(listOut[i-1])+1;
    strcpy(listOut[i],list[i]);
  }
  listOut[nElem] = NULL;
  
  return(listOut);
}

/* Copy a list and make the result temporary */
char **TCopyList(char **list)
{
  char **listOut = CopyList(list);
  
  TempList(listOut);
  return(listOut);
}

/* ?? a verifier */
/* Convert an array of strings of size 'nElem'  into a list */
char **Array2List(char **array, int nElem)
{  
  char **listOut;
  int i,totalSize;
  
  /* Case of an empty array */
  if (nElem == 0) {
    listOut = Malloc(sizeof(char *));
    *listOut = NULL;
    return(listOut);
  }
     
  /* Total size allocation of the list */
  totalSize = 0;
  for (i=0;i<nElem;i++) totalSize += strlen(array[i]) +1;
  
  /* Let's allocate */
  listOut = Malloc((nElem+1)*sizeof(char*));
  *listOut = Malloc(totalSize*sizeof(char));
  
  /* We have to set the listOut[i] */
  strcpy(listOut[0],array[0]);
  for (i=1;i<nElem;i++) {
    listOut[i] = listOut[i-1]+strlen(listOut[i-1])+1;
    strcpy(listOut[i],array[i]);
  }
  listOut[nElem] = NULL;
  
  return(listOut);
}

/* Same as above but make the list temporary*/
char **TArray2List(char **array, int nElem)
{
  char **list = Array2List(array, nElem);
  TempList(list);
  return(list);
}


/* Convert a list to a string by separating each element of the list by 'separator' */
char *List2Str(char **list,char *separator)
{
  int size;
  char **list1;
  char *strOut;
  int n;

  if (list == NULL) Errorf("List2Str() : cannot convert a NULL list to a string");

  /* Case of an empty list */
  if (*list == NULL) {
    strOut = CharAlloc(1);
    *strOut = '\0';
    return(strOut);
  }
      
  n = strlen(separator);
  
  /* Total size allocation of the list */
  size = 0;
  list1 = list;
  while (*list1 != NULL) {
    size += strlen(*list1)+n;
    list1++;
  }
  size-=n;
  
  /* Allocation of the string */      
  strOut = CharAlloc(size+1);
  strOut[0] = '\0';
  
  /* Fill the string ?? merde a optimiser*/
  list1 = list;
  while (*list1 != NULL) {
    strcat(strOut,*list1);
    if (*(list1+1) != NULL) strcat(strOut,separator);
    list1++;
  }
  return(strOut);
}

/* Convert a list to a string and make this string temporary */
char *TList2Str(char **list,char *separator)
{
  char * strOut  = List2Str(list,separator);
  
  TempStr(strOut);
  return(strOut);
}


/*
 * Convert a succession of strings into a list 
 *   The succession of strings is given by two arrays
 *   begStr[] and endStr[] (these arrays end when both == NULL)
 */
 
char **BegEndStr2List(char *begStr[],char *endStr[])
{
  char **listOut,**list;
  int n,size,i;

  /* Case the list should be the empty list */
  if (begStr[0] == NULL || endStr[0] == NULL) {
    listOut = Malloc(sizeof(char *));
    *listOut = NULL;
    return(listOut);
  }

  /* Let's compute what the total size the list should be */
  size = 0;  
  n = 0;
  while (begStr[n] != NULL) { 
    size += (endStr[n] - begStr[n]) + 2;
    n++;
  }
  
  /* Let's alloc */    
  listOut = Malloc((n+1)*sizeof(char*));
  *listOut = Malloc(size*sizeof(char));
  
  list = listOut;
  for (i=0;i<n;i++) {
    strncpy(*list,begStr[i],endStr[i]-begStr[i]+1);
    (*list)[endStr[i]-begStr[i]+1] = '\0';
    *(list+1) = *list + (endStr[i]-begStr[i])+2;
    list++;
  }
  *list = NULL;
  
  return(listOut);
}

/* Same as above but make it temporary */ 
char **TBegEndStr2List(char *begStr[],char *endStr[])
{
  char **list = BegEndStr2List(begStr,endStr);
  TempList(list);
  return(list);
}



/* Convert a succession of strings into a list */
#define MaxNumStrs 200
char **Str2List(char *str,...)
{
  va_list ap;
  char **listOut,**list;
  char *theStrs[MaxNumStrs+1];
  int size[MaxNumStrs+1];
  int n,i,tsize;

  if (str == NULL) {
    listOut = Malloc(sizeof(char*));
    *listOut = NULL;
    return(listOut);
  }
      
  va_start(ap,str);
   
  theStrs[0] = str;
  n=1;
  tsize = 0;
  
  while (n <= MaxNumStrs && theStrs[n-1] != NULL) { 
    size[n-1] = strlen(theStrs[n-1])+1;
    tsize += size[n-1];
    theStrs[n] = va_arg(ap,char *);
    n++;
  }
  
  va_end(ap);
  
  if (n>MaxNumStrs) Errorf("Str2List() : Too many strings to append (should be less than %d)",MaxNumStrs);
    
  listOut = Malloc(n*sizeof(char*));
  *listOut = Malloc(tsize*sizeof(char));
  
  list = listOut;
  for (i=0;i<n;i++) {
    strcpy(*list,theStrs[i]);
    (*list)[size[i]-1] = '\0';
    *(list+1) = *list + size[i];
    list++;
  }
  *list = NULL;
  
  return(listOut);
}


/* Convert a succession of strings into a list and make it temporary */
#define MaxNumStrs 200
char **TStr2List(char *str,...)
{
Errorf("Pas implement TStr2List");
}


/* Print each word of a list seprated with a 'c' character and then \n */
void PrintList(char **list,char c)
{
  while (*list) {
    if (*(list+1) != NULL) Printf("%s%c",*list,c);
    else Printf("%s\n",*list);
  }
}


/*
 * Function that returns YES if 'str' is a list 
 * (i.e., braces must be placed around a string before concatenating with another string
 * in order to form a list).
 */

char IsList(char *str)
{
  char flagBraces;
  int n;
      
  flagBraces = NO;
  n = 0;
  while(1) {
    if (*str == '\0') break;
    if (*str == ' ' || *str == '\n' || *str == '\r') {flagBraces = YES; break;}
    if (*str == '{') {
      while (1) {
        while (*str != '\0' && *str != '{' && *str != '}') str++;
        if (*str == '{') {n++; str++; continue;}
        if (*str == '}') {n--; str++; if (n == 0) break; else continue;}
        flagBraces = YES;
        break;
      }
    }
    else str++;
  }
  
  return(flagBraces);
}

int GetListSize(char **list)
{
  char **list1;
  
  for(list1=list;*list1!=NULL;list1++) {};
  
  return(list1-list);
}





/* Make a List temporary */
void TempList(char **list)
{
  if (list == NULL) Errorf("TempList() (weired bug) : list is NULL");
  TempPtr(list);
  if (list[0]) TempPtr(list[0]);
}


 /*
 * Print successive strings in columns.
 * To initialize it one must call this function with str == NULL and size == size of a column
 * Then one calls it with str != NULL and 0
 */




/************************************************
 *
 * Command on lists
 *
 ************************************************/

void C_List(char **argv)
{
  char *action,*str;
  char **list,**list1;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  

  /* add action */
  if (!strcmp(action,"add")) {
    argv = ParseArgv(argv,tLIST,&list,tSTR,&str,0);  
    SetResultStr("");
    while (*list) {
      AppendListResultStr(*list);
      list++;
    }
    AppendListResultStr(str);
  } 

  /* append action */
  else if (!strcmp(action,"append")) {
    SetResultStr("");
    while (1) {
      argv = ParseArgv(argv,tLIST_,NULL,&list,-1);
      if (list == NULL) break;
      while (*list) {
        AppendListResultStr(*list);
        list++;
      }
    }
    NoMoreArgs(argv);
  } 

  /* reverse action */
  else if (!strcmp(action,"reverse")) {
    argv = ParseArgv(argv,tLIST,&list,0);
    SetResultStr("");
    list1 = list;
    while(*list1) list1++;
    list1--;
    while (list1 != list) {
      AppendListResultStr(*list1);
      list1--;
    }
    AppendListResultStr(*list1);  
  } 
     
       
  else Errorf("Unknown action '%s'",action);
}


/*
 * The field list
 */
struct field fieldsStr[] = {

  "", GetExtractStrV, SetExtractStrV, GetExtractOptionsStrV, GetExtractInfoStrV,
  "length", GetLengthStrV, NULL, NULL, NULL,
  "llength", GetLLengthStrV, NULL, NULL, NULL,
  "tonum", GetToNumStrV, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};


/*
 * The type structure for str
 */

TypeStruct tsStr = {

  "{{{&string} {This type corresponds to strings. \n \
- Constructors : You can use the syntax \"string\" or 'string'. \n \
- Operator + (and +=) : string1+string2, concatenation \n \
- Operator * (and *=) : string*n,  repetition of the string n times.}} \
{{&list} {This type corresponds to a string that can be interpreted as a list.}}}",  /* Documentation */

  &strType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteStrValue,     /* The Delete function */
  NewStrValue,     /* The Delete function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrStrContent,      /* String conversion */
  PrintStrContent,   /* The Print function : print the object when 'print' is called */
  PrintInfoStrContent,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsStr,      /* The list of fields */
};




