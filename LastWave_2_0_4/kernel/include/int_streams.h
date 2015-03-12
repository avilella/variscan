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
/*  streams.h       This file contains the stream definitions               */
/*                                                                          */
/****************************************************************************/


#ifndef STREAMS_H

#define STREAMS_H

/*
 * The buffer structure
 */

typedef struct buffer_ {
   long *buffer;
   long *start,*end;
   long size;
   char flagEof;    /* Set if an end of file character has been pulled (then the buffer will freeze) */
} Buffer, *BUFFER;

extern char PushBuffer(BUFFER buf,long c);
extern long PullBuffer(BUFFER buf);
extern BUFFER NewBuffer(int size);
extern void DeleteBuffer(BUFFER);
extern long AskBuffer(BUFFER b);
extern char AskBufferNewLine(BUFFER b);


/*
 * The different stream modes 
 */
enum {
  StreamRead = 0,
  StreamWrite
};

/*
 * the structure of a stream
 */
typedef struct stream_{

   int id;                   /* The id in the toplevelCur->theStreams array */
   unsigned short refCount;  /* The number of reference of this stream */

   char mode;                /* The mode could be Read or Write */
   
   FILE *stream;             /* the FILE * corresponding to a file */
   
   BUFFER buffer;            /* Case the stream is of type "r" and is associated to a buffer */
   
} Stream, *STREAM;



/*
 * Some extern variables
 */
 
/*
 * The id of the standard streams and the corresponding streams 
 */

extern STREAM _StdinStream,_StdoutStream,_StderrStream,_StdnullStream;

/*
 * The id of the eventually redirectionned standard streams 
 */

extern STREAM stdinStream,stdoutStream,stderrStream,stdnullStream;


/*
 * Some external functions on streams
 */

extern char *ConvertFilename(char *filename);
extern STREAM OpenFileStream(char *filename,char *mode);
extern STREAM OpenBufferStream(int size);
extern STREAM OpenStringStream(char *str);
extern void CloseStream(STREAM stream);
extern STREAM RefStream(STREAM in);
extern STREAM CopyStream(STREAM *streamIn,STREAM *streamOut);


extern char FEof(STREAM stream);
extern char Eof(void);

extern void FPrintf(STREAM stream, char *format,...);
extern void Printf(char *format,...);
extern void PrintfErr(char *format,...);
extern long FGetChar(STREAM stream);
extern long GetChar(void);
extern int FGetLine(STREAM stream, char *str);
extern int GetLine(char *str);
extern int FGetCommandLine(STREAM stream, char *str);
extern int GetCommandLine(char *str);

extern void InitTerminalInput(void);

/*
 * Some external functions on the terminal
 */

extern void TermBufferPushKey(unsigned long);

#endif
