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

#include "lastwave.h"


void XXOpenGraphics(void)
{
}

void XXCloseGraphics(void)
{
}


/* Set the style of the line */
void XXSetLineStyle(FRAME frame,int flag)
{
}

/* Change the size of the pen of the Graphic Port */
void XXSetPenSize(FRAME frame,int size)
{ 
}

/* Set the pen mode of the Graphic Port */
void XXSetPenMode(FRAME frame,int mode)
{ 
}


/* Draw a line in the graphic port */
void XXDrawLine(FRAME frame,int x,int y,int x1,int y1)
{
}

/* Draw a point in the Graphic Port */
void XXDrawPoint(FRAME frame,int x,int y)
{
}

/* Draw an ellipse in a rect */
void XXDrawEllipse(FRAME frame,int x,int y,int dx, int dy)
{
}

/* Fill an ellipse in a rect */
void XXFillEllipse(FRAME frame,int x,int y,int dx, int dy)
{
}


/* Draw a rectangle */
void XXDrawRect(FRAME frame,int x,int y,int dx,int dy)
{
}

/* Fill a rectangle */
void XXFillRect(FRAME frame,int x,int y,int dx,int dy)
{
} 

/* Set the clip rect */
void XXSetClipRect(FRAME frame, int x, int y, int w, int h)
{
}


/*
 * Managing Pixmaps
 */

void XXAllocPixMap(int w,int h,unsigned char **pData,int *pRowBytes)
{
}

void XXDeletePixMap(void)
{  
}
  
/* Display an image in a frame */
void XXDisplayPixMap(FRAME frame,int winX,int winY)
{
 
}

char XXIsDisplayBLittle(void)
{
  return(NO);
}

void XXFlush(void)
{
}

/********************************************************/
/*                Other functions                       */
/********************************************************/

/* Delete a window */
void XXDeleteFrame(FRAME frame)
{
}

/* Create a window */
FRAME XXNewFrame(char *title,int x, int y, int w,int h)
{
  return(NULL);
}

/* Quand c'est juste la position ou le titre qui change ne pas faire d'expose ni configure */
/* Quand la taille change avec enlever juste le configure et garder l'expose */
/* Change the size the position and the title of a window */
void XXChangeFrame(FRAME frame,char *title,int x,int y,int w, int h)
{
}

FRAME XXGetFrontFrame(void)
{
 return(NULL);
}

void XXFrontFrame(FRAME frame)
{
}



/********************************************************/
/*               Event managing                         */
/* Return the next event of type EventMask on the queue */
/* Return 0 if no event                                 */
/* EventMask =     
     1- BUTTONPRESS - Param : x,y,button
     2- REFRESH     - Param : x,y,dx,dy
        The parameters corresponds to the area of the
	window which has to be refreshed.
        This event must take care of the change
	of size/position of the window (this is indicated
	by dx < 0)
     3- KEYPRESS/KEYRELEASE    - Param : key,x,y
     4- ENTERWINDOW - no param
     5- LEAVEWINDOW - no param
   Any combination of those is possible                 */
/********************************************************/

/* Turn the autorepeat on */
void XXAutoRepeatOn(void)
{
}

/* Turn the autorepeat off */
void XXAutoRepeatOff(void)
{
}



void XXGetNextEvent(EVENT event,int flagWait)
{
  static char str[10000];
  static n = 0;
  static flagUp = NO;
  char *str1;
  
  if (flagUp == YES) {
    event->type = KeyUp;
    event->object = NULL;
    event->key = str[n];
    flagUp = NO;
    n++;
    return;
  }
    
  if (n==0 || str[n] == '\0') {
    gets(str);
    strcat(str,"\n");
    n = 0;
  }
  
  event->type = KeyDown;
  event->object = NULL;
  event->key = str[n];
  flagUp = YES;
}


/* Change the color of the Graphic port */
void XXSetColor(FRAME frame, unsigned long pixel)
{
}


/*
 * Set the current colormap according to the values of red, green and blue. 
 * Specifies the index of the color of the cursor and of the colors that must
 * behave 'properly' during a PenInverse mode.
 * Returns the number of color cells that are used.
 */

int XXSetColormap(unsigned short red[],unsigned short green[],unsigned short blue[],
		  unsigned long pixels[],int nCols,int flagSharedColormap, int mouseMode,
		  unsigned short mouseRed, unsigned short mouseGreen, unsigned short mouseBlue)
{
 return(100);
}


/*
 * Animate one color of the colormap
 */

void XXAnimateColor(unsigned long pixel, unsigned short r, unsigned short g, unsigned short b)
{
}


/*
 * Returns the number of colors available 
 */
int XXNumOfColors(void)
{
return(65000);
}

/*
 * Returns the depth
 */
int XXDepth(void)
{
return(16);
}

/*
 * Is the screen BW ?
 */

int XXIsBWScreen(void)
{
return(NO);
}

/*
 * Type of the screen
 */

char * XXScreenType(void)
{
  return("ansi");
}

void XXScreenRect(int *x, int *y, int *w, int *h)
{
  *x = *y = 0;
  *w = *h = 1000;
}

/*
 * Get the foreground and background colors of the terminal window 
 */

void XXBgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
}

void XXFgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
}




/* Draw a string in the graphic port */
void XXDrawString(FRAME frame,int x,int y,char *str)
{
}


char *XXGetDefaultFont(FONTID *fontStruct,int *size) 
{
  return("none");
}


void XXGetFontInfo(FONT font, int *ascent, int *descent, int *interline)
{
}

int XXGetStringWidth(FONT font, char *text)
{
return(10);
}

void XXSetFont(FRAME frame,FONT font)
{
}


void XXFontMatch(char *name,char flagSize, int size,char flagStyle,unsigned char style)
{
}


char XXExistFont(char *name,int size, unsigned char style,FONTID *fontStruct)
{
  return(YES);
}


void C_System(char **argv)
{
}

#endif

