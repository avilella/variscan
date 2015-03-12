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
/*  toplevel.h       This file contains the toplevel variables              */
/*                                                                          */
/****************************************************************************/

#ifndef TOPLEVEL_H

#define TOPLEVEL_H





/*
 *
 * The LEVEL structure. 
 *
 *   It holds all the necessary environment (local variables ...)
 *
 */
 
 
typedef struct level {

  unsigned int n;                   /* The level number */
  
  HASHTABLE theVariables;           /* The variables hash table */
  
  struct level *levelVar;           /* The current level we use to get variables value 
                                       (it is generally equal to the level itself, however,
                                       one could use a different level for executing a command) */
                                         
  PROC scommand;                 /* The scommand that corresponds to this level 
                                     * If there is none (i.e., it is the first level)
                                     * it is set to NULL */

  char **cmdList;                   /* The list corresponding to the command */
  struct scriptline *scriptline;    /* The current script line that is currently being executed */
  char *wordCur;
  
  int cmdNum;                       /* The cmd Number */
                                                             
  int nLoops;                       /* number of imbricated loops */ 

  char flagTrace;                   /* flag that indicates whether commands should be traced */
    
  STREAM out,in,err;                /* The 3 default standard streams */
  
} Level, *LEVEL;

/* the current level */
extern LEVEL levelCur, levelFirst, levelVar;

/* Some functions */
extern void OpenLevel(PROC sc);
extern void CloseLevel(void );


/*
 *
 * The TOPLEVEL structure. 
 *
 *   Along with the first level it holds all the necessary environment for the 
 *   global level.
 *
 */

/* Definition used for the temporary allocation system */
#define MaxNumTempAllocs 100000
#define MaxNumMarkerTempAllocs 400

/* Maximum number of imbricated levels (i.e., scommands) */
#define MaxNumLevels 100

/* Maximum number of streamed open at the same time */
#define MaxNumStreams 50

/* Maximum length for the terminal line */
#define MaxLengthTermLine 1000

/* Maximum length for the result */
#define MaxLengthResult 10000

/* Maximum length for the resultList */
#define MaxLengthResultList 14000

/* The different terminal modes */
enum {
  UnknownTMode = 0,
  ScanTMode,
  CommandTMode,
  GetcharTMode
};

/* The different terminal result mode */
enum {
  TermResultNormalMode = 1,
  TermResultHackMode,
  TermResultInfoMode
};

typedef struct toplevel {

  /* Variables for managing streams */
  int nStream;
  STREAM theStreams[MaxNumStreams];  
  
  /* These are the current streams */
  STREAM out, in, err;

  jmp_buf environment;    /* The environment in case of an error */
  
  HISTORY history;        /* The history */
  
  PROC promptProc;    /* The procedure to compute the prompt */

  char *sourceFilename;   /* The current source filename being executed */
  char *packageName;      /* The current package name being loaded */

  unsigned char termResultMode;       /* The mode the result will be printed in the terminal */                
  char flagStoreResult;               /* Should the result be stored ? */
  struct value *resultContent;  /* The result in case it is not a string */
  char *resultType;                   /* The type of the result (strType, numType,....) */               
  char result[MaxLengthResult];       /* The result if it is a string */
  int nResult;                         /* The size of the pointer array above  */
  char *begResultList[MaxLengthResultList];    /* The pointers to the result string if the result is a list  (begining of words) */
  char *endResultList[MaxLengthResultList];    /* The pointers to the result string if the result is a list  (end of words) */
  int nResultList;                     /* The size of the pointer array above  */
  float resultNum;                    /* The result of a script if it is a number */
  char flagSaveError;                 /* Should the error be saved > */
    
  struct event *lastEvent;       /* The last binded event */
  struct window *lastWindow;     /* The last visited window */
  struct gobject *theObjCur;     /* The gobject which is currently handling a message */ 
  
  unsigned long nEvents;        /* Number of events received */
                                           
  char termLine[MaxLengthTermLine+1];  /* The current terminal line */
  int termCursor;                      /* And the current cursor */
  char termMode,oldTermMode;           /* The terminal modes */
  
  int nLevel;             /* The number of levels */
  Level levels[MaxNumLevels];  /* the levels */

  /* Variables for the temporary allocation system */
  void *tempAlloc[MaxNumTempAllocs];
  unsigned  nTempAlloc;
  unsigned  markerTempAlloc[MaxNumMarkerTempAllocs];
  unsigned  nMarkerTempAlloc;  
  
  unsigned char flags;  /* set when 'continue', 'break' or 'return' command is executed */
  char flagReturn;  /* Yes if a script returned a value */ 
  
  char flagInDrawMessage; /* Yes while executing a draw message */

} *TOPLEVEL;


/* The different flagControls */
#define RETURN ((unsigned char) 1)
#define CONTINUE ((unsigned char) 1<<1)
#define BREAK ((unsigned char) 1<<2)


/* Maximum nuber of toplevels */
#define MaxNToplevels 6


/* The current toplevel number */
extern int nToplevel;

/* The toplevels */
extern TOPLEVEL theToplevels[];

/* The current toplevel */
extern TOPLEVEL toplevelCur;



/* Some extern functions */
extern void OpenToplevel(void);
extern void CloseToplevel(void);
extern void CloseAllToplevels(void);
extern void InitToplevels(void);
extern LEVEL GetLevel(int nlevel);

extern void EndOfCommandLine(void);

extern void SetCurStreams(char **argv);

/* For structure controls */
extern void StartLoop(void);
extern void EndLoop(void);
extern char IsLoop(void);
extern char IsStopFlag(void);
extern char IsReturnFlag(void);
extern char IsBreakFlag(void);
extern char IsContinueFlag(void);
extern void ClearStopFlag(void);
extern void  SetReturnFlag(void);
extern void  SetBreakFlag(void);
extern void  SetContinueFlag(void);


/* Functions in int_hash.c */
extern void NewArrayVariableLevel(LEVEL level, char *name, int size);
extern void NewArrayVariable(char *name, int size);
extern HASHTABLE GetArrayVariableLevel(LEVEL level, char *name);  
extern HASHTABLE GetArrayVariable(char *name);


#endif
