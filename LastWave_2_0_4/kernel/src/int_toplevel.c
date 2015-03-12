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
/*  int_toplevel.c   Everything on toplevels                                */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
  
  
/**************************************************************************
 *
 * Functions for copying streams 
 *
 **************************************************************************/

static void Level2ToplevelStreams(LEVEL level, TOPLEVEL toplevel)
{
  stdinStream = CopyStream(&level->in,&toplevel->in);  
    
  stdoutStream = CopyStream(&level->out,&toplevel->out);  

  stderrStream = CopyStream(&level->err,&toplevel->err);  
}

static void Toplevel2LevelStreams(TOPLEVEL toplevel,LEVEL level)
{
  CopyStream(&toplevel->in,&level->in);  
  CopyStream(&toplevel->err,&level->err);  
  CopyStream(&toplevel->out,&level->out);  
}


/**************************************************************************
 *
 * Functions managing the levels 
 *
 **************************************************************************/
 
/* The current level and the first level */
LEVEL levelCur = NULL;
LEVEL levelFirst = NULL;

/* Init the fields of the current level */
static void InitLevelFields(int hashSize)
{
  if (levelCur->theVariables == NULL) levelCur->theVariables = NewHashTable(hashSize);

  levelCur->theVariables->level = levelCur;  
  levelCur->cmdNum = 0;
  levelCur->nLoops = 0;
  levelCur->cmdList = NULL;
  levelCur->scriptline = NULL;
  levelCur->flagTrace = NO;
  levelCur->in = levelCur->out = levelCur->err = NULL;
  levelCur->levelVar = levelCur;
  levelCur->scommand = NULL;
}

/* Open a new level associated to the scommand 'sc' */
void OpenLevel(PROC sc)
{  
  if (toplevelCur->nLevel == MaxNumLevels) 
    Errorf("Too many imbricated function calls (should be less than %d)",MaxNumLevels);
    
  toplevelCur->nLevel++;
  levelCur = &(toplevelCur->levels[toplevelCur->nLevel-1]);

  InitLevelFields(10);
  
  levelCur->scommand = sc;
  sc->nRef++;
  
  Toplevel2LevelStreams(toplevelCur,levelCur);  
}

/* Close the last level (never called if it is the first one) */
void CloseLevel(void)
{  
  if (toplevelCur->nLevel == 1) Errorf("Cannot close first level");

  ClearHashTable(levelCur->theVariables); 

  if (levelCur->scommand != NULL) DeleteProc(levelCur->scommand);
  
  CloseStream(levelCur->out);
  levelCur->out = NULL;
  CloseStream(levelCur->in);
  levelCur->in = NULL;
  CloseStream(levelCur->err);
  levelCur->err = NULL; 
  toplevelCur->nLevel--;

  levelCur = &(toplevelCur->levels[toplevelCur->nLevel-1]);

  levelCur->levelVar = levelCur;

  Level2ToplevelStreams(levelCur,toplevelCur);  
}

/* 
 * Getting a level variable environment from the level number 
 *
 *  If 'nlevel' is > 0 then we send back the corresponding level 
 *      (1 is for global level, 2 is the next one and so on up to toplevelCur->nLevel included)
 *  If 'nlevel' is <=0 it goes backward starting from 0 which corresponds to the current level
 */ 
 
LEVEL GetLevel(int nlevel)
{
  LEVEL level;

  /*
   * Absolute level number 
   */
  if (nlevel > 0) {
  
    /* Some basic test for absolute level number */
    if (nlevel > toplevelCur->nLevel) {
      SetErrorf("Bad level number '%d'",nlevel);
      return(NULL);
    }
    
    /* Get the corresponding level */
    level = &(toplevelCur->levels[nlevel-1]);
  }
  
  /*
   * Relative level number 
   */
  else {  
    /* Get the current Level for variables */
    level = levelCur;
    while (level->levelVar != level) level = level->levelVar;  

    /* Some basic test for relative level number */
    if (-nlevel > level->n) {
      SetErrorf("Level '%d' does not exist",nlevel); 
      return(NULL);
    }
     
    /* Get the corresponding level */  
    level = &(toplevelCur->levels[level->n+nlevel]);
  }
  
  /* Get the levelVar */
  while (level->levelVar != level) level = level->levelVar;    
  
  /* returns it */
  return(level);
}

/*
 * Parsing a level
 */
char ParseLevel_(char *arg, LEVEL defVal, LEVEL *level)
{
  int l;
  char *endp;
  
  *level = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseLevel_() : NULL string cannot be converted to a level");
    return(NO);
  }
  
  l = strtol(arg,&endp,0);
  
  if (*endp != '\0')  {
    SetErrorf("ParseLevel_() : '%s' not a level",arg);
    return(NO);
  }
  
  *level = GetLevel(l);
  
  if (*level == NULL) {
    *level = defVal;
    return(NO);
  }
  else return(YES);  
}

/* Same as above but generate an error (no default value) */
void ParseLevel(char *arg, LEVEL *level)
{
  if (ParseLevel_(arg,NULL,level) == NO) Errorf1("");
}

/**************************************************************************
 *
 * Functions managing the toplevels 
 *
 **************************************************************************/

/* Global variables of the toplevel */
int nToplevel = 0;
TOPLEVEL theToplevels[MaxNToplevels];
TOPLEVEL toplevelCur = NULL;

/* Create a new current toplevel */
void OpenToplevel(void)
{
  int i;
  
  if (nToplevel == MaxNToplevels) Errorf("Sorry too many toplevels already open");
    
  /* We create the structure */
  theToplevels[nToplevel] = (TOPLEVEL) Malloc(sizeof(struct toplevel));
  if (theToplevels[nToplevel] == NULL) Errorf("Malloc failed for toplevel");
  
  
  
  /* we create the history */
  theToplevels[nToplevel]->history = NewHistory();
  
  /* Set the current toplevel and level */      
  toplevelCur = theToplevels[nToplevel];
  levelFirst = levelCur = &(toplevelCur->levels[0]);

  /* Init the levels */
  for (i=0;i<MaxNumLevels;i++) {
    toplevelCur->levels[i].n = i;
    toplevelCur->levels[i].levelVar = &(toplevelCur->levels[i]);
  }    
    
  
  /* Initialize streams (the order here should correspond the order of the enum in streams.h) */
  toplevelCur->nStream = 0;
  for (i=0;i<MaxNumStreams;i++) toplevelCur->theStreams[i] = NULL;
  
  /* Create the standard streams (if not done already) */  
  if (_StdinStream == NULL) {
    _StdinStream = OpenBufferStream(1000);
    _StdoutStream = OpenFileStream(NULL,"w");
    _StderrStream = OpenFileStream(NULL,"w");
    _StdnullStream = OpenFileStream(NULL,"w");
  }
 
  stdinStream = toplevelCur->in = RefStream(_StdinStream);
  stdoutStream = toplevelCur->out = RefStream(_StdoutStream);
  stderrStream = toplevelCur->err = RefStream(_StderrStream);
  stdnullStream = _StdnullStream;

  toplevelCur->sourceFilename = NULL;
  toplevelCur->packageName = NULL;
  
  toplevelCur->nEvents = 0;
  toplevelCur->lastWindow = NULL;
  
  /* Init the hash tables levels */
  for (i=0;i<MaxNumLevels;i++) toplevelCur->levels[i].theVariables = NULL;
  
  
  /* Initialization of the first level */
  toplevelCur->nLevel = 1;
  InitLevelFields(100);  
  Toplevel2LevelStreams(toplevelCur,levelCur);
  
  /* Initialization of the Temporary allocation system */        
  toplevelCur->nTempAlloc = 0;
  toplevelCur->nMarkerTempAlloc = 0;
   
  /* Init the prompt script */
  toplevelCur->promptProc = NULL;
  
  /* Init of the terminal line */
  InitTerminalInput();
  toplevelCur->termMode = UnknownTMode;
   
  /* the 'return', 'break' and 'continue' flags */       
  toplevelCur->flags = 0;
    
  /* Init the last event field */  
  toplevelCur->lastEvent = NULL;

  /* Init the DrawMessage flag */
  toplevelCur->flagInDrawMessage = NO;

  /* Init the result */
  toplevelCur->nResultList = 0;
  toplevelCur->nResult = 0;
  toplevelCur->resultType = NULL;
  toplevelCur->resultContent = NULL;
  toplevelCur->flagStoreResult = YES;
  toplevelCur->flagSaveError = YES;
  toplevelCur->termResultMode = TermResultNormalMode;
  
  /* There's one more toplevel ! */
  nToplevel++;  
}  


/***************************/
/* Close the last toplevel */
/***************************/

void CloseToplevel(void)  /* merde CAREFUL WITH stdnull */
{
  int i,l;
  LEVEL level;
  char *filename;
  char *home;

  /* If it is the last Toplevel, we have to write the history file before deleting it */
  if (nToplevel == 1) {
    home = GetStrVariable("Home");
    filename = CharAlloc(strlen(home)+strlen(GetHistoryFile())+10);
    sprintf(filename,"%s/%s",home,GetHistoryFile());
    WriteHistory(toplevelCur->history,filename);
    Free(filename);
  }
  DeleteHistory(toplevelCur->history);

  /* Delete the prompt script if any */
  if (toplevelCur->promptProc != NULL) DeleteProc(toplevelCur->promptProc);

 /* Delete all the levels */
 for (l = toplevelCur->nLevel-1; l>= 0;l--) {
    level = &(toplevelCur->levels[l]);
    DeleteHashTable(level->theVariables);
    CloseStream(level->out);
    CloseStream(level->in);
    CloseStream(level->err);
  }
    
  /* Close all the non-closed streams */
  CloseStream(toplevelCur->in);
  CloseStream(toplevelCur->out);
  CloseStream(toplevelCur->err);
  for (i=toplevelCur->nStream-1;i>=0;i--) CloseStream(toplevelCur->theStreams[i]);
    
  /* There's one toplevel less ! */  
  nToplevel--;
  
  /* we restore some stuff */
  if (nToplevel != 0) {
    toplevelCur = theToplevels[nToplevel-1];
    levelCur = &(toplevelCur->levels[toplevelCur->nLevel-1]);
    levelFirst = &(toplevelCur->levels[0]);
    stdinStream = toplevelCur->in;
    stdoutStream = toplevelCur->out;
    stderrStream = toplevelCur->err;
  }
  else {
    toplevelCur = NULL;
    levelCur = levelFirst = NULL;
    stdinStream = stdoutStream = stderrStream = NULL;
  } 
  
  Free(theToplevels[nToplevel]);
}


/*
 * This function is called after each command from the terminal window
 * is executed
 */
 
void EndOfCommandLine(void) 
{
  int l,i;
  LEVEL level;
  
  /* Close Postscript file if necessary */
  PSClose(); 

  for (l = toplevelCur->nLevel-1; l>= 1;l--) {
    level = &(toplevelCur->levels[l]);
    ClearHashTable(level->theVariables);
    
    level->levelVar = level;
    
    level->scriptline = NULL;
    level->cmdList = NULL;
    
    CloseStream(level->out);
    level->out = NULL;
    CloseStream(level->in);
    level->in = NULL;
    CloseStream(level->err);
    level->err = NULL;
  }
  
  toplevelCur->sourceFilename = NULL;
  if (toplevelCur->packageName) {
    Free(toplevelCur->packageName);
    toplevelCur->packageName = NULL;
  }
  
  toplevelCur->nLevel = 1;
  levelCur = &(toplevelCur->levels[0]);
  levelCur->levelVar = levelCur;
  
  ClearStopFlag();
  levelCur->nLoops = 0;

  levelCur->cmdList = NULL;
  levelCur->scriptline = NULL;  
  
  /* Close all the non-closed streams */
  stdinStream = CopyStream(&_StdinStream,&toplevelCur->in);
  stdoutStream = CopyStream(&_StdoutStream,&toplevelCur->out);
  stderrStream = CopyStream(&_StderrStream,&toplevelCur->err);
  for (i=toplevelCur->nStream-1;i>=4;i--) CloseStream(toplevelCur->theStreams[i]);
  
  /* Clear the temporary allocation */  
  ClearAllTempAlloc();
  
  /* Init of the terminal line */
  InitTerminalInput();
  toplevelCur->termMode = UnknownTMode;
  _StdinStream->buffer->flagEof = NO;
  
  /* Clear the clip rect */
  WSetClipRect(NULL,0,0,0,0);
  
  /* Initializes gupdates */
  InitGUpdates();
  
  /* Init the drawMessage flag */
  toplevelCur->flagInDrawMessage = NO; 
}



/******************************************************************
 *
 * Initialization of the toplevels : we create the first toplevel 
 *    and the standard streams
 *
 ******************************************************************/

void InitToplevels(void)
{
  char filename[150];
   
  /* We open a toplevel ! */
  OpenToplevel();
  
  if (setjmp(toplevelCur->environment) == 0) {

    /* We init the variables */
    InitVariables();  

    /* We read the history file */
    sprintf(filename,"%s/%s",GetStrVariable("Home"),GetHistoryFile());
    ReadHistory(toplevelCur->history,filename);

    /* We set the cmdNum */
    levelFirst->cmdNum = toplevelCur->history->index+1;
  }
  else return;

}


/***************************************************
 *
 * Close all toplevels (and the standard streams)
 *
 ***************************************************/

void CloseAllToplevels(void)
{
  CloseStream(_StdinStream);
  CloseStream(_StdoutStream);
  CloseStream(_StderrStream);
  
  while (nToplevel != 0) CloseToplevel();
  toplevelCur = NULL;
}




/**************************
 *
 * the toplevel flags 
 *
 **************************/

char IsStopFlag(void)
{
  return (toplevelCur->flags & (CONTINUE | BREAK | RETURN));
}

char IsReturnFlag(void)
{
  return (toplevelCur->flags & RETURN);
}

char IsBreakFlag(void)
{
  return (toplevelCur->flags & BREAK);
}

char IsContinueFlag(void)
{
  return (toplevelCur->flags & CONTINUE);
}

void ClearStopFlag(void)
{
  toplevelCur->flags = 0;
}

void  SetReturnFlag(void)
{
  toplevelCur->flags |= RETURN;
}

void  SetBreakFlag(void)
{
  toplevelCur->flags |= BREAK;
}

void  SetContinueFlag(void)
{
  toplevelCur->flags |= CONTINUE;
}


/* Functions on loops */
void StartLoop(void)
{
  levelCur->nLoops++;
}
void EndLoop(void)
{
  levelCur->nLoops--;
  if (IsContinueFlag() || IsBreakFlag()) ClearStopFlag();
  if (!IsReturnFlag()) InitResult();
}

char IsLoop(void)
{ 
  return(levelCur->nLoops != 0);
}


/****************************************************
 *
 * Set the streams according to the command line
 * Looking for the character '200' that indicates 
 * that the command line is redirected
 *
 ****************************************************/
 
void SetCurStreams(char **argv)
{
  char *arg;
  STREAM stream;
  char *str;
  char flagIn,flagErr,flagOut,flagSIn,flagCloseStream;
  char *mode;
  int streamId;
  
  if (argv == NULL) {
    Level2ToplevelStreams(levelCur,toplevelCur);
    return;
  }
  
  /* We loop on the arguments */
  while (*argv != NULL) {
  
    arg = *argv;
  
    /* If there is no redirection then error */
    if (*arg != '>' && *arg != '<') Errorf("Expecting redirection string : %s",arg);
  
    flagIn=flagErr=flagOut=flagSIn=NO;
   
    /* Syntax analysis */
    if (!strncmp(arg,">>!",3)) {str = arg+3;mode = "a";flagErr = YES;}
    else if (!strncmp(arg,">>*",3)) {str = arg+3;mode = "a";flagErr = YES;flagOut = YES;}
    else if (!strncmp(arg,">>",2)) {str = arg+2;mode = "a";flagOut = YES;}
    else if (!strncmp(arg,">!",2)) {str = arg+2;mode = "w";flagErr = YES;}
    else if (!strncmp(arg,">*",2)) {str = arg+2;mode = "w";flagErr = YES;flagOut = YES;}
    else if (!strncmp(arg,">",1)) {str = arg+1;mode = "w";flagOut = YES;}
    else if (!strncmp(arg,"<<",2)) {str = arg+2;mode = "r";flagSIn = YES;}
    else if (!strncmp(arg,"<",1)) {str = arg+1;mode = "r";flagIn = YES;}
  
    flagCloseStream = YES;
  
    /* Let's get the stream */
    if (flagSIn) {
      stream = OpenStringStream(str);
      stdinStream = CopyStream(&stream,&toplevelCur->in);
    }
    else {
      if (flagIn && *str == '\0') Errorf("You should specify a filename or stream i.d. after '<'"); 
    
      /* We are either looking for a stream i.d. or a filename */
      if (ParseInt_(str,0,&streamId)) {
        ParseStream(str,&stream);
        if ((flagOut || flagErr) && stream->mode != StreamWrite) Errorf("The stream '%s' is not writable",str);
        if (flagIn && stream->mode != StreamRead) Errorf("The stream '%s' is not readable",str);
        flagCloseStream = NO;
      }
      else if (*str != '\0')  {
        InitError();
        stream = OpenFileStream(str,mode);
        if (stream == NULL) Errorf("Cannot open file '%s'",str);
      }
      else {
        stream = stdnullStream;
        flagCloseStream = NO;
      }
      if (flagOut) stdoutStream = CopyStream(&stream,&toplevelCur->out);
      if (flagErr) stderrStream = CopyStream(&stream,&toplevelCur->err);
      if (flagIn) stdinStream = CopyStream(&stream,&toplevelCur->in);
    }

    if (flagCloseStream) CloseStream(stream);
  
    /* Next argument */
    argv++;
  }
}






