
/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
/*                                                                          */
/*      Copyright (C) 2001 Raphael Blouet, Remi Gribonval, Emmanuel Bacry.  */
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


/********************************************************************/
/*                                                                  */
/*   win32_terminal.c : Win32 terminal functions                    */
/*                                                                  */
/********************************************************************/


#include "computer.h"

#ifdef LASTWAVE_WIN32_GRAPHICS
#include <windows.h>
#include <stdio.h>
#ifndef NO
#define NO 0
#endif
#ifndef YES
#define YES 1
#endif
extern void Errorf(char *format,...);


//#include "lastwave.h"

#define WIN32LW_VERSION "LastWave for Windows ALPHA VERSION" 
HANDLE stdInHandle;
HANDLE stdOutHandle;
COORD   maxPos;

void XXInitTerminal(void)
{
  FreeConsole();	
  if(!AllocConsole()){
    printf("impossible d'allouer la console!\n");	
  }	
  //Initialize the console : title, color ... are here
  SetConsoleTitle(WIN32LW_VERSION);
  
  // Initialize the stdIn and stdOut handles 
  if((stdInHandle = GetStdHandle(STD_INPUT_HANDLE))== INVALID_HANDLE_VALUE){
    printf("XXInitTerminal : bad console stdIn handle !\n");	
  }
  if((stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE))== INVALID_HANDLE_VALUE){
    printf("XXInitTerminal : bad console stdOut handle !\n");	
  }
  
  maxPos = GetLargestConsoleWindowSize(stdOutHandle);
}

void XXCloseTerminal(void)
{
  FreeConsole();
}


/* 
 * Make a beep
 */
 
void XXTerminalBeep(void)
{
	MessageBeep(MB_OK);
//  putchar(7);
//  fflush(stdout);
}

/*
 * Just print a string (not editable ==> no track of the cursor)
 */
void  XXTerminalPrintStr(char *str)
{  
//	printf("essaie d'imprimer un string\n");
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
//	printf("essaie d'imprimer un caractere\n");
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
 * Make the cursor go forward by n space
 */
 
void  XXTerminalCursorGoForward(int n)
{
  COORD newPos;
  CONSOLE_SCREEN_BUFFER_INFO scrInfo;
  
  GetConsoleScreenBufferInfo (stdOutHandle,&scrInfo);
  if(n<0){
    Errorf("XXTerminalCursorGoForward : Weird error n = %d",n);
  }
  
  newPos.X= scrInfo.dwCursorPosition.X+n;
  newPos.Y= scrInfo.dwCursorPosition.Y;
  
  if(newPos.X > maxPos.X)
    newPos.X = maxPos.X;
  
  SetConsoleCursorPosition(stdOutHandle,newPos);
}



/*
 * Make the cursor go backward by one space
 */

void  XXTerminalCursorGoBackward(int n)
{
  COORD newPos;
  CONSOLE_SCREEN_BUFFER_INFO scrInfo;
  
  GetConsoleScreenBufferInfo (stdOutHandle,&scrInfo);
  if(n<0){
    Errorf("XXTerminalCursorGoBackward : Weird error n = %d",n);
  }
  
  newPos.X=scrInfo.dwCursorPosition.X-n;
  newPos.Y=scrInfo.dwCursorPosition.Y;
  
  if(newPos.X < 0)
    newPos.X = 0;
  
  SetConsoleCursorPosition(stdOutHandle,newPos);
}



/*
 * Print an editable character from the cursor position 
 * The 'str' corresponds to what is already displayed on the screen
 * and should be redrawn (shifted by 1 character).
 * The cursor should stay at the same position.
 */
void  XXTerminalInsertChar(char c,char *str)
{
  DWORD nbWritten;
  CONSOLE_SCREEN_BUFFER_INFO scrInfo;
  
  GetConsoleScreenBufferInfo (stdOutHandle,&scrInfo);
  WriteConsoleOutputCharacter(stdOutHandle,&c,1,scrInfo.dwCursorPosition,&nbWritten);
  XXTerminalCursorGoForward(1);
  GetConsoleScreenBufferInfo (stdOutHandle,&scrInfo);
  if(strlen(str)>0)
    WriteConsoleOutputCharacter(stdOutHandle,str,strlen(str),scrInfo.dwCursorPosition,&nbWritten);
}


/*
 * Same as above but "inserts" a delete character n times
 */
 
void  XXTerminalDeleteInsertChar(int n, char *str)
{
  DWORD nbWritten1,nbWritten2;
  CONSOLE_SCREEN_BUFFER_INFO scrInfo;
  
  static char empty_string[200];
  static char stringIsFilled = NO;
  
  int i;
  
  // This should go in XXInitTerminal and XXResizeTerminalWindow 
  if(stringIsFilled == NO) {
    for(i = 0; i<198; i++) empty_string[i] = ' ';
    empty_string[199] = '\0';
    stringIsFilled = YES;
  }
  
  XXTerminalCursorGoBackward(n);
  if(strlen(str) > 0) {
    GetConsoleScreenBufferInfo (stdOutHandle,&scrInfo);
    WriteConsoleOutputCharacter(stdOutHandle,str,strlen(str),scrInfo.dwCursorPosition,&nbWritten1);
    XXTerminalCursorGoForward(nbWritten1);
  }
  else{
    nbWritten1 = 0;
  }
  if(n > 0) {
    GetConsoleScreenBufferInfo (stdOutHandle,&scrInfo);
    WriteConsoleOutputCharacter(stdOutHandle,empty_string,n,scrInfo.dwCursorPosition,&nbWritten2);
  }
  else {
    nbWritten2 = 0;
  }
  XXTerminalCursorGoBackward(nbWritten1);
}


/* Move the terminal window */
void XXTerminalMoveWindow(int x,int y)
{
}

/* Resize the terminal window */
void XXTerminalResizeWindow(int w,int h)
{
}

#endif//LASTWAVE_WIN32_GRAPHICS

