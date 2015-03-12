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


/* Default name for an array */
static char defaultName[] = "";

char *arrayType = "&array";


/*
 * Answers to the different print messages
 */

void PrintHash(HASHTABLE table)
{
  int r;
  AHASHELEM e,enext;
  
  Printf("%s (size=%d) {\n",arrayType,table->nElems);
  if (table->nElems == 0) {
    Printf("}\n");
    return;
  }
  for (r = 0; r<table->nRows;r++) {
    for (e = table->rows[r]; e != NULL; e = enext) {    
      enext = e->next;
      Printf("%.20s : %s\n",e->name,ToStrValue(e,NO));
    }
  }
  Printf("}\n");
}

char *ToStrHash(HASHTABLE val, char flagShort)
{
  static char str[30];
  
  sprintf(str,"%s(size=%d)",arrayType,val->nElems);
  return(str);
}

void PrintInfoHash(HASHTABLE val)
{
  Printf("   nElems =  %d\n",val->nElems);
  Printf("   nRows  =  %d\n",val->nRows);
}

/*
 * Get a field
 */
void *GetFieldArray(VALUE value, void **arg)
{
  HASHTABLE table = (HASHTABLE) value;
  VALUE *pCont1,*pValue = ARG_G_GetResPValue(arg);
  char *field = ARG_G_GetField(arg);
      
  *pValue = (VALUE) GetElemHashTable(table,field);
      
  if (*pValue == NULL) return(NULL);
      
  if (IsVariable(*pValue)) {
    /* ??? est ce vraiment necessaire */
    pCont1 = GetVariablePContent((VARIABLE) *pValue, NULL);
    if (*pCont1 == NULL) return(NULL);
    *pValue = *pCont1;
  }
      
  AddRefValue(*pValue);
  TempValue(*pValue);
  
  return(GetTypeValue(*pValue));
}
      
/*
 * The field list
 */
struct field fieldsHash[] = {
  NULL, NULL, NULL, NULL, NULL
};


HASHTABLE NewHash(void)
{
  return(NewHashTable(30));
}


/*
 * The type structure for HASH
 */

TypeStruct tsHash = {

  "{{{&array} {This type is used to store an array. The syntax is array.field}}}",    /* Documentation */

  &arrayType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteHashTable,     /* The Delete function */
  NewHash,     /* The New function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrHash,       /* String conversion */
  PrintHash,       /* The Print function : print the object when 'print' is called */
  PrintInfoHash,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsHash,      /* The list of fields */
};


/*
 * The hashing function
 */

static int Hash(HASHTABLE table, char *name) 
{
  unsigned long r = 0;
  
  while (*name != '\0') r += *(name++);
  
  r = r % table->nRows;
  
  return(r);
}

/*
 * Add a reference to a hash table
 */
 
void AddRefHashTable(HASHTABLE t)
{
  AddRefValue( t);
}

/*
 * Remove a reference to a hash table
 */
 
void RemoveRefHashTable(HASHTABLE t)
{
  RemoveRefValue( t);
}

/*
 * Creating a new Hash table with 'nrows' rows
 */
 
HASHTABLE NewHashTable(int nRows)
{
  HASHTABLE table ;

#ifdef DEBUGALLOC
DebugType = "Hashtable";
#endif
table = (HASHTABLE) Malloc(sizeof(struct hashTable));

  InitValue(table,&tsHash);

  table->nRows = nRows;
  table->rows = (AHASHELEM *) Calloc(nRows,sizeof(AHASHELEM));
  table->nElems = 0;
  table->level = NULL;
  table->name = defaultName;
    
  return(table);
}


/*
 * Clearing a hash table
 */
 
void ClearHashTable(HASHTABLE table)
{
  int r;
  AHASHELEM e,enext;
  VARIABLE v;
  
  if (table->nElems == 0) return;
  
  for (r = 0; r<table->nRows;r++) {
    for (e = table->rows[r]; e != NULL; e = enext) {    
      enext = e->next;
      if (IsVariable(e)) {
        v = (VARIABLE) e;
        if (v->hashTable == table) v->hashTable = NULL;
      }
      DeleteValue(e);
    }
    table->rows[r] = NULL;
  }
}      

  
/*
 * Deleting a hash table
 */
 
void DeleteHashTable(HASHTABLE table)
{
  RemoveRefHashTable(table);
  if (table->nRef > 0) return;
  
  ClearHashTable(table);

  if (table->name != defaultName) Free(table->name);
  
  Free(table->rows);
#ifdef DEBUGALLOC
DebugType = "Hashtable";
#endif
  
  Free(table);
}


/*
 * Add an element to a hash table
 */
 
int AddElemHashTable(HASHTABLE table,AHASHELEM e)
{
  int r;

  r = Hash(table,e->name);

  e->next = table->rows[r];
  table->rows[r] = e;

  e->nRef++;

  table->nElems++;

  return(YES);
}      


/*
 * Get an element from a hash table (from name)
 */
 
AHASHELEM GetElemHashTable(HASHTABLE table,char *name)
{
  int r;
  AHASHELEM e;
  
  r = Hash(table,name);

  for (e = table->rows[r];e != NULL; e = e->next) 
    if (!strcmp(name,e->name)) break;

  return(e);    
}      

/*
 * Get and Remove an element from a hash table (from name)
 */
 
AHASHELEM GetRemoveElemHashTable(HASHTABLE table,char *name)
{
  AHASHELEM e,eprev;
  int r;
  VARIABLE v;
  
  r = Hash(table,name);
  
  for (e = table->rows[r], eprev = NULL;e != NULL; eprev = e, e = e->next) 
    if (!strcmp(name,e->name)) break;

  if (e != NULL) {
    if (IsVariable(e)) {
      v = (VARIABLE) e;
      if (v->hashTable == table) v->hashTable = NULL;
    }
    if (eprev != NULL) eprev->next = e->next;
    else table->rows[r] = e->next;
    table->nElems--;
  }
    
  return(e);    
}      

/*
 * The command to manage array variables
 */
 
void C_Array(char **argv)
{
  char *action,*name;
  int size,r;
  HASHTABLE t;
  VARIABLE e;
  VALUE value;
  char *filter;
  LISTV lv;
    
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* The 'new' action */  
  if (!strcmp(action,"new")) {
    argv = ParseArgv(argv,tINT_,8,&size,0);
    t = NewHashTable(size);
    TempValue( t);
    SetResultValue(t);
  }

  /* The 'list' action */  
  else if (!strcmp(action,"list")) {
    argv = ParseArgv(argv,tVAL,&value,tSTR_,"*",&filter,0);
    if (GetTypeValue(value) != arrayType) Errorf("Expect an &array as first argument");
    t = (HASHTABLE) value;
    lv = TNewListv();
    if (t == NULL) Errorf1("");
    for (r = 0; r<t->nRows;r++) {
      for (e = (VARIABLE) (t->rows[r]); e != NULL; e = (VARIABLE) (e->next)) {
        if (!MatchStr(e->name,filter)) continue;
        value = *(GetVariablePContent(e,NULL));
        if (value != NULL) AppendStr2Listv(lv,e->name);
      }
    }
    SetResultValue(lv);
  }

 
  else Errorf("Unknown action '%s'",action);  
}
 
