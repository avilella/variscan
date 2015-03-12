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



#ifndef INT_HASH_H

#define INT_HASH_H

/*
 * This is an abstract structure for the elements of an hash table
 *
 *   Any structure you include in a hash table must start with 
 *   the following fields.
 *   This structure should start with the VALUE structure fields
 *   which is another abstract structure for variable's contents.
 *   That allows hash elements to be used as the content of variables.
 */

#define AHashElemFields \
  ValueFields; \
  void *next;  \
  char *name   
   
typedef struct ahashElem {

  AHashElemFields;    
  
} *AHASHELEM;


/*
 * The hash table structure.
 *
 *   Basically it is just a list of AHASHELEMENT along with a 
 *   C-function to delete an element of the table.
 *
 *   Since a hash table of variables can be used as the content
 *   of a variable (for arrays of variables), this structure must
 *   fit the VALUE abstract structure (cf int_variables.h).
 */

typedef struct hashTable {

  /* These fields are the fields of the VALUE abstract structure */
  ValueFields;
  
  char *name;
    
  unsigned long nRows; /* The number of rows in the table */
  unsigned long nElems; /* The number of elements */
  
  AHASHELEM *rows; /* The rows */
  
  struct level *level; /* The associated level if any */
  
} *HASHTABLE;


extern char *arrayType;

extern void *GetFieldArray(VALUE value, void **arg);

extern void AddRefHashTable(HASHTABLE t);  
extern void RemoveRefHashTable(HASHTABLE t);  
extern HASHTABLE NewHashTable(int nRows);
extern void ClearHashTable(HASHTABLE table);
extern void DeleteHashTable(HASHTABLE table);
extern int AddElemHashTable(HASHTABLE table,AHASHELEM e);
extern AHASHELEM GetElemHashTable(HASHTABLE table,char *name);
extern AHASHELEM GetRemoveElemHashTable(HASHTABLE table,char *name);

#endif
