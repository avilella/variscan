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


#include "lastwave.h"
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"

extern unsigned char TTExpr1(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam) ;

#define Clean(val)  if ((val) != NULL) {(val) = NULL;}

/* Variable used in the BPER,BMIR,... macros */
long __borderTempLong__;


/****************************************************************************/
/*                                                                          */
/*  int_listfsi.c                 */
/*                                                                          */
/****************************************************************************/



/*  
 * Some static variables to avoid dynamic allocation 
 */
 
#define NFSILISTS 10
static FSIList theSFIList[NFSILISTS];
static int nSFIList = -1;


/*
 * Init the static SFI list 
 */
static void InitSFIList(void)
{
  int j;
  
  for (j=0;j<NFSILISTS;j++) theSFIList[j].size = 0;
  nSFIList = 0;
}


/* 
 * Returns an SFIList that is available (from the static list)
 */ 
 static int yyy=0;
FSIList *NewSFIList(void)
{
  int j;
  FSIList *list;
  
  if (nSFIList == -1) InitSFIList();

  if (theSFIList[nSFIList].size == 0) j = nSFIList;
  else {
    for (j=0;j<NFSILISTS;j++) if (theSFIList[j].size == 0) break;
    if (j == NFSILISTS) {
      for (j=0;j<NFSILISTS;j++) DeleteFSIList(theSFIList+j);
      Errorf("NewSFIList() : Sorry, too many lists are used (maximum is %d)",NFSILISTS);
    }
  }
  list = &(theSFIList[j]);
  nSFIList = (j+1)%NFSILISTS;
  list->nSep = 0;
  list->flagImage = NO;
  
  yyy++;
  return(list);    
}


/* 
 * Delete an SFIList
 */ 
void DeleteFSIList(FSIList *list)
{
  if (list-theSFIList >= NFSILISTS) Errorf("DeleteFSIList() : Weird");

  nSFIList = list-theSFIList;

  list->size = 0;
  yyy--;
}


/*
 * Read the options of an FSI List
 */
char  FSIReadExtractOption(char *begin, char **left, char **options, unsigned long *optionFlag)
{
  char *begin1;
  char **options1;
  char flagNoOption;
  
  if (options == NULL) return(0);
  
  SkipSpace(begin);
  if (*begin != '*') return(0);

  *optionFlag = 0;
  flagNoOption = NO;
  while (*begin == '*') {
    
    /* Get next option */
    begin1 = begin+1;
    while (*begin1 != '*' && *begin1 != ',' && *begin1 != FSISEPARATOR && *begin1 != ')' && *begin1 != '\0') begin1++;
    
    /* No Option *- ? */
    if (begin1-begin == 2 && *(begin1-1) == '-') {
      flagNoOption = YES;
      begin = begin1;
      SkipSpace(begin);
      if (*begin == '\0' || *begin == FSISEPARATOR) break;
      if (*begin == ',') begin++;
      SkipSpace(begin);
      continue;
    }
    
    /* Compare it */
    options1 = options;
    while (*options1) {
      if (begin1-begin <= strlen(*options1) && !strncmp(begin,*options1,begin1-begin)) break;
      options1++;
    }
    if (*options1 == NULL) {
      SetErrorf("Bad extracting option");
      *left = begin;
      return(NO);
    }
    *optionFlag |= (1<<(options1-options));
 
    begin = begin1;
    SkipSpace(begin);
    if (*begin == '\0') break;
    if (*begin == FSISEPARATOR) {begin++;SkipSpace(begin);break;}
    if (*begin == ',') begin++;
    SkipSpace(begin);
  }
    
  *left = begin;
  
  if (flagNoOption) *optionFlag = 0;
  
  return(YES);
}  
    
     
/*
 * Macro to test the validity of indexes for extraction 
 */

#define ValidIndexBound(index) \
((index) >= min && (index) <= max ? 1 : (ei->flags & EIErrorBound ? SetErrorf("Indexes are out of range"), -1 : 0))

#define ValidIndexInt(index) \
((index) != (int) index ? SetErrorf("Expecting integer indexes"), -1 : 1)


/*
 * Read a list of content.
 *
 * The arguments are :
 *  
 *    begin, left : as usual
 *    ei : the information needed about the indexes to extract (if no extraction then it is NULL)
 *
 */
 
FSIList *SFIListExpr(char* begin, char** left, ExtractInfo *ei, ExprDefParam *defParam) 
{
  FSIList *list;  
  int j,kk;
  VALUE val;
  float f;
  char flagError;
  SIGNAL s;
  RANGE rg;
  int size;
  IMAGE i;
  float min,max,dx,dy;
  int *nValid,nVal;
  LISTV lv;
  int nlv;
  char *type;
  
  unsigned char r;  
  int jsep,row1NCol,row2NCol;
  int nvblock,nhblock;
  int nrowCur,ncolCur;
  int nrow,ncol;
  char *begin1;
  char flagExtract;
  DefaultVectVar;
  
  ParamSkipSpace(begin);
  
  /*
   * Case of an empty []    
   */
  if (*begin == ']') {
    list = NewSFIList();
    list->nx = list->ny = list->nx1 = list->ny1 = 0;
    *left = begin;
    return(list);
  }
  
  /* 
   * Some inits 
   */
  flagExtract = (ei != NULL); 
  if (ei && ei->nSignals<=0) Errorf("SFIListExpr() : Weird : bad extractInfo->nSignals<=0");
  list = NewSFIList();
  j = 0;
  flagError = NO;

  /* Set the min max for the first signal in case of extraction and a pointer to what will be the number of indexes in range */  
  if (flagExtract) {
    min = ei->xmin;
    dx = ei->dx;
    if (dx<=0) s = ei->xsignal;
    else s = NULL;
    max = ei->xmax;
    nValid = &(list->nx1);
    *nValid = 0;
    list->ny1 = 0;
    if (min <= max) {
      SaveDefaultVector(defParam);
      SetDefaultVector(defParam,min,dx,max,s,YES);
    }
  }
  
  
  nrow = 0;    /* The total number of rows */
  ncol = 0;    /* The total number of cols */
  ncolCur = 0; /* The current number of cols */
  nrowCur = 0; /* The current number of rows */
  nvblock = 1; /* The current vertical block number */
  nhblock = 1; /* The current horizontal block number */
  jsep = -1;   /* The index where the first separator ',' is found (used only in the case of extractIm) */ 
  row1NCol = row2NCol = -1; /* The number of columns of the first and second rows (used only in the case of flagExtract) */ 
  
  lv = NULL;
  
  while(1) {

    /* Treat the next element in the listv  if there is any */
    if (lv != NULL) {
      if (lv->length != nlv) {
        type = GetListvNth(lv,nlv,&val,&f);
        if (type == rangeType) r = RangeType;
        else if (type == numType) r = FloatType;
        else if (type == signaliType) r = SignalType;
        else if (type == imageiType) r = ImageType;
        else if (type == imageType || type == signalType) {nlv++; continue;}
        else {
          SetErrorf("Bad '%s' element type in listv for extraction (should be one of '%s', '%s, '%s' or '%s')",type,floatType,rangeType,signaliType,imageiType);
          *left = begin;
          flagError = YES;
          break;
        }
      }
      nlv++;
      if (lv->length == nlv) lv = NULL;
    }
        
    /* Otherwise just parse the line */
    else {
      begin1 = begin;
      val = NULL;
      r = TTExpr1(begin, left,&f,&val,FloatType | SignalType | ImageType | RangeType | ListvType,defParam);
      if (r == 0) {
        flagError = YES;
        break;
      }
      /* case of a listv */
      if (r==ListvType) {
        lv = (LISTV) val;
        if (lv->length == 0) {
          lv = NULL;
          goto jumpEmptyListv;
        }
        nlv = 0;
        continue;
      }
    }

    if (j == NFSI) { /* Too many elements */
      flagError = YES;
      *left = begin1;
      SetErrorf("Sorry, too many elements (Maximum is %d)",NFSI);
      break; 
    }

    /* Get the type of the next content */
    switch(r) {
      case FloatType :  list->fsi[j].type = FSIFloat; break;
      case SignalType :  list->fsi[j].type = FSISignal; break;
      case RangeType :  list->fsi[j].type = FSIRange; break;
      case ImageType :  list->fsi[j].type = FSIImage; break;
    }

    /* Case it is a signal or a range */        
    if (r == SignalType || r == RangeType) {
      if (r == SignalType) {
        s = (SIGNAL) val;
        size = s->size;
      }
      else {
        rg = (RANGE) val;
        size = rg->size;
      }
      if (nhblock == 1) {nrowCur = 1; nrow += nrowCur;}
      else if (nrowCur != 1) { /* a signal is on the same row as an image */
        flagError = YES;
        *left = begin1;
        SetErrorf("Expect an image with %d rows (not a signal or a range!)",nrowCur);
        break; 
      }
      if (nvblock == 1) ncol += size;
      else {
        ncolCur += size;
        if (!flagExtract && ncolCur > ncol) { /* One row has too many columns */
          flagError = YES;
          *left = begin1;
          SetErrorf("Row with an exceeding number of columns %d (>%d)",ncolCur,ncol);
          break; 
        } 
      }
      
      /* Some checkings with ExtractInfo if necessary */
      if (flagExtract) {
         /* Case of Ranges */
        if (r==RangeType) { 
          /* Checking min, max */
          if (min > max) *nValid += size;
          else {
            if ((ei->flags & EIErrorBound) && ((rg->step > 0 && (rg->first < min || rg->first+rg->step*(size-1) > max)) || (rg->step < 0 && (rg->first > max || rg->first+rg->step*(size-1) < min)))) {
              flagError = YES;
              SetErrorf("Indexes out of range");
              break;
            }
            else {
              if (rg->step > 0) nVal = (int) ((MIN(rg->first+rg->step*(size-1),max) - MAX(rg->first,min))/rg->step+1.1);
              else nVal = (int) ((MIN(rg->first,max) - MAX(rg->first+rg->step*(size-1),min))/(-rg->step)+1.1);
              if (nVal > 0) *nValid += nVal;
            }
          }
          /* Checking int */
          if (ei->flags & EIIntIndex) {
            if (rg->first != (int) rg->first || (rg->step != (int) rg->step && size != 1)) {
              flagError = YES;
              SetErrorf("Expect integer indexes");
              break;
            }
          }
        }
        /* Case of signals */          
        else {
          if (min > max) {
            *nValid += s->size;
            if (ei->flags & EIIntIndex) {
              for(kk=0;kk<s->size;kk++) {
                if (ValidIndexInt(s->Y[kk]) == -1) break;
              }
            }
          }
          else {
            for(kk=0;kk<s->size;kk++) {
              if ((ei->flags & EIIntIndex) && ValidIndexInt(s->Y[kk]) == -1) break;
              nVal = ValidIndexBound(s->Y[kk]);
              if (nVal == -1) break;
              *nValid += nVal;
            }
          }
          if (kk != s->size) {
            flagError = YES;
            break;
          }
        }
      }
                
      /* Setting the new element in the list */
      if (r==SignalType) {list->fsi[j].val.s = s; list->fsi[j].array = s->Y;}
      else {list->fsi[j].val.r = rg; list->fsi[j].array = NULL;}
      list->fsi[j].size = size;
      s = NULL;
      val = NULL;
      j++;
      list->size++;
    }

    /* Case it is an image */
    else if (r == ImageType) {
      i = (IMAGE) val;
      if (flagExtract && i->nrow != 1) {
        if (ei->nSignals == 1) {
          SetErrorf("Can't use image for extraction indexes");
          flagError = YES;
          break;
        }
        if (list->size != 0 || i->nrow != 2) {
          SetErrorf("Can't use more than 1 image with 2 rows for extraction indexes");
          flagError = YES;
          break;
        }
      }
      if (nhblock == 1) {nrowCur = i->nrow; nrow += nrowCur;}
      else if (nrowCur != i->nrow) { /* an image has the wrong number of rows */
        flagError = YES;
        *left = begin1;
        SetErrorf("Bad number of rows %d (expect %d rows)",i->nrow,nrowCur);
        break; 
      } 
      if (nvblock == 1) ncol += i->ncol;
      else {
        ncolCur += i->ncol;
        if (!flagExtract && ncolCur > ncol) { /* One row has too many columns */
          flagError = YES;
          *left = begin1;
          SetErrorf("Row with an exceeding number of columns %d (>%d)",ncolCur,ncol);
          break; 
        } 
      }
      list->fsi[j].val.i = i;
      list->fsi[j].array = i->pixels;
      list->fsi[j].size = i->nrow*i->ncol;
      i = NULL;
      val = NULL;
      j++;
      list->size++;
    }

    /* Case of a float */
    else {
      if (nhblock == 1) {nrowCur = 1; nrow += nrowCur;}
      else if (nrowCur != 1) { /* a float is on the same row as an image */
        flagError = YES;
        *left = begin1;
        SetErrorf("Expect an image with %d rows (not a float !)",nrowCur);
        break; 
      } 
      if (nvblock == 1) ncol ++;
      else {
        ncolCur ++;
        if (!flagExtract && ncolCur > ncol) { /* One row has too many columns */
          flagError = YES;
          *left = begin1;
          SetErrorf("Row with an exceeding number of columns %d (>%d)",ncolCur,ncol);
          break; 
        } 
      }
      if (flagExtract) {
        if (min > max) (*nValid)++;
        else {
          nVal = ValidIndexBound(f);
          if (nVal == -1) {
            flagError=YES;
            break;
          }
          *nValid += nVal;
        }
        if (ei->flags & EIIntIndex) {
          if (ValidIndexInt(f) == -1) {
            flagError= YES;
            break;
          }
        }
      }
      list->fsi[j].val.f = f;
      list->fsi[j].array = &(list->fsi[j].val.f);
      list->fsi[j].size = 1;
      j++;
      list->size++;
    }
    
 jumpEmptyListv :   
 
    /* Skip spaces */
    if (!lv) {
      begin = *left;
      ParamSkipSpace(begin);
    }
    
    /* another column with a ',' */
    if (*begin == ',') {
      nhblock++;
      begin++; 
      ParamSkipSpace(begin);
      continue;
    };

    /* another column with a listv */
    if (lv) {
      nhblock++;
      continue;
    };

    /* another row */
    if (*begin == FSISEPARATOR) {
      if (ei && ei->nSignals == 1) {
        SetErrorf("Expecting only one row of indexes");
        flagError = YES;
        break;
      }
      if (flagExtract) {
        min = ei->ymin;
        dy = ei->dy;
        max = ei->ymax;
        nValid = &(list->ny1);
        *nValid = 0;
        RestoreDefaultVector(defParam);
        if (min <= max) {
          SaveDefaultVector(defParam);
          SetDefaultVector(defParam,min,dy,max,NULL,YES);
        }
      }
      list->fsi[j].type = FSISeparator;
      list->size++;
      list->nSep++;
      if (jsep == -1) jsep = j;
      j++;
      begin++;
      ParamSkipSpace(begin);
      if (!flagExtract && nvblock > 1 && ncolCur != ncol) {
        flagError = YES;
        *left = begin1;
        SetErrorf("Row with a wrong number of columns %d (should be %d)",ncolCur,ncol);
        break; 
      }
      if (row1NCol == -1) row1NCol = ncol;

      nhblock = 1;
      nvblock++;
      ncolCur = 0;
      nrowCur = 0;
      continue;
    }
    
    break;      
   
  }

  if (flagExtract) {
    RestoreDefaultVector(defParam);
  }

  /* If we expected 2 signals and we get an image with 2 rows, it's ok ! */
  if (!flagError && ei && ei->nSignals != list->nSep+1) {
    if (list->size != 1 || list->fsi[0].type != FSIImage || list->fsi[0].val.i->nrow != 2) {
      SetErrorf("Expecting %d rows of indexes",ei->nSignals);
      flagError = YES;
    }
    else {
      i = list->fsi[0].val.i;
      if (ei->flags & (EIErrorBound | EIIntIndex)) {
        for (j=0;j<2*i->ncol;j++) {
          if (ei->flags & EIIntIndex && i->pixels[j] != (int) i->pixels[j]) {
            SetErrorf("Expecting integer indexes");
            flagError = YES;
            break;
          }
          if (ei->flags & EIErrorBound &&
              (j<i->ncol && (i->pixels[j]<ei->xmin || i->pixels[j]>ei->xmax) ||
              j>=i->ncol && (i->pixels[j]<ei->ymin || i->pixels[j]>ei->ymax))) {
            SetErrorf("Indexes are out of range");
            flagError = YES;
            break;
          }
        }
      }
      if (!flagError) {
        list->size = 3;
        list->fsi[0].type = FSISignal;
        list->fsi[0].val.s = NULL;
        list->fsi[0].array = i->pixels;
        list->fsi[0].size = i->ncol;
        list->fsi[1].type = FSISeparator;
        list->fsi[1].array = NULL;
        list->fsi[2].type = FSISignal;
        list->fsi[2].val.s = NULL;
        list->fsi[2].array = i->pixels+i->ncol;
        list->fsi[2].size = i->ncol; 
        ncolCur = ncol = i->ncol;
        list->flagImage = YES;
      }
    }
  }
    
  /* A test (the end of a list is like a new separator) */
  if (!flagExtract && nvblock > 1 && ncolCur != ncol) {
    flagError = YES;
    *left = begin1;
    SetErrorf("Row with a wrong number of columns %d (should be %d)",ncolCur,ncol);
  }
 
  /* Set the number of columns of the second row */
  if (flagExtract) {
    row1NCol = ncol;
    row2NCol = ncolCur;
  }
  
  /* Set the nx,ny fields */
  if (flagExtract) {
    list->nx = row1NCol;
    list->ny = row2NCol;
  }
  else {
    list->nx = ncol;
    list->ny = nrow;
  }
  
  /* Error */
  if (flagError) {
    DeleteFSIList(list);
    Clean(val);
    return(NULL);
  }

  ParamSkipSpace(*left);

  list->ei = ei;
  
  return(list);
}



/*
 * Convert a size 1 signal, range or image to a float and returns YES or NO if went well or not
 */
#define Convert2Float(elem,fl) \
    ((elem).type == FSIFloat ? ((fl) = (elem).val.f, YES) : \
       (((elem).type == FSISignal && (elem).val.s->size == 1) ? ((fl) = (elem).val.s->Y[0], YES) : \
         (((elem).type == FSIRange && (elem).val.r->size == 1) ? ((fl) = (elem).val.r->first, YES) : \
           (((elem).type == FSIImage && (elem).val.i->nrow == 1 && (elem).val.i->ncol == 1)  ? ((fl) = (elem).val.i->pixels[0], YES) : NO))))


/*
 * Expansion of a list of content into a signal, an image or a float
 */
unsigned char ExpandFSIList(FSIList *list, float *resFlt, VALUE *resVC, unsigned char flagType) 
{
  int nrow, ncol,nrow1,ncol1,nrow2,k,l,k1,k2,n;
  SIGNAL s,s1;
  IMAGE i,i1;
  unsigned char answer;
  RANGE rg;
  int ii;
  
  if (list->nx == 0) {
    *resVC = (VALUE) TNewSignal();
    DeleteFSIList(list);
    return(SignalType);
  }
  nrow = list->ny;
  ncol = list->nx;
  
  /* Error */
  if (nrow>1 && !(flagType&ImageType)) {
    SetErrorf("Do not expect an image");
    DeleteFSIList(list);
    return(NO);
  }
  if (nrow==1 && ncol>1 && flagType==FloatType) {
    SetErrorf("Expect a float");
    DeleteFSIList(list);
    return(NO);
  }

  
  /*********************************************
   *
   * Case of a single row
   *
   *********************************************/
   
  if (nrow == 1) {
    /* Case we can return a float */
    if (ncol == 1) {
      ii = 0;
      while (list->fsi[ii].size == 0) ii++;
      if (Convert2Float(list->fsi[ii],*resFlt)==NO) Errorf("ExpandFSIList() : Weird");
      Clean(*resVC);
      DeleteFSIList(list);
      return(FloatType);
    }
    /* Case of a single signal or range */
    else if (list->size==1 && (list->fsi[0].type == FSISignal || list->fsi[0].type == FSIRange)) {
      if (list->fsi[0].val.s->nRef == 1 && list->fsi[0].type != FSIRange) {
        Clean(*resVC);
        *resVC = (VALUE) list->fsi[0].val.s;
      } 
      else {
        if (list->fsi[0].type == FSIRange) {
          Clean(*resVC);
          *resVC = (VALUE) TNewSignal();
          SizeSignal((SIGNAL) *resVC,list->fsi[0].val.r->size,YSIG);
          for (k=0;k<list->fsi[0].val.r->size;k++) {
            ((SIGNAL) *resVC)->Y[k] = RangeVal(list->fsi[0].val.r,k);
          }
        }
        else {
          if (*resVC == NULL || GetTypeValue(*resVC) != signaliType) {
            Clean(*resVC);
            *resVC = (VALUE) TNewSignal();
          }
          CopySig(list->fsi[0].val.s,(SIGNAL) *resVC);
        }
      }
      DeleteFSIList(list);
      return(SignalType);
    }

    /* Case of more than one signal/range or of single rowed images */
    /* allocation */
    if (*resVC == NULL || GetTypeValue(*resVC) != signaliType) {
      Clean(*resVC);
      *resVC = (VALUE) TNewSignal();
    }
    s = (SIGNAL) *resVC;
    SizeSignal(s,ncol,YSIG);

    k = 0;
    
    /* Set the x0/dx if necessary */
    if (list->fsi[0].type == FSISignal && list->fsi[0].val.s->type == YSIG) {
      s->x0 = list->fsi[0].val.s->x0;
      s->dx = list->fsi[0].val.s->dx;
    }       
    
    for (n=0;n<list->size;n++) {
      
      /* case 0 */
      if (list->fsi[n].type == FSISeparator) Errorf("ExpandFSIList() : Weird 4");
      
      /* case FloatType */
      else if (list->fsi[n].type == FSIFloat) {
        s->Y[k] = list->fsi[n].val.f; 
        k++;
      }
              
      /* case SignalType */
      else if (list->fsi[n].type == FSISignal) {
        s1 = list->fsi[n].val.s;
        memcpy(s->Y+k,s1->Y,s1->size*sizeof(float));k+= s1->size;
      }

      /* case RangeType */
      else if (list->fsi[n].type == FSIRange) {
        rg = list->fsi[n].val.r;
        for (k1 = 0;k1<rg->size;k1++,k++) s->Y[k] = RangeVal(rg,k1);
      }
      
      /* case ImageType */
      else if (list->fsi[n].type == FSIImage) {
        i1 = list->fsi[n].val.i;
        memcpy(s->Y+k,i1->pixels,i1->ncol*sizeof(float));k+= i1->ncol;
      }
      
      else Errorf("ExpandFSIList() : weird");

    }
    
    answer = SignalType;
  }    


  /*********************************************
   *
   * Case of a several rows
   *
   *********************************************/
  else {

    /* Case of a single image */
    if (list->size==1 && list->fsi[0].type == FSIImage) {
      if (list->fsi[0].val.i->nRef == 1) {
        Clean(*resVC);
        *resVC = (VALUE) list->fsi[0].val.i;
      } 
      else {
        if (*resVC == NULL || GetTypeValue(*resVC) != imageiType) {
          Clean(*resVC);
          *resVC = (VALUE) TNewImage();
        }
        CopyImage(list->fsi[0].val.i,(IMAGE) *resVC);
      }
      DeleteFSIList(list);
      return(ImageType);
    }

    /* Case of more than one image */
    /* allocation */
    if (*resVC == NULL || GetTypeValue(*resVC) != imageiType) {
      Clean(*resVC);
      *resVC = (VALUE) TNewImage();
    }
    i = (IMAGE) *resVC;
    SizeImage(i,ncol,nrow);
    
    k = 0;
    nrow1 = ncol1 = 0;
    for (n=0;n<list->size;n++) {


      /* case 0 */
      if (list->fsi[n].type == FSISeparator) {nrow1+= nrow2; ncol1 = 0;}
      
      /* case FloatType */
      else if (list->fsi[n].type == FSIFloat) {
        i->pixels[k] = list->fsi[n].val.f; 
        k++;
        ncol1++;
        nrow2 = 1;
      }
              
      /* case SignalType */
      else if (list->fsi[n].type == FSISignal) {
        s1 = list->fsi[n].val.s;
        memcpy(i->pixels+k,s1->Y,s1->size*sizeof(float));k+= s1->size;
        ncol1+= s1->size;
        nrow2 = 1;
      }

      /* case RangeType */
      else if (list->fsi[n].type == FSIRange) {
        rg = list->fsi[n].val.r;
        for (k1 = 0;k1<rg->size;k1++,k++) i->pixels[k] = RangeVal(rg,k1);
        ncol1+= rg->size;
        nrow2 = 1;
      }
      
      /* case ImageType */
      else if (list->fsi[n].type == FSIImage) {
        i1 = list->fsi[n].val.i;
        l = 0;
        for (k2 = nrow1;k2<nrow1+i1->nrow;k2++) {
          for (k1 = ncol1;k1<ncol1+i1->ncol;k1++) {
            i->pixels[k2*ncol+k1] = i1->pixels[l];
            l++;
          }
        }
        ncol1 += i1->ncol;
        nrow2 = i1->nrow;
        k=nrow1*ncol+ncol1;
      }
      
      else Errorf("ExpandFSIList() : weird");

    }

    answer = ImageType;
  }    

  /* Desallocation */
  DeleteFSIList(list);  
  
  return(answer);
}
