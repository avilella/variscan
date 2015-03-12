/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
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
#include <ctype.h>
#include <stdarg.h>
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"

#define MaxLengthList 14000       /* Maximum length of a list                 */
#define MaxNumWords 1500          /* Maximum number of words in a list        */
#define MaxNumCommands 400        /* Maximum number of commands in a script   */



/***********************************************************************************
 *
 *  Parsing a number  
 *
 * if 'flagSubst' is set then it performs substitution
 * if 'flagFast', it starts by trying to read a simple number before going into the evaluator
 * if 'flagTempAlloc' then temporary alocation will be deleted after at the end of the call 
 *
 ***********************************************************************************/

char ParseNumLevel_(LEVEL level, char *arg, float *f, char flagTempAlloc, char flagSubst, char flagFast)
{
  char *type;
  float resFlt;
  VALUE resVC;
  
  if (flagTempAlloc) SetTempAlloc();

  resVC = NULL;
  type = TTEvalExpressionLevel_(level,arg,&resFlt,&resVC,FloatType,flagSubst,flagFast,AnyType,NO);

  if (flagTempAlloc) ClearTempAlloc();

  if (type == NULL) return(NO);
  *f = resFlt;
  return(YES);
}


/***********************************************************************************
 *
 *  Parsing an int  
 *
 ***********************************************************************************/

char ParseIntLevel_(LEVEL level, char *arg, int defVal, int *i)
{
  long l;
  float f;
  char *endp;

  *i = defVal;
  if (arg == NULL) {
    SetErrorf("ParseIntLevel_() : NULL string cannot be converted to an int");
    return(NO);
  }

  if (*arg == '\0') {
    SetErrorf("ParseIntLevel_() : empty string cannot be converted to an int");
    return(NO);
  }
  
  l = strtol(arg,&endp,0);
  
  if (*endp != '\0') {
  
    if (!(ParseNumLevel_(level,arg,&f,NO,NO,NO))) return(NO);
    if (f != (int) f) {
      SetErrorf("ParseInt_() : '%.8g' is not an integer",f);
      return(NO);
    }
    l = (int) f;
  }   

  if (l >= INT_MAX || l <= INT_MIN) {
    SetErrorf("ParseInt() : '%s' is out of range",arg);
    return(NO);
  }
  
  *i = l;

  return(YES);
}

/* Same as above but with current level */
char ParseInt_(char *arg, int defVal, int *i)
{
  return(ParseIntLevel_(levelCur,arg,defVal,i));;
}

/* Same as above but generate an error (no default value) */
void ParseIntLevel(LEVEL level, char *arg, int *i)
{
  if (ParseIntLevel_(level,arg,0,i) == NO) Errorf1("");
}

/* Same as above but with current level */
void ParseInt(char *arg, int *i)
{
  if (ParseIntLevel_(levelCur,arg,0,i) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a float  
 *
 ***********************************************************************************/

char ParseFloatLevel_(LEVEL level, char *arg, float defVal, float *x)
{
  float f;
  char *endp;
  
  *x = defVal;
  if (arg == NULL) {
    SetErrorf("ParseFloatLevel_() : NULL string cannot be converted to a float");
    return(NO);
  }
  
  if (*arg == '\0') {
    SetErrorf("ParseFloatLevel_() : empty string cannot be converted to a float");
    return(NO);
  }
  
  f = (float) strtod(arg,&endp);
  
  if (*endp != '\0' && !(ParseNumLevel_(level,arg,&f,NO,NO,NO))) return(NO);   

/*  merde CW9 if (l >= FLT_MAX || l <= FLT_MIN) {
    SetErrorf("ParseFloat() : '%s' is out of range",arg);
    return(NO);
  }
  */
  
  *x = f;

  return(YES);
}

/* Same as above but with current level */
char ParseFloat_(char *arg, float defVal, float *i)
{
  return(ParseFloatLevel_(levelCur,arg,defVal,i));
}

/* Same as above but generate an error (no default value) */
void ParseFloatLevel(LEVEL level, char *arg, float *i)
{
  if (ParseFloatLevel_(level,arg,0,i) == NO) Errorf1("");
}

/* Same as above but with current level */
void ParseFloat(char *arg, float *i)
{
  if (ParseFloatLevel_(levelCur,arg,0,i) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a val
 *
 ***********************************************************************************/

/*
 * Parse the string 'arg' at level 'level' to get a value and puts it 
 * either in 'f' if it is a float or in 'val' for all other cases.
 * In the case the result is not a float, 'val' is a temporary object
 *
 * 'flagType' is a flag to determine which types are accepted
 *
 * It returns either NULL or the type of the read Value.
 *
 */ 
 
char *ParseFloatValLevel_(LEVEL level, char *arg, float *f, VALUE *val, unsigned char flagType, unsigned char listvElemType, char flagEmptySigIm)
{
  *val = NULL;
  
  /* If arg is NULL --> error */
  if (arg == NULL) {
    SetErrorf("ParseFloatValLevel_() : NULL string cannot be converted to a value");
    return(NULL);
  }
  
  /* If arg == '' ---> error */
  if (*arg == '\0') {
    SetErrorf("ParseFloatValLevel_() : empty string cannot be converted to a value");
    return(NULL);
  }
  
  /* Eval the expression 'arg' */
  *val = NULL;
  return(TTEvalExpressionLevel_(level,arg,f,val,flagType,NO,NO,listvElemType,flagEmptySigIm));  
}


/*
 * The routines for Parsing Values at a given level
 */

/* The basic routine */
char ParseValLevel__(LEVEL level, char *arg, VALUE defVal, VALUE *val, unsigned char flagType, unsigned char listvElemType, char flagEmptySigIm)
{
  char *answer;
  float f;
  NUMVALUE nc;
  
  answer = ParseFloatValLevel_(level,arg,&f,val,flagType,listvElemType,flagEmptySigIm);
  
  if (answer == NULL) {
    *val = defVal;
    if (defVal != NULL) {
      AddRefValue(defVal);
      TempValue(defVal);
    }
    return(NO);
  }
  
  if (*val != NULL) return(YES);

  nc = NewNumValue();
  SetNumValue(nc,f);
  *val = (VALUE) nc;
  TempValue(*val);
  return(YES);
}

char ParseTypedValLevel_(LEVEL level, char *arg, VALUE defVal, VALUE *val, char *type)
{
  char flag;
  
  if (type == valobjType) return(ParseValLevel__(level,arg,defVal,val,AnyType-FloatType,AnyType,YES));
  if (type == valType) return(ParseValLevel__(level,arg,defVal,val,AnyType,AnyType,YES));
  if (type == scriptType) return(ParseScriptLevel_(level,&arg,(SCRIPT) defVal,(SCRIPT *) val));
  if (type != numType) flag = ParseValLevel__(level,arg,defVal,val,AnyType-FloatType,AnyType,YES);
  else flag = ParseValLevel__(level,arg,defVal,val,FloatType,AnyType,YES);  
   
  if (flag == NO) return(NO);
  
  if (GetTypeValue(*val) != type && GetBTypeContent(*val) != type) {
    SetErrorf("ParseTypedValLevel_() : value is of type '%s' and not of expected type '%s'",type,GetTypeValue(*val));
    if (*val == defVal) return(NO);
    *val = defVal;
    
    if (*val == NULL) return(NO);
  
    if (GetTypeValue(*val) != type) {
      SetErrorf("ParseTypedValLevel_() : default value is of type '%s' and not of expected type '%s'",type,GetTypeValue(*val));
      return(NO);
    }
    return(NO);
  }
  
  return(flag);
}

char ParseValLevel_(LEVEL level, char *arg, VALUE defVal, VALUE *val)
{
  return(ParseValLevel__(level,arg,defVal,val,AnyType,AnyType,YES));
}

void ParseValLevel(LEVEL level, char *arg, VALUE *val)
{
  if (!ParseValLevel_(level,arg,NULL,val)) Errorf1("");
}


/*
 * The routines for Parsing Values at the current level
 */
char ParseVal_(char *arg, VALUE defVal, VALUE *val)
{
  return(ParseValLevel_(levelCur,arg,defVal,val));
}

void ParseVal(char *arg, VALUE *val)
{
  if (!ParseVal_(arg,NULL,val)) Errorf1("");
}

/***********************************************************************************
 *
 *  Parsing a valobj, i.e., parsing a value which is not a number
 *
 ***********************************************************************************/

char ParseValObjLevel_(LEVEL level, char *arg, VALUE defVal, VALUE *val)
{
  return(ParseValLevel__(level,arg,defVal,val,AnyType-FloatType,AnyType,YES));
}

void ParseValObjLevel(LEVEL level, char *arg, VALUE *val)
{
  if (!ParseValObjLevel_(level,arg,NULL,val)) Errorf1("");
}

char ParseValObj_(char *arg, VALUE defVal, VALUE *val)
{
  return(ParseValObjLevel_(levelCur,arg,defVal,val));
}

void ParseValObj(char *arg, VALUE *val)
{
  if (!ParseValObj_(arg,NULL,val)) Errorf1("");
}

/***********************************************************************************
 *
 *  Parsing a double  
 *
 ***********************************************************************************/

char ParseDoubleLevel_(LEVEL level,char *arg, double defVal, double *x)
{
  float f;
  char *endp;
  
  *x = defVal;
  if (arg == NULL) {
    SetErrorf("ParseDoubleLevel_() : NULL string cannot be converted to a double");
    return(NO);
  }
  
  if (*arg == '\0') {
    SetErrorf("ParseDoubleLevel_() : empty string cannot be converted to a double");
    return(NO);
  }
  
  f = (float) strtod(arg,&endp);
  
  if (*endp != '\0' && !(ParseNumLevel_(level,arg,&f,NO,NO,NO))) return(NO);   
  
  *x = f;

  return(YES);
}

/* Same as above but with current level */
char ParseDouble_(char *arg, double defVal,double *i)
{
  return(ParseDoubleLevel_(levelCur,arg,defVal,i));
}

/* Same as above but generate an error (no default value) */
void ParseDoubleLevel(LEVEL level, char *arg, double *i)
{
  if (ParseDoubleLevel_(level,arg,0,i) == NO) Errorf1("");
}

/* Same as above but with current level */
void ParseDouble(char *arg, double *i)
{
  if (ParseDoubleLevel_(levelCur,arg,0,i) == NO) Errorf1("");
}



/***********************************************************************************
 *
 *  Parsing a character evaluated
 *
 ***********************************************************************************/

char ParseCharLevel_(LEVEL level, char *arg, char defVal, char *c)
{
  char *str;
  
  if (ParseStrLevel_(level,arg,NULL,&str) == NO) {
    *c = defVal;
    return(NO);
  }
  
  if (str[1] != '\0') {
    SetErrorf("ParseCharLevel_() : cannot convert string %s to a character",str);
    *c = defVal;
    return(NO);
  }
  
  *c = *str;  
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseCharLevel(LEVEL level, char *arg, char *c)
{
  if (ParseCharLevel_(level,arg,'\0',c) == NO) Errorf1("");
}

char ParseChar_(char *arg, char defVal, char *c)
{
  return(ParseCharLevel_(levelCur, arg,defVal,c));
}


/* Same as above but generate an error (no default value) */
void ParseChar(char *arg, char *i)
{
  if (ParseChar_(arg,'\0',i) == NO) Errorf1("");
}

/***********************************************************************************
 *
 *  Parsing a word 
 *
 ***********************************************************************************/

char ParseWord_(char *arg, char *defVal, char **str)
{
  *str = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseWord_() : NULL string cannot be converted to a string");
    return(NO);
  }

  *str = arg;
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseWord(char *arg, char **i)
{
  if (ParseWord_(arg,"",i) == NO) Errorf1("");
}

/***********************************************************************************
 *
 *  Parsing a word which is a symbol  
 *
 ***********************************************************************************/

/* 
 * Is it a valid symbol ? 
 */
 
char IsValidSymbol(char *name)
{
  if (name == NULL || *name == '\0') return(NO);
  if (IsValidSymbolChar1(*name) == NO) return(NO);
  
  name++;
  while (*name != '\0' && IsValidSymbolChar(*name)) name++;
  if (*name == '\0') return(YES);
  return(NO);
}


char ParseSymbol_(char *arg, char *defVal, char **str)
{
  *str = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseSymbol_() : NULL string cannot be converted to a symbol");
    return(NO);
  }
  
  if (!IsValidSymbol(arg)) {
    SetErrorf("ParseSymbol_() : '%s' not a valid symbol",arg);
    return(NO);
  }

  *str = arg;
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseSymbol(char *arg, char **i)
{
  if (ParseSymbol_(arg,NULL,i) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a variable name just check that it starts with the right character
 *
 ***********************************************************************************/

char ParseVName_(char *arg, char *defVal, char **str)
{
  *str = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseVName_() : NULL string cannot be converted to a variable name");
    return(NO);
  }
  
  if (!IsValidSymbolChar1(*arg)) {
    SetErrorf("ParseVName_() : '%s' not a valid variable name",arg);
    return(NO);
  }

  *str = arg;
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseVName(char *arg, char **i)
{
  if (ParseVName_(arg,NULL,i) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a string evaluated (no allocation)  
 *
 ***********************************************************************************/

char ParseStrValueLevel_(LEVEL level, char *arg, STRVALUE defVal, STRVALUE *sc)
{
  char *type;
  float resFlt;
  VALUE resVC;

  *sc = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseStrLevel_() : NULL string cannot be converted to a string");
    return(NO);
  }
  
  resVC = NULL;
  type = TTEvalExpressionLevel_(level,arg,&resFlt,&resVC,StringType,NO,YES,AnyType,YES);
  if (type == NULL) return(NO);
  *sc = (STRVALUE) resVC;
  
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseStrValueLevel(LEVEL level, char *arg, STRVALUE *sc)
{
  if (ParseStrValueLevel_(level,arg,NULL,sc) == NO) Errorf1("");
}


char ParseStrLevel_(LEVEL level, char *arg, char *defVal, char **str)
{
  char *type;
  float resFlt;
  VALUE resVC;

  *str = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseStrLevel_() : NULL cannot be converted to a string");
    return(NO);
  }
  
  resVC = NULL;
  type = TTEvalExpressionLevel_(level,arg,&resFlt,&resVC,StringType,NO,YES,AnyType,YES);  
  if (type == NULL) return(NO);

  *str = ((STRVALUE) resVC)->str;
  
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseStrLevel(LEVEL level, char *arg, char **i)
{
  if (ParseStrLevel_(level,arg,NULL,i) == NO) Errorf1("");
}

char ParseStr_(char *arg, char *defVal, char **str)
{
  return(ParseStrLevel_(levelCur, arg,defVal,str));
}

/* Same as above but generate an error (no default value) */
void ParseStr(char *arg, char **i)
{
  if (ParseStr_(arg,NULL,i) == NO) Errorf1("");
}


/*
 * Parsing a float or a string
 *
 */

char ParseFloatStrLevel_(LEVEL level,char *arg,float *f, char **str)
{
  char *type;
  VALUE val;

  if (arg == NULL) {
    SetErrorf("ParseFloatStrLevel_() : NULL cannot be converted to a float or string");
    return(NO);
  }
  
  type = TTEvalExpressionLevel_(level,arg,f,&val,StringType | FloatType,NO,NO,AnyType,YES);
  
  if (type == NULL) return(NO);
  
  if (type == strType) *str = ((STRVALUE) val)->str;
  else {
    *str = NULL;
    if (val != NULL) *f = ((NUMVALUE) val)->f;
  }
  return(YES);
}
 
 
/*****************************************************************
 *
 *
 *
 *  Parsing of a list (with special characters " and {})
 *
 *
 *
 *****************************************************************/
 
/* Maximum number of imbricated brackets or quotes in a list */
#define maxNImbricatedBrackets 200

/* 
 * 'theLine' starts with a { or " and it returns the corresponding } or "
 * It returns NULL if none 
 */
char * LookForBracketList(char *theLine, char *line)
{
  static char *brackets[maxNImbricatedBrackets];  
  int nBrackets;
  char flagStop;

  nBrackets = 0;
  brackets[nBrackets++] = line++;
  flagStop = NO;
  
  while (!flagStop && *line && nBrackets != 0) {
    switch(*line) {
      case '\\' : 
        if (*(++line) != '\0') line++; 
        break;
      case '{': 
        if (nBrackets >= maxNImbricatedBrackets) {
          SetErrorf("Too many imbricated brackets");
          return(NULL);
        }
        brackets[nBrackets++] = line++;
        break;      
      case '}':         
        if (*(brackets[nBrackets-1]) != '{') flagStop = YES;
        else {
          line++;
          nBrackets--;
        }
        break;
      case '[': 
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else brackets[nBrackets++] = line++;
        break;      
      case ']': 
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else if (*(brackets[nBrackets-1]) == '[') {
          line++;
          nBrackets--;
        }
        else flagStop = YES;
        break;
      case '(': 
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else brackets[nBrackets++] = line++;
        break;      
      case ')': 
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else if (*(brackets[nBrackets-1]) == '(') {
          line++;
          nBrackets--;
        }
        else flagStop = YES;
        break;
      case '`':
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) != '`') {
          if (nBrackets >= maxNImbricatedBrackets) {
            SetErrorf("Too many imbricated brackets");
            return(NULL);
          }
          brackets[nBrackets++] = line++;
        }
        else {
          line++;
          nBrackets--;
        }
        break;
      case '"':
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) != '"') {
          if (nBrackets >= maxNImbricatedBrackets) {
            SetErrorf("Too many imbricated brackets");
            return(NULL);
          }
          brackets[nBrackets++] = line++;
        }
        else {
          line++;
          nBrackets--;
        }
        break;
      case '\'':
        if (*(brackets[nBrackets-1]) == '{') {line++; continue;}
        if (*(brackets[nBrackets-1]) == '"') {line++;break;}
        if (*(brackets[nBrackets-1]) != '\'') {
          if (nBrackets >= maxNImbricatedBrackets) {
            SetErrorf("Too many imbricated brackets");
            return(NULL);
          }
          brackets[nBrackets++] = line++;
        }
        else {
          line++;
          nBrackets--;
        }
        break;
      default: line++;
    }
  }
  
  if (nBrackets != 0) {
    if (*(brackets[nBrackets-1]) == '`') SyntaxError("Missing the corresponding `",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '"') SyntaxError("Missing the corresponding \"",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '\'') SyntaxError("Missing the corresponding '",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '{') SyntaxError("Missing the corresponding }",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '[') SyntaxError("Missing the corresponding ]",theLine,brackets[nBrackets-1]);
    return(NULL);
  }
  else return(line-1);
}
 

/* 
 * Get the corresponding character to an escape sequence in a list
 */
static int GetCharFromEscapeList(char c)
{
  switch (c) {
    case '\\' : return('\\');
    case '{' : return('{');
    case '}' : return('}');
    case '[' : return('[');
    case ']' : return(']');
    case '"' : return('"');
    case '\'' : return('\'');
    case 't' : return('\t');
    case 'r' : return('\r');
    case 'n' : return('\n');
    default : return(-1);
  }
}

/* Just test that we have enough allocation */
#define TestAlloc(n) \
  if ((n)+nNewLine > MaxLengthList) { \
    SyntaxError("Word is too long",theLine,line); \
    return(-1); \
  }

/* Just parses one character */
#define Parse1Char(parseStr) { \
   nNewLine++; \
   TestAlloc(0); \
   *newLine = *(parseStr); \
   newLine++; \
   parseStr++;}


/*
 *
 *  The Main function for parsing a list
 *
 *    'theLine' = the total line (used in case of error)
 *
 *  returns arrays of the begining/end of words an in the variables beg and end
 *  and it returns the number of words or -1 if an error occured
 *
 */

int ParseListBegEnd(char *theLine, char ***beg, char ***end)
{
  static char *theBegWords[MaxNumWords+1];
  static char *theEndWords[MaxNumWords+1];
  static char theNewLine[MaxLengthList+1],*newLine;

  int nWord;
  char *line,*bracket,*lineError,*line1,*braceBeg,*braceEnd;
  int nNewLine;
  char *str;
  char flagEscape,flagCreateWord,flagStop;
  int i;

  /* If line is NULL then error */
  if (theLine == NULL) return(-1);

  /* Set the result variables */    
  if (beg != NULL && end != NULL) {
    *beg = theBegWords;
    *end = theEndWords;
  }

  /* 
   * Some inits concerning the line 
   */
  line = theLine;
  while (*line == ' ' || *line == '\t' || (*line == '\\' && (line[1] == '\n' || line[1] == '\r'))) {
    if (*line == '\\') line+=2;
    else  line++;
  }

  /* If empty line */
  if (*line == '\0') {
    theBegWords[0] = theEndWords[0] = NULL;
    return(0);
  }
  
  /* If {} surrounds the list we should set a flag */
  braceBeg = NULL;
  if (*line == '{') {
    braceEnd = LookForBracketList(theLine,line);
    if (braceEnd == NULL) return(-1);
    line1 = braceEnd+1;
    while (*line1 == ' ' || *line1 == '\t' || (*line1 == '\\' && (line1[1] == '\n' || line1[1] == '\r'))) line1++;
    if (*line1 != '\0') {braceBeg = NULL; braceEnd= NULL;}
    else {
      braceBeg = line;
      line1 = line+1;
      while (*line1 == ' ' || *line1 == '\t' || (*line1 == '\\' && (line1[1] == '\n' || line1[1] == '\r'))) {
        if (*line1 == '\\') line1+=2;
        else line1++;
      }
      if (line1== braceEnd) {
        theBegWords[0] = theEndWords[0] = NULL;
        return(0);
      }
    }
  } 

  /* 
   * Some inits concerning the processed line 
   */
  *theNewLine = '\0';
  newLine = theNewLine;
  nNewLine = 0;

  /*
   * Other inits 
   */
  nWord = 0;
  flagEscape = 0;  
    
  flagStop = NO; 
  flagCreateWord = NO;
  
  theBegWords[nWord] = NULL;
  theEndWords[nWord] = NULL;


  
  /*
   * Let's loop on the characters 
   */
  while(1) {

    /* 
     * If we were asked to create a new word just do it 
     */
    if (flagCreateWord) {
    
      /* Let's end the word */       
      theEndWords[nWord] = (nNewLine != 0 ? newLine-1 : newLine);
      nWord++;
            
      /* And init the next one */
      theBegWords[nWord] = theEndWords[nWord] = NULL;

      /* Skip spaces and tabs or escaped new lines */
      while (*line == ' ' || *line == '\t' || (*line == '\\' && (line[1] == '\n' || line[1] == '\r'))) {
        if (*line == '\\') line+=2;
        else  line++;
      }
      
      flagCreateWord = NO;
      
    }

    /*
     * Case we must exit the loop : 
     *   - end of the line
     */
    if (*line == '\0') {
  
      /* We expected an escaped characters */
      if (flagEscape) {
        SyntaxError("You cannot escape a null character",theLine,line);
        return(-1);
      }
    
      /* We must end the last word if necessary */
      if (theBegWords[nWord] != NULL) {
        theEndWords[nWord] = newLine-1;
        nWord++;
      }
      
      break;
    }

    /*
     * We must init the new word if any 
     */
    if (theBegWords[nWord]==NULL) {
    
      if (nWord > MaxNumWords-1) {
        SetErrorf("List is too long");
        return(-1);
      }
    
      theBegWords[nWord] = newLine;
      theEndWords[nWord] = NULL;
      
    }


    /* 
     * 'lineError' points toward the current character ('line' will move in the switch).
     * It will be used in case of errors.
     */
    lineError = line;
      
    switch(*line) {
          
       /************************* {} [] Signs ********************************/                  
       case '{' : case '[' :
       
         /* Is character escaped ? */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* If surrounding brace --> just skip it */
         if (line == braceBeg && *line != '[') {
           line++;
           break;
         }
 
         /* We just need to look for the matching '}' or  and copy all the characters in between */
         bracket = LookForBracketList(theLine,line);
         if (bracket == NULL) {
           if (*line == '[') {
             Parse1Char(line);
             break;
           }
           else return(-1);
         }
               
         /* We just copy all the characters */
         i = bracket-line+1;
         TestAlloc(i);
         strncpy(newLine,line,i);
         nNewLine += i;
         newLine += i;
         line = bracket+1;
         break;

       case '}' : 
         /* Is character escaped ? */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }
         
         /* If surrounding brace --> just skip it */
         else if (line == braceEnd) {
           line++;
           break;
         }

         SyntaxError("Missing the corresponding {",theLine,lineError);
         return(-1);

       case ']' : 
         /* Is character escaped ? */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }
         SyntaxError("Missing the corresponding [",theLine,lineError);
         return(-1);

 
       /************************* ` ********************************/                         
       case '`' :
         /* Is character escaped ? */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* We just need to look for the matching ` or and copy all the characters in between BUT NOT THE ` */
         bracket = LookForBracketList(theLine,line);
         if (bracket == NULL) return(-1);
         line++;
         bracket--;
       
         /* We just copy all the characters and replace escaped characters */
         i = bracket-line+1;
         TestAlloc(i);
         while (1) {
           str = strchr(line,'\\');
           if (str == NULL) {
             i = bracket-line+1;
             strncpy(newLine,line,i);
             nNewLine += i; line += i; newLine += i;
             line++;
             break;
           }
           if (str != line) {
             i = str-line;
             strncpy(newLine,line,i);
             nNewLine += i; line += i; newLine += i;
           }
           line++;
           i = GetCharFromEscapeList(*line);
           if (i == -1) {
             SyntaxError("This character cannot be escaped",theLine,line);
             return(-1);
           }
           *newLine = (char) i;
           nNewLine += 1; line += 1; newLine += 1;
         }
         break;

      /************************* " and ' and () ********************************/                         
       case '"' : case '\'' : case '(' :
         /* Is character escaped ? */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* We just need to look for the matching " or ' or and copy all the characters in between */
         bracket = LookForBracketList(theLine,line);
         if (bracket == NULL) return(-1);
       
         /* We just copy all the characters */
         i = bracket-line+1;
         TestAlloc(i);
         strncpy(newLine,line,i);
         nNewLine += i;
         newLine += i;
         line = bracket+1;
         break;

       case ')' : 

         /* If escaped --> just the ) */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* Otherwise --> error */
         else {
           SyntaxError("Missing the corresponding (",theLine,lineError);
           return(-1);
         }

       /************************* Word separators ********************************/                                     
       case '\n': case '\r' : /* These are word separators */
         if (flagEscape) {
           flagEscape = 0;
           line++;
           break;
         }       
       
       case ' ': case '\t' : 
         if (flagEscape) {
            SyntaxError("A space or a tabulation character cannot be escaped",theLine,lineError);
            return(-1);
         }

         /* let's end the word */       
         flagCreateWord = YES;
         
         break;

      /************************* \ sign ********************************/              
      case '\\':
        /* Is character escaped ? */
        if (flagEscape) {
          Parse1Char(line);
          flagEscape = 0;
          break;
        }

        /*
         * The escape character 
         */
        flagEscape = 1;
        line++;
        
        break;
          
      /************************* Everything else ******************************/              
      default :
        if (flagEscape) {
          i = GetCharFromEscapeList(*line);
          if (i == -1) {
            SyntaxError("This character cannot be escaped",theLine,lineError+1);
            return(-1);
          }
          else {
            Parse1Char(line);
            *(newLine-1) = i;
          }
          flagEscape = 0;
        }
        else  {
          Parse1Char(line);
        }
        break;
    }
  }
     
  theBegWords[nWord] = theEndWords[nWord] = NULL;
  
  return(nWord);
} 

/* The function for Parsing a list (no subtstitution but no error generated) */
char ParseWordList_(char *theLine, char **defList, char ***list)
{
  int n;
  char **beg,**end;
  
  *list = defList;
  if (theLine == NULL) {
    SetErrorf("ParseWordList_() : NULL string cannot be converted to a wordlist");
    return(NO);
  }

  n = ParseListBegEnd(theLine,&beg,&end);
  if (n == -1) {
    *list = defList;
    return(NO);
  }
  
  *list = BegEndStr2List(beg,end);

  TempList(*list);
  return(YES);
}

/* The function for Parsing a list (no subtstitution and errors generated) */
void ParseWordList(char *theLine, char ***list)
{
  int n;
  
  n = ParseWordList_(theLine,NULL,list);
  if (n==NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a list evaluated (no allocation)  
 *
 ***********************************************************************************/

char ParseList_(char *arg, char **defVal, char ***list)
{
  char *type;
  float resFlt;
  VALUE resVC;
  
  *list = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseList_() : NULL string cannot be converted to a list");
    return(NO);
  }

  resVC = NULL;
  type = TTEvalExpressionLevel_(levelCur,arg,&resFlt,&resVC,StringType,NO,YES,AnyType,YES);
  if (type == NULL) return(NO);
  
  *list = GetListFromStrValue((STRVALUE) resVC);
  if (*list == NULL) {
    *list = defVal;
    SetErrorf("ParseList_() : Cannot read a consistent list (there might be extra '{' or '}')");
    return(NO);
  }
    
  return(YES);
}

/* Same as above but generate an error (no default value) */
void ParseList(char *arg, char ***list)
{
  if (ParseList_(arg,NULL,list) == NO) Errorf1("");
}

/***********************************************************************************
 *
 *  Parsing a listv 
 *
 ***********************************************************************************/

char ParseListvLevel_(LEVEL level, char *arg, LISTV defVal, LISTV *lv)
{
  char *type;
  float resFlt;
  VALUE resVC;
  
  if (arg == NULL) {
    *lv = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    SetErrorf("ParseListv_() : NULL string cannot be converted to a listv");
    return(NO);
  }

  resVC = NULL;
  type = TTEvalExpressionLevel_(level,arg,&resFlt,&resVC,ListvType,NO,YES,AnyType,YES);
  
  if (type == NULL) {
    *lv = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    return(NO);
  }
  
  *lv = (LISTV) resVC;
 
  return(YES);
}

char ParseListv_(char *arg, LISTV defVal, LISTV *lv)
{
  return(ParseListvLevel_(levelCur,arg,defVal,lv));
}

/* Same as above but generate an error (no default value) */
void ParseListvLevel(LEVEL level, char *arg, LISTV *lv)
{
  if (ParseListvLevel_(level, arg,NULL,lv) == NO) Errorf1("");
}

void ParseListv(char *arg, LISTV *lv)
{
  if (ParseListv_(arg,NULL,lv) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a range 
 *
 ***********************************************************************************/

char ParseRangeLevel_(LEVEL level, char *arg, RANGE defVal, RANGE *rg)
{
  char *type;
  float resFlt;
  VALUE resVC;
  
  if (arg == NULL) {
    *rg = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    SetErrorf("ParseRangeLevel_() : NULL string cannot be converted to a range");
    return(NO);
  }

  resVC = NULL;
  type = TTEvalExpressionLevel_(level,arg,&resFlt,&resVC,RangeType,NO,YES,AnyType,YES);
  
  if (type == NULL) {
    *rg = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    return(NO);
  }
  
  *rg = (RANGE) resVC;
 
  return(YES);
}

char ParseRange_(char *arg, RANGE defVal, RANGE *rg)
{
  return(ParseRangeLevel_(levelCur,arg,defVal,rg));
}

/* Same as above but generate an error (no default value) */
void ParseRangeLevel(LEVEL level, char *arg, RANGE *rg)
{
  if (ParseRangeLevel_(level, arg,NULL,rg) == NO) Errorf1("");
}

void ParseRange(char *arg, RANGE *rg)
{
  if (ParseRange_(arg,NULL,rg) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a listv or a list
 *
 ***********************************************************************************/

char ParseListOrListv_(char *arg, char ***list, LISTV *lv)
{
  char *type;
  float resFlt;
  VALUE resVC;
  
  if (arg == NULL) {
    SetErrorf("ParseListOrListv_() : NULL string cannot be converted to a listv");
    return(NO);
  }

  resVC = NULL;
  type = TTEvalExpressionLevel_(levelCur,arg,&resFlt,&resVC,ListvType | StringType,NO,YES,AnyType,YES);
  
  if (type == NULL) return(NO);

  if (type != listvType  && type != strType) Errorf("ParseListOrListv_() : Weird error");

  if (type == listvType) {  
    *lv = (LISTV) resVC;
    *list = NULL;
  }
  else {
    *list = GetListFromStrValue((STRVALUE) resVC);
    *lv = NULL;
  }

  return(YES);
}


/***********************************************************************************
 *
 *  Parsing a proc 
 *
 ***********************************************************************************/

char ParseProcLevel_(LEVEL level, char *arg, PROC defVal, PROC *p)
{
  char *type;
  float resFlt;
  VALUE resVC;
  
  if (arg == NULL) {
    *p = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    SetErrorf("ParseProcLevel_() : NULL string cannot be converted to a proc");
    return(NO);
  }

  resVC = NULL;
  type = TTEvalExpressionLevel_(level,arg,&resFlt,&resVC,OtherType,NO,YES,AnyType,YES);
  
  if (type == NULL) {
    *p = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    return(NO);
  }
  
  if (type != procType) {
    *p = defVal;
    if (defVal != NULL) {
      AddRefValue( defVal);
      TempValue( defVal);
    }
    SetErrorf("ParseProcLevel_() : expect a '&proc' type (instead of '%s')",type);
    return(NO);
  }
  
  *p = (PROC) resVC;
 
  return(YES);
}

char ParseProc_(char *arg, PROC defVal, PROC *p)
{
  return(ParseProcLevel_(levelCur,arg,defVal,p));
}

/* Same as above but generate an error (no default value) */
void ParseProcLevel(LEVEL level, char *arg, PROC *p)
{
  if (ParseProcLevel_(level, arg,NULL,p) == NO) Errorf1("");
}

void ParseProc(char *arg, PROC *p)
{
  if (ParseProc_(arg,NULL,p) == NO) Errorf1("");
}


/*****************************************************************
 *
 *
 *
 *  Parsing of a script
 *
 *
 *
 *****************************************************************/


/*
 * Function for skipping newlines spaces and comments at the begining of a script
 * or after each new scriptline.
 */
 
static void SkipComments (char **pscript)  
{
  while (1) {
  
    while (**pscript == ' ' || **pscript == '\t' || **pscript == '\n' || **pscript == '\r' || 
           (**pscript == '\\' && ((*pscript)[1] == '\n' || (*pscript)[1] == '\r'))) (*pscript)++;
    
    if (**pscript == '#') {
      (*pscript)++;
      while (1) {
        if (**pscript == '\\') (*pscript) += 2;
        else if (**pscript != '\n' && **pscript != '\r' && **pscript != '\0') (*pscript)++;
        else break;
      }
    }
    else break;
    
    if (**pscript != '\0') (*pscript)++;
    else break;
    
  }
}


/* 
 * 'theLine' starts with a { or " and it returns the corresponding } or "
 * It returns NULL if none 
 */
char * LookForBracketScript(char *theLine, char *line)
{
  static char *brackets[maxNImbricatedBrackets];  
  int nBrackets;
  char flagStop;
  
  nBrackets = 0;
  brackets[nBrackets++] = line++;
  flagStop = NO;
  
  while (!flagStop && *line && nBrackets != 0) {
    switch(*line) {
      case '\\' : 
        if (*(++line) != '\0') line++; 
        break;
      case '{': 
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (nBrackets >= maxNImbricatedBrackets) {
          SetErrorf("Too many imbricated brackets");
          return(NULL);
        }
        brackets[nBrackets++] = line++;
        break;      
      case '}':         
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) != '{') flagStop = YES;
        else {
          line++;
          nBrackets--;
        }
        break;
      case '[': 
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else brackets[nBrackets++] = line++;
        break;      
      case ']': 
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else if (*(brackets[nBrackets-1]) == '[') {
          line++;
          nBrackets--;
        }
        else flagStop = YES;
        break;
      case '(': 
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else brackets[nBrackets++] = line++;
        break;      
      case ')': 
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) == '{') line++;
        else if (*(brackets[nBrackets-1]) == '(') {
          line++;
          nBrackets--;
        }
        else flagStop = YES;
        break;
      case '`':
        if (*(brackets[nBrackets-1]) == '"' || *(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) != '`') {
          if (nBrackets >= maxNImbricatedBrackets) {
            SetErrorf("Too many imbricated brackets");
            return(NULL);
          }
          brackets[nBrackets++] = line++;
        }
        else {
          line++;
          nBrackets--;
        }
        break;
      case '"':
        if (*(brackets[nBrackets-1]) == '\'') {line++;break;}
        if (*(brackets[nBrackets-1]) != '"') {
          if (nBrackets >= maxNImbricatedBrackets) {
            SetErrorf("Too many imbricated brackets");
            return(NULL);
          }
          brackets[nBrackets++] = line++;
        }
        else {
          line++;
          nBrackets--;
        }
        break;
      case '\'':
        if (*(brackets[nBrackets-1]) == '"') {line++;break;}
        if (*(brackets[nBrackets-1]) != '\'') {
          if (nBrackets >= maxNImbricatedBrackets) {
            SetErrorf("Too many imbricated brackets");
            return(NULL);
          }
          brackets[nBrackets++] = line++;
        }
        else {
          line++;
          nBrackets--;
        }
        break;
      case '\n' : case '\r' :
        line++;
        while (*line == ' ' || *line == '\t') line++;
        if (*line == '#') SkipComments(&line);
        break;
        
      default: line++;
    }
  }
  
  if (nBrackets != 0) {
    if (*(brackets[nBrackets-1]) == '`') SyntaxError("Missing the corresponding `",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '(') SyntaxError("Missing the corresponding )",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '"') SyntaxError("Missing the corresponding \"",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '\'') SyntaxError("Missing the corresponding '",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '{') SyntaxError("Missing the corresponding }",theLine,brackets[nBrackets-1]);
    else if (*(brackets[nBrackets-1]) == '[') SyntaxError("Missing the corresponding ]",theLine,brackets[nBrackets-1]);
    return(NULL);
  }
  else return(line-1);
}
 

/* 
 * Get the corresponding character to an escape sequence in a list
 */
int GetCharFromEscapeScript(char c)
{
  switch (c) {
    case '#' : return('#');
    case 't' : return('\t');
    case 'r' : return('\r');
    case 'n' : return('\n');
    default : return(-1);
  }
}

  

/*
 *
 *  The Main function for parsing a scriptline (No allocation is made)
 *
 *    'theLine'          = the total line (used in case of error)
 *    'beg' and 'end'    = will be filled wit the begining/end of each found words
 *                         both array are NULL terminated
 *    'endScriptLine'    = will be pointing towards the end of the detected script line  
 *    'endScriptLine1'   = will be pointing towards the end of the last word of the script line            
 *    'ptrs' ('nPtrs')   = array (its size) pointing towards either each $ char that were found
 *                         (in case of $[], one pointer is pointing towards the $ and the next towards the the ])
 *                         or if none were found towards the [] 
 *                         (in which case  one pointer is pointing towards the [ and the next towards the the ])
 *    'flagEndIsBracket' = if YES, it means that the script line is expected to end with a ]
 *    'flagSetv'         = if YES, it means that we dealt with a 'i=3' type of command (third word is the expression, second word is the '=' or '+=' ... sign)
 *    'redirectWord'     = if not -1 it is the word number (first is index 0) the redirection :: starts at
 *    'flagSubst'        = if NO then no substitution will be performed
 *
 *    It returns the number of words found or -1 if the script is not complete and -2 if there is an error
 *
 */
 #define MaxNPtrs 30
 
int ParseScriptLineBegEnd(char *theLine, char ***beg, char ***end, char **endScriptLine, char **endScriptLine1, char ***ptrs, int *nPtrs, char flagEndIsBracket, char *braceEnd,char *flagSetv, int *redirectWord, char flagSubst)
{
  static char *theBegWords[MaxNumWords+1];
  static char *theEndWords[MaxNumWords+1];
  static char *thePtrs[MaxNPtrs+1];
  static char theNewLine[MaxLengthList+1],*newLine;
  static char *theEndScriptLine;
  static char *bracketExt[maxNImbricatedBrackets];
  static char *par[maxNImbricatedBrackets];
  
  int nWord,nPtr,theRedirectWord;
  char *line,*bracket,*lineError,flagStop,*bquote,flagRedirect,*redirectChar,*quote,*start;
  
  int nPar;
  int nBracketExt;
  int nNewLine;
  char flagEscape;
  int i;
  char flagDollars;
  char flagSetv1;
  char flagCreateWord;

  /* If line is NULL then error */
  if (theLine == NULL) return(-2);
      
  /* Set the result variables */    
  if (beg != NULL && end != NULL) {
    *beg = theBegWords;
    *end = theEndWords;
  }
  if (ptrs != NULL) *ptrs = thePtrs;

  /* 
   * Some inits concerning the line 
   */
  line = theLine;
  while (*line == ' ' || *line == '\n' || *line == '\t' || *line == '\r' || (*line == '\\' && (line[1] == '\n' || line[1] == '\r'))) {
    if (*line == '\\') line+=2;
    else line++;
  }
  start = line;
  
  if (endScriptLine != NULL) *endScriptLine1 = *endScriptLine = line;
  
  /* If empty line */
  if (*line == '\0' || (*line == ']' && flagEndIsBracket) || (line == braceEnd)) {
    theBegWords[0] = theEndWords[0] = NULL;
    *nPtrs = 0;
    if (endScriptLine != NULL) *endScriptLine1 = *endScriptLine = line;
    return(0);
  }
    

  /* 
   * Some inits concerning the processed line 
   */
  *theNewLine = '\0';
  newLine = theNewLine;
  nNewLine = 0;

  
  /*
   * Other inits 
   */
  nWord = nPtr = 0;
  flagEscape = 0;
  flagDollars = NO;
  flagSetv1 = NO;
  theRedirectWord = -1;

  nPar = 0;
  nBracketExt = 0;
  flagStop = NO;  
  bquote = NULL;
  quote = NULL;  
  flagRedirect = NO;
  redirectChar = NULL;
  flagCreateWord = NO;
  
  theBegWords[nWord] = NULL;
  theEndWords[nWord] = NULL;
  
  
  /*
   * Let's loop on the characters 
   */
  while(1) {

    if (braceEnd != *endScriptLine1) *endScriptLine1 = line;
 
    /* 
     * If we were asked to create a new word just do it 
     */
    if (flagCreateWord) {
    
      /* Let's end the word */       
      theEndWords[nWord] = (nNewLine != 0 ? newLine-1 : newLine);
      nWord++;
         
      /* And init the next one */
      theBegWords[nWord] = theEndWords[nWord] = NULL;

      
      /* Skip spaces and tabs or escaped new lines */
      while (*line == ' ' || *line == '\t' || (*line == '\\' && (line[1] == '\n' || line[1] == '\r'))) {
        if (*line == '\\') line+=2;
        else  line++;
      }
         
      /* If there is a new line --> we are done */
      if (*line == '\n' || *line == '\r') {
        flagStop = YES;
      }
 
      /* If there is a ;; */
      else if (*line == ';' && line[1] == ';') {
        line+=2;
        flagStop = YES;
      }
      
      if (flagRedirect) flagRedirect--;
      
      flagCreateWord = NO;
    }
     
    /*
     * Case we must exit the loop : 
     *   - end of the line or 
     *   - flagStop 
     */
    if (*line == '\0'  || flagStop) {
  
      /* Do we expect to have stream names specified ? */
      if (flagRedirect == 2) {
        SyntaxError("You need to specify the stream names for redirection",theLine,redirectChar);
        return(-2);
      }
      
      /* We expected an escaped characters */
      if (flagEscape) {
        SyntaxError("You cannot escape a null character",theLine,line);
        return(-1);
      }
    
      /* We must end the last word if necessary */
      if (theBegWords[nWord] != NULL) {
        theEndWords[nWord] = newLine-1;
        nWord++;
      }
      
      /* Skip (escaped) newlines, tabs and spaces */
      while (1) {
        if (*line == ' ' || *line == '\n' || *line == '\t' || *line == '\r') line++;
        else if (*line == '\\' && (*line == '\n' || *line == '\t')) line+=2;
        break;
      }
      theEndScriptLine = line;

      break;
    }


    /*
     * We must init the new word if any 
     */
    if (theBegWords[nWord]==NULL) {
    
      if (nWord > MaxNumWords-1) {
        SetErrorf("Script line is too long");
        return(-2);
      }
    
      theBegWords[nWord] = newLine;
      theEndWords[nWord] = NULL;
      
    }


    /* 
     * 'lineError' points toward the current character ('line' will move in the switch).
     * It will be used in case of errors.
     */
    lineError = line;
    
    
    /*
     *
     * Main switch
     *
     */   
    switch(*line) {
       
       
       /***************************************   
        *         [] Signs 
        ***************************************/
        
       case '[' :
       
         /* If escaped or in between quotes and no dollars --> just the bracket */
         if (flagEscape || (quote && (line == theLine || *(line-1) != '$'))) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }
         
         /* We look for the matching ']' */
         bracket = LookForBracketScript(theLine,line);
         if (bracket == NULL) return(-1);

         /* 
          * if the character before is a valid variable character then it means that this is an extraction type of []
          */
         if (line != theLine && IsValidSymbolChar(*(line-1))) {
           
           /* We parse the [ */
           Parse1Char(line);
        
           /* We set the matching bracket */
           bracketExt[nBracketExt] = bracket;
           nBracketExt++;
           break;
         }
         
         /*
          * Otherwise it is a script type of [] 
          */
       
         /* We just copy all the characters in between the [] */
         i = bracket-line+1;
         TestAlloc(i);
         strncpy(newLine,line,i);
         nNewLine += i;
         newLine += i;

         /* Should we remember the position of the brackets ? */
         if (flagDollars == NO) { /* Case of a regular [] */
           if (nPtr >= MaxNPtrs-2) Errorf("ParseScriptLineBegEnd() : Too many '[...]' in script line");
           thePtrs[nPtr] = newLine-i; nPtr++;
           thePtrs[nPtr] = newLine-1; nPtr++;
         }
         else if (line != theLine && *(line-1) == '$' && flagSubst) { /* Case of a $[] -> the opening brcket has already been stored */
           if (nPtr >= MaxNPtrs-1) Errorf("ParseScriptLineBegEnd() : Too many '$' in script line");
           thePtrs[nPtr] = newLine-1; nPtr++;
         }

         /* Jump to the character right after the closing ] */         
         line = bracket+1;
         
         break;

       case ']' :

         /* If escaped or quoted --> just the bracket */
         if (flagEscape || quote) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* If it is the closing bracketExt --> just parse it */
         if (nBracketExt != 0 && bracketExt[nBracketExt-1] == line) {
           Parse1Char(line);
           nBracketExt--;
           break;
         }
         
         /* Maybe it is the endBracket */
         if (flagEndIsBracket) {flagStop = YES; continue;}
         
         /* Otherwise --> error */
         SyntaxError("Missing the corresponding [",theLine,lineError);
         return(-1);


       /***************************************   
        *         {} Signs 
        ***************************************/
        
       case '{' : 
       
         /* If escaped or quote --> just the brace */
         if (flagEscape || quote) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }
          
         /* We look for the matching '}' */
         bracket = LookForBracketScript(theLine,line);
         if (bracket == NULL) return(-1);
          
         /* We just copy all the characters in between */
         i = bracket-line+1;
         TestAlloc(i);
         strncpy(newLine,line,i);
         nNewLine += i;
         newLine += i;
         line = bracket+1;
         
         break;

       case '}' : 

         /* If escaped --> just the brace */
         if (flagEscape || quote) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }
 
         /* If surrounding brace --> just skip it */
         else if (line == braceEnd) {
           line++;
           break;
         }
        
         /* Otherwise --> error */
         else {
           SyntaxError("Missing the corresponding {",theLine,lineError);
           return(-1);
         }

         break;
         
         
       /***************************************   
        *         ` Sign 
        ***************************************/
        
       case '`' : 
        
         /* If escaped OR if within quote --> just the ( */
         if (flagEscape || quote) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* If it is the closing bquote --> just skip it */
         if (bquote == line) {
           line++;
           bquote = NULL;
           break;
         }
         
         /* Otherwise we just set the matching bquote */
         bracket = LookForBracketScript(theLine,line);
         if (bracket == NULL) return(-1);
         bquote = bracket;
         line++;
         break;


       /***************************************   
        *         () Sign 
        ***************************************/
        
       case '(' : 

         /* We parse it */
         Parse1Char(line);
        
         /* If escaped OR if within quote --> just the ( */
         if (flagEscape || quote) {
           flagEscape = 0;
           break;
         }
         
         /* Otherwise we just set the matching parenthesis */
         bracket = LookForBracketScript(theLine,line-1);
         if (bracket == NULL) return(-1);
         par[nPar] = bracket;
         nPar++;
         break;

       case ')' : 

         /* If escaped OR if within quote --> just the ) */
         if (flagEscape || quote) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* If it is the closing par --> just parse it */
         if (nPar != 0 && par[nPar-1] == line) {
           Parse1Char(line);
           nPar--;
           break;
         }

         /* Otherwise --> error */
         else {
           SyntaxError("Missing the corresponding (",theLine,lineError);
           return(-1);
         }
         
         break;


       /***************************************   
        *         " or ' Signs
        ***************************************/
        
       case '"' : 
       case '\'' : 
        
         /* If escaped --> just the quote */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* If it is the closing quote --> just the quote and close it */
         if (quote == line) {
           Parse1Char(line);
           quote = NULL;
           break;
         }
         
         /* if within a different quote --> just the quote */
         if (quote != NULL) {
           Parse1Char(line);
           break;
         }
         
         /* Otherwise we just set the matching quote */
         bracket = LookForBracketScript(theLine,line);
         if (bracket == NULL) return(-1);
         quote = bracket;
         Parse1Char(line);
         break;


       /***************************************   
        *         $ Sign 
        ***************************************/

       case '$' : 

         /* If escaped OR or no substitution --> just the dollar */
         if (flagEscape || !flagSubst) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }
         
         /* Set the dollar mode if not set */
         if (flagDollars == NO) {
           flagDollars = YES;
           nPtr = 0;
         }
         
         /* Store the new dollar sign */
         if (nPtr >= MaxNPtrs-1) Errorf("ParseScriptLineBegEnd() : Too many '$' signs in script line");
         thePtrs[nPtr] = newLine; nPtr++;
         *newLine = '$';
         nNewLine += 1; line += 1; newLine += 1;

         break;
        


       /***************************************   
        * Word/line separators : '\n', '\r', ' ', '\t' 
        ***************************************/
        
       case '\n': case '\r' :
        
        /* If escaped --> just skip it */
         if (flagEscape) {
           flagEscape = 0;
           line++;
           break;
         }       

         /* If within (b)quotes or parenthesis --> just the character  */
         if (nBracketExt != 0 || nPar!=0 || bquote || quote) {
           Parse1Char(line);
           break;
         }
       
         /* Otherwise, let's end the word */       
         flagCreateWord = YES;
 
         break;
 
       case ' ': case '\t' : 

         /* the ' ' and '\t' caharacters cannot be escaped */
         if (flagEscape) {
            SyntaxError("A space or a tabulation character cannot be escaped",theLine,lineError);
            return(-2);
         }

         /* If within (b)quotes or parenthesis or setv --> just the character  */
         if (nBracketExt != 0 || nPar!=0 || bquote || quote || flagSetv1) {
           Parse1Char(line);
           break;
         }
       
         /* Otherwise, let's end the word */       
         flagCreateWord = YES;
         
         break;
        
       case ';':
       
         /* Case it is escaped */
         if (flagEscape || line[1] != ';') {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* If within (b)quotes or parenthesis --> just the character  */
         if (nBracketExt != 0 || nPar!=0 || bquote || quote) {
           Parse1Char(line);
           break;
         }

         flagCreateWord = YES;
         break;

       /***************************************   
        * \ character  
        ***************************************/
        
       case '\\':
       
         /* If escaped --> just the \ */
         if (flagEscape) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

        /* If within quotes --> just the \ and the next char */
         if (quote) {
           Parse1Char(line);
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* The escape character */
         flagEscape = 1;
         line++;
        
         break;


       /***************************************   
        * :: character  
        ***************************************/
        
       case ':':
       
         /* Case it is escaped or there is no '=' afterwards */
         if (flagEscape || line[1] != '=') {
         
           /* If escaped OR if no ':' after or if not a whole word then just the character */
           if (nBracketExt != 0 || nPar!=0 || quote || bquote || flagEscape || line == theLine || *(line-1) != ' ' || *(line+1) != ':' || (*(line+1) != '\0' && *(line+2) != ' ' && *(line+2) != '\0')) {
             Parse1Char(line);
             flagEscape = 0;
             break;
           }
         
           if (redirectChar != NULL) {
             SyntaxError("You cannot use the redirection symbols '::' twice",theLine,lineError);
             return(-2);
           }
         
           /* The redirect word number */
           redirectChar = line;
           theRedirectWord = nWord;
           Parse1Char(line);
           Parse1Char(line);
           flagRedirect = 2;
           continue;
         }
          


        /* If an '=' afterwards we just execute the following code */
        
       /***************************************   
        *         = Character 
        ***************************************/
        
       case '+' : case '-' : case '*' : case '/' : case '^' : case '%' :

         if (flagEscape) {
           i = GetCharFromEscapeScript(*line);
           if (i == -1) {
             SyntaxError("This character cannot be escaped",theLine,lineError+1);
             return(-2);
           }
           else {
             Parse1Char(line);
             *(newLine-1) = i;
           }
           flagEscape = 0;
         }

         /* 
          * If there is an '=' after the '+' and the '+' is the first character of the line or in the first word or at the begining of the second word
          * this is a setv command. Otherwise it is a regular character
          */
          
         if (line[1] != '=' || 
             line[0] == '/' && line[1] == '/' && line[2] != '=' ||
             nWord!=0 && nWord>=2 && (nWord!=1 || (theEndWords[0]-theBegWords[0]+1 != nNewLine && theEndWords[0]-theBegWords[0]+2 != nNewLine))) {
           Parse1Char(line);
           break;
         }
         
         /* We set the setv flag */
         flagSetv1 = YES;
         
         /* 
          * In case this is the begining of the 2nd word, we parse the 2 characters and create a world
          * If it is in the 1st word, we just ask to create a word and wait for the second pass in this code
          */
         if (nWord == 1) {
           if (line[0] == '/' && line[1] == '/') {Parse1Char(line);}
           Parse1Char(line); 
           Parse1Char(line);
         }
         flagCreateWord = YES;
         break;
                    
          
       case '=' :
       
         /* If escaped OR if within braces --> just the = */
         if (nBracketExt != 0 || flagEscape || nPar!=0 || quote ||  *(line+1) == '=' || (line != theLine && (*(line-1) == '=' || *(line-1) == '!' || *(line-1) == '<' || *(line-1) == '>'))) {
           Parse1Char(line);
           flagEscape = 0;
           break;
         }

         /* 
          * If the '=' is the first character of the line or in the first word or at the begining (or the right after the begining) of the second word
          * this is a setv command. Otherwise it is a regular character
          */
          
         if (nWord!=0 && nWord>=2 || (nWord==1 && (theEndWords[0]-theBegWords[0]+1 != nNewLine && theEndWords[0]-theBegWords[0]+2 != nNewLine))) {
           Parse1Char(line);
           break;
         }
 
         /* Error if the  word starts with a number ----> NON on peut faire 1+2= */
         /* if (isdigit(*(theBegWords[0])) || *(theBegWords[0]) == '-') {
           SyntaxError("Bad name for variable",theLine,start);
           return(-2);
         } *

         /* We set the setv flag */
         flagSetv1 = YES;
         
         /* 
          * In case this is the begining of the 2nd word, we parse the character and create a world
          * If it is in the 1st word, we just ask to create a word and wait for the second pass in this code
          */
         if (nWord == 1) {Parse1Char(line);}
         flagCreateWord = YES;
         
         break;
       


       /***************************************   
        * everything else  
        ***************************************/
 
       default :
 
         /* If escaped --> get the corresponding character */
         if (flagEscape) {
           i = GetCharFromEscapeScript(*line);
           if (i == -1) {
             SyntaxError("This character cannot be escaped",theLine,lineError+1);
             return(-2);
           }
           else {
             Parse1Char(line);
             *(newLine-1) = i;
           }
           flagEscape = 0;
         }
         
         /* Otherwise --> just the character */
         else  {
           Parse1Char(line);
         }
         break;
     }
      
  } /* end of main loop */
     
  /* Clean up everything */   
  theBegWords[nWord] = theEndWords[nWord] = NULL;
  if (nPtrs != NULL) *nPtrs = nPtr;
  if (endScriptLine != NULL) *endScriptLine = theEndScriptLine;
  if (flagSetv != NULL) *flagSetv = flagSetv1;
  if (redirectWord) *redirectWord = theRedirectWord;
  
  return(nWord);
}



/*
 *
 * Subroutine for parsing a script (there is no checking whether it has already been compiled or not)
 *
 *   'theLine'           = the line to be parsed
 *   'flagTemp'          = if YES the script will be allocated temporary
 *   'flagEndIsBracket' = if YES, it means that the script line is expected to end with a ]
 *   'flagBrace'         = YES then it will withdraw eventual braces around the script.
 *   'flagErr'          = if not NULL this is used to return 0 if the script is correct 1 if it is not complete
 *                         and 2 if an error occurred (in both last cases the script returned is NULL)

 *
 * It either returns the allocated script or NULL
 */

/* The maximum number of script lines within a script */ 
#define MAXNSCRIPTLINES 5000 

SCRIPT ParseScript__(char *theLine,char flagTemp, char flagEndIsBracket, char flagSubst, char flagBrace, char *flagErr)
{
  SCRIPTLINE sl[MAXNSCRIPTLINES];
  
  SCRIPT script;
  int nWords;
  char **beg,**end,*endScriptLine,**ptrs,*braceEnd,*endScriptLine1;
  int nsl,i,nPtrs;
  char c,flagError,flagSetv;
  int redirectWord;
  
  /* If no script is given --> error */
  if (theLine == NULL) Errorf("ParseScript__() : Weird");

  /* Skip the comments */
  SkipComments(&theLine);
  
  /* Deals with surrounding {} */
  braceEnd = NULL;
  if (flagBrace) {
    if (*theLine == '{') {
      braceEnd = LookForBracketScript(theLine,theLine);
      if (braceEnd == NULL) {
        if (flagErr) *flagErr = 2;
        return(NULL);
      }
      if (!(braceEnd[1] == ' ' || braceEnd[1] == '\t' || braceEnd[1] == '\0' || braceEnd[1] == '\n' || braceEnd[1] == '\r')) braceEnd= NULL;
      else {
        theLine++;
        SkipComments(&theLine);
      }
    }
  }

  /* If line is empty then return empty script */
  if (*theLine == '\0') {
    script = NewScript();
    if (flagTemp) TempValue(script);
    return(script);
  }

  /* The loop */
  nsl = 0;  
  flagError = NO;
  if (flagErr) *flagErr = 0;
  while(*theLine != '\0') {
  
    if (nsl >= MAXNSCRIPTLINES-1) {
      flagError = YES;
      SetErrorf("ParseScript__() : Script has too many lines (max is %d)",MAXNSCRIPTLINES);
      break;
    }
      
    /* We read a command */
    nWords = ParseScriptLineBegEnd(theLine, &beg, &end,&endScriptLine,&endScriptLine1,&ptrs,&nPtrs,flagEndIsBracket,braceEnd,&flagSetv,&redirectWord,flagSubst);
     
    /* If Error --> delete the former script lines */
    if (nWords < 0) {
      for (i=0;i<nsl;i++) DeleteScriptLine(sl[i]);
      if (flagErr) {
        if (nWords == -1) *flagErr = 1;
        else *flagErr = 2;
      }
      return(NULL);
    }

    /* Check that it is not empty */
    if (nWords == 1 && end[0] == beg[0] && (end[0][0] == '\0' || end[0][0] == '\n' || end[0][0] == '\r' || end[0][0] == '\t')) nWords = 0;
    
    /* We create the corresponding scriptline */
    if (nWords != 0) {      
      c = *endScriptLine1;
      *endScriptLine1 = '\0';
      sl[nsl] = MakeScriptLine(theLine,nWords,beg,end,nPtrs,ptrs,flagSetv,redirectWord,flagSubst);
      *endScriptLine1 = c;
      if (sl[nsl] == NULL) {
        flagError = YES;
        if (flagErr) *flagErr = 2;
        break;
      }
      nsl++;
    }

    if (flagEndIsBracket && *endScriptLine == ']') break;
    if (*endScriptLine == '}') break;
    
    theLine = endScriptLine;

    /* We skip the comments */
    SkipComments(&theLine);
  }
  
  /* Error */
  if (flagError) {
    for (i=0;i<nsl;i++) DeleteScriptLine(sl[i]);
    return(NULL);
  }

  /* Allocation */
  script = NewScript();
  script->nsl = nsl;
  if (nsl != 0) script->sl = Malloc(nsl*sizeof(SCRIPTLINE));
  else script->sl = NULL;
  for (i=0;i<nsl;i++) script->sl[i] = sl[i];
  
  /* Make it temporary if necessary */
  if (flagTemp) TempValue(script);

  /* return */  
  return(script);
}


/*
 *
 * Subroutine for parsing a script
 * If it has already been compiled we just return the compilation otherwise it compiles it and returns 
 *
 *   'theScript'         = A pointer to the the string that represents the script to be parsed
 *   'pScript'           = the result parsed script (or NULL)
 *   'flagBrace'         = YES then it will withdraw eventual braces around the script.
 *   'flagErr'           = 0 if everything went ok 1 if it is not a complete script and 2 if there is an error
 *
 * It either returns YES (script has been parsed succesfully) or NO (if not)
 */

extern COMPSTRUCT NewCompStruct (void);
extern void ClearCompStruct (COMPSTRUCT cs);

char ParseCompiledScript_(char **theScript, SCRIPT *pScript, char flagSubst, char flagBrace, char *flagErr)
{
  int n,nw;
  SCRIPTLINE sl;
  COMPSTRUCT cs;

  *flagErr = 0;
  
  /* Get the current script line */  
  sl = levelCur->scriptline;
  
  /* Is 'theScript' a word of the current scriptline ? */
  if (sl) {
    if (theScript >= sl->words && theScript < sl->words + sl->nWords) {
      n = theScript-sl->words;
    }
    else n = -1;
  }
  else n = -1;
  
  /* If it is not just do the regular processing */
  if (n == -1)  {
    *pScript = ParseScript__(*theScript,YES,NO,flagSubst,flagBrace, flagErr);
    if (*pScript == NULL) return(NO);
    else return(YES);
  } 

  /* If the script is already compiled ... it's easy */
  if (sl->cs != NULL && sl->cs[n] != NULL && sl->cs[n]->pos.dollar == NULL && sl->cs[n]->scripts.dollar != NULL) {
    *pScript = sl->cs[n]->scripts.dollar[0]; 
    (*pScript)->nRef++;
    TempValue(*pScript);
    return(YES);
  }
  
  /*
   * Otherwise we must compile it 
   */

  /* We first parse the script */
  *pScript = ParseScript__(*theScript,NO,NO,flagSubst,flagBrace,flagErr);
  if (*pScript == NULL) return(NO);
  
/*  Printf("compilation of %s\n",*theScript); */
    
  /* Perform allocation */
  if (sl->cs == NULL) {
    sl->cs = Malloc(sizeof(COMPSTRUCT)*sl->nWords);
    for (nw=0; nw<sl->nWords;nw++) sl->cs[nw] = NULL;
  }
  if (sl->cs[n] == NULL) sl->cs[n] = NewCompStruct();
  else  ClearCompStruct(sl->cs[n]);  

  cs = sl->cs[n];
  
  cs->scripts.dollar = Malloc(sizeof(SCRIPT)*2);
  cs->scripts.dollar[0] = *pScript;
  cs->scripts.dollar[1] = NULL;
  
  return(YES);
}


/* 
 * Basic function to parse a script.
 *
 * If it starts with a { or it contains a blank, then it assumes that it is a script string
 * Otherwise it looks for a variable of type script 
 * 
 */ 
char ParseScriptLevel_(LEVEL level, char **theScript, SCRIPT defScript,SCRIPT *pScript)
{
  char flagErr;
  VALUE val;

  *pScript = defScript;
  if (*theScript == NULL) {
    SetErrorf("ParseScriptLevel_() : NULL string cannot be converted to a script");
    if (*pScript != NULL) {
      AddRefValue(*pScript);
      TempValue(*pScript);
    }
    return(NO);
  }

  
  if (**theScript == '{' || **theScript == '\0') {
    if (ParseCompiledScript_(theScript,pScript,YES,YES,&flagErr)) return(YES);
    if (defScript == NULL) *pScript = NULL;
    else {
      *pScript = defScript;
      defScript->nRef++;
      TempValue( defScript);
    }
    return(NO);
  }

  if (ParseValLevel_(level,*theScript,(VALUE) defScript,&val) == NO) {
    if (defScript == NULL) *pScript = NULL;
    else {
      *pScript = defScript;
      defScript->nRef++;
      TempValue( defScript);
    }
    return(NO);
  }
    
  if (GetTypeValue(val) != scriptType) {
    SetErrorf("ParseScriptLevel_() : Value of '%s' is not of type '%s'",*theScript,scriptType);
    if (defScript == NULL) *pScript = NULL;
    else {
      *pScript = defScript;
      defScript->nRef++;
      TempValue( defScript);
    }
    return(NO);
  }
  
  *pScript = (SCRIPT) ValueOf(val);
  return(YES);
}

/* The function for Parsing a script (errors generated) */
void ParseScriptLevel(LEVEL level, char **theScript, SCRIPT *pScript)
{  
  if (ParseScriptLevel_(level,theScript,NULL,pScript) != YES) Errorf1("");
}

/* The function for Parsing a script */
char ParseScript_(char **theScript, SCRIPT defScript, SCRIPT *pScript)
{  
  return(ParseScriptLevel_(levelCur,theScript,defScript,pScript));
}

char ParseStrScript_(char **theScript, SCRIPT defScript, SCRIPT *pScript)
{
  char flagErr;

  if (ParseCompiledScript_(theScript,pScript,YES,YES,&flagErr)) return(YES);
  
  if (defScript == NULL) *pScript = NULL;
  else {
    *pScript = defScript;
    defScript->nRef++;
    TempValue( defScript);
  }
  return(NO);
}

void ParseStrScript(char **theScript, SCRIPT *pScript)
{
  char flagErr;

  if (ParseCompiledScript_(theScript,pScript,YES,YES,&flagErr)) return;
  Errorf1("");
}

/* The function for Parsing a script (errors generated) */
void ParseScript(char **theScript, SCRIPT *pScript)
{  
  if (ParseScript_(theScript,NULL,pScript) != YES) Errorf1("");
}

/* The function for Parsing a script (no substitution) ?????????? critere de decision pas terrible */
void ParseNoSubstScript(char **theScript, SCRIPT *pScript)
{  
  char flagErr;
  VALUE val;
  
  if (**theScript == '{' || strchr(*theScript,' ') || **theScript == '\0' || 
      ParseValLevel_(levelCur,*theScript,NULL,&val) == NO) {
    if (ParseCompiledScript_(theScript,pScript,NO,YES,&flagErr)) return;
    Errorf1("");
  }
  
  if (GetTypeValue(val) != scriptType) 
    Errorf("ParseScriptLevel_() : Expression '%s' is not of type '%s'",theScript,scriptType);
  
  *pScript = (SCRIPT) val;
}


/*
 * The function for Parsing a complete script
 *
 * It generates an error if an error is found in the script.
 * In case the script is found not to be complete, it does not generate an error
 * and simply returns NO
 * If everything went well it returns YES and the script is in 'pScript'
 * If flagBrace is YES then it will withdraw eventual braces around the script.
 */
char ParseCompleteScript(char **theScript, SCRIPT *pScript, char flagBrace)
{ 
  char flagErr;
  
  if (ParseCompiledScript_(theScript,pScript,YES,flagBrace,&flagErr)) return(YES);
  
  if (flagErr == 1) return(NO);
  
  Errorf1("");
}



/*******************************************
 *
 * Mechanism for adding new parsing types
 *
 *******************************************/

extern char *varTypeNames[];
extern void *varTypeParseFunctions[];
extern int varTypes[];
extern int nVarTypes;

/* Adding a new INT parsing type */
int AddParseTypeInt(char  (*function)(char *, int, int*))
{
  Warningf("AddParseTypeInt() : old function");
  return(10);
}

/* Adding a new float parsing type */
int AddParseTypeFloat(char  (*function)(char *, float, float*))
{
  Warningf("AddParseTypeFloat() : old function");
  return(10);
}

/* Adding a new CHAR parsing type */
int AddParseTypeChar( char  (*function)(char *, char, char*))
{
  Warningf("AddParseTypeChar() : old function");
  return(10);
}

/* Adding a new PTR parsing type */
int AddParseTypePtr(char  (*function)(char *, void *, void **))
{
  Warningf("AddParseTypePtr() : old function");
  return(10);
}


/***********************************************************************************
 *
 *  The Main function for Parsing arguments to a command
 *
 ***********************************************************************************/

char **ParseArgv(char **argv,...)
{
  va_list ap;
  int type,indexType,flag;
  void *pVar;
  char **list;
  int defInt;
  unsigned long defColor;
  double defDouble;
  char *defpChar,**defppChar,defChar;
  GOBJECT defGObject;
  GOBJECT* defGObjectList;
  WINDOW defWindow;
  SCRIPT defScript;
  PROC defProc;
  STREAM defStream;
  LEVEL defLevel;
  RANGE defRg;
  FONT defFont;
  char flagMoreArgs;
  char (*fint) (LEVEL,char *,int, int*);
  char (*fflt) (LEVEL,char *,float, float*);
  char (*fstr) (LEVEL,char *,char*, char**);
  char (*fvc) (LEVEL,char *,VALUE, VALUE *);
  VALUE defVal;
    
  va_start(ap,argv);
  
  list = argv;
  
  while(1) {

    InitError();
      
    /* read the pointer to the next type */
    type = va_arg(ap,int);
    if (type == END) {
      flagMoreArgs = NO;
      break;
    }
    else if (type == MORE) {
      flagMoreArgs = YES;
      break;
    }
    
    /* If the type is a not an original type we compute its index in the array parseTypeSizes */
    if (type >= tLAST) indexType = (type-tLAST)/2;
    
    /* Is it a valid type ? */
    if (type >= tLAST && indexType >= nVarTypes || type <= 0) 
      Errorf("ParseArgv() : Bad type '%d'. \nYou certainly forgot either to end the ParseArgv function with a 0 or a -1 or to specify the optional value of some optional field",type);     
     
    /*
     * Read default value for optional type
     */ 
    defpChar = NULL;
    defppChar = NULL; 
    defStream = NULL;
    defLevel = NULL;
    defWindow = NULL;
    defGObject = NULL; 
    defFont = NULL;
    defScript = NULL;
    defRg = NULL;
    defProc = NULL;
    defVal = NULL;
    if (type % 2 == 0) {
    
      /* Case of predefined types */
      toplevelCur->flagSaveError = NO;
      if (type < tLAST) {
         switch(type) {
          case tINT_: defInt = va_arg(ap,int); break;
          case tFLOAT_: defDouble = va_arg(ap,double); break;
          case tDOUBLE_: defDouble = va_arg(ap,double); break;
          case tVAL_: case tVALOBJ_: defVal = va_arg(ap,VALUE); break;
          case tLISTV_: defVal = va_arg(ap,VALUE); break;
          case tRANGE_: defRg = va_arg(ap,RANGE); break;
          case tSTR_: case tWORD_: defpChar = va_arg(ap,char *); break;
          case tCHAR_:  defChar = va_arg(ap,int); break; 
          case tVNAME_: defpChar = va_arg(ap,char *); break;
          case tSYMB_: defpChar = va_arg(ap,char *); break;
          case tSTREAM_: defStream = va_arg(ap,STREAM); break;
          case tLEVEL_: defLevel = va_arg(ap,LEVEL); break;
          case tCOLOR_: defColor = va_arg(ap,unsigned long); break;
          case tCOLORMAP_: defColor = va_arg(ap,unsigned long); break;
          case tLIST_: case tWORDLIST_: defppChar = va_arg(ap,char **); break;
          case tSCRIPT_: defScript = va_arg(ap,SCRIPT); break;
          case tPROC_: defProc = va_arg(ap,PROC); break;
          case tGOBJECT_: defGObject = va_arg(ap,GOBJECT); break;
          case tGOBJECTLIST_: defGObjectList = va_arg(ap,GOBJECT *); break;
          case tWINDOW_: defWindow = va_arg(ap,WINDOW); break;
          case tFONT_: defFont = va_arg(ap,FONT); break;
          default : toplevelCur->flagSaveError = YES;
        }
   
      }
      
      /* Case of newly defined types */    
      else {
        if (type % 2 == 0) toplevelCur->flagSaveError = YES;
        switch (varTypes[indexType]) {
          case tINT : defInt = va_arg(ap,int); break;
          case tFLOAT: defDouble = va_arg(ap,double); break;
          case tVAL:  defVal = va_arg(ap,VALUE); break; 
          case tSTR:  defpChar = va_arg(ap,char *); break;
        }
      }
      
    }
    else toplevelCur->flagSaveError = YES;
    
    /* read the variable to put the value in */
    pVar = va_arg(ap,void *);
    
    /* If no arguments are left and type is not optional then error */
    if (*list == NULL && type % 2 == 1) {
      toplevelCur->flagSaveError = YES;
      SetErrorf("Not enough arguments\n");
      ErrorUsage1();
    }
    
    
    /* 
     * Parse the list according to the type 
     */

    /* Case of predefined types */
    if (type < tLAST) {
     
      switch(type) {
    
        case tINT: ParseInt(*list,(int *) pVar); list++; break;
        case tINT_: if(ParseInt_(*list,defInt,(int *) pVar)) list++; break;

        case tFLOAT: ParseFloat(*list,(float *) pVar); list++; break;
        case tFLOAT_: if(ParseFloat_(*list,defDouble,(float *) pVar)) list++; break;

        case tDOUBLE: ParseDouble(*list,(double *) pVar); list++; break;
        case tDOUBLE_: if(ParseDouble_(*list,defDouble,(double *) pVar)) list++; break;

        case tVAL: ParseVal(*list,(VALUE *) pVar); list++; break;
        case tVAL_: if(ParseVal_(*list,defVal,(VALUE *) pVar)) list++; break;

        case tVALOBJ: ParseValObj(*list,(VALUE *) pVar); list++; break;
        case tVALOBJ_: if(ParseValObj_(*list,defVal,(VALUE *) pVar)) list++; break;

        case tLISTV: ParseListv(*list,(LISTV *) pVar); list++; break;
        case tLISTV_: if(ParseListv_(*list,(LISTV) defVal,(LISTV *) pVar)) list++; break;

        case tRANGE: ParseRange(*list,(RANGE *) pVar); list++; break;
        case tRANGE_: if(ParseRange_(*list,(RANGE) defVal,(RANGE *) pVar)) list++; break;

        case tCHAR: ParseChar(*list,(char *) pVar); list++; break;
        case tCHAR_: if(ParseChar_(*list,defChar,(char *) pVar)) list++; break;
    
        case tWORD: ParseWord(*list,(char **) pVar); list++; break;
        case tWORD_: if (ParseWord_(*list,defpChar,(char **) pVar)) list++; break;

        case tSTR: ParseStr(*list,(char **) pVar); list++; break;
        case tSTR_: if (ParseStr_(*list,defpChar,(char **) pVar)) list++; break;

        case tVNAME: ParseVName(*list,(char **) pVar); list++; break;
        case tVNAME_: if (ParseVName_(*list,defpChar,(char **) pVar)) list++; break;

        case tSYMB: ParseSymbol(*list,(char **) pVar); list++; break;
        case tSYMB_: if (ParseSymbol_(*list,defpChar,(char **) pVar)) list++; break;
    
        case tSTREAM: ParseStream(*list,(STREAM *) pVar); list++; break;
        case tSTREAM_: if(ParseStream_(*list,defStream,(STREAM *) pVar)) list++; break;

        case tLEVEL: ParseLevel(*list,(LEVEL *) pVar); list++; break;
        case tLEVEL_: if(ParseLevel_(*list,defLevel,(LEVEL *) pVar)) list++; break;

        case tCOLOR: ParseColor(*list,(unsigned long *) pVar); list++; break;
        case tCOLOR_: if(ParseColor_(*list,defColor,(unsigned long *) pVar)) list++; break;

        case tCOLORMAP: ParseColorMap(*list,(unsigned long *) pVar); list++; break;
        case tCOLORMAP_: if(ParseColorMap_(*list,defColor,(unsigned long *) pVar)) list++; break;

        case tWORDLIST: ParseWordList(*list,(char ***) pVar); list++; break;
        case tWORDLIST_: if (ParseWordList_(*list,defppChar,(char ***) pVar)) list++; break;

        case tLIST: ParseList(*list,(char ***) pVar); list++; break;
        case tLIST_: if (ParseList_(*list,defppChar,(char ***) pVar)) list++; break;

        case tSCRIPT: ParseScript(list,(SCRIPT *) pVar); list++; break;
        case tSCRIPT_: if (ParseScript_(list,defScript,(SCRIPT *) pVar) == YES) list++; break;

        case tPROC: ParseProc(*list,(PROC *) pVar); list++; break;
        case tPROC_: if(ParseProc_(*list,(PROC) defVal,(PROC *) pVar)) list++; break;

        case tGOBJECT: ParseGObject(*list,(GOBJECT *) pVar); list++; break;
        case tGOBJECT_: if (ParseGObject_(*list,defGObject,(GOBJECT *) pVar)) list++; break;

        case tGOBJECTLIST: ParseGObjectList(*list,(GOBJECT **) pVar); list++; break;
        case tGOBJECTLIST_: if (ParseGObjectList_(*list,defGObjectList,(GOBJECT **) pVar)) list++; break;

        case tWINDOW: ParseWindow(*list,(WINDOW *) pVar); list++; break;
        case tWINDOW_: if (ParseWindow_(*list,defWindow,(WINDOW *) pVar)) list++; break;

        case tFONT: ParseFont(*list,(FONT *) pVar); list++; break;
        case tFONT_: if (ParseFont_(*list,defFont,(FONT *) pVar)) list++; break;
      }
 
    }
    
    
    /* Case of newly defined types */    
    else {
      switch (varTypes[indexType]) {
        case tINT: 
          fint = (char (*) (LEVEL,char *,int, int*)) varTypeParseFunctions[indexType];      
          flag = (*fint)(levelCur,*list,defInt,(int *) pVar);      
          if (flag == NO && type % 2 == 1) Errorf1("");
          if (flag) list++;
          break;
        case tFLOAT: 
          fflt = (char (*) (LEVEL,char *,float, float*)) varTypeParseFunctions[indexType];     
          flag = (*fflt)(levelCur,*list,(float) defDouble,(float *) pVar);      
          if (flag == NO && type % 2 == 1) Errorf1("");
          if (flag) list++;
          break;
        case tVAL: 
          if (*list == NULL) {
            *((VALUE *) pVar) = defVal;
            if (defVal != NULL) {
              AddRefValue(defVal);
              TempValue(defVal);
            }
            flag = NO;
          }
          else {
            fvc =  (char (*) (LEVEL,char *,VALUE, VALUE *)) varTypeParseFunctions[indexType];
            flag = (*fvc)(levelCur, *list,defVal,(VALUE *) pVar);      
          }
          if (flag == NO && type % 2 == 1) Errorf1("");
          if (fvc == (void *) ParseValLevel_) {
            if ((*((VALUE *) pVar)) == NULL) {
              SetErrorf("ParseArgv() : Bad type value NULL ('%s' expected)", varTypeNames[indexType]);
              flag = NO;
            }
            else if (GetTypeValue((*((VALUE *) pVar))) != varTypeNames[indexType]) {
              SetErrorf("ParseArgv() : Bad type '%s' ('%s' expected)", GetTypeValue((*((VALUE *) pVar))),varTypeNames[indexType]);
              flag = NO;
            }
          }          
          if (flag == NO && type % 2 == 1) Errorf1("");
          if (flag) list++;
          else *((VALUE *) pVar) = defVal;
          break;
        case tSTR: 
          fstr =  (char (*) (LEVEL,char *,char*, char**)) varTypeParseFunctions[indexType];     
          flag = (*fstr)(levelCur,*list,defpChar,(char **) pVar);      
          if (flag == NO && type % 2 == 1) Errorf1("");
          if (flag) list++;
          break;
      }
    }
  }
  
  toplevelCur->flagSaveError = YES;
    
  if (!flagMoreArgs) NoMoreArgs(list);
  
  return(list);
}


/*
 * Some useful functions
 */

/* It should be the end of argv */
void NoMoreArgs(char **argv)
{
  if (*argv != NULL) {
    SetErrorf("Too many arguments : %s ... \n",*argv);
    ErrorUsage1();
  }
}

/* Function for parsing options */
char ParseOption(char ***pargv)
{
   char c;
   char **argv = *pargv;
   
   if (*argv == NULL) return('\0');
   
   if ((*argv)[0] != '-' || (*argv)[1] == '\0') Errorf("Not recognized options : %s ... ",*argv);
   if ((*argv)[2] != '\0') Errorf("Bad option : %s ... ",*argv);

   c = (*argv)[1];
   
   *pargv = *pargv + 1;
   return(c);   
}   
  
  
         
