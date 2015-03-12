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

#include <Controls.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>
#include <Sound.h>
#include <Files.h>

#include "lastwave.h"
#include "MacTextWindow.h"

#include <console.h>
#include <signal.h>
#include <Traps.h>




/*
 * Function called by the Ansi library
 */

short InstallConsole(short fd)
{
  return(0);
}

void RemoveConsole(void)
{
  return;
}



/*
 * Initialization of the terminal console
 */

void XXInitTerminal(void)
{
}

/* 
 * Close the terminal
 */
 
void XXCloseTerminal(void)
{
  DeleteAllTextWindows();
}


/* 
 * Make a beep
 */
 
void XXTerminalBeep(void)
{
  SysBeep(1);
}

/*
 * Just print a string (not editable ==> no track of the cursor)
 */
void  XXTerminalPrintStr(char *str)
{  
  printf("%s",str);
}

 
void  XXTerminalPrintErrStr(char *str)
{
  printf("%s",str);
}

/* 
 * Same thing as above but with a character
 */

void  XXTerminalPrintChar(char c)
{  
  if (c == '\r') c = '\n';
  putchar(c);
}

/*
 * Just flushes the terminal output
 */
 
void XXTerminalFlush(void)
{
  fflush(stdout);
}

static void AdjustCursor (void)
{
  if ((*XXTerminalWindow->textEdit)->selStart != (*XXTerminalWindow->textEdit)->selEnd)
    TESetSelect(XXTerminalWindow->selStart, XXTerminalWindow->selStart, XXTerminalWindow->textEdit);  
  
}


/*
 * Make the cursor go forward by one space
 */
 
void  XXTerminalCursorGoForward(int n)
{
  int teSelEnd;
  
  AdjustCursor();
  
  teSelEnd = (*XXTerminalWindow->textEdit)->selEnd;
  
  TESetSelect(teSelEnd+n,teSelEnd+n, XXTerminalWindow->textEdit); 
}



/*
 * Make the cursor go backward by one space
 */

void  XXTerminalCursorGoBackward(int n)
{
  int teSelStart;
  
  AdjustCursor();
  
  teSelStart =  (*XXTerminalWindow->textEdit)->selStart;
  
  TESetSelect(teSelStart-n,teSelStart-n, XXTerminalWindow->textEdit); 
}


/*
 * Print an editable character from the cursor position 
 * The 'str' corresponds to what is already displayed on the screen
 * and should be redrawn (shifted by 1 character).
 * The cursor should stay at the same position.
 */
 
void  XXTerminalInsertChar(char c,char *str)
{
  AdjustCursor();

  TEKey(c, XXTerminalWindow->textEdit);
}


/*
 * Same as above but "inserts" a delete character n times
 */
 
void  XXTerminalDeleteInsertChar(int n, char *str)
{
  int teSelStart;
  
  AdjustCursor();

  teSelStart = (*XXTerminalWindow->textEdit)->selStart;

  TESetSelect(teSelStart-n, teSelStart, XXTerminalWindow->textEdit);
  TEDelete(XXTerminalWindow->textEdit);
}


/* Move the terminal window */
void XXTerminalMoveWindow(int x,int y)
{
	MoveWindow((WindowPtr) (&(XXTerminalWindow->window)),(short) x,(short) y,false);
	
}

/* Resize the terminal window */
void XXTerminalResizeWindow(int w,int h)
{
	TWResizeWindow(XXTerminalWindow,w,h);
	
}




#endif
