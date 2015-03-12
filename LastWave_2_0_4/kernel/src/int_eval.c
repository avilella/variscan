/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 1                           */
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


/*
 * Set the variables of a scommand 
 *
 * It evaluates in level 'level'
 * and set them in level 'levelCur'
 */

 
void  SetVarsFromList(LEVEL level, char **varTypeList,char **varList,char **varDefList,char **valList)
{
  char *type,*var,*def,*val;
  LISTV lv;
  float f;
  VALUE value;
  
  while (level != level->levelVar) level = level->levelVar;
  
  while (varList != NULL && varList[0] != NULL) {

    /* Read the name of the argument */  
    var = *varList;
    varList++;

    /* Read the type of the argument */  
    type = *varTypeList;
    if (type == NULL) {
      if (*var == '.') type = listvType;
      else type = valType;
    }
    varTypeList++;

    /* Read the default value of the argument */  
    def = *varDefList;
    varDefList++;

    /* Case of a .list variable */
    if (*var == '.') {
      if (type == wordlistType) SetStrVariableListLevel(levelCur,var+1,valList,NO);
      else {
        lv = TNewListv();
        while (*valList) {
          if (ParseFloatValLevel_(level,*valList,&f,&value,AnyType,AnyType,YES) == NULL) Errorf1("");
          if (value) AppendValue2Listv(lv,value);
          else AppendFloat2Listv(lv,f);
          valList++;
        }
        SetVariableLevel(levelCur,var+1,(VALUE) lv);
      }
      return;
    }

    /* Read the value of the argument */  
    if (*valList == NULL) {
      if (def == NULL) {
        SetErrorf("Not enough arguments in procedure call\n");
        ErrorUsage1();
      }
      else val = def;
    }
    else {
      val = *valList;
      valList++;
    }    
    
    if (ImportFromStr(type,level,val,levelCur,var) == NO) {
      if (def == NULL) Errorf("Bad type '%s' for argument '%s'",type,val);
      if (def == val) Errorf1("\n** Bad type '%s' for default argument '%s'",type,val);
      if (ImportFromStr(type,level,def,levelCur,var) == NO) Errorf1("\n** Bad type '%s' for default argument '%s'",type,val);
      valList--;
    }
  }

  if (*valList != NULL) {
    SetErrorf("Too many arguments\n");
    ErrorUsage1();
  }
} 
   

/*
 * Set the variables of a scommand from a listv 
 */

void SetVarsFromListv(char **varTypeList,char **varList,char **varDefList,LISTV lv)
{
  int i,j;
  VALUE val;
  float f;
  LISTV lv1;
  char *type;
  
  i = j = 0;
  while(varList[i]) {

    type = varTypeList[i];
    if (type == NULL) {
      if (*(varList[i]) == '.') type = listvType;
      else type = valType;
    }
    
    if (lv->length <= j) {
      if (varDefList[i] == NULL && *(varList[i]) != '.') {
        SetErrorf("SetVarsFromListv() : Too few arguments (the listv is too short)\n");
        ErrorUsage1();
      }
      else if (*(varList[i]) == '.') {
        lv1 = TNewListv();
        for (;j<lv->length;j++) {
          GetListvNth(lv,j,&val,&f);
          if (val) AppendValue2Listv(lv1,val);
          else AppendFloat2Listv(lv1,f);
        }
        SetVariable(varList[i]+1,(VALUE) lv1);        
      }
      else {
        /* ???? ATTENTION on devrait pouvoir importer une valeur... genre {} */
        if (ImportFromStr(type,levelCur,varDefList[i],levelCur,varList[i]) == NO)
          Errorf("SetVarsFromListv() : Bad default value '%s' of type '%s' in procedure definition",varDefList[i],type);
      }
      i++;
    }
    else {
    
      if (*(varList[i]) == '.') {
        if (type == wordlistType) {
          if (varDefList[i] == NULL) Errorf("SetVarsFromListv() : No way the ('%s') %s argument can be read from a '%s' variable",wordlistType,varList[i],listvType);
          if (ImportFromStr(type,levelCur,varDefList[i],levelCur,varList[i]+1) == NO)
            Errorf("SetVarsFromListv() : Bad default value '%s' of type '%s' in procedure definition",varDefList[i],type);  
        }
        else {
          lv1 = TNewListv();
          for (;j<lv->length;j++) {
            GetListvNth(lv,j,&val,&f);
            if (val) AppendValue2Listv(lv1,val);
            else AppendFloat2Listv(lv1,f);
          }
          SetVariable(varList[i]+1,(VALUE) lv1);
        }
      }
      
      else {   
        GetListvNth(lv,j,&val,&f);
        if (ImportFromValue(type,val,f,levelCur,varList[i]) == NO) {
          if (varDefList[i] == NULL) Errorf1("");
          else {
            if (varDefList[i] == NULL) {
              SetErrorf("SetVarsFromListv() : Sorry, argument '%s' is not optional\n",varList[i]);
              ErrorUsage1();
            }
            if (ImportFromStr(type,levelCur,varDefList[i],levelCur,varList[i]) == NO)
              Errorf("SetVarsFromListv() : Bad default value '%s' of type '%s' in procedure definition",varDefList[i],type);
          }
        }
        else j++;
      }
      
      i++;
    }
  }  
  if (j != lv->length) {
    SetErrorf("SetVarsFromListv() : Too many arguments (the listv is too long)\n");
    ErrorUsage1();
  }

}


/* 
 * Execute the procedure 'proc' with the listv 'lv' as the list of argument
 *
 * WARNING : It is not yet implemented if 'proc' is a C-Procedure
 */
 
void ApplyProc2Listv(PROC proc, LISTV lv)
{
  SCRIPT script;
  char flagTrace,flag;
  int i;
  LEVEL levelVar;
  
  if (proc->flagSP == NO) Errorf("ApplyProc() : Sorry, this function is not yet implemented for C-procedures");

  levelVar = levelCur->levelVar;
  /* Open a new environment associated to this command */
  flagTrace = levelCur->flagTrace;
  OpenLevel(proc);
  
  /* Should we trace this command ?*/
  if (proc->flagTrace || flagTrace) {
    for (i=toplevelCur->nLevel-1;i>1;i--) Printf("   ");
    Printf(">>> %s ...\n",proc->name);
    levelCur->flagTrace = proc->flagTrace==2;
  }
        
  /* Set the local variables */
  SetVarsFromListv(proc->p.sp->varTypeList,proc->p.sp->varList,proc->p.sp->varDefList,lv);   

  /* This flag is used to track whether a scommand returns a value */  
  toplevelCur->flagReturn = NO;
    
  /* Eval the script corresponding to this scommand */
  script =  proc->p.sp->script;
  for (i=0; i< script->nsl; i++) {
    if (i == script->nsl-1) flag = toplevelCur->flagStoreResult;
    else flag = NO;
    EvalScriptLine(script->sl[i],NO,flag);
    if (IsReturnFlag()) break;
  }

  ClearStopFlag();

  /* Close the local environment */
  CloseLevel();    

  levelCur->levelVar = levelVar;

  /* Should we trace this command ?*/
  if (proc->flagTrace || levelCur->flagTrace)  {
    for (i=toplevelCur->nLevel;i>1;i--) Printf("   ");
    Printf("<<< %s ...\n",proc->name);
  }    
}

void ApplyProc2List(PROC proc, char **argv)
{
  SCRIPT script;
  char flagTrace,flag;
  int i;
  LEVEL l = levelCur;
  
  if (proc->flagSP == NO) Errorf("ApplyProc() : Sorry, this function is not yet implemented for C-procedures");

  /* Open a new environment associated to this command */
  flagTrace = levelCur->flagTrace;
  OpenLevel(proc);
  
  /* Should we trace this command ?*/
  if (proc->flagTrace || flagTrace) {
    for (i=toplevelCur->nLevel-1;i>1;i--) Printf("   ");
    Printf(">>> %s ...\n",proc->name);
    levelCur->flagTrace = proc->flagTrace==2;
  }
        
  /* Set the local variables */
  SetVarsFromList(l,proc->p.sp->varTypeList,proc->p.sp->varList,proc->p.sp->varDefList,argv);   

  /* This flag is used to track whether a scommand returns a value */  
  toplevelCur->flagReturn = NO;
    
  /* Eval the script corresponding to this scommand */
  script =  proc->p.sp->script;
  for (i=0; i< script->nsl; i++) {
    if (i == script->nsl-1) flag = toplevelCur->flagStoreResult;
    else flag = NO;
    EvalScriptLine(script->sl[i],NO,flag);
    if (IsReturnFlag()) break;
  }

  ClearStopFlag();

  /* Close the local environment */
  CloseLevel();    

  /* Should we trace this command ?*/
  if (proc->flagTrace || levelCur->flagTrace)  {
    for (i=toplevelCur->nLevel;i>1;i--) Printf("   ");
    Printf("<<< %s ...\n",proc->name);
  }    
}

static void _EvalScriptLine(SCRIPTLINE sl);

extern void StartAppendStr2StringList(char *strlist, int nWords, int maxSizeAlloc);
extern void EndAppendStr2StringList(void);
extern void AppendStr2StringList(char *str);

void C_Apply(char **argv)
{
  static char str[10000];
  SCRIPTLINE sl;
  LEVEL level,level1;
  int n,i;
  PROC proc;
  char *action;
  LISTV lv;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if (!strcmp(action,"listv")) {
    argv = ParseArgv(argv,tLEVEL_,levelCur,&level,tPROC,&proc,tLISTV,&lv,0);
    ApplyProc2Listv(proc,lv);
    return;
  }
  
  if (strcmp(action,"args")) Errorf("Unknown action '%s'",action);
  
  argv = ParseArgv(argv,tLEVEL_,levelCur,&level,tPROC,&proc,-1);
  
  n = 0;
  while(argv[n]!=NULL) n++;
  
  if (proc->flagStillExist == NO) proc->flagStillExist = YES;
  
  sl = NewScriptLine();
  TempPtr(sl);
  
  StartAppendStr2StringList(str,n+1,10000);
  AppendStr2StringList(proc->name);
  for(i=0;i<n;i++) AppendStr2StringList(argv[i]);
  EndAppendStr2StringList();
  sl->line = CopyStr(str);
  TempPtr(sl->line);
  
  sl->nWords = n+1;
  sl->words = argv-1;  /* bof ?????????? + Pb de comptage des mallocs en debug a cause du New script line et TempPTR */
  sl->proc = proc; 

  while (level != level->levelVar) level = level->levelVar;
  level1 = levelCur->levelVar;
  levelCur->levelVar = level;

  _EvalScriptLine(sl);

  /* restore the level if necessary */
  levelCur->levelVar = level1;

  sl->nWords = 0;
  sl->words = NULL;
  sl->proc = NULL;
}




 
/***********************************************************************************
 *
 *  Evaluating a command .... (WARNING if there is a ';' it is a script not a command)
 *
 ***********************************************************************************/

extern char *GetStringExpr(LEVEL level, char *begin, char **left, int nw);

#define MaxNDollars 30

static char **SLSubstitution(SCRIPTLINE sl)
{
  char **list,*left[MaxNDollars],*str[MaxNDollars],*str1,*word;
  int nw,i,l,m,s;
  COMPSTRUCT cs;

  /* First we build a list of the right size */
  list = TMalloc((sl->nWords+1)*sizeof(char *));
   
  /* We loop on the words */
  for (nw = 0; nw<sl->nWords;nw++) {

    cs = sl->cs[nw];
    word = sl->words[nw];
  
    /* If no dollars then just use the word */
    if (cs == NULL || cs->pos.dollar == NULL || *(cs->pos.dollar[0]) != '$') {
      list[nw] = word;
      continue;
    }
   
    /* If not we first make all the substitutions and compute the size */
    l = 0;
    for (s=0,m=0,i=0; cs->pos.dollar[i] != NULL; i++,m++) {
    
      /* Case the $ have already been taken into account by the last evaluation */
      if (m != 0 && left[m-1] > cs->pos.dollar[i]) {
        if (*(cs->pos.dollar[i]+1) == '[') i++;
        continue;
      }
        
      if (i == 0) l+= cs->pos.dollar[0]-word;
      else l+= cs->pos.dollar[i]-left[m-1];
      
      /* Case of a $[] */
      if (*(cs->pos.dollar[i]+1) == '[') {
        str[m] = GetStringExpr(levelCur,cs->pos.dollar[i],&(left[m]),nw);
        if (str[m] == NULL) Errorf1("");
        l+= strlen(str[m]);
        s++;
        i++;
      }
     
      /* Case of a $ */
      else {
        str[m] = GetStringExpr(levelCur,cs->pos.dollar[i],&(left[m]),nw);
        if (str[m] == NULL) Errorf1("");
        l+= strlen(str[m]);
      }
      if (str[m] == NULL) Errorf1("");
            
    }
    l+= strlen(left[m-1])+1;
    
    /* Alloc the corresponding word */
    list[nw] = TMalloc(sizeof(char)*l);
    
    /* And set it */
    l = 0;
    str1 = list[nw];
    for (m=0,i=0; cs->pos.dollar[i] != NULL; i++,m++) { 
 
      /* Case the $ have already been taken into account by the last evaluation */
      if (m != 0 && left[m-1] > cs->pos.dollar[i]) {
        if (*(cs->pos.dollar[i]+1) == '[') i++;
        continue;
      }
   
      if (i == 0) {
        l= cs->pos.dollar[0]-word;
        strncpy(str1,word,l);
      }
      else {
        l = cs->pos.dollar[i]-left[m-1];
        strncpy(str1,left[m-1],l);
      }
      str1 += l;
 
      if (*(cs->pos.dollar[i]+1) == '[') i++;
     
      l = strlen(str[m]);
      strncpy(str1,str[m],l);
      str1+=l;
      
    }
    strcpy(str1,left[m-1]);
  }      


  list[nw] = NULL;
  
  return(list);
}
      
/*
 * Basic function that executes a command that is described as a 'list'.
 *
 * It either returns (execution was successful) or an error occurs
 * It is executed within a TempAlloc
 */


static void _EvalScriptLine(SCRIPTLINE sl)
{
  char **list1,**list,**argv;
  PROC sc;
  PROC cc;
  char *name;
  char flagTrace;
  int i;
  char flag;
  SCRIPT script;
  char **oldCmdList;
  SCRIPTLINE oldSL;
  VALUE val;
  LEVEL theLevel;
  
  
  if (sl == NULL) Errorf("_EvalScriptLine() : weird bug");
  list = sl->words;
  
  /* If the line is empty just returns */
  if (list[0] == NULL) return;

  SetTempAlloc(); 

  /* Init the error message and the result */
  InitError();
  InitResult();

  /* Save/Set the cmdList and scriptline */
  theLevel = levelCur;
  oldCmdList = theLevel->cmdList;
  oldSL = theLevel->scriptline;
  theLevel->cmdList = sl->words;
  theLevel->scriptline = sl;
  
  /* Make $ substitution if needed */
  if ((sl->flags & SLDollarFlag) && sl->cs != NULL) {
    list = SLSubstitution(sl);
    theLevel->cmdList = list;
  }
  
  /* Make redirections of streams */  
  if (sl->redirect != NULL) SetCurStreams(sl->redirect-sl->words+list+1);
  else SetCurStreams(NULL);

  /* Set the argv */
  if (sl->flags & SLSetvFlag) argv = list;
  else argv = list+1;
  
  /*
   * Read the command 
   */
  sc = NULL;
  cc = NULL;
  
  /* Get the name */
  if (sl->flags & SLSetvFlag) name = "setv";
  else name = list[0];
  if (!strcmp(name,"!")) name = "shell";
  
  /* Case the command name has already been processed */
  if (sl->proc != NULL && (sl->proc->flagSP == NO || sl->cs == NULL || sl->cs[0] == NULL || sl->cs[0]->pos.dollar == NULL)) {

    /* If it is not the right version then delete it */
    if (!sl->proc->flagStillExist && (sl->proc->flagSP == YES || name[0] != '\\')) {
      DeleteProc(sl->proc);
      sl->proc = NULL;
    }
    /* Otherwise we set it */
    else {
      if (sl->proc->flagSP) sc = sl->proc;
      else cc = sl->proc;
    }
      
  } 
  
  /* case we have to process the command name */
  if (cc == NULL && sc == NULL) {

    /* If the command name starts with a \ then it must be a CProc */
    if (name[0] == '\\') {
      name++;
      cc = GetCProc(name);
      if (cc == NULL) Errorf("Unknown command: %s",list[0]);
      sl->proc = cc;
      cc->nRef++;
    }
    /* Otherwise just process it */
    else {      
      /* get the command */
      cc = GetProc(name);
      if (cc != NULL) {
        sl->proc = cc;
        cc->nRef++;    
        if (cc->flagSP) {
          sc = cc;
          cc = NULL;
        }
      }
    }
  }
  
  /******************
   *
   * Eval a scommand 
   *
   ******************/
    
  if (sc != NULL) {
      
    /* Open a new environment associated to this command */
    flagTrace = theLevel->flagTrace;
    OpenLevel(sc);
  
    /* Should we trace this command ?*/
    if (sc->flagTrace || flagTrace) {
      for (i=toplevelCur->nLevel-1;i>1;i--) Printf("   ");
      Printf(">>> ");
      for (list1 = list; *list1!= NULL; list1++) Printf("%s ",*list1);
      Printf("\n");
      levelCur->flagTrace = sc->flagTrace==2;
    }
        
    /* Set the local variables */
    SetVarsFromList(GetLevel(-1),sc->p.sp->varTypeList,sc->p.sp->varList,sc->p.sp->varDefList,argv);

    /* This flag is used to track whether a scommand returns a value */  
    toplevelCur->flagReturn = NO;
    
    /* Eval the script corresponding to this scommand */
    script =  sc->p.sp->script;
    for (i=0; i< script->nsl; i++) {
     if (i == script->nsl-1) flag = toplevelCur->flagStoreResult;
     else flag = NO;
     EvalScriptLine(script->sl[i],NO,flag);
     if (IsReturnFlag()) break;
    }

    ClearStopFlag();

    /* Close the local environment */
    CloseLevel();    

    /* Should we trace this command ?*/
    if (sc->flagTrace || theLevel->flagTrace)  {
      for (i=toplevelCur->nLevel;i>1;i--) Printf("   ");
      Printf("<<< ");
      for (list1 = list; *list1!= NULL; list1++) Printf("%s ",*list1);
      Printf("\n");
    }    
  }
          

  /******************
   * Eval a ccommand 
   ******************/
  else if (cc != NULL) {

    /* Should we trace this command ?*/
    if (theLevel->flagTrace || cc->flagTrace) {
      for (i=toplevelCur->nLevel-1;i>0;i--) Printf("   ");
      Printf("C>> ");
      for (list1 = list; *list1!= NULL; list1++) {
        if (sl->flags & SLSetvFlag && list1 == list+1) Printf("= ");
        if (strchr(*list1,'\n')) {
          Printf("...");
          break;
        }
        Printf("%s ",*list1);
      }
      Printf("\n");
    }

    (cc->p.cp->function)(argv);

  }

  /********************
   * If we are in terminal --> try to evaluate
   ********************/
   else if (toplevelCur->nLevel == 1 && (list[1] == NULL || !(IsValidSymbolChar1(*list[0])))) {
     if (sl->nWords == 1) ParseVal(sl->words[0],&val); /* ???? idiot : il faut concatener les words */
     else ParseVal(sl->line,&val);
     SetResultValue(val);
   }

  /********************
   * Command not found
   ********************/
   else Errorf("Unknown command: %s",list[0]);
  

  /* Restore the commands that are currently executed */
  levelCur = theLevel;
  theLevel->cmdList = oldCmdList;
  theLevel->scriptline = oldSL;
  
  SetCurStreams(NULL);
  
  /* Erase the temporary memory zone */ 
  ClearTempAlloc();
}


/*
 * This is the function to call from a C function in order to execute a command line
 *    flagHistory == YES  ==> it records the command in the history
 *    flagStoreResult == YES  ==> the result of the evaluation will be used
 */

 
void EvalList(char **list, int nWords, char flagStoreResult)
{
  struct scriptline _sl;
  SCRIPTLINE sl = &_sl;
  char oldFlag;
  
  sl->line = NULL;
  sl->nWords = nWords;
  sl->words = list;
  sl->cs = NULL;
  
  oldFlag = toplevelCur->flagStoreResult;
  toplevelCur->flagStoreResult = flagStoreResult;
  _EvalScriptLine(sl);
  toplevelCur->flagStoreResult = oldFlag;

}

     
/*
 * The corresponding command
 */

void C_Eval(char **argv)
{
extern void EvalScriptLevel(LEVEL level, SCRIPT script,char flagStoreResult);

  SCRIPT script;
  LEVEL level,level1;
   
  argv = ParseArgv(argv,tLEVEL_,levelCur,&level,tSCRIPT,&script,0);
   
  while (level != level->levelVar) level = level->levelVar;

  EvalScriptLevel(level,script,toplevelCur->flagStoreResult);
}
 
   
/*
 * This is the function to call in order to execute a command 
 * which you already have as a list.
 *    flagHistory == YES  ==> it records the command in the history
 *    flagStoreResult == YES  ==> the result of the evaluation will be used
 */

void EvalScriptLine(SCRIPTLINE sl, char flagHistory,char flagStoreResult)
{
  char oldFlag;
  
  /* Init the error message  */
  InitError();
  
  /* Record the command line in the history */
  if (flagHistory) RecordHistory(sl->line);

  oldFlag = toplevelCur->flagStoreResult;
  toplevelCur->flagStoreResult = flagStoreResult;
  
  _EvalScriptLine(sl);
  
  toplevelCur->flagStoreResult = oldFlag;
}   
  


/***********************************************************************************
 *
 *  Evaluating a script .... (i.e., multiple commands)
 *
 ***********************************************************************************/

/*
 * Basic function for executing a script which is represented by a list of commands
 */

void EvalScriptLevel(LEVEL level, SCRIPT script,char flagStoreResult)
{
  char flag;
  int i;
  LEVEL level1;
  
  InitError();
  
  if (script == NULL) 
    Errorf("EvalScriptLevel() : weired bug");
  
  if (script->nsl == 0) return;
  
  level1 = levelCur->levelVar;
  levelCur->levelVar = level;
  
  flag = NO;   
  for (i=0;i<script->nsl;i++) {
    if (flagStoreResult == YES && i==script->nsl-1) flag = YES;
    EvalScriptLine(script->sl[i],NO,flag);
    if (IsStopFlag()) return;
  }
  levelCur->levelVar = level1;
}  

void EvalScript(SCRIPT script,char flagStoreResult)
{
  EvalScriptLevel(levelCur,script,flagStoreResult);
}

/*
 * Function for executing a script which is represented by a line
 * If the script is not complete it returns NO and does not perform anything.
 * otherwise it returns YES.
 *
 */

char EvalScriptStringIfComplete(char *str,char flagHistory,char flagStoreResult)
{
  SCRIPT script;

  if (str == NULL) Errorf("EvalScriptStringIfComplete() : Weired bug");
  
  InitError();
  
  SetTempAlloc();

  if (ParseCompleteScript(&str,&script,NO) == NO) {
    ClearTempAlloc();
    return(NO);
  }

  if (script->nsl == 0) {
    ClearTempAlloc();
    return(YES);
  }
     
  if (flagHistory) RecordHistory(str);
   
  EvalScript(script,flagStoreResult);
  
  ClearTempAlloc();
  
  return(YES);
}  

 
 
 
 
           
                    
