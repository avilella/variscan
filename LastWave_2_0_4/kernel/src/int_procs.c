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



#include "lastwave.h"


/***********************************************************************************
 *
 *  Managing scriptlines
 *
 ***********************************************************************************/

COMPSTRUCT NewCompStruct (void)
{
  COMPSTRUCT cs;
  
#ifdef DEBUGALLOC
DebugType = "CompStruct";
#endif

  cs = (COMPSTRUCT) Malloc(sizeof(CompStruct));
  
  cs->pos.dollar = NULL;
  cs->scripts.dollar = NULL;

  return(cs);
}

void ClearCompStruct (COMPSTRUCT cs)
{
  SCRIPT *script1;
  
  if (cs == NULL) return;
  
  if (cs->pos.dollar != NULL) {
    Free(cs->pos.dollar);
    cs->pos.dollar = NULL;
  }
  if (cs->scripts.dollar != NULL) {
    for (script1 = cs->scripts.dollar; *script1 != NULL; script1++) DeleteScript(*script1);
    Free(cs->scripts.dollar);
    cs->scripts.dollar = NULL;
  }
}

void DeleteCompStruct (COMPSTRUCT cs)
{
  if (cs == NULL) return;

  ClearCompStruct(cs);  

#ifdef DEBUGALLOC
DebugType = "CompStruct";
#endif

  Free(cs);  
}

SCRIPTLINE NewScriptLine(void)
{
  SCRIPTLINE sl;
  
#ifdef DEBUGALLOC
DebugType = "ScriptLine";
#endif

  sl = (SCRIPTLINE) (Malloc(sizeof(struct scriptline)));
  sl->line = NULL;

  sl->nWords = 0;
  sl->words = NULL;

  sl->cs = NULL;

  sl->flags = 0;

  sl->proc = NULL;

  sl->redirect = NULL;
  
  return (sl);
}

void DeleteScriptLine(SCRIPTLINE sl)
{
  int i;
  
  if (sl->line) Free(sl->line);

  if (sl->nWords != 0 && sl->words != NULL) DeleteList(sl->words);

  if (sl->cs != NULL) {
    for (i=0;i<sl->nWords;i++) DeleteCompStruct(sl->cs[i]);
    Free(sl->cs);
  }
  
  if (sl->proc != NULL) DeleteProc(sl->proc);

#ifdef DEBUGALLOC
DebugType = "ScriptLine";
#endif

  Free(sl);
}


/* Make script line from beg,end list */

#define NMaxScripts 5

extern SCRIPT ParseScript__(char *theLine,char flagTemp, char flagEndIsBracket, char flagSubst, char flagBrace, char *flagError);

/* Warning : ptrs is a static variable */

SCRIPTLINE MakeScriptLine(char *line, int nWords, char **beg, char **end, int nPtrs, char **ptrs, char flagSetv, int redirectWord, char flagSubst)
{
  char **list;
  SCRIPTLINE sl;
  int i,j,nd,nw,ns,ns1;
  SCRIPT script;
  
  if (nWords == 0) Errorf("MakeScriptLine() : empty line");
  
  /* First we create the list of words */
  list = BegEndStr2List(beg,end);
    
  /* Allocation of the script line */
  sl = NewScriptLine();

  /* Setting the line */
  sl->line = CharAlloc(strlen(line)+1);
  strcpy(sl->line,line);
    
  /* Setting the list of words and number of words */
  sl->words = list;
  sl->nWords = nWords;
  
  /* Allocation of the COMPSTRUCT if necessary */
  if (nPtrs != 0) {
    sl->cs = Malloc(sizeof(COMPSTRUCT)*nWords);
    for (nw=0; nw<nWords;nw++) sl->cs[nw] = NULL;
  }
  else sl->cs = NULL;
  
  /* Setting the flagSetv */
  if (flagSetv) sl->flags |= SLSetvFlag;
  
  /* Setting the dollar or bracket positions and allocation */
  if (nPtrs != 0) {
  
    if (**ptrs == '$') sl->flags |= SLDollarFlag;
  
    nw = 0;
    ns = 0;
    for (i=0; i<nPtrs;) {
    
      /* This word has no dollars */
      while (nw != nWords-1 && ptrs[i] >= beg[nw+1]) nw++;
      
      /* This word has at least 1 dollar or bracket, let's allocate the compStruct */
      sl->cs[nw] = NewCompStruct();
        
      /* Counting the number of dollars or brackets and scripts */
      ns1 = 0;
      for (j=i; j<nPtrs;j++) {
        if (nw != nWords-1 && ptrs[j] >= beg[nw+1]) break;
        if (**ptrs != '$' || *(ptrs[j]+1) == '[') {ns1++;j++;}
      }
      nd = j-i;
              
      /* Allocation of the pos.dollar (i.e., pos.bracket) and dollar scripts (i.e., bracket scripts) */
      sl->cs[nw]->pos.dollar = Malloc(sizeof(char *)*(nd+1));
      if (ns1 != 0) {
        sl->cs[nw]->scripts.dollar = Malloc(sizeof(SCRIPT *)*(ns1+1));
        sl->cs[nw]->scripts.dollar[0] = NULL;
      }
      ns += ns1;
        
      /* Setting the pos.dollar */
      for (j=i;j<i+nd;j++) {
        sl->cs[nw]->pos.dollar[j-i] = (ptrs[j]-beg[nw])+list[nw];
      }      
      sl->cs[nw]->pos.dollar[j-i] = NULL;
      if (j<i+nd) break;
      
      i = j;
      nw++;   
    }
        
    if (i<nPtrs) {
      DeleteScriptLine(sl);
      return(NULL);
    }
  }
  
  
  /* Setting the scripts */
  if (nPtrs != 0 && ns != 0) {
  
    for (nw = 0; nw<sl->nWords ; nw++) {
      
      if (sl->cs[nw] == NULL || sl->cs[nw]->pos.dollar == NULL) continue;
            
      ptrs = sl->cs[nw]->pos.dollar;

      ns1 = 0;      
      for (i=0;ptrs[i]!=NULL;i++) {
        if (**ptrs != '$' || *(ptrs[i]+1) == '[') {
          if (**ptrs != '$') script = ParseScript__(ptrs[i]+1,NO,YES,flagSubst,NO,NULL);
          else script = ParseScript__(ptrs[i]+2,NO,YES,flagSubst,NO,NULL);
          if (script == NULL) break;
          sl->cs[nw]->scripts.dollar[ns1] = script;
          ns1++;
          i++;
        }
      }
          
      if (ptrs[i] != NULL) break;
      
      if (ns1 != 0) sl->cs[nw]->scripts.dollar[ns1] = NULL;
      
    }
        
    if (nw<sl->nWords) {
      DeleteScriptLine(sl);
      return(NULL);
    }
  }
  
  if (redirectWord >= 0) {
    sl->redirect = sl->words + redirectWord;
    sl->words[redirectWord] = NULL;
  } 
  else sl->redirect = NULL;
  
  return(sl);

}


/***********************************************************************************
 *
 *  Managing scripts
 *
 ***********************************************************************************/

/* the type */
char *scriptType = "&script";


/*
 * The field list
 */
struct field fieldsScript[] = {
  NULL, NULL, NULL, NULL, NULL
};


static char *ToStrScript(SCRIPT val,char flagShort)
{
  static char str[30];
   sprintf(str,"(&script)");
   return(str);
}

static void PrintScript(VALUE val)
{
  SCRIPT s = (SCRIPT) val;
  int i;
  
        Printf("(&script) {\n");
      for (i=0;i<s->nsl;i++)
        Printf("  %s\n",s->sl[i]->line);
      Printf("}\n");
}

static void PrintInfoScript(VALUE val)
{
   Printf("   nScriptLines =  %d\n",((SCRIPT) val)->nsl);
}



/*
 * The type structure for SCRIPT
 */

TypeStruct tsScript = {

  "{{{&script} {This corresponds to a script. It can be obtained from a procedure using the 'script' field of a \
'&proc' variable. A script can be directly built using the %%`a script` syntax.}}}",              /* Documentation */

  &scriptType,       /* The basic (unique) type name */
  NULL,             /* The GetType function */                       
  
  DeleteScript,     /* The Delete function */
  NewScript,     /* The New function */
  
  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrScript,        /* String conversion */
  PrintScript,   /* The Print function : print the object when 'print' is called */
  PrintInfoScript,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsScript,      /* The list of fields */
};



void DeleteScript(SCRIPT script)
{
  int i;
  
  if (script) {
    if (script->nRef==0) {
      Warningf("DeleteScript() : Trying to delete a temporary script\n");
      return;
    }
    RemoveRefValue(script);
    if (script->nRef > 0) return;

    if (script->nsl != 0 && script->sl != NULL) {
      for (i=0;i<script->nsl;i++) DeleteScriptLine(script->sl[i]);
      Free(script->sl);
    }

#ifdef DEBUGALLOC
DebugType = "Script";
#endif

    Free(script);
  }
  
}

SCRIPT NewScript(void)
{
  SCRIPT script;

#ifdef DEBUGALLOC
DebugType = "Script";
#endif
  
  
  script = (SCRIPT) (Malloc(sizeof(struct script)));
  
  InitValue(script,&tsScript);
  
  script->nsl = 0;
  script->sl = NULL;
 
  return (script);
}




/***********************************************************************************
 *
 *  Managing the CProcs
 *
 ***********************************************************************************/

/*
 * Some useful variables 
 */
 
static PROC *theCProcs;         /* Sorted Array of all the Commands which are CProcs */
static int nCProcs = 0;            /* Size of the array */
extern CProcTable cprocTables[];    /* The original table commands of the kernel */


/* Creating a new command from a ccommand */
extern void Warningf1(char *format,...);
char *_theDoc = NULL;
static char ** MakeDescriptionList(CProcTable *table, int j) 
{
  char **help;
  char *des;
  
  if (table->procs[j].description != NULL) des = table->procs[j].description;
  else {
    _theDoc = NULL;
    (*(table->procs[j].function))(NULL);
    des=_theDoc;
  }

  if (des != NULL && ParseWordList_(des,NULL,&help)) return(CopyList(help));
  
  Warningf1("** Error in the help of C-Proc '%s' (in table '%s', in package '%s')\n",table->procs[j].name,table->name,table->packageName);
  return(Str2List(NULL));
}

static PROC NewProcFromCP(CProcTable *table, int j)
{
  PROC c;
  
  c = NewProc();
  c->name = table->procs[j].name;
  c->flagSP = NO;
  c->p.cp = table->procs + j;
  c->description = MakeDescriptionList(table,j);
  c->package = CopyStr(table->packageName);
  c->procTable = table;
  
  return(c);
}


/* Function used to sort the 'theCProcs' array */
static int qsortcmp(const void *s1,const void *s2)
{
  PROC * pc1 = (PROC *) s1;
  PROC * pc2 = (PROC *) s2;
  
  return(strcmp((*pc1)->name,(*pc2)->name));
}

  
/* Function used to search in the 'theCProcs' array */
static int bsearchcmp(const void *n,const void *pc)
{
  char *name = (char *) n;
  PROC *pc1 = (PROC *) pc;
  
  return(strcmp(name,(*pc1)->name));
}

/* Function to Initialize the ccommands of the kernel */
static void InitCProcs(void)
{ 
  int n;
  
  /* We just add the command tables that are in cprocTables */
  n = 0;
  while (cprocTables[n].name != NULL) AddCProcTable(&(cprocTables[n++]));    
}


#define NCCmdTables 200
static CPROCTABLE theCCmdTables[NCCmdTables+1]; /* The table commands */


/* Function to interactively add some ccommands (they will be stored in the 'theCProcs' array */
void AddCProcTable(CProcTable *ccmdTable)
{
  int j,i,n,nb;
  PROC *array;
  CPROC commands;
  PROC *pc;

  if (ccmdTable == NULL || ccmdTable->packageName == NULL || ccmdTable->name == NULL) return;
  commands = ccmdTable->procs;

  /* Add the command table to the array of all the command tables */
  for (i=0;i<=NCCmdTables;i++) {
    if (theCCmdTables[i] == NULL) break;
  }
  if (i == NCCmdTables+1) Errorf("AddCommandTable() : Sorry already too many command tables (should be <=%d)",NCCmdTables); 
  theCCmdTables[i] = ccmdTable;
  theCCmdTables[i+1] = NULL;

  
  /* Count the number of commands in this table */
  for (nb = 0;commands[nb].name; nb++);
  if (nb == 0) return;
    
  /* 
   * In the case it is the first table, we just add it and sort it
   */
  if (nCProcs == 0) {
    theCProcs = (PROC *) Malloc(nb*sizeof(PROC));
    for (n=0,j = 0;commands[j].name; j++) {
      /* If procedure name not valid then do not define it */
      if (!IsValidProcName(commands[j].name)) {
        Warningf("** Invalid name for C-Proc '%s' (in table '%s', in package '%s') --> was not defined\n",commands[j].name,ccmdTable->name,ccmdTable->packageName);
        continue;
      }
      /* Otherwise, we define it */
      theCProcs[n] = NewProcFromCP(ccmdTable,j);
      n++;
    }
    nCProcs = n;
    qsort(theCProcs,nCProcs,sizeof(PROC),&qsortcmp);
  }

  /* 
   * Otherwise, we look whether some commands are redefined.
   * In that case we only keep the last version of it and we set the
   * name of the old version to NULL (we will have to be careful 
   * when looping on the command names using the 'theCCmdTables' array) 
   */
  else {
  
    /* Make a new array that will contain everything */
    array = Malloc((nCProcs+nb)*sizeof(PROC));
    for (j=0,n=0;j<nCProcs;j++,n++) array[n] = theCProcs[j];
    
    /* Loop on the commands to add */
    for (j = 0;commands[j].name; j++) {

      /* If procedure name not valid then do not define it */
      if (!IsValidProcName(commands[j].name)) {
        Warningf("** Invalid name for C-Proc '%s' (in table '%s', in package '%s') --> was not defined\n",commands[j].name,ccmdTable->name,ccmdTable->packageName);
        continue;
      }
    
      /* Does this command already exist ? */
      pc = (PROC *) bsearch(commands[j].name,theCProcs,nCProcs,sizeof(PROC),&bsearchcmp);
    
      /* If it does then print a warning and replace the old command */
      if (pc != NULL) {
        Warningf("AddCommandTable() : redefining C-command '%s'\n",commands[j].name);
        (*pc)->p.cp = commands+j;
        DeleteList((*pc)->description);
        (*pc)->description = MakeDescriptionList(ccmdTable,j);
      }
      else {
        array[n]= NewProcFromCP(ccmdTable,j);
        n++;
      }
    }
    
    /* Delete the old array and set the new one */
    Free(theCProcs);
    theCProcs = array;
    nCProcs = n;
    
    /* And sort it */
    qsort(theCProcs,nCProcs,sizeof(PROC),&qsortcmp);
  }   
}

/* Looking for a ccommand with name 'name' */
PROC GetCProc(char *name)
{
  PROC *pc;
  
  pc = (PROC *) bsearch(name,theCProcs,nCProcs,sizeof(PROC),&bsearchcmp);
  if (pc == NULL) return(NULL);
  return(*pc);
}

  
 
/***********************************************************************************
 *
 *  Managing the SCommands
 *
 ***********************************************************************************/
 
/* The hash table of the scommands */
static HASHTABLE procHashTable; 

static PROC NewSProc(void)
{
  PROC c;
  
  c = NewProc();
  
  c->flagSP = YES;
  
#ifdef DEBUGALLOC
DebugType = "SProc";
#endif

  c->p.sp = (SPROC) Malloc(sizeof(SProc));
 
   
  c->p.sp->varList = c->p.sp->varDefList = c->p.sp->varTypeList = NULL;
  c->p.sp->script = NULL;
  c->p.sp->filename = NULL;
  return(c);
}


/* Remove a command from the hash table (do not decrease the nref) */
PROC RemoveSProc(PROC c)
{
  PROC c1;
  char flagSP = c->flagSP;

  if (flagSP == NO) Errorf("RemoveSProc() : Weird error (%s)",c->name);

  c1 = (PROC) GetRemoveElemHashTable(procHashTable,c->name);
  if (c1 != c) 
    Errorf("RemoveSProc() Weird error");
  c1->flagStillExist = NO;
  
  return(c1);
}


/* Function to Initialize the scommands hash table (to be called at initialization) */
static void InitSProcs(void)
{
  /* We just need to create the hash table */
  procHashTable = NewHashTable(1000);  
}

/* Looking for a scommand and its index in the table */
PROC GetSProc(char *name)
{
  if (procHashTable==NULL) return(NULL);
  
  return((PROC) GetElemHashTable(procHashTable,name));
}


/* Check the name of a procedure */
char IsValidProcName(char *name)
{
  if (IsValidSymbolChar1(*name) == NO) return(NO);
  name++;
  while(*name != '\0') {
    if (IsValidSymbolChar(*name) == NO) return(NO);
    name++;
  }
  return(YES);
}


/* General GetProc */
PROC GetProc(char *name)
{
  PROC c;
  
  if (*name == '\\') return(GetCProc(name+1));
  c = GetSProc(name);
  if (c==NULL) return(GetCProc(name));
  else return(c);
}


/*
 * Get a variable Pattern
 */
int GetVarPattern(char **list,char ***type, char ***var, char ***def,char *procName)
{
  int n;
  char **list1;
  char *var1;
  
  n = GetListSize(list);
  
  *type = (char **) TMalloc((n+1)*sizeof(char *));
  *var = (char **) TMalloc((n+1)*sizeof(char *));
  *def = (char **) TMalloc((n+1)*sizeof(char *));
  
  n = 0;
  while (*list) {
    if (*list != NULL && **list == '{') {
      ParseWordList(*list,&list1);
      if (*list1 == NULL || 
          (**list1 != '&' && *(list1+1) != NULL && *(list1+2) != NULL) ||
          (**list1 == '&' && *(list1+1) == NULL) ||
          (**list1 == '&' && *(list1+1) != NULL && *(list1+2) != NULL && *(list1+3) != NULL)) {
        if (procName) Errorf("GetVarPattern() : Bad variable list in definition of procedure '%s'",procName);
        Errorf("GetVarPattern() : Bad variable pattern");
      }
      if (**list1 == '&') {
        (*type)[n] = GetArgType(*list1);
        if ((*type)[n] == NULL) {
          if (procName) Errorf("GetVarPattern() : Unknown variable type '%s' in variable list of procedure '%s'",*list1,procName);
          Errorf("GetVarPattern() : Unknown variable type '%s'\n",*list1);
        }
        list1++;
      }
      else (*type)[n] = NULL;
      (*var)[n] = list1[0];
      (*def)[n] = list1[1];
    }
    else {
      (*type)[n] = NULL;
      (*var)[n] = *list;
      (*def)[n] = NULL;
    }

    /* Check variable name */
    var1 = (*var)[n];
    if ((*var1 == '.'  && !IsValidSymbol(var1+1)) || (*var1 != '.'  && !IsValidSymbol(var1))) {
      if (procName) Errorf("GetVarPattern() : Bad variable name '%s' in variable list of procedure '%s'",var1,procName);
      Errorf("GetVarPattern() : Bad variable name '%s' in variable pattern",var1);
    }
    if (*var1 == '.' && list[1] != NULL) {
      if (procName) Errorf("GetVarPattern() : Dotted variable '%s' must be the last in variable list of procedure '%s'",var1,procName);
      Errorf("GetVarPattern() : Dotted variable '%s' must be the last in variable pattern",var1);
    }
    if (*var1 == '.' && (*type)[n] != NULL && (*type)[n] != wordlistType && (*type)[n] != listvType) {
      if (procName) Errorf("GetVarPattern() : Dotted variable '%s' must be of type '%s' or '%s' in variable list of procedure '%s'",var1,wordlistType,listvType,procName);
	  Errorf("GetVarPattern() : Dotted variable '%s' must of type '%s' or '%s'",var1,wordlistType,listvType);    
	}

    list++;
    n++;
  }

  (*type)[n] = NULL;
  (*var)[n] = NULL;
  (*def)[n] = NULL;
  
  return(n);
}


SPROC SetProc(char *name, char **list, char **help, SCRIPT script)
{
  int nargs;
  char **type,**def,**var;
  PROC sc;
  static char str[100]; 
  int i;
  char str4[30];

  /* Reading the variable pattern */
  if (*list == NULL) nargs = 0;
  else nargs = GetVarPattern(list,&type, &var, &def,NULL);

  /* Deleting former version of the function if not anonymous */
  if (name != NULL) {
    sc = GetProc(name);  
    if (sc != NULL) {
      if (sc->flagSP) {
        Printf("  ... redefining script command '%s'\n",sc->name);
        RemoveSProc(sc);
        Free(sc->name);
        sprintf(str4,"<%p>",(void *) sc);
        sc->name = CopyStr(str4);
        DeleteProc(sc);
      }
      else {
        Printf("  ... overloading C-Proc '%s'\n",sc->name);
        sc->flagStillExist = NO;
      }
    }
  }   

  /* Allocation of the sproc */ 
  sc = NewSProc();
  if (name != NULL) {
    sc->name = CopyStr(name);
  	AddElemHashTable(procHashTable,(AHASHELEM) sc);
  }
  else  {
    sprintf(str,"<%p>",(void *) sc);
    sc->name = CopyStr(str);
  }
  TempValue(sc);
  
  /* Setting the varList, varDefList, varTypeList of the scommand */
  sc->p.sp->varList = Malloc((nargs+1)*sizeof(char *));
  sc->p.sp->varDefList = Malloc((nargs+1)*sizeof(char *));
  sc->p.sp->varTypeList = Malloc((nargs+1)*sizeof(char *));
  for (i=0;i<nargs; i++) {
    sc->p.sp->varTypeList[i] = type[i];
    sc->p.sp->varList[i] = CopyStr(var[i]);
    if (def[i]) sc->p.sp->varDefList[i] = CopyStr(def[i]);
    else sc->p.sp->varDefList[i] = NULL;
  }    
  sc->p.sp->varList[i] = sc->p.sp->varTypeList[i] = sc->p.sp->varDefList[i] = NULL;    
  
  /* Set the script */
  sc->p.sp->script = script;
  script->nRef++;
  sc->flagStillExist = YES;
  
  /* Set the filename it has been sourced from */
  if (toplevelCur->sourceFilename != NULL) 
    sc->p.sp->filename = CopyStr(toplevelCur->sourceFilename);
  else
    sc->p.sp->filename = CopyStr("Terminal");
    
  /* Set the Usage and the help if any */  
  if (help != NULL) sc->description = CopyList(help);
  
  /* And the package name */
  if (toplevelCur->packageName != NULL) sc->package = CopyStr(toplevelCur->packageName);
  else sc->package = CopyStr("kernel");
  
  return((SPROC) sc);
}



/*
 * Defining a new scommand : the 'setproc' command 
 */

void C_SetProc(char **argv)
{ 
  int i;
  char **type,**var,**def,**list,**help;
  char *name;
  int nargs;
  SCRIPT script;
  PROC sc;
  static char str[100]; 

  /* reading the name */
  argv = ParseArgv(argv,tWORD,&name,-1);
  
  /* Checking the name */
  if (strcmp(name,"-") && !IsValidProcName(name)) Errorf("Invalid name for procedure '%s'",name);
  if (!strcmp(name,"-")) name = NULL;
  
  /* Counting the number of arguments */
  for (i = 0;argv[i] != NULL;i++);
  
  help = NULL;
  if (i == 2)  argv = ParseArgv(argv,tWORDLIST,&list,tSCRIPT,&script,0);
  else argv = ParseArgv(argv,tWORDLIST,&list,tLIST,&help,tSCRIPT,&script,0);
  
  SetResultValue(SetProc(name,list,help,script));
}


void  GetVarListProc (PROC c)
{
  char **varList,**varDefList, **varTypeList;

  SetResultStr("");
  
    varList = c->p.sp->varList;
    if (varList == NULL) return;
    varDefList = c->p.sp->varDefList;
    varTypeList = c->p.sp->varTypeList;
    while(*varList != NULL) {
      if (*varTypeList != NULL) {
        if (*varDefList != NULL) AppendResultf("{%s %s %s}",*varTypeList,*varList,*varDefList);
        else AppendResultf("{%s %s}",*varTypeList,*varList);
      }
      else {
        if (*varDefList != NULL) AppendResultf("{%s %s}",*varList,*varDefList);
        else AppendResultStr(*varList);
      }
      if (varList[1] != NULL) AppendResultf(" ");
      varList++;
      varDefList++;
      varTypeList++;
    }

}

/*
 * Manipulating scommands ans ccommands
 */

void C_Proc(char **argv)
{
  char *action,*name,*str,*str1,*str2,*str3;
  PROC c,sc;
  char **list,**list1;
  int i,j;
  char opt;
  char flagMatch;
  int n;
  LISTV lv,lvs,lvc,lv1;
  char str4[100];
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* help command */
  if (!strcmp(action,"help")) {
    argv = ParseArgv(argv,tSTR,&name,tLIST_,NULL,&list,0);
    if (list == NULL) {
	  c = GetProc(name);
	  if (c != NULL) {
	    lv = TNewListv();
	    if (c->description != NULL) {
	      for(list= c->description;*list != NULL;list++) {
	        ParseWordList(*list,&list1);
	        lv1 = TNewListv();
	        for(;*list1 != NULL;list1++) {
	          AppendStr2Listv(lv1,*list1);
	        }
	        AppendValue2Listv(lv,(VALUE) lv1);
	      } 
	    }
	    SetResultValue(lv);
	    return;
	  }
	  Errorf("Unknown command name '%s'",name);
	}
	else {
	  c = GetSProc(name);
	  if (c == NULL) Errorf("Unknown script command name '%s'",name);
	  if (c->description != NULL) DeleteList(c->description);
	  c->description = CopyList(list);
	}
  }


  /* get action */
  else if (!strcmp(action,"get")) {
    argv = ParseArgv(argv,tSTR,&name,0);
    c = GetProc(name);
    if (c == NULL) Errorf("Unknown procedure '%s'",name);
    SetResultValue(c);
    return;
  }

  /* var action */
  else if (!strcmp(action,"var")) {
    argv = ParseArgv(argv,tSTR,&name,0);
    SetResultStr("");
    c = GetSProc(name);
    if (c == NULL) Errorf("Unknown script command '%s'",name);
    GetVarListProc(c);
    return;
  }    

  /* The file action */
  else if (!strcmp(action,"file")) {
    argv = ParseArgv(argv,tSTR,&name,0);
    c = GetSProc(name);
    if (c == NULL) Errorf("Unknown script command '%s'",name);
    SetResultStr(c->p.sp->filename);
  }    

  /* The package action */
  else if (!strcmp(action,"package")) {
    argv = ParseArgv(argv,tSTR,&name,0);
    c = GetProc(name);
    if (c != NULL) {
      if (c->package) SetResultStr(c->package);
      return;
    }
    Errorf("Unknown command '%s'",name);
  }    

  /* The 'hash' action (used just for checking...) */
  else if (!strcmp(action,"hash")) {
    argv = ParseArgv(argv,0);
    for (i=0;i<procHashTable->nRows;i++) {
      n = 0;
      for (c = (PROC) procHashTable->rows[i]; c != NULL; c = c->next) n++;
      if (i != 0) AppendResultf(" %d",n);
      else AppendResultf("%d",n);
    }
  }
  
  /* script action */
  else if (!strcmp(action,"script")) {
    argv = ParseArgv(argv,tSTR,&name,0);
    c = GetSProc(name);
    if (c == NULL) Errorf("Unknown script command '%s'",name);  
    SetResultValue(c->p.sp->script);
  } 
  
  /* list, slist, clist  action */
  else if (!strcmp(action,"list") || !strcmp(action,"slist") || !strcmp(action,"clist")) {
    argv = ParseArgv(argv,tSTR_,"[^_]*",&str,tSTR_,"*",&str1,tSTR_,"*",&str2,-1);
    
    if (!strcmp(action,"slist")) str2 = str1;
    
    if (!strcmp(str1,"-m")) {
      str1[0] = '*';
      str1[1] = '\0';
      flagMatch = NO;
    }
    else flagMatch = YES;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
      case 'm': flagMatch = NO; break;
      default : ErrorOption(opt);
      }
    }  
    NoMoreArgs(argv);  

    lvs = NULL;
    if (!strcmp(action,"list") || !strcmp(action,"slist")) {
      lvs = TNewListv();
      lvc = NULL;
      for (i=0;i<procHashTable->nRows;i++) {
        for(c = (PROC) (procHashTable->rows[i]);c != NULL;c = c->next) {
          if (((flagMatch && MatchStr(c->name,str)) || (!flagMatch && !strcmp(c->name,str))) && 
              ((c->package == NULL && (!strcmp(str2,"*") || !strcmp(str2,""))) || (c->package != NULL && MatchStr(c->package,str2))))
            AppendStr2Listv(lvs,c->name);
        }
      }
    }

    if (!strcmp(action,"list") || !strcmp(action,"clist")) {
      
      lvc = TNewListv();
      if (*str == '\\') str3 = str+1;
      else str3 = str;     
      
      for (i=0;i<nCProcs;i++) {
        c = theCProcs[i];
        if (MatchStr(c->procTable->name,str1) && MatchStr(c->procTable->packageName,str2) &&
            ((flagMatch && MatchStr(c->name,str3)) || (!flagMatch && !strcmp(str3,c->name)))) {
          if (str3 != str) AppendListResultf("\\%s",c->name); 
          else  AppendStr2Listv(lvc,c->name);
        }
      } 
      
     } 
    
     if (lvs == NULL) SetResultValue(lvc);
     else if (lvc == NULL) SetResultValue(lvs);
     else {
       lv = TNewListv();
       AppendValue2Listv(lv,(VALUE) lvs);
       AppendValue2Listv(lv,(VALUE) lvc);
       SetResultValue(lv);
     }
  }
  
  /* ctable action */
  else if (!strcmp(action,"ctable")) {
    argv = ParseArgv(argv,tSTR_,"*",&str,tSTR_,"*",&str1,0);
    lv = TNewListv();
    for (i=0; theCCmdTables[i] != NULL; i++) {
      if (MatchStr(theCCmdTables[i]->name,str) && MatchStr(theCCmdTables[i]->packageName,str1)) {
        AppendStr2Listv(lv,theCCmdTables[i]->name);
      }
    }
    SetResultValue(lv);
  }
  
  /* delete action */
  else if (!strcmp(action,"delete")) {
  	if (*argv == NULL) Errorf("Missing <command> argument");
    while (1) {
      if (*argv == NULL) break;
      if (**argv == '%') {
        argv = ParseArgv(argv,tPROC,&c,-1);
        if (c->flagSP == NO) Errorf("Sorry cannot delete c-procedure '%s'",c->name);
      }
      else {
        argv = ParseArgv(argv,tSTR,&name,-1);
        c = (PROC) GetElemHashTable(procHashTable,name);
        if (c == NULL) Errorf("Unknown script command '%s'",name);
      }
      if (!(c->flagStillExist)) Errorf("The procedure '%s' is not in the procedure table",c->name);
      
      c->flagStillExist = NO;
      RemoveSProc(c);
      Free(c->name);
      sprintf(str4,"<%p>",(void *) c);
      c->name = CopyStr(str4);
      DeleteProc(c);
    }
  }

  /* undef action */
  else if (!strcmp(action,"undef")) {
    while (1) {
      if (*argv == NULL) break;
      argv = ParseArgv(argv,tWORD,&name,-1);
      c = (PROC) GetElemHashTable(procHashTable,name);
      if (c == NULL) Errorf("Unknown script command '%s'",name);
      c->flagStillExist = NO;
      RemoveSProc(c);
      Free(c->name);
      sprintf(str4,"<%p>",(void *) c);
      c->name = CopyStr(str4);
      DeleteProc(c);
    }
  }

  /* def action */
  else if (!strcmp(action,"def")) {
    argv = ParseArgv(argv,tWORD,&name,tPROC,&c,0);
    if (!IsValidProcName(name)) Errorf("Bad name for procedure : '%s'",name);
    if (c->flagSP == NO) Errorf("Expect a script procedure");
    c = (PROC) GetElemHashTable(procHashTable,name);
    if (c != NULL) Errorf("Expect an anonymous procedure");
    
    /* Deleting former version of the function if not anonymous */
    sc = GetProc(name);  
    if (sc != NULL) {
      if (sc->flagSP) {
        Printf("  ... redefining script command '%s'\n",sc->name);
        RemoveSProc(sc);
        Free(sc->name);
        sprintf(str4,"<%p>",(void *) sc);
        sc->name = CopyStr(str4);
        DeleteProc(sc);
      }
      else {
        Printf("  ... overloading C-Proc '%s'\n",sc->name);
        sc->flagStillExist = NO;
      }
    }
    Free(c->name);
    c->name = CopyStr(name);
  	AddElemHashTable(procHashTable,(AHASHELEM) c);
  	
  	SetResultValue(c);

  }
  
  else Errorf("Unknown action '%s'",action);
}
	

/***********************************************************************************
 *
 *  Managing the Commands
 *
 ***********************************************************************************/


/*
 * 'help' field
 */

static char *helpDoc = "{} {Gets the help of a proc as a listv}";

static void * GetHelpProcV(VALUE val, void **arg)
{
  LISTV lv,lv1;
  PROC c = (PROC) val;
  char **list,**list1;
  
  /* Documentation */
  if (val == NULL) return(helpDoc);

  lv = TNewListv();

  if (c->description != NULL) {
    for(list= c->description;*list != NULL;list++) {
	  if (ParseWordList_(*list,NULL,&list1) == NO) return(NULL);
	  lv1 = TNewListv();
	  for(;*list1 != NULL;list1++) {
	    AppendStr2Listv(lv1,*list1);
	  }
	  AppendValue2Listv(lv,(VALUE) lv1);
	} 
  }
  
  ARG_G_SetResValue(arg,lv);
  return(listvType);
}

static char *shelpDoc = "{[= <helpString>]} {Sets/Gets the help of a proc as a string}";

static void * GetSHelpProcV(VALUE val, void **arg)
{
  char *str;
  PROC c = (PROC) val;
  
  /* Documentation */
  if (val == NULL) return(shelpDoc);

  if (c->description) str = List2Str(c->description," ");
  else str = CopyStr("");
  TempStr(str);
  
  return(GetStrField(str,arg));
}

static void * SetSHelpProcV(VALUE val, void **arg)
{
  PROC c = (PROC) val;
  char **list;
  char *str;
  
  /* doc */
  if (val == NULL) return(shelpDoc);

  if (c->description) str = List2Str(c->description,"");
  else str = CopyStr("");
  
  if (SetStrField(&str,arg) == NULL) return(NULL);
  TempStr(str);
    
  if (ParseWordList_(str,NULL,&list)==NO) return(NULL);
  
  if (c->description) DeleteList(c->description);
  
  c->description = CopyList(list);

  return(strType);
}

/*
 * 'file' field
 */

static char *fileDoc = "{} {Gets the filename of a proc}";

static void * GetFileProcV(VALUE val, void **arg)
{
  PROC c;
  
  /* Documentation */
  if (val == NULL) return(fileDoc);
 
  c = (PROC) val;
  if (c->flagSP == NO) return(GetStrField("",arg));
  return(GetStrField(c->p.sp->filename,arg));
}

/*
 * 'package' field
 */

static char *packDoc = "{} {Gets the package name of a proc}";

static void * GetPackageProcV(VALUE val, void **arg)
{
  PROC c;
  
  /* Documentation */
  if (val == NULL) return(packDoc);
 
  c = (PROC) val;
  return(GetStrField(c->package,arg));
}

/*
 * 'script' field
 */

static char *scriptDoc = "{} {Gets the script of a proc}";

static void * GetScriptProcV(VALUE val, void **arg)
{
  PROC c;
  
  /* Documentation */
  if (val == NULL) return(scriptDoc);
 
  c = (PROC) val;
  if (c->flagSP == NO) {
    ARG_G_SetResValue(arg,nullValue);
    return(nullType);
  }
  ARG_G_SetResValue(arg,c->p.sp->script);
  return(scriptType);
}

/*
 * 'name' field
 */

static char *nameDoc = "{[= <name>]} {Gets/Sets the name of a proc}";

static void * GetNameProcV(VALUE val, void **arg)
{
  PROC c;
  
  /* Documentation */
  if (val == NULL) return(nameDoc);
 
  c = (PROC) val;
  return(GetStrField(c->name,arg));
}


/* The type of a command */
char *procType = "&proc";


/*
 * The field list
 */
struct field fieldsProc[] = {
  "help", GetHelpProcV, NULL, NULL, NULL,
  "shelp", GetSHelpProcV, SetSHelpProcV, NULL, NULL,
  "file", GetFileProcV, NULL, NULL, NULL,
  "package", GetPackageProcV, NULL, NULL, NULL,
  "script", GetScriptProcV, NULL, NULL, NULL,
  "name", GetNameProcV, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL
};



static char *ToStrProc(PROC val,char flagShort)
{
  static char str[100];
  
  sprintf(str,"%%%s",val->name);
  return(str);
}


static void PrintInfoProc(VALUE val)
{
  PROC c = (PROC) val;
  
     if (c->flagSP) Printf("   Script procedure '%s'\n",c->name);
      else Printf("   C-procedure '%s'\n",c->name);
      if (!(c->flagSP)) Printf("   procTable =  %s\n",c->procTable->name);
      Printf("   package =  %s\n",c->package);
}

/*
 * The type structure for PROC
 */

TypeStruct tsProc = {

  "{{{&proc} {Corresponds to procedures. The object corresponding to a defined procedure anmed 'name' can be obtained using the \
syntax %name. One can build anonymous procedure usinng the syntax %{arg1 ... argN}`my script`}}}",              /* Documentation */

  &procType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteProc,     /* The Delete function */
  NewProc,     /* The new function */

  NULL,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrProc,       /* String conversion */
  NULL,  /* The Print function : print the object when print is called */
  PrintInfoProc,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsProc,      /* The list of fields */
};



/* 
 * Clean the fields of a Proc except the 'name' field 
 */
 
void CleanProc(PROC c)
{
  int i;
  
  if (c->description != NULL) DeleteList(c->description);
  c->flagTrace = NO;
  if (c->package) Free(c->package);
  c->package = NULL;
  c->flagStillExist = YES;
  
  /* Case of a cc (Do not delete the ccommand strcuture !) */
  if (c->flagSP == NO) c->p.cp = NULL;

  /* Case of a sc */
  else {
    if (c->p.sp->filename != NULL) Free(c->p.sp->filename);
    if (c->p.sp->script != NULL) DeleteScript(c->p.sp->script);
    if (c->p.sp->varList != NULL) {
      for (i=0;c->p.sp->varList[i] != NULL;i++) {
        Free(c->p.sp->varList[i]);  
        if (c->p.sp->varDefList[i] != NULL) Free(c->p.sp->varDefList[i]);  
      }
      Free(c->p.sp->varList);
      Free(c->p.sp->varDefList);
      Free(c->p.sp->varTypeList);    
    }
  
    c->p.sp->varList = c->p.sp->varTypeList = c->p.sp->varDefList = NULL;

    #ifdef DEBUGALLOC
    DebugType = "SProc";
    #endif
    Free(c->p.sp);
    c->p.sp = NULL;
  }
}

/* Delete a command */
void DeleteProc(PROC c)
{
  PROC c1;
  char flagSP = c->flagSP;

  /* remove a reference */
  c->nRef--;
  if (c->nRef>0) return;

  if (flagSP == YES && c->flagStillExist) {  
    c1 = (PROC) GetElemHashTable(procHashTable,c->name);
    if (c1 != NULL) RemoveSProc(c);
  }

  /* Clean it */
  CleanProc(c);
  
  /* clean it a little more (in a case of a sc) */
  if (flagSP == YES) {
    if (c->name != NULL) Free(c->name);
  }

  #ifdef DEBUGALLOC
  DebugType = "Proc";
  #endif

  Free(c);
}




/* Allocation of an Proc */
PROC NewProc(void)
{
  PROC c;
  
#ifdef DEBUGALLOC
DebugType = "Proc";
#endif


  c = (PROC) Malloc(sizeof(Proc));
  InitValue(c,&tsProc);

  c->name = NULL;
  c->flagStillExist = YES;
  c->flagTrace = NO;
  c->package = NULL;
  c->description = NULL;
  c->package = NULL;
  c->flagSP = NO;
  c->p.sp = NULL;
  c->procTable = NULL;

  return(c);
}

  

/***********************************************************************************
 *
 *  Sourcing a file
 *
 ***********************************************************************************/

/*
 * The default source directories
 */
#define MaxNumSourceDirectories 1000
static char *sourceDirectories[MaxNumSourceDirectories];
static int  nSourceDirectories = 0;
 
void InitSourceDirs(char *dir)
{
  SetPackageDir(dir);
  AddSourceDir(dir);
}

/*
 * Add a source directories
 */

void AddSourceDir(char *dir)
{
  int i;
  
  for (i=0;i<nSourceDirectories;i++) {
    if (!strcmp(dir, sourceDirectories[i])) return;
  }
  
  if (nSourceDirectories == MaxNumSourceDirectories) 
    Errorf("AddSourceDir() : Sorry too many directories already loaded");
  
  sourceDirectories[nSourceDirectories] = CopyStr(dir);
  nSourceDirectories++;
}

/*
 * Set the source directories
 */
void C_SetSourceDirs(char **argv)
{
  LISTV lv;
  int i;
  
  argv = ParseArgv(argv,tLISTV_,NULL,&lv,0);
    
  /* If no arguments then we just return the directories */ 
  if (lv == NULL) {
    lv = TNewListv();
    for (i=0;i<nSourceDirectories;i++) AppendStr2Listv(lv,sourceDirectories[i]);
    SetResultValue(lv);  
    return;
  }
  
  /*
   * Otherwise we set the source directories 
   */
      
  /* We first delete former directories if any */
  while (nSourceDirectories != 0) {
    Free(sourceDirectories[nSourceDirectories-1]);
    nSourceDirectories--;
  }
    
  for (i=0;i<lv->length;i++) AddSourceDir(GetListvNthStr(lv,i));
}


/*
 * Let's source a file 
 */
 
#define MaxSourceLength 30000

char Source(char **filenameList)
{
  char theFilename[400],*oldFile,*str;
  char theSource[MaxSourceLength+1];
  FILE *stream;
  int i,n;
  char flagPackageFile;  
  SCRIPT script;
  char flagFirst,*filename;
  
  flagFirst = YES;
  while(*filenameList) {

    filename = *filenameList;
    
    /* Get the associated packageName if any */
    flagPackageFile = NO;
    if (toplevelCur->packageName == NULL) {
      str = filename;
      while(*str == '.') str++;
      while(*str && *str != '.') str++;
      if (*str == '.') {
        if (!strcmp(str,".pkg")) {
          *str = '\0';
          toplevelCur->packageName = CopyStr(filename);
          flagPackageFile = YES;
          *str = '.';
        }
      }
    }  
       
    for (i=0;i<nSourceDirectories;i++) {
      theFilename[0] = '\0';
      if (strcmp(".",sourceDirectories[i])) {
        strcat(theFilename,sourceDirectories[i]);
        strcat(theFilename,"/");
      }
      strcat(theFilename,filename);
      stream = FOpen(theFilename,"r");
      if (stream!=NULL) break;
    }
  
    if (stream == NULL) {
      SetErrorf("I could not find the '%s' source file",filename);
      if (flagPackageFile) {
        Free(toplevelCur->packageName);
        toplevelCur->packageName = NULL;
      }
      return(NO);
    }
  
    /* So now we have to read the source file */  
    if (flagFirst) Printf("[sourcing %s",filename);
    else Printf(" %s",filename);
    if (filenameList[1] == NULL) Printf("]\n");
    
    n = fread(theSource,sizeof(char),MaxSourceLength,stream);
    theSource[n] = '\0';
  
    if (!feof(stream)) {
      Errorf("C_Source() : Sorry, file to source '%s' is too big (should be less than %d characters)",theFilename,MaxSourceLength);
    }
  
    oldFile = toplevelCur->sourceFilename;
  
    toplevelCur->sourceFilename = theFilename;

    str = theSource;
    ParseStrScript(&str,&script);
    EvalScript(script,NO);
    
    SetResultStr(theFilename);  

    toplevelCur->sourceFilename = oldFile;
    
    if (flagPackageFile) {
      Free(toplevelCur->packageName);
      toplevelCur->packageName = NULL;
    }

    FClose(stream);
    
    filenameList++;
    flagFirst = NO;
  }
 
  return(YES);
}


void C_Source(char **argv)
{
  if (Source(argv) == NO) Errorf1("");
}
 
/*
 *
 *
 */
 
void InitProcs(void)
{
  InitCProcs();
  InitSProcs();
}
