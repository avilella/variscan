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
 *  The Num content
 *
 *****************************************************************/  


#include "lastwave.h"
#include <stdarg.h>
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"



/* The types returned when asked */
char *numType = "&num";

/* Two more types are provided only for inquiry */
char *intType = "&int";
char *floatType = "&float";


/*
 * Print a number
 */

char * ToStrNum(NUMVALUE val, char flagShort)
{
  static char str[30];  
#ifdef NUMDOUBLE
  sprintf(str,"%.16g",val->f);
#else
  sprintf(str,"%.8g",val->f);
#endif
  return(str);
}


/*
 * Print the info of a num
 */
void PrintInfoNum(NUMVALUE nc)
{
#ifdef NUMDOUBLE
  Printf("   f =  %.16g\n",nc->f);
#else
  Printf("   f =  %.8g\n",nc->f);
#endif
}

/* 
 * The function that manages setting a number using explicit parameters
 */

char *SetNum_(float *f, float flt, VALUE val,char *equal, char *fieldName)
{
  if (val != NULL) {
    if (GetTypeValue(val) != numType) {
      if (fieldName == NULL) SetErrorf("Cannot assign '%s' to number",fieldName);
      else SetErrorf("Cannot assign field '%s' with non number argument (of type '%s')",fieldName,GetTypeValue(val));
      return(NULL);
    }
    flt = ((NUMVALUE) val)->f;
  }

  switch(*equal) {
  
  case ':' :
    SetErrorf("Inadequate operator ':=' for number");
    return(NULL);
  case '=' : *f = flt; return(numType);
  case '+' : *f += flt; return(numType);
  case '-' : *f -= flt; return(numType);
  case '*' : *f *= flt; return(numType);
  case '^' : *f = pow(*f,flt); return(numType);
  case '/' : 
    if (flt == 0) {
      SetErrorf("Division by 0");
      return(NULL);
    }
    *f /= flt; 
    return(numType);
  case '%' : 
    if ((int) flt != flt || (int) *f != *f) {
      SetErrorf("operaor % must be used with integers");
      return(NULL);
    }
    *f = ((int) *f) % ((int) flt); 
    return(numType);
  default :
    SetErrorf("Weird error");
    return(NULL);
  }
}

/*
 * Basic routine to deal with setting a number field
 */
void *SetFloatField(float *pflt, void **arg,char flag)
{
   char *field = ARG_S_GetField(arg);
   char *type = ARG_S_GetRightType(arg);      
   float flt = ARG_S_GetRightFloat(arg);   
   VALUE val = ARG_S_GetRightValue(arg);
   char *equal = ARG_S_GetEqual(arg);
   float *pfltRes = ARG_S_GetResPFloat(arg);
   float tempFlt = *pflt;
   
   if (SetNum_(&tempFlt,flt,val,equal,field)==NULL) return(NULL);
   switch(flag) {
   case 0: break;
   case FieldPositive : 
     if  (tempFlt < 0) {
       SetErrorf("Expect a positive float for field '%s'",field);
       return(NULL);
     }
     break;
   case FieldSPositive : 
     if  (tempFlt <= 0) {
       SetErrorf("Expect a strictly positive float for field '%s'",field);
       return(NULL);
     }
     break;
   case FieldNegative : 
     if  (tempFlt > 0) {
       SetErrorf("Expect a negative float for field '%s'",field);
       return(NULL);
     }
     break;
   case FieldSNegative : 
     if  (tempFlt <= 0) {
       SetErrorf("Expect a strictly negative float for field '%s'",field);
       return(NULL);
     }
     break;
   default : Errorf("Bad value for last argument in SetFloatField() call");
   }
    
   *pflt = tempFlt;
   *pfltRes = tempFlt;
   return(numType); 
}

void *SetIntField(int *pint, void **arg,char flag)
{
  char *field = ARG_S_GetField(arg);
  char *type = ARG_S_GetRightType(arg);      
  float flt = ARG_S_GetRightFloat(arg);   
  VALUE val = ARG_S_GetRightValue(arg);
  char *equal = ARG_S_GetEqual(arg);
  float *pfltRes = ARG_S_GetResPFloat(arg);
  float flt1 = *pint;
   
  if (SetNum_(&flt1,flt,val,equal,field) == NULL) return(NULL);
   
  if  (flt1 != (int) flt1) {
    SetErrorf("Expect an integer for field '%s'",field);
    return(NULL);
  }
  *pint = (int) flt1;
   
  switch(flag) {
  case 0: return(numType);
  case FieldPositive : 
    if  (flt1 < 0) {
      SetErrorf("Expect a positive int for field '%s'",field);
      return(NULL);
    }
    break;
  case FieldSPositive : 
     if  (flt1 <= 0) {
       SetErrorf("Expect a strictly positive pint for field '%s'",field);
       return(NULL);
     }
    break;
  case FieldNegative : 
     if  (flt1 > 0) {
       SetErrorf("Expect a negative pint for field '%s'",field);
       return(NULL);
     }
    break;
  case FieldSNegative : 
     if  (flt1 <= 0) {
       SetErrorf("Expect a strictly negative pint for field '%s'",field);
       return(NULL);
     }
    break;
  default : Errorf("Bad value for last argument in SetIntField() call");
  }
  
  *pint = (int) flt1;
  *pfltRes = flt1;

  return(numType);
}


void *GetFloatField(float f, void **arg)
{
  float *pFlt = ARG_G_GetResPFloat(arg);

  *pFlt = f;
  
  return(numType);
}
void *GetIntField(int i, void **arg)
{
  float *pFlt = ARG_G_GetResPFloat(arg);

  *pFlt = i;
  
  return(numType);
}




/*
 * Desallocation of a Num content
 */
 
void DeleteNumValue(NUMVALUE nc)
{
  RemoveRefValue( nc);
  if (nc->nRef>0) return;

#ifdef DEBUGALLOC
DebugType = "NumValue";
#endif
  Free(nc);
}   

/*
 * Init Num Content
 */
void InitNumValue(NUMVALUE nc) 
{
  extern TypeStruct tsNum;

  InitValue(nc,&tsNum);
  
  nc->f = 0;
}
 
/*
 * Allocation of a num content
 */
 
NUMVALUE NewNumValue(void)
{
  NUMVALUE nc;

#ifdef DEBUGALLOC
DebugType = "NumValue";
#endif
  
  nc = Malloc(sizeof(struct numValue));
  
  InitNumValue(nc);
  return(nc);
}

NUMVALUE TNewNumValue(void)
{
  NUMVALUE nc = NewNumValue();
  
  TempValue(nc);
  
  return(nc);
}


void CopyNumValue(NUMVALUE in, NUMVALUE out)
{
  out->f = in->f;
}



/* 
 * Set a basic content with a number 
 */

void SetNumValue(NUMVALUE nc, float value)
{
  nc->f = value;
}


/* 
 * Getting the integer value of a variable named 'name' at a given level 'level'
 *    (generate an error if not the right type)
 */ 

int GetIntVariableLevel(LEVEL level,char *name)
{
  VARIABLE var;
  VALUE val;
  NUMVALUE sc;
    
  var = GetVariableLevel(level,name);
  val = ValueOf((VALUE) var);
  if (GetTypeValue(val) != numType) Errorf("GetIntVariableLevel() : Bad variable type '%s'",GetTypeValue(val));
  sc = (NUMVALUE) val;
  if (sc->f != (int) sc->f) Errorf("GetIntVariableLevel() : Bad variable type '%s'",floatType);
  
  return((int) (sc->f));
} 

int GetIntVariable(char *name)
{
  return(GetIntVariableLevel(levelCur,name));
}


/* 
 * Getting the float value of a variable named 'name' at a given level 'level'
 *    (generate an error if not the right type)
 */ 

float GetFloatVariableLevel(LEVEL level,char *name)
{
  VARIABLE var;
  VALUE val;
  NUMVALUE sc;
    
  var = GetVariableLevel(level,name);
  val = ValueOf((VALUE) var);
  if (GetTypeValue(val) != numType) Errorf("GetFloatVariableLevel() : Bad variable type '%s'",GetTypeValue(val));
  sc = (NUMVALUE) val;
  
  return((int) (sc->f));
} 

float GetFloatVariable(char *name)
{
  return(GetFloatVariableLevel(levelCur,name));
}


/* 
 * Setting a num content variable at a given level (creates it if it does not exist)
 * and sets it with float 'value'
 */ 

void SetNumVariableLevel(LEVEL level,char *name,float value)
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
  if (!DoesTypeOverwrite(type,numType)) 
    Errorf("SetNumVariableLevel() : Cannot overwrite variable '%s' of type '%s' with '%s' typed value",name,type,numType);

  /* If the content is a numType can we use it ? */
  if (type == numType && (*pcont)->nRef == 1) {
    SetNumValue((NUMVALUE) (*pcont),value);
    return;
  }
  
  /* Delete the content */
  DeleteValue(*pcont);

  /* Create it */
  *pcont = (VALUE) NewNumValue();
  
  /* Set it */
  SetNumValue((NUMVALUE) (*pcont),value);
}

 
void SetNumVariable(char *name,float value)
{
  SetNumVariableLevel(levelCur,name,value);
}  


/*
 * The field list
 */
struct field fieldsNum[] = {
  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for NUMVALUE
 */

TypeStruct tsNum = {

  "{{{&num} {This type corresponds to numbers. \n \
- +,-,*,/,^ (and +=,-=,*=,/=,^=) : regular operators \n \
- //,% : integer division and remainder \n \
- ==,!=,<=,>=,<,> : regular tests \n \
- &&,|| : and,or \n \
- sinh,sin,cosh,cos,tanh,tan,acos,asin,atan : trigonometric operators \n \
- log2,log,ln,sqrt,abs,exp,ceil,floor,round,frac,int : other math functions \n \
- Constants : pi,grand,urand}} \
{{&float} {Extended type : same as '&num'}} \
{{&int} {Extended type which corresponds to '&num' which are integers}}}",  /* Documentation */

  &numType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteNumValue,     /* The Delete function */
  NewNumValue,     /* The New function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrNum,       /* String conversion */
  NULL,   /* The Print function : print the object when 'print' is called */
  PrintInfoNum,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsNum,      /* The list of fields */
};
