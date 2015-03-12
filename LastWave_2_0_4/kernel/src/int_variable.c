/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 2                           */
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

#include "lastwave.h"
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"
#include "xx_system.h"

/* Maximum variable (or field) name length */
#define MaxNameLength 50



char *varType = "&var";


/*****************************************************************
 *
 *  (Des)Allocation of Variables
 *
 *****************************************************************/  

/*
 * Allocation of  a variable 
 */

VARIABLE GetVariableHT(HASHTABLE *t, char flagCreate, char *name, char **left, char *flag);


static VARIABLE NewVariable(void) 
{
  extern TypeStruct tsVar;
  VARIABLE variable;
  
 #ifdef DEBUGALLOC
DebugType = "Variable";
#endif
  variable = (VARIABLE) Malloc(sizeof(struct variable));
  InitValue(variable,&tsVar);

  variable->name = NULL;
  variable->content = NULL;
  variable->traceVarName = NULL;
  variable->hashTable = NULL;
  return(variable);
}

static VALUE NewVar(void) 
{
  return(nullValue);
}

/*
 * Desallocation of  a variable 
 *
 * (not to be used by the user)
 * Delete the variable structure (variable has been removed from hash table already
 */

static void DeleteVariable_(VALUE v)
{
  VARIABLE variable = (VARIABLE) v;
  VARIABLE var1;

 
  RemoveRefValue( variable);
  if (variable->nRef > 0) return;
  
  if (variable->content != NULL) {
    /* Case the content is not a variable */
    if (!IsVariable(variable->content)) DeleteValue(variable->content);
    /* Case it is a variable */
    else {
      var1 = (VARIABLE) variable->content;
      /* We must delete var1 if there is just one reference to it
         or if there are 2 and it is pointing to a null content */
      if (var1->nRef == 2 && var1->content == NULL) var1->nRef = 1;
      if (var1->nRef > 1) RemoveRefValue( var1);
      else {
        if (var1->hashTable) GetRemoveElemHashTable(var1->hashTable,var1->name);
        DeleteVariable_((VALUE) var1);
      }
    }
  }    
  
  variable->content = NULL;
  
  if (variable->nRef <= 0) {  
    if (variable->name != NULL) Free(variable->name);
    if (variable->traceVarName != NULL) Free(variable->traceVarName);

 #ifdef DEBUGALLOC
DebugType = "Variable";
#endif
    Free(variable);
  }
}

/*
 * Regular desallocation 
 */
 
void DeleteVariableLevel(LEVEL level, char *name)
{
  VARIABLE var;
  HASHTABLE t;
  char *left;
  char flag;
  
  while (level->levelVar != level) level = level->levelVar;
  t = level->theVariables;
  
  var = GetVariableHT(&t,0,name,&left,&flag);
  if (flag != 0 || *left != '\0') Errorf1("");
  
  GetRemoveElemHashTable(t,var->name);  
  
  DeleteVariable_((VALUE) var);
}

void DeleteVariable(char *name)
{
  DeleteVariableLevel(levelCur, name);
}


/* Delete Variable if it exists */
void DeleteVariableIfExistLevel(LEVEL level, char *name)
{
  VARIABLE var;
  HASHTABLE t;
  char *left,flag;
  
  while (level->levelVar != level) level = level->levelVar;
  t = level->theVariables;
  
  var = GetVariableHT(&t,0,name,&left,&flag);
  
  /* case variable does not exist */
  if (flag == 2) return;
  
  /* Case there is an error */
  if (flag != 0 || *left != '\0') Errorf1("");
  
  /* Case we should delete it */
  GetRemoveElemHashTable(t,var->name);  
  
  DeleteVariable_((VALUE) var);
}

void DeleteVariableIfExist(char *name)
{
  DeleteVariableIfExistLevel(levelCur, name);
}


/*
 * String conversion
 */
char *ToStrVar(VARIABLE val, char flagShort)
{
  if (val->content == NULL) return("");
  
  return(ToStrValue(val->content, flagShort));
}

/*
 * Print a variable
 */
void PrintVar(VARIABLE val)
{
  if (val->content == NULL) return;
  
  PrintValue(val->content);
}


/*
 * Print the info of a variable
 */
void PrintInfoVar(VARIABLE val)
{
  if (val->content == NULL) return;
  
  PrintInfoValue(val->content);
}


/*
 * Get the type of a variable
 */
 
static char *GetTypeVar(VALUE val)
{
  if (((VARIABLE) val)->content == NULL) return(NULL);
  
  return(GetTypeValue(((VARIABLE) val)->content));
}

/*
 * Set a field
 */
static void *SetFieldVar(VALUE v, void **arg)
{
  VARIABLE var = (VARIABLE) v;
  char *field;
  FSIList *fsiList;
  char *type,*type1,**pstrRes;
  float flt,*pfltRes;
  VALUE val,*pcont1,*pValueRes;
  char *equal;
  extern char *SetStr_(char **str,FSIList *fsiList, float flt, VALUE val,char *equal, char *fieldName);
  extern char *SetNum_(float *f, float flt, VALUE val,char *equal, char *fieldName);
  extern void *SendMessage2Num(void *content,int message,void **arg);
  extern void *SendMessage2Str(void *content,int message,void **arg);
  extern void *SetFieldArg(VALUE value, void **arg);

  field = ARG_S_GetField(arg);
  fsiList = ARG_S_GetFsiList(arg);
  type = ARG_S_GetRightType(arg);      
  flt = ARG_S_GetRightFloat(arg);   
  val = ARG_S_GetRightValue(arg);
  equal = ARG_S_GetEqual(arg);
  pValueRes = ARG_S_GetResPValue(arg);
  pfltRes = ARG_S_GetResPFloat(arg);
  pstrRes = ARG_S_GetResPStr(arg);
      
  /* There should be no field and no fsiList */
  if (field != NULL || fsiList != NULL) {
    SetErrorf("SetFieldVar() : Weird");
    return(NULL);
  }
      
  /* Get a pointer to the variable content and its type */     
  pcont1 = GetVariablePContent(var,NO);
  type1 = GetTypeValue(*pcont1);
      
  /* Check if we can overwrite the variable with the value */
  if (*equal == '=' && !DoesTypeOverwrite(type1,type)) {
    SetErrorf("Cannot overwrite '%s' variable with '%s' value",type1,type);
    return(NULL);
  }
      
  /* If the value to set is a variable then we must set it unless it loops !!!! */
  if (val != NULL && IsVariable(val)) {
    if (ValueOf(val) == *pcont1) {
      SetErrorf("Sorry, no recursive reference allowed");
      return(NULL);
    }
    DeleteValue(*pcont1);
    *pcont1 = val;
    AddRefValue(val);
    *pValueRes = ValueOf(val);
    return(GetTypeValue(val));
  }

  /* Get the real content of val */
  if (val != NULL) val = ValueOf(val);
      
      
  /***************************
   *
   * Managing operators *=, +=, := ...
   *
   ***************************/
            
  /* If variable type is a number and operator is not '=', it must be handled by the SetNum_ routine */
  if (*equal != '=' && type1 == numType) {
    if (SetNum_(&(((NUMVALUE) *pcont1)->f),flt,val,equal,NULL) == NULL) return(NULL);
    *pfltRes = (((NUMVALUE) *pcont1)->f);
    return(numType);
  }

  /* If variable type is a string and operator is not '=', it must be handled by the SetStr_ routine */
  if (*equal != '=' && type1 == strType) {
    if (SetStr_(&(((STRVALUE) *pcont1)->str),NULL,flt,val,equal,NULL) == NULL) return(NULL);
    *pstrRes = (((STRVALUE) *pcont1)->str);
    return(strType);
  }

  /* If variable type is a signal and operator is not '=', it must be handled by the SetSignalField_ routine */
  if (*equal != '=' && type1 == signaliType) {
    if (SetSignalField_((SIGNAL) *pcont1,NULL,NULL,flt,val,equal,NULL) == NULL) return(NULL);
    *pValueRes = *pcont1;
    return(type1);
  }
  
  /* Other cases */
  if (*equal != '=') {
    return(SetFieldArg(*pcont1,arg));
  }    

  /***************************
   *
   * Limit (des)allocation
   *
   ***************************/
       
  /* Limit (des)allocation in case of a number */
  if (type1 == numType && type == numType && (*pcont1)->nRef == 1) {
    if (val == NULL) SetNumValue((NUMVALUE) (*pcont1),flt);
    else SetNumValue((NUMVALUE) (*pcont1),((NUMVALUE) val)->f);
    *pfltRes = ((NUMVALUE) (*pcont1))->f;
    return(numType);
  }

  /* Limit (des)allocation in case of a range */
  if (type1 == rangeType && type == rangeType && (*pcont1)->nRef == 1) {
    ((RANGE) (*pcont1))->first = ((RANGE) val)->first;
    ((RANGE) (*pcont1))->step = ((RANGE) val)->step;
    ((RANGE) (*pcont1))->size = ((RANGE) val)->size;
    *pValueRes = *pcont1;
    return(rangeType);
  }
  

  /***************************
   *
   * If the value to set is a variable content with a count ref of 1 then just set it
   *
   ***************************/
       
  if (val != NULL && val->nRef == 1) {
    DeleteValue(*pcont1);
    *pcont1 = val;
    AddRefValue(val);
    *pValueRes = val;
    return(type1);
  }

  /***************************
   *
   * Handle copying if any
   *
   ***************************.

  /* If the value to set is a number then we must create a number content */
  if (val == NULL || type == numType) {
    DeleteValue(*pcont1);
    *pcont1 = (VALUE) NewNumValue();
    if (val == NULL) SetNumValue((NUMVALUE) (*pcont1),flt);
    else SetNumValue((NUMVALUE) (*pcont1),((NUMVALUE) val)->f);
    *pfltRes = ((NUMVALUE) *pcont1)->f;
    return(numType);
  }

  /* If the value to set is a string */
  if (type == strType) {
    DeleteValue(*pcont1);
    *pcont1 = (VALUE) NewStrValue();
    SetStrValue((STRVALUE) (*pcont1),((STRVALUE) val)->str);
    *pstrRes = ((STRVALUE) val)->str;
    return(strType);
  }

  /* If the value to set is a range then we must create a range content */
  if (type == rangeType) {
    DeleteValue(*pcont1);
    *pcont1 = (VALUE) NewRange();
    ((RANGE) (*pcont1))->first = ((RANGE) val)->first;
    ((RANGE) (*pcont1))->step = ((RANGE) val)->step;
    ((RANGE) (*pcont1))->size = ((RANGE) val)->size;
    *pValueRes = *pcont1;
    return(rangeType);
  }

  /***************************
   *
   * Regular case
   *
   ***************************/

  DeleteValue(*pcont1);
  *pcont1  = val;
  AddRefValue(val);
  *pValueRes = *pcont1;
  return(type1);  
}


/*
 * NumExtract
 */
void *NumExtractVar(VALUE v, void **arg)
{
  if (((VARIABLE) v)->content == NULL) return("");
/*  NumExtractValue(v,arg); ??*/
  return(NumExtractValue(((VARIABLE) v)->content,arg));
}

/*
 * The field list
 */
struct field fieldsVariable[] = {
  NULL, NULL, NULL, NULL, NULL
};


/*
 * The type structure for VARIABLE
 */

TypeStruct tsVar = {

  "",              /* Documentation */

  &varType,       /* The basic (unique) type name */
  GetTypeVar,     /* The GetType function */                       
  
  DeleteVariable_,     /* The Delete function */
  NewVar,     /* The New function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
    
  ToStrVar,       /* String conversion */
  PrintVar,   /* The Print function : print the object when 'print' is called */
  PrintInfoVar,   /* The PrintInfo function : called by 'info' */

  NumExtractVar,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsVariable,    /* The list of fields */
};




/*
 * Functions to manage types
 */

/* The variables to manage new variable types */
#define NVarTypes 100 
char *varTypeNames[NVarTypes];
TypeStruct *typeStruct[NVarTypes];
void *varTypeParseFunctions[NVarTypes];
char *typePackage[NVarTypes];
int varTypes[NVarTypes];
int nVarTypes = 0;


/* Get a type from its name */
char * GetArgType(char *str)
{
  int i;
  
  extern TypeStruct tsNull;
  
  if (!strcmp(str,nullType)) return(nullType);
  if (!strcmp(str,"&str")) return(strType);
  if (!strcmp(str,strType)) return(strType);
  if (!strcmp(str,valType)) return(valType);
  if (!strcmp(str,valobjType)) return(valobjType);
  if (!strcmp(str,wordType)) return(wordType);
  if (!strcmp(str,wordlistType)) return(wordlistType);
  if (!strcmp(str,listType)) return(listType);
  if (!strcmp(str,listvType)) return(listvType);
  if (!strcmp(str,rangeType)) return(rangeType);
  if (!strcmp(str,numType)) return(numType);
  if (!strcmp(str,intType)) return(intType);
  if (!strcmp(str,floatType)) return(floatType);
  if (!strcmp(str,varType)) return(varType);
  if (!strcmp(str,arrayType)) return(arrayType);
  if (!strcmp(str,scriptType)) return(scriptType);
  if (!strcmp(str,procType)) return(procType);
  
  for (i=0; i<nVarTypes;i++) {
    if (!strcmp(str,varTypeNames[i])) return(varTypeNames[i]);
  }
  
  return(NULL);  
}  

/* Get the type structure from a type */
TypeStruct *GetTypeStruct(char *str)
{
  int i;
  extern TypeStruct tsHash;
  extern TypeStruct tsNull;
  
  if (!strcmp(str,nullType)) return(&tsNull);
  if (!strcmp(str,"&str")) return(&tsStr);
  if (!strcmp(str,strType)) return(&tsStr);
  if (!strcmp(str,arrayType)) return(&tsHash);
  if (!strcmp(str,listType)) return(&tsStr);
  if (!strcmp(str,listvType)) return(&tsListv);
  if (!strcmp(str,rangeType)) return(&tsRange);
  if (!strcmp(str,numType)) return(&tsNum);
  if (!strcmp(str,intType)) return(&tsNum);
  if (!strcmp(str,floatType)) return(&tsNum);
  if (!strcmp(str,scriptType)) return(&tsScript);
  if (!strcmp(str,procType)) return(&tsProc);
  
  for (i=0; i<nVarTypes;i++) {
    if (!strcmp(str,varTypeNames[i])) return(typeStruct[i]);
  }
  
  return(NULL);
}
 


/*
 * Add a new variable type to LastWave interpreter
 *
 *   typeName : the name of the type. 
 *              it should be of the form "&type" and should a be a pointer (always the same!) to a string
 *
 *   type     : specifies the type associated to the LastWave variable. It must be one of
 *              tINT --> in case of integers (NUMVALUE)
 *              tFLOAT --> in case of floats (NUMVALUE)
 *              tSTR --> in case of strings (STRVALUE)
 *              tVAL --> in case of pointers to VALUE
 *
 *   parse    : this is the main parse function. This function is used both in the case a "&type" is met
 *              or when an explicit C-Parse is called.
 *              The type of this function depends on the value of the 'type' parameter.
 *                tINT --> char *parse (LEVEL level, char *str, int defVal, int *val)
 *                tFLOAT --> char *parse (LEVEL level, char *str, float defVal, float *val)
 *                tSTR --> char *parse (LEVEL level, char *str, char *defVal, char **val)
 *                tVAL --> char *parse (LEVEL level, char *str, void *defVal, void **val)
 *              In any case, the parameters are
 *                'level' : the level the parsing takes place in
 *                'str'   : the string expression to be parseed
 *                'defVal' : the default value if parsing has failed
 *                'val'    : the parsed value
 *              The function returns YES or NO depending upon whether the parsing has succeeded or failed
 *        
 *   The 'parse' function can be NULL in which case the LastWave evaluator is called and the type of the result
 *   is checked to be the right type.
 *
 *   ts       : the type structure associated to the type if any (NULL otherwise)
 *          
 *   This function returns the code tTYPE for the ParseArgv function (tTYPE_ MUST be set to be tTYPE_+1)
 *       
 */


static int AddVariableType_(char *typeName, int type, void *parse, TypeStruct *ts)
{
  if (typeName == NULL) Errorf("AddVariableType_() : cannot add NULL type");
  if (typeName[0] != '&') Errorf("AddVariableType_() : type name (%s) should start with a '&'",typeName);
  if (GetArgType(typeName) != NULL) Errorf("AddVariableType_() : type '%s' already exists",typeName);
  
  if (nVarTypes >= NVarTypes) Errorf("AddVariableType_() : sorry too many types already defined");
  
  /* Managing type */
  varTypeNames[nVarTypes] = typeName;
  if (parse == NULL) parse = (void *) ParseValLevel_;
  varTypeParseFunctions[nVarTypes] = parse;
  varTypes[nVarTypes] = type;
  typeStruct[nVarTypes] = ts;
  if (toplevelCur->packageName != NULL) typePackage[nVarTypes] = CopyStr(toplevelCur->packageName);
  else typePackage[nVarTypes] = CopyStr("kernel");
  
  nVarTypes++;
  return((nVarTypes-1)*2+tLAST);
}


/*
 * The corresponding routines 
 */
 
int AddVariableTypeInt(char *type, char (*parse) (LEVEL level, char *, int, int *))
{
  return (AddVariableType_(type,tINT, (void *) parse,NULL));
}

int AddVariableTypeFloat(char *type, char (*parse) (LEVEL level, char *, float, float *))
{
  return (AddVariableType_(type,tFLOAT, (void *) parse,NULL));
}

int AddVariableTypeStr(char *type, char (*parse) (LEVEL level, char *, char *, char **))
{
  return (AddVariableType_(type,tSTR, (void *) parse,NULL));
}

int AddVariableTypeValue(char *type, TypeStruct *ts, char (*parse) (LEVEL level, char *, void *, void **))
{
  return (AddVariableType_(type,tVAL, (void *) parse,ts));
}

void AddVariableType(char *type, char (*f) (LEVEL, char *, void *, void **))
{
  Warningf("AddVariableType(%s) : Old function",type);
}


/* 
 * Getting a (simple) variable at a given level.
 *   flagCreate = 0 ==> nothing is done if variable does not exist
 *   flagCreate = 1 ==> variable is created and initialized with nullValue
 *   flagCreate = 2 ==> variable is created and is an array
 */ 

static VARIABLE GetSimpleVariableHT(HASHTABLE t,char *name, char flagCreate)
{
  VARIABLE variable;
  
  variable = (VARIABLE) GetElemHashTable(t,name);
  
  if (flagCreate && variable == NULL) {    
    variable = NewVariable();
    variable->name = CopyStr(name);
    variable->hashTable = t;
    if (flagCreate == 1) {
      variable->content = nullValue;
      nullValue->nRef++;
    }
    else variable->content = (VALUE) NewHashTable(5);
    AddElemHashTable(t,(AHASHELEM) variable); 
    RemoveRefValue( variable);
  }
  
  return(variable);
}


/* 
 * Getting a variable in a given hashtable.
 * The variable is of the form a.b.c where a and a.b are arrays
 *
 *
 * It returns :
 *
 *    the last variable it found
 *    *left is the character it stopped parsing the name at
 *    *flag is 0 if it went well and stopped at a character which is not a '.'
 *          is 1 if it stopped because the variable is not an array and it should be or any other bad error
 *          is 2 if variable does not exist (in which case it returns NULL)
 *    in any case 't' is changed and contains the hashtable the returned variable is in
 *    
 * Does not generate an error
 */ 

VARIABLE GetVariableHT(HASHTABLE *t, char flagCreate, char *name, char **left, char *flag)
{  
  char *name1,*name2;
  char field[MaxNameLength];
  VARIABLE variable;
  VALUE c;
  
  *flag = 0;
  
  variable = NULL;
  
  name1 = name2 = name;

  if (IsValidSymbolChar1(*name2)) {
    name2++;
    while (IsValidSymbolChar(*name2)) name2++;
  }
  if (*name2 == '\0') {
    variable = GetSimpleVariableHT(*t,name1,flagCreate);
    if (variable == NULL) {
      SetErrorf("Variable '%s' does not exist",name1);
      *left = name1;
      *flag = 2;
    }
    else *left = name2;
    return(variable);
  }
  if (name2-name1+1 > MaxNameLength) {
    SetErrorf("Variable or field name is too long (you should increase MaxNameLength in int_variable.c");
    *flag = 1;
    *left = name1;
    return(NULL);
  }
  strncpy(field,name1,name2-name1);
  field[name2-name1] = '\0';    
  if (flagCreate) variable = GetSimpleVariableHT(*t,field,2);
  else variable = GetSimpleVariableHT(*t,field,0);
  if (variable == NULL) {
    SetErrorf("Variable '%s' does not exist",name1);
    *left = name1;
    *flag = 2;
    return(NULL);
  }
  *left = name2;
  
  while(1) {  

    /* Next character is a '.' */
    if (*name2 != '.') {
      *left = name2;
      *flag = 0;
      return(variable);
    }
    name2++;

    /* Getting the array corresponding to the variable */
    c = ValueOf((VALUE) variable);
    if (GetTypeValue(c) != arrayType) {
      SetErrorf("Expect array right before '%s'",name1);
      *left = name1;
      *flag = 1;
      return(NULL);
    }
    *t = ((HASHTABLE) c);

    /* Looking for the next field */
    name1 = name2;
    while (IsValidSymbolChar(*name2)) name2++;
    if (name2-name1+1 > MaxNameLength) {
      SetErrorf("Variable or field name is too long (you should increase MaxNameLength in int_variable.c");
      *flag = 1;
      *left = name1;
      return(NULL);
    }
    strncpy(field,name1,name2-name1);
    field[name2-name1] = '\0';    
    
    /* Getting the new variable */
    if (flagCreate) {
      if (*name2 == '.') variable = GetSimpleVariableHT(*t,field,2);
      else variable = GetSimpleVariableHT(*t,field,1);
    }
    else variable = GetSimpleVariableHT(*t,field,0);
    if (variable == NULL) {
      SetErrorf("Index array '%s' does not exist",field);
      *flag = 2;
      *left = name1;
      return(NULL);
    }
    
    /* Are we done ? */
    if (*name2 == '\0') {
      *left = name2;
      return(variable);
    }
  }
}

/* 
 * Same as above but at a given (eventually current) level
 *
 * Returns the variable if it exists or NULL if it does not
 *
 * If there is a syntax error --> Generate an error
 */ 

/* same as below but does not generate an error */
VARIABLE GetVariableLevel_(LEVEL level,char *name)
{
  VARIABLE v;
  HASHTABLE t;
  char *left,flag;
  
  while (level->levelVar != level) level = level->levelVar;
  t = level->theVariables;
  
  v = GetVariableHT(&t,0,name,&left,&flag);
  if (flag == 1 || *left != '\0') return(NULL); 

  return(v);  
}

VARIABLE GetVariableLevel(LEVEL level,char *name)
{
  VARIABLE v = GetVariableLevel_(level,name);
  if (v == NULL) Errorf1("");
  return(v);
}

/*
 * Same as above but at the current level
 */
VARIABLE GetVariable(char *name)
{
  return(GetVariableLevel(levelCur,name));
}

/*
 * Does the variable'name' of a given type exist at a given level ?
 *
 * The 'name' can be one of
 *    - a simple variable like 'me'
 *    - an array field like 'me.house.kitchen'
 */

VALUE ExistVariableLevel(LEVEL level, char *name, char *type)
{
  VARIABLE variable;
  VALUE value,*pcont;
  char *type1;
  char *left,flag;
  HASHTABLE t;
  
  /* Get the variable */
  while (level->levelVar != level) level = level->levelVar;
  t = level->theVariables;
  variable = GetVariableHT(&t,0,name,&left,&flag);
  if (variable == NULL || *left != '\0') return(NULL);

  variable = GetVariableLevel(level,name);
  
  /* If does not exist --> return */
  if (variable == NULL) return(NULL);
  
  /* Get a pointer to the variable content and the associated content */
  pcont = GetVariablePContent(variable,NULL);
  value = *pcont;
  
  /* If content is NULL --> return NULL */
  if (value == NULL) return(NULL);
  
  /* If type is not NULL we must check the type */
  if (type != NULL) {
    type1 = GetTypeValue(value);
    if (type1 == type) return(value);
    if (type1 == signaliType && type == signalType) return(value);
    if (type1 == imageiType && type == imageType) return(value);
    if (type1 == numType && type == floatType) return(value);
    if (type1 == numType && type == intType && ((NUMVALUE) value)->f == (int) ((NUMVALUE) value)->f) return(value);
    return(NULL);
  }
  
  return(value);
} 

/*
 * Same as above but at the current level
 */
VALUE ExistVariable(char *name,char *type)
{
  return(ExistVariableLevel(levelCur,name,type));
}


/*
 * Get the pointer to the variable's content (after following all the eventual indirections)
 */
VALUE *GetVariablePContent(VARIABLE variable, char *pflagTrace)
{
  if (variable == NULL) return(NULL);
  
  while (variable->content != NULL && IsVariable(variable->content)) {
    variable = (VARIABLE) variable->content;
  }
  
  return(&(variable->content));
} 

/*******************************************************************************
 *
 * Getting variable contents (this is different from evaluation)
 *
 * ?????????????? A SUPPRIMER
 */

VALUE GetVariableContentLevel_(LEVEL level, char *name, char *type) 
{
  Errorf("GetVariableContentLevel_ A Supprimer");
} 
VALUE GetVariableContentLevel(LEVEL level, char *name, char *type) 
{
  Errorf("GetVariableContentLevel A Supprimer");
}
VALUE GetVariableContent(char *name, char *type) 
{
  Errorf("GetVariableContent A Supprimer");
}


/********************************************************************************
 *
 *  Parsing variables  ?????????????
 * 
 * A SUPPRIMER
 *
 ********************************************************************************/

char ParseVariableLevel_(LEVEL level, char *name,VALUE def, VALUE *val,char *type, char flagTemp) 
{
  Errorf("ParseVariableLevel_ A Supprimer");
}  
void ParseVariableLevel(LEVEL level, char *name, VALUE *val,char *type, char flagTemp) 
{
  Errorf("ParseVariableLevel A Supprimer");
}  
void ParseVariable(char *name, VALUE *val,char *type, char flagTemp) 
{
  Errorf("ParseVariable A Supprimer");
}  
char ParseVariable_(char *name, VALUE def, VALUE *val,char *type, char flagTemp) 
{
  Errorf("ParseVariable_ A Supprimer");
}  
VARIABLE GetSimpleVariableHashTable(HASHTABLE t,char *name)
{
  Errorf("GetSimpleVariableHashTable A Supprimer");
}
VARIABLE CGetVariableLevel(LEVEL level,char *name)
{
  Errorf("CGetVariableLevel A Supprimer");
}
VARIABLE CGetVariable(char *name)
{
  Errorf("CGetVariable A Supprimer");
}

 

/****************************************************************************************
 *
 *
 * Functions to manage evaluation of a variable (with eventual fields/extraction
 *
 * 
 ****************************************************************************************/

#define Clean(val)  if ((val) != NULL) {(val) = NULL;}

extern char GetEventVariable2(char *name, char **str, float *f);
extern char * LookForBracketScript(char *theLine, char *line);

extern unsigned char TGetVariableContentLevelExpr(LEVEL level, char *begin, char **left, float *resFlt, VALUE *resVC, unsigned char flagType, unsigned char flagSimple, ExprDefParam *defParam);
extern char IsErrorMsge(void);


/* 
 * Send a GetField message to the content 'value' with the field 'field' the FSIList 'fsiList' and put the result
 * in f,str or val
 * It returns what the message returned
 */
static char *GetField(VALUE value, char *field, FSIList *fsiList,float *f,char **str, VALUE *val)
{
  void *arg[5];
  Field *fi;
  
  arg[0] = field;
  arg[1] = fsiList;
  arg[2] = f;
  arg[3] = str;
  *str = NULL;
  arg[4] = val;
  *val = NULL;
  
  if (value->ts == NULL) return((*(value->sendMessage))(value,GetFieldMsge,arg));
  
  while(value != NULL && IsVariable(value)) value = ((VARIABLE) value)->content;
  if (value == NULL) return(NULL);
  
  if (GetTrueTypeValue(value) == arrayType) return(GetFieldArray(value,arg));
    
  fi = FindField(value->ts->fields,arg[0]);
  if (fi==NULL) {
    SetErrorf("Bad field name '%s' for variable of type '%s'",arg[0],GetTypeValue(value));
    return(NULL);
  }
  return(GetFieldValue(*fi,value,arg));
}

/* 
 * Send a NumExtract message to the content 'value' with the number 'f' and the flagDot and put the result
 * in f,str or val
 * It returns what the message returned
 */
static char *NumExtract(VALUE value, float f, char flagDot, float *f1,char **str1, VALUE *vc1)
{
  void *arg[5];

  arg[0] = &f;
  arg[1] = &flagDot;
  arg[2] = f1;
  arg[3] = str1;
  *str1 = NULL;
  arg[4] = vc1;
  *vc1 = NULL;
  
  InitError();
  
  return(NumExtractValue(value,arg));
}


/*
 * Basic function to extract from a content a field using the . syntax 
 * or some values using the () syntax.
 *
 * The parameters are :
 *    
 *     - level   : the current level  
 *     - theCont : the content to extract/getfield from 
 *                 (the nref should include the pointer handled here which is supposed to be temporary)
 *
 *     - begin   : the string (starting by '.' or '[') corresponding to the command line
 *     - left    : a pointer in which will be returned the first character of the command line which was not read
 *     - resFlt  : the returned float if the result is float (which do not correspond to a string)
 *     - resVC   : the returned content if not a "pure" float (if this result is not used its content is deleted)
 *     - defParam : information for extraction (such as (1:))
 *
 * It returns the type of the corresponding value
 *
 */


unsigned char TGetFieldOrExtractContentExpr(LEVEL level, VALUE theCont, char *begin, char **left, float *resFlt, VALUE *resVC, unsigned char flagType, ExprDefParam *defParam, char flagDollar)
{
  static STRVALUE sc = NULL;
  static NUMVALUE nc = NULL;
  VALUE value;
  char *end,*name,*tempStr;
  float tempFlt;
  char *type,*type1;
  unsigned char answer;
  FSIList *list;
  ExtractInfo *ei,extractInfo;
  char **options;
  unsigned long optionFlag;
  char string[MaxNameLength];
  VALUE tempCont;
  void *arg[7];
  char flagNull;
  char oldFlagSpace;
  
  /* Allocation */
  if (sc == NULL) sc = NewNullStrValue();
  else sc->str = NULL;
  if (nc == NULL) nc = NewNumValue();

  /* Get the level the variable must be looked at */
  while (level->levelVar != level) level = level->levelVar;

  /* Init */
  value = theCont;
  type1 = NULL;
  type = GetTypeValue(theCont);
  
  /* 
   * Main Loop
   */
  end = begin;     
  name = NULL;
  oldFlagSpace = defParam->flagSpace;
  while (1) {
    
    defParam->flagSpace = oldFlagSpace;

    /* Are we done ? */
    if (*end != '[' && *end != '.') break;
    
    /* we are not in the case of a field extraction that has to be handled by an object */
    if (name == NULL) {
      /* Get type */
      type1 = type;
      type = GetTypeValue(value);    
    }
    
    /****************************
     *
     * Case of extraction 
     *
     ****************************/
    if (*end == '[') {

      defParam->flagSpace = YES;

      end++;

      ParamSkipSpace(end);
      if (name == NULL) {
        arg[0] = NULL;
        options = GetExtractOptionsValue(value,arg);
        if (options == NULL) {
          Clean(*resVC);
          if (!IsErrorMsge()) SetErrorf("No extraction possible");
          return(0);
        }
      }
      
      /* Get the extraction options if any */
      if (*end == '*') {
        if (options[0] == '\0') {
          Clean(*resVC);
          if (!IsErrorMsge()) SetErrorf("No option available for extraction");
          return(0);
        }
        if (FSIReadExtractOption(end,left,options,&optionFlag) == NO) {
          Clean(*resVC);
          return(0);
        }
        end = *left;
      }
      else optionFlag = 0;
  
      arg[0] = name;
      arg[1] = &optionFlag;
      ei = GetExtractInfoValue(value,arg);
      if (ei == NULL) {
        Clean(*resVC);
        if (!IsErrorMsge()) SetErrorf("No extraction possible");
        return(0);
      }
      
      /* Copy the info */
      extractInfo = *ei;
     
      /* Extract list */      
      ParamSkipSpace(end);
      flagNull = NO;
      if (*end != ']') {
        list = SFIListExpr(end, left, &extractInfo,defParam);
        if (list != NULL && list->nx == 0) flagNull = YES;
      }
      else list = SFIListExpr(end, left, &extractInfo,defParam);
      
      if (list==NULL) {
        Clean(*resVC);
        return(0);
      }
      if (!flagNull && list->nx==0) {
        DeleteFSIList(list);
        SetErrorf("Expecting [:] instead of []");
        *left = end-1;
        Clean(*resVC);
        return(0);
      }
      list->options = optionFlag;
           
      if ((**left)!=']') {
        DeleteFSIList(list);
        SetErrorf("Cannot find the matching bracket"); 
        Clean(*resVC);
        DeleteFSIList(list);
        *left = end;
        return(0);
      }
     
      (*left)=(*left)+1;
      end = *left;

 /*     if (flagNull) {
        value = nullValue;
        continue;
      }
 */
      
      /* Send an extract message and get the new VALUE */
      type = GetField(value,name,list,&tempFlt,&tempStr,&tempCont);
      if (tempCont) value = tempCont;
      else if (tempStr) {
        sc->str = NULL;
        SetStrValue(sc,tempStr);
        value = (VALUE) sc;
      }
      else if (type == numType) {
        SetNumValue(nc,tempFlt);
        value = (VALUE) nc;
      }
      else value = NULL;

      /* Delete the list */
      DeleteFSIList(list);
      
      /* Error if nothing */
      if (value == NULL) {
        *left = end;
        Clean(*resVC);
        return(0);
      }
      
      name = NULL;
    } 

    /****************************
     *
     * Case of a field 
     *
     ****************************/

    else {
   
      /* Begin is pointing to the first character of the field */
      begin = end+1;
      end = begin;
           
      /* 
       * Get the name of the field
       * Substitution case 
       */
      if (*end == '$' && defParam->flagSubst) {
        answer = TGetVariableContentLevelExpr(level,end,left,resFlt,resVC,StringType | FloatType,NO,defParam);
        if (answer == 0) return(0);
        end = *left;
        if (answer == FloatType) {
          Flt2Str(*resFlt,tempStr);
          name = tempStr;
        }
        else name = GetStrFromStrValue((STRVALUE) *resVC);
        *resVC = NULL;
        if (*name == '\0') {
          SetErrorf("Missing field name");
          return(0);
        }
      }
        
      /* 
       * Get the name of the field
       * No Substitution case 
       */
      else {
        if (IsValidSymbolChar(*end)) {
          end++;
          while (*end != '\0' && IsValidSymbolChar(*end) && *end != '.') end++;
        }
      
        /* If we didn't get anything --> error */
        if (end == begin) {
          *left = begin;
          Clean(*resVC);
          SetErrorf("Missing field name");
          return(0);
        }       

        /* Get the name of the field */
        if (MaxNameLength +2 < end-begin) Errorf("TGetFieldOrExtractContentExpr() : Weird Error : MaxNameLength is too short");
        strncpy(string,begin,end-begin);
        string[end-begin] = '\0';
        name = string;
      }
         
      /* 
       * If there is an extraction right after we must send an GetExtractInfoSignal message first
       * to check whether the field extraction will be handled by the object itself or by the
       * object associated to the field
       */
      if (*end == '[') {
        arg[0] = name;
        options = GetExtractOptionsValue(value,arg);
        if (options != NULL) continue;
      }

      /*
       * Let's get the field object 
       */
      type = GetField(value,name,NULL,&tempFlt,&tempStr,&tempCont);
      if (tempCont) value = tempCont;
      else if (tempStr) {
        sc->str = NULL;
        SetStrValue(sc,tempStr);
        value = (VALUE) sc;
      }
      else if (type == numType) {
        SetNumValue(nc,tempFlt);
        value = (VALUE) nc;
      }
      else value = NULL;

      /* Error if nothing */
      if (value == NULL) {
        *left = begin;
        Clean(*resVC);
        return(0);
      }
      
      name = NULL;
    }
         
  }

  /* 
   * End of loop
   * Now we must check the type
   */
   
  Clean(*resVC);
  *left = end;

  defParam->flagSpace = oldFlagSpace;

  /* Set the returned values */  
  if (type == signalType || type == signaliType) {answer = SignalType; *resVC = value;}
  else if (type == imageType || type == imageiType) {answer = ImageType; *resVC = value;}
  else if (type == listvType) {answer = ListvType; *resVC = value;}
  else if (type == rangeType) {answer = RangeType; *resVC = value;}
  else if (type == numType) {
    *resFlt = ((NUMVALUE) value)->f; 
    if (value != (VALUE) nc) *resVC = value;
    else *resVC = NULL;
    answer = FloatType;
  }
  else if (type == strType) {
    if (sc == (STRVALUE) value) {
      *resVC = (VALUE) NewStrValue();
      SetStrValue((STRVALUE) *resVC, sc->str);
      TempValue(*resVC);
    }
    else *resVC = value;
    answer = StringType;
  }
  else if (type == nullType) {
    *resVC = value;
    answer = NullType;
  }
  else {
    *resVC = value;
    answer = OtherType;
  }

  sc->str = NULL;
  
  ParamSkipSpace(*left);

  /* Then just return */
  return(answer);  
}


/*
 * Function to read 0a like syntax
 * begin[0] must be either '.' or a digit
 * The result is in resVC and is made temporary
 */
static unsigned char GetNumberExtraction(char *begin, char **left,  float *resFlt, VALUE *resVC, ExprDefParam *defParam)
{
  char flagDot;
  float f,tempFlt;
  char *end;
  unsigned char answer;
  VALUE value,tempCont;
  char *tempStr;
  char *type;
  
    if (begin[0] == '.') {begin++; flagDot = 1;}
    else flagDot = 0;
    f = strtol(begin,&end,10);
    if (end != begin) {
      /* case of 10a */
      if (IsValidSymbolChar1(*end)) {   
        begin = end;
        answer = TGetVariableContentLevelExpr(DefaultLevel(defParam),begin,left,resFlt,resVC,AnyType,YES,defParam);
        end = *left;
        if (answer == 0) {
          Clean(*resVC);
          return(0);
        }
        if (*resVC == NULL) {
          SetErrorf("Cannot perform number extraction on a float variable");
          *left = begin;
          return(0);
        }
      }
      /* case of 10 */
      else {
        *resVC = (VALUE) GetVariableLevel_(DefaultLevel(defParam),"objCur");
        if (*resVC != NULL) *resVC = ValueOf((VALUE) *resVC);
        if (*resVC == NULL) {
          SetErrorf("Variable ObjCur expected to be defined");
          *left = begin;
          Clean(*resVC);
          return(0);
        }
      }      
      value = *resVC;
      type = NumExtract(value,f, flagDot, &tempFlt,&tempStr,&tempCont);
      if (type == NULL) {
        Clean(*resVC);
        if (!IsErrorMsge()) SetErrorf("Bad syntax");
        *left = begin;
        return(0);
      }
      if (tempCont != NULL) {
        AddRefValue(tempCont);
        TempValue(tempCont);
        *resVC = tempCont; 
        *left = end;
        return(1);
      }
      else if (tempStr != NULL) {
        tempCont = (VALUE) NewNullStrValue();
        TempValue(tempCont);
        SetStrValue((STRVALUE) tempCont, tempStr);
        *resVC = tempCont; 
        *left = end;
        return(1);
      }
      else {
        tempCont = (VALUE) NewNumValue();
        TempValue(tempCont);
        SetNumValue((NUMVALUE) tempCont, tempFlt);
        *resVC = tempCont; 
        *left = end;
        return(1);
      }
    }
    else {
      SetErrorf("Do not expect a number");
      *left = begin;
      Clean(*resVC);
      return(0);
    }
}

 

/*
 * Function to get the content of a variable
 *
 *   'level' : the level it looks the variable at
 *   'begin', 'left' : the usual parsing strings
 *   'resFlt', 'resVC' : the results (if resVC is NULL then the result is a float in resFlt, if not and if the result is a float 
 *                       the result is in both variable)
 *   'flagType' : we are looking for this type
 *   'flagSimple' : we just look for a simple variable (not a.dx or a[1])
 *   'defParam' : the current extraction parameter (could be NULL)
 *
 */

extern char ParseCompiledScript_(char **theScript, SCRIPT *pScript, char flagSubst, char flagBrace, char *flagErr);
extern char * LookForBracketList(char *theLine, char *line);
extern SPROC SetProc(char *name, char **list, char **help, SCRIPT script);

unsigned char TGetVariableContentLevelExpr(LEVEL level, char *begin, char **left, float *resFlt, VALUE *resVC, unsigned char flagType, unsigned char flagSimple, ExprDefParam *defParam)
{
extern void EvalScriptLevel(LEVEL level, SCRIPT script,char flagStoreResult);
  static NUMVALUE nc = NULL;
  STRVALUE sc;
  VARIABLE var;
  VALUE value,*pcont;
  char *end,*type,**pos,*str,*tempStr,flag,**list;
  HASHTABLE ht;
  char flagDollar,flagBrace,flagEventVar,flagEval,flagProc,flagProcAn,flagScript;
  char flagErr;
  unsigned char answer;
  int i,j,i0,k;
  SCRIPT script;
  SCRIPTLINE sl;
  float f;
  char string[MaxNameLength];
  char oldFlagSpace;
  char oldFlagEmptySI;
  
  /* Init */
  Clean(*resVC);
  sc = NULL;
  value = NULL;
  *left = begin;
  oldFlagEmptySI = defParam->flagEmptySI;
  defParam->flagEmptySI = YES;
  if (nc == NULL) nc = NewNumValue();

    
  /* Get the level the content must be looked at */
  while (level->levelVar != level) level = level->levelVar;
  ht = level->theVariables;
  
  /*
   * Substitution ?
   * Look for $ or ${}
   */
  flagDollar = NO;
  flagBrace = NO;
  if (*begin == '$') {
    if (defParam->flagSubst) {
      flagDollar = YES;
      if (begin[1] == '{') {flagBrace = YES; begin+=2;}
      else {begin++;}
    }
    else {
      Clean(*resVC);
      SetErrorf("Syntax error");
      return(0);
    }
  }
  
  /* set end to begin */
  end = begin;


  /*************************************************************************
   *
   * We want to get the content 'value' and have it temporary referenced
   *
   *************************************************************************/
    
  
  /*
   * Event variable ?
   * Look for @
   */
  if (*begin == '@') {flagEventVar = YES; end++;}
  else flagEventVar = NO;
   
  /*
   * Pointer variable
   */
   if (*begin == '&') {
     if (!(flagType & OtherType)) {
       SetErrorf("Do not expect '&variable' type");
       *left = begin;
       Clean(*resVC);
       return(0);
     }
     begin++;
     var = GetVariableHT(&ht,0,begin,left,&flag);
     if (flag != 0) {
       Clean(*resVC);
       return(0);
     }
     *resVC = (VALUE) var;
     var->nRef++;
     TempValue( var);
     return(OtherType);
   }
   
  /*
   * % procedure
   */
   flagProcAn = NO;
   if (*begin == '%' && begin[1] != '%') {
     if (!(flagType & OtherType)) {
       SetErrorf("Do not expect '&proc' type");
       *left = begin;
       Clean(*resVC);
       return(0);
     }
     flagProc = YES;
     end++;
     if (*end == '{') flagProcAn = YES;
     begin++;
   }
   else flagProc = NO;   

  /*
   * %%script
   */
   if (*begin == '%' && begin[1] == '%') {
     if (!(flagType & OtherType)) {
       SetErrorf("Do not expect '&script' type");
       *left = begin;
       Clean(*resVC);
       return(0);
     }
     flagScript = YES;
     end+=2;
     begin+=2;
   }
   else flagScript = NO;   
     
  /*
   * Command ?
   * Look for []
   */
  if (*begin == '[') {flagEval = YES; end++;}
  else flagEval = NO;
  
  /* 
   * If not a command line and not a script get the variable name.
   */  
  if (flagEval == NO && flagScript==NO && !flagProcAn) {
    if (IsValidSymbolChar1(*end) || *end == '\\' && flagProc) {
      end++;
      while (*end != '\0' && IsValidSymbolChar(*end) && *end != '.') end++;
    }
    /* If we didn't get anything --> error */
    if (end == begin && !isdigit(*begin) && *begin != '.') {
      *left = begin;
      SetErrorf("Syntax Error");
      return(0);
    }
    
    /* The name/line will just correspond to begin */
    if (MaxNameLength +2 < end-begin) Errorf("TGetVariableContentLevelExpr() : Variable name is too long");
    strncpy(string,begin,end-begin);
    string[end-begin] = '\0';
  }


  /* 
   * Get the variable if 10a or 10
   */ 
  if (isdigit(begin[0]) || begin[0]=='.') {
    answer = GetNumberExtraction(begin,left,resFlt,resVC,defParam);
    if (answer==0) return(0);
    value = *resVC;
    end = *left;
  }
  
  /* 
   * Get procedure
   */ 
  else if (flagProc) {
    /* Case of an anonymous command */
    if (flagProcAn == YES) {
      list = NULL;
      script = NULL;
      str= LookForBracketList(NULL,begin);
      if (str != NULL) {
        *str = '\0';
        ParseWordList_(begin+1,NULL,&list);
        *str = '}';
        if (list != NULL) {
          str++;
          ParseStrScript_(&str,NULL,&script);
        }
      }
      if (script == NULL) {
        *left = begin;
        Clean(*resVC);
        return(0);
      }
      Clean(*resVC);
      *resVC = (VALUE) SetProc(NULL,list,NULL,script);
      *left = begin+strlen(begin);
      return(OtherType);
    }
    /* Other case */
    else {
      if (*string == '\\') value = (VALUE) GetCProc(string+1);
      else value = (VALUE) GetProc(string);
      if (value == NULL) {
        SetErrorf("Unknown procedure name '%s'",string);
        *left = begin;
        Clean(*resVC);
        return(0);
      }     
      AddRefValue(value);
      TempValue(value);
    }
  }

  /* 
   * Get script
   */ 
  else if (flagScript) {
    script = NULL;
    if (!ParseCompiledScript_(&end,&script,NO,NO,&flagErr)) {
      *left = begin;
      Clean(*resVC);
      return(0);
    }
    *resVC = (VALUE) script;
    *left = end+strlen(end);
    return(OtherType);
  }
  
  /* 
   * Get the variable if regular variable (add a ref)
   */ 
  else if (!flagEventVar && !flagEval) {
    var = GetSimpleVariableHT(ht,string,0);
    pcont = GetVariablePContent(var,NULL);   
    if (pcont == NULL || *pcont == NULL) {
      SetErrorf("Variable does not exist");
      *left = begin;
      return(0);
    }
    value = *pcont;
    AddRefValue(value);
    TempValue(value);
  }


  /* 
   * Get the variable if event variable (store it in nc or sc)
   */ 
  else if (!flagEval) {
    flag = GetEventVariable2(string+1,&tempStr,&f);
    if (flag == NO) {
      SetErrorf("Bad event variable");
      *left = begin;
      return(0);
    }
    if (tempStr != NULL) {
      sc = NewStrValue();
      SetStrValue(sc,tempStr);
      value = (VALUE) sc;
      TempValue(value);
   /*   Warningf("On peut accelerer ca...\n"); *//* ????? */
    }
    else {
      SetNumValue(nc,f);
      value = (VALUE) nc;
    }
  }

  
  /* 
   * Get the variable if command line 
   */ 
  else {
    
    /* Try to find the compiled script */
    script = NULL;
    if (levelCur->scriptline && levelCur->scriptline->cs != NULL) {
      sl = levelCur->scriptline;
      if (defParam->nWord == -1) {i0=0; j = 0; k=0;}
      else {i0 = defParam->nWord; j = defParam->nPos; k = defParam->nScript;}
      for (i=i0;i<sl->nWords;i++) {
        if (sl->cs[i] == NULL || sl->cs[i]->pos.bracket == NULL) {j=k=0;continue;}
        for (pos = sl->cs[i]->pos.bracket+j; *pos != NULL; j++, pos++) {
          if (!flagDollar) { /* Loop when it is not a $[..] script line */
            if (*pos > begin || **pos == '$') break;
            if (*pos < begin) {
              k++;j++;pos++;
              continue;
            }
          }
          else {  /* Loop when it is a $[..] script line */
            if (*pos > begin-1 || **pos == '[') break;
            if (*pos < begin-1) {
              if (*(*pos+1) == '[') {k++;j++;pos++;}
              continue;
            }
          }          

          /* We found it !! */          
          defParam->nWord = i;
          defParam->nPos = j;
          defParam->nScript = k;
          script = sl->cs[i]->scripts.bracket[k];
          break;
        }
        j=k=0;
        if (script || *pos != NULL) break;
      }
    }

    /* Case we could not find any compiled script */    
    if (script == NULL) {
      /* Get the script line */
/*      Printf("TGetVariableContentLevelExpr() : Pas de compil\n"); */
      end = LookForBracketScript(begin,end-1);
      if (end == NULL) {
        *left = begin;
        return(0);
      }
      /* Get the script ??????????? attention peut etre pb !!!!!  
        ++ FAIRE UNE COPIE au cas ou chaine est statique */
      *end = '\0';
      str = begin+1;
      if (ParseStrScript_(&str,NULL,&script) != YES) {
        *end = ']';
        return(0);
      }
      *end = ']';
      EvalScriptLevel(level,script,YES); /* ??? mettre ici le levelVar --> ouvrir un nouveau level */
    }
    
    /* Case we found a compiled script */
    else {
      EvalScriptLevel(level,script,YES);
      end = sl->cs[defParam->nWord]->pos.bracket[defParam->nPos+1];
    }
    
    type = GetResultType();
    if (type == numType) {
      SetNumValue(nc,GetResultFloat());
      value = (VALUE) nc;
    }
    else {
      value = GetResultValue();
      AddRefValue(value);
      TempValue(value);
    }
    end++;
    InitResult();
  }
     
    
  /*
   * Manage extraction/getfield messages if any and get the type of the so-obtained variable
   */
  if (!flagSimple && (*end == '[' || *end == '.')) {
    answer = TGetFieldOrExtractContentExpr(level,value,end,left,resFlt,resVC,flagType,defParam,flagDollar);
    if (answer != 0) {
      if (*resVC == NULL) type = floatType;
      else type = GetTypeValue(*resVC);
    }
    else { 
      Clean(*resVC);
      return(0);
    }
  }
  
  /*
   * Otherwise just get the type
   */
  else {
    *left = end;
    type = GetTypeValue(value);
    if (type == numType) {
      *resFlt = ((NUMVALUE) value)->f;
      if (value != (VALUE) nc) *resVC = value;
      else *resVC = NULL;
      answer = FloatType;
    }
    else {
      *resVC = value;
      if (type == strType) answer = StringType;
      else if (type == signalType || type == signaliType) answer = SignalType;
      else if (type == imageType || type == imageiType) answer = ImageType;
      else if (type == listvType) answer = ListvType;
      else if (type == rangeType) answer = RangeType;
      else if (type == nullType) answer = NullType;
      else answer = OtherType;
    }
  }
  
  /*
   * We are ready to perform some checkings
   */  
  if (flagBrace) {
    if (**left != '}') {
      Clean(*resVC);
      SetErrorf("Missing the corresponding '}'");
      return(0);
    }
    else *left += 1;
  }
  defParam->flagEmptySI =  oldFlagEmptySI;

  if (!(answer  & flagType)) {
    *left = begin;
    Clean(*resVC);
    SetErrorf("Do not expect type '%s'",type);
    return(0);
  }

  if (type == signalType && !defParam->flagEmptySI) {
    *left = begin;
    Clean(*resVC);
    SetErrorf("Signal is empty");
    return(0);
  }
  if (type == imageType && !defParam->flagEmptySI) {
    *left = begin;
    Clean(*resVC);
    SetErrorf("Image is empty");
    return(0);
  }

  ParamSkipSpace(*left);
  
  /* Just return */
  return(answer);
}


/*
 * Basic function for getting variable's string value and making substitutions (with the $ sign)
 * 
 * We expect begin to start with $
 *
 * 'nw' is the word number (-1 if none) 
 *
 * Returns a temporary string
 */
 
char *GetStringExpr(LEVEL level, char *begin, char **left, int nw)
{  
  float resFlt;
  VALUE resVC;
  unsigned char type;
  char *str;
  ExprDefParam defParam;
  
  InitDefaults(&defParam,level,YES,NO);
  if (nw >= 0) {
    defParam.nWord = nw;
    defParam.nScript = 0;
    defParam.nPos = 0;
    defParam.flagSpace = NO;
  }
    
  /* Evaluation */
  resVC = NULL;
  type = TGetVariableContentLevelExpr(level, begin, left, &resFlt, &resVC, AnyType,NO,&defParam);
  
  /* Inits again */
  InitDefaults(&defParam,NULL,NO,NO);

  if (type == 0) {
    Clean(resVC);
    return(NULL);
  }
  
  if (type == StringType) {
    str = GetStrFromStrValue((STRVALUE) resVC);
    return(str);
  }
  else if (type == FloatType) {
    str = CharAlloc(30);
    TempPtr(str);  
    Flt2Str(resFlt,str);
    return(str);
  }
  else {
    SetErrorf("Bad variable type");
    Clean(resVC);
    return(NULL);
  }

}  


/********************************************************************************
 *
 *  Setting variables 
 *
 ********************************************************************************/

void SetVariableLevel(LEVEL level,char *name,VALUE content)
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
  if (!DoesTypeOverwrite(type,GetTypeValue(content))) 
    Errorf("SetVariableLevel() : Cannot overwrite variable '%s' of type '%s' with '%s' typed value",name,type,GetTypeValue(content));

  /* Delete the content */
  DeleteValue(*pcont);
  
  /* And sets it */
  *pcont = content;
  AddRefValue(content);
}
   
void SetVariable(char *name,VALUE content)
{
  SetVariableLevel(levelCur,name,content);
}


/* 
 * Send a SetField message to the content 'value' with the field 'field' the FSIList 'fsiList' and put the result
 * It returns what the message returned
 */
void *SetFieldArg(VALUE value, void **arg)
{
  Field *f;

  if (value->ts == NULL) return((*(value->sendMessage))(value,SetFieldMsge,arg));
  if (IsVariable(value)) return(SetFieldVar(value,arg));

  if (GetTrueTypeValue(value) == arrayType) Errorf("SetField() : weird");
    
  f = FindField(value->ts->fields,arg[0]);
  if (f == NULL) {
    SetErrorf("Bad field name '%s'",arg[0]);
    return(NULL);
  }
  if (f->Set == NULL) {
    SetErrorf("Read only field '%s'",arg[0]);
    return(NULL);
  }
  return(SetFieldValue(*f,value,arg));
}

static void *SetField(VALUE value, char *field, FSIList *fsiList,char *type, float fltSet, VALUE vcSet, char *equal, VALUE *pValueRes, float *pfltRes, char **pstrRes)
{
  void *arg[9];
  
  arg[0] = field;
  arg[1] = fsiList;
  arg[2] = type;
  arg[3] = &fltSet;
  arg[4] = &vcSet;  
  arg[5] = equal;
  arg[6] = pValueRes;
  arg[7] = pfltRes;
  arg[8] = pstrRes;

  return(SetFieldArg(value,arg));  
}


/*
 * Subroutine to set the content of a variable
 *
 */
static unsigned char SetVariableLevelExpr_(char *equal, LEVEL level, VALUE value, char *begin, char **left, char *type, float fltSet, VALUE vcSet, ExprDefParam *defParam, VALUE *pValueRes, float *pfltRes, char **pstrRes)
{
  static STRVALUE sc = NULL;
  char *field, *begin1;
  char *type1;
  char name[MaxNameLength];
  float f1;
  VALUE vc1;
  char *str1;
  void *arg[7];
  ExtractInfo *ei,extractInfo;
  unsigned long optionFlag;
  FSIList *list;
  void *answer;
  char **options;
  char flagNull;

/* 
 * Main loop 
 */
 
while (1) 
{
  begin1 = begin;
  
  /* Get the type of the current content */
  type1 = GetTypeValue(value);
  
  /* 
   * case 'value[]'
   */
  if (*begin == '[') {
  
    /* must get the extract options */
    begin++;
    ParamSkipSpace(begin);
    
    /* We must get the extract info */
    arg[0] = NULL;
    options = GetExtractOptionsValue(value,arg);
    if (options == NULL) {
      SetErrorf("No extraction possible on '%s'",type1);
      return(0);
    }
    if (*begin == '*') Errorf("Sorry no *options yet for setting variables");
    optionFlag = 0;
    arg[1] = &optionFlag;
    ei = GetExtractInfoValue(value,arg);
    if (ei == NULL) {
      if (!IsErrorMsge()) SetErrorf("No extraction possible");
      return(0);
    }
    
    /* Copy the info */
    extractInfo = *ei;     
 
    /* Extract list */
    ParamSkipSpace(begin);
    flagNull = NO;
    if (*begin != ']') {
      list = SFIListExpr(begin, left, &extractInfo,defParam);
      if (list == NULL || list->nx == 0) flagNull = YES;
    }
    else list = SFIListExpr(begin, left, &extractInfo,defParam);
    if (list==NULL) return(0);
    list->options = optionFlag;

    /* Read the matching bracket */   
    if ((**left)!=']') {
      SetErrorf("Cannot find the matching bracket"); 
      DeleteFSIList(list);
      *left = begin;
      return(0);
    }
    begin = *left;
    begin++;
    ParamSkipSpace(begin);

    /* If no '.' or no '[' after then send a SetField message */
    if (*begin != '.' && *begin != '[') {
      if (flagNull == YES) {
        *pValueRes = value;
        DeleteFSIList(list);
        return(1);
      }
      answer = SetField(value,NULL,list,type,fltSet,vcSet,equal,pValueRes,pfltRes,pstrRes);
      if (answer == NULL) *left = NULL;    
      DeleteFSIList(list);
      return(answer!=NULL);
    }
    
    /* Case of value[]. or value[][] */
    else {
      if (flagNull == YES) {
        *pValueRes = nullValue;
        type1 = nullType;
      }
      else type1 = GetField(value,NULL,list,pfltRes,pstrRes,pValueRes);
      if (*pstrRes == NULL && *pValueRes == NULL) {
        DeleteFSIList(list);
        *left = begin;
        SetErrorf("No extraction or field allowed to numbers");
        return(0);
      }
      if (*pstrRes != NULL) {
        DeleteFSIList(list);
        *left = begin;
        SetErrorf("Sorry, cannot manage this case of extraction (as left value)");
        return(0);
      }
      value = *pValueRes;
      *pValueRes = NULL;
      DeleteFSIList(list);
      continue;
    }
    
  }
  
  /* 
   * case 'value.'
   */
  if (*begin == '.') {
    begin++;
    
    /* 
     * Get the field name 
     */
    field = begin;
    while (IsValidSymbolChar(*begin)) begin++;
    
    /* 
     * Case value.field[]
     */
    if (*begin == '[') {
      begin1 = begin;
      /* Get the name of the field in the variable 'name' */
      if (begin-field+1 > MaxNameLength) Errorf("SetVariableLevelExpr() : field name is too long");
      strncpy(name,field,begin-field);
      name[begin-field] = '\0';

      /* must get the extract options */
      begin++;
      ParamSkipSpace(begin);
      arg[0] = name;
      options = GetExtractOptionsValue(value,arg);
      if (*begin == '*') Errorf("Sorry no *options yet for setting variables");
    
      /* We must get the extract info : we have to understand who to send the message to */
      arg[0] = name;
      optionFlag = 0;
      arg[1] = &optionFlag;

      /* If no options then it means we have to get the field itself and the corresponding extractInfo */
      if (options == NULL) {
        if((type1 = GetField(value,name,NULL,&f1,&str1,&vc1)) == NULL) return(0);
        if (vc1 == NULL && str1 == NULL) {
          SetErrorf("No extraction for a number");
          return(0);
        }
        
        /* Get the extract info of the field */
        if (vc1 == NULL) {
          if (sc == NULL) sc = NewNullStrValue();
          sc->str = str1;
          vc1 = (VALUE) sc;
          arg[0] = NULL;
          ei = GetExtractInfoValue(vc1,arg);
          field = name;
          value = vc1;
        }
        else {
          arg[0] = NULL;
          field = NULL;
          value = vc1;
          options = GetExtractOptionsValue(value,arg);
          if (options == NULL) {
            SetErrorf("No extraction for '%s'",GetTypeValue(value));
            return(0);
          }
          ei = GetExtractInfoValue(vc1,arg);
        }
      }
      /* Otherwise we get the extract info directly */
      else {
        ei = GetExtractInfoValue(value,arg);
        field = name;
      }

      if (ei == NULL) {
        if (!IsErrorMsge()) SetErrorf("No extraction possible");
        return(0);
      }

      extractInfo = *ei;
        
      /* Build the corresponding fsiList */
      ParamSkipSpace(begin);
      flagNull = NO;
      if (*begin != ']') {
        list = SFIListExpr(begin, left, &extractInfo,defParam);
        if (list == NULL || list->nx == 0) flagNull = YES;
      }
      else list = SFIListExpr(begin, left, &extractInfo,defParam);
 
      if (list==NULL) return(0);
      list->options = optionFlag;

      /* Skip the ']' */
      if (**left != ']') {
        *left = begin1;
        DeleteFSIList(list);
        SetErrorf("Missing the corresponding ']'");
        return(0);
      }
      *left +=1;
      
      /* If there is a [ or a . after --> performs a get and continue */
      if (**left == '[' || **left == '.') {
        if (value == (VALUE) sc) {
          DeleteFSIList(list);
          SetErrorf("Syntax error");
          return(0);
        }
        if (flagNull) {
          vc1 = nullValue;
          type1 = nullType;
        }
        else if ((type1 = GetField(value,field,list,&f1,&str1,&vc1)) == NULL) return(0);
        if (vc1 == NULL && str1 == NULL) {
          DeleteFSIList(list);
          SetErrorf("No extraction or field for a number");
          return(0);
        }
        if (str1 != NULL) {
          DeleteFSIList(list);
          SetErrorf("Case not implemented yet");
          return(0);
        }
        value = vc1;
        begin = *left;
        DeleteFSIList(list);
        continue;
      }
        
      /* It should be the end of the expression */
      if (**left != '\0') {
        DeleteFSIList(list);
        SetErrorf("End of right handside expression expected");
        return(0);
      }
        
      /* We have to send the set message */
      if (flagNull) {
        DeleteFSIList(list);
        *pValueRes = value;
        return(1);
      } 
      answer = SetField(value,field,list,type,fltSet,vcSet,equal,pValueRes,pfltRes,pstrRes);
      if (answer == NULL) *left = NULL;
      DeleteFSIList(list);
      return(answer!=NULL);
    }
   
    /* 
     * Case value.field
     */    
    else if (*begin == '\0') {
    
      /*
       * case value is an array
       */
      if (type1 == arrayType) {
        /* We get the element named 'field' in the array 'value' */
        value = (VALUE) GetSimpleVariableHT((HASHTABLE) value,field,*equal=='=');
        if (value == NULL) {
          *left = field;
          SetErrorf("field does not exist");
          return(0);
        }
        /* And then we set it with the value --> RETURN */
        answer = SetField(value,NULL,NULL,type,fltSet,vcSet,equal,pValueRes,pfltRes,pstrRes);
        if (answer == NULL) *left = NULL;
        return(answer!=NULL);
      }
      
      /*
       * case value is not an array --> we just send a setfield message --> RETURN 
       */      
      else {
        answer = SetField(value,field,NULL,type,fltSet,vcSet,equal,pValueRes,pfltRes,pstrRes);
        if (answer == NULL) *left = NULL;
        return(answer!=NULL);
      }
    }
    
    /*
     * Case value.field.
     */
    else if (*begin == '.') {

      /* Get the name of the field in the variable 'name' */
      if (begin-field+1 > MaxNameLength) Errorf("SetVariableLevelExpr() : field name is too long");
      strncpy(name,field,begin-field);
      name[begin-field] = '\0';
      
      /*
       * Case 'value' is an array 
       */
      if (type1 == arrayType) {
        
        /* Get the element named 'field' in the array 'value' and creates an array if it does not exist */
        value = (VALUE) GetSimpleVariableHT(((HASHTABLE) value),name, 2*(*equal=='='));
        if (value == NULL) {
          *left = field;
          SetErrorf("Field does not exist");
          return(0);
        }
        
        /* Set the new value --> LOOP */
        value = ValueOf(value);
        continue;
      }
      
      /*
       * Case 'value' is not an array 
       */
      else {
        /* Get the field 'field' of 'value' and its type */
        if((type1 = GetField(value,name,NULL,&f1,&str1,&vc1)) == NULL) return(0);

        /*
         * We must set the new 'value' wit hthe value of the field 
         */
         
        /* Case we need to create a numcontent  */
        if (type1 == numType && vc1 == NULL) {
          value = (VALUE) TNewNumValue();
          SetNumValue((NUMVALUE) value,f1);
        }

        /* Case we need to create a strcontent  */
        else if (type1 == strType && vc1 == NULL) {
          value = (VALUE) TNewStrValue();
          SetStrValue((STRVALUE) value,str1);
        }
        
        /* Othercases */
        else value = vc1;
        
        /* --> LOOP */
        continue;
      }
    }
    else {
      *left = begin;
      SetErrorf("Syntax error");
      return(0);
    }
  }
}   
}


/*
 * Function to set the content of a variable
 */

unsigned char SetVariableLevelExpr(char *equal, LEVEL level, char *begin, char **left, char *type, float fltSet, VALUE vcSet, ExprDefParam *defParam, VALUE *pValueRes, float *pfltRes, char **pstrRes)
{
  VARIABLE var;
  char *begin1;
  char name[MaxNameLength];
  void *res;
  float f;
  unsigned char answer;
  VALUE val;
  
  /* skip space */
  while (*begin==' ') begin++;
  begin1 = begin;

  /* Case of number extraction 10a */
  if (*begin == '.' || isdigit(*begin)) {
    val = NULL;
    answer = GetNumberExtraction(begin, left, &f, &val,defParam);
    if (answer == NO) return(NO);
    begin = *left;
    if (*begin == '\0') {
      res=SetField(val,NULL,NULL,type,fltSet,vcSet,equal,pValueRes,pfltRes,pstrRes);
      if (res == NULL) {*left = NULL; return(NO);} else return(YES);   
    }
    if (*begin != '.' && *begin != '[') {
      SetErrorf("Syntax error");
      return(NO);
    }
    return(SetVariableLevelExpr_(equal,level,val,begin,left,type,fltSet,vcSet,defParam,pValueRes,pfltRes,pstrRes));
  }

  /* We read the first variable name */
  if (*begin == '%') {
    begin++;
    if (*begin == '\\') begin++;
  }
  if (IsValidSymbolChar1(*begin)) {
    begin++;
    while (IsValidSymbolChar(*begin)) begin++;
  }
  if (*begin != '[' && *begin != '.' && *begin != '\0' || begin == begin1) {
    SetErrorf("Bad variable name");
    *left = begin1;
    return(NO);
  }
    
  /* Get the variable if nothing after its name */
  if (*begin == '\0') {
    if (*begin1 == '%') {
      *left = begin1;
      SetErrorf("Cannot set a procedure");
      return(0);
    }
    /* We get the variable */
    var = GetSimpleVariableHT(level->theVariables,begin1,*equal=='=');
    if (var == NULL) {
      *left = begin1;
      SetErrorf("Variable does not exist");
      return(0);
    }
    /* And set the variable */
    res=SetField((VALUE) var,NULL,NULL,type,fltSet,vcSet,equal,pValueRes,pfltRes,pstrRes);
    if (res == NULL) {*left = NULL; return(NO);} else return(YES);    
  }


  /* Get the name of the variable in the variable 'name' */
  if (begin-begin1+1 > MaxNameLength) Errorf("SetVariableLevelExpr() : variable or field name too long");
  strncpy(name,begin1,begin-begin1);
  name[begin-begin1] = '\0';
  
  if (begin1[0] == '%') {
    if (*name == '\\') var = (VARIABLE) GetCProc(name+2);
    else var = (VARIABLE) GetProc(name+1);
    if (var == NULL) {
      SetErrorf("Unknown procedure name '%s'",name+1);
      *left = begin1;
      return(0);
    }     
  }

  /* Case the variable name is followed by [] */
  if (*begin == '[') {
    /* Get the variable (returns NULL if it does not exist) */
    if (begin1[0] != '%') {var = GetSimpleVariableHT(level->theVariables,name,0);}
    if (var == NULL) {
      SetErrorf("Undefined variable");
      *left = begin1;
      return(NO);
    }
  }
  else {
    /* Get the variable (if it does not exist create an array) */
    if (begin1[0] != '%') {var = GetSimpleVariableHT(level->theVariables,name,2*(*equal=='='));}
    if (var == NULL)  {
      *left = begin1;
      SetErrorf("Variable does not exist");
      return(0);
    }
  }
     
  /* We just call the routine that deals with extraction and fields */    
  return(SetVariableLevelExpr_(equal,level,ValueOf((VALUE) var),begin,left,type,fltSet,vcSet,defParam,pValueRes,pfltRes,pstrRes));
}


/*
 * The command for setting/getting string variables 
 */
void C_Set(char **argv)
{
  char *name,*val;
  STRVALUE sc;
  
  argv = ParseArgv(argv,tWORD,&name,tWORD,&val,0);
  
  sc = TNewStrValue();
  SetStrValue(sc,val);
  
  SetVariable(name,(VALUE) sc);
}

void C_SetVar(char **argv)
{
  Errorf("C_SetVar A Supprimer");
}



/*
 * Import a value which must be of type 'type' which is either defined by the VALUE 'val' or 
 * (if val == NULL) the float 'f'. The value has to be imported in 'level' using the name 'name'.
 * 
 * It returns YES if it succeeded or NO if not 
 */

int ImportFromValue(char *type, VALUE val, float f, LEVEL level,char *name) 
{
  char *type1;
  STRVALUE sc;

  if (type == NULL) type = valType;
  if (type == wordType || type == wordlistType) {
    SetErrorf("ImportFromValue() : bad type '%s'",type);
    return(NO);
  }
  
  if (val) type1 = GetTypeValue(val);
  else type1 = numType;
  
  /* Case we need to assign a val  */
  if (type == valType) {
    if (type1 == numType) {
      if (val) f = ((NUMVALUE) ValueOf(val))->f;
      SetNumVariableLevel(level,name,f);
    }
    else if (type1 == strType) {
      SetStrVariableLevel(level,name,((STRVALUE) ValueOf(val))->str);
    }      
  	else SetVariableLevel(level,name,val);
  	return(YES);
  }

  /* Case we need to assign a valobj  */
  if (type == valobjType) {
  	if (val) {
      if (type1 == numType) f = ((NUMVALUE) ValueOf(val))->f;
      else if (type1 == strType) {
        SetStrVariableLevel(level,name,((STRVALUE) ValueOf(val))->str);
      }      
  	  else SetVariableLevel(level,name,val);
  	}
  	else {
  	  SetErrorf("ImportFromValue() : cannot convert a numberto a &valobj");
  	  return(NO);
  	}
  	return(YES);
  }

  /* Case we need to assign a string  */
  if (type == strType) {
  	if (type1 != strType) {
  	  SetErrorf("ImportFromValue() : Cannot convert '%s' to '%s'",type1,type);
  	  return(NO);
    } 
    SetStrVariableLevel(level,name,((STRVALUE) val)->str);
    return(YES);
  }

  /* Case we need to assign a list  */
  if (type == listType) {
  	if (type1 != strType) {
  	  SetErrorf("ImportFromValue() : Cannot convert '%s' to '%s'",type1,type);
  	  return(NO);
  	}
    sc  = TNewStrValue();
    CopyStrValue((STRVALUE) val, sc);
    SetVariableLevel(level,name,(VALUE) sc);
    return(YES);
  }

  /* Case we need to assign a number  */
  if (type == numType || type == floatType || type == intType) {
  	if (type1 != numType) {
  	  SetErrorf("ImportFromValue() : Cannot convert '%s' to '%s'",type1,type);
  	  return(NO);
  	}
    if (val) f = ((NUMVALUE) val)->f;
    if (type == intType && f != (int) f) {
      SetErrorf("ImportFromValue() : Cannot convert '%g' to '%s'",f,intType);
      return(NO);
    }
    SetNumVariableLevel(level,name,f);
    return(YES);
  }

  /* Case we need to import a signal */
  if (type == signalType || type == signaliType) {
    if (val == NULL) {
      SetErrorf("ImportFromValue() : Cannot convert the float '%g' to '%s'",f,type);
      return(NO);
    }
    type1 = GetTypeValue(val);
    if (type == signalType && (type1 == signalType || type1 == signaliType)) {
      SetVariableLevel(level,name,(VALUE) val);
      return(YES);
    }
    if (type == signaliType && type1 == signaliType) {
      SetVariableLevel(level,name,(VALUE) val);
      return(YES);
    }
    SetErrorf("ImportFromValue() : Cannot convert '%s' to '%s'",type1,type);
    return(NO);
  }

  /* Case we need to import an image */
  if (type == imageType || type == imageiType) {
    if (val == NULL) {
      SetErrorf("ImportFromValue() : Cannot convert the float '%g' to '%s'",f,type);
      return(NO);
    }
    type1 = GetTypeValue(val);
    if (type == imageType && (type1 == imageType || type1 == imageiType)) {
      SetVariableLevel(level,name,(VALUE) val);
      return(YES);
    }
    if (type == imageiType && type1 == imageiType) {
      SetVariableLevel(level,name,(VALUE) val);
      return(YES);
    }
    SetErrorf("ImportFromValue() : Cannot convert '%s' to '%s'",type1,type);
    return(NO);
  }
  
  /* Any other types */
  if (type1 != type) {
    SetErrorf("ImportFromValue() : Cannot convert '%s' to '%s'",type1,type);
  	return(NO);
  }
  
  SetVariableLevel(level,name,(VALUE) val);
  return(YES);

}   
 

/*
 * Import a value which must be of type 'type' and which is defined by the symbolic string 'val' which 
 * has to be evaluated at the level 'level1'. The value has to be imported in 'level2' using the name 'name'.
 * 
 * It returns YES if it succeded or NO if not 
 */
  
int ImportFromStr(char *type, LEVEL level1,char *val,LEVEL level2,char *name) 
{
  HASHTABLE t,tt,t1;
  float f;
  PROC p;
  int i;
  int resInt;
  float resFlt;
  void *resPtr;
  char *resStr;
  char (*parsePtr)(LEVEL, char *,void *,void **);
  char (*parseInt)(LEVEL, char *,int,int *);
  char (*parseStr)(LEVEL, char *,char *,char **);
  char (*parseFlt)(LEVEL, char *,float,float *);
  VARIABLE gvar,lvar;
  VALUE *plcont,*pgcont,value;
  int flag;
  char *type1,**list;
  LISTV lv;
  STRVALUE sc,sc1;
  SCRIPT script;
  RANGE rg;
  char *left,flag1;
   
  if (type == NULL) Errorf("ImportFromStr() : Weird");

  if (level1 == level2) Errorf("ImportFromStr() : Cannot import from/to the same level");
  
  /* Case we just need to assign a word */
  if (type == wordType) {
    SetStrVariableLevel(level2,name,val);
    return(YES);
  }

  /* Case we just need to assign a wordlist */
  if (type == wordlistType) {
    if (!ParseWordList_(val,NULL,&list)) return(NO);
    SetStrVariableListLevel(level2,name,list,NO);
    return(YES);
  }

  /* Case we just need to assign a val  */
  if (type == valType) {
    if (ParseValLevel__(level1,val,NULL,&value,AnyType,AnyType,YES)) {
      value = ValueOf(value);
      type1 = GetTypeValue(value);
      if (type1 == numType) SetNumVariableLevel(level2,name,((NUMVALUE) value)->f);
      else if (type1 == strType)
        SetStrVariableLevel(level2,name,((STRVALUE) value)->str);
  	  else SetVariableLevel(level2,name,value);
  	  return(YES);
    }
    return(NO);
  }

  /* Case we just need to assign a valobj  */
  if (type == valobjType) {
    if (ParseValLevel__(level1,val,NULL,&value,AnyType-FloatType,AnyType,YES)) {
      value = ValueOf(value);
      type1 = GetTypeValue(value);
      if (type1 == numType) SetNumVariableLevel(level2,name,((NUMVALUE) val)->f);
      else if (type1 == strType)
        SetStrVariableLevel(level2,name,((STRVALUE) value)->str);
  	  else SetVariableLevel(level2,name,value);
  	  return(YES);
    }
    return(NO);
  }

  /* Case we just need to assign a string  */
  if (type == strType) {
    if (ParseStrValueLevel_(level1,val,NULL,&sc)) {
      SetStrVariableLevel(level2,name,sc->str);
      return(YES);
    }
    return(NO);
  }

  /* Case we just need to assign a list  */
  if (type == listType) {
    if (ParseStrValueLevel_(level1,val,NULL,&sc)) {
      if (GetListFromStrValue(sc)) {
        if (sc->nRef == 1) sc1 = sc;
        else {
          sc1  = TNewStrValue();
          CopyStrValue(sc,sc1);
        }
        SetVariableLevel(level2,name,(VALUE) sc1);
        return(YES);
      }
    }
    return(NO);
  }

  /* Case we just need to assign a listv  */
  if (type == listvType) {
    if (ParseListvLevel_(level1,val,NULL,&lv)) {
      SetVariableLevel(level2,name,(VALUE) lv);
      return(YES);
    }
    return(NO);
  }

  /* Case we just need to assign a range  */
  if (type == rangeType) {
    if (ParseRangeLevel_(level1,val,NULL,&rg)) {
      SetVariableLevel(level2,name,(VALUE) rg);
      return(YES);
    }
    return(NO);
  }
  
  /* A float value */      
  else if (type == floatType || type == numType) {
    if (ParseFloatLevel_(level1,val,0,&f)) {
      SetNumVariableLevel(level2,name,f);
      return(YES);
    }
    return(NO);
  }

  /* Case we just need to assign a proc  */
  if (type == procType) {
    if (ParseProcLevel_(level1,val,NULL,&p)) {
      SetVariableLevel(level2,name,(VALUE) p);
      return(YES);
    }
    return(NO);
  }
         
  /* A int value */      
  if (type == intType) {
    if (ParseIntLevel_(level1,val,0,&i)) {
      SetNumVariableLevel(level2,name,i);
      return(YES);
    }
    return(NO);
  }

  /* Case we just need to assign a script  */
  if (type == scriptType) {
    if (ParseScriptLevel_(level1,&val,NULL,&script)) {
      SetVariableLevel(level2,name,(VALUE) script);
      return(YES);
    }
    return(NO);
  }
        
  /* A variable or an array */      
  if (type == varType || type == arrayType) {
  
    /* Get the variable (eventually creates it) */
    while (level2->levelVar != level2) level2 = level2->levelVar;
    t = level2->theVariables;
    lvar = GetVariableHT(&t, YES, name, &left, &flag1);
    if (lvar==NULL || flag1 != 0 || *left != '\0') Errorf1("");

    /* Its variable content */
    plcont  = GetVariablePContent(lvar,NULL);
    if (!DoesTypeOverwrite(GetTypeValue(*plcont),type)) Errorf("ImportFromStr() : Cannot overwrite variable of type '%s' with variable of type '%s'",GetTypeValue(*plcont),type);
    DeleteValue(*plcont);
    *plcont = NULL;
    
    /* Get the value to import */
    while (level1->levelVar != level1) level1 = level1->levelVar;
    t = level1->theVariables;
    if (type == arrayType) {
      t1 = t;
      gvar = GetVariableHT(&t1, NO, val, &left, &flag1);
      if (gvar == NULL) {
        tt = NewHashTable(8);
        TempValue( tt);
        SetVariableLevel(level1,val,(VALUE) tt);
      }
    }
    
    gvar = GetVariableHT(&t, YES, val, &left, &flag1);
    if (gvar==NULL || flag1 != 0 || *left != '\0') Errorf1("");

    /* Check type if array */
    if (type == arrayType) {
      pgcont = GetVariablePContent(gvar,NULL);
      if (*pgcont != NULL && (type1 = GetTypeValue(*pgcont)) != arrayType) {
        if (name != NULL){
           SetErrorf("ImportFromStr() : Imported variable '%s' is not of type &array but of type %s",val,type1); 
        }
        return(NO);
      }
    }
        
    /* And set it */
    *plcont = (VALUE) gvar;
    AddRefValue( gvar);

    return(YES);
  } 

  /*
   * The newly defined types 
   */
  for (i=0;i<nVarTypes;i++) 
    if (type == varTypeNames[i]) break;    
  if (i == nVarTypes) Errorf("ImportFromStr() : Unknown type '%s'",type);
 
  /* parse value to import */
  switch(varTypes[i]) {
  case tVAL : 
    parsePtr =   (char (*)(LEVEL, char *,void *,void **)) varTypeParseFunctions[i];
    flag = (*parsePtr)(level1,val,NULL,&resPtr);
    if (flag == 0) return(NO);
    if ((void *) parsePtr == (void *) ParseValLevel_) {
      if (GetTypeValue((VALUE) resPtr) != varTypeNames[i]) {
        SetErrorf("ImportFromStr() : Bad type '%s' ('%s' expected)", GetTypeValue((VALUE) resPtr),varTypeNames[i]);
        return(NO);
      }
    }
    /* And set it */
    value = (VALUE) resPtr;
    AddRefValue( resPtr);
    break;
  case tINT : 
    parseInt =   (char (*)(LEVEL, char *,int,int *)) varTypeParseFunctions[i];
    flag = (*parseInt)(level1,val,0,&resInt);
    if (flag == 0) return(NO);

    /* And set it */
    value = (VALUE) NewNumValue();
    SetNumValue ((NUMVALUE) value,resInt);
    break;
  case tFLOAT : 
    parseFlt =   (char (*)(LEVEL, char *,float,float *)) varTypeParseFunctions[i];
    flag = (*parseFlt)(level1,val,0.,&resFlt);
    if (flag == 0) return(NO);

    /* And set it */
    value = (VALUE) NewNumValue();
    SetNumValue ((NUMVALUE) value,resFlt);
    break;
  case tSTR : 
    parseStr =   (char (*)(LEVEL, char *,char *,char **)) varTypeParseFunctions[i];
    flag = (*parseStr)(level1,val,NULL,&resStr);
    if (flag == 0) return(NO);

    /* And set it */
    value = (VALUE) NewStrValue();
    SetStrValue ((STRVALUE) value,resStr);
    break;

  default: Errorf("ImportFromStr() : Weird");
  }

  /* Get the variable (eventually creates it) */
  while (level2->levelVar != level2) level2 = level2->levelVar;
  t = level2->theVariables;
  lvar = GetVariableHT(&t, YES, name, &left, &flag1);
  if (lvar==NULL || flag1 != 0 || *left != '\0') Errorf1("");

  /* Its variable content */
  plcont = GetVariablePContent(lvar,NULL);
  type = GetTypeValue(*plcont);
  if (!DoesTypeOverwrite(type,varTypeNames[i])) Errorf("ImportFromStr() : Cannot overwrite variable of type '%s' with variable of type '%s'",type,varTypeNames[i]);
  DeleteValue(*plcont);
  *plcont = value;
    

  return(YES);
}


 
  
/*
 * The command for importing values from a different level
 */

extern void  SetVarsFromList(LEVEL level,char **varTypeList,char **varList,char **varDefList,char **valList);
extern int GetVarPattern(char **list,char ***type, char ***var, char ***def,char *procName);
extern void  SetVarsFromListv(char **varTypeList,char **varList,char **varDefList,LISTV lv);

void C_Import(char **argv)
{
  LEVEL level;
  char **typeList,**varList,**defList;
  char *str,*type,*val,*name,**list,*val1;
  char flagTest,answer;
  char *action,**pattern;
  LISTV lv;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* list command : import list level {a b {&int c 1}} list */
  if (!strcmp(action,"list")) {
    argv = ParseArgv(argv,tLEVEL_,NULL,&level,tWORDLIST,&pattern,tLIST,&list,0);
    if (level == NULL) level = GetLevel(-1);
    if (level == NULL) level = GetLevel(0);
    GetVarPattern(pattern,&typeList,&varList,&defList,NULL);
    SetVarsFromList(level,typeList,varList,defList,list);
    return;
  }

  /* listv command : import listv level {a b {&int c 1}} listv */
  if (!strcmp(action,"listv")) {
    argv = ParseArgv(argv,tWORDLIST,&pattern,tLISTV,&lv,0);
    GetVarPattern(pattern,&typeList,&varList,&defList,NULL);
    SetVarsFromListv(typeList,varList,defList,lv);
    return;
  }
  
  if (strcmp(action,"args")) Errorf("Bad action name '%s'",action);
  
  /* Get the level number */
  argv = ParseArgv(argv,tLEVEL_,NULL,&level,-1);
  if (level == NULL) level = GetLevel(-1);
  if (level == NULL) level = GetLevel(0);
  
  if (*argv == NULL) ErrorUsage1();

  /* Is there a '?' */
  if (!strcmp("?",*argv)) {
    flagTest = YES;
    argv++;
  }
  else flagTest = NO;
  
  /* Get the type if there is one */
  if (**argv == '&') {
    argv = ParseArgv(argv,tWORD,&str,-1);
    type = GetArgType(str);
  }
  else type = varType;
       
  /* Loop on the values to import */
  answer = YES;
  while (1) {

    argv = ParseArgv(argv,tWORDLIST_,NULL,&list,-1);

    if (*list == NULL) ErrorUsage1();

    if (*(list+1) == NULL) {
      val = *list;
      val1 = val;
      name = val1;
      while(1) {
        while (*val1 != '.' && *val1 != '\0') val1++;
        if (*val1 == '\0') break;
        val1++;
        name = val1;
      }
    } 
    else if (*(list+2) == NULL) {
      val = *list;
      name = *(list+1);
    }
    else ErrorUsage1();
      
    answer = answer && ImportFromStr(type,level,val,levelCur,name); 

    if (answer == NO && !flagTest) Errorf1("");
      
    if (*argv == NULL) break;
  }

  NoMoreArgs(argv);

  if (flagTest) SetResultInt((int) answer);    
}

/*
 * The command for importing values from the global level
 */

void C_Global(char **argv)
{
  LEVEL level;
  char *str,*type,*val,*name,**list,*val1;
  char answer;

  if (*argv == NULL) ErrorUsage1();

  level = levelFirst;
  
  /* Get the type if there is one */
  if (**argv == '&') {
    argv = ParseArgv(argv,tWORD,&str,-1);
    type = GetArgType(str);
  }
  else type = varType;
       
  /* Loop on the values to import */
  answer = YES;
  while (1) {

    argv = ParseArgv(argv,tWORDLIST_,NULL,&list,-1);

    if (*list == NULL) ErrorUsage1();

    if (*(list+1) == NULL) {
      val = *list;
      val1 = val;
      name = val1;
      while(1) {
        while (*val1 != '.' && *val1 != '\0') val1++;
        if (*val1 == '\0') break;
        val1++;
        name = val1;
      }
    } 
    else if (*(list+2) == NULL) {
      val = *list;
      name = *(list+1);
    }
    else ErrorUsage1();
      
    answer = answer && ImportFromStr(type,level,val,levelCur,name); 

    if (answer == NO) Errorf1("");
      
    if (*argv == NULL) break;
  }

  NoMoreArgs(argv);
}




/*
 * The main command for managing variables
 */
void C_Var(char **argv)
{
  char *action,*str,*str1,*type;
  VARIABLE var;
  VALUE value;
  int i,flagFirst,n;
  LEVEL level;
  float f;
  
  argv = ParseArgv(argv,tWORD,&action,tLEVEL_,levelCur,&level,-1);
  
  /* The 'exist' action */  
  if (!strcmp(action,"exist")) {
    argv = ParseArgv(argv,tWORD,&str,tSTR_,NULL,&type,0);

    if (type != NULL) {
      if (GetArgType(type) == NULL) Errorf("Bad type '%s'",type);
      type = GetArgType(type);
    }

    /* Case of an event variable */
    if (str[0] == '@') {
      str1 = NULL;
      if (GetEventVariable2(str+1,&str1,&f)) {
        if (type == NULL) SetResultInt(1);
        else if (str1) SetResultInt(type==strType);
        else {
          if (type == floatType || type == numType) SetResultInt(1);
          else if (type == intType) SetResultInt(f == (int) f);
          else SetResultInt(0);
        }
      }
      else SetResultInt(0);
      return;
    }

    /* Regular variable */
    value = ExistVariableLevel(level,str,type);
    SetResultInt((int) (value != NULL));
    return;
  } 
  
  /* The 'env' action */
  else if (!strcmp(action,"env")) {
    argv = ParseArgv(argv,tWORD_,"[^_]*",&str,tWORD_,NULL,&type,0);
    flagFirst = YES;
    if (type != NULL) {
      if (GetArgType(type) == NULL) Errorf("Bad type '%s'",type);
      type = GetArgType(type);
    }
    for (i=0;i<level->theVariables->nRows;i++) {
      for (var = (VARIABLE) level->theVariables->rows[i]; var != NULL; var = var->next) {
        if (MatchStr(var->name,str)) {
          value = ValueOf((VALUE) var);
          type = GetTypeValue(value);
          if (type != NULL && value != NULL && GetTypeValue(value)!=type) value = NULL;
          if (value != NULL) {
            if (flagFirst == NO) 
              AppendResultf(" {%s %s ",var->name,GetTypeValue(var->content));
            else {
              AppendResultf("{%s %s ",var->name,GetTypeValue(var->content));            
              flagFirst = NO;
            }
            if (type == numType) AppendResultf("%g",((NUMVALUE) ValueOf((VALUE) var))->f);
            else if (type == strType) AppendResultf("'%s'",((STRVALUE) ValueOf((VALUE) var))->str);
            else if (type == nullType) AppendResultf("null");
            AppendResultf("} ");
          }
        }
      }
    }
  }

  /* The 'list' action */
  else if (!strcmp(action,"list")) {
    argv = ParseArgv(argv,tWORD_,"[^_]*",&str,tWORD_,NULL,&type,0);
    for (i=0;i<level->theVariables->nRows;i++) {
      for (var = (VARIABLE) level->theVariables->rows[i]; var != NULL; var = var->next) {
        if (MatchStr(var->name,str)) {
          value = *(GetVariablePContent(var,NULL));
          if (type != NULL && value != NULL && strcmp(GetTypeValue(value),type)) value = NULL;
          if (value != NULL) AppendListResultStr(var->name);
        }
      }
    }
  }

  /* The 'hash' action (used just for checking...) */
  else if (!strcmp(action,"hash")) {
    argv = ParseArgv(argv,0);
    for (i=0;i<level->theVariables->nRows;i++) {
      n = 0;
      for (var = (VARIABLE) level->theVariables->rows[i]; var != NULL; var = var->next) n++;
      if (i != 0) AppendResultf(" %d",n);
      else AppendResultf("%d",n);
    }
  }

  /* The 'delete' action */
  else if (!strcmp(action,"delete")) {
    while (1) {
      argv = ParseArgv(argv,tVNAME_,NULL,&str,-1);
      if (str == NULL) {
        NoMoreArgs(argv);
        return;
      }
      DeleteVariableLevel(level,str);
    }
  }

  /* The 'unix' action */
  else if (!strcmp(action,"unix")) {
    argv = ParseArgv(argv,tWORD,&str,0);
    #ifdef LASTWAVE_UNIX_SYSTEM
    SetResultStr(getenv(str));
    #endif
    #ifdef LASTWAVE_WIN32_SYSTEM
    str1 = getenv(str);
    if(str1!=NULL) {
      SetResultStr(str1);
    } else {
      SetResultStr("");
    }
    #endif
  }
    
  /* The 'type' action */
  else if (!strcmp(action,"type")) {
    argv = ParseArgv(argv,tWORD,&str,tWORD_,NULL,&type,0);
    value = ExistVariableLevel(level,str,type);
    if (value != NULL) {
      SetResultStr(GetTypeValue(value));
      return;
    }
    InitError();
  }
   
  else Errorf("Unknown action '%s'",action);  
}


void C_Type(char **argv)
{
  char *action,*name;
  void *arg[1];
  char *type,*match,*packageName;
  TypeStruct *ts;
  Field *fields;
  LISTV lv;
  char flagSet, flagGet, flagExtract, flagTS, opt;
  int i;
  
  argv = ParseArgv(argv,tWORD,&action,-1);

  /* The 'list' action */
  if (!strcmp(action,"list")) {
    ParseArgv(argv,tSTR_,NULL,&packageName,0);
    lv = TNewListv();
    SetResultValue(lv);
    if (packageName == NULL || !strcmp(packageName,"kernel")) {
      AppendStr2Listv(lv,nullType);
      AppendStr2Listv(lv,strType);
      AppendStr2Listv(lv,valType);
      AppendStr2Listv(lv,valobjType);
      AppendStr2Listv(lv,wordType);
      AppendStr2Listv(lv,wordlistType);
      AppendStr2Listv(lv,listType);
      AppendStr2Listv(lv,listvType);
      AppendStr2Listv(lv,rangeType);
      AppendStr2Listv(lv,numType);
      AppendStr2Listv(lv,intType);
      AppendStr2Listv(lv,floatType);
      AppendStr2Listv(lv,arrayType);
      AppendStr2Listv(lv,scriptType);
      AppendStr2Listv(lv,procType);
    }

    for (i=0; i<nVarTypes;i++) {
      if (packageName == NULL || !strcmp(packageName,typePackage[i]))  AppendStr2Listv(lv,varTypeNames[i]);
    }
    return;
  }

  argv = ParseArgv(argv,tSTR,&type,-1);

  /* The 'exist' action */  
  if (!strcmp(action,"exist")) {

    flagTS = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
      case 's': 
        flagTS = YES; 
        break;
      default:
        ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
    if (GetArgType(type) == NULL) {
      SetResultInt(0);
      return;
    }
    if (flagTS == YES && GetTypeStruct(type) == NULL)  {
      SetResultInt(0);
      return;
    }
    SetResultInt(1);
    return;
  }
  
  /* The 'field' action */  
  if (!strcmp(action,"field")) {
    argv = ParseArgv(argv,tSTR_,"*",&match,-1);

    flagSet = NO;
    flagGet = NO;
    flagExtract = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
      case 's': 
        flagSet = YES; 
        break;
      case 'g': 
        flagGet = YES; 
        break;
      case 'e': 
        flagExtract = YES; 
        break;
      default:
        ErrorOption(opt);
      }
    }    

    NoMoreArgs(argv);
    if (*type != '&') Errorf("Bad type '%s'",type);
    ts = GetTypeStruct(type);  
    if (ts == NULL) {
      if (GetArgType(type) == NULL) Errorf("Bad Type '%s'",type);
      Errorf("No Type Structure associated to type '%s'",type);
    }
    lv = TNewListv();
    fields = ts->fields;
    while(fields[0].name != NULL) {
      if (MatchStr(fields[0].name,match) && 
          (!flagSet || fields[0].Set != NULL) &&
          (!flagGet || fields[0].Get != NULL) &&
          (!flagExtract || fields[0].ExtractInfo != NULL)) AppendStr2Listv(lv,fields[0].name); 
        
      fields++;
    }
    SetResultValue(lv);
    return;
  }

  /* The 'help' action */  
  if (!strcmp(action,"help")) {

    if (*type != '&') Errorf("Bad type '%s'",type);
    if (!strcmp(type,wordlistType)) {
      SetResultStr("{{{&wordlist} {Extended type that indicates that the argument of a procedure is not evaluated and should correspond to a list.}}}");
      return;
    }
    else if (!strcmp(type,wordType)) {
      SetResultStr("{{{&word} {Extended type that indicates that the argument of a procedure is not evaluated.}}}");
      return;
    }
    else if (!strcmp(type,valobjType)) {
      SetResultStr("{{{&valobj} {Extended type that indicates that the argument of a procedure is evaluated but must not correspond to a litteral number.}}}");
      return;
    }
    else if (!strcmp(type,valType)) {
      SetResultStr("{{{&valType} {Extended type that is the default type of the argument of a procedure : indicates that the argument is evaluated.}}}");
      return;
    }
    ts = GetTypeStruct(type);  
    if (ts == NULL) {
      if (GetArgType(type) == NULL) Errorf("Bad Type '%s'",type);
      Errorf("No Type Structure associated to type '%s'",type);
    }
    
    if (*argv == NULL) {
      SetResultStr(ts->doc);
      return;
    } 
    
    if (*argv != NULL && !strcmp(*argv,"-n")) {
      if (ts->NumExtract == NULL) SetResultValue(nullValue);
      else SetResultStr(NumExtractTS(ts,NULL));
      return;
    }

    argv = ParseArgv(argv,tSTR,&name,0);
    
    fields = ts->fields;
    while(1) {
      if (fields[0].name == NULL) Errorf("Bad field name '%s'",name);
      if (!strcmp(fields[0].name,name)) { 
        arg[0] = name;
        lv = TNewListv();
        if (fields[0].Get) AppendStr2Listv(lv,(char *) GetFieldValue(fields[0],NULL,arg));
        else AppendValue2Listv(lv,(VALUE) nullValue);
        if (fields[0].Set) AppendStr2Listv(lv,(char *) SetFieldValue(fields[0],NULL,arg));
        else AppendValue2Listv(lv,(VALUE) nullValue);
        if (fields[0].ExtractOptions) AppendStr2Listv(lv,(char *) ExtractOptionFieldValue(fields[0],NULL,arg));
        else AppendValue2Listv(lv,(VALUE) nullValue);
        SetResultValue(lv);
        break;
      } 
      fields++;
    }
    return;
  }

  /* The 'help' action */  
  if (!strcmp(action,"helpnum")) {

    if (*type != '&') Errorf("Bad type '%s'",type);
    ts = GetTypeStruct(type);  
    if (ts == NULL) {
      if (GetArgType(type) == NULL) Errorf("Bad Type '%s'",type);
      Errorf("No Type Structure associated to type '%s'",type);
    }
    NoMoreArgs(argv);
    
    if (ts->NumExtract == NULL) SetResultValue(nullValue);
    else SetResultStr(NumExtractTS(ts,NULL));
    
    return;
  }
  
  Errorf("Unknown action '%s'",action);
}


void C_New(char **argv)
{
  char *type;
  VALUE val;
  TypeStruct *ts;
  
  argv = ParseArgv(argv,tWORD,&type,0);

  if (*type != '&') Errorf("Bad type '%s'",type);
  ts = GetTypeStruct(type);  
  if (ts == NULL) {
    if (GetArgType(type) == NULL) Errorf("Bad Type '%s'",type);
    Errorf("No Type Structure associated to type '%s'",type);
  }

  val = NewValue(ts);
  TempValue(val);
  SetResultValue(val);
}


void C_Delete(char **argv)
{
  char *str;
  LEVEL level;
  PROC p;
  
  argv = ParseArgv(argv,tLEVEL_,levelCur,&level,-1);

  argv = ParseArgv(argv,tVNAME,&str,0);
  DeleteVariableLevel(level,str);
}

/*
 * Copy a value 
 */
void C_Copy(char **argv)
{
  VALUE val,val1;
  
  argv = ParseArgv(argv,tVAL,&val,-1);
  if (*argv != NULL) argv = ParseArgv(argv,tVAL,&val1,0);
  else val1 = NULL;
  
  if (val->ts == NULL || val->ts->Copy == NULL) Errorf("Cannot send copy message to '%s' values",GetTypeValue(val));
  
  if (val == val1) {
    SetResultValue(val1);
    return;
  }
  
  val = ((VALUE (*)(VALUE,VALUE)) (val->ts->Copy))(val,val1);

  if (val1 == NULL) {
    TempValue(val);  
    SetResultValue(val);
  }
  else SetResultValue(val1);
} 

/*
 * Clear a value 
 */
void C_Clear(char **argv)
{
  VALUE val;
  
  argv = ParseArgv(argv,tVAL,&val,0);
  
  if (val->ts == NULL || val->ts->Clear == NULL) Errorf("Cannot send clear message to '%s' values",GetTypeValue(val));
  
  ((void (*)(VALUE)) (val->ts->Clear))(val);
} 

/*
 * Initializations of some global variables
 */

VALUE nullValue;
 
void InitVariables(void)
{
  char *home,*system,*computer,*termType,*scriptDir,*user;
  int i[4];
  LISTV lv;
  extern void InitSourceDirs(char *dir);
  extern VALUE NewNullValue(void);

  
  /* create the nullValue variable */
  nullValue = NewNullValue();
  
  /* Set some default constants */
  XXGetSystemInfo(&home,&system,&computer,&termType,&scriptDir,&user);
  
  SetStrVariable("Home",home);
  SetStrVariable("User",user);
  SetStrVariable("System",system);
  SetStrVariable("Term",termType);
  SetStrVariable("Computer",computer);
  InitSourceDirs(scriptDir);
  
  if (IsCPULittleEndian) SetStrVariable("BinaryMode","little");
  else SetStrVariable("BinaryMode","big");

  SetNumVariable("Display.ScreenDepth",WDepth());

  SetNumVariable("Display.BWScreen",(WIsBWScreen() ? 1 : 0));
  SetStrVariable("Display.ScreenType",WScreenType());
  WScreenRect(i,i+1,i+2,i+3);
  lv = TNewListv();
  AppendIntArray2Listv(lv,i,4);
  SetVariable("Display.ScreenRect",(VALUE) lv);

  if (GraphicMode) SetNumVariable("Display.flag",1);
  else SetNumVariable("Display.flag",0);

}







