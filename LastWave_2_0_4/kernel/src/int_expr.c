/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 3                           */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
/*      email : lastwave@cmap.polytechnique.fr                              */
/*                                                                          */
/*..........................................................................*/
/*                                                                          */
/*      Note : This file implements a small expression evaluator. It is     */
/*             based on an evaluator written by Kittiphan Techakittiroj at  */
/*             Ball State University (16 Dec 1994).                         */
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

#define Clean(val)  if ((val) != NULL) {(val) = NULL;}

unsigned char TTExpr1(char* begin, char** left, float *resFlt,VALUE *resVC, unsigned char flagType,ExprDefParam *defParam); 


/*
 * The default values
 */
int _theSignalSize;
double _theSignalDx,_theSignalX0;
int _theImageNRow;
int _theImageNCol;
float _vmin,_vmax,_vdx;

 /* The level everything is evaluated at */
LEVEL _theLevel;


/*
 * TermExpr -> ....
 *
 */

extern unsigned char TGetVariableContentLevelExpr(LEVEL level, char *begin, char **left, float *resFlt, VALUE *resVC, unsigned char flagType, unsigned char flagSimple, ExprDefParam *defParam);
extern unsigned char TGetFieldOrExtractContentExpr(LEVEL level, VALUE theCont, char *begin, char **left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam);

#define MaxStrLength 10000

static unsigned char TermExpr(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam)
{
  static char str[MaxStrLength+10];
  char *str1,*begin1;
  char *end,*type;
  int j=0;
  unsigned char r;
  unsigned char answer;
  FSIList *list;
  int n;
  VALUE value;
  char flagStop,c;
  char quote;
  LISTV lv;
  char oldFlagSpace;
  int ds,di;
  RANGE rg;
  char oldFlagEmptySI;
  
  answer = 0;
   
  *left = begin; 

  /************************************
   *
   * A number written as a sequence of digits 
   *
   ************************************/
       
  if (flagType & FloatType ) {   
     *resFlt = strtod(begin,&end);
     /* If it is of the form 10.dx, 10a or 10[] then it is not a number */
     if (end != begin && !IsValidSymbolChar1(*end) && *end!= '[') {
       ParamSkipSpace(end);
       (*left) = end;
       Clean(*resVC);
       return(FloatType);
     }
   }
  
  /************************************
   *
   * Test the strings 'string'
   *
   ************************************/

  if (*begin == '\'' || *begin == '\"') {
    if (!(flagType & StringType)) {
      Clean(*resVC);
      SetErrorf("Do not expect a string");
      return(0);
    }

    quote = *begin;
    begin++;
    str1 = str;
    n = 0;
    flagStop = NO;
    
    while (1) {
     
      switch(*begin) {
      
      case '\0' : flagStop = YES; break;

      case '\'' : 
        if (quote != '\'') c = '\'';
        else flagStop = YES;
        break;

      case '"' : 
        if (quote != '"') c = '"';
        else flagStop = YES;
        break;

      case '\\' :
        switch(begin[1]) {
        case '\n' : begin+=2; continue;
        case '\r' : begin+=2; continue;
        case 'n' : c ='\n'; begin++; break;
        case 't' : c ='\t'; begin++; break;
        case 'r' : c ='\r'; begin++; break;
/*        case '{' : c ='{'; begin++; break;
        case '}' : c ='}'; begin++; break;
        case '(' : c ='('; begin++; break;
        case ')' : c =')'; begin++; break;
        case '[' : c ='['; begin++; break;
        case ']' : c =']'; begin++; break;
        case '`' : c ='`'; begin++; break;
 */       case '\'' : c ='\''; begin++; break;
        case '"' : c ='"'; begin++; break;
        case '$' : c ='$'; begin++; break;
        case '\\' : c ='\\'; begin++; break;
        default : Clean(*resVC); *left = begin; SetErrorf("Bad Escaped character"); return(0);
        }
        break;
       
      default :
        c = *begin;
        break;
      }
      
      if (flagStop) break;
                      
      *str1 = c;
      n++;
      if (n>=MaxStrLength) {
        Clean(*resVC);
        Errorf("TermExpr() : Sorry, string is too long, Max length is %d",MaxStrLength);
      }
      str1++;
      begin++;
    }
    
    if (*begin != '\'' && *begin != '\"') {
      Clean(*resVC);
      SetErrorf("Can't find the corresponding quote %c ",quote);
      return(0);
    }
    *str1 = '\0';
        
    Clean(*resVC);
    *resVC = (VALUE) NewStrValue();
    TempValue( *resVC);
    SetStrValue((STRVALUE) *resVC,str);
    begin++;
    ParamSkipSpace(begin);
    *left = begin; 
    answer=StringType;
  }
       

  /************************************
   *
   * Test the listv {}
   *
   ************************************/
 
  else if (*begin == '{') {
    if (!(flagType & ListvType)) {
      Clean(*resVC);
      SetErrorf("Do not expect a listv");
      return(0);
    }
    if (begin[1] == '}') {
      *resVC = (VALUE) TNewListv();
      *left = begin+2;
      ParamSkipSpace(*left);
      answer = ListvType;
    }
    else {
    
      oldFlagSpace = defParam->flagSpace;
      defParam->flagSpace = NO;
    
      lv = TNewListv();
      begin++; 
    
      while (1) {
        Clean(*resVC);

        r = TTExpr1(begin,left,resFlt,resVC,defParam->listvElemType,defParam);
        if (r == 0) return(0);
        begin = *left;
        while (*begin == ' ' || *begin == '\t' || *begin == '\n' || *begin == '\r') begin++;
        if (*begin == '}') {
          *left = begin+1;
          if (r==FloatType) AppendFloat2Listv(lv,*resFlt);
          else AppendValue2Listv(lv,*resVC);
          *resVC = (VALUE) lv;
          defParam->flagSpace = oldFlagSpace;
          ParamSkipSpace(*left);
          answer = ListvType;
          break;
        }
        if (begin == *left) {      
          Clean(*resVC);
          SetErrorf("Expecting '}', i.e., the end of the listv");
          return(0);
        }
        if (r==FloatType) AppendFloat2Listv(lv,*resFlt);
        else AppendValue2Listv(lv,*resVC);
        *left = begin;
      }
    }    
  } 

  /************************************
   *
   * Test the constants
   *
   ************************************/

  /* @<, @> and @+ */   
  else if (!strncmp(begin,"@<",2) || !strncmp(begin,"@>",2) || !strncmp(begin,"@+",2)) {
    if (!(flagType & FloatType)) {
      SetErrorf("Not expecting a number");
      Clean(*resVC);
      return(0);
    }
    if (NoDefaultVector(defParam)) {
      if (NoDefaultSig(defParam)) {
        if (begin[1] != '>') SetErrorf("Cannot infer range parameter '@>'");
        else if (begin[1] != '>') SetErrorf("Cannot infer range parameter '@>'");
        else SetErrorf("Cannot infer range parameter '@+'");
        Clean(*resVC);
        *left = begin;
        return(0);
      }
      if (begin[1] == '<') *resFlt = DefaultSigX0(defParam);
      else if (begin[1] == '+') *resFlt = DefaultSigDx(defParam);
      else *resFlt = DefaultSigSize(defParam);
    }
    else {
      if (begin[1] == '<') *resFlt = DefaultVMin(defParam);
      else if (begin[1] == '>') *resFlt = DefaultVMax(defParam);
      else *resFlt = DefaultVDx(defParam);
    }
    *left = begin+2;
    ParamSkipSpace(*left);
    answer = FloatType;
  }
    
  /* pi */   
  else if (!strncmp(begin,"pi",2) && !IsValidSymbolChar1(begin[2]) && begin[2] != '.') {
    if (!(flagType & FloatType)) {
      SetErrorf("Not expecting a number");
      Clean(*resVC);
      return(0);
    }
    (*left) = begin+2;
    *resFlt = M_PI;
    ParamSkipSpace(*left);
    return(FloatType);
  }
    
  /* urand */   
  else if (!strncmp(begin,"urand",5) && !IsValidSymbolChar1(begin[5]) && begin[5] != '.') {
    if (!(flagType & FloatType)) {
      SetErrorf("Not expecting a number");
      Clean(*resVC);
      return(0);
    }
    (*left) = begin+5;
    *resFlt = Urand();
    ParamSkipSpace(*left);
    return(FloatType);
  }

  /* grand */   
  else if (!strncmp(begin,"grand",5) && !IsValidSymbolChar1(begin[5]) && begin[5] != '.') {
    if (!(flagType & FloatType)) {
      SetErrorf("I am not expecting a number");
      Clean(*resVC);
      return(0);
    }
    (*left) = begin+5;
    *resFlt = Grand(1);
    ParamSkipSpace(*left);
    return(FloatType);
  }

  /* null */   
  else if (!strncmp(begin,"null",4) && !IsValidSymbolChar1(begin[4]) && begin[4] != '.') {
    if (!(flagType & NullType)) {
      SetErrorf("I am not expecting 'null' value");
      Clean(*resVC);
      return(0);
    }
    (*left) = begin+4;
    *resVC = nullValue;
    ParamSkipSpace(*left);
    return(NullType);
  }

  /************************************
   *
   * A float/signal/image <...>
   *
   ************************************/
   
  else if (*begin == '<') {
    begin1 = begin;
    begin++;
    
    ds = NoDefaultSig(defParam);
    di = NoDefaultIm(defParam);

   /* test for empty signal */
    if (*begin == '>') {
      if (flagType & SignalType && (defParam->flagEmptySI || begin[1] == '.')) {
        *resVC = (VALUE) TNewSignal(); 
        answer = SignalType;
        *left = begin+1;
      }
      else if (defParam->flagEmptySI) {
        SetErrorf("Do not expect a signal");
        Clean(*resVC);
        *left = begin1;
        return(0);
      }
      else {
        SetErrorf("Do not expect an empty signal");
        Clean(*resVC);
        *left = begin1;
        return(0);
      }      
    }
    /* test for empty image */
    else if (begin[0] == ';' && begin[1] == '>' ) {
      if (flagType & ImageType  && (defParam->flagEmptySI  || begin[1] == '.')) {
        *resVC = (VALUE) TNewImage(); 
        answer = ImageType;
        *left = begin+2;
      }
      else if (defParam->flagEmptySI) {
        SetErrorf("Do not expect an image");
        Clean(*resVC);
        *left = begin1;
        return(0);
      }
      else {
        SetErrorf("Do not expect an empty image");
        Clean(*resVC);
        *left = begin1;
        return(0);
      }      
    }
    else {          
/*      oldFlagSpace = defParam->flagSpace;
      defParam->flagSpace = YES; deconne pour {<0,0> <0,0>} */
      list = SFIListExpr(begin,left, NULL,defParam); 
/*      defParam->flagSpace = oldFlagSpace; */
     
      if (list == NULL) {
        Clean(*resVC);
        return(0);
      }
      if ((**left)!='>') {
        SetErrorf("Expecting a bracket '>'"); 
        Clean(*resVC);
        DeleteFSIList(list);
        return(0);
      }
     
      (*left)=(*left)+1;
      begin = *left;
      answer = ExpandFSIList(list, resFlt, resVC, AnyType); 
    }
     
    /* Set Default range/sig/im */
    if (answer == SignalType) {
      if (ds) {ForceNoDefaultSig(defParam);}
      else SetDefaultSig(defParam,(SIGNAL) *resVC,NO);
    }
    if (answer == ImageType) {
      if (di) {ForceNoDefaultIm(defParam);}
      else SetDefaultIm(defParam,(IMAGE) *resVC,NO);
    }
  }


  /************************************
   *
   * Test the constants (signals)
   *
   ************************************/

  /* X */   
  else if (!strncmp(begin,"X",1) && !IsValidSymbolChar1(begin[1])) {
    if (DefaultX(defParam) == NULL) {
      if (NoDefaultSig(defParam)) {
        SetErrorf("Cannot infer what 'X' refers to");
        Clean(*resVC);
        return(0);
      }
      rg = TNewRange();
      rg->step = DefaultSigDx(defParam);
      rg->first = DefaultSigX0(defParam);
      rg->size = DefaultSigSize(defParam);
      *resVC = (VALUE) rg;
      answer = RangeType;
      *left = begin+1; 
    }
    else {
      type = GetTypeValue(DefaultX(defParam));
      if (type == rangeType) {
        if (!(flagType & RangeType)) {
          SetErrorf("I am not expecting a range");
          Clean(*resVC);
          return(0);
        }
        *resVC = DefaultX(defParam);
        (*resVC)->nRef++;
        TempValue(*resVC);
        answer = RangeType;
        *left = begin+1; 
      }
      else if (flagType & SignalType) {
        *resVC = DefaultX(defParam);
        (*resVC)->nRef++;
        TempValue(*resVC);
        answer = SignalType;
        *left = begin+1; 
      }
      else {
        SetErrorf("I am not expecting a signal");
        Clean(*resVC);
        return(0);
      }
    }
  }

  /************************************
   *
   * Test an expression ()
   *
   ************************************/
  
  else if (begin[0] == '(') {
    begin1 = begin;
    begin++;
    oldFlagSpace = defParam->flagSpace;
    defParam->flagSpace = YES;
    r = TTExpr1(begin, left,resFlt,resVC,flagType,defParam);
    defParam->flagSpace = oldFlagSpace;
    if (r==NO) return(0);
    if ((**left)!=')') {
      if (**left == '\0') SetErrorf("Expecting a parenthesis ')'"); 
      else SetErrorf("Syntax error");
      Clean(*resVC);
      return(0);
    }
    (*left)=(*left)+1;
    begin = *left;
    while (**left == ' ') (*left)++;
    answer = r;
  }

  /************************************
   *
   * Case of the range ':'
   *
   ************************************/
  
  else if (begin[0] == ':') {
    if (!(flagType & RangeType)) {
      SetErrorf("Not expecting a range"); 
      Clean(*resVC);
      return(0);
    }
    if (NoDefaultVector(defParam)) {
      SetErrorf("Cannot infer what ':' refers to"); 
      Clean(*resVC);
      return(0);
    }
    
    *left = begin+1;
    ((RANGE) *resVC) = TNewRange();
    ((RANGE) *resVC)->size = (int) ((DefaultVMax(defParam)-DefaultVMin(defParam))/DefaultVDx(defParam)+1);
    ((RANGE) *resVC)->step = DefaultVDx(defParam);
    ((RANGE) *resVC)->first = DefaultVMin(defParam);
    
    answer = RangeType; 
  }

  
  /************************************
   *
   * A variable
   *
   ************************************/
  
  else {
    answer = TGetVariableContentLevelExpr(DefaultLevel(defParam),begin,left,resFlt,resVC,flagType,NO,defParam);
    if (answer == SignalType) {SetDefaultSig(defParam,(SIGNAL) *resVC,NO);}
    else if (answer == ImageType) {SetDefaultIm(defParam,(IMAGE) *resVC,NO);}
    else if (answer == RangeType && ((RANGE) *resVC)->step > 0) SetDefaultSizeX0Dx(defParam,((RANGE) *resVC)->size,((RANGE) *resVC)->first,((RANGE) *resVC)->step,NO);    
    return(answer);
  }
  
  
  /************************************
   *
   * If there is a . (i.e., field) or a () (extraction)
   * we must go on...
   *
   ************************************/

  if ((**left == '.' || **left == '[') && *resVC != NULL) {
    value = *resVC;
    *resVC = NULL;
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = YES;
    answer = TGetFieldOrExtractContentExpr(DefaultLevel(defParam), value, *left,left, resFlt,resVC, flagType, defParam);
    defParam->flagEmptySI =  oldFlagEmptySI;
    if (answer == SignalType) {
      if (!defParam->flagEmptySI && ((SIGNAL) *resVC)->size == 0) {
        SetErrorf("Do not expect empty signal");
        *left = begin;
        Clean(*resVC);
        return(0);
      }
      SetDefaultSig(defParam,(SIGNAL) *resVC,NO);
    }
    else if (answer == ImageType) {
      if (!defParam->flagEmptySI && (((IMAGE) *resVC)->nrow == 0 || ((IMAGE) *resVC)->ncol == 0)) {
        SetErrorf("Do not expect empty image");
        *left = begin;
        Clean(*resVC);
        return(0);
      }
      SetDefaultIm(defParam,(IMAGE) *resVC,NO);
    }
    else if (answer == RangeType && ((RANGE) *resVC)->step > 0) SetDefaultSizeX0Dx(defParam,((RANGE) *resVC)->size,((RANGE) *resVC)->first,((RANGE) *resVC)->step,NO);    
    return(answer);
  }

  ParamSkipSpace(*left);

  /* Just return */
  return(answer);

}



/*
 * Generic macro that performs unary operations
 */
#define Apply1(out,in,rg,op) \
  if (in!=NULL) {\
    for (j=0;j<size;j++) out[j] = op(in[j]);\
  }\
  else {\
    for (j=0;j<size;j++) out[j] = op(RangeVal(rg,j));\
  }


/*
 * Generic function that Sets the output of a unary operator on images and signals
 *
 * This function is called below by the TTExpr8 function which deal with unary operator
 *
 * If the argument of the operator is different from an image or a signal then this function is not called.
 *
 * The arguments are : 
 *
 *    - r1, vc1 : correspond to the argument of the unary operator (r1 is the type
 *                and vc1 the corresponding variable content). 
 *
 * The function returns the type of the result and assign vc3 so that the result will be stored in it.
 * (allocation wil be performed only if necessary).
 *      
 * 
 */
static char TTSetOutput1(unsigned char r1, VALUE vc1, VALUE *vc3)
 {
   SIGNAL s3,s1;
   RANGE rg1;
   IMAGE i3,i1;
   char flagTemp1;
   
   if (r1 == SignalType) s1 = (SIGNAL) vc1;
   else if (r1 == ImageType) i1 = (IMAGE) vc1;
   else if (r1 == RangeType) rg1 = (RANGE) vc1;
   else Errorf("TTSetOutput1() : Weird error");
   
   if(vc1->nRef == 1) flagTemp1 = YES;
   else flagTemp1 = NO;
   
   s3 = NULL;
   i3 = NULL;
   if (flagTemp1) {
     if (r1 == SignalType) {*vc3 = (VALUE) s1; return(1);}
     if (r1 == ImageType) {*vc3 = (VALUE) i1; return(1);}
   }
   
   if (r1 == SignalType) {
     s3 = TNewSignal();
     SizeSignal(s3,s1->size,s1->type);
     CopyFieldsSig(s1,s3);
     if (s1->type == XYSIG) memcpy(s3->X,s1->X,s1->size*sizeof(float));
     *vc3 = (VALUE) s3;
     return(1);
   }

   if (r1 == RangeType) {
     s3 = TNewSignal();
     SizeSignal(s3,rg1->size,YSIG);
     *vc3 = (VALUE) s3;
     return(1);
   }

   if (r1 == ImageType) {
     i3 = TNewImage();
     SizeImage(i3,i1->ncol,i1->nrow);
     CopyFieldsImage(i1,i3);
     *vc3 = (VALUE) i3;
     return(1);
   }    
   
   Errorf("SetOutput1() : Weired Error");
   
   return(0);
 }
 
/*
 * expr9 -> most of the unary operators and some special binary ones
 *
 */

static unsigned char TTExpr9(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam)
{
  /* The operators 'op term' or 'op(expr0)' */
  static char *opArray1[] = {"!", "+", "-", "~", NULL};
  static int opSizeArray1[] = {1,1,1,1,0};

  /* The operators 'op(expr0)' */
  static char *opArray2[] = {"sinh","sin","cosh","cos","tanh","tan","acos","asin","atan",\
                             "log2","log","ln","sqrt","abs","exp","ceil","floor","round","frac","int", NULL};
  static int opSizeArray2[] = {4,3,4,3,4,3,4,4,4,4,3,2,4,3,3,4,5,5,4,3,0};

  /* The operators 'op' 'op(expr0_Num)' 'op(expr0_Num,expr0,Num)' */
  static char *opArray3[] = {"Zero","One","Id","I","J","Grand","Urand", NULL};
  static int opSizeArray3[] = {4,3,2,1,1,5,5,0};

  /* The operators 'op(expr0_Sig,expr0_Sig)' */
  static char *opArray4[] = {"XY", NULL};
  static int opSizeArray4[] = {2,0};

  /* The operators 'op(expr0_Sig)' */
  static char *opArray5[] = {"X","der","prim", "diag", NULL};
  static int opSizeArray5[] = {1,3,4,4,0};

  /* The operator 'op(expression)' */
  static char *opArray6[] = {"type","btype", NULL};
  static int opSizeArray6[] = {4,5,0};

  /* The operator 'op(expression)' or 'op(expression,expression)' that returns a float/images/signal */
  static char *opArray7[] = {"min","max", "sum", "any", "all", "mean", NULL};
  static int opSizeArray7[] = {3,3,3,3,3,4,0};

  /* The operator 'op(expression)' that returns a listv */
  static char *opArray8[] = {"find", NULL};
  static int opSizeArray8[] = {4,0};

  char **op;
  int *opSize;
  char opIndex;
  char arrayIndex;

  char oldFlagSpace,oldFlagEmptySI;

  SIGNAL s,s1,s2,s3;
  RANGE rg1,rg2,rg3,rg;
  IMAGE i,i1,i2,i3;
  unsigned char r,r1,r2,r3;
  char flagTemp;
  float f1,f2;
  float *array,*array3,temp,temp1;
  int size,size1,size2,size3;
  int j,j1,n;
  char *begin1,*begin2;
   VALUE vc1,vc2,vc3;
   STRVALUE sc;
  

  /***********************************************************
   * 
   * If it starts with a number just call Term
   *
   ***********************************************************/
   
   if (isdigit(*begin)) {
      r = TermExpr(begin,left,resFlt,resVC,flagType,defParam);
     if (r==NO) return(0);
     return(r);
   }
     

  /***********************************************************
   * 
   * Operators !,+,- and sin, sinh...
   * 
   * 'op term' or 'op(expr0)'
   * 'op(expr0)'
   *
   ***********************************************************/

  /* First array */
  arrayIndex = 1;
  for (op = opArray1, opSize = opSizeArray1 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize)) break; 
  }
  opIndex = op-opArray1+1;

  /* Second array */
  if (*op == NULL) {
    arrayIndex = 2;
    for (op = opArray2, opSize = opSizeArray2 ;;op++,opSize++) {                                                         
      if (*op == NULL) break; 
      if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
    }
    opIndex = op-opArray2+1;
  }
    
  if (*op != NULL) {    
    /* Skip operator */ 
    begin  += *opSize; 
    ParamSkipSpace(begin);
    *left = begin;
    
    /* Case (expr0) */
    if (*begin == '(') {
      begin1 = begin;
      begin++;
      *left = begin;
      oldFlagSpace = defParam->flagSpace;
      defParam->flagSpace = YES;
      oldFlagEmptySI = defParam->flagEmptySI;
      defParam->flagEmptySI = NO;
      r = TTExpr1(begin,left,resFlt,resVC,flagType & ~(ListvType | OtherType | StringType),defParam);
      defParam->flagEmptySI = oldFlagEmptySI;
      defParam->flagSpace = oldFlagSpace;
      if (r==NO) return(0);
      if (**left != ')') {
        SetErrorf("Expecting a parenthesis ')'"); 
        Clean(*resVC);
        return(0);
      }
      (*left)++;
      begin = (*left);      
    }
    
    /* Case term (first array only) */
    else if (arrayIndex == 1) {
      oldFlagEmptySI = defParam->flagEmptySI;
      defParam->flagEmptySI = NO;
      r = TermExpr(begin,left,resFlt,resVC,flagType & ~(ListvType | OtherType | StringType),defParam);
      defParam->flagEmptySI = oldFlagEmptySI;
      if (r==NO) return(0);
      begin = (*left);            
    }
    else {
      Clean(*resVC);
      return(0);
    }

    s = NULL;
    i = NULL;
    rg = NULL;
    if (r == SignalType) s = (SIGNAL) *resVC;
    else if (r == RangeType) rg = (RANGE) *resVC;
    else if (r == ImageType) i = (IMAGE) *resVC;
    else if (r != FloatType) Errorf("TTExpr9() : Weird Error");
    if (r != FloatType && (*resVC)->nRef == 1) flagTemp = YES;
    else flagTemp = NO;
    
    /* If '+' : Nothing to do ! */
    if (arrayIndex == 1 && opIndex == 2) return(r);

    /* If '-' of a range : almost Nothing to do ! */
    if (arrayIndex == 1 && opIndex == 3 && rg != NULL && rg->nRef == 1) {
      rg->first = -rg->first;
      rg->step = -rg->step;
      return(RangeType);
    }

    /* If '~' */
    if (arrayIndex == 1 && opIndex == 4) {
      if (r == FloatType) return(r);
      if (r == ImageType) {
        if (i->ncol != 1) {
          *resVC = (VALUE) TNewImage();
          TranspImage(i,(IMAGE) *resVC);
        }
        else {
          *resVC = (VALUE) TNewSignal();
          SizeSignal((SIGNAL) *resVC,i->nrow,YSIG);
          for(j=0;j<i->nrow;j++) ((SIGNAL) *resVC)->Y[j] = i->pixels[j];
          r = SignalType;
        }
        return(r);
      }
      if (r == SignalType) {
        *resVC = (VALUE) TNewImage();
        SizeImage((IMAGE) *resVC,1,s->size);
        memcpy(((IMAGE) *resVC)->pixels,s->Y,sizeof(float)*s->size);
        return(ImageType);
      }
      if (r == RangeType) {
        *resVC = (VALUE) TNewImage();
        SizeImage((IMAGE) *resVC,1,rg->size);
        for(j=0;j<rg->size;j++) {
          ((IMAGE) *resVC)->pixels[j] = rg->first+j*rg->step;
        }
        return(ImageType);
      }
      Errorf("Not yet implemented");
    }
    
    /* Set output */
    if (r == FloatType) {Clean(*resVC);}
    else TTSetOutput1(r,*resVC,&vc3);

    /* Set Input/Output arrays */
    s3 = NULL;
    i3 = NULL;
    rg3 = NULL;
    if (r==SignalType) {
      s3 = (SIGNAL) vc3;
      r3 = SignalType;
      size = s->size;
      array = s->Y; 
      array3 = s3->Y; 
    }
    else if (r==RangeType) {
      s3 = (SIGNAL) vc3;
      r3 = SignalType;
      size = rg->size;
      array = NULL;
      array3 = s3->Y; 
    }
    else if (r==ImageType) {
      i3 = (IMAGE) vc3;
      r3 = ImageType;
      array = i->pixels;
      size = i->nrow*i->ncol;
      array3 = i3->pixels; 
    }
    else {
      array3 = array = resFlt;
      size = 1;
      r3 = FloatType;
    } 
    
    /* Computation */
    if (arrayIndex == 1) {
      switch(opIndex) {

        /* ! */ 
        case 1 : 
          /* Range case */
          if (array == NULL) {for (j=0;j<size;j++) array3[j] = (RangeVal(rg,j)==0); break;}
          /* Regular Signal case */
          else { for (j=0;j<size;j++) array3[j] = (array[j]==0); break;}

        /* - */ 
        case 3 : 
          /* Vector case */
          if (array == NULL) {for (j=0;j<size;j++) array3[j] = -(RangeVal(rg,j)); break;}
          /* Regular Signal case */
          else {for (j=0;j<size;j++) array3[j] = -array[j]; break;}

        default : Errorf("Expr9() : Weired error Array1");
      }
    }
    else {
      switch(opIndex) {

        /* sinh */ 
        case 1 : Apply1(array3,array,rg,sinh); break;

        /* sin */ 
        case 2 : Apply1(array3,array,rg,sin); break;

        /* cosh */ 
        case 3 : Apply1(array3,array,rg,cosh); break;

        /* cos */ 
        case 4 : Apply1(array3,array,rg,cos); break;

        /* tanh */ 
        case 5 : Apply1(array3,array,rg,tanh); break;

        /* tan */ 
        case 6 : Apply1(array3,array,rg,tan); break;

        /* acos */ 
        case 7 : Apply1(array3,array,rg,acos); break;

        /* asin */ 
        case 8 : Apply1(array3,array,rg,asin); break;

        /* atan */ 
        case 9 : Apply1(array3,array,rg,atan); break;

        /* log2 */ 
        case 10 : Apply1(array3,array,rg,MyLog2); break;

        /* log */ 
        case 11 : Apply1(array3,array,rg,MyLog10); break;

        /* ln */ 
        case 12 : Apply1(array3,array,rg,MyLog); break;

        /* sqrt */ 
        case 13 : Apply1(array3,array,rg,MySqrt); break;

        /* abs */ 
        case 14 : Apply1(array3,array,rg,fabs); break;

        /* exp */ 
        case 15 : Apply1(array3,array,rg,exp); break;

        /* ceil */ 
        case 16 : Apply1(array3,array,rg,ceil); break;

        /* floor */ 
        case 17 : Apply1(array3,array,rg,floor); break;

        /* round */ 
        case 18 : 
          /* Range case */
          if (array == NULL) {for (j=0;j<size;j++) array3[j] = (RangeVal(rg,j) >= 0 ? (int) (RangeVal(rg,j)+.5) : (int) (RangeVal(rg,j)-.5)); break;}          
          /* Regular Signal case */
          else {for (j=0;j<size;j++) array3[j] = (array[j] >= 0 ? (int) (array[j]+.5) : (int) (array[j]-.5)); break;}

        /* frac */ 
        case 19 : 
          /* Vector case */
          if (array == NULL) {for (j=0;j<size;j++) array3[j] = RangeVal(rg,j) - ((int) RangeVal(rg,j));  break;}          
          /* Regular Signal case */
          else {for (j=0;j<size;j++) array3[j] = array[j] - ((int) array[j]); break;}

        /* int */ 
        case 20 : Apply1(array3,array,rg,(int)); break;
        
        default : Errorf("Expr8() : Weired error Array2");
      }
    }   
    
    if (s3) *resVC = (VALUE) s3;
    else if (i3) *resVC = (VALUE) i3;
    else if (rg3) *resVC = (VALUE) rg3;
    
    if (r3 == SignalType && NoDefaultSig(defParam)) {SetDefaultSig(defParam,((SIGNAL) *resVC),NO);}
    else if (r3 == ImageType && NoDefaultSig(defParam)) {SetDefaultIm(defParam,((IMAGE) *resVC),NO);}
    
    return(r3);
  }
  
  
  
  
  /***********************************************************
   * 
   * Operators "Zero","One","Id","I","J","Grand","Urand"
   * 
   * 'op' 'op(expr0_Num)' 'op(expr0_Num,expr0_Num)' 
   *
   ***********************************************************/

  /* Third array */
  arrayIndex = 3;
  for (op = opArray3, opSize = opSizeArray3 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
  }
  opIndex = op-opArray3+1;
    
  if (*op != NULL) {    

    /* Check some types */
    if (flagType == FloatType) {
      SetErrorf("Expecting a number, not a signal or an image !");
      Clean(*resVC);
      return(0);
    }

    /* Skip operator */
    begin2= begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin); 
    *left = begin;


    /* Case there is a op(expr0_Int... */
    f1 = f2 = -1;
    s3 = NULL;
    i3 = NULL;
    if (*begin == '(') {
      begin1 = begin;
      begin++;
      *left = begin;
      oldFlagSpace = defParam->flagSpace;
      defParam->flagSpace = YES;
      if (!TTExpr1(begin,left,resFlt,resVC,FloatType,defParam)) return(0);
      if (*resFlt <= 0 || *resFlt != ((int) *resFlt)) {
        *left = begin;
        SetErrorf("Bad dimension '%g'",*resFlt);
        Clean(*resVC);
        return(0);
      }
      r = SignalType;
      f1 = *resFlt;
      begin = *left;
      
      /* Case of op(expr0_Int,expr0_Int) */
      if (**left == ',') {
        
        /* Error if Id(n, */
        if (opIndex==3) {
          *left = begin2;
          SetErrorf("The operator 'Id' takes only 1 argument");
          Clean(*resVC);
          return(0);
        }

        r = ImageType;
        begin++;
        *left = begin;
        if (!TTExpr1(begin,left,resFlt,resVC,FloatType,defParam)) {
          Clean(*resVC);
          return(0);
        }
        if (*resFlt <= 0 || *resFlt != ((int) *resFlt)) {
          *left = begin;
          SetErrorf("Bad dimension '%g'",*resFlt);
          Clean(*resVC);
          return(0);
        }
        begin = *left;
        f2 = *resFlt;
        if (**left != ')') {
          SetErrorf("Expecting a parenthesis ')'"); 
          Clean(*resVC);
          return(0);
        }
        (*left)++;
        begin = (*left);
        defParam->flagSpace = oldFlagSpace;
      }
      

      /* Error if J(n) */
      else if (opIndex==5) {
        *left = begin2;
        SetErrorf("The operator 'J' is missing one argument");
        Clean(*resVC);
        return(0);
      }
      
      /* Closing parenthesis of  op(expr0_Int) */
      else {
        if (**left != ')') {
          SetErrorf("Expecting a parenthesis ')'"); 
          Clean(*resVC);
          return(0);
        }
        (*left)++;
        begin = (*left);
        defParam->flagSpace = oldFlagSpace;

        /* Id(n) */
        if (opIndex==3) {f2 = f1; r = ImageType;}
      }

      
      /* Set the default sig/images */
      if (f2 == -1) {SetDefaultSizeX0Dx(defParam,f1,0,1,NO);}
      else if (f1 == 1) {SetDefaultSizeX0Dx(defParam,f2,0,1,NO);}
      else if (f2 != -1) {SetDefaultNColNRow(defParam,f2,f1,NO);}
    }

    /* Case there is a op */    
    else {
      /* if the operator is not J and we don't know if we should build an image or a signal error */
      if ((flagType & ImageType && defParam->_theImageNRow != -1) && (flagType & SignalType && defParam->_theSignalSize!=-1) && opIndex!=5) {
        SetErrorf("Cannot infer whether I should build a signal or an image (you should specify the sizes)");
        *left = begin2;
        Clean(*resVC);
        return(0);
      }
      /* J ==> image, otherwise we check what is expected */
      if (opIndex == 5) {
        r = ImageType;
        if (defParam->_theImageNRow == -1)  {
          SetErrorf("Cannot infer the size for the operator '%s'",opArray3[opIndex-1]);
          *left = begin2;
          Clean(*resVC);
          return(0);
        }
      } 
      else if (flagType & SignalType && defParam->_theSignalSize != -1) r = SignalType;
      else if (flagType & ImageType && defParam->_theImageNRow != -1) r = ImageType;
      else {
        SetErrorf("Cannot infer the size for the operator '%s'",opArray3[opIndex-1]);
        *left = begin2;
        Clean(*resVC);
        return(0);
      }

      /* Id ==> squared image, otherwise error */
      if (opIndex == 3 && r == SignalType) {
        SetErrorf("You should specify an argument to 'Id' operator");
        *left = begin2;
        Clean(*resVC);
        return(0);
      }
      if (opIndex == 3 &&  (defParam->_theImageNRow == -1 || defParam->_theImageNRow != defParam->_theImageNCol)) {
        SetErrorf("You should specify an argument to 'Id' operator (default image is not square)");
        *left = begin2;
        Clean(*resVC);
        return(0);
      }
    }
    
    /* Some conversions */
    if (f1 == 1 && (flagType & SignalType)) {
      f1 = f2;
      f2 = -1;
      r = SignalType;
    }
        
    /* Check some types */
    if (!(r & flagType)) {
      if (r == SignalType) SetErrorf("Expecting an image");
      else SetErrorf("Expecting a signal");
      Clean(*resVC);
      *left = begin2;
      return(0);
    }
    
    /* Setting f1 and f2 (the signal/image dimension) */
    if (f1 == -1 && r==SignalType) {
      if (NoDefaultSig(defParam)) {
        *left = begin2;
        SetErrorf("Do not know the size of the signal");
        Clean(*resVC);
        return(0);
      }
      f1 = DefaultSigSize(defParam);
    }
    else if (f1 == -1 && r==ImageType) {
      if (NoDefaultIm(defParam) == -1) {
        *left = begin2;
        SetErrorf("Do not know the size of the image");
        Clean(*resVC);
        return(0);
      }
      f1 = DefaultImNRow(defParam);
      f2 = DefaultImNCol(defParam);
    }
      
    /* Allocation */
    if (r==SignalType) {
      
      /* Case of I (=>Vector) */
      if (opIndex == 4) {
        Clean(*resVC);
        rg3 = TNewRange();
        *resVC = (VALUE) rg3;
        rg3->first = 0;
        rg3->step = 1;
        rg3->size = f1;
        SetDefaultSizeX0Dx(defParam,rg->size,0,1,NO);
        return(RangeType);
      }
      
      if (*resVC != NULL && (*resVC)->nRef == 1 && (GetTypeValue(*resVC) == signaliType || GetTypeValue(*resVC) == signalType)) s3 = (SIGNAL) *resVC;
      else {
        Clean(*resVC);
        s3 = TNewSignal();
        *resVC = (VALUE) s3;
      }

      /* Case of J (=>One) */
      if (opIndex == 5) opIndex = 2;

      SizeSignal(s3,(int) f1,YSIG);
    }
    else {
      if (*resVC != NULL && (*resVC)->nRef == 1 && (GetTypeValue(*resVC) == imageiType || GetTypeValue(*resVC) == imageType)) i3 = (IMAGE) *resVC;
      else {
        Clean(*resVC);
        i3 = TNewImage();
        *resVC = (VALUE) i3;
      }
      SizeImage(i3,f2,f1);
    }
    
    /* Set Input/Output arrays */
    if (*resVC != NULL && r == SignalType) { 
      size = s3->size;
      array3 = s3->Y; 
    }
    else if (*resVC != NULL) {
      size = i3->nrow*i3->ncol;
      array3 = i3->pixels; 
    }


    /* Computation */
    switch(opIndex) {

      /* Zero */ 
      case 1 : for (j=0;j<size;j++) array3[j] = 0; break;

      /* One */ 
      case 2 : for (j=0;j<size;j++) array3[j] = 1; break;

      /* Id */ 
      case 3 : for(j=0;j<f1;j++) for(j1=0;j1<f2;j1++) i3->pixels[j * ((int) f2) + j1] = (j==j1); break;

      /* I */ 
      case 4 : 
        if (i3 == NULL) break;
        else  for(j=0;j<f1;j++) for(j1=0;j1<f2;j1++) i3->pixels[j * ((int) f2) + j1] = j1;
        break;

      /* J */ 
      case 5 :  for(j=0;j<f1;j++) for(j1=0;j1<f2;j1++) i3->pixels[j * ((int) f2) + j1] = j; break;

      /* Grand */ 
      case 6 : for (j=0;j<size;j++) array3[j] = Grand(1); break;

      /* Urand */ 
      case 7 : for (j=0;j<size;j++) array3[j] = Urand(); break;

      default : Errorf("Expr8() : Weired error Array3");
    }
    
    if (r==SignalType) {SetDefaultSig(defParam,(SIGNAL) (*resVC),NO);}
    else if (r==ImageType) {SetDefaultIm(defParam,(IMAGE) (*resVC),NO);}

    return(r);
  }

  /***********************************************************
   * 
   * Operator "XY"
   * 
   * 'op(expr0_Sig,expr0_Sig)'
   *
   ***********************************************************/

  /* Fourth array */
  arrayIndex = 4;
  for (op = opArray4, opSize = opSizeArray4 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
  }
  opIndex = op-opArray4+1;
    
  if (*op != NULL) {    

    /* Check some types */
    if (!(flagType & SignalType)) {
      SetErrorf("Expecting a signal !");
      Clean(*resVC);
      return(0);
    }

    /* Skip operator */ 
    begin2= begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
    *left = begin;

    /* Check '(' */
    if (*begin != '(') {
      *left = begin;
      SetErrorf("Missing a parenthesis '('");
      Clean(*resVC);
      return(0);
    }
    begin1 = begin;
    begin++;
    *left = begin;
    
    /* Get first argument */
    oldFlagSpace = defParam->flagSpace;
    defParam->flagSpace = YES;
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    if (!(r1=TTExpr1(begin,left,resFlt,resVC,SignalType | RangeType,defParam))) return(0);
    defParam->flagEmptySI = oldFlagEmptySI;

    vc1 = *resVC;
   
    /* Read 'op(expr0_Sig,expr0_Sig)' */
    begin = *left;
    if (*begin != ',') {
      SetErrorf("Missing a comma ','");
      Clean(*resVC);
      return(0);
    }
    s1 = NULL;
    rg1 = NULL;
    if (r1 == SignalType) {s1 = (SIGNAL) vc1; size1 = s1->size;}
    else {rg1 = (RANGE) vc1; size1 = rg1->size;}
    begin++;
    *left = begin;
        
    /* Get second argument */
    SetDefaultX(defParam,vc1);
    vc2 = NULL;
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    if (!(r2 = TTExpr1(begin,left,&f2,&vc2,SignalType | RangeType,defParam))) {
      Clean(*resVC);
      return(0);
    }
    defParam->flagEmptySI = oldFlagEmptySI;
    SetDefaultX(defParam,NULL);
    s2 = NULL;
    rg2 = NULL;
    if (r2 == SignalType) {s2 = (SIGNAL) vc2; size2 = s2->size;}
    else {rg2 = (RANGE) vc2;  size2 = rg2->size;}
    if (size1 != size2) {
      *left = begin2;
      SetErrorf("Signals/Ranges have different size");
      Clean(vc1);
      Clean(vc2);
      *resVC = NULL;
      return(0);
    }

    /* Closing parenthesis */
    if (**left != ')') {
      SetErrorf("Expecting a parenthesis ')'"); 
      Clean(vc1);
      Clean(vc2);
      *resVC = NULL;
      return(0);
    }
    (*left)++;
    begin = (*left);

    defParam->flagSpace = oldFlagSpace;

    s3 = TNewSignal();
    if (rg1 && rg1->step > 0) {
      SizeSignal(s3,rg1->size,YSIG);
      if (s2) for (j=0;j<rg1->size;j++) {s3->Y[j] = s2->Y[j];}
      else  for (j=0;j<rg1->size;j++) {s3->Y[j] = RangeVal(rg2,j);}
      s3->dx = rg1->step;
      s3->x0 = rg1->first;
    }
    else {
      SizeSignal(s3,size1,XYSIG);
      if (rg1  && s2) for (j=0;j<rg1->size;j++) {s3->X[j] = RangeVal(rg1,j);s3->Y[j] = s2->Y[j];}
      else if (rg1  && rg2) for (j=0;j<rg1->size;j++) {s3->X[j] = RangeVal(rg1,j);s3->Y[j] = RangeVal(rg2,j);}
      else if (s1 && rg2) for (j=0;j<s1->size;j++) {s3->X[j] = s1->Y[j];s3->Y[j] = RangeVal(rg2,j);}
      else  for (j=0;j<s1->size;j++) {s3->X[j] = s1->Y[j];s3->Y[j] = s2->Y[j];}
    }
    Clean(vc1);
    Clean(vc2);
    *resVC = (VALUE) s3;
    if (s3->type == XYSIG) SortSig(s3);
    return(SignalType);
  }
             

  /***********************************************************
   * 
   * Operators "X","der","prim"
   * 
   * 'op(expr0_Sig)'
   *
   ***********************************************************/

  /* Fifth array */
  arrayIndex = 5;
  for (op = opArray5, opSize = opSizeArray5 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
  }
  opIndex = op-opArray5+1;
    
  if (*op != NULL && (opIndex != 1 || begin[*opSize] == '(')) {    

    /* Check some types */
    if (!(flagType & SignalType)) {
      SetErrorf("Not expecting a signal");
      Clean(*resVC);
      return(0);
    }

    /* Skip operator */ 
    begin2= begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
    *left = begin;

    /* Check '(' */
    if (*begin != '(') {
      SetErrorf("Missing a parenthesis '('");
      Clean(*resVC);
      return(0);
    }
    begin1= begin; 
    begin++;
    *left = begin;
    
    /* Get argument */
    oldFlagSpace = defParam->flagSpace;
    defParam->flagSpace = YES;
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    if (!(r=TTExpr1(begin,left,resFlt,resVC,SignalType | RangeType,defParam))) return(0);
    defParam->flagEmptySI = oldFlagEmptySI;

    /* Closing parenthesis of op(expr0_Int) */
    if (**left != ')') {
      SetErrorf("Expecting a parenthesis ')'"); 
      Clean(*resVC);
      return(0);
    }
    (*left)++;
    begin = (*left);
    
    defParam->flagSpace = oldFlagSpace;

    /* Set output */
    if (opIndex != 1 && opIndex != 4) TTSetOutput1(r,*resVC,&vc3);
    else if (opIndex == 1) {
      AddRefValue(*resVC);
      TTSetOutput1(r,*resVC,&vc3);
      RemoveRefValue(*resVC);
    }
    s = NULL;
    rg = NULL;
    if (r == SignalType) s = (SIGNAL) *resVC;
    else rg = (RANGE) *resVC;
    if (opIndex != 4) s3 = (SIGNAL) vc3;
    else {
      i3 = TNewImage();
      if (rg) SizeImage(i3,rg->size,rg->size);
      else SizeImage(i3,s->size,s->size);
      vc3 = (VALUE) i3;
    }

    /* Computation */
    switch(opIndex) {
    
      /* X */
      case 1 : 
        if (rg) for (j=0;j<s3->size;j++) s3->Y[j] = j;
        else for (j=0;j<s3->size;j++) s3->Y[j] = XSig(s,j); 
        break;
        
      /* der */
      case 2 : 
        if (rg) {
          temp = s3->Y[0] = RangeVal(rg,0);
          for (j=1;j<s3->size;j++) {
            temp1 = RangeVal(rg,j);
            s3->Y[j] = temp1-temp;
            temp = temp1;
          }
        }
        else {
          temp = s3->Y[0] = s->Y[0];
          for (j=1;j<s3->size;j++) {
            temp1 = s->Y[j];
            s3->Y[j] = temp1-temp;
            temp = temp1;
          }
        }
        break;

      /* prim */
      case 3 : 
        if (rg) {
          s3->Y[0] = RangeVal(rg,0);
          for (j=1;j<s3->size;j++) s3->Y[j] = RangeVal(rg,j) + s3->Y[j-1];
        }
        else {
          s3->Y[0] = s->Y[0];
          for (j=1;j<s3->size;j++) {
          s3->Y[j] = s->Y[j] + s3->Y[j-1];
          }
        }
        break;

      /* diag */
      case 4 : 
        ZeroImage(i3);
        if (rg) {
          for (j=0;j<rg->size;j++) i3->pixels[j*(i3->ncol+1)] = RangeVal(rg,j);
        }
        else {
          for (j=0;j<s->size;j++) i3->pixels[j*(i3->ncol+1)] = s->Y[j];
        }
        break;
        
      default : Errorf("Expr9() : Weired error Array3");
    }

    if (*resVC != vc3) {
      Clean(*resVC);
      *resVC = vc3;
    }
    return(r);
  }
  
 
   /***********************************************************
   * 
   * Operators "type"
   * 
   * 'op(expr)'
   *
   ***********************************************************/

  /* Sixth array */
  arrayIndex = 6;
  for (op = opArray6, opSize = opSizeArray6 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
  }
  opIndex = op-opArray6+1;
    
  if (*op != NULL && (opIndex != 1 || begin[*opSize] == '(')) {    

    /* Check some types */
    if (!(flagType & StringType)) {
      SetErrorf("Not expecting a string");
      Clean(*resVC);
      return(0);
    }

    /* Skip operator */ 
    begin2= begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
    *left = begin;

    /* Check '(' */
    if (*begin != '(') {
      SetErrorf("Missing a parenthesis '('");
      Clean(*resVC);
      return(0);
    }
    begin1= begin; 
    begin++;
    *left = begin;
    
    /* Get argument */
    oldFlagSpace = defParam->flagSpace;
    defParam->flagSpace = YES;
    if (!(r=TTExpr1(begin,left,resFlt,resVC,AnyType,defParam))) return(0);

    /* Closing parenthesis of op(expr) */
    if (**left != ')') {
      SetErrorf("Expecting a parenthesis ')'"); 
      Clean(*resVC);
      return(0);
    }
    (*left)++;
    begin = (*left);
    
    defParam->flagSpace = oldFlagSpace;
    
    /* Computation */
    switch(opIndex) {
    
      /* type */
      case 1 : 
        sc = TNewStrValue();
        if (*resVC == NULL) SetStrValue(sc,numType);
        else SetStrValue(sc,GetTypeValue(*resVC));
        *resVC = (VALUE) sc;        
        r = StringType;
        break;

      /* btype */
      case 2 : 
        sc = TNewStrValue();
        if (*resVC == NULL) SetStrValue(sc,numType);
        else SetStrValue(sc,GetBTypeContent(*resVC));
        *resVC = (VALUE) sc;        
        r = StringType;
        break;
        
      default : Errorf("Expr9() : Weired error Array6");
    }

    return(r);
  }

  /***********************************************************
   * 
   * Operators "min","max"
   * 
   * 'op' 'op(expression)' 'op(num,num)' 
   *
   ***********************************************************/

  /* Seventh array */
  arrayIndex = 7;
  for (op = opArray7, opSize = opSizeArray7 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
  }
  opIndex = op-opArray7+1;
    
  if (*op != NULL) {    

    /* Check some types */
    if (!(flagType & (FloatType | SignalType | ImageType))) {
      SetErrorf("Not expecting a number, a signal or an image");
      Clean(*resVC);
      return(0);
    }

    /* Skip operator */
    begin2= begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin); 
    *left = begin;


    /* test (... */
    if (*begin != '(') {
      SetErrorf("Expecting a '('");
      *left = begin;
      Clean(*resVC);
      return(0);
    }
    begin2 = begin;
    
    /* Read expression */
    i3 = NULL;
    begin1 = begin;
    begin++;
    *left = begin;
    oldFlagSpace = defParam->flagSpace;
    defParam->flagSpace = YES;
    if (!(r=TTExpr1(begin,left,resFlt,resVC,FloatType | RangeType | ImageType | SignalType,defParam))) return(0);
    defParam->flagSpace = oldFlagSpace;
    
    /* Case of op(expression) */
    if (**left == ')') {
      (*left)++;
      if (!(flagType & FloatType)) {
        SetErrorf("Not expecting a number");
        *left = begin;
        Clean(*resVC);
        return(0);
      }
      if (r == FloatType) {
        Clean(*resVC);
        return(FloatType);
      }
      if (r != RangeType) {
        if (r == SignalType) {
          s3 = (SIGNAL) *resVC;
          array3 = s3->Y;
          size3 = s3->size;
          if (size3 == 0) {
            SetErrorf("Not expecting emty signal");
            *left = begin2+1;
            Clean(*resVC);
            return(0);
          }
        }
        else if (r == FloatType) {
          array3 = resFlt;
          size3 = 1;
        }
        else {
          i3 = (IMAGE) *resVC;
          array3 = i3->pixels;
          size3 = i3->ncol*i3->nrow;
          if (size3 == 0) {
            SetErrorf("Not expecting emty image");
            *left = begin2+1;
            Clean(*resVC);
            return(0);
          }
        }

        switch(opIndex) {
        
        /* min */
        case 1:
          *resFlt = array3[0];
          for(j=1;j<size3;j++) *resFlt = MIN(*resFlt,array3[j]);
          break;
        
        /* max */
        case 2:
          *resFlt = array3[0];
          for(j=1;j<size3;j++) *resFlt = MAX(*resFlt,array3[j]);
          break;

        /* sum */
        case 3:
          *resFlt = array3[0];
          for(j=1;j<size3;j++) *resFlt += array3[j];
          break;

        /* any */
        case 4:
          *resFlt = 0;
          for(j=0;j<size3;j++) if (array3[j] != 0) {*resFlt = 1; break;}
          break;

        /* all */
        case 5:
          *resFlt = 1;
          for(j=0;j<size3;j++) if (array3[j] == 0) {*resFlt = 0; break;}
          break;

        /* mean */
        case 6:
          *resFlt = array3[0];
          for(j=1;j<size3;j++) *resFlt += array3[j];
          *resFlt/=size3;
          break;

        }

        Clean(*resVC);
        return(FloatType); 
      }
      
      if (r == RangeType) {
        rg3 = (RANGE) *resVC;
        if (rg3->size == 0) {
          SetErrorf("Not expecting emty range");
          *left = begin2+1;
          Clean(*resVC);
          return(0);
        }
        switch(opIndex) {
        
        /* sum */
        case 3 :
          *resFlt = rg3->size*rg3->first+rg3->step*rg3->size*(rg3->size-1)/2;
          break;

        /* any */
        case 4 :
          if (rg3->first != 0 || rg3->size != 1) *resFlt = 1;
          else *resFlt = 0;
          break;

        /* all */
        case 5 : 
          f1 = -rg3->first/rg3->step;
          if (f1 >= 0 && f1 <rg3->size && f1 == ((int) f1)) *resFlt = 0;
          else *resFlt = 1;
          break;

        /*min and max */
        case 1: case 2:          
          if (rg3->step >= 0) {
            if (opIndex == 1) *resFlt = rg3->first;
            else if (opIndex == 1) *resFlt = rg3->first;
            else *resFlt = RangeLast(rg3);
          }
          else {
            if (opIndex == 2) *resFlt = rg3->first;
            else *resFlt = RangeLast(rg3);
          }
          break;
        }  
        
        Clean(*resVC);
        return(FloatType); 
      }
    }      


     /* Case op(expression,expression) */
     if (opIndex >= 3) {
       SetErrorf("Bad syntax");
       *left = begin;
       Clean(*resVC);
       return(0);
     }

     if (**left != ',') {
       SetErrorf("Expecting a comma");
       *left = begin;
       Clean(*resVC);
       return(0);
     }
     
     vc3 = *resVC;
     temp = *resFlt;
     begin = *left+1;
     oldFlagSpace = defParam->flagSpace;
     defParam->flagSpace = YES;
     if (r == SignalType || r == RangeType) r = RangeType | SignalType;
     if (!(r=TTExpr1(begin,left,resFlt,resVC,r,defParam))) return(0);
     defParam->flagSpace = oldFlagSpace;
     if (**left != ')') {
       SetErrorf("Expecting a ')'");
       *left = begin;
       Clean(*resVC);
       return(0);
     }

     if (r & flagType == 0) {
       if (r==FloatType) SetErrorf("Not expecting a float");
       else if (r==SignalType) SetErrorf("Not expecting a signal");
       else if (r==ImageType) SetErrorf("Not expecting an image");
       *left = begin2;
       Clean(*resVC);
       return(0);
     }
     
     (*left)++;
     
     /* Case of floats */
     if (r == FloatType) {
       if (opIndex == 1) *resFlt = MIN(*resFlt,temp);
       else *resFlt = MAX(*resFlt,temp);
       Clean(*resVC);
       return(r);
     }  

     /* Case of images */
     if (r == ImageType) {
       i1 = (IMAGE) *resVC;
       i2 = (IMAGE) vc3;
       if (i1->nrow != i2->nrow || i1->ncol != i2->ncol) {
         SetErrorf("Images should have the same size");
         *left = begin2;
         Clean(*resVC);
         return(0);
       }
       if (i1->nRef == 1) i3 = i1;
       else if (i2->nRef == 1) i3 = i2;
       else {
         i3 = TNewImage();
         SizeImage(i3,i1->ncol,i1->nrow);
       }
       *resVC = (VALUE) i3;
       if (opIndex == 1) {
         for (j=0;j<i3->nrow*i3->ncol;j++) i3->pixels[j] = MIN(i1->pixels[j],i2->pixels[j]);
       }
       else {
         for (j=0;j<i3->nrow*i3->ncol;j++) i3->pixels[j] = MAX(i1->pixels[j],i2->pixels[j]);
       }
       return(r);
     }  


     /* Case of signals/ranges */
     if (GetTypeValue(vc3) == rangeType) {
       s1 = NULL;
       rg1 = (RANGE) vc3;
       size1 = rg1->size;
     }
     else {
       s1 = (SIGNAL) vc3;
       rg1 = NULL;
       size1 = s1->size;
     }
     if (r == RangeType) {
       s2 = NULL;
       rg2 = (RANGE) *resVC;
       size2 = rg2->size;
     }
     else {
       s2 = (SIGNAL) *resVC;
       rg2 = NULL;
       size2 = s2->size;       
     }

     if (size1 != size2) {
       SetErrorf("Signals/ranges should have the same size");
       *left = begin2;
       Clean(*resVC);
       return(0);
     }
     if (s1 && s1->nRef == 1) s3 = s1;
     else if (s2 && s2->nRef == 1) s3 = s2;
     else {
       s3 = TNewSignal();
       if (s1) {
         SizeSignal(s3,size1,s1->type);
         CopyFieldsSig(s1,s3);
         if (s1->type == XYSIG) memcpy(s3->X,s1->X,s1->size*sizeof(float));
       }
       else SizeSignal(s3,size1,YSIG);
     }
     *resVC = (VALUE) s3;
     if (opIndex == 1) {
       if (rg1) {
         if (rg2) for (j=0;j<s3->size;j++) s3->Y[j] = MIN(RangeVal(rg1,j),RangeVal(rg2,j));
         else for (j=0;j<s3->size;j++) s3->Y[j] = MIN(RangeVal(rg1,j),s2->Y[j]);
       }
       else {
         if (rg2) for (j=0;j<s3->size;j++) s3->Y[j] = MIN(s1->Y[j],RangeVal(rg2,j));
         else for (j=0;j<s3->size;j++) s3->Y[j] = MIN(s1->Y[j],s2->Y[j]);
       }
     }
     else {
       if (rg1) {
         if (rg2) for (j=0;j<s3->size;j++) s3->Y[j] = MAX(RangeVal(rg1,j),RangeVal(rg2,j));
         else for (j=0;j<s3->size;j++) s3->Y[j] = MAX(RangeVal(rg1,j),s2->Y[j]);
       }
       else {
         if (rg2) for (j=0;j<s3->size;j++) s3->Y[j] = MAX(s1->Y[j],RangeVal(rg2,j));
         else for (j=0;j<s3->size;j++) s3->Y[j] = MAX(s1->Y[j],s2->Y[j]);
       }
     }
     return(r);
  }

  /***********************************************************
   * 
   * Operators "find"
   * 
   * 'op(expression)' returns a signal/image
   *
   ***********************************************************/

  /* Eighth array */
  arrayIndex = 8;
  for (op = opArray8, opSize = opSizeArray8 ;;op++,opSize++) {                                                         
    if (*op == NULL) break; 
    if (!strncmp(begin,*op,*opSize) && !IsValidSymbolChar(begin[*opSize])) break; 
  }
  opIndex = op-opArray8+1;
    
  if (*op != NULL) {    

    *left = begin;
    
    /* Check some types */
    if (!(flagType & (ImageType | SignalType))) {
      SetErrorf("Not expecting a signal or an image");
      Clean(*resVC);
      return(0);
    }

    /* Skip operator */
    begin2= begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin); 
    *left = begin;


    /* test (... */
    if (*begin != '(') {
      SetErrorf("Expecting a '('");
      *left = begin;
      Clean(*resVC);
      return(0);
    }
    begin2 = begin;
    
    /* Read expression */
    begin1 = begin;
    begin++;
    *left = begin;
    oldFlagSpace = defParam->flagSpace;
    defParam->flagSpace = YES;
    if (!(r=TTExpr1(begin,left,resFlt,resVC,ImageType | SignalType,defParam))) return(0);
    defParam->flagSpace = oldFlagSpace;
    
    if (**left != ')') {
      SetErrorf("Expecting a ')'");
      Clean(*resVC);
      return(0);    
    }

    (*left)++;
    if (r == SignalType) {
      s1 = (SIGNAL) *resVC;
      s3 = TNewSignal();
      for(j1=0,j=0;j<s1->size;j++) {
        if (s1->Y[j] != 0) j1++;
      }
      SizeSignal(s3,j1,YSIG);
      for(j1=0,j=0;j<s1->size;j++) {
        if (s1->Y[j] != 0) s3->Y[j1++] = j;
      }
      *resVC = (VALUE) s3;
      return(SignalType);
    }


    if (r == ImageType) {
      i1 = (IMAGE) *resVC;
      i3 = TNewImage();
      for(n=0,j=0;j<i1->nrow;j++) {
        for(j1=0;j1<i1->ncol;j1++) {
          if (i1->pixels[j*i1->ncol+j1] != 0) n++;
        }
      }
      SizeImage(i3,n,2);
      for(n=0,j=0;j<i1->nrow;j++) {
        for(j1=0;j1<i1->ncol;j1++) {
          if (i1->pixels[j*i1->ncol+j1] != 0) {
            i3->pixels[n] = j;
            i3->pixels[(n++)+i3->ncol] = j1;
          }
        }
      }      
      *resVC = (VALUE) i3;
      return(ImageType);
    }

  }
    
    
  /***********************************************************
   * 
   *  A terminal expression
   * 
   * 
   *
   ***********************************************************/
   
   r = TermExpr(begin,left,resFlt,resVC,flagType,defParam);
   if (r==NO) return(0);
   
   return(r);
}




/*
 * Generic function that Sets the output of a binary operator on floats, images, ranges and signals
 *
 * This function is called below by the TTExpr<n> functions which deal with binary
 * operators, i.e., the functions TTExpr<n> functions with 2<=n<=7
 *
 * If both arguments of the operator are floats then this function is not called.
 * If one of the argument is neither a range, a signal, an image or a float it is not called either.
 *
 * The arguments are : 
 *
 *    - r1, vc1 : correspond to the first argument of the binary operator (r1 is the type
 *                and vc1 the corresponding variable content). If it is a float then
 *                vc1 = NULL
 *    - r2, vc2 : correspond to the second argument of the binary operator (r2 is the type
 *                and vc2 the corresponding variable content). If it is a float then
 *                vc2 = NULL
 *    - flag    : one of '+', '*' or '/' to manage more efficiently vectors 
 *                (a range + a range with the same step is a range .....)
 *
 * The function returns 1 if succeded and 0 otherwise.
 * (allocation wil be performed only if necessary).
 *      
 * 
 */
static unsigned char TTSetOutput2(unsigned char r1, VALUE vc1,  unsigned char r2, VALUE vc2, VALUE *vc3, char flag)
 {
   char flagRange;
   char flagTemp1;
   char flagTemp2;
   SIGNAL s1,s2,s3;
   IMAGE i1,i2,i3;
   RANGE rg1,rg2,rg3;
   unsigned char r3;

   /* Some checkings */
   if (r1!=SignalType && r1 != RangeType && r1 != ImageType && r1 != FloatType ||
       r2!=SignalType && r2 != RangeType && r2 != ImageType && r2 != FloatType) 
       Errorf("TTSetOutput2() : Weird 1");
   if (r1 == FloatType && r2 == FloatType) {
     Errorf("TTSetOutput2() : Weird 2");
   }   
   
   /* Bad combination error */
   if (((r1==SignalType || r1 == RangeType) && r2==ImageType) || ((r2==SignalType|| r2 == RangeType) && r1==ImageType)) {
     SetErrorf("Bad combination image - signal or range");
     return(0);
   }

   /* 
    * Just set the variables 
    */
   *vc3 = NULL;
   s1 = s2 = s3 = NULL;
   i1 = i2 = i3 = NULL;
   rg1 = rg2 = rg3 = NULL;
   if (r1 == SignalType) s1 = (SIGNAL) vc1;
   else if (r1 == RangeType) rg1 = (RANGE) vc1;
   else if (r1 == ImageType) i1 = (IMAGE) vc1;
   if (r2 == SignalType) s2 = (SIGNAL) vc2;
   else if (r2 == RangeType) rg2 = (RANGE) vc2;
   else if (r2 == ImageType) i2 = (IMAGE) vc2;
   if (vc1 && vc1->nRef == 1) flagTemp1 = YES;
   else flagTemp1 = NO;
   if (vc2 && vc2->nRef == 1) flagTemp2 = YES;
   else flagTemp2 = NO;
   
   
   /*
    * Case one argument is a signal or a range
    */
   if (r1==SignalType || r2 == SignalType || r1 == RangeType || r2 == RangeType) {
   
     /* If both are signals or ranges we must check their sizes */
     if (r1==SignalType && r2 == SignalType && s1->size != s2->size) {
       SetErrorf("Signals have different size");
       return(0);
     }
     if (r1==SignalType && r2 == RangeType && s1->size != rg2->size) {
       SetErrorf("Signal and range have different size");
       return(0);
     }
     if (r2==SignalType && r1 == RangeType && s2->size != rg1->size) {
       SetErrorf("Range and signal have different size");
       return(0);
     }
     if (r2==RangeType && r1 == RangeType && rg2->size != rg1->size) {
       SetErrorf("Ranges have different size");
       return(0);
     }
 
     /* Is the result a range ? */
     if (flag=='+' && ((r1 == RangeType && r2 == FloatType) || (r2 == RangeType && r1 == FloatType) || 
         (r1 == RangeType && r2 == RangeType))) flagRange = YES; 
     else if (flag=='*' && ((r1 == RangeType && r2 == FloatType) || (r2 == RangeType && r1 == FloatType))) flagRange = YES;
     else if (flag=='/' && (r1 == RangeType && r2 == FloatType)) flagRange = YES;
     else flagRange = NO;
       
     /* Set rg3 if necessary */
     if (flagRange) {
       if (flagTemp1 && r1 == RangeType) rg3 = rg1;
       else if (flagTemp2 && r2 == RangeType) rg3 = rg2;
       else rg3 = TNewRange();
       *vc3 = (VALUE) rg3;
       r3 = RangeType;
       if (rg1) rg3->size = rg1->size;
       else if (rg2) rg3->size = rg2->size;
     }
     
     /* Set s3 if necessary */
     else {
       if (r1 == SignalType && flagTemp1) s3 = s1;
       else if (r2 == SignalType && flagTemp2) s3 = s2;
       else {
         s3 = TNewSignal();
         if (r1==SignalType) {
           SizeSignal(s3,s1->size,s1->type);
           CopyFieldsSig(s1,s3);
           if (s1->type == XYSIG) memcpy(s3->X,s1->X,s1->size*sizeof(float));
         }
         else if (r2==SignalType) {
           SizeSignal(s3,s2->size,s2->type);
           CopyFieldsSig(s2,s3);
           if (s2->type == XYSIG) memcpy(s3->X,s2->X,s2->size*sizeof(float));
         }
         else {
           if (r1 == RangeType) {
             SizeSignal(s3,rg1->size,YSIG);
             s3->dx = 1;
           }
           else if (r2 == RangeType) {
             SizeSignal(s3,rg2->size,YSIG);
             s3->dx = 1;
           }
           else Errorf("TTSetOutput2() : Weird 3");
         }
       }
       r3 = SignalType;
       *vc3 = (VALUE) s3;
     }
     
     return(r3);
   }

   /*
    * Case of images 
    */
   if (r1==ImageType || r2 == ImageType) {


     /* If both are images we must check their sizes */
     if (r1==ImageType && r2 == ImageType && (i1->nrow != i2->nrow || i1->ncol != i2->ncol)) {
       SetErrorf("Images have different size");
       return(0);
     }
     
     /* Set i3 */
     if (r1 == ImageType && flagTemp1) i3 = i1;
     else if (r2 == ImageType && flagTemp2) i3 = i2;
     else {
      i3 = TNewImage();
       if (r1==ImageType) {
         SizeImage(i3,i1->ncol,i1->nrow);
         CopyFieldsImage(i1,i3);
       }
       else {
         SizeImage(i3,i2->ncol,i2->nrow);
         CopyFieldsImage(i2,i3);
       }
     }
     
     *vc3 = (VALUE) i3;
     r3 = ImageType;
     return(r3);
   }
   
   Errorf("SetOutput2() : Weired 4");
   
   return(0);
 }
 


/*
 * expr8 -> expr9 op expr9 .... op expr9
 *
 * where op is one of ^ 
 */

static unsigned char TTExpr8(char* begin, char** left,float *resFlt, VALUE *resVC, unsigned char flagType,ExprDefParam *defParam) 
{
  static char *opArray[] = {"^^","^","*^", NULL};
  static int opSizeArray[] = {2,1,2,0};

  char **op;                                                           
  int *opSize;                                                         
  char opIndex;                                                        
     
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;                                               
  RANGE rg1,rg2,rg3;                                                     
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3,f;                         
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j1,j2,j3,j;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI;
  
  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */  
  flagType1 = flagType; 
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 = flagType1 | FloatType | RangeType; 
  r = r1 = TTExpr9(begin, left, resFlt, resVC,flagType1,defParam);
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                            
                                                            
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {               
    ParamSkipSpace(begin);             
    begin2 = begin; 
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
     
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==RangeType) SetErrorf("Do not expect a range"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    opIndex = op-opArray+1; 
 
    /* Check the type */ 
    if (opIndex != 1) { 
      if (r == ListvType || r == StringType || r == NullType || r == OtherType) { 
        if (r==StringType) SetErrorf("Do not expect a string"); 
        else if (r==ListvType) SetErrorf("Do not expect a listv"); 
        else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
        else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
        Clean(*resVC); 
        *left = begin2; 
        return(0); 
      }
    }
    else {
      if (r != ImageType) {
        SetErrorf("Expect an image before the '^^' operator");
        Clean(*resVC);
        *left = begin2;
        return(0);
      } 
      if (((IMAGE) vc1)->nrow != ((IMAGE) vc1)->ncol) {
        SetErrorf("Expect a square image before the '^^' operator");
        Clean(*resVC);
        *left = begin2;
        return(0);
      } 
    }
    
    /* Skip operator */ 
    begin1 = begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
     
    /* Read the second argument */ 
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    flagType2 = FloatType;
    if (r1 == ImageType) flagType2 = flagType2 | ImageType; 
    if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
    if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
    if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
    if (opIndex == 1) flagType2 = FloatType;
    r2 = TTExpr9(begin,left,&f2,&vc2,flagType2,defParam);
    if (r2==0)  { 
      Clean(*resVC); 
      return(0); 
    } 
    begin = *left; 
    defParam->flagEmptySI = oldFlagEmptySI;
    if (opIndex == 1) {
      if (r2 != FloatType && (int) f2 != f2) {
        SetErrorf("Expect an integer after the '^^' operator");
        Clean(*resVC);
        *left = begin2;
        return(0);
      } 
    }
 
    /*  
     * Set the inputs 
     */ 
    if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    }


    if (opIndex == 3) {
      if (array2 != NULL) {
        for (j2=0;j2<size2;j2++) {
          if ((int) array2[j2] != array2[j2] || array2[j2]< 0) {
            SetErrorf("Expect only positive integers after the '*^' operator");            
            Clean(*resVC);
            *left = begin2;
            return(0);
          }
        }
      } 
      else {
        if (rg2->first != (int) rg2->first || rg2->first < 0 || RangeLast(rg2) < 0 || (rg2->step != (int) rg2->step && rg2->size > 1)) {
          SetErrorf("Expect only integers after the '*^' operator");            
          Clean(*resVC);
          *left = begin2;
          return(0);
        }
      }
    }


    /*  
     * Set the output 
     */ 
    if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
      r3 = FloatType;
    } 
    else if (opIndex == 1) {
      if (i1->nRef == 1) i3 = i1;
      else {
        i3 = TNewImage();
        SizeImage(i3,i1->ncol,i1->nrow);
      }
      vc3 = (VALUE) i3;
      size3 = i1->ncol*i1->nrow;
      r3 = ImageType;
    }
    else { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,' '))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size;
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    }      
    
    
  switch(opIndex) {
     
    /* ^^ */ 
    case 1 : 
    PowerImage(i1,i3,(int) f2);
    break;

    /* ^ */ 
    case 2 :     
    if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = pow(fabs(RangeVal(rg1,j1)),array2[j2]);
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = pow(fabs(array1[j1]),RangeVal(rg2,j2));
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = pow(fabs(RangeVal(rg1,j1)),RangeVal(rg2,j2));
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = pow(fabs(array1[j1]),array2[j2]);
      }
    }
      break;

    /* ^. */ 
    case 3 :     
    if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2] == 0) array3[j3] = 1;
        else {
          f = array3[j3] = RangeVal(rg1,j1);
          for (j = 0; j<array2[j2]-1;j++) array3[j3] *= f;
        }
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2) == 0) array3[j3] = 1;
        else {
          f = array3[j3] = array1[j1];
          for (j = 0; j<RangeVal(rg2,j2)-1;j++) array3[j3] *= f;
        }
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2) == 0) array3[j3] = 1;
        else {
          f = array3[j3] = RangeVal(rg1,j1);
          for (j = 0; j<RangeVal(rg2,j2)-1;j++) array3[j3] *= f;
        }
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2] == 0) array3[j3] = 1;
        else {
          f = array3[j3] = array1[j1];
          for (j = 0; j<array2[j2]-1;j++) array3[j3] *= f;
        }
      }
    }
      break;

  }
 
   
    /* 
     * Set the new input 
     */ 
    if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      f1 = *resFlt = f3; 
    }
    r = r1 = r3; 
  } 
  begin = *left; 
  return(r);
  }



/*
 * expr7 -> expr8 op expr8 .... op expr8
 *
 * where op is one of *, / , %, // 
 */


static unsigned char TTExpr7(char* begin, char** left,float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam)
{
  static char *opArray[] = {"//", "/", "%", "**", "*", NULL};
  static int opSizeArray[] = {2,1,1,2,1,0};
  char flag; 

  char **op;                                                           
  int *opSize;                                                         
  char opIndex;        
  
  STRVALUE sc1,sc3;         
  LISTV lv1,lv3;         
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;                                               
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3; 
  RANGE rg1,rg2,rg3;                        
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j,j1,j2,j3,j4,n;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI; 
  
  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */    
  flagType1 = flagType;
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 |= FloatType | RangeType | SignalType | ImageType; 
  r = r1 = TTExpr8(begin, left, resFlt, resVC,flagType1,defParam);
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                            
                                                            
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {               
    ParamSkipSpace(begin);             
    begin2 = begin;
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
     
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    opIndex = op-opArray+1; 
 
    /* Some type checking and prepare the second argument */
    if (opIndex != 5) {
      if (r1 != SignalType && r1 != ImageType && r1 != FloatType && r1 != RangeType) {
        SetErrorf("Only expect numbers, signals, ranges or images with this operator");
        Clean(*resVC);
        return(0);
      }
    }
    else {
      if (r1 != ListvType && r1 != StringType && r1 != SignalType && r1 != ImageType && r1 != FloatType && r1 != RangeType) {
        SetErrorf("Only expect numbers, signals, ranges, images, strings or listv with this operator");
        Clean(*resVC);
        return(0);
      }
    }
    flagType2 = FloatType;
    if (opIndex != 4) {
      if (r1 == ImageType) flagType2 = flagType2 | ImageType ; 
      if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
      if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
      if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
    }
    else flagType2 = FloatType | SignalType | RangeType | ImageType;
    
    /* Skip operator */ 
    begin1 = begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
     
    /* Read the second argument */ 
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    r2 = TTExpr8(begin,left,&f2,&vc2,flagType2,defParam);
    defParam->flagEmptySI = oldFlagEmptySI;
    if (r2==0)  { 
      Clean(*resVC); 
      return(0); 
    } 
    begin = *left; 
    if ((r1 == StringType || r1 == ListvType) && ((int) f2 != f2 || f2 < 0)) {
      SetErrorf("Expect a positive integer");
      Clean(*resVC);
      return(0);
    }

    /*  
     * Set the inputs 
     */ 
    if (r1 == StringType) sc1 = (STRVALUE) vc1;
    else if (r1 == ListvType) lv1 = (LISTV) vc1;
    else if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    };

  if (opIndex == 2) flag = '/';
  else if (opIndex == 5) flag = '*';
  else flag = ' ';

    /*  
     * Set the output 
     */ 
    if (r1 == StringType) { 
      if (sc1->nRef == 1) sc3 = sc1;
      else sc3 = TNewStrValue();
      vc3 = (VALUE) sc3;
      r3 = StringType;
    } 
    else if (r1 == ListvType) { 
      if (lv1->nRef == 1) lv3 = lv1;
      else lv3 = TNewListv();
      vc3 = (VALUE) lv3;
      r3 = ListvType;
    } 
    else if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
    } 
    /* Case of '**' */
    else if (opIndex == 4) {
      if (r1 == FloatType || r2 == FloatType) {
        if (size1 != size2) {
          *left = begin1;
          SetErrorf("Inconsistent size for '**' operator");
          Clean(*resVC);
          return(0);
        }
        array3 = &f3;
        size3 = 1;
        r3 = FloatType;
      }
      else if ((r1 == SignalType || r1 == RangeType) && (r2 == SignalType || r2 == RangeType)) {
        if (size1 != size2 || size1 != 1) {
          *left = begin1;
          SetErrorf("Inconsistent size for '**' operator");
          Clean(*resVC);
          return(0);
        }
        array3 = &f3;
        size3 = 1;
        r3 = FloatType;
      }
      else if ((r1 == SignalType || r1 == RangeType) && r2 == ImageType) {
        if (size1 != i2->nrow) {
          *left = begin1;
          SetErrorf("Inconsistent size for '**' operator");
          Clean(*resVC);
          return(0);
        }
        if (i2->ncol == 1) {
          size3 = 1;
          array3 = &f3;
          r3 = FloatType;
        }
        else {
          s3 = TNewSignal();
          size3 = i2->ncol;
          SizeSignal(s3,size3,YSIG);
          array3=s3->Y;
          vc3 = (VALUE) s3;
          r3 = SignalType;
        }
      }
      else if ((r2 == SignalType || r2 == RangeType) && r1 == ImageType) {
        if (i1->ncol != 1) {
          *left = begin1;
          SetErrorf("Inconsistent size for '**' operator");
          Clean(*resVC);
          return(0);
        }
        i3 = TNewImage();
        SizeImage(i3,size2,i1->nrow);
        size3 = i3->ncol*i3->nrow;
        array3=i3->pixels;
        vc3 = (VALUE) i3;
        r3 = ImageType;
      }
      else if (r2 == ImageType && r1 == ImageType) {
        if (i1->ncol != i2->nrow) {
          *left = begin1;
          SetErrorf("Inconsistent size for '**' operator");
          Clean(*resVC);
          return(0);
        }
        i3 = TNewImage();
        SizeImage(i3,i2->ncol,i1->nrow);
        size3 = i3->ncol*i3->nrow;
        array3=i3->pixels;
        vc3 = (VALUE) i3;
        r3 = ImageType;
      }
      else Errorf("NONO");          
    }
    else { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,flag))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size;
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    };
    
    
  switch(opIndex) {
     
    /* // */ 
    case 1 :     
    if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2]==0) {
          Warningf("Division (//) by 0");
          array3[j3] = INT_MAX;
        }
        else array3[j3] = ((int) RangeVal(rg1,j1))/((int)array2[j2]);
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2)==0) {
          Warningf("Division (//) by 0");
          array3[j3] = INT_MAX;
        }
        else array3[j3] = ((int) array1[j1]) / ((int) RangeVal(rg2,j2));
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2)==0) {
          Warningf("Division (//) by 0");
          array3[j3] = INT_MAX;
        }
        else array3[j3] = ((int) RangeVal(rg1,j1))/((int) RangeVal(rg2,j2));
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2]==0) {
          Warningf("Division (//) by 0");
          array3[j3] = INT_MAX;
        }
        else array3[j3] = ((int) array1[j1]) / ((int) array2[j2]);
      }
    }
      break;


    /* / */ 
    case 2 :     
    if (array1 == NULL && array2 != NULL) {
      if (size2 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          if (array2[j2]==0) {
            Warningf("Division by 0");
            array3[j3] = FLT_MAX;
          }
          else array3[j3] = RangeVal(rg1,j1)/array2[j2];
        }
      }
      else  {
        if (*array2 == 0) {
          Warningf("Division by 0");
          rg3->first = FLT_MAX; rg3->size = size3; rg3->step = FLT_MAX;
        }
        else  {rg3->first = rg1->first/(*array2); rg3->size = size3; rg3->step = rg1->step/(*array2);}
      } 
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2)==0) {
          Warningf("Division by 0");
          array3[j3] = FLT_MAX;
        }
        else array3[j3] = array1[j1]/RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2)==0) {
          Warningf("Division by 0");
          array3[j3] = FLT_MAX;
        }
        else array3[j3] = RangeVal(rg1,j1)/ RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2]==0) {
          Warningf("Division by 0");
          array3[j3] = FLT_MAX;
        }
        else array3[j3] = array1[j1]/ array2[j2];
      }
    }
      break;

    /* % */ 
    case 3 :     
    if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2]==0) {
          Warningf("Division (%) by 0");
          array3[j3] = INT_MAX;
        }
        else array3[j3] = ((int) RangeVal(rg1,j1))%((int)array2[j2]);
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2)==0) {
          Warningf("Division (%) by 0");
          array3[j3] = INT_MAX;
        }
        else  array3[j3] = ((int) array1[j1]) % ((int) RangeVal(rg2,j2));
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (RangeVal(rg2,j2)==0) {
          Warningf("Division (%) by 0");
          array3[j3] = INT_MAX;
        }
        else  array3[j3] = ((int) RangeVal(rg1,j1))%((int) RangeVal(rg2,j2));
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        if (array2[j2]==0) {
          Warningf("Division (%) by 0");
          array3[j3] = INT_MAX;
        }
        else array3[j3] = ((int) array1[j1]) % ((int) array2[j2]);
      }
    }
      break;

    /* * */ 
    case 5 :     
    if (r1 == StringType) MultStrValue(sc1,(int) f2,sc3);
    else if (r3 == ListvType) MultListv(lv1,(int) f2,lv3);
    else if (array1 == NULL && array2 != NULL) {
      if (size2 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          array3[j3] = RangeVal(rg1,j1)*array2[j2];
        }
      }
      else  {rg3->first = rg1->first*(*array2); rg3->size = size3; rg3->step = rg1->step*(*array2);} 
    }
    else if (array1 != NULL && array2 == NULL) {
      if (size1 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          array3[j3] = array1[j1]*RangeVal(rg2,j2);
        }
      }
      else  {rg3->first = rg2->first*(*array1); rg3->size = size3; rg3->step = rg2->step*(*array1);} 
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)* RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]* array2[j2];
      }
    }
      break;

    /* ** */ 
    case 4 :     
      if (size1 == 1 || size2 == 1) {
        if (array1 != NULL && array2 != NULL) *array3 = *array2**array1;
        else if (array1 == NULL && array2 != NULL) *array3 = *array2* RangeFirst(rg1);
        else if (array1 != NULL && array2 == NULL) *array3 = *array1* RangeFirst(rg2);
        else *array3 = RangeFirst(rg1) * RangeFirst(rg2);
      }
      else if (r1 == SignalType) {
        for (j1=0;j1<size3;j1++) {
          array3[j1] = 0;
          for(j=0;j<size1;j++) array3[j1] += array1[j]*array2[j*i2->ncol+j1];
        }
      }
      else if (r1 == RangeType) {
        for (j1=0;j1<size3;j1++) {
          array3[j1] = 0;
          for(j=0;j<size1;j++) array3[j1] += RangeVal(rg1,j)*array2[j*i2->ncol+j1];
        }
      }
      else if (r2 == SignalType) {
        for(j=0,j1=0;j1<i1->nrow;j1++) {
          for(j2=0;j2<size2;j2++) {
            array3[j++] = array1[j1]*array2[j2];
          }
        }
      }
      else if (r2 == RangeType) {
        for(j=0,j1=0;j1<i1->nrow;j1++) {
          for(j2=0;j2<size2;j2++) {
            array3[j++] = array1[j1]*RangeVal(rg2,j2);
          }
        }
      }
      else {
        for (j=0,j3 = 0; j3< i3->nrow; j3++) {
          for (j4 = 0; j4< i3->ncol; j4++) {
            array3[j] = 0;
            for (n=0;n<i1->ncol;n++) array3[j]+=array1[j3*i1->ncol+n]*array2[n*i2->ncol+j4];
            j++;
          }
        }
      }
      
      break;
    }
  

    /* 
     * Set the new input 
     */ 
    if (r3 == StringType) {
      vc1 = vc3;
      vc2 = NULL;
      *resVC = vc3;
      r1 = StringType;
    }
    else if (r3 == ListvType) {
      vc1 = vc3;
      vc2 = NULL;
      *resVC = vc3;
      r1 = ListvType;
    }
    else if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      f1 = *resFlt = f3; 
    }
    r = r1 = r3; 
  }
  begin = *left; 
  return(r);
    
}


/*
 * expr6 -> expr7 op expr7 .... op expr7
 *
 * where op is one of +, -
 */

static unsigned char TTExpr6(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam)
{
  static char *opArray[] = {"+", "-", NULL};
  static int opSizeArray[] = {1,1,0};
  char flag; 

  char **op;                                                           
  int *opSize;                                                         
  char opIndex;                                                        
  
  STRVALUE sc1,sc2,sc3;         
  LISTV lv1,lv2,lv3;         
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;                                               
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3; 
  RANGE rg1,rg2,rg3;                        
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j1,j2,j3;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI;
  
  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */        
  flagType1 = flagType; 
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 = flagType1 | FloatType | RangeType; 
  r = r1 = TTExpr7(begin, left, resFlt, resVC,flagType1,defParam);
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                            
                                                            
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {               
    ParamSkipSpace(begin);             
    begin2 = begin;
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
     
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    opIndex = op-opArray+1; 
 
    /* Some type checking and prepare the second argument */
    if (opIndex != 1) {
      if (r1 != SignalType && r1 != ImageType && r1 != FloatType && r1 != RangeType) {
        SetErrorf("Only expect numbers, signals, ranges or images with this operator");
        Clean(*resVC);
        return(0);
      }
    }
    else {
      if (r1 != ListvType && r1 != StringType && r1 != SignalType && r1 != ImageType && r1 != FloatType && r1 != RangeType) {
        SetErrorf("Only expect numbers, signals, ranges, images, strings or listv with this operator");
        Clean(*resVC);
        return(0);
      }
    }
    flagType2 = FloatType;
    if (r1 == ListvType) flagType2 = AnyType;
    else if (r1 == StringType) flagType2 = StringType;
    else {
      if (r1 == ImageType) flagType2 = flagType2 | ImageType; 
      if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
      if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
      if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
    }
    
    /* Skip operator */ 
    begin1 = begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
     
    /* Read the second argument */ 
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    r2 = TTExpr7(begin,left,&f2,&vc2,flagType2,defParam);
    defParam->flagEmptySI = oldFlagEmptySI;
    if (r2==0)  { 
      Clean(*resVC); 
      return(0); 
    } 
    begin = *left; 
 
    /*  
     * Set the inputs 
     */ 
    if (r1 == StringType) { 
      sc1 = (STRVALUE) vc1;
      sc2 = (STRVALUE) vc2;
    } 
    else if (r1 == ListvType) { 
      lv1 = (LISTV) vc1;
      if (r2 == ListvType) lv2 = (LISTV) vc2;
      else lv2 = NULL;
    } 
    else if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    };

  flag = '+';

    /*  
     * Set the output 
     */ 
    if (r1 == StringType) { 
      if (sc1->nRef == 1) sc3 = sc1;
      else if (sc2->nRef == 1) sc3 = sc2;
      else sc3 = TNewStrValue();
      vc3 = (VALUE) sc3;
      r3 = StringType;
    } 
    else if (r1 == ListvType) { 
      if (lv1->nRef == 1) lv3 = lv1;
      else if (lv2 != NULL && lv2->nRef == 1) lv3 = lv2;
      else lv3 = TNewListv();
      vc3 = (VALUE) lv3;
      r3 = ListvType;
    } 
    else if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
    } 
    else { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,flag))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size; 
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    }        

  switch(opIndex) {
     
    /* + */ 
    case 1 :     
    if (r3 == StringType) ConcatStrValue(sc1,sc2,sc3);
    else if (r3 == ListvType) {
      if (lv2 == NULL) {
        if (lv3 != lv1) CopyListv(lv1,lv3);
        if (vc2 == NULL) AppendFloat2Listv(lv3,f2);
        else AppendValue2Listv(lv3,vc2);
      }
      else ConcatListv(lv1,lv2,lv3);
    }
    else if (array1 == NULL && array2 != NULL) {
      if (size2 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          array3[j3] = RangeVal(rg1,j1)+array2[j2];
        }
      }
      else {rg3->first = rg1->first+(*array2); rg3->size = size3; rg3->step = rg1->step;}
    }
    else if (array1 != NULL && array2 == NULL) {
      if (size1 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          array3[j3] = RangeVal(rg2,j2)+array1[j1];
        }
      }
      else  {rg3->first = rg2->first+(*array1); rg3->size = size3; rg3->step = rg2->step;} 
    }
    else if (array1 == NULL && array2 == NULL) {
      if (rg1->step+rg2->step != 0) {
        rg3->first = rg2->first + rg1->first; rg3->size = size3; rg3->step = rg1->step+rg2->step;
      }
      else {
        s3 = TNewSignal();
        SizeSignal(s3,rg1->size,YSIG);
        r3 = SignalType;
        vc3 = (VALUE) s3;
        array3 = s3->Y;
        for (j1=0;j1<rg1->size;j1++) s3->Y[j1] = rg1->first+rg2->first;
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]+ array2[j2];
      }
    }
      break;

    /* - */ 
    case 2 :     
    if (array1 == NULL && array2 != NULL) {
      if (size2 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          array3[j3] = RangeVal(rg1,j1)-array2[j2];
        }
      }
      else {rg3->first = rg1->first-(*array2); rg3->size = size3; rg3->step = rg1->step;}
    }
    else if (array1 != NULL && array2 == NULL) {
      if (size1 > 1) {
        for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
          j1 %= size1; j2 %= size2;
          array3[j3] = array1[j1]-RangeVal(rg2,j2);
        }
      }
      else {rg3->first = -rg2->first+(*array1); rg3->size = size3; rg3->step = -rg2->step;}
    }
    else if (array1 == NULL && array2 == NULL) {
      if (rg1->step-rg2->step != 0) {
        rg3->first = -rg2->first + rg1->first; rg3->size = size3; rg3->step = rg1->step-rg2->step;
      }
      else {
        s3 = TNewSignal();
        SizeSignal(s3,rg1->size,YSIG);
        r3 = SignalType;
        vc3 = (VALUE) s3;
        array3 = s3->Y;
        for (j1=0;j1<rg1->size;j1++) s3->Y[j1] = rg1->first-rg2->first;
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]- array2[j2];
      }
    }
      break;
  }
  
   /* 
     * Set the new input 
     */ 
    if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      f1 = *resFlt = f3; 
    }
    r = r1 = r3; 
  } 
  begin = *left; 
  return(r);
}

/*
 * expr5 -> expr6 op expr6
 *
 * where op is one of <, >,<=,>=
 */


static unsigned char TTExpr5(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam)
{
  static char *opArray[] = {"<=", ">=", "<", ">", NULL};
  static int opSizeArray[] = {2,2,1,1,0};
  char flagString = YES;
  char flag=' ';
  char flagSingleLoop = YES;

  char **op;                                                           
  int *opSize;                                                         
  char opIndex;                                                        
  
  char *str1,*str2;         
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;           
  RANGE rg1,rg2,rg3;                                                                         
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3;                         
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j1,j2,j3;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI; 
  
  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */       
  flagType1 = flagType; 
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 = flagType1 | FloatType | RangeType; 
  if (flagString)   flagType1 = flagType1 | StringType; 
  r = r1 = TTExpr6(begin, left, resFlt, resVC,flagType1,defParam);
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                            
                                                            
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {               
    ParamSkipSpace(begin);             
    begin2 = begin; 
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
    /* Prevent to read a > if and of a listv */
    if (op != NULL && op-opArray==3 && begin[1] == ' ' && defParam->flagSpace == NO) op = NULL;
    
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    opIndex = op-opArray+1; 
 
    /* Skip operator */ 
    begin1 = begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);
     
    /* Read the second argument */ 
    if (r1 == NullType) {
      SetErrorf("non valid 'null' value");
      Clean(*resVC);
      *left = begin2;
      return(0);
    }
    if (r1 == StringType) flagType2 = StringType; 
    else if (r1 == OtherType) flagType2 = OtherType; 
    else if (r1 == ListvType) flagType2 = ListvType; 
    else {
      flagType2 = FloatType;
      if (r1 == ImageType) flagType2 = flagType2 | ImageType; 
      if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
      if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
      if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
    }
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    r2 = TTExpr6(begin,left,&f2,&vc2,flagType2,defParam);
    defParam->flagEmptySI = oldFlagEmptySI;
    if (r2==0)  { 
      if ((*op)[0] == '>' && (*op)[1] == '\0') { 
        *left = begin1;  
        if (r1 & FloatType) Clean(*resVC); 
        return(r1); 
      } 
      Clean(*resVC); 
      return(0); 
    } 
    begin = *left; 
 
    /*  
     * Set the inputs 
     */ 
    if (r1 == StringType && r2 != StringType || r2 == StringType && r1 != StringType ) { 
      SetErrorf("Both arguments should be of the same type"); 
      *left = begin1; 
      Clean(vc1); 
      Clean(vc2); 
      *resVC = NULL; 
      return(0); 
    } 
    if (r1 == OtherType && r2 != OtherType || r2 == OtherType && r1 != OtherType ) { 
      SetErrorf("Both arguments should be of the same type"); 
      *left = begin1; 
      Clean(vc1); 
      Clean(vc2); 
      *resVC = NULL; 
      return(0); 
    } 
    if (r1 == ListvType && r2 != ListvType || r2 == ListvType && r1 != ListvType ) { 
      SetErrorf("Both arguments should be of the same type"); 
      *left = begin1; 
      Clean(vc1); 
      Clean(vc2); 
      *resVC = NULL; 
      return(0); 
    } 
    if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    }

    /*  
     * Set the output 
     */ 
    if (r1 == StringType) { 
      str1 = GetStrFromStrValue((STRVALUE) vc1); 
      str2 = GetStrFromStrValue((STRVALUE) vc2); 
      vc3 = NULL; 
      r3 = FloatType; 
    } 
    else if (r1 == OtherType || r1 == ListvType) { 
      vc3 = NULL; 
      r3 = FloatType; 
    } 
    else if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
    } 
    else if (r1 != OtherType && r1 != ListvType) { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,flag))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size;
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    }    
    
  switch(opIndex) {
     
    /* <= */ 
    case 1 :     
    if (r1 == StringType) f3 = (strcmp(str1,str2) <= 0);
    else if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)<=array2[j2];
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]<=RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)<= RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]<= array2[j2];
      }
    }
      break;
      
    /* >= */ 
    case 2 :     
    if (r1 == StringType) f3 = (strcmp(str1,str2) >= 0);
    else if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)>=array2[j2];
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]>=RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)>= RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]>= array2[j2];
      }
    }
      break;

    /* < */ 
    case 3 :     
    if (r1 == StringType) f3 =  (strcmp(str1,str2) < 0);
    else if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)<array2[j2];
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]<RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)< RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]< array2[j2];
      }
    }
      break;
      
    /* > */ 
    case 4 :     
    if (r1 == StringType) f3 =  (strcmp(str1,str2) > 0);
    else if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)>array2[j2];
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]>RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)> RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]>array2[j2];
      }
    }
      break;

  }
  
   
    /* 
     * Set the new input 
     */ 
    if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      *resFlt = f3; 
    }
    r = r1 = r3; 
    if (flagSingleLoop) break; 
  } 
  begin = *left; 
  return(r);
}

/*
 * expr4 -> expr5 op expr5
 *
 * where op is one of ==, !=, is, isnot 
 */


static unsigned char TTExpr4(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam)
{
  static char *opArray[] = {"==", "!=", "isnot", "is",  NULL};
  static int opSizeArray[] = {2,2,5,2,0};
  static char flagString = YES;
  static char flagOther = YES;
  static char flag = ' '; 
  static char flagSingleLoop = YES;
  
  STRVALUE sc1,sc2;
  LISTV lv1,lv2;
  char flagSpace = YES;
  char **op;                                                           
  int *opSize;                                                         
  char opIndex;                                                        
  char *str1,*str2;         
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;        
  RANGE rg1,rg2,rg3;                                       
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3;                         
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j1,j2,j3;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI;
  
  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */        
  if (flagType & FloatType) flagType1 = AnyType; 
  else flagType1 = flagType;
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 = flagType1 | FloatType | RangeType; 
  oldFlagEmptySI = defParam->flagEmptySI;
  defParam->flagEmptySI = YES;
  r = r1 = TTExpr5(begin, left, resFlt, resVC,flagType1,defParam);
  defParam->flagEmptySI = oldFlagEmptySI;
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                            
                                                            
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {  
    ParamSkipSpace(begin);             
    begin2 = begin; 
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
     
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        if (r == SignalType && !defParam->flagEmptySI && ((SIGNAL) *resVC)->size == 0) {
          SetErrorf("Do not expect an empty signal");
          Clean(*resVC);
          return(0);
        } 
        else if (r == ImageType && !defParam->flagEmptySI && (((IMAGE) *resVC)->nrow == 0 || ((IMAGE) *resVC)->ncol == 0)) {
          SetErrorf("Do not expect an empty image");
          Clean(*resVC);
          return(0);
        } 
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2;
      return(0); 
    } 
    opIndex = op-opArray+1; 

    /* Skip operator */ 
    *left = begin;
    begin1 = begin; 
    begin  += *opSize; 
    ParamSkipSpace(begin);

    /* Check some types and prepare reading the second argument */
    flagType2 = r1;
    if (opIndex <= 2) { /* case == or != */
      oldFlagEmptySI = defParam->flagEmptySI;
      defParam->flagEmptySI = NO;
      if (r1 == OtherType) {
        SetErrorf("Bad operator for type '%s' (you should use 'is' or 'isnot')",GetTypeValue(vc1));
        Clean(*resVC); 
        return(0); 
      }
      if (r1 == StringType) flagType2 = StringType; 
      else if (r1 == ListvType) flagType2 = ListvType; 
      else {
        flagType2 = FloatType;
        if (r1 == ImageType) flagType2 = flagType2 | ImageType; 
        if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
        if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
        if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
      }
      flagType2 |= NullType;
    } 
    else {
      flagType2 = AnyType;
      oldFlagEmptySI = defParam->flagEmptySI;
      defParam->flagEmptySI = YES;
    }


    /* Get the second argument */
    r2 = TTExpr5(begin,left,&f2,&vc2,flagType2,defParam);
    defParam->flagEmptySI = oldFlagEmptySI;
    if (r2==0)  { 
      Clean(*resVC); 
      return(0); 
    } 
    begin = *left; 
    
    /* Treat the null case separately */
    if (r1 == NullType || r2 == NullType) {
      if (opIndex == 1 || opIndex == 4) *resFlt = (r1==r2);
      else *resFlt = (r1!=r2);
      Clean(*resVC);
      if (flagType & FloatType) return(FloatType);
      else {
        SetErrorf("Do not expect a number");
        return(0);
      }
    }
 
    /*  
     * Set the inputs for floats, signals and images
     */ 
    if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    }
    
    /*  
     * Set the output 
     */ 
    if (r1 == StringType) { 
      sc1 = (STRVALUE) vc1;
      str1 = GetStrFromStrValue(sc1);
      sc2 = (STRVALUE) vc2;
      str2 = GetStrFromStrValue(sc2);
      vc3 = NULL; 
      r3 = FloatType; 
    } 
    else if (r1 == ListvType) { 
      lv1 = (LISTV) vc1;
      lv2 = (LISTV) vc2;
      if (lv1->length != lv2->length) {
        SetErrorf("the 2 listv should have the same length");
        Clean(*resVC);
        return(0);
      }
      if (opIndex < 3) {
        s3 =  TNewSignal();
        vc3 = (VALUE) s3; 
        SizeSignal(s3,lv1->length,YSIG);
        r3 = SignalType; 
      }
      else {
        r3 = FloatType;
        vc3 = NULL;
      }
    } 
    else if (r1 == OtherType || r1 != FloatType && (opIndex == 3 || opIndex == 4)) { 
      vc3 = NULL; 
      r3 = FloatType; 
    } 
    else if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
    } 
    else if (r1 != StringType && r1 != OtherType && r1 != ListvType) { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,flag))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size;
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    }    

  switch(opIndex) {
     
    /* == */ 
    case 1 :     
     if (r1 == StringType) f3 =  !strcmp(str1,str2);
     else if (r1 == ListvType) {
       for (j1=0;j1<lv1->length;j1++) {
         if (lv1->values[j1] != NULL) {
           str1 = GetTypeValue(lv1->values[j1]);
           if (str1 == numType) lv1->f[j1] = ((NUMVALUE) ValueOf(lv1->values[j1]))->f;
         } 
         else str1 = numType;
         if (lv2->values[j1] != NULL) {
           str2 = GetTypeValue(lv2->values[j1]);
           if (str2 == numType) lv2->f[j1] = ((NUMVALUE) ValueOf(lv2->values[j1]))->f;
         }
         else str2 = numType;
         if (str1 != str2) {
           s3->Y[j1] = 0;
           continue;
         }
         if (str1 == numType) {
           if (lv1->f[j1] == lv2->f[j1]) s3->Y[j1] = 1;
           else s3->Y[j1] = 0;
           continue;
         }
         if (str1 == strType) s3->Y[j1] = !strcmp(((STRVALUE) ValueOf(lv1->values[j1]))->str,((STRVALUE) ValueOf(lv2->values[j1]))->str);
         else  s3->Y[j1] = ValueOf(lv1->values[j1]) == ValueOf(lv2->values[j1]);
       }
     }
     else if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)==array2[j2];
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]==RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)==RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]== array2[j2];
      }
    }
      break;

      
    /* != */ 
    case 2 :     
     if (r1 == StringType) f3 =  strcmp(str1,str2);
     else if (r1 == ListvType) {
       for (j1=0;j1<lv1->length;j1++) {
         if (lv1->values[j1] != NULL) {
           str1 = GetTypeValue(lv1->values[j1]);
           if (str1 == numType) lv1->f[j1] = ((NUMVALUE) ValueOf(lv1->values[j1]))->f;
         } 
         else str1 = numType;
         if (lv2->values[j1] != NULL) {
           str2 = GetTypeValue(lv2->values[j1]);
           if (str2 == numType) lv2->f[j1] = ((NUMVALUE) ValueOf(lv2->values[j1]))->f;
         }
         else str2 = numType;
         if (str1 != str2) {
           s3->Y[j1] = 1;
           continue;
         }
         if (str1 == numType) {
           if (lv1->f[j1] == lv2->f[j1]) s3->Y[j1] = 0;
           else s3->Y[j1] = 1;
           continue;
         }
         if (str1 == strType) s3->Y[j1] = (strcmp(((STRVALUE) ValueOf(lv1->values[j1]))->str,((STRVALUE) ValueOf(lv2->values[j1]))->str)!=0);
         else  s3->Y[j1] = ValueOf(lv1->values[j1]) != ValueOf(lv2->values[j1]);
       }
     }
     else if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)!=array2[j2];
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]!=RangeVal(rg2,j2);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = RangeVal(rg1,j1)!= RangeVal(rg2,j2);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = array1[j1]!= array2[j2];
      }
    }
      break;
      
    /* isnot */ 
    case 3 : f3 = (vc1 == NULL || vc1 != vc2); break;

    /* is */ 
    case 4 : f3 = (vc1 != NULL && vc1 == vc2); break;


  }
  
  /* 
   * Set the new input 
   */ 
    if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      *resFlt = f3; 
    }
    r = r1 = r3; 
    if (flagSingleLoop) break; 
  } 

  begin = *left; 
  return(r);
}
 

/*
 * expr3 -> expr4 op expr4 .... op expr4
 *
 * where op is &&
 */


static unsigned char TTExpr3(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam) 
{
  static char *opArray[] = {"&&", NULL};
  static int opSizeArray[] = {2,0};

  char **op;                                                           
  int *opSize;                                                         
  char opIndex;                                                        
  
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;                                               
  RANGE rg1,rg2,rg3;                                                     
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3;                         
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j1,j2,j3;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI;
  
  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */        
  flagType1 = flagType; 
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 = flagType1 | FloatType | RangeType; 
  r = r1 = TTExpr4(begin, left, resFlt, resVC,flagType1,defParam);
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                            
                                                            
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {               
    ParamSkipSpace(begin);             
    begin2 = begin; 
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
     
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==RangeType) SetErrorf("Do not expect a range"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    opIndex = op-opArray+1; 
 
    /* Check the type */ 
    if (r == ListvType || r == StringType || r == NullType || r == OtherType) { 
      if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    
    /* Skip operator */ 
    begin1 = begin; 
    begin  += *opSize; 
    
    ParamSkipSpace(begin);
     
    /* Read the second argument */ 
    flagType2 = FloatType;
    if (r1 == ImageType) flagType2 = flagType2 | ImageType; 
    if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
    if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
    if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    r2 = TTExpr4(begin,left,&f2,&vc2,flagType2,defParam);
    defParam->flagEmptySI =  oldFlagEmptySI;
    if (r2 == 0) return(0);
    begin = *left; 
 
    /*  
     * Set the inputs 
     */ 
    if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    }
    
   
    /*  
     * Set the output 
     */ 
    if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
    } 
    else { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,' '))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size;
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    }      

  switch(opIndex) {
     
    /* && */ 
    case 1 :     
    if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (RangeVal(rg1,j1)!=0)&&(array2[j2]!=0);
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (array1[j1]!=0)&&(RangeVal(rg2,j2)!=0);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (RangeVal(rg1,j1)!=0)&&(RangeVal(rg2,j2)!=0);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (array1[j1] != 0) && (array2[j2] != 0);
      }
    }
      break;
  }
  
    /* 
     * Set the new input 
     */ 
    if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      f1 = *resFlt = f3; 
    }
    r = r1 = r3; 
  } 
  begin = *left; 
  return(r); 
}
 


/*
 * expr2 -> expr3 op expr3 .... op expr3
 *
 * where op is ||
 */


static unsigned char TTExpr2(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam) 
{
  static char *opArray[] = {"||", NULL};
  static int opSizeArray[] = {2,0};

  char **op;                                                           
  int *opSize;                                                         
  char opIndex;                                                        
  
  VALUE vc1,vc2,vc3; 
  float *array1,*array2,*array3;                                       
  int size1,size2,size3;                                               
  RANGE rg1,rg2,rg3;                                                     
  SIGNAL s1,s2,s3;                                                     
  IMAGE i1,i2,i3;                         
  float f1,f2,f3;                         
  unsigned char flagType1,flagType2;    
  unsigned char r,r1,r2,r3;                                           
  int j1,j2,j3;                                
  char *begin1,*begin2; 
  char oldFlagEmptySI;

  if (*begin == '\0') {
    *left = begin;
    SetErrorf("Do not expect end of expression");
    Clean(*resVC);
    return(NO);                                      
  }
  
  /* Read the first argument */        
  flagType1 = flagType; 
  if (flagType & SignalType || flagType & ImageType || flagType & RangeType) flagType1 = flagType1 | FloatType | RangeType; 
  r = r1 = TTExpr3(begin, left, resFlt, resVC,flagType1,defParam);
  if (r1 == NO)  return(0);  
  vc1 = *resVC; f1 = *resFlt; 
  begin=*left;                                                                                                                        
                                                                
  /* 
   * Then we loop on the operators 
   */                                                            
                               
  vc2 = NULL;                     
                                                              
  while (1) {               
    ParamSkipSpace(begin);               
    begin2 = begin;
    if (*begin == '\0') op = NULL; 
    else { 
      /* Look for the matching operator */       
      for (op = opArray, opSize = opSizeArray ;;op++,opSize++) {     
        if (*op == NULL) break; 
        if (!strncmp(begin,*op,*opSize)) break; 
      } 
    } 
     
    /* If none then just return */ 
    if (op == NULL || *op == NULL) { 
      if (*resVC == vc1) {Clean(vc2);} 
      else if (*resVC == vc2) {Clean(vc1);} 
      if (r & flagType) {
        *left = begin2;
        return(r); 
      }
      if (r==SignalType) SetErrorf("Do not expect a signal"); 
      else if (r==ImageType) SetErrorf("Do not expect an image"); 
      else if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==FloatType) SetErrorf("Do not expect a number"); 
      else if (r==RangeType) SetErrorf("Do not expect a range"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    opIndex = op-opArray+1; 
 
    /* Check the type */ 
    if (r == ListvType || r == StringType || r == NullType || r == OtherType) { 
      if (r==StringType) SetErrorf("Do not expect a string"); 
      else if (r==ListvType) SetErrorf("Do not expect a listv"); 
      else if (r==NullType) SetErrorf("Do not expect a 'null' value"); 
      else SetErrorf("Do not expect a '%s' variable",GetTrueTypeValue(*resVC)); 
      Clean(*resVC); 
      *left = begin2; 
      return(0); 
    } 
    
    /* Skip operator */ 
    begin1 = begin; 
    begin  += *opSize; 
    
    ParamSkipSpace(begin);
     
    /* Read the second argument */ 
    flagType2 = FloatType;
    if (r1 == ImageType) flagType2 = flagType2 | ImageType; 
    if (r1 == SignalType || r1 == RangeType) flagType2 = flagType2 | SignalType | RangeType; 
    if (r1 == FloatType && flagType & SignalType) flagType2 = flagType2 | SignalType | RangeType; 
    if (r1 == FloatType && flagType & ImageType) flagType2  = flagType2 | ImageType; 
    oldFlagEmptySI = defParam->flagEmptySI;
    defParam->flagEmptySI = NO;
    r2 = TTExpr3(begin,left,&f2,&vc2,flagType2,defParam);
    defParam->flagEmptySI =  oldFlagEmptySI;
    if (r2==NO) return(0);
    begin = *left; 
 
    /*  
     * Set the inputs 
     */ 
    if (r1==SignalType) { 
      s1 = (SIGNAL) vc1; 
      array1 = s1->Y; 
      size1 = s1->size; 
    } 
    else if (r1==RangeType) { 
      rg1 = (RANGE) vc1; 
      array1 = NULL; 
      size1 = rg1->size; 
    } 
    else if (r1==ImageType) { 
      i1 = (IMAGE) vc1; 
      array1 = i1->pixels; 
      size1 = i1->nrow*i1->ncol; 
    }   
    else if (r1==FloatType) { 
      array1 = &f1; 
      size1 = 1; 
    } 
    if (r2==SignalType) { 
      s2 = (SIGNAL) vc2; 
      array2 = s2->Y; 
      size2 = s2->size; 
    } 
    else if (r2==RangeType) { 
      rg2 = (RANGE) vc2; 
      array2 = NULL;
      size2 = rg2->size; 
    } 
    else if (r2==ImageType) { 
      i2 = (IMAGE) vc2; 
      array2 = i2->pixels; 
      size2 = i2->nrow*i2->ncol; 
    }  
    else if (r2==FloatType) { 
      array2 = &f2; 
      size2 = 1; 
    }


    /*  
     * Set the output 
     */ 
    if (r1 == FloatType && r2 == FloatType) { 
      vc3 = NULL; 
      r3 = FloatType; 
      array3 = &f3; 
      size3 = 1; 
    } 
    else { 
      if (!(r3=TTSetOutput2(r1,vc1,r2,vc2,&vc3,' '))) { 
        Clean(vc1);  
        Clean(vc2); 
        *resVC = NULL; 
        return(0); 
      } 
      if (r3==RangeType) { 
        array3 = NULL; 
        rg3 = (RANGE) vc3; 
        size3 = rg3->size;
        r3 = RangeType; 
      } 
      else if (r3==SignalType) { 
        s3 = (SIGNAL) vc3; 
        array3 = s3->Y;
        size3 = s3->size;
        r3 = SignalType; 
      }
      else if (r3==ImageType) { 
        i3 = (IMAGE) vc3; 
        array3 = i3->pixels; 
        size3 = i3->nrow*i3->ncol; 
        r3 = ImageType; 
      } 
      else { 
        Errorf("the weird"); 
      }
    }      
    
    
  switch(opIndex) {
     
    /* || */ 
    case 1 :     
    if (array1 == NULL && array2 != NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (RangeVal(rg1,j1)!=0)||(array2[j2]!=0);
      }
    }
    else if (array1 != NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (array1[j1]!=0)||(RangeVal(rg2,j2)!=0);
      }
    }
    else if (array1 == NULL && array2 == NULL) {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (RangeVal(rg1,j1)!=0)||(RangeVal(rg2,j2)!=0);
      }
    }
    else {
      for (j1=j2=j3=0;j3<size3;j3++,j2++,j1++) {
        j1 %= size1; j2 %= size2;
        array3[j3] = (array1[j1] != 0) || (array2[j2] != 0);
      }
      break;
   }
  }
  
  
    /* 
     * Set the new input 
     */ 
    if (r3 != FloatType) { 
      if (vc3 == vc2) {Clean(vc1);} 
      else if (vc3 == vc1) {Clean(vc2);} 
      else { 
        Clean(vc1); 
        Clean(vc2); 
      } 
      *resVC = vc1 = vc3; 
      vc2 = vc3 = NULL; 
    }
    else {
      Clean(vc1);
      Clean(vc2);
      *resVC = vc3 = NULL;
      f1 = *resFlt = f3; 
    }
    r = r1 = r3; 
  } 
  begin = *left; 
  return(r);  
}


/*
 * This tries to read a vector of the type 1:1:2
 *
 * expr1 -> expr2:expr2:expr2 or expr2
 *
 */

/* 
 * flagFirst, flagLast and flagSize will be either 0,1,2,3 
 *   !:          :!          #
 * 0 : no such sign
 * 1 = after the first digit
 * 2 = after the second digit
 * 3 = before the third digit
 */
unsigned char TTExpr1(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam) 
{
  float f1,f2,f3,f;
  char flagDef1,flagDef2,flagDef3,flagErr;
  char flagFirst,flagLast,flagSize; /* flagFirst, flagLast and flagSize will be either 0,1,2,3 */ 
  unsigned char r;
  RANGE rg;
  SIGNAL sig;    
  int size,i,i1,i3;
  unsigned char flagType1;
  char *begin1;
 

  /* If we do not accept a float and it is of the form 10 or .10, then we are dealing with the current object */
  if (!(flagType & FloatType) && (isdigit(*begin) || *begin == '.')) {
    begin1 = begin;
    if (*begin1 == '.') begin1++;
    while (isdigit(*begin1)) begin1++;
    ParamSkipSpace(begin1);
    if (*begin1 == '\0' || *begin1 == ' ' || *begin1 == '}' || *begin1 == ')') {
      *resVC = NULL;
      return(TGetVariableContentLevelExpr(DefaultLevel(defParam),begin,left,resFlt,resVC,flagType,NO,defParam));   
    }
  }      



  begin1 = begin;
  
  /*
   * Case it does not start with a ':' or a !: 
   */
  if (*begin!=':' && strncmp(begin,"!:",2)) {
  
    /* Read first argument */
    flagType1 = flagType;
    if (flagType & (SignalType | ImageType | RangeType)) flagType1 = flagType1 | FloatType;
    r = TTExpr2(begin, left, resFlt, resVC, flagType1,defParam);
    if (r == 0) return(0);
      
    /* Case there is no ':' or !: */
    if (**left != ':' && strncmp(*left,"!:",2)) {
      if (r == FloatType && !(flagType & FloatType)) {
        *left = begin;
        SetErrorf("Not expecting a number");
        Clean(*resVC);
        return(0);
      }
      return(r);
    }

    /* If result is not a Float then error */    
    if (r != FloatType) {
      SetErrorf("Expecting a number");
      Clean(*resVC);
      *left = begin;
      return(0);
    }
  }
  
  /* Case we do not expect a range or a signal */
  if (!(flagType & SignalType) && !(flagType & RangeType)) {
    SetErrorf("Not expecting a range");
    *left = begin;
    Clean(*resVC);
    return(0);
  }


  /*
   * Case we detected a range 
   */
  
  /* Some inits */
  flagDef1 = flagDef2 = flagDef3 = YES;
  flagSize = 0;
  flagFirst = flagLast = 0;
  
  
  /*
   * First digit 
   */
  
  /* Did we already read a digit ? */
  if (*begin!=':' && strncmp(begin,"!:",2)) {
    f1 = *resFlt;
    flagDef1 = NO;
    begin = *left;
  }    

  /* What sign do we have ? */
  if (!strncmp(begin,"!:",2)) {
    flagFirst = 1;
    begin+=2;
  }
  else if (!strncmp(begin,":!",2)) {
    flagLast = 1;
    begin+=2;
  }
  else if (*begin == ':') {
    begin++;
  }
  
  *left = begin;
  
  /*
   * Second digit 
   */
  if (!(*begin == ' ' || *begin == '}' || *begin == ']' || *begin == ',' || *begin == ')' || *begin == '*' || *begin == '/' || 
      *begin == '%' || *begin == '^' || *begin == '<' || *begin == '>' ||
      *begin == '&' || *begin == '|' || *begin == '=' || *begin == FSISEPARATOR)) {
  
    /* Should we first read a number ? */
    if (strncmp(begin,"!:",2) && *begin!=':' || *begin == '#') {      
      if (*begin == '#') {
        flagSize = 1;
        begin++;
      }
      r = TTExpr2(begin,left,resFlt,resVC, FloatType,defParam);
      if (r == 0) return(0);
      if (flagSize && (*resFlt <=0 || *resFlt != (int) *resFlt)) {
        SetErrorf("Range size should be a strictly positive integer");
        *left = begin;
        Clean(*resVC);
        return(0);
      }
      begin = *left;
      f2 = *resFlt;
      flagDef2 = NO;
    }
    
    /* Let's look at a sign */
    if (!strncmp(begin,"!:",2)) {
      SetErrorf("Misplaced '!:'");
      Clean(*resVC);
      return(0);
    }
    if (!strncmp(begin,":!",2)) {
      if (flagLast) {
        SetErrorf("Misplaced '!:'");
        Clean(*resVC);
        return(0);
      }
      flagLast = 2;
      begin+=2;
    }
    else if (*begin == ':') {
      begin++;
    }
    else {
      if (flagSize) {
        SetErrorf("bad syntax for ranges");
        Clean(*resVC);
        return(0);
      }
      flagDef3 = NO;
      flagDef2 = YES;
      f3 = f2;
    }
    
    *left = begin;
        
    /*
     * Third digit 
     */
    if (flagDef3 && !(*begin == ' ' || *begin == '}' || *begin == ']' || *begin == ',' || *begin == ')' || *begin == '*' || *begin == '/' || 
        *begin == '%' || *begin == '^' || *begin == '<' || *begin == '>' ||
        *begin == '&' || *begin == '|' || *begin == '=' || *begin == FSISEPARATOR)) {
      r = TTExpr2(begin,left,resFlt,resVC, FloatType,defParam);
      if (r == 0) return(0);
      f3 = *resFlt;
      flagDef3 = NO;
    }
  }
  
  /* 
   * We have read the whole vector, we need to take care of the default values
   */
   
  flagErr = NO;

  /* Deal with default values in the case there is a default vector */
  if (!NoDefaultVector(defParam)) {
    if (flagDef2 == YES) {
      if (DefaultVDx(defParam)<=0) {     /* Case it is a signal (extraction of an XYSIG) --> we eventually add 2 points */
        if (flagDef1) f1 = DefaultVMin(defParam);
        if (flagDef3) f3 = DefaultVMax(defParam);
        if (f1 >= f3) {
          SetErrorf("Sorry : this case of extraction for XYSig has not been implemented yet");
          Clean(*resVC);
          return(0);
        }
        if (f1 > defParam->_vxsignal->X[defParam->_vxsignal->size-1] || f3 < defParam->_vxsignal->X[0]) {
          SetErrorf("Empty range");
          Clean(*resVC);
          return(0);
        }
        i1 = ISig(defParam->_vxsignal,f1);
        if (defParam->_vxsignal->X[i1] < f1) i1++;
        i3 = ISig(defParam->_vxsignal,f3);
        if (defParam->_vxsignal->X[i3] > f3) i3--;
        sig = TNewSignal();
        SizeSignal(sig,i3-i1+3,YSIG);
        sig->size = i3-i1+1;
        i=0;
        if (!flagFirst && defParam->_vxsignal->X[0] > f1) {
          i = 1;
          sig->Y[0] = f1;
          sig->size++;
        }
        memcpy(sig->Y+i,defParam->_vxsignal->X+i1,(i3-i1+1)*sizeof(float));
        i+=(i3-i1+1);
        if (!flagLast && defParam->_vxsignal->X[defParam->_vxsignal->size-1] < f3) {
          sig->Y[i] = f3;
          sig->size++;
        }
        *resVC = (VALUE) sig;
        return(SignalType);
      }      
      f2 = DefaultVDx(defParam);
    }
    if (flagDef1 == YES) {
      if (f2 > 0) f1 = DefaultVMin(defParam);
      else f1 = DefaultVMax(defParam);
    }
    if (flagDef3 == YES) {
      if (f2 < 0) f3 = DefaultVMin(defParam);
      else f3 = DefaultVMax(defParam);
    }
    /* if f1:f3  ===> we change it so that it fits the grid */
    if (flagDef2) {
      i1 = (int) ((f1-DefaultVMin(defParam))/f2);
      i3 = (int) ((f3-DefaultVMin(defParam))/f2);
      if (i1*f2+DefaultVMin(defParam) < f1) i1++;
      if (flagFirst && i1*f2+DefaultVMin(defParam) > f1) flagFirst = NO;
      f1 = i1*f2+DefaultVMin(defParam);
      if (i3*f2+DefaultVMin(defParam) > f3) i3--;
      if (flagLast && i3*f2+DefaultVMin(defParam) < f3) flagLast = NO;
      f3 = i3*f2+DefaultVMin(defParam);
    }
  }

  /* Deal with default values in the case there is no default vector but a default signal */
  else if (!NoDefaultSig(defParam)) {
    if (flagDef1 == YES) {
      if (flagDef2 == YES) {
        if (flagDef3 == YES) { /* : */
          f1 = DefaultSigX0(defParam);
          f2 = DefaultSigDx(defParam);
          f3 = (DefaultSigSize(defParam)-1)*f2+f1;
        }
        else flagErr = YES; /* :f3 */
      }
      else flagErr = YES; /* :f2:f3 */
    }
    else {
      if (flagDef2 == YES) {
        if (flagDef3 == YES) flagErr = YES; /* f1: */
        else {
          if (flagSize) flagErr = YES;
          else {
            flagSize = 1;
            f2 = DefaultSigSize(defParam);
          }
        }
      }
      else {
        if (flagDef3 == YES) flagErr = YES; /* f1:f2: */
      }
    }
  }
  else if (flagDef1 == NO && flagDef2 == YES && flagDef3 == NO) {if (f3 >= f1) f2 = 1; else f2 = -1;} /* f1:f3 */    
  else if (flagDef1 == YES || flagDef2 == YES || flagDef3 == YES) flagErr = YES;
  
  /* Take care of eventual errors */
  if (flagErr) {
    Clean(*resVC);
    SetErrorf("Cannot infer range parameters");
    *left = begin1;
    return(0);
  }
         
  /* test consistency of parameters */
  if ((flagSize==NO && ((f3 < f1 && f2 > 0) || (f3 > f1 && f2 <0) || f2 == 0)) ||
      (flagSize==YES && f2<=0)) {
    if (!(defParam->flagEmptySI)) {
      *left = begin1;
      SetErrorf("Range parameters are not consistent");
      Clean(*resVC);
      return(0);
    }
    Clean(*resVC);
    *resVC = (VALUE) TNewSignal();
    return(SignalType);
  }
  
  Clean(*resVC);

  /* computing the size,step and first */
  if (!flagFirst) {
    if (flagSize) {
      size = f2;
      if (!flagLast) {  /* a:#b:c */
        if (size == 1) f2 = 1;
        else f2 = (f3-f1)/(size-1);
      }                 
      else {           /* a:#b:!c */
        f2 = (f3-f1)/size;
      }
    }
    else {
      size = (int) (fabs(((f3-f1)/f2))+1); /* a:b:c */
      if (flagLast) {                        /* a:b:!c */
        if ((f2 >0 && f1+f2*(size-1) >= f3) || (f2 <0 && f1+f2*(size-1) <= f3)) size--;
      }
    }
  }
  else {
    if (flagSize) {
      size = f2;   
      if (!flagLast)  {                /* a!:#b:c */
        f2 = (f3-f1)/size;
        f1 += f2;
      }
      else {                        /* a!:#b:!c */
        f2 = (f3-f1)/(size+1);
        f1 += f2;
      }
    }
    else {               /* a!:b:c */
      size = (int) (fabs(((f3-f1)/f2))+1);
      size--;
      f1 += f2;
      if (flagLast) {                        /* a!:b:!c */
        if ((f2 >0 && f1+f2*(size-1) >= f3) || (f2 <0 && f1+f2*(size-1) <= f3)) size--;
      }
    }
  }

  /* test consistency ... again .. just in case ... */
  if (size <= 0) {
    *left = begin;
    SetErrorf("Range not consistent");
    Clean(*resVC);
    return(0);
  }

  /* Case we must return a float */
  if (size == 1) {
    *resFlt = f1;
    return(FloatType);
  }
  /* Case we must return a signal */
  else if (!(flagType & RangeType)) { 
    sig = TNewSignal();
    *resVC = (VALUE) sig;
    SizeSignal(sig,size,YSIG);
    for (f = f1,i=0;i<size;f+=f2,i++) sig->Y[i] = f;
    SetDefaultSizeX0Dx(defParam,size,0,1,NO);
    return(SignalType);
  }
  /* Case we must return a range */
  else {
    rg = TNewRange();
    *resVC = (VALUE) rg;
    rg->first = f1;
    rg->step = f2;
    rg->size = size;
    SetDefaultSizeX0Dx(defParam,size,0,1,NO);
    return(RangeType);
  }
}


/*
 * expr0 -> expr0bis and nothing after
 *
 */


static unsigned char TTExpr0(char* begin, char** left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam) 
{
  unsigned char r;
  
  ParamSkipSpace(begin);
  r = TTExpr1(begin,left,resFlt,resVC,flagType,defParam);
  if (r==0) return(0);
  
  begin = *left;
  
  ParamSkipSpace(begin);
  
  if (*begin != '\0') {
    SetErrorf("Syntax error");
    Clean(*resVC);
    return(0);
  }
  
  if (r == RangeType && ((RANGE) *resVC)->size == 1) {
    *resFlt = ((RANGE) *resVC)->first;
    Clean(*resVC);
    r = FloatType;
  }
  return(r);
}

 

/*
 * This is the most basic evaluator function 
 *
 * It tries to evaluate the expression 'arg' at level 'level'.
 *
 * The expression is expected to be of type 'flagType' 
 *   (where flagType is a combination of 
 *    FloatType, SignalType, ImageType, StringType and OtherType or AnyType).
 *
 * The type of the result is returned it is one of strType, floatType, isignalType.....
 *    floatType => 'result' is stored in resFloat (and maybe resVC at the same time)
 *    all other types => 'result' is  stored in resVC which is a VariableContent
 *    The reference count of result is incremented by 1 and TempValue is called
 *
 * If it returns  NULL it means that an error ocurred and *resVC is NULL (desallocated)
 *
 *
 * If 'flagFast' is set it first look if the expression is just a number before going into
 * the evaluator.
 *
 * if 'flagSubst' is set then $ substitution are performed during evaluation.
 *
 * 'listvElemType' is the type of listv elements.
 * 
 * 'flagEmptySI' is a flag that authorize empty signals or images.
 */
 
char *TTEvalExpressionLevel_(LEVEL level,char* arg, float *resFloat, VALUE *resVC,  unsigned char flagType, char flagSubst, char flagFast, unsigned char listvElemType, char flagEmptySI)
{
  char *error,*endp,*begin;
  char answer,*type;
  ExprDefParam defParam;
  double d;


  /* First we try to answer fast if possible */
  if (flagFast && (flagType & FloatType)) {
    d = strtod(arg,&endp);
    if (*endp == '\0') {
      *resFloat = (float) d;
      Clean(*resVC);
      return(numType);
    }
  }
  
  /* Some inits (default size of signal/image) */
  InitDefaults(&defParam,level,flagSubst,flagType & FloatType);
  defParam.listvElemType = listvElemType;
  defParam.flagEmptySI = flagEmptySI;
  
  /* If we do not accept a float and it is of the form 10 or .10, then we are dealing with the current object */
  begin = NULL;
  if (!(flagType & FloatType) && (isdigit(*arg) || *arg == '.')) {
    begin = arg;
    if (*begin == '.') begin++;
    while (isdigit(*begin)) begin++;
    SkipSpace(begin);
    if (*begin == '\0') {
      *resVC = NULL;
      answer = TGetVariableContentLevelExpr(DefaultLevel(&defParam),arg,&error,resFloat,resVC,flagType,NO,&defParam);
    }
    else begin = NULL;
  }      
  
  /* Otherwise, regular evaluation */  
  if (begin == NULL) {
    *resVC = NULL;
    answer = TTExpr0(arg,&error,resFloat,resVC,flagType,&defParam);
  }

  /* Inits again */
  InitDefaults(&defParam,NULL,NO,NO);

  /* Set the error message if failed */
  if (answer == 0) {
    SyntaxError(NULL,arg,error);
    return(NULL);
  }  

  /* Get the type */
  if (*resVC != NULL) type = GetTypeValue(*resVC);
  else type = numType;
  
  return(type);
}


void Setv1(LEVEL level, char *name, char *equal, char *type, VALUE val, float f)
{
  VALUE vcRes;
  float fltRes;
  char *strRes;
  char *error;
  extern unsigned char SetVariableLevelExpr(char *equal, LEVEL level, char *begin, char **left, char *type, float fltSet, VALUE vcSet, ExprDefParam *defParam,VALUE *pValueRes, float *pfltRes, char **pstrRes);
  ExprDefParam defParam;

  InitDefaults(&defParam,level,YES,0);

  error = name;
  vcRes = NULL;
  strRes = NULL;
  if (SetVariableLevelExpr(equal,level,name,&error,type,f,val,&defParam,&vcRes,&fltRes,&strRes)==NO) {
    if (error != NULL) {
      SyntaxError(NULL,name,error);
      Errorf1("");
    }
    else Errorf1("");
    return;
  }  
  if (vcRes != NULL) SetResultValue(vcRes);
  else if (strRes != NULL) SetResultStr(strRes);
  else SetResultFloat(fltRes);
}

void SetvLevel(LEVEL level, char *name, VALUE val)
{
  char *type = GetTypeValue(val);
  float f;
  
  if (type == numType) f = ((NUMVALUE) val)->f;
  
  Setv1(level,name,"=",type,val,f);
}

void Setv(char *name, VALUE val)
{
  SetvLevel(levelCur,name,val);
}

  
void C_Setv(char **argv)
{  
  char *name,**nameList;
  char *type;
  char *expr,*equal;
  LISTV lv,lv1;
  float f;
  int i,j;
  VALUE val;
  LEVEL level;
  char opt;
    
  argv = ParseArgv(argv,tWORD,&name,-1);
  if (*argv == NULL) {/* setv name */
    equal = NULL;
    expr = NULL;
  }
  else if (argv[1] == NULL) { /* setv name expr */
    argv = ParseArgv(argv,tWORD,&expr,-1);
    equal = "=";
  }
  else if (!strcmp(argv[1],"-l") && argv[2] != NULL) {/* setv name expr -l level */
    argv = ParseArgv(argv,tWORD,&expr,-1);
    equal = "=";  
  }
  else { /* setv name = expr */
    argv = ParseArgv(argv,tWORD,&equal,tWORD,&expr,-1);
  }
  
  level = levelCur;
  while(opt = ParseOption(&argv)) { 
    switch(opt) {
      case 'l': argv = ParseArgv(argv,tLEVEL,&level,0); break;
      default: ErrorOption(opt);
    }
  }    
  NoMoreArgs(argv);
  
  /*
   * Case there is just one argument ---> just evaluation
   */
  if (expr == NULL) {
    type = TTEvalExpressionLevel_(level,name,&f,&val,AnyType,NO,NO,AnyType,YES);
    if (type==NULL) Errorf1("");
    if (val != NULL) SetResultValue(val);
    else SetResultFloat(f);
    return;
  }
  
  /*
   * Case there are 2 arguments ---> evaluation + assignement
   */
  type = TTEvalExpressionLevel_(levelCur,expr,&f,&val,AnyType,NO,NO,AnyType,YES);
  
  if (type==NULL) 
    Errorf1("");

  /* If the value is not a listv then just assign it */
  if (type != listvType) Setv1(level,name,equal,type,val,f);
  
  /* Otherwise we have to read a list of names */
  else {
    if (*name != '{') {
      Setv1(level,name,equal,type,val,f);
      return;
    }

    ParseWordList(name,&nameList);
  
    /* If just one name and no bracket --> just assignment */
    if (nameList[0] != NULL && nameList[1] == NULL && *name!='{') Setv1(level,name,equal,type,val,f);
    
    /* We have to loop */
    else {
      
      lv = (LISTV) val;
      for (i=0;*nameList != NULL && i<lv->length;i++,nameList++) {

        if (**nameList != '.') {
          type = GetListvNth(lv,i,&val,&f);
          Setv1(level,*nameList,equal,type,val,f);
        }
        else {
          lv1 = TNewListv();
          SetLengthListv(lv1,lv->length-i);
          for (j=i;j<lv->length;j++) {
            type = GetListvNth(lv,j,&val,&f);
            if (type == numType) SetListvNthFloat(lv1,j-i,f);
            else SetListvNthValue(lv1,j-i,val);
          }
          Setv1(level,(*nameList)+1,equal,listvType,(VALUE) lv1,f);
          i = lv->length-1;
        }
      }
      
      if (*nameList != NULL && **nameList == '.') {
        Setv1(level,(*nameList)+1,equal,nullType,(VALUE) nullValue,f);
        nameList++;
      }
      
      if (*nameList != NULL) Errorf("The evaluated listv is too short");
      if (i!=lv->length) Errorf("The evaluated listv is too long");
      SetResultValue(lv);
    }
  }
        
}


void C_Info(char **argv)
{
  char *expr;
  float f;
  VALUE val;
  
  argv = ParseArgv(argv,tWORD,&expr,0);
  
  if (ParseFloatValLevel_(levelCur,expr,&f,&val,AnyType,AnyType,YES)==NULL) Errorf1("");
  if (val==NULL) {
    Printf("Type '%s'   [%d] : \n",numType,0);
#ifdef NUMDOUBLE
  Printf("   f =  %.16g\n",f);
#else
  Printf("   f =  %.8g\n",f);
#endif
    return;
  }

  Printf("Type '%s'   [nref=%d] [%p]\n",GetTypeValue(val),val->nRef,val);
  PrintInfoValue(val);
}


void C_Print(char **argv)
{
  VALUE val;
  
  argv = ParseArgv(argv,tVAL,&val,-1);
  if (*argv != NULL && !strcmp(*argv,"-s")) {
    argv++;
    NoMoreArgs(argv);
    Printf("%s\n",ToStrValue(val,YES));
    return;
  }
    
  Printf("%s = \n",*(argv-1));
  PrintValue(val);
  
  while (*argv != NULL) {
    argv = ParseArgv(argv,tVAL,&val,-1);
    Printf("%s = \n",*(argv-1));
    PrintValue(val);
  }
}



/*
 * The main command for managing values
 */
void C_Val(char **argv)
{
  char *action,*str,*type,*type1;
  LEVEL level;
  VALUE val;
  Field *fields;
  LISTV lv;
  char opt,flag,flagError;
  char flagBasic,flagEmptySI;
  unsigned char cType1;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  level = levelCur;
  
  /* The 'test' action */  
  if (!strcmp(action,"test")) {
    argv = ParseArgv(argv,tWORD,&str,tSTR_,NULL,&type,-1);
    if (type != NULL) {
      if (GetArgType(type) == NULL) Errorf("Bad type '%s'",type);
      type = GetArgType(type);
    }
    flagEmptySI = YES;
    type1 = NULL;
    flagError = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'l': argv = ParseArgv(argv,tLEVEL,&level,-1); break;
        case 't': if (type != listvType) Errorf("Option '-t' is valid only with type '&listv'");
           argv = ParseArgv(argv,tSTR,&type1,-1);
           if (GetArgType(type1) == NULL) Errorf("Bad type '%s'",type1);
           type1 = GetArgType(type1);
           break;
        case 'E': flagError = YES; break;
        case 'e': flagEmptySI = NO; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);

    if (type1 != NULL) {
      if (type1 == valobjType) cType1=AnyType-FloatType;
      else if (type1 == strType) cType1=StringType;
      else if (type1 == numType) cType1=FloatType;
      else cType1=AnyType;
      flag = ParseValLevel__(level,str,NULL,&val,ListvType,cType1,flagEmptySI);
    }
    else if (type==NULL) {
      flag=ParseValLevel_(level,str,NULL,&val);
    }
    else {
      flag = ParseTypedValLevel_(level,str,NULL,&val,type);
    }
    
    if (flag) {
      lv = TNewListv();
      AppendStr2Listv(lv,GetTypeValue(val));
      AppendValue2Listv(lv,val);
      SetResultValue(lv);
    }
    else if (!flagError) SetResultValue(nullValue);
    else Errorf1("");
    return;
  }

  /* The 'eval' action */  
  if (!strcmp(action,"eval")) {
    argv = ParseArgv(argv,tWORD,&str,-1);
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'l': argv = ParseArgv(argv,tLEVEL,&level); break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    ParseValLevel(level,str,&val);
    SetResultValue(val);
    return;
  }

  /* The 'type' action */  
  if (!strcmp(action,"type")) {
    argv = ParseArgv(argv,tWORD,&str,-1);
   
    flagBasic = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
      case 'l': argv = ParseArgv(argv,tLEVEL,&level); break;
      case 'b': 
        flagBasic = YES; 
        break;
      default:
        ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    
    ParseValLevel(level,str,&val);
    
    if (flagBasic==NO) SetResultStr(GetTypeValue(val));
    else SetResultStr(GetBTypeContent(val));
    
    return;
  }

  Errorf("Unknown action '%s'",action);  
}
