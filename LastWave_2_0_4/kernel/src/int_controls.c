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
#include "signals.h"
#include "images.h"
#include "int_fsilist.h"


void C_Break(char **argv)
{
  NoMoreArgs(argv);
  
  if (!IsLoop()) Errorf("'break' command cannot be called outside of the body of a loop");
  
  SetBreakFlag();
}

void C_Continue(char **argv)
{
  NoMoreArgs(argv);
  
  if (!IsLoop()) Errorf("'continue' command cannot be called outside of the body of a loop");
  
  SetContinueFlag();
}

void C_Return(char **argv)
{
  char *result;
  char flag;
  float f;
  
  argv = ParseArgv(argv,tWORD_,NULL,&result,0);

  SetReturnFlag();

  if (result) {
    flag = toplevelCur->flagStoreResult;
    toplevelCur->flagStoreResult = YES;

    /* Conversion automatique ?????? A ENLEVER faire return = returnv */
    if (ParseFloat_(result,0,&f)) SetResultFloat(f);
    else SetResultStr(result);  
    
    toplevelCur->flagStoreResult = flag;
    toplevelCur->flagReturn = YES;
  } 
  else toplevelCur->flagReturn = NO;
}

void C_Returnv(char **argv)
{
  char *expr,*type;
  float f;
  VALUE val;
  char flag;
    
  if (*argv == NULL) { 
    SetReturnFlag();
    toplevelCur->flagReturn = NO;
    return;
  }
    
  argv = ParseArgv(argv,tWORD,&expr,-1);
  
  if (*argv != NULL) {
    if (argv[1] != NULL) ErrorUsage();
    if (strcmp(argv[0],"-e")) ErrorUsage();
    type = NULL;
  }
  else {    
    type = ParseFloatValLevel_(levelCur,expr,&f,&val,AnyType,AnyType,YES);
    if (type == NULL) Errorf1("");
  }

  SetReturnFlag();

  flag = toplevelCur->flagStoreResult;
  toplevelCur->flagStoreResult = YES;
  
  if (type == NULL) SetResultStr(expr);
  else if (val != NULL) SetResultValue(val);
  else SetResultFloat(f);

  toplevelCur->flagStoreResult = flag;
  toplevelCur->flagReturn = YES;
}


/*
 * The 'for' loop
 *
 *    for {beginScript} {whileExpr} {nextScript} {bodyScript}
 */
 
void C_For(char **argv)
{
  SCRIPT beginScript,nextScript,bodyScript;
  char *whileExpr;
  float result;
   
  argv = ParseArgv(argv,tSCRIPT,&beginScript,tWORD,&whileExpr,tSCRIPT,&nextScript,tSCRIPT,&bodyScript,0);
  
  EvalScript(beginScript,NO);
  
  StartLoop();
  
  while (1) {
  
    if (!ParseNumLevel_(levelCur,whileExpr,&result,YES,YES,YES)) {
      if (*whileExpr == '{') Errorf("The test of an 'for' clause should not be within {}"); 
      Errorf1("");  
    }
    
    if (result == 0) break;

    EvalScript(bodyScript,NO);

    if (IsReturnFlag() || IsBreakFlag()) break;
    if (IsContinueFlag()) ClearStopFlag();

    EvalScript(nextScript,NO);
  }
     
  EndLoop();
}

/*
 * The 'foreach' loop
 *
 *    foreach var {list} {body}
 */
 
void C_Foreach(char **argv)
{
  char *var;
  VALUE val;
  char *type;
  LISTV lv;
  char **list;
  float f;
  SIGNAL sig;
  RANGE rg;
  SCRIPT bodyScript;
  int i;
  float x;
 
  /* Parse the variable name */
  argv = ParseArgv(argv,tVNAME,&var,-1);
  
  /* Parse the value */
  if (ParseVal_(*argv,NULL,&val) == NO) Errorf1("");
  type = GetTrueTypeValue(val);
  argv++;

  /* Parse the script */
  argv = ParseArgv(argv,tSCRIPT,&bodyScript,0);
  
  /* Start the loop */
  StartLoop();
  
  /* 
   * Case of a listv 
   */
  if (type == listvType) {
    lv = CastValue(val,LISTV);
    
    for (i=0;i<lv->length;i++) {
      if (lv->values[i] == NULL) SetNumVariable(var,lv->f[i]);
      else SetVariable(var,lv->values[i]);
          
      EvalScript(bodyScript,NO);
      
      SetVariable(var,nullValue);

      if (IsReturnFlag() || IsBreakFlag()) break;
      if (IsContinueFlag()) ClearStopFlag();
    }
  }
     
  /* 
   * Case of a list 
   */
  else if (type == strType) {
    list = GetListFromStrValue(CastValue(val,STRVALUE));
    
    while (1) {
      if (*list == NULL) break;
      SetStrVariable(var,*list);
      list++;

      EvalScript(bodyScript,NO);

      if (IsReturnFlag() || IsBreakFlag()) break;
      if (IsContinueFlag()) ClearStopFlag();
    }
  }

  /* 
   * Case of a number 
   */
  else if (type == numType) {
    f = CastValue(val,NUMVALUE)->f;
    SetNumVariable(var,f);

    EvalScript(bodyScript,NO);

    if (IsContinueFlag()) ClearStopFlag();
  }

  /* 
   * Case of a signal 
   */
  else if (type == signalType) {
    sig = CastValue(val,SIGNAL);
    for (i=0;i<sig->size;i++) {
      SetNumVariable(var,sig->Y[i]);
          
      EvalScript(bodyScript,NO);

      if (IsReturnFlag() || IsBreakFlag()) break;
      if (IsContinueFlag()) ClearStopFlag();
    }
  }

  /* 
   * Case of a range 
   */
  else if (type == rangeType) {
    rg = CastValue(val,RANGE);
    
    for (x=RangeFirst(rg),i=0;i<rg->size;i++,x+=rg->step) {
      SetNumVariable(var,x);
          
      EvalScript(bodyScript,NO);

      if (IsReturnFlag() || IsBreakFlag()) break;
      if (IsContinueFlag()) ClearStopFlag();
    }
  }
  
  /* Error */
  else Errorf("Bad type '%s' to perform a foreach loop on (only &listv,&list,&signal,&num,&range)",type); 
    
  EndLoop();
}


/*
 * The 'while' loop
 *
 *    while {test} {bodyScript}
 */
 
void C_While(char **argv)
{
  SCRIPT bodyScript;
  char *whileExpr;
  float result;
   
  argv = ParseArgv(argv,tWORD,&whileExpr,-1);
  
  StartLoop();

  bodyScript = NULL;

  while (1) {

    if (!ParseNumLevel_(levelCur,whileExpr,&result,YES,YES,YES)) {
      if (*whileExpr == '{') Errorf("The test of a 'while' clause should not be within {}"); 
      Errorf1("");  
    }

    if (result == 0) break;

    if (bodyScript == NULL) {
     argv = ParseArgv(argv,tSCRIPT,&bodyScript,0);
    }
    
    EvalScript(bodyScript,NO);

    if (IsReturnFlag() || IsBreakFlag()) break;
    if (IsContinueFlag()) ClearStopFlag();
  }
     
  EndLoop();
}

/*
 * The 'do' loop
 *
 *    do {bodyScript} {test} 
 */
 
void C_Do(char **argv)
{
  SCRIPT bodyScript;
  char *whileExpr;
  float result;
   
  argv = ParseArgv(argv,tSCRIPT,&bodyScript,tWORD,&whileExpr,0);

  StartLoop();

  while (1) {

    EvalScript(bodyScript,NO);

    if (IsReturnFlag() || IsBreakFlag()) break;
    if (IsContinueFlag()) ClearStopFlag();

    if (!ParseNumLevel_(levelCur,whileExpr,&result,YES,YES,YES)) {
      if (*whileExpr == '{') Errorf("The test of a 'do' clause should not be within {}"); 
      Errorf1("");  
    }

    if (result == 0) break;

  }
     
  EndLoop();
}



/*
 * if {expr} {body1} elseif {expr} {body2} .... else {bodyn}
 *
 */
 
void C_If(char **argv)
{
  char *test;
  SCRIPT bodyScript;
  float result;
    
   while (1) {   

    argv = ParseArgv(argv,tWORD,&test,-1);

    if (!ParseNumLevel_(levelCur,test,&result,NO,YES,YES)) {
      if (*test == '{') Errorf("The test of an 'if' clause should not be within {}"); 
      Errorf1("");
    }
        
    if (*argv == NULL) Errorf("BodyScript is missing (Check that the '{' is not misplaced)");

    /* Test is successful */
    if (result) {
      argv = ParseArgv(argv,tSCRIPT,&bodyScript,-1);
      EvalScript(bodyScript,toplevelCur->flagStoreResult);
      return;
    }
    argv++;
    if (*argv == NULL) return;
    
    /* Case of an else */
    if (!strcmp(*argv,"else")) {
      argv++;
      argv = ParseArgv(argv,tSCRIPT,&bodyScript,0);
      EvalScript(bodyScript,toplevelCur->flagStoreResult);
      return;
    }
    
    /* case of an elseif */
    if (!strcmp(*argv,"elseif")) {
      argv++;
      continue;
    }
    
    Errorf("If : Too many arguments");
  }
}
