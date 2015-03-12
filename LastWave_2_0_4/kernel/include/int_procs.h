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



/****************************************************************************/
/*                                                                          */
/*  int_procs.h       Defines the PROC structure                            */
/*                                                                          */
/****************************************************************************/

#ifndef PROCS_H

#define PROCS_H

/*
 *
 * The SCRIPTLINE structure
 *
 *     A scriptline is a single command line of a script
 *
 */

/*
 * The compile structure
 */


typedef struct compStruct {
  
union { 
  char **dollar;                 /* position of the successive $ signs if any */
  char **bracket;                /* position of the successive [] signs if any (no dollars) */
} pos;
  
union  {
  struct script **dollar;    /* for any $ followed by a [], the corresponding script is stored here */
  struct script **bracket;   /* the corresponding script of [] is stored here */
} scripts;
     


} CompStruct, *COMPSTRUCT;

 
typedef struct scriptline {

  char *line;                 /* Original line */

  int nWords;                 /* The number of words */
  char **words;               /* The words of the command line */
  
  COMPSTRUCT *cs;            /* Some compilation info for each word */

  unsigned char flags;        /* Some useful flags */

  struct proc *proc;         /* The proc associated to the script line */

  char **redirect;            /* Points to the redirection word if any */
    
} ScriptLine, *SCRIPTLINE;   


/* some flags */
#define SLDollarFlag (1l)
#define SLSetvFlag (2l)

/*
 *
 * The SCRIPT structure
 *
 *
 */

extern char *scriptType;
extern TypeStruct tsScript;

typedef struct script {

  ValueFields;
  
  int nsl;             /* the number of script lines */
  SCRIPTLINE *sl;      /* The script lines */
  
  
} Script, *SCRIPT;   


extern SCRIPTLINE NewScriptLine(void);
extern void DeleteScriptLine(SCRIPTLINE sl);
extern SCRIPT NewScript(void);
extern void DeleteScript(SCRIPT script);
extern SCRIPTLINE MakeScriptLine(char *line, int nWords, char **beg, char **end, int nDollars, char **dollars,char flagSetv, int redirectWord, char flagSubst);


/*
 *
 * The PACKAGE structure
 *
 */

typedef struct package {
  
  /* The function to be called to load the package */
  void (*load)(void);
  
  /* The name of the package */
  char *name;
  
  /* If YES then the package has been loaded already */
  char flagLoaded;
  
  /* The year it has been created */
  int year;

  /* The version number of the package */
  char *version;
  
  /* The authors' list */
  char *authors;

  /* Some info about the package */
  char *info;
  
} Package, *PACKAGE;



/*
 *
 * The CPROC structure
 *
 *     A ccommand is command that corresponds to some C code
 *
 */
 
 
typedef struct cproc {
  char *name;                     /* name of the command */
  void (*function)(char **argv);  /* C function corresponding to the command */
  char *description;              /* one-line help */
} CProc, *CPROC;


/*
 * The cproc table type
 * 
 * This structure is used to group procedures that are defined in C
 */

typedef struct cprocTable {

  CProc *procs;              /* Array of C proc */
  char *packageName;         /* Name of the package the table belongs to */
  char *name;                /* Name of the table */
    
} CProcTable, *CPROCTABLE;


/* Some functions */
extern void AddCProcTable(CProcTable *ccmdTable); /* Add a new C command table */


/*
 *
 * The SPROC structure
 *
 *     A sproc is a proc that corresponds to some source (script langage)  code
 *
 */


typedef struct sproc {

  char **varList;               /* an array of the successive variables */
  char **varDefList;            /* an array of the successive default values of the variables */
  char **varTypeList;            /* an array of the successive types of the variables */
  
  SCRIPT script;                 /* the script */
  char *filename;

} SProc, *SPROC;


/* Some functions */
extern struct proc * RemoveSProc(struct proc *c);


extern char Source(char **filenameList);
extern void AddSourceDir(char *dir);


/*
 * 
 * The PROC structure 
 *
 * It is used to store either a scommand or a ccommand
 */
 
typedef struct proc {

  /* The fields needed for a hash table element */
  AHashElemFields;

  /* This flag is used to remember whether a scommand was deleted or ccommand has been overloaded */
  char flagStillExist; 

  /* The help list */
  char **description;
   
  /* Should this command be traced ? */
  char flagTrace;
  
  /* The package */
  char *package;
  
  /* The union of the scommand and ccommand */
  char flagSP;
  union {
    SPROC sp;
    CPROC cp;
  } p;
  
  /* Proc table if cc */
  CPROCTABLE procTable;
  
} Proc, *PROC;
   
extern char *procType;   
extern TypeStruct tsProc;

extern PROC GetCProc(char *name);
extern PROC GetSProc(char *name);
extern void CleanProc(PROC c);
extern void DeleteProc(PROC c);
extern PROC NewProc(void);
extern PROC GetProc(char *name);
extern char IsValidProcName(char *name);


#endif
