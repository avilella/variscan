/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   A n s i   2 . 0                                   */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
/*      email : lastwave@polytechnique.fr                                   */
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

#ifdef LASTWAVE_ANSI_GRAPHICS

#include <stdio.h>

void XXInitTerminal(void)
{
}

void XXCloseTerminal(void)
{
}


/* 
 * Make a beep
 */
 
void XXTerminalBeep(void)
{
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


/*
 * Make the cursor go forward by one space
 */
 
void  XXTerminalCursorGoForward(int n)
{
}



/*
 * Make the cursor go backward by one space
 */

void  XXTerminalCursorGoBackward(int n)
{
}


/*
 * Print an editable character from the cursor position 
 * The 'str' corresponds to what is already displayed on the screen
 * and should be redrawn (shifted by 1 character).
 * The cursor should stay at the same position.
 */
 
void  XXTerminalInsertChar(char c,char *str)
{
}


/*
 * Same as above but "inserts" a delete character n times
 */
 
void  XXTerminalDeleteInsertChar(int n, char *str)
{
}


/* Move the terminal window */
void XXTerminalMoveWindow(int x,int y)
{
}

/* Resize the terminal window */
void XXTerminalResizeWindow(int w,int h)
{
}



#endif
