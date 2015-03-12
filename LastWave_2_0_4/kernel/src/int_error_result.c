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
#include "signals.h"

#include <stdarg.h>



#define ErrorLineLength 38

/********************************************************
 *
 * Managing errors
 *
 ********************************************************/


#define MessageLength 10000

/*
 * The corresponding error Msge
 */
static char msge0[MessageLength];
static char msge1[MessageLength];
static char msge2[MessageLength];
static char *errorMsge = msge0;


/*
 * Two useful strings
 */
 
static char tempStr[MessageLength];
static char tempStr1[MessageLength];

 

/* Init the error message */
void InitError()
{
  *errorMsge = '\0';
}

/*
 * Should the error be saved ?
 */

char ShouldSaveError(void)
{
  return(toplevelCur->flagSaveError);
}

/*
 * Is there an error msge ? 
 */
char IsErrorMsge(void)
{
  return(errorMsge[0] != '\0');
}

/*
 * Set the error msge
 */

void SetErrorf(char *format,...)
{
  va_list ap;
  
  if (!ShouldSaveError()) return;
  
  va_start(ap,format);
  vsprintf(errorMsge,format,ap);
  va_end(ap);
}

/*
 * Append a msge to the existing error msge
 */
void AppendErrorf(char *format,...)
{
  va_list ap;

  if (!ShouldSaveError()) return;

  va_start(ap,format);
  vsprintf(tempStr,format,ap);
  va_end(ap);
  strcat(errorMsge,tempStr);
}

/*
 * Append a string to the existing error msge
 */
void AppendStrError(char *str,char *end)
{
  int l;
  
  if (!ShouldSaveError()) return;

  if (end == NULL) strcat(errorMsge,str);
  else {
    l = strlen(errorMsge);
    strncpy(errorMsge+l,str,end-str);
    errorMsge[l+end-str] = '\0';
  }
}
  
/*
 * Prints a syntax error in the 'line' and at the position 'ptr'
 */

void SyntaxError(char msge[],char *line, char *ptr)
{
  char c;
  char *ptr1;
  int n;

  if (!ShouldSaveError()) return;

  if (msge == NULL) strcpy(msge2,errorMsge);
  else strcpy(msge2,msge);

  msge = msge2;
  
  InitError();

  n = 0;
  
  if (ptr<line) ptr = line;
  
  for (ptr1 = MAX(ptr-1,line); ptr1>line && ptr1 >= ptr-ErrorLineLength && *ptr1 != '\n';ptr1--);
  
  if (*ptr1 != '\n' && ptr1 != line)  {
    AppendErrorf("...");
    n+=3;
  }
  if (*ptr1 == '\n' && ptr1 != ptr) ptr1++;
  while (*ptr1 == ' ' &&  ptr1 != ptr) ptr1++;

  AppendStrError(ptr1,ptr);
  n += ptr-ptr1; 

  for (ptr1 = ptr; *ptr1 != '\0' && ptr1 <= ptr+ErrorLineLength/2 && *ptr1 != '\n';ptr1++);

  AppendStrError(ptr,ptr1);
  
  if (*ptr1 != '\0' && *ptr1 != '\n' && *ptr1 != '\r') AppendErrorf("...");     

  AppendErrorf("\n**        ");
  if (toplevelCur->sourceFilename) n+= strlen(toplevelCur->sourceFilename)+3;
  while (n != 0) {AppendErrorf(" "); n--;}
  AppendErrorf(".^.\n");
  
  AppendErrorf("** --> %s",msge);
}
  
/*
 * Generate an error (after appending a new msge)
 */

static void PrintCmdLines(void)
{
  static char c = ' ';
  char line[ErrorLineLength+1];
  char *str,*str1,c1;
  int i;
  
  for (i=toplevelCur->nLevel-1;i>=0;i--) {
    
    if (toplevelCur->levels[i].scriptline == NULL) return;

    if (toplevelCur->levels[i].scriptline->line == NULL) {
      str = TList2Str(toplevelCur->levels[i].scriptline->words,&c);  /* ???? Separateur pas terrible il faut {} */
    }
    else str = toplevelCur->levels[i].scriptline->line;
    
    str1 = strchr(str,'\n');
    if (str1 != NULL && str1[1] == '\0') str1 = NULL;
    if (str1) {
      c1 = *str1;
      *str1 = '\0';
    }
    if (strlen(str) <= ErrorLineLength) {
      if (!str1) PrintfErr("---> %s\n",str);
      else PrintfErr("---> %s...\n",str);
    }
    else {
      strncpy(line,str,ErrorLineLength-3);
      line[ErrorLineLength-3] = '\0';
      PrintfErr("---> %s...\n",line);
    }
    if (str1) *str1 = c1;
  }
}

static void MakeError(void)
{
  struct event event;
  char flagInPromptScript;
 
  /* Print the error message */
  if (toplevelCur->sourceFilename == NULL)  PrintfErr("** Error : %s\n",errorMsge);
  else PrintfErr("** Error [%s] : %s\n",toplevelCur->sourceFilename,errorMsge);

  toplevelCur->flagStoreResult = YES;
  toplevelCur->flagSaveError = YES;
  toplevelCur->packageName = NULL;

  toplevelCur->sourceFilename = NULL;
  
  PrintCmdLines();

  Flush();  
  
  /* Do Error bindings if there are any */
  if (errorMsge == msge0) {
    errorMsge = msge1;
    event.type = ErrorEvent;
    event.object = NULL;
    SendEvent(&event);
  }
  
  errorMsge = msge0;
 
  flagInPromptScript = GetPrompt(NULL);
  if (flagInPromptScript) longjmp(toplevelCur->environment,2);
  else longjmp(toplevelCur->environment,1);
}

void Errorf1(char *format,...)
{
  va_list ap;

 if (errorMsge == msge1) PrintfErr("\n**** Error in error handler\n"); 
     
  va_start(ap,format);
  vsprintf(tempStr,format,ap);
  va_end(ap);
  strcat(errorMsge,tempStr);

  MakeError();
}  

void Errorf(char *format,...)
{
  va_list ap;

 if (errorMsge == msge1) PrintfErr("\n**** Error in error handler\n"); 

  InitError();
       
  va_start(ap,format);
  vsprintf(tempStr,format,ap);
  va_end(ap);
  strcat(errorMsge,tempStr);

  MakeError();
}  

/*
 * Prints a warning message 
 */
   
void Warningf(char *format,...)
{
  va_list ap;
  
  va_start(ap,format);
  vsprintf(tempStr,format,ap);
  va_end(ap);
  
  PrintfErr("** Warning : %s\n",tempStr);
  Flush();  
}

void Warningf1(char *format,...)
{
  va_list ap;
  
  va_start(ap,format);
  vsprintf(tempStr,format,ap);
  va_end(ap);
  
  PrintfErr("\n%s** Error : %s\n\n",tempStr,errorMsge+1); /* ????????????? */
  Flush();  
}


/*
 * Simulate an error and print the usage of the current command 
 */
void ErrorUsage(void)
{
  InitError();
  ErrorUsage1();
}

/*
 * Same as above but call Errorf1 instead of Errorf
 */
void ErrorProcUsage(PROC c)
{
  char *name;
  char **list1,**list;
  extern void GetVarListProc(PROC);
  
  if (c == NULL) Errorf1("");
  
  list = c->description;
  name = c->name;
  
  AppendErrorf("** Usage");
  AppendErrorf(" : ");

  if (list == NULL) {
    toplevelCur->flagStoreResult = YES;
    GetVarListProc (c);
    AppendErrorf(" %s {%s}", name,GetResultStr());
    Errorf1("");
  }
  
  if (*list == NULL) {
    AppendErrorf(" %s",name);
    Errorf1("");
    return;
  }

  ParseWordList(*list,&list1);
  if (*list1 == NULL) {
    AppendErrorf(" %s",name);
    Errorf1("");
    return;
  }
  
  list1[0]++;
  list1[0][strlen(list1[0])-1] = '\0';
  AppendErrorf("%s %s",name,*list1);  
  
  list++;
  while (*list != NULL) {
    ParseWordList(*list,&list1);
    if (*list1 == NULL) break;
    list1[0]++;
    list1[0][strlen(list1[0])-1] = '\0';
    AppendErrorf("\n           %s %s",name,*list1);
    list++;
  }
  Errorf1("");
}

void ErrorUsage1(void)
{
  if (levelCur->scriptline && levelCur->scriptline->proc)
    ErrorProcUsage(levelCur->scriptline->proc);
  
  ErrorProcUsage(levelCur->scommand);
}



/* Simulate an error and print the char 'c'
   as being a bad option for the current command */
void ErrorOption(char c)
{
  Errorf("invalid option or argument (-%c)",c);
}



/********************************************************
 *
 *
 *
 *      Managing results
 *
 *
 *
 ********************************************************/

 
/* Init result */
void InitResult()
{
  toplevelCur->resultType = NULL;
  toplevelCur->nResult = 0;
  toplevelCur->nResultList = 0;
  if (toplevelCur->resultContent != NULL) {
    DeleteValue(toplevelCur->resultContent);
    toplevelCur->resultContent = NULL;
  }
}

/*******************************************
 *
 * Routines to set the result
 *
 *******************************************/

/* If it is a formatted string */
void SetResultf(char *format,...)
{
  va_list ap;

  /* Delete old result */  
  InitResult();  

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;
  
  /* Store it */
  va_start(ap,format);  
  vsprintf(toplevelCur->result,format,ap);
  va_end(ap); 
  
  toplevelCur->nResult = strlen(toplevelCur->result);

  toplevelCur->resultType = strType;  
}

/* If it is a string */
void SetResultStr(char *str)
{
  int n;
  STRVALUE sc;
  
  /* Delete old result */  
  InitResult();  

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;
  
  n = strlen(str);
  if (n >= MaxLengthResult-1) {
    sc = TNewStrValue();
    SetStrValue(sc,str);
    SetResultValue(sc);
    return;
  }
  
  toplevelCur->nResult = n;
    
  /* Store it */
  strcpy(toplevelCur->result,str);

  toplevelCur->resultType = strType;
}

void SetResultList(char **list)
{
  while(*list) {AppendListResultStr(*list); list++;}
}


/* 
 * static routine to append a string to the existing result 
 * WARNING : Not to be used outside of this file 
 */
static void AppendResultStr_(char *str)
{
  int l = strlen(str);
  
  if (toplevelCur->nResult + l >= MaxLengthResult-1) 
    Errorf("AppendResultStr_() : string is too long (%d characters) --> change 'MaxLengthResult' in kernel/include/int_toplevel.h and recompile",toplevelCur->nResult+l);
  strcpy(toplevelCur->result+toplevelCur->nResult,str);
  toplevelCur->nResult += l;
  toplevelCur->resultType = strType;
}

/* Append a string to the existing result */
void AppendResultStr(char *str)
{
  /* Some inits */
  if (toplevelCur->resultType != strType && toplevelCur->resultType != NULL) InitResult();
  
  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  /* store it */
  AppendResultStr_(str);
}


/* Append a formatted string to the existing result */
void AppendResultf(char *format,...)
{
  va_list ap;

  /* Some inits */
  if (toplevelCur->resultType != strType && toplevelCur->resultType != NULL) InitResult();
  
  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  /* store it */
  va_start(ap,format);
  vsprintf(tempStr,format,ap);
  va_end(ap);

  AppendResultStr_(tempStr);
}


/* The result is supposed to contain a list and we just want to append a string */
static void AppendListResultStr_(char *str)
{
  char flagBraces;
   
  /* Test if not too many elements */
  if (toplevelCur->nResultList >= MaxLengthResultList-1) 
    Errorf("AppendListResultStr_() : list is too long (%d words) --> change 'MaxLengthResultList' in kernel/include/int_toplevel.h and recompile",MaxLengthResultList);

  /* If it is not the beginning of the result then we print a ' ' */
  if (toplevelCur->nResult != 0) AppendResultStr_(" ");
  
  /* Should we put braces around the result ? */
  flagBraces = IsList(str);
  if (*str == '\0') flagBraces = YES;
  
  /* Let's do it */
  if (!flagBraces) {
    toplevelCur->begResultList[toplevelCur->nResultList] = toplevelCur->result+toplevelCur->nResult;
    AppendResultStr_(str);
    toplevelCur->endResultList[toplevelCur->nResultList] = toplevelCur->result+toplevelCur->nResult-1;
  }
  else {
    AppendResultStr_("{");
    toplevelCur->begResultList[toplevelCur->nResultList] = toplevelCur->result+toplevelCur->nResult;
    AppendResultStr_(str);
    toplevelCur->endResultList[toplevelCur->nResultList] = toplevelCur->result+toplevelCur->nResult-1;
    AppendResultStr_("}");
  }
  toplevelCur->nResultList++;
}

/* The result is supposed to contain a list and we just want to append a string */
void AppendListResultStr(char *str)
{
  /* Some inits */
  if (toplevelCur->resultType != strType && toplevelCur->resultType != NULL) InitResult();

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  AppendListResultStr_(str);  
}

/* The result is supposed to contain a list and we just want to append a string */
void AppendListResultf(char *format,...)
{
  va_list ap;
   
  /* Some inits */
  if (toplevelCur->resultType != strType && toplevelCur->resultType != NULL) InitResult();

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  va_start(ap,format);
  vsprintf(tempStr1,format,ap);
  va_end(ap);

  AppendListResultStr_(tempStr1);  
}

/* If it is an VALUE */
void SetResultContent_(VALUE val)
{
  SIGNAL signal;
  
  /* Delete old result */  
  InitResult();  

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  /* Store it */
  val = ValueOf(val);
  toplevelCur->resultType = GetTypeValue(val);
  
  if (toplevelCur->resultType == numType)
    toplevelCur->resultNum = CastValue(val,NUMVALUE)->f;
  
  else if (toplevelCur->resultType == signaliType || toplevelCur->resultType == signalType) {
    signal = CastValue(val,SIGNAL);
    if (signal->size == 1 && signal->type == YSIG) {
      toplevelCur->resultNum = signal->Y[0];
      toplevelCur->resultType = numType;
      return;
    }
    toplevelCur->resultContent = val;
    AddRefValue(val);
  }   
 
  else {
    toplevelCur->resultContent = val;
    AddRefValue(val);
  }   
}


/* If it is a float */
void SetResultFloat(float f)
{
  /* Delete old result */  
  InitResult();  

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  /* Store it */
  toplevelCur->resultNum = f;
  toplevelCur->resultType = numType;
}


/* If it is an int */
void SetResultInt(int i)
{
  /* Delete old result */  
  InitResult();  

  /* Should we store the result ? */
  if (!toplevelCur->flagStoreResult) return;

  /* Store it */
  toplevelCur->resultNum = i;
  toplevelCur->resultType = numType;
}



/*******************************************
 *
 * Routines to get the result
 *
 *******************************************/

/* Get the result Type */
char *GetResultType(void)
{
  if (toplevelCur->resultType == NULL) return(NULL);
  return(toplevelCur->resultType);
}
  

/* Function to get the string result */
char *GetResultStr(void)
{
  if (toplevelCur->resultType == NULL) Errorf("GetResultStr() : Sorry no result to get");

  if (toplevelCur->resultType != strType)
    Errorf("GetResultStr() : Sorry the result is not a string, it is of type '%s'",toplevelCur->resultType);  
  
  if (toplevelCur->resultContent == NULL) return(toplevelCur->result);
  
  return(CastValue(toplevelCur->resultContent,STRVALUE)->str);
}

/* Function to get the VALUE result which is neither a string nor a number */
VALUE GetResultValue(void)
{
  VALUE val;

  if (toplevelCur->resultContent != NULL) {
    val = toplevelCur->resultContent;
    return(val);
  }
 
  if (toplevelCur->resultType == NULL) return(nullValue);
  
  if (toplevelCur->resultType == strType) {
    val = (VALUE) TNewStrValue();
    SetStrValue((STRVALUE) val, toplevelCur->result);
    if (toplevelCur->nResultList != 0) {
      toplevelCur->begResultList[toplevelCur->nResultList] = NULL;
      toplevelCur->endResultList[toplevelCur->nResultList] = NULL;
      ((STRVALUE) val)->list = BegEndStr2List(toplevelCur->begResultList,toplevelCur->endResultList);
    }
    return(val);
  }
  
  if (toplevelCur->resultType == numType) {
    val = (VALUE) TNewNumValue();
    SetNumValue((NUMVALUE) val, toplevelCur->resultNum);
    return(val);
  }

  Errorf("GetResultValue() : Weird error");
}  

/* Get the result as an integer */
int GetResultInt(void)
{
  float f;
  
  if (toplevelCur->resultType == NULL) Errorf("GetResultInt() : Sorry no result to get");

  if (toplevelCur->resultType != numType)
    Errorf("GetResultInt() : Sorry the result is not a number, it is of type '%s'",toplevelCur->resultType);  

  if (toplevelCur->resultContent != NULL) f = CastValue(toplevelCur->resultContent,NUMVALUE)->f;
  else f = toplevelCur->resultNum;

  if (f != (int) f) Errorf("GetResultInt() : Sorry the result is not an integer it is a foat (%g)",f);
  
  return((int) f);
}

/* Get the result as a float */
float GetResultFloat(void)
{
  if (toplevelCur->resultType == NULL) Errorf("GetResultInt() : Sorry no result to get");

  if (toplevelCur->resultType != numType)
    Errorf("GetResultInt() : Sorry the result is not a number, it is of type '%s'",toplevelCur->resultType);  

  if (toplevelCur->resultContent != NULL) return(CastValue(toplevelCur->resultContent,NUMVALUE)->f);

  return(toplevelCur->resultNum);
}


/*******************************************
 *
 * Routines to print the result
 *
 *******************************************/

/* A simple print */
void PrintResult(void)
{
  static STRVALUE sc = NULL;
  static NUMVALUE nc = NULL;
  VALUE val;
  char *type;
  char *str;
  
  if (sc == NULL) sc = NewNullStrValue();
  else sc->str = NULL;
  if (nc == NULL) nc = NewNumValue();
  
  type = GetResultType();
  if (type == NULL) return;
  
  if (type == strType) {
    SetStrValue(sc,GetResultStr());
    val = (VALUE) sc;
  }
  else if (type == numType) {
    SetNumValue(nc,GetResultFloat());
    val = (VALUE) nc;
  }
  else val = GetResultValue();
  
  Printf("= %s\n",ToStrValue(val,NO));
}

