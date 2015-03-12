
/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   M a c i n t o s h   2 . 0                         */
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


#include "computer.h"

#ifdef LASTWAVE_MAC_GRAPHICS

#include "lastwave.h"

#include <Controls.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>

#include "MacTextWindow.h"

#include <console.h>
#include <signal.h>
#include <Traps.h>



/***********************
 *
 * Some basic macros on the output buffer of a text window
 *
 ***********************
 
 
/* Set the buffer to 0 */ 
#define ZEROBUFFER(buffer) { \
   buffer->curPos =	buffer->endPos = buffer->startPos;	\
   buffer->textHandlePos = -1; \
}

/* Get the current size of the buffer */
#define CURRENTBUFSIZE(buffer) (buffer->endPos - buffer->startPos)

/* Flush the buffer if adding 'c' characters to it leads to an overflow */
#define CHECKFOROVERFLOW(window,c) { \
  if (CURRENTBUFSIZE(window->buffer) + (c) >= window->buffer->size)	FlushTWBuffer(window);\
}

/* Delete 'num' chars from the current position of the buffer */
#define DELETEFROMBUFFER(buffer,num) {\
  if (buffer->curPos != buffer->endPos)	\
    BlockMoveData(buffer->curPos,buffer->curPos - (num),buffer->endPos - buffer->curPos);\
  buffer->endPos -= (num); \
  buffer->curPos -= (num); \
}

/* In case of a '\n' we must 'roll back' the 'num' characters that are after the current position */
#define ROLLBACKBUFFER(buffer,num) buffer->curPos = buffer->endPos - (num)

/* Insert a 'change line' character at the end of the line */
#define INSERTLINEFEED(buffer){	\
  *(buffer->endPos) = 0x0d;\
  buffer->endPos++;\
  buffer->curPos = buffer->endPos;\
  buffer->textHandlePos = -1;\
}

/* Insert a character 'c' */
#define INSERTCHAR(window,c) {\
  TEXTWINDOWBUFFER theBuffer = (window)->buffer; \
  if (theBuffer->textHandlePos == -1) {\
	*(theBuffer->curPos) = (c);\
	if (theBuffer->curPos == theBuffer->endPos) theBuffer->endPos++;	\
	theBuffer->curPos++; \
   } else { \
    TEHandle theTEH = window->textEdit;\
    TESetSelect(theBuffer->textHandlePos,theBuffer->textHandlePos + 1,theTEH);\
    TEKey(c, theTEH);\
    theBuffer->textHandlePos++;\
    if (theBuffer->textHandlePos == (*theTEH)->teLength - 1)	\
	  theBuffer->textHandlePos = -1;\
  }\
 }



/************
 *
 *   (Des)Allocation functions of text window buffers
 *
 *************/
 
TEXTWINDOWBUFFER XXNewTextWindowBuffer(short size) 
{
  TEXTWINDOWBUFFER buffer;
  
  if ((buffer = (TEXTWINDOWBUFFER) NewPtr(sizeof(struct TextWindowBuffer))) == NULL) return(NULL);  

  buffer->size = size;
  if ((buffer->startPos = (char *)NewPtr(size)) == NULL) return(NULL);
  ZEROBUFFER(buffer);

  return(buffer);
}


void XXDeleteTextWindowBuffer(TEXTWINDOWBUFFER buffer)
{
  DisposePtr(buffer->startPos);
  DisposePtr((char *)buffer);
}


/*************
 *
 *   Some Basic functions for IO on text windows
 *
 *************/
 
/*
 * Computes the offset in the text handle of the current line
 */
 
static long OffsetOnCurrentLine(TEXTWINDOW window)
{
    TEXTWINDOWBUFFER buffer = window->buffer;
    TEHandle theTEH = window->textEdit;
	char *ptr, *start;
	long result;

	/*	Check for a CR in the buffer ... */
	if (buffer->endPos != buffer->startPos) {
	  for (start = buffer->startPos, ptr = buffer->endPos; ptr > start; ptr--)
	     if (*ptr == 0x0d) return (buffer->endPos - ptr);
	}

	HLock((Handle)theTEH);
	HLock((*theTEH)->hText);

	start = *(*theTEH)->hText;
	ptr = *(*theTEH)->hText + (*theTEH)->selStart;
	while (ptr > start && ptr[-1] != 0x0d)
		ptr--;

	result = *(*theTEH)->hText + (*theTEH)->selStart - ptr + buffer->endPos - buffer->startPos;

	HUnlock((*theTEH)->hText);
	HUnlock((Handle)theTEH);

	return result;
}



/*
 * Flush a text window buffer 
 */
 
static void FlushTWBuffer(TEXTWINDOW window)
{
    TEXTWINDOWBUFFER buffer = window->buffer;
	TEHandle theTEH = window->textEdit;
	short teLength;
	

	HLock((Handle)theTEH);
	teLength = (*theTEH)->teLength;

	if ((teLength + CURRENTBUFSIZE(buffer)) > 32767) {
  	    /*	Insert will grow TEHandle past 32K so we cut out excess from top ... */
		char *ptr;
		short todelete = (short) ((teLength + CURRENTBUFSIZE(buffer)) - 32767) + 8*buffer->size;
		
		/*	Make sure that the text to be cut ends on a CR ... */
		HLock((*theTEH)->hText);
		for (ptr = *(*theTEH)->hText + todelete; *ptr != 0x0d; ptr++) ;

		/*	We now point at the CR, increment ptr to point after it ... */
		todelete += ++ptr - (*(*theTEH)->hText + todelete);
		HUnlock((*theTEH)->hText);
		
		/*	We hit the fields directly to keep TE from redrawing twice */
		(*theTEH)->selStart = 0;
		(*theTEH)->selEnd   = todelete;
		TEDelete(theTEH);

		/*	Now fix things up... */
		teLength = (*theTEH)->teLength;
	}

	TESetSelect(teLength, teLength, theTEH);
	
	/*	Now insert the new text ... */
	TEInsert(buffer->startPos, CURRENTBUFSIZE(buffer), theTEH);
	teLength = (*theTEH)->teLength;
	ZEROBUFFER(buffer);

	TESetSelect(teLength, teLength, theTEH);

	HUnlock((Handle)theTEH);
	
	window->scrollbarDirty = true;
    window->selStart = (*window->textEdit)->selStart;
}

 
/*
 * Write a char to the current text window 
 *
 * ** WARNING : This function is directly called by the ANSI library when writing to stdout or stderr
 */
  
long WriteCharsToConsole(char *buffer, long n)
{
  long counter;
  char aChar;
  GrafPtr saveport;
  TEXTWINDOW window = XXTerminalWindow;
    
  GetPort(&saveport);
  SetPort((WindowPtr)window);

  window->state = PRINTFING;

  for(counter = n; counter > 0; counter--) {

    aChar = *buffer++;
	switch(aChar) {
	  case 0x0d:	/*	Line Feed (Mac newline) */
  	    INSERTLINEFEED(window->buffer);
	    break;
	  case 0x0a:	/*	Carriage Return (move to start of line) */
	  /*  i = OffsetOnCurrentLine(window);
		if (i <= CURRENTBUFSIZE(window->buffer)) ROLLBACKBUFFER(window->buffer,i);
		else  window->buffer->textHandlePos = 
		      (*window->textEdit)->teLength - (i - CURRENTBUFSIZE(window->buffer));*/
		break;
 	  case '\f':	/*	Form Feed */
/*	    CHECKFOROVERFLOW(window,window->linesInWindow);
		for (i = window->linesInWindow; i > 0; i--) INSERTLINEFEED(window->buffer);
		break; */
	  case '\b':	/*	Backspace */
		if (CURRENTBUFSIZE(window->buffer) != 0) {DELETEFROMBUFFER(window->buffer,1);}
		/*	Need to delete the last character from the TextEdit Handle */
		else  {
		  short teLength = (*window->textEdit)->teLength;
		  if (teLength > 0) {
		    TESetSelect(teLength-1, teLength, window->textEdit);
		    TEDelete(window->textEdit);
		  }
		}
		break;
	  default:	/*	just add it to MT ... */
		INSERTCHAR(window,aChar);
	    break;
	}
	
	CHECKFOROVERFLOW(window,0);
  }
  
  FlushTWBuffer(window);

  window->state = IDLE;

  SetPort(saveport);

  return n;
}


/*
 * FUNCTION NOT USED
 *
 *  ** WARNING : This function is directly called by the ANSI library when reading from stdin
 */
 
 long ReadCharsFromConsole(char *buffer, long n)
{
}

/* 
 * ** WARNING : This function is directly called by the ANSI library 
 */
 
char *__ttyname(long fildes)
{
	static char *__MTDeviceName = "Nestor";
	
	if (fildes >= 0 && fildes <= 2)
		return (__MTDeviceName);

	return (NULL);
}

#endif

