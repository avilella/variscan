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
 * RANGE contents
 *
 *********************************************************/


#include "lastwave.h"
#include <stdarg.h>
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"



/* Type of a range */
char *rangeType = "&range";

/*
 * Answers to the different print messages
 */
 
char *ToStrRange(RANGE rg, char flagShort)
{
  static char str[40];
  
  if (rg->step == 1) sprintf(str,"%g:%g",rg->first,RangeVal(rg,rg->size-1));
  else sprintf(str,"%g:%g:%g",rg->first,rg->step,RangeVal(rg,rg->size-1));
  
  return(str);
}


void PrintInfoRange(RANGE rg)
{
  Printf("   first =  %g\n",rg->first);
  Printf("   step  =  %g\n",rg->step);
  Printf("   last  =  %g\n",RangeVal(rg,rg->size-1));
  Printf("   size  =  %d\n",rg->size);
}

/*
 * The different extract options 
 */

static char *extractOptionsRange[] = {"*nolimit","*bperiodic","*bmirror","*bmirror1","*bconst","*b0",NULL};
enum {
  FSIOptRangeNoLimit = FSIOption1,
  FSIOptRangeBPer = FSIOption2,
  FSIOptRangeBMir = FSIOption3,
  FSIOptRangeBMir1 = FSIOption4,
  FSIOptRangeBConst = FSIOption5,
  FSIOptRangeB0 = FSIOption6
};



/*
 * Get and extraction of fields NULL
 */
static char *doc = "{[*opt,...]} {Get range values}"; 

static void *GetExtractRangeV(VALUE val, void **arg)
{
  RANGE rg;
  FSIList *fsiList;
  float *pFlt;
  char **pStr;
  VALUE *pValue;

  int max;
  FSI_DECL;
  BorderType bt;
  SIGNAL sig;
  RANGE rg1;
  
  /* doc */
  if (val == NULL) return(doc);

  rg = (RANGE) val;
  fsiList = ARG_G_GetFsiList(arg);
  pFlt = ARG_G_GetResPFloat(arg);
  pStr = ARG_G_GetResPStr(arg);
  pValue = ARG_G_GetResPValue(arg);

  if (fsiList->options & FSIOptRangeBPer) bt = BorderPer;
  else if (fsiList->options & FSIOptRangeBMir) bt = BorderMir;
  else if (fsiList->options & FSIOptRangeBMir1) bt = BorderMir1;
  else if (fsiList->options & FSIOptRangeBConst) bt = BorderCon;
  else if (fsiList->options & FSIOptRangeB0) bt = Border0;
  else bt = BorderNone;

  /* Get the max size */
  max = rg->size;
 
  /*
   * Case the result will be a simple float
   */
   if (fsiList->options & (FSIOptRangeBPer | FSIOptRangeBMir | FSIOptRangeBMir1 | FSIOptRangeBConst | FSIOptRangeB0) && fsiList->nx == 1 ||
       !(fsiList->options & (FSIOptRangeBPer | FSIOptRangeBMir | FSIOptRangeBMir1 | FSIOptRangeBConst | FSIOptRangeB0)) && fsiList->nx1 == 1) {

     _i = (int) FSIArray((&(fsiList->fsi[0])),0);
 
    /* The loop in the case of *nolimit */
    if (fsiList->options & FSIOptRangeNoLimit) {
      FSI_FOR_START(fsiList); 
      if (_i<0 || _i > max-1) continue;
      *pFlt = RangeVal(rg,_i);
      break;       
      FSI_FOR_END;
    }
    
    /* Other cases */
    else {
      switch (bt) {
      case BorderPer : *pFlt = RangeVal(rg,BPER(_i,max)); break;
      case BorderMir : *pFlt = RangeVal(rg,BMIR(_i,max)); break;
      case BorderMir1 : *pFlt = RangeVal(rg,BMIR1(_i,max)); break;
      case Border0 : 
        if (_i<0 || _i>=max) *pFlt = 0;
        else *pFlt = RangeVal(rg,_i);
        break;
      case BorderCon : *pFlt = RangeVal(rg,BCONST(_i,max)); break;
      default : *pFlt = RangeVal(rg,_i);
      }
    }
    
    return(numType);
  }

  /* Case the result will be a range */
  if (fsiList->size == 1 && fsiList->fsi[0].type == FSIRange) {
    if (!(fsiList->options & (FSIOptRangeNoLimit | FSIOptRangeBPer | FSIOptRangeBMir | FSIOptRangeBMir1 | FSIOptRangeBConst | FSIOptRangeB0)) ||
         RangeMin(fsiList->fsi[0].val.r) >= 0 && RangeMax(fsiList->fsi[0].val.r) < rg->size)  {
      rg1 = TNewRange();
      if (fsiList->fsi[0].val.r->step > 0) {
        rg1->first = RangeVal(rg,MAX(0,fsiList->fsi[0].val.r->first));
        rg1->size = MIN(fsiList->fsi[0].val.r->size,rg->size-MAX(0,fsiList->fsi[0].val.r->first));
        rg1->step = rg->step;
      }
      else {
        rg1->first = RangeVal(rg,MIN(rg->size-1,RangeFirst(fsiList->fsi[0].val.r)));
        rg1->size = MIN(fsiList->fsi[0].val.r->size,MIN(rg->size-1,RangeFirst(fsiList->fsi[0].val.r))+1);
        rg1->step = -rg->step;
      }
      *pValue = (VALUE) rg1;
      return(rangeType);
    }
  }
      
  /* Allocation of the result */
  sig = TNewSignal();
  if (fsiList->options & (FSIOptRangeBPer | FSIOptRangeBMir | FSIOptRangeBMir1 | FSIOptRangeBConst | FSIOptRangeB0)) SizeSignal(sig,fsiList->nx,YSIG);
  else SizeSignal(sig,fsiList->nx1,YSIG);

  /* The loop in the case of *nolimit */
  if (fsiList->options & FSIOptRangeNoLimit) {
    FSI_FOR_START(fsiList); 
    if (_i<0 || _i > max-1) continue;
    sig->Y[_k] = RangeVal(rg,_i);
    FSI_FOR_END;
  }
    
  /* Other cases */
  else {
    FSI_FOR_START(fsiList); 
    switch (bt) {
    case BorderPer : sig->Y[_k] = RangeVal(rg,BPER(_i,max)); break;
    case BorderMir : sig->Y[_k] = RangeVal(rg,BMIR(_i,max)); break;
    case BorderMir1 : sig->Y[_k] = RangeVal(rg,BMIR1(_i,max)); break;
    case Border0 : 
      if (_i<0 || _i>=max) sig->Y[_k] = 0;
      else sig->Y[_k] = RangeVal(rg,_i);
      break;
    case BorderCon : sig->Y[_k] = RangeVal(rg,BCONST(_i,max)); break;
    default : sig->Y[_k] = RangeVal(rg,_i);
    }
    FSI_FOR_END;
  }
             
  *pValue = (VALUE) sig;
        
  return(signaliType);
}


/*
 * Get the options for extraction (called for field NULLonly)
 */

static char *optionDoc = "{{*nolimit,*b0,*bconst,*bmirror,*bmirror1,*bperiodic,*x,*xlin} \
{*nolimit : indexes can be out of range} \
{*b0 : border effect with 0 value} \
{*bconst : border effect with constant values (last range value for right handside and first range value for left handside)} \
{*bperiodic : periodic border effect)} \
{*bmirror1 : mirror+periodic border effect (first and last points are repeated)} \
{*bmirror : mirror+periodic border effect (first and last points are NOT repeated)}\
}";

static void *GetExtractOptionsRangeV(VALUE val, void **arg)
{
  /* doc */
  if (val == NULL) return(optionDoc);

  return(extractOptionsRange);
}


/*
 * Function to get the ExtractInfo for fields NULL
 */

static void *GetExtractInfoRangeV(VALUE val, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  
  RANGE rg = (RANGE) val;
  char *field = ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);

  /* If *bperiodic,... then *nolimit must be off */
  if (*options & (FSIOptRangeBPer | FSIOptRangeBMir | FSIOptRangeBMir1 | FSIOptRangeB0 | FSIOptRangeBConst)) *options &= ~FSIOptRangeNoLimit;
       
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.xmin = 0;
    extractInfo.dx = 1;
    extractInfo.nSignals = 1;
    flagInit = NO;
  }

  if (rg->size == 0) {
    SetErrorf("No extraction on empty range");
    return(NULL);
  }
  
  extractInfo.xmax = rg->size-1;

  /* '*nolimit' option : set some flags */
  if (*options & (FSIOptRangeBPer | FSIOptRangeBMir | FSIOptRangeBMir1 | FSIOptRangeB0 | FSIOptRangeBConst | FSIOptRangeNoLimit)) extractInfo.flags = EIIntIndex;
  else extractInfo.flags = EIErrorBound | EIIntIndex;
     
  return(&extractInfo);
} 


/*
 * 'size' field
 */

static char *sizeDoc = "{[= <size>]} {Sets/Gets the size of a range}";

static void * GetSizeRangeV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(sizeDoc);
  
  return(GetIntField(((RANGE) val)->size,arg));
}

static void * SetSizeRangeV(VALUE val, void **arg)
{
  /* doc */
  if (val == NULL) return(sizeDoc);

 return(SetIntField(&(((RANGE) val)->size),arg,FieldSPositive));
}

/*
 * 'first' field
 */
static char *firstDoc = "{[= <first>]} {Sets/Gets the first value of a range}";

static void * GetFirstRangeV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(firstDoc);
  
  return(GetFloatField(((RANGE) val)->first,arg));
}

static void * SetFirstRangeV(VALUE val, void **arg)
{
  /* doc */
  if (val == NULL) return(firstDoc);

 return(SetFloatField(&(((RANGE) val)->first),arg,0));
}

/*
 * 'last' field
 */
static char *lastDoc = "{[= <last>]} {Sets/Gets the last value of a range}";

static void * GetLastRangeV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(lastDoc);
  
  return(GetFloatField(RangeLast((RANGE) val),arg));
}

static void * SetLastRangeV(VALUE val, void **arg)
{
 RANGE rg;
 float last;
 
   /* doc */
  if (val == NULL) return(lastDoc);

 rg = ((RANGE) val);
 last = RangeLast(rg);

 if (SetFloatField(&last,arg,0) == NULL) return(NULL);
 
 rg->first = last-rg->step*(rg->size-1);
}

/*
 * 'step' field
 */
static char *stepDoc = "{[= <step>]} {Sets/Gets the step of a range}";

static void * GetStepRangeV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(stepDoc);
  
  return(GetFloatField(((RANGE) val)->step,arg));
}

static void * SetStepRangeV(VALUE val, void **arg)
{
  /* doc */
  if (val == NULL) return(stepDoc);

 return(SetFloatField(&(((RANGE) val)->step),arg,FieldSPositive));
}
        


/* Allocate a new range */
RANGE NewRange(void)
{
  RANGE r;
  
#ifdef DEBUGALLOC
DebugType = "Range";
#endif

  r = Malloc(sizeof(struct rangeValue));
  InitValue(r,&tsRange);
  
  r->step = 1;
  r->first = 0;
  r->size = 1;
  
  return(r);
}

RANGE TNewRange(void)
{
  RANGE r;
  
  r = NewRange();
  
  TempValue(r);
  
  return(r);
}

void DeleteRange(RANGE r)
{
  r->nRef--;
  if (r->nRef >0) return;
  
#ifdef DEBUGALLOC
DebugType = "Range";
#endif

 Free(r);  
}

/*
 * The field list
 */
struct field fieldsRange[] = {

  "", GetExtractRangeV, NULL, GetExtractOptionsRangeV, GetExtractInfoRangeV,
  "size", GetSizeRangeV, SetSizeRangeV, NULL, NULL,
  "first", GetFirstRangeV, SetFirstRangeV, NULL, NULL,
  "step", GetStepRangeV, SetStepRangeV, NULL, NULL,
  "last", GetLastRangeV, SetLastRangeV, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for Range
 */

TypeStruct tsRange = {

  "{{{&range} {This type corresponds to ranges. \n \
- Constructors : syntax is first:step:last or first:#size:last. \
One can use first!:... to except the first point or ...:!last to except the last one. \
If implicit the first, last, size or step can be ommitted. \
Moreover @> (resp. @< or @+) refers to the 'implicit' last (resp. first or step) point. \n \
- Most of the operators valid for signals are valid for ranges.}}}",  /* Documentation */

  &rangeType,       /* The basic (unique) type name */
  NULL,
  
  DeleteRange,     /* The Delete function */
  NewRange,     /* The New function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrRange,       /* String conversion */
  NULL,   /* The Print function : print the object when 'print' is called */
  PrintInfoRange,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsRange,      /* The list of fields */
};
