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
/*  stream.c   Everything on streams                                        */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "xx_system.h"
#include <stdarg.h>

/*#define DEBUG */

/***************************************************************
 *
 *   Functions on buffers
 *
 ***************************************************************/

/*
 * Allocation 
 */
BUFFER NewBuffer(int size)
{
  BUFFER buf;
  
  if (size <= 0) Errorf("NewBuffer() : size of buffer cannot be <= 0 (size == %d)",size);
 
 #ifdef DEBUGALLOC
DebugType = "Buffer";
#endif
 
  buf = (BUFFER) Malloc(sizeof(struct buffer_));
  buf->buffer = Malloc(sizeof(long)*(size+1));
  buf->size = size+1;
  buf->start = buf->end = buf->buffer;
  buf->flagEof = NO;
  
  return(buf);
}

/*
 * Desallocation
 */
void DeleteBuffer(BUFFER buf)
{
  if (buf != NULL) {
    Free(buf->buffer);
 #ifdef DEBUGALLOC
DebugType = "Buffer";
#endif
    Free(buf);
  }
}

/*
 * Push a long into a buffer
 */
char PushBuffer(BUFFER b,long c)
{
  if (b->flagEof) return(NO);
  
  /* We return NO if no room in the buffer */
  if ((b->end-b->start+b->size)%b->size ==  b->size-1) return(NO);

  *(b->end) = c;
  b->end = ((b->end-b->buffer)+1)%b->size+b->buffer;
  return(YES);
}

/*
 * Pull a long from a buffer (0 if empty)
 */
long PullBuffer(BUFFER b)
{
  long c;
  
  if (b->flagEof == YES) return(EofKC);
  
  if (b->end == b->start) return(0);

  c = *(b->start);
  b->start = ((b->start-b->buffer)+1)%b->size+b->buffer;
  
  if (c == EofKC) b->flagEof = YES;
  
  return(c);
} 



/*
 * Copy the buffer to a string 
 */
long Buffer2Str(BUFFER b, char *str)
{
  int i;
  long *pc;
  
  if (b->flagEof) {
    str[0] = '\0';
    return(0);
  }
  
  if (*(b->start) == EofKC) {
    b->flagEof = YES;
    str[0] = '\0';
    return(0);
  }
      
  i = 0;
  while (1) {
    pc = ((b->start-b->buffer)+i)%b->size+b->buffer;
    if (pc == b->end || *pc == EofKC) break;
    *(str++) = *pc;
    i++;
  }  
  *str = '\0';
} 

/*
 * Returns the next element in the buffer without pulling it
 */
long AskBuffer(BUFFER b)
{
  if (b->flagEof == YES) return(EofKC);
  
  if (b->end == b->start) return(0);
 
  return(*(b->start));
}

/*
 * Ask if there is an occurence of a new line or an eof in the buffer. 
 */
char AskBufferNewLine(BUFFER b)
{
  int i;
  long c;
  
  if (b->flagEof) return(YES);
  
  if (b->end == b->start) return(NO);

  i = 0;
  while(1) {
    if (((b->start-b->buffer)+i)%b->size+b->buffer == b->end) return(NO);
    c = *(((b->start-b->buffer)+i)%b->size+b->buffer);
    if (c == NewlineKC || c == EofKC) return(YES);
    i++;
  }
} 


/***************************************************************
 *
 *   Functions on streams
 *
 ***************************************************************/

 
/*
 * The standard streams (not redirectionned) 
 */
STREAM _StdinStream = NULL;
STREAM _StdoutStream = NULL;
STREAM _StderrStream = NULL;
STREAM _StdnullStream = NULL;

/*
 * The standard streams (evenetually redirectionned) 
 */
STREAM stdinStream = NULL;
STREAM stdoutStream = NULL;
STREAM stderrStream = NULL;
STREAM stdnullStream = NULL;


/*
 * Convert a unix type filename in a real filename
 */

/* ANSI changed */

char *ConvertFilename(char *file)
{
  return(XXConvertFilename(file));
}

/*
 * Convert a unix type filename in a real filename and open it
 *    returns the FILE * or NULL (if file could not be opened)
 */

static FILE * _FOpen(char *file, const char * mode)
{
  return(fopen(ConvertFilename(file),mode));  
}

/*
 *  FOpen and FClose routines (called by C-functions that don't want to deal with Streams)
 */

FILE * FOpen(char *filename, char * mode)
{
  STREAM stream; 
  
  /* We open a stream */
  stream = OpenFileStream(filename,mode);
  
  if (stream == NULL) return(NULL);
  else return(stream->stream);
}

int FClose(FILE *s)
{
  int i;
  
  if (s == NULL) return(1);
  
  /* We look for the corresponding stream */
  for (i=4;i<MaxNumStreams;i++) 
    if (toplevelCur->theStreams[i] != NULL && toplevelCur->theStreams[i]->stream == s) break;

  if (i != MaxNumStreams) CloseStream(toplevelCur->theStreams[i]);
  else fclose(s);
  
  return(1);
}


/*
 * Allocation of a stream associated to a file 
 *    (filename == NULL for opening stderr or stdout)
 *   returns NULL if could not be opened
 */
STREAM OpenFileStream(char *filename,char *mode) 
{
  STREAM stream;
  FILE *f;
  int id,i;
  int nStream;
  char mymode;
  
  /* Looking for some room in the toplevel stream array */
  if (toplevelCur->nStream < MaxNumStreams)  {
    id = toplevelCur->nStream;
    nStream = toplevelCur->nStream+1;
  }
  else {
    for (i=4;i<MaxNumStreams;i++) if (toplevelCur->theStreams[i] == NULL) break;
    if (i == MaxNumStreams) Errorf("NewStream() : too many streams already opened (< %d)",MaxNumStreams);
    id = i;
    nStream = toplevelCur->nStream;
  }
    
  /* Checking the mode */
  if (!strcmp(mode,"r")) mymode = StreamRead;
  else if (!strcmp(mode,"w")) mymode = StreamWrite;
  else if (!strcmp(mode,"a")) mymode = StreamWrite;
  else Errorf("NewStream() : Bad mode '%s'",mode);
  
  /* Opening 'filename' if != NULL */
  if (filename != NULL) {
    f = _FOpen(filename,mode);
    if (f == NULL) return(NULL);
  }
  else f = NULL;
    
  /* Allocation of the stream structure */
  stream = (STREAM) Malloc(sizeof(struct stream_));
  
  /* Let's fill up the fields ! */
  stream->mode = mymode;
  stream->refCount = 1;
  stream->stream = f;
  stream->id = id;
  stream->buffer = NULL;
  
  toplevelCur->nStream = nStream;
  toplevelCur->theStreams[id] = stream;
  
#ifdef DEBUG
printf("stream %d  ---> %d\n",stream->id,stream->refCount);
#endif  

  return(stream);
}

/*
 * Allocation of a input stream associated to a buffer 
 */
STREAM OpenBufferStream(int size) 
{
  STREAM stream;
  int id,i;
  int nStream;
  
  /* Looking for some room in the toplevel stream array */
  if (toplevelCur->nStream < MaxNumStreams)  {
    id = toplevelCur->nStream;
    nStream = toplevelCur->nStream+1;
  }
  else {
    for (i=4;i<MaxNumStreams;i++) if (toplevelCur->theStreams[i] == NULL) break;
    if (i == MaxNumStreams) Errorf("NewStream() : too many streams already opened (< %d)",MaxNumStreams);
    id = i;
    nStream = toplevelCur->nStream;
  }
    
  /* Allocation of the stream structure and buffer */
  stream = (STREAM) Malloc(sizeof(struct stream_));
  stream->buffer = NewBuffer(size);
  
  /* Let's fill up the fields ! */
  stream->mode = StreamRead;
  stream->refCount = 1;
  stream->stream = NULL;
  stream->id = id;
  
  toplevelCur->nStream = nStream;
  toplevelCur->theStreams[id] = stream;

#ifdef DEBUG
printf("stream %d  ---> %d\n",stream->id,stream->refCount);
#endif  
  
  return(stream);
}

/*
 * Allocation of a input stream associated to a input buffer and we fill it up with a string
 */
 
#define MaxStringBuffer 500
 
STREAM OpenStringStream(char *str)
{
  int i;
  
  STREAM stream;
  
  if (strlen(str)+1 > MaxStringBuffer) 
    Errorf("OpenStringStream() : Buffer is too large (asked for %d and max is %d)",strlen(str)+1,MaxStringBuffer);
    
  stream = OpenBufferStream(strlen(str)+1);
  
  i = 0;
  while (*str != '\0') stream->buffer->buffer[i++] = *(str++);
  stream->buffer->buffer[i] = EofKC;
  
  stream->buffer->end = stream->buffer->buffer+stream->buffer->size -1;
  
  return(stream);
} 


/*
 * DesAllocation of a stream 
 */
void CloseStream(STREAM stream)
{
  if (stream == NULL) return;
  
  stream->refCount--;
#ifdef DEBUG
printf("stream %d  ---> %d\n",stream->id,stream->refCount);
#endif  
  
  if (stream->refCount == 0) {
    toplevelCur->theStreams[stream->id] = NULL;
    while (toplevelCur->nStream != 0 && toplevelCur->theStreams[toplevelCur->nStream-1] == NULL) toplevelCur->nStream--;
    if (stream->stream != NULL) fclose(stream->stream);
    if (stream->buffer != NULL) DeleteBuffer(stream->buffer);
    Free(stream);
  }
}

/*
 * Add 1 reference to a stream and returns it
 */
STREAM RefStream(STREAM stream)
{
  if (stream == NULL) return(NULL);
  
  stream->refCount++;
#ifdef DEBUG
printf("stream %d  ---> %d\n",stream->id,stream->refCount);
#endif  
  return(stream);
}


/*
 * Copy a stream in an other one (after closing the latter) and returns the stream
 */
STREAM CopyStream(STREAM *streamIn,STREAM *streamOut)
{
  if (*streamOut == *streamIn) return(*streamOut);
  
  CloseStream(*streamOut);
  *streamOut = NULL;
  
  *streamOut = RefStream(*streamIn);   
  
  return(*streamOut);
}



/*
 *  Parsing a stream
 */

char ParseStream_(char *arg, STREAM defVal, STREAM *stream)
{
  int s;
  
  *stream = defVal;
  if (arg == NULL) {
    SetErrorf("ParseStream_() : NULL string cannot be converted to a stream");
    return(NO);
  }
  
  if (!strcmp(arg,"stdin"))  {
    *stream = toplevelCur->in;
    return(YES);
  }
  
  if (!strcmp(arg,"stdout"))  {
    *stream = toplevelCur->out;
    return(YES);
  }

  if (!strcmp(arg,"stderr"))  {
    *stream = toplevelCur->err;
    return(YES);
  }

  if (!strcmp(arg,"stdnull"))  {
    *stream = stdnullStream;
    return(YES);
  }
  
  if (ParseInt_(arg,0,&s)) {
    if (s< 0) {
      SetErrorf("ParseStream_() : '%d' invalid stream i.d. (must be positive)",s);
      return(NO);
    }
    if (s >= toplevelCur->nStream) {
      SetErrorf("ParseStream_() : '%d' invalid streamId i.d. (too large --> only '%d' streams are currently open)",s, toplevelCur->nStream);
      return(NO);
    }
    if (toplevelCur->theStreams[s] == NULL) {    
      SetErrorf("ParseStream_() : '%d' invalid streamId i.d.",s);
      return(NO);
    }  
    *stream = toplevelCur->theStreams[s];
    return(YES);
  }
  
  SetErrorf("ParseStream_() : '%s' invalid stream name",arg);
  return(NO);
}  
  

/* Same as above but generate an error (no default value) */
void ParseStream(char *arg, STREAM *stream)
{
  if (ParseStream_(arg,NULL,stream) == NO) Errorf1("");
}

/*
 * Function for testing whether a stream is eof
 */
char FEof(STREAM stream)
{
  if (stream->mode == StreamWrite) 
    Errorf("FEof() : the stream is a write only stream");
  
  if (stream->buffer != NULL) return(stream->buffer->flagEof);

  return(feof(stream->stream));
}

/*
 * Function for testing whether the stdin is eof
 */
char Eof(void)
{
  return(FEof(toplevelCur->in));
}


/*
 * The file command
 */
 
void C_File(char **argv)
{
  static char name[500];
  char *action;
  char *filename,*filename1,*mode;
  STREAM stream;
  FILE *file;
  char *str,*expr,*dir,*path;
  int res,i,flag,size,type;
  LISTV lv;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* Open action */
  if (!strcmp(action,"open")) {
    argv = ParseArgv(argv,tSTR,&filename,tSTR,&mode,0);
    if (strcmp(mode,"w") && strcmp(mode,"r") && strcmp(mode,"a")) Errorf("Bad mode '%s'",mode); 
    stream = OpenFileStream(filename,mode);
    if (stream == NULL) Errorf("Cannot open file '%s'",filename);
    SetResultInt(stream->id);
  }

  /* remove action */
  else if (!strcmp(action,"remove")) {
    argv = ParseArgv(argv,tSTR,&filename,0);
    if (remove(ConvertFilename(filename)) != 0) Errorf("Could not find file %s",filename);
  }

  /* move action */
  else if (!strcmp(action,"move")) {
    argv = ParseArgv(argv,tSTR,&filename,tSTR,&filename1,0);
    str = TCopyStr(ConvertFilename(filename));
    if (rename(str,ConvertFilename(filename1)) != 0) Errorf("Could not find file %s",filename);
  }

  /* tmp action */
  else if (!strcmp(action,"tmp")) {
    NoMoreArgs(argv);
    SetResultStr(tmpnam(NULL));
  }
   
  /* Exist action */
  else if (!strcmp(action,"exist")) {
    argv = ParseArgv(argv,tSTR,&filename,tSTR_,"r",&mode,0);
    file = _FOpen(filename,mode);
    if (file == NULL) SetResultInt(0);
    else {
      SetResultInt(1);
      fclose(file);
    }
  }
    
  /* Openstr action */
  else if (!strcmp(action,"openstr")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    stream = OpenStringStream(str);
    SetResultInt(stream->id);
  }
  
  /* Close action */
  else if (!strcmp(action,"close")) {
    argv = ParseArgv(argv,tSTREAM,&stream,0);
    if (stream == _StdnullStream || stream == _StdoutStream || stream == _StdinStream || stream == _StderrStream) return;
    CloseStream(stream);
  }
 
  /* CreateDir action */
  else if (!strcmp(action,"createdir")) {
    argv = ParseArgv(argv,tSTR,&filename,tSTR,&filename1,0);
    if (XXCreateDirectory(filename,filename1) == NO) 
      Errorf("Unable to create directory named '%s' in directory '%s",filename1,filename);
  }
 
  /* Eof action */
  else if (!strcmp(action,"eof")) {
    argv = ParseArgv(argv,tSTREAM_,NULL,&stream,0);
    if (stream != NULL) res = FEof(stream);
    else res = Eof();
    SetResultInt(res);
  }

  /* set action */
  else if (!strcmp(action,"set")) { 
    argv = ParseArgv(argv,tWORD,&str,tSTREAM_,NULL,&stream,-1);
 
    if (stream == NULL && *argv != NULL) Errorf("Bad stream '%s'",*argv);
    
    NoMoreArgs(argv);
    
    /* Case we have to get ... */
    if (stream == NULL) {
      if (!strcmp(str,"stdin")) SetResultInt(stdinStream->id);
      else if (!strcmp(str,"stdout")) SetResultInt(stdoutStream->id);
      else if (!strcmp(str,"stderr")) SetResultInt(stderrStream->id);
      else Errorf("Bad argument '%s'",str);
    }

    /* Case we have to set ... */
    else if (levelFirst == levelCur) Errorf("Sorry cannot redirect '%s' out of a command definition",str);
    else {
      if (!strcmp(str,"stdin")) stdinStream = CopyStream(&stream,&levelCur->in);
      else if (!strcmp(str,"stdout")) stdoutStream = CopyStream(&stream,&levelCur->out);
      else if (!strcmp(str,"stderr")) stderrStream = CopyStream(&stream,&levelCur->err);
      else Errorf("Bad argument '%s'",str);
    }
  }

  /* list action */
  else if (!strcmp(action,"list") || !strcmp(action,"listp")) {

    argv = ParseArgv(argv,tSTR,&path,0);
    if (action[4] == '\0') flag = YES;
    else flag = NO;

    /* Getting the directory and the expr to match */
    for (i=strlen(path)-1;i>=0;i--) if (path[i] == '/') break;
    if (i == -1) {
      expr = TCopyStr(path);
      dir = TCopyStr(".");
      flag = YES;
    }
    else {
      path[i] = '\0';
      dir = TCopyStr(path);
      path[i] = '/';
      expr = TCopyStr(path+i+1);
    }
    /* Getting the corresponding filename */
    XXGetFilenames(dir);
    lv = TNewListv();
    while (str = XXGetFilenames(NULL)) {
      if (MatchStr(str,expr)) {
	    if (flag == YES) AppendStr2Listv(lv,str);
	    else {
	      strcpy(name,dir);
	      strcat(name,"/");
	      strcat(name,str);
          AppendStr2Listv(lv,name);
        }
      }
    }
    SetResultValue(lv);
    return;
  }

  /* info action */
  else if (!strcmp(action,"info")) {
    argv = ParseArgv(argv,tSTR,&path,0);
    size = 0;
    XXGetFilenameInfo(path,&type,&size);
    lv = TNewListv();
    switch(type) {
    case DirectoryFile: AppendStr2Listv(lv,"directory"); break;
    case RegularFile: AppendStr2Listv(lv,"file"); break;
    case UnknownTypeFile: AppendStr2Listv(lv,"unknown"); break;
    default: return;
    }
    AppendInt2Listv(lv,size);
    SetResultValue(lv);
  }

  /* cd action */
  else if (!strcmp(action,"cd")) {
    argv = ParseArgv(argv,tSTR,&path,0);
    XXChangeDirectory(path);
  }

  else Errorf("Unknown action '%s'",action); 
}


/***************************************************************
 *
 *   Functions for output 
 *
 ***************************************************************/

static char linePrinted[10000];

/*
 * Same as fprintf in C.
 * Instead of a FILE * the first argument is a streamId.
 */
void FPrintf(STREAM stream, char *format,...)
{
  va_list ap;
  
  /* Case the stream is null */
  if (stream == stdnullStream) return;


  va_start(ap,format);

  /* Case we are not in the main loop of the program yet */
  if (toplevelCur == NULL) {
    fprintf(stderr,format,ap);
    va_end(ap);
    return;
  }
  
  /* Get the current stream and check we can write on it */
  if (stream->mode != StreamWrite) Errorf("FPrintf() : Stream '%d' is not writable",stream->id);
  
  /* Case the stream corresponds to the terminal stdout */
  if (stream == _StdoutStream) {
    vsprintf(linePrinted,format,ap);
    XXTerminalPrintStr(linePrinted);
  }

  /* Case the stream corresponds to the terminal stderr */
  else if (stream == _StderrStream) {
    vsprintf(linePrinted,format,ap);
    XXTerminalPrintErrStr(linePrinted);
  }
  
  /* Case the stream corresponds to a file */
  else vfprintf(stream->stream,format,ap);

  va_end(ap);
}

/*
 * Perform an FPrintf of the current stdout
 */
void Printf(char *format,...)
{
  va_list ap;
  STREAM stream;
  int streamId;

  /* Case the stream is null */  
  if (stdoutStream == stdnullStream) return;

  va_start(ap,format);

  /* Case we are not in the main loop of the program yet */
  if (toplevelCur == NULL) {
    fprintf(stderr,format,ap);
    va_end(ap);
    return;
  }

  /* Get the current stream and check we can write on it */
  stream = toplevelCur->out; 
  streamId = stream->id;
  if (stream->mode != StreamWrite) Errorf("Printf() : Stream '%d' is not writable",streamId);

  /* Case the stream corresponds to the terminal stdout */  
  if (stream == _StdoutStream) {
    vsprintf(linePrinted,format,ap);
    InitTerminalInput();
    XXTerminalPrintStr(linePrinted);
  }

  /* Case the stream corresponds to the terminal stderr */
  else if (stream == _StderrStream) {
    vsprintf(linePrinted,format,ap);
    InitTerminalInput();
    XXTerminalPrintErrStr(linePrinted);
  }

  /* Case the stream corresponds to a file */
  else vfprintf(stream->stream,format,ap);

  va_end(ap);
}

/*
 * Perform an FPrintf of the current stdin
 */
void PrintfErr(char *format,...)
{
  va_list ap;
  STREAM stream;
  int streamId;

  /* Case the stream is null */  
  if (stderrStream == stdnullStream) return;
  
  va_start(ap,format);

  /* Case we are not in the main loop of the program yet */
  if (toplevelCur == NULL) {
    fprintf(stderr,format,ap);
    va_end(ap);
    return;
  }

  /* Get the current stream and check we can write on it */
  stream = toplevelCur->err; 
  streamId = stream->id;
  
  if (stream->mode != StreamWrite) Errorf("Printf() : Stream '%d' is not writable",streamId);
  
  /* Case the stream corresponds to the terminal stdout */  
  if (stream == _StdoutStream) {
    vsprintf(linePrinted,format,ap);
    InitTerminalInput();
    XXTerminalPrintStr(linePrinted);
  }

  /* Case the stream corresponds to the terminal stderr */
  else if (stream == _StderrStream) {
    vsprintf(linePrinted,format,ap);
    InitTerminalInput();
    XXTerminalPrintErrStr(linePrinted);
  }

  /* Case the stream corresponds to a file */
  else vfprintf(stream->stream,format,ap);

  va_end(ap);
}



/****************************************************
 * 
 * Commands for output
 *
 ****************************************************/

/*
 * Just print on stdin each of it's argument
 */ 
void C_Echo(char **argv)
{
  while(*argv != NULL) {
    if (*(argv+1) != NULL) Printf("%s ",*argv);
    else Printf("%s",*argv);
    argv++;
  }
  Printf("\n");
  Flush();
}


/*
 * The main function for the printf and sprintf commands
 *   (The line to be printed is in result)  ????? OPTIMISER 
 */


#define PrintfStrLength 2000
#define PrintfFormatLength 1000
  
void _CPrintf(char **argv, char *result)
{
  char *format,*str1;
  char f[PrintfFormatLength+1],*f1;
  char *result1;
  char argType;
  int ival;
  float fval;
  char *sval,c;
  VALUE val;
  
  argv = ParseArgv(argv,tSTR,&format,-1);

  /* Some inits */
  result1 = result;
  result[0] = '\0';  

  /* First we print whatever is before the first % sign */
  while (*format != '\0' && *format != '%') *(result1++) = *(format++);
  *result1 = '\0';
          
  /* Loop on the different arguments */
  while(1) {

    /* If format is over then go out of the loop */
    if (*format == '\0') break;

    /* If %% --> character is % */
    if (format[1] == '%') {
      *result1 = '%';
      result1++;
      *result1 = '\0';
      format += 2;
      while (*format != '\0' && *format != '%') *(result1++) = *(format++);
      *result1 = '\0';
      continue;
    }
    
    /* If % then get everything up to the next % */
    f[0] = '\0';
    f1 = f;
    *(f1++) = *(format++); /* Copy the % sign */
    while (*format != '\0' && *format != '%' && !isalpha(*format)) *(f1++) = *(format++);
    
    if (isalpha(*format)) {
      argType = *format;
      *(f1++) = *(format++);
      while (*format != '\0' && *format != '%') *(f1++) = *(format++);
      *f1 = '\0';
    }
    else Errorf("Bad format in 'printf'");
        
    
    /* We have to get the next argument variable */
    /* Case of an int */
    if (argType == 'd' || argType == 'i' || argType == 'o' ||
        argType == 'x' || argType == 'X' || argType == 'u') {
      argv = ParseArgv(argv,tINT,&ival,-1);
      sprintf(result1,f,ival);
    }

    /* Case of an double */
    else if (argType == 'f' || argType == 'e' || argType == 'E' ||
        argType == 'g' || argType == 'G') {
      argv = ParseArgv(argv,tFLOAT,&fval,-1);
      sprintf(result1,f,fval);
    }

    /* Case of a string */
    else if (argType == 's') {
      argv = ParseArgv(argv,tSTR,&sval,-1);
      sprintf(result1,f,sval);
    }

    /* Case of a char */
    else if (argType == 'c') {
      argv = ParseArgv(argv,tCHAR,&c,-1);
      sprintf(result1,f,c);
    }

    /* Case of a VAL (LONG) */
    else if (argType == 'V') {
      argv = ParseArgv(argv,tVAL,&val,-1);
      str1 = ToStrValue(val,NO);
      sprintf(result1,"%s",str1);
      sprintf(result1+strlen(str1),f+2);
    }

    /* Case of a VAL (SHORT) */
    else if (argType == 'v') {
      argv = ParseArgv(argv,tVAL,&val,-1);
      str1 = ToStrValue(val,YES);
      sprintf(result1,"%s",str1);
      sprintf(result1+strlen(str1),f+2);
    }
         
    /* Case unknown */
    else Errorf("_CPrintf() : Format '%c' unknown",argType);
    
    result1 = result + strlen(result);
  }  
    
  NoMoreArgs(argv);
  
  return;
}

/*
 * The printf command
 */
void C_Printf(char **argv)
{
  char result[PrintfStrLength+1];
  
  _CPrintf(argv,result);

  Printf("%s",result);
  Flush();

  InitResult();
}

/*
 * Same as printf but on stderr and then generate an error
 */ 
void C_Errorf(char **argv)
{
  char result[PrintfStrLength+1];
  
  _CPrintf(argv,result);

  SetErrorf("%s",result);
  InitResult();

  Errorf1("");
}

/*
 * The sprintf command
 */  
void C_SPrintf(char **argv)
{
  char result[PrintfStrLength+1];
  char *var;
  
  argv = ParseArgv(argv,tVNAME,&var,-1);
  
  _CPrintf(argv,result);

  SetStrVariable(var,result);
  SetResultStr(result);
}



/***************************************************************
 *
 *   Functions for terminal line edition
 *
 ***************************************************************/
 
/*
 * Inserting a string in the terminal line at the cursor position.
 * If there is a new line then we print the new line at the end of the terminal line and stop.
 *
 *    WARNING : the string might change !
 *
 */
static void TermInsertStr(char *str)
{
  int flagNewLine;
  int l,i,ls;
  char *str1,*str2;

  flagNewLine = NO;
  
  /* Looking for the end of 'str' or for the first new line */
  str1 = str;
  while (*str1 != '\0' && *str1 != '\n' && *str1 != '\r') str1++;
  if (*str1 != '\0') {
    flagNewLine = YES;
    *str1 = '\0';
  }
  ls = strlen(str);

  l = strlen(toplevelCur->termLine);

  /* Too many characters already ! */
  if (l + ls + flagNewLine> MaxLengthTermLine) {Beep();return;}
  
  /* Case the 'str' is not empty or just equal to "\n" */
  if (ls != 0) {
  
    /* In case the cursor is at the end of the line we just need to append... */
    if (toplevelCur->termLine[toplevelCur->termCursor] == '\0') {
      str1 = str;
      while (*str1 != '\0') {
        XXTerminalInsertChar(*str1,"");
        str1++;
      }
    }
  
    /* Otherwise we insert it */
    else {
      for (i = l+ls; i > toplevelCur->termCursor+ls-1;i--)  toplevelCur->termLine[i] = toplevelCur->termLine[i-ls]; 
      str1 = str;
      while (*(str1+1) != '\0') {
        XXTerminalInsertChar(*str1,"");
        str1++;
      }
      XXTerminalInsertChar(*str1,toplevelCur->termLine+i+ls);
    }

    /* We copy the string and update the cursor */
    str1 = str;
    str2 = toplevelCur->termLine + toplevelCur->termCursor;
    while (*str1 != '\0') {
      *str2 = *str1;
      str2++;
      str1++;
    }
    toplevelCur->termCursor += ls;
    toplevelCur->termLine[l+ls] = '\0';
  }
  
  /* Manage the new line if any */
  if (flagNewLine) {
    if (toplevelCur->termCursor != l)  XXTerminalCursorGoForward(l-toplevelCur->termCursor);
    toplevelCur->termCursor = l;
    toplevelCur->termLine[l+ls] = '\n';
    toplevelCur->termLine[l+ls+1] = '\0';
    XXTerminalPrintChar('\n');
  }
    
  /* And flush ! */
  XXTerminalFlush();
}  


/*
 * Inserting a character in the terminal line at the cursor position
 */
static void TermInsertChar(char c)
{
  char str[2];
  
  str[0] = c;
  str[1] = '\0';
  
  TermInsertStr(str);
}

/*
 * Delete n characters just before the cursor 
 */
static void TermDeleteNChars(int n,char flagBeep) 
{
  int l,i;
  
  if (n <= 0) return;
  
  l = strlen(toplevelCur->termLine);
  
  if (toplevelCur->termCursor < n) {
    if (flagBeep) {Beep(); return;}
    else n = toplevelCur->termCursor;
  }

  /* In case the cursor is at the end of the line we just need to erase the last N chars */
  if (toplevelCur->termLine[toplevelCur->termCursor] == '\0') {
    toplevelCur->termLine[l-n] = '\0';
    XXTerminalDeleteInsertChar(n,"");
  }
  
  /* Otherwise we must shift left what's on the right of the cursor */
  else {
    for (i = toplevelCur->termCursor-n; i+n <= l;i++) toplevelCur->termLine[i] = toplevelCur->termLine[i+n]; 
    XXTerminalDeleteInsertChar(n,toplevelCur->termLine+toplevelCur->termCursor-n);
  }
  
  toplevelCur->termCursor -= n;
  XXTerminalFlush();
}


/*
 * Move the cursor
 */
static void TermMoveCursor(int n,char flagBeep)
{
  if (n == 0) return;
  
  /* Backward */
  if (n < 0) {
    if (-n > toplevelCur->termCursor) {
      if (flagBeep) {Beep(); return;}
      n =  -toplevelCur->termCursor;
    }
    if (n != 0) {
      toplevelCur->termCursor += n;
      XXTerminalCursorGoBackward(-n);
    }
  }

  /* Forward */  
  else {
    if (n > strlen(toplevelCur->termLine) - toplevelCur->termCursor) {
      if (flagBeep) {Beep(); return;}
      n =  strlen(toplevelCur->termLine) - toplevelCur->termCursor;
    }
    if (n != 0) {
      toplevelCur->termCursor += n;
      XXTerminalCursorGoForward(n);
    }
  }
}


/***************************************************************
 *
 *   The main command for terminal line edition
 *
 ***************************************************************/

void C_Terminal(char **argv)
{
  char *action,*str;
  int n;
  int x,y;
  PROC proc;
  char *c;
    
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  
  /* Erase line action */
  if (!strcmp(action,"eraseline")) {
    NoMoreArgs(argv);
    TermMoveCursor(MaxLengthTermLine,NO);
    TermDeleteNChars(MaxLengthTermLine,NO);
  }
  
  /* Erase chars action */
  else if (!strcmp(action,"erasechars")) {
    argv = ParseArgv(argv,tINT_,1,&n,0);
    TermDeleteNChars(n,YES);
  }
  
  /* movecursor action */
  else if (!strcmp(action,"movecursor")) {
    argv = ParseArgv(argv,tINT,&n,0);
    TermMoveCursor(n,NO);
  }

  /* cursor action */
  else if (!strcmp(action,"cursor")) {
    if (*argv == NULL) SetResultInt(toplevelCur->termCursor);
    else {
      argv = ParseArgv(argv,tINT,&n,0);
      TermMoveCursor(n-toplevelCur->termCursor,NO);
    }
  }
  
  /* prompt action */
  else if (!strcmp(action,"prompt")) {
    if (*argv == NULL) {
      GetPrompt(&str);
      SetResultStr(str);
    }
    else {
      argv = ParseArgv(argv,tPROC,&proc,0);
      if (proc->flagSP == NO) Errorf("Sorry, the prompt procedure must be a script procedure");
      if (toplevelCur->promptProc != NULL) DeleteProc(toplevelCur->promptProc);
      toplevelCur->promptProc = proc;
      proc->nRef++;
    }
  }
   
  /* beep action */
  else if (!strcmp(action,"beep")) {
    NoMoreArgs(argv);
    Beep();
  }
  
  /* Insert action */
  else if (!strcmp(action,"insert")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    while (*str != '\0') {
      TermBufferPushKey(*str);
      str++;
    }
  }

  /* line action */
  else if (!strcmp(action,"line")) {
    argv = ParseArgv(argv,tSTR_,NULL,&str,0);
    if (str == NULL) SetResultStr(toplevelCur->termLine);
    else {
      TermMoveCursor(MaxLengthTermLine,NO);
      TermDeleteNChars(MaxLengthTermLine,NO);
      /* ????? */
      if ((c = strchr(str,'\n')) || (c = strchr(str,'\r'))) *c = '\0';
      TermInsertStr(str);
/*      while (*str != '\0') {
        TermBufferPushKey(*str);
        str++;
      } */
    }
  }
  
  /* eof action */
  else if (!strcmp(action,"eof")) {
    argv = ParseArgv(argv,0);
    TermBufferPushKey(EofKC);
  }

  /* state action */
  else if (!strcmp(action,"mode")) {
    NoMoreArgs(argv);
    if (toplevelCur->termMode == ScanTMode) SetResultStr("scanline"); 
    else if (toplevelCur->termMode == GetcharTMode) SetResultStr("getchar"); 
    else if (toplevelCur->termMode == CommandTMode) SetResultStr("command"); 
    else SetResultStr("unknown"); 
  }

  /* movewindow action */
  else if (!strcmp(action,"movewindow")) {
    argv = ParseArgv(argv,tINT,&x,tINT,&y,0);
    if (x <= 0 || y <=0) Errorf("No negative values allowed for window position");
    XXTerminalMoveWindow(x,y);
  }

  /* resizewindow action */
  else if (!strcmp(action,"resizewindow")) {
    argv = ParseArgv(argv,tINT,&x,tINT,&y,0);
    if (x <= 0 || y <=0) Errorf("No negative values allowed for window size");
    XXTerminalResizeWindow(x,y);
  }

  /* change the way the result will be printed */
  else if (!strcmp(action,"result")) {
    argv = ParseArgv(argv,tSTR,&str);
    if (!strcmp(str,"normal")) toplevelCur->termResultMode = TermResultNormalMode;      
    else if (!strcmp(str,"hack")) toplevelCur->termResultMode = TermResultHackMode;      
    else Errorf("Bad mode '%s'",str);
  }

  /* Unknown action */
  else Errorf("Unknown action '%s'",action);
    
}

/***************************************************************
 *
 *   Basic functions for terminal key event management
 *
 ***************************************************************/

/*
 * Initialization of the current terminal line 
 */
void InitTerminalInput(void)
{
  toplevelCur->termLine[0] = '\0';
  toplevelCur->termCursor = 0;
}  


/*
 * Push a key code in the terminal buffer (called by ProcessNextEvent)
 */
void TermBufferPushKey(unsigned long c)
{
  PushBuffer(_StdinStream->buffer,c);
}

/*
 * Wait for the next terminal keyPress and returns the corresponding key code
 */
static unsigned long WaitForTerminalKeyPress(void)
{
  unsigned long key;
  
  while((key = PullBuffer(_StdinStream->buffer)) == 0) ProcessNextEvent(YES);

  return(key);
}
   
/*
 * Pull terminal line characters up to 'pos' (excluded) and copy them in out (if out != NULL)
 */
static void PullTermLine(char *pos,char *out)
{
  char *line = toplevelCur->termLine;
  char *line1;
  
  if (pos == line) {
    if (out == NULL) return;
    out[0] = '\0';
  }

  if (out != NULL)  {
    line1 = line;
    while(line1 != pos) *(out++) = *(line1++);
    *out = '\0';
  }
  while (*pos != '\0') *(line++) = *(pos++);
  *line = '\0';
  toplevelCur->termCursor = line-toplevelCur->termLine;
}      

/*
 * Function to call for allowing the user to write a whole line on the terminal
 * Basically it loops until a newline or an eof is typed
 */
 
static void DoLineTerm(void)
{
  unsigned long c; 
  char *line,*str;

  if (toplevelCur->oldTermMode  != UnknownTMode) 
    Errorf("DoLineTerm() : cannot wait for two terminal inputs at the same time");
  
  /* We first check whether there is not a new line or an eof in the queue already */
  for (line = toplevelCur->termLine;*line != '\0';  line++)
    if (*line == NewlineKC /*|| *line == EofKC */) {
      return;
    }
  
  /* Otherwise we have to wait */  
  while (1) {
  
    /* Get the next char and echo it */
    c = WaitForTerminalKeyPress();
    
    if (c == DeleteKC) TermDeleteNChars(1,YES);
    else if (c != EofKC) {        
      str = KeyCode2Str(c,YES);
      TermInsertStr(str);
    }
      
    /* Stops if EOF or \n or \r */
    if (c == EofKC || c == NewlineKC) break;
  }  
}
  

/**************************************************************
 * 
 * Functions and commands for getting a key from any stream 
 *
 *************************************************************/

/*
 * Getting a char from any stream and returns it (or returns EOF)
 */
long FGetChar(STREAM stream)
{
  unsigned long c;
  
  if (stream->mode != StreamRead) Errorf("FGetChar() : stream '%d' not readable",stream->id);
  
  /* Case the stdin is the terminal */
  if (stream == _StdinStream) {
    toplevelCur->oldTermMode = toplevelCur->termMode;
    toplevelCur->termMode = GetcharTMode;
    c = WaitForTerminalKeyPress();
    toplevelCur->termMode = toplevelCur->oldTermMode;
    if (c == EofKC) c = EOF;
  }
  
  /* Case the stdin is a string stream */
  else if (stream->buffer != NULL) {
    c =  PullBuffer(stream->buffer); 
    if (c == EofKC) c = EOF;
  }
  
  /* Case the stdin is a file */
  else {
    c = fgetc(stream->stream);
  }
  
  return(c);
}

/*
 * Getting a char from the stdin
 */
long GetChar(void)
{
  return(FGetChar(stdinStream));
}

/*
 * Command for getting a char 
 */
void C_GetChar(char **argv)
{
  unsigned long c;
  char str[2];
  char *var;
     
  argv = ParseArgv(argv,tVNAME_,NULL,&var,0);
  
  c = GetChar();

  SetResultStr(KeyCode2Str(c,NO));
  
  str[0] = c;
  str[1] = '\0';
  
  if (var != NULL) SetStrVariable(var,str);
}


/****************************************************
 * 
 * Functions and commands for getting a whole line
 *
 ****************************************************/
    
/*
 * Main function for getting a line from any stream 
 * The terminal mode has been set previously and will
 * be restored before before this function returns.
 * This function returns 1 or EOF
 */
static int _FGetLine(STREAM stream, char *str)
{
  long l;
  char *str1,*line,c;
  
  if (stream->mode != StreamRead) Errorf("FGetLine() : stream '%d' not readable",stream->id);

  /* Init the string */  
  *str = '\0';
  
  /*
   * We separate the 3 cases of streams
   */
  
  /* Case the input is the terminal */
  if (stream == _StdinStream) {

    DoLineTerm();

    toplevelCur->termMode = toplevelCur->oldTermMode;
        
    /* Looking for the new line or the end of file */
    for (line = toplevelCur->termLine;*line != '\n' && *line != '\r' && *line != '\0' ; line++);
    
    /* Case there is just an end of file */
    if (*line == '\0' && line == toplevelCur->termLine) return(EOF);
    
    /* Skip the eventual \n */
    c = *line;
    if (c != '\0')  line++;
    
    /* Pull one line from the current terminal line */
    PullTermLine(line,str);
    
    /* Take out the eventual '\n' */
    if (c != '\0') str[strlen(str)-1] = '\0';
      
    return(1);
  }
  
  /* Case it is a file stream */
  if (stream->buffer == NULL) {
    l = fscanf(stream->stream,"%[^\n\r]",str);  
    if (feof(stream->stream) && l == EOF) return(EOF);
    else {
     fgetc(stream->stream);
     return(1);
    }
  }
  
  /* Case it is a string stream */
  str1 = str;
  while(l = PullBuffer(stream->buffer)) {
    if (l == '\n' || l == '\r' || l == EofKC) break;   
    *(str++) = l;
  }
  *str = '\0';
  if (str1 != str) return(1);
  if (l == EofKC) return(EOF);
  return(1);
}

/*
 * Getting a line from any stream
 *   returns 1 or EOF
 */
int FGetLine(STREAM stream, char *str)
{

  /* We set the terminal mode (if the input is the terminal) and just call _FGetLine */
  
  if (stream == _StdinStream) {
    toplevelCur->oldTermMode = toplevelCur->termMode;
    toplevelCur->termMode = ScanTMode;
  }
  
  return(_FGetLine(stream,str));    
}

/*
 * Getting a command from any stream
 *   returns 1 or EOF
 * The only difference with the above function is the terminal mode.
 */
int FGetCommandLine(STREAM stream, char *str)
{

  /* We set the terminal mode (if the input is the terminal) and just call _FGetLine */
  
  if (stream == _StdinStream) {
    toplevelCur->oldTermMode = toplevelCur->termMode;
    toplevelCur->termMode = CommandTMode;
  }
  
  return(_FGetLine(stream,str));    
}

/*
 * Getting a line from stdin 
 */
int GetLine(char *str)
{
  return(FGetLine(stdinStream,str));
}

/*
 * Getting a command from stdin 
 */
int GetCommandLine(char *str)
{
  return(FGetCommandLine(stdinStream,str));
}


/*
 * Command for Getting a line from stdin
 */
void C_GetLine(char **argv)
{
  char str[2000];
  int ans;
  char *var;
     
  argv = ParseArgv(argv,tVNAME_,NULL,&var,0);
  
  ans = GetLine(str);

  if (var != NULL) {
    SetStrVariable(var,str);
    if (ans != EOF) SetResultInt(ans);
    else SetResultStr("eof");
  }
  else SetResultStr(str);
}


/****************************************************
 * 
 * Commands for input
 *
 ****************************************************/

/*
 * The main function for the scanf and sscanf commands
 *   string : a pointer to the string to analyze or NULL
 *   stream : a pointer to the file stream to analyze or NULL
 *   (only one of the two values above must be not NULL)
 *   nArgs  : a pointer to what will be the number of arguments read
 *   argv   : the list of all the variable's names.
 *
 *   returns NO, YES or EOF
 */

static int _CScanf(char **string,FILE * stream,int *nArgs, char **argv)
{
  int n,res;
  int nChars;
  char *format,*theFormat;
  char format1[1000];
  char *str;
  char argType,flagSetVariable;
  int ival;
  float fval;
  char sval[1000],*var;  

  /* Some checks */
  if (stream != NULL && string != NULL) Errorf("_CScan() : cannot pass both a stream and a file");
  if (stream == NULL && string == NULL) Errorf("_CScan() : you must pass either a stream or a file");
  
  /* Then parse the format */
  argv = ParseArgv(argv,tSTR,&format,-1);

  if (strlen(format) > 990) Errorf("_CScanf() : format is too long");
    
  /* Make some initialization */
  *nArgs = 0;
  nChars = 0;
  theFormat = format;
        
  /* Loop on the string or the stream */
  while(1) {

    /* If we reached the end of the format  --> return(YES) */
    if (*format == '\0') return(YES);

    /*
     * STRING : First we match whatever is before the first % sign 
     */    
    if (string != NULL) {

      /* First we read whatever is before the % sign */
      while (*format != '\0' && *format != '%') {

        /* Case *format == ' ' || '\n' ==> we read as many blanks as possible in the string */
        if (*format == ' ' || *format == '\n') { 
          while (**string == ' ') {*string += 1;nChars++;}  /* Spaces before the '\n' */
          if (*format == '\n') {                            /* The '\n' */
            if (**string != '\n') return(NO);               
            *string+=1;
            nChars++;
          }
          format++;
          continue;
        }        
        /* Otherwise *format has to match */
        if (*format != **string) return(NO);
        *string+=1;
        nChars++;
        format++;
      }
      
      /* If we reached the end of the string before the end of the format --> return EOF (unless format == %n) */
      if (**string == '\0' && *format != '\0' && strncmp(format,"%n",2)) return(EOF);    
      
      /* If *format did not match --> return NO */
      if (*format != '%' && *format != **string) return(NO);
      
      /* If we read all the format --> return YES */
      if (*format == '\0') return(YES);
      
      /* Case we stopped at a %% sign */
      if (format[1] == '%') {
        if (**string != '%') return(NO);
        *string+=1;
        nChars++;
        *format += 2;
        continue;
      }
    }

    /*
     * STREAM : First we match whatever is before the first % sign 
     */
    else {
    
      /* We read in 'format1' everything before the first %  (which is not a %%) */
      str = format1;
      while (*format != '\0' && (*format != '%' || *(format+1) == '%')) *(str++) = *(format++);
    
      /* If we did read something  then we fscanf (after adding %n) */
      if (str != format1) {
        *str = '\0';
        strcat(format1,"%n");
        n = -1;
        /* During this fscanf, it reads the 'format' AND all the spaces that follow */
        res = fscanf(stream,format1,&n);
        if (n != -1) nChars += n;
        if (res == EOF) return(EOF);
        continue;
       
      }
    }     
        
        
    /*
     * So now 'format' is pointing to the '%' argument --> so let's get the format in 'format1' !
     */    
    str = format1;
    if (format[1] == '\0') Errorf("'%%' without a type in scanf format : '%s'",theFormat);
    *(str++) = *(format++);
    while (*format != '\0' && *format != '%' && !isalpha(*format) && *format != '[') *(str++) = *(format++);
    
    /* if we found a '[' then we just have to look for the matching ']' */
    if (*format == '[') {
      while (*format != '\0' && *format != ']') *(str++) = *(format++);
      if (*format != ']') Errorf("Missing ']' in scanf format : '%s'",theFormat);
    }
    
    /* read 1 more char if needed */
    if (isalpha(*format) || *format == ']') {
      argType = *format;
      *(str++) = *(format++);
    }
    else Errorf("Bad format in 'scanf' : '%s'",theFormat);
     
    /* End 'format1' ad concatenates a %n */
    *str = '\0';
    strcat(str,"%n");
    
    /* Is it a %* type of format ? */
    if (format1[1] == '*') flagSetVariable = NO;
    else flagSetVariable = YES; 
    
    /*
     * So let's read the next argument corresponding to 'format1'
     */

    /* Case of an int */
    if (argType == 'd' || argType == 'i' || argType == 'o' ||
        argType == 'x') {
      n = -1;
      if (string != NULL) res = sscanf(*string,format1,&ival,&n);
      else res = fscanf(stream,format1,&ival,&n);
      if (n != -1) {
        nChars += n;
        if (string != NULL) *string+=n;
      }
      if (res == EOF) return(EOF);
      if (res == 0) return(NO);
      if (flagSetVariable) {
        argv = ParseArgv(argv,tVNAME,&var,-1);
        SetNumVariable(var,ival);
        *nArgs += 1;
      }
    }

    /* Case of a float */
    else if (argType == 'e' || argType == 'f' || argType == 'g') {
      n = -1;
      if (string != NULL) res = sscanf(*string,format1,&fval,&n);
      else res = fscanf(stream,format1,&fval,&n);
      if (n != -1) {
        nChars += n;
        if (string != NULL) *string+=n;
      }
      if (res == EOF) return(EOF);
      if (res == 0) return(NO);
      if (flagSetVariable) {
        argv = ParseArgv(argv,tVNAME,&var,-1);
        SetNumVariable(var,fval);
        *nArgs += 1;
      }
    }

    /* Case of a string */
    else if (argType == 's' || argType == ']') {
      n = -1;
      if (string != NULL) res = sscanf(*string,format1,sval,&n);
      else res = fscanf(stream,format1,sval,&n);
      if (n != -1) {
        nChars += n;
        if (string != NULL) *string+=n;
      }
      if (res == EOF) return(EOF);
      if (res == 0) return(NO);
      if (flagSetVariable) {
        argv = ParseArgv(argv,tVNAME,&var,-1);
        SetStrVariable(var,sval);
        *nArgs += 1;
      }
    }

    /* Case of a char */
    else if (argType == 'c') {
      n = -1;
      if (string != NULL) res = sscanf(*string,"%c%n",sval,&n);
      else res = fscanf(stream,"%c%n",sval,&n);
      if (n != -1) {
        nChars += n;
        if (string != NULL) *string+=n;
      }
      if (res == EOF) return(EOF);
      if (res == 0) return(NO);
      sval[1] = '\0';
      if (flagSetVariable) {
        argv = ParseArgv(argv,tVNAME,&var,-1);
        SetStrVariable(var,sval);
        *nArgs += 1;
      }
    }
    
    /* Case of a %n */
    else if (argType == 'n') {
      argv = ParseArgv(argv,tVNAME,&var,-1);
      SetNumVariable(var,nChars);
    }
    

    /* Case unknown */
    else Errorf("_CScanf() : Format '%c' unknown in '%s'",argType,theFormat);
  }  
  
  return(YES);
}


/* 
 * The 'scanf' command
 */
void C_Scanf(char **argv)
{
  STREAM stream;
  char str[1000],*str1,*str2;
  int nArgs;
  int result;

  /* Get the current stdin */  
  stream = toplevelCur->in;

  /*
   * Case of the terminal (wait for \n or an eof) 
   */
  if (stdinStream == _StdinStream) {
  
  
    toplevelCur->oldTermMode = toplevelCur->termMode;
    toplevelCur->termMode = ScanTMode;
    DoLineTerm();
    toplevelCur->termMode = toplevelCur->oldTermMode; 
       
    str1 = toplevelCur->termLine;
    _CScanf(&str1,NULL,&nArgs,argv);
    result = nArgs;
    PullTermLine(str1,NULL);    
  }

  /* Case of a string buffer */
  else if (stream->buffer != NULL) {
    Buffer2Str(stream->buffer,str);
    str1 = str;
    _CScanf(&str1,NULL,&nArgs,argv);
    result = nArgs;
    str2 = str;
    while(str2 != str1) {
      PullBuffer(stream->buffer);
      str2++;
    }
  }
  
  /* Case of a stream */
  else {
    _CScanf(NULL,stream->stream,&nArgs,argv);
    result = nArgs;
  }
  
  SetResultInt(result);
}

/* 
 * The 'sscanf' command
 */
void C_SScanf(char **argv)
{
  char *str1;
  char *theString;
  int nArgs;
  int result;


  argv = ParseArgv(argv,tSTR,&theString,-1);

  str1 = theString;
  result = _CScanf(&str1,NULL,&nArgs,argv);

  SetResultInt(nArgs);
}  
 
   



