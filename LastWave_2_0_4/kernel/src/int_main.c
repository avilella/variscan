/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 4                           */
/*                                                                          */
/*      (C) 1998-2003 Copyright Emmanuel Bacry, All Right Reserved.         */
/*      Author Emmanuel Bacry                                               */
/*                                                                          */
/*..........................................................................*/



/****************************************************************************/
/*                                                                          */
/*  int_loop.c       This file contains the basic initializations required  */
/*                   by the interpreter langage, and its main loop          */
/*                                                                          */
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#include "lastwave.h"
#include "xx_system.h" /* ANSI changed */

#ifdef __cplusplus
}
#endif



/***********************************
 * This is the main loop...
 **********************************/

static char *scripts[10];
static int nScripts = 0;


/*
 * Function to manage the prompt
 */ 

static char prompt[200];
static char flagInPromptProc = NO;

#ifdef __cplusplus
extern "C" {
#endif
extern void ApplyProc2Listv(PROC proc, LISTV lv);
#ifdef __cplusplus
}
#endif


static void ComputePrompt(void)
{

  LISTV lv;
  
  flagInPromptProc = NO;

  /* If there is not a promptProc then we just print the history number */
  if (toplevelCur->promptProc == NULL) 
     sprintf(prompt,"%d > ",toplevelCur->levels[0].cmdNum);
 
  /* Otherwise we execute the promptProc */
  else {
    flagInPromptProc = YES;
    lv = TNewListv();
    ApplyProc2Listv(toplevelCur->promptProc,lv);
    flagInPromptProc = NO;
    sprintf(prompt,"%s",GetResultStr());
    InitResult();
    InitError();        
  }
}


int GetPrompt(char **pPrompt)
{
  if (pPrompt != NULL) *pPrompt = prompt;
  return(flagInPromptProc);
}  



/*
 * The main loop
 */
int TheLoop (void)
{ 
  int i,val;
  char *endp;
  char line[1000];
  char line1[1000];
  
  while (YES) {
   
    flagInPromptProc = NO;
    
    /*
     * The setjmp will return 1 if an error occured 
     * or 2 if an error occured while executing the promptProc 
     */
    if ((val = setjmp(toplevelCur->environment)) == 0) {  

      /* that's the end */
      if (nToplevel == 0) return(YES);

      line[0] = '\0';
      while(1) {       
             
        InitResult();
        InitError();        
        
        if (nScripts == 0) {
          /* We should print the prompt */
          if (*line !='\0') Printf("--> ");
          else {
            ComputePrompt();
            Printf("%s",prompt);
          }
          
          Flush();
          
          if (GetCommandLine(line1) == EOF) return(YES);
         
          /* Abort */
          if (!strcmp(line1,".")) {
            Printf("Abort...\n");
            break;
          }
        
          if (line[0] != '\0') strcat(line,"\n");
          strcat(line,line1);
        }
        else {
          strcpy(line,scripts[--nScripts]);
/*          Printf("[%s]\n",line); */
        }
        
        /* Quit ? */
        if (!strcmp("quit",line) || !strcmp("exit",line) || !strcmp("bye",line)) return(YES); 

        /* if command is a number, gets the command in the History */
        endp = line;
        while (*endp == ' ') endp++;
        if (*endp != '\0') {
          i = strtol(line,&endp,0);
          if (*endp == '\0') {
            strcpy(line,GetHistory(toplevelCur->history,i));
            Printf ("%s\n",line);
          }
        }
     
     
        /* Evaluating the script if complete (and record in history) */
        if (EvalScriptStringIfComplete(line, YES,YES) == YES) break;

      }
      
      PrintResult();        
      InitResult();      
    }
   
   /* Case an error occured while executing the promptProc (we should delete it) */
   else if (val == 2) {
     if (toplevelCur->promptProc) {
       DeleteProc(toplevelCur->promptProc);
       toplevelCur->promptProc = NULL;
     }
   }
   
    
    EndOfCommandLine();
 }

  return(YES);
}



/*****************************/
/* The start and the end     */
/*****************************/

#ifdef __cplusplus
extern "C" {
#endif
extern void CloseTerminal(void);
extern void InitProcs(void);
extern void InitTerminal(void);
extern void InitCPUBinaryMode(void) ;
#ifdef __cplusplus
}
#endif



static void Start(void)
 {
  InitCPUBinaryMode();
  
  InitTerminal();

  /* Needs to be here for the -b option ... don't understand why ???*/
  Flush();
  
  InitGraphics();


  InitToplevels();

}

static void End(void) 
{
  CloseGraphics();  
  CloseAllToplevels();
  CloseTerminal();
}


/*********************/
/* The main function */
/*********************/

/* ANSI changed */
extern void XXStartup(int *argc, char ***argv);

main(int argc,char **argv)
{
  char flag;
  
  scripts[nScripts++] = "source startup";

  XXStartup(&argc,&argv);
  flag = GraphicMode;   

  Start();
      
  Printf("***************************************************************************\n");
  Printf("****\n");
  Printf("****   L a s t W a v e   2.0.4     ");
  Printf("****\n");
  Printf("****   Copyright (C) 1998-2004 E. Bacry.\n");
  Printf("****\n");
  Printf("****   CMAP, Ecole Polytechnique, 91128 Cedex, Palaiseau, FRANCE.\n");
  Printf("****   (lastwave@cmap.polytechnique.fr )\n");
  Printf("****\n");
  Printf("****   LastWave comes with ABSOLUTELY NO WARRANTY.\n");
  Printf("****   It is a free software and you are welcome to redistribute it\n");
  Printf("****   under certain circumstances.\n");
  Printf("****   For details read the file COPYRIGHT.\n");
  Printf("****\n");
  Printf("***************************************************************************\n\n\n");

  /* Some initializations */
  InitProcs();

  if (!GraphicMode && flag == YES) {
    fprintf(stderr,"Warning : Cannot establish connection to the X Server\n");
    fprintf(stderr,"     ---> Disable all the graphic commands\n\n");
  }

  /* Some initializations */
  if (setjmp(toplevelCur->environment) == 0) {
    InitResult();
    SetResultStr("");
    while (argc != 0) {
      AppendListResultStr(*argv);
      argv++;
      argc--;
    }
    SetVariable("argv",GetResultValue());
    InitResult();
    UserInit();  
  }
  else {
    PrintfErr("** The above error occured while executing the 'UserInit' function\n"); 
    nScripts = 0;
  }

  TheLoop();
  
  End(); 
}








