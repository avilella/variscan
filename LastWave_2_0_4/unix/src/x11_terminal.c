/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   U n i x   2 . 0                                   */
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

#ifdef LASTWAVE_X11_GRAPHICS

#include "x11_graphics.h"
#include "lastwave.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <signal.h>
#include <sys/wait.h>


/* X11 Include Files */
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>


/*
 * The pipe between the two processes 
 * (refered to as the 'terminal' (the father) and the 'program')
 */
int fildes[2];

/* The process identifier */
static int pid;

/* Display variable of the terminal */
static Display *myDisplay;

/* The event window variable of the terminal */
static FRAME myEventWindow;

/* Not used for now */
static char flagCursorAbility;

/* The codes for moving the cursor */
static char cursorForwardCode[5];
static char cursorBackwardCode[5];
static char eraseLastCharCode[5];


/*
 * Are we in the terminal process ?
 */

static int IsTerm()
{
  return(pid!=0);
}


/*
 * Quit the terminal process (the father)
 * in case the child stopped
 */

static void QuitTerm(int s)
{
  exit(0);
}


/*
 * Called when the getchar function of the 
 * terminal process read a character
 */

static void  XXTerminalGetNewChar(unsigned char c)
{
  XKeyEvent event;

  event.keycode = c;
  event.window = myEventWindow;
  event.send_event = True;
  event.type = KeyPress;
  event.display = myDisplay;
  
  /* Then send the event */
  XSendEvent(myDisplay,myEventWindow,False,KeyPressMask,(XEvent *) (&event));
  XFlush(myDisplay); 
}


/*
 * Make the terminal to beep
 */

void XXTerminalBeep(void)
{
  putchar(7);
  fflush(stdout);
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
  fprintf(stderr,"%s",str);
  fflush(stderr);
}

/* 
 * Same thing as above but with a character
 */

void  XXTerminalPrintChar(char c)
{
  putchar(c);
}

/*
 * Just flushes the terminal output
 */

void  XXTerminalFlush(void)
{
  fflush(stdout);
}


/*
 * Make the cursor go forward by one space
 */

void  XXTerminalCursorGoForward(int n)
{
  int i;

  while(n!=0) {
    i=0;
    while (cursorForwardCode[i]!=0) putchar(cursorForwardCode[i++]);
    n--;
  }
  fflush(stdout);
}

/*
 * Make the cursor go backward by one space
 */

void  XXTerminalCursorGoBackward(int n)
{
  int i;

  while(n!=0) {
    i=0;
    while (cursorBackwardCode[i]!=0) putchar(cursorBackwardCode[i++]);
    n--;
  }
  fflush(stdout);
}


/*
 * Print an editable character from the cursor position 
 * The 'str' corresponds to what is already displayed on the screen
 * and should be redrawn (shifted by 1 character).
 * The cursor should stay at the same position.
 */

void  XXTerminalInsertChar(char c,char *str)
{
  int i;

  printf("%c%s",c,str);

  while(*str!='\0') {
    i = 0;
    while (cursorBackwardCode[i]!=0) putchar(cursorBackwardCode[i++]);
    str++;
  }
  fflush(stdout);
}


/*
 * Same as above but "inserts" a delete character n times
 */

void  XXTerminalDeleteInsertChar(int n, char *str)
{
  int i;
  int total;
  int n1 = n;
  int n2;
  
/*  if (str == NULL || str[0] == '\0') {
    i = 0;
    while (eraseLastCharCode[i]!=0) putchar(eraseLastCharCode[i++]);
    return;
  }
 */
 
  while (n1 !=0) {
    i = 0;
    while (cursorBackwardCode[i]!=0) putchar(cursorBackwardCode[i++]); 
    n1--;
  }

  if (str == NULL) n1 = 0;
  else n1 = strlen(str);

  printf("%s",str);
  n2 = n;
  while(n2!=0) {putchar(' ');n2--;};

  n1 += n;    
  while(n1!=0) {
    i = 0;
    while (cursorBackwardCode[i]!=0) putchar(cursorBackwardCode[i++]);
    n1--;
  }
    
  fflush(stdout);
}


/*
 * The main loop the terminal process is performing 
 */

static void XXTerminalLoop()
{
  unsigned char c;
  unsigned long theChar;

  if (GraphicMode) signal(SIGCHLD,QuitTerm);

  while(1) {
    c = getchar();
    XXTerminalGetNewChar(c);
  }		
}	


/*
 * Get the key equivalent (for the terminal) of a the action 'str'
 * (it uses the file tempFileName for reading it)
 */

static char *tempFileName;

static char* GetKey(char *str) 
{
  static char code[5];
  FILE *f;
  char tempStr[200];
  int i,n;

  code[0] = '\0';
  sprintf(tempStr,"tput %s | od -d > %s",str,tempFileName);
  system(tempStr);  
  f = fopen(tempFileName,"r");
  fscanf(f,"%d",&i);
  n = fscanf(f,"%d",&i);
  if (IsCPULittleEndian) {
    if (n != 0 && n != -1) {
      code[1] = i/256;
      code[0] = i-(i/256)*256;
      n = fscanf(f,"%d",&i);
      if (n != 0 && n!= -1) {
        code[3] = i/256;
        code[2] = i-(i/256)*256;
        code[4] = 0;
      }
    }
  } else {
    if (n != 0 && n != -1) {
      code[0] = i/256;
      code[1] = i-(i/256)*256;
      n = fscanf(f,"%d",&i);
      if (n != 0 && n!= -1) {
        code[2] = i/256;
        code[3] = i-(i/256)*256;
        code[4] = 0;
      }
    }
  }

  fclose(f);
  return(code);
}


/*
 * Initialization of terminal when graphics are used 
 */

void XXInitTerminalSession()
{
  unsigned long int window = 0;

  /* The pipes for communication between the processes */
  pipe(fildes);

  /* We create the 2 processes */
  pid = fork();

  /* All the remain is executed only by the terminal process */
  if (IsTerm() == YES) {

    /* Open the display of the terminal process */
    myDisplay = XOpenDisplay(NULL);

    /* Read the id of the event process (created by the program process) */
    while(read(fildes[0],&window,sizeof(unsigned long))== 0);
    myEventWindow = window;

    /* The loop the terminal will do for ever */
    XXTerminalLoop();
  }
}


/*
 * Initialization of the terminal 
 * (the 2 processes have not been created yet)
 */

void XXInitTerminal(void)
{
  FILE *stream;
  char str[200],*str1;
  int i;

  flagCursorAbility = YES;

  /* Trying to open graphics */
  if (GraphicMode) {
    myDisplay = XOpenDisplay(NULL);
    if (myDisplay == NULL) {
      SetGraphicMode(NO);
    }
    else {
      /* init of the terminal codes */
      tempFileName = tmpnam(NULL);
      strcpy(cursorForwardCode,GetKey("cuf1"));
      strcpy(cursorBackwardCode,GetKey("cub1"));
      strcpy(eraseLastCharCode,GetKey("dch1"));
      sprintf(str,"rm %s",tempFileName);
      system(str);
      
      /* init of the terminal itself */
      system("stty -echo");
      system("stty -icanon min 1");
      system("stty -iexten"); 
      system("stty -ixon"); 
      system("stty -ixoff"); 
      system("stty -echonl");
      system("stty noflsh");  
      XCloseDisplay(myDisplay);
    }
  }

  if (GraphicMode) XXInitTerminalSession();
}


/* 
 * Close the terminal process
 */

void XXCloseTerminal(void)
{
}

void XXTerminalMoveWindow(int x,int y)
{
}

/* Resize the terminal window */
void XXTerminalResizeWindow(int w,int h)
{	
}


#endif

