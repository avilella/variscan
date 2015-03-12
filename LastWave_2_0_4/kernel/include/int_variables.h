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




/****************************************************************************/
/*                                                                          */
/*  variables.h       This file contains the variable definition            */
/*                                                                          */
/****************************************************************************/


#ifndef VARIABLES_H

#define VARIABLES_H 



/*
 * The structure of a variable
 *   Since it will be an element of a hash table it should start
 *   with the fields of AHASHELEM
 */
 
typedef struct variable {

  /* The Fields of AHASHELEM */
  AHashElemFields; 
 
  VALUE content; /* The content of a variable */

  HASHTABLE hashTable;  /* The corresponding hash table */
  
  char *traceVarName;  
  
} *VARIABLE;




#ifndef TOPLEVEL_H
typedef struct level *LEVEL;
#endif



extern void DeleteVariableLevel(LEVEL level, char *name);
extern void DeleteVariable(char *name);
extern void DeleteVariableIfExistLevel(LEVEL level, char *name);
extern void DeleteVariableIfExist(char *name);

extern VARIABLE GetVariableHT(HASHTABLE *t, char flagCreate, char *name, char **left, char *flag);

extern VARIABLE GetSimpleVariableHashTable(HASHTABLE t,char *name);
extern VALUE *GetVariablePContent(VARIABLE variable, char *pflagTrace);
extern VARIABLE GetVariableLevel(LEVEL level,char *name);
extern VALUE *GetVariablePContent(VARIABLE variable, char *flagTrace);
extern VALUE GetVariableContentLevel(LEVEL level, char *name, char *type);
extern VALUE GetVariableContent(char *name, char *type);
extern VARIABLE GetVariable(char *name);
extern VARIABLE CGetVariableLevel(LEVEL level,char *name);
extern VARIABLE CGetVariable(char *name);

extern char ParseVariableLevel_(LEVEL level, char *name,VALUE def, VALUE *val,char *type, char flagTemp); 
extern void ParseVariableLevel(LEVEL level, char *name, VALUE *val,char *type,char flagTemp); 
extern void ParseVariable(char *name, VALUE *val,char *type, char flagTemp); 
extern char ParseVariable_(char *name, VALUE def, VALUE *val,char *type, char flagTemp); 

extern void SetVariableLevel(LEVEL level,char *name,VALUE content);
extern void SetVariable(char *name,VALUE content);
extern void Setv1(LEVEL level, char *name, char *equal, char *type, VALUE val, float f);
extern void SetvLevel(LEVEL level, char *name, VALUE val);
extern void Setv(char *name, VALUE val);



extern VALUE ExistVariableLevel(LEVEL level,char *name,char *type);
extern VALUE ExistVariable(char *name,char *type);

extern int ImportFromValue(char *type, VALUE val, float f, LEVEL level,char *name);
extern int ImportFromStr(char *type,LEVEL level1,char *val,LEVEL level2,char *name); 
extern char *GetArgType(char *);

extern void AddVariableType(char *type, char (*f) (LEVEL, char *, void *, void **));
extern int AddVariableTypeInt(char *type, char (*parse) (LEVEL level, char *, int, int *));
extern int AddVariableTypeFloat(char *type, char (*parse) (LEVEL level, char *, float, float *));
extern int AddVariableTypeStr(char *type, char (*parse) (LEVEL level, char *, char *, char **));
extern int AddVariableTypeValue(char *type, TypeStruct *ts, char (*parse) (LEVEL level, char *, void *, void **));



extern void InitVariables(void);


#endif
