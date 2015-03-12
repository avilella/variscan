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




/****************************************************************************/
/*                                                                          */
/*  int_value.c   Deal with VALUE                                           */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include <stdarg.h>
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"


/*****************************************************************
 *
 *  Function dealing with general variable contents
 *
 *****************************************************************/  
 
Field *FindField(Field *fields, char *name)
{
  char *name1;
  
  if (name == NULL) name1 = "";
  else name1 = name;
  
  while(fields[0].name != NULL && strcmp(name1,fields[0].name)) fields++;
  if (fields[0].name == NULL) return(NULL);
  return(fields);
}

char *GetBTypeContent(VALUE vc)
{
  VALUE c;
  
  c = vc;
  while (c != NULL && IsVariable(c)) c = ((VARIABLE) vc)->content;
  
  return(GetTrueTypeValue(vc));
}


/*
 * Return a VALUE in a get field
 */
 
void *GetValue_(VALUE vc, void **arg)
{
  VALUE *pvc = ARG_G_GetResPValue(arg);

  *pvc = vc;
  
  return(GetTypeValue(vc));
}
 
 /*
  * Add a reference to a VALUE 
  */
 
 void AddRefValue_(VALUE value)
 {
   if (value == NULL) Errorf("AddRefValue()  (Weird error) : NULL content !");
   value->nRef++; 
 }
 
 /*
  * Remove a reference to a VALUE 
  */
 
 void RemoveRefValue_(VALUE value)
 {
   if (value == NULL) Errorf("RemoveRefValue()  (Weird error) : NULL content !");
   value->nRef--;   
 }
  
/*
 * Generic function that decides whether a type can be overwritten by another on 
 */
char DoesTypeOverwrite(char *type, char *typeOverwrite)
{
  if (type == typeOverwrite || type == nullType || typeOverwrite == nullType) return(YES);
  
  if ((type == arrayType || type == strType || type == listType || type == listvType || type == numType || type == strType || 
       type == rangeType || type == scriptType || type == procType || type == floatType || type == intType || type == signalType || 
       type == signaliType || type == imageType || type == imageiType) &&
      (typeOverwrite == arrayType || typeOverwrite == strType || typeOverwrite == listType || typeOverwrite == listvType || typeOverwrite == numType || typeOverwrite == strType || 
       typeOverwrite == rangeType || typeOverwrite == scriptType || typeOverwrite == procType || typeOverwrite == floatType || typeOverwrite == intType || typeOverwrite == signalType || 
       typeOverwrite == signaliType || typeOverwrite == imageType || typeOverwrite == imageiType)) return(YES);
       
  return(NO);
} 

/*****************************************************************
 *
 *  The NULL content
 *
 *****************************************************************/  

/* 
 * There should be just one instance of this variable.
 * It is created at startup and its name is'nullValue'
 */
 
char *nullType = "&null";


/*
 * String conversion
 */

char *ToStrNull(VALUE vc,char flagShort)
{
  static char str[30];
  sprintf(str,"null");
  return(str);
}

/*
 * Print the info 
 */
void PrintInfoNull(VALUE vc)
{
  Printf("null\n");
}

/*
 * Delete null
 */
 
static void DeleteNull(VALUE vc)
{ 
}

static VALUE NewNull(void)
{ 
  return(nullValue);
}


/*
 * The field list
 */
struct field fieldsNull[] = {
  NULL, NULL, NULL, NULL, NULL
};


/*
 * The type structure for NUMVALUE
 */

TypeStruct tsNull = {

  "{{{&null} {This is the type of the element 'null'}}}",              /* Documentation */

  &nullType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteNull,     /* The Delete function */
  NewNull,     /* The New function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrNull,       /* String conversion */
  NULL,  /* The Print function  */
  PrintInfoNull,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsNull,      /* The list of fields */
};



VALUE NewNullValue(void)
{
  VALUE vc = (VALUE) Malloc(sizeof(struct value));
  InitValue(vc,&tsNull);
  
  return(vc);
}
 


