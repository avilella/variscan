/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 4                           */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
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
/*  window_manager.c        The Machine dependent Graphic functions         */
/*                          This is the intermediate layer                  */ 
/*                                                                          */    
/****************************************************************************/


#include "lastwave.h"
#include <time.h>



static char flagInvisible = NO;
static FONT currentFont = NULL;


/***************************************/
/*     Initialization                  */
/***************************************/

static void WInitFont(void);

char isDisplayBLittle = NO;

void WOpenGraphics(void)
{
  if (GraphicMode) XXOpenGraphics();

  WInitFont();
  isDisplayBLittle = WIsDisplayBLittle();
}


void WCloseGraphics(void)
{
   if (GraphicMode) XXCloseGraphics();
}

void WFlush(void)
{
   if (GraphicMode) XXFlush();
}


/**********************/
/* Set the attributes */
/**********************/

/* Change the current color */
void WSetColor(WINDOW win,unsigned long color)
{
  int pixel;

  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");

  if (color & invisibleColor) {
    flagInvisible = YES;
    return;
  }
  else flagInvisible = NO;
  
  if (PSMode) PSSetColor(color);
  else if (GraphicMode) {
    pixel = Color2Pixel(color);     
    XXSetColor(win->frame,pixel);
  }	
}	


/* Set the style of the line */
void WSetLineStyle(WINDOW win,int lineStyle)
{
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
  if (PSMode) PSSetLineStyle(lineStyle); 
  else if (GraphicMode) XXSetLineStyle(win->frame,lineStyle); 
}

/* Change the size of the pen */
void WSetPenSize(WINDOW win,int size)
{ 
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
 if (PSMode) PSSetPenSize(size); 
 else if (GraphicMode) XXSetPenSize(win->frame,size); 
}

/* Change the mode of the pen */
void WSetPenMode(WINDOW win, int mode)
{ 
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
 if (PSMode) PSSetPenMode(mode); 
 else if (GraphicMode) XXSetPenMode(win->frame, mode);
}


/* Set the clipping rectangle */
static int clipX,clipY,clipW,clipH;
static WINDOW clipWin = NULL;

void WSetClipRect(WINDOW win, int x, int y, int w, int h)
{
  if (win != (WINDOW) NULL) {
    if (win->flagHide) return;
    if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
    clipWin = win;
    clipX = x;
    clipY = y;
    clipW = w;
    clipH = h;
    if (PSMode) PSSetClipRect(x,y,w,h);
    else if (GraphicMode) XXSetClipRect(win->frame,x,y,w,h);
  }
  else clipWin = NULL;
}

/*  if (GraphicMode) {
    if (win != NULL && win->frame != (FRAME) NULL) {
      if (win->flagHide) return;
      clipWin = win;
      clipX = x;
      clipY = y;
      clipW = w;
      clipH = h;
      if (PSMode) PSSetClipRect(x,y,w,h);
      else XXSetClipRect(win->frame,x,y,w,h);
    }
    else clipWin = (WINDOW) NULL;
  }
}
*/


/* Get the clipping rectangle */
void WGetClipRect(WINDOW *win, int *x, int *y, int *w, int *h)
{
  *win = clipWin;
  *x = clipX;    
  *y = clipY;    
  *w = clipW;    
  *h = clipH;    
}

/*************************************
 *
 * Managing Pixmaps
 *
 ************************************/
 
static unsigned char *pixMapData = NULL;
static int pixMapNCols = 0;
static int pixMapNRows = 0;
static int depth = 0;
static int pixMapRowBytes;

void WInitPixMap(int nRows, int nCols)
{  
  if (PSMode) PSInitPixMap(nRows,nCols);
  else if (GraphicMode) {
    if (pixMapData != NULL) {
      XXDeletePixMap();
      pixMapData = NULL;
    }
    depth = WDepth();
    XXAllocPixMap(nCols,nRows,&pixMapData,&pixMapRowBytes);
    pixMapNCols = nCols;
    pixMapNRows = nRows;
  }
}


void WSetPixelPixMap(int i, int j, unsigned long color)
{
  unsigned long pixel;
  int shift;
  
  if (PSMode) PSSetPixelPixMap(i, j, color);
  
  else if (GraphicMode) {
  
    if (i < 0 || i >= pixMapNRows) {
      Warningf("WSetPixelPixMap() : Bad 'i' index %d",i);
      return;
    }

    if (j < 0 || j >= pixMapNCols) {
      Warningf("WSetPixelPixMap() : Bad 'j' index %d",j);
      return;
    }

    pixel = Color2Pixel(color);
  
    switch(depth) {
    
       case 8 : 
        *(pixMapData+pixMapRowBytes*i+j) = pixel & 0xFF;  
        break;
      
      case 16 :
        shift = pixMapRowBytes*i+2*j;
        if (isDisplayBLittle) {
          *(pixMapData+shift) = pixel & 0xFF;  
          *(pixMapData+shift+1) = (pixel & 0xFF00)>>8;  
        } else {
          *(pixMapData+shift+1) = pixel & 0xFF;  
          *(pixMapData+shift) = (pixel & 0xFF00)>>8;  
        }
        break;

#ifdef LASTWAVE_MAC_GRAPHICS  /* ANSI changed  ????????????? */
      case 24 : 
        shift = pixMapRowBytes*i+3*j;
        if (isDisplayBLittle) {
          *(pixMapData+shift+2) = (pixel & 0xFF0000)>>16;  
          *(pixMapData+shift+1) = (pixel & 0xFF00)>>8;  
          *(pixMapData+shift) = pixel & 0xFF;  
        }
        else {
          *(pixMapData+shift) = (pixel & 0xFF0000)>>16;  
          *(pixMapData+shift+1) = (pixel & 0xFF00)>>8;  
          *(pixMapData+shift+2) = pixel & 0xFF;  
        }
        break;
#endif

#ifndef LASTWAVE_MAC_GRAPHICS  /* ANSI changed */
      case 24 :
#endif       
      case 32 :       
        shift = pixMapRowBytes*i+4*j;
        if (isDisplayBLittle) {
          *(pixMapData+shift+3) = (pixel & 0xFF000000)>>24;
          *(pixMapData+shift+2) = (pixel & 0xFF0000)>>16;  
          *(pixMapData+shift+1) = (pixel & 0xFF00)>>8;  
          *(pixMapData+shift) = pixel & 0xFF;  
        } 
        else {
          *(pixMapData+shift) = (pixel & 0xFF000000)>>24;
          *(pixMapData+shift+1) = (pixel & 0xFF0000)>>16;  
          *(pixMapData+shift+2) = (pixel & 0xFF00)>>8;  
          *(pixMapData+shift+3) = pixel & 0xFF;  
        }
        break;
        
      default : 
        Errorf("SetPixelPixMap() : Do not know how to deal with that screen depth");
    }
  }
}
  
    
/* Display the current pixmap */
/* ??? PB if winX,winY are completly out of bounds */
void WDisplayPixMap(WINDOW win,int winX,int winY)
{
  if (win->flagHide) return;
  if (flagPSMode == YES) PSDisplayPixMap(winX,winY); 
  else if (GraphicMode) {
    XXDisplayPixMap(win->frame,winX,winY);    
    XXDeletePixMap();
    pixMapData = NULL;
  }
}


char WIsDisplayBLittle(void)
{
  if (!GraphicMode) return(NO);
  return(XXIsDisplayBLittle());
}


/********************************************************/
/*                Other functions                       */
/********************************************************/

/* Create a frame */
FRAME WNewFrame(char *title, int x, int y,int w, int h)
{  
  if (GraphicMode)  return(XXNewFrame(title,x,y,w,h));
  else return((FRAME) NULL);
}

/* Delete a frame */
void WDeleteFrame(FRAME frame)
{  
  if (frame == (FRAME) NULL) return;
  if (GraphicMode)  {
    if (clipWin && clipWin->frame == frame) clipWin = (WINDOW) NULL;
    XXDeleteFrame(frame);
  }
}

/* Change the size the position and the title of a window */
void WChangeFrame(FRAME frame,char *title, int x, int y, int w, int h)
{  
  if (frame == (FRAME) NULL) return;
  if (GraphicMode)  XXChangeFrame(frame,title,x,y,w,h);
}

/* Gets the window associated to a frame */
WINDOW Frame2Window(FRAME frame) 
{
  int r;
  AHASHELEM e;
  WINDOW win;
  
  if (frame == (FRAME) NULL) return(NULL);
  
  /* We loop on the windows */
  for (r = 0; r<theWindowsHT->nRows;r++) {
    for (e = theWindowsHT->rows[r]; e != NULL; e = e->next) {    
      win = (WINDOW) e;
      if (win->frame == frame) return(win);
    }
  }
  
  return(NULL);
}

WINDOW WGetFrontWindow(void)
{
  return(Frame2Window(XXGetFrontFrame()));
}

void WFrontWindow(WINDOW win)
{
  XXFrontFrame(win->frame);
}

/********************************************************/
/*               Event managing                         */
/* Return the next event of type EventMask on the queue */
/* Return 0 if no event                                 */
/* EventMask =     
     1- BUTTONDOWN - Param : x,y,button
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
void WAutoRepeatOn(void)
{
   if (GraphicMode) XXAutoRepeatOn();
}

/* Turn the autorepeat off */
void WAutoRepeatOff(void)
{
  if (GraphicMode) XXAutoRepeatOff();
}

void WGetNextEvent(EVENT event, int flagWait)
{
  char c;

  if (GraphicMode) {
    XXGetNextEvent(event,flagWait);
    if (event->type != NoEvent && event->object != NULL) toplevelCur->lastWindow = (WINDOW) event->object;
  }
  else {
    c = getchar();
    if (c == EOF || c == 4) event->key = EofKC;
    else if (c == 9) event->key = TabKC;
    else if (c == 8 || c == 127) event->key = DeleteKC;
    else if (c == '\r' || c == '\n') event->key = NewlineKC;
    else if (c == 27) event->key = EscapeKC;
    else event->key = c;
    event->object = NULL;
    event->type = KeyDown;
  }
}



/****************************************/
/*               Color stuff            */
/****************************************/

/*
 * Set the current colormap according to the values of red, green and blue. 
 *    The first color is the cursor color. 
 *    The next 'nCursorColors' are the colors that must behave 'well' during a PenInverse mode
 * Returns the number of colors that are used.
 */

int WSetColormap(unsigned short red[],unsigned short green[],unsigned short blue[],
                 unsigned long pixels[], int nCols, int flagSharedColormap, int mouseMode)
{
  unsigned short r,g,b;
  
  if (GraphicMode) {
    Color2RGB(mColor,&r,&g,&b);
    return(XXSetColormap(red,green,blue,pixels,nCols,flagSharedColormap,mouseMode,r,g,b));
  }
  return(nCols);
}

/* Animate one color */
void WAnimateColor(unsigned long c, unsigned short r, unsigned short g, unsigned short b)
{
  if (GraphicMode) XXAnimateColor(Color2Pixel(c),r,g,b);
}


/* Returns the number of colors available */
int WNumOfColors(void)
{
  if (GraphicMode) return(XXNumOfColors());
  else return(65536);
}

/* Returns the depth of the screen */
int WDepth(void)
{
  if (GraphicMode) return(XXDepth());
  else return(32);
}


/*
 * Is the screen BW ?
 */

int WIsBWScreen(void)
{
  if (GraphicMode)  return(XXIsBWScreen());
  else return(NO);
}

/*
 * Type of the screen
 */

char *WScreenType(void)
{
  if (GraphicMode)  return(XXScreenType());
  else return("TrueColor");
}

/*
 * Get the screen size
 */
void WScreenRect(int *x, int *y, int *w, int *h)
{
  if (GraphicMode)  XXScreenRect(x, y, w, h);
  else {
    *x = *y = 0;
    *w = *h = 10000;
  }
}

/*
 * Get the default foreground and background colors
 */

void WBgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  if (GraphicMode)  XXBgColor(r,g,b);
  else *r = *g = *b = 65535;
}

void WFgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  if (GraphicMode)  XXFgColor(r,g,b);
  else *r = *g = *b = 0;
}



 
 
/*********************************************************************
 *
 * Functions to draw on windows using gobject or window coordinates
 *
 *********************************************************************/
 
/* Draw a line */
void WDrawLine(GOBJECT o,float x,float y,float x1,float y1)
{
  int wx,wy,wx1,wy1,i;
  WINDOW win;

  if (flagInvisible) return;
  win = GetWin(o);
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
 
  Local2Global(o,x,y,&wx,&wy);
  Local2Global(o,x1,y1,&wx1,&wy1);
  
  /* Intersecting the window boundary ? */
  if (wx < 0 || wx1 < 0 || wx > win->w || wx1 > win->w) {
    if (wx < 0 && wx1 < 0) return;
    if (wx > win->w && wx1 > win->w) return;
    if (wx > wx1) {
      i = wx; wx = wx1; wx1 = i;
      i = wy; wy = wy1; wy1 = i;
    }
    if (wx < 0) {
      wy = -wx*(wy1-wy)/(wx1-wx)+wy;
      wx = 0; 
    }
    else if (wx1 > win->w) {
      wy1 = (win->w-wx)*(wy1-wy)/(wx1-wx)+wy;
      wx1 = win->w; 
    }
  }
  if (wy < 0 || wy1 < 0 || wy > win->h || wy1 > win->h) {
    if (wy < 0 && wy1 < 0) return;
    if (wy > win->h && wy1 > win->h) return;
    if (wy > wy1) {
      i = wx; wx = wx1; wx1 = i;
      i = wy; wy = wy1; wy1 = i;
    }
    if (wy < 0) {
      wx = -wy*(wx1-wx)/(wy1-wy)+wx;
      wy = 0; 
    }
    else if (wy1 > win->h) {
      wx1 = (win->h-wy)*(wx1-wx)/(wy1-wy)+wx;
      wy1 = win->h; 
    }
  }
        
  if (PSMode) PSDrawLine(wx,wy,wx1,wy1); 
  else if (GraphicMode) XXDrawLine(win->frame,wx,wy,wx1,wy1);
}

/* Draw a point */
void WDrawPoint(GOBJECT o,float x,float y)
{
  int wx,wy;
  WINDOW win;
  
  if (flagInvisible) return;
  win = GetWin(o);
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");

  Local2Global(o,x,y,&wx,&wy);

  if (PSMode) PSDrawPoint(wx,wy); 
  else if (GraphicMode) XXDrawPoint(win->frame,wx,wy);
}


/*
 *
 * Drawing Shapes defined by a rectangle
 *
 */

enum {  
  ShapeRect = 1,
  ShapeEllipse,
  ShapeCross
};


static void DrawShape(char shape, GOBJECT o, float x,float y, float dx, float dy, char flagSizeIsInPixel, RectType rectType,char flagFilled)
{
  int wx,wy,ww,wh;
  WINDOW win;
  
  if (flagInvisible) return;

  win = GetWin(o);
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
  
  if (!flagSizeIsInPixel) Local2GlobalRect(o,x,y,dx,dy,rectType,&wx,&wy,&ww,&wh);
  else {
    Local2Global(o,x,y,&wx,&wy);
    ww = dx;
    wh = dy;
    GRect2Rect(&wx,&wy,&ww,&wh,rectType);
  }
  
  if (shape == ShapeRect) {
    if (!flagFilled) {  
      if (PSMode) PSDrawRect(wx,wy,ww,wh);
      else if (GraphicMode) XXDrawRect(win->frame,wx,wy,ww,wh);
    }
    else {
      if (PSMode) PSFillRect(wx,wy,ww,wh);
      else if (GraphicMode) XXFillRect(win->frame,wx,wy,ww,wh);
    }
  }
  else if (shape == ShapeEllipse) {
    if (!flagFilled) {  
      if (PSMode) PSDrawEllipse(wx,wy,ww,wh);
      else if (GraphicMode) XXDrawEllipse(win->frame,wx,wy,ww,wh);
    }
    else {
      if (PSMode) PSFillEllipse(wx,wy,ww,wh);
      else if (GraphicMode) XXFillEllipse(win->frame,wx,wy,ww,wh);
    }
  }
}  

void WClearRect(GOBJECT o,unsigned long color, float x,float y,float dx,float dy,char flagSizeIsInPixel,RectType rectType)
{
  WINDOW win;
    
  win = GetWin(o);
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
  WSetColor(win,color); 

  DrawShape(ShapeRect,o,x,y,dx,dy,flagSizeIsInPixel,rectType,YES);
} 

void WDrawRect(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel,RectType rectType)
{
  DrawShape(ShapeRect,o,x,y,dx,dy,flagSizeIsInPixel,rectType,NO);
} 

void WFillRect(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel, RectType rectType)
{
  DrawShape(ShapeRect,o,x,y,dx,dy,flagSizeIsInPixel,rectType,YES);
} 

void WDrawEllipse(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel, RectType rectType)
{
  DrawShape(ShapeEllipse,o,x,y,dx,dy,flagSizeIsInPixel,rectType,NO);
} 

void WFillEllipse(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel, RectType rectType)
{
  DrawShape(ShapeEllipse,o,x,y,dx,dy,flagSizeIsInPixel,rectType,YES);
} 


/*
 *
 * Drawing Shapes defined by a centered rectangle
 *
 */
  
static void DrawCenteredShape(char shape, GOBJECT o, float x,float y, float r1, float r2, char flagSizeIsInPixel, char flagFilled)
{
  int wx,wy,wx1,wy1,dx,dy;
  WINDOW win;

  if (flagInvisible) return;
  
  if (shape == ShapeCross && PSMode) {
    Local2Global(o,x,y,&wx,&wy);
    PSDrawLine(wx-r1,wy,wx+r1,wy);
    PSDrawLine(wx,wy-r2,wx,wy+r2);
    return;
  }

  if (flagSizeIsInPixel) {
    Local2Global(o,x,y,&wx,&wy);
    wx -= r1;
    wy -= r2;
    dx = 2*r1+1;
    dy = 2*r2+1;
   }
   else {
    Local2Global(o,x-r1,y-r2,&wx,&wy);
    Local2Global(o,x+r1,y+r2,&wx1,&wy1);
    if (wx1 >= wx) wx1++;
    else wx++;
    if (wy1 >= wy) wy1++;
    else wy++;   
    dx = wx1-wx;
    dy = wy1-wy;
    if (dx < 0) {wx += dx-1;dx=-dx;}
    if (dy < 0) {wy += dy-1;dy=-dy;}
  } 
  win = GetWin(o);
  if (win->flagHide) return;

  if (shape == ShapeRect) {
    if (!flagFilled) {  
      if (PSMode) PSDrawRect(wx,wy,dx,dy);
      else if (GraphicMode) XXDrawRect(win->frame,wx,wy,dx,dy);
    }
    else {
      if (PSMode) PSFillRect(wx,wy,dx,dy);
      else if (GraphicMode) XXFillRect(win->frame,wx,wy,dx,dy);
    }
  }
  else if (shape == ShapeEllipse) {
    if (!flagFilled) {  
      if (PSMode) PSDrawEllipse(wx,wy,dx,dy);
      else if (GraphicMode) XXDrawEllipse(win->frame,wx,wy,dx,dy);
    }
    else {
      if (PSMode) PSFillEllipse(wx,wy,dx,dy);
      else if (GraphicMode) XXFillEllipse(win->frame,wx,wy,dx,dy);
    }
  }
  else if (shape == ShapeCross) {
    if (GraphicMode) {
      XXDrawLine(win->frame,wx+r1,wy,wx+r1,wy+dy);
      XXDrawLine(win->frame,wx,wy+r2,wx+dx,wy+r2);
    }      
  }
}  

void WDrawCenteredRect(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel)
{
  DrawCenteredShape(ShapeRect,o,x,y,dx,dy,flagSizeIsInPixel,NO); /* ?? ps ??*/
} 

void WFillCenteredRect(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel)
{
  DrawCenteredShape(ShapeRect,o,x,y,dx,dy,flagSizeIsInPixel,YES);
} 

void WDrawCenteredEllipse(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel)
{
  DrawCenteredShape(ShapeEllipse,o,x,y,dx,dy,flagSizeIsInPixel,NO); /* ?? ps ??*/
} 

void WDrawCenteredCross(GOBJECT o,float x, float y, float dx, char flagSizeIsInPixel)
{
  DrawCenteredShape(ShapeCross,o,x,y,dx,dx,flagSizeIsInPixel,NO); /* ?? ps ??*/
} 

void WFillCenteredEllipse(GOBJECT o,float x, float y, float dx, float dy, char flagSizeIsInPixel)
{
  DrawCenteredShape(ShapeEllipse,o,x,y,dx,dy,flagSizeIsInPixel, YES);
} 



/******************************************************************
 *
 *  Drawing axis 
 *
 ******************************************************************/

/*
 * The following is derived from the quickdraw source ( with major modif.)
 */


#define ALMOST1		.9999999
#define ALMOST1_DIV_5		.1999999
# define PXTICK 4.0 

static double Round[] = { 1., 2., 2.5, 5., 10., 20. };

/* The long ticks of the axis */
static double theXLongTicks[20], theYLongTicks[20];
static int nXLongTicks, nYLongTicks;

/* The short ticks of the axis */
static double theXShortTicks[100], theYShortTicks[100];
static int nXShortTicks, nYShortTicks;


static double g_stsize(double vmin, double vmax,int * n12,double * xvmin,double * xvmax,double * rmin , int *nn0)
{
  double pstep, log10, rstep, order, power, smin, use1, vdif;
  int i, rmin_old;
	
  vdif = vmax - vmin;
  pstep = fabs(vmax - vmin) / 6;
  log10 = log(10.0);
  order = log(pstep)/log10;
	
  if(order < 0) order = order - ALMOST1;
	
  order = (int)order;
  power = pow(10.0, order);
	
  for(i = 0; i < 6; i++) {
    rstep = Round[i]*power;
    if(rstep>=pstep)
    break;
  }
	
  smin = vmin/rstep;
  if(smin < 0) smin = smin - ALMOST1_DIV_5;
  if(vmax < vmin) smin += ALMOST1_DIV_5;	
  *rmin = (int)(5 * smin) / 5. ;
  rmin_old = (int)(smin) ;
  *nn0 = (int)((*rmin - rmin_old) * 5) ;
  if(*nn0 <= 0) *nn0 = - *nn0 ;
  else *nn0 = 5 - *nn0 ;
  *rmin *= rstep ;
  use1 = fabs(rstep);
	
  rstep = (vdif > 0) ? use1 : -use1;
  *xvmin = vmin - vdif * 1.e-5;
  *xvmax = vmax + vdif * 1.e-5;
	
  *n12 = (6 + 1) * (5 + 1);
	
  return (rstep / 5.);
}


static void ComputeTicks(float xMin, float xMax, float yMin, float yMax)
{
  double xvmin, xvmax;
  double value, rstep2, rmin;
  int j, n12, nn, nn0;

  /* The x-axis ticks */
  rstep2 = g_stsize(yMin, yMax, &n12, &xvmin, &xvmax, &rmin, &nn0);
	
  nYLongTicks = -1;
  nYShortTicks = -1;
	
  for(value = rmin, nn = nn0, j = 0; j < n12; j++) {
    if(((value-xvmax)*(value-xvmin)) <= 0) {
      if(fabs(value) < 1.e-3*MAX(fabs(xvmax),fabs(xvmin))) value = 0;

      /* Long tick */
      if(nn == 0) {	
        nYLongTicks++;
        theYLongTicks[nYLongTicks] = value;
      }
      
      /* Short tick */
      nYShortTicks++;
	  theYShortTicks[nYShortTicks] = value;  
    }
	  
	if(nn == 0) nn = 5;
    nn--;	  
    value = rmin + (j + 1) *rstep2; 
  }


  /* The y-axis ticks */  		
  rstep2 = g_stsize(xMin, xMax, &n12, &xvmin, &xvmax, &rmin, &nn0);

  nXLongTicks = -1;
  nXShortTicks = -1;  	
	
  for(value = rmin, nn = nn0, j = 0; j < n12; j++) {
    if((value-xvmax)*(value-xvmin) <= 0) {
      if(fabs(value) < 1.e-3*MAX(fabs(xvmax),fabs(xvmin))) value = 0;
      
      /* Long tick */
	  if(nn == 0) {	
        nXLongTicks++;
        theXLongTicks[nXLongTicks] = value;
      }

      /* Short tick */	    
      nXShortTicks++;
	  theXShortTicks[nXShortTicks] = value;  	    
    }
	  
	if(nn == 0) nn = 5;
    nn--;
    value = rmin+(j+1)*rstep2;
  }	
}



static void DrawAxis_(WINDOW win,int x, int y, int w, int h, float xMin, float xMax, float yMin, float yMax, 
                      char *xText,char * yText,char * titleText,int pixelMargin, char flagTicksIn, char flagFrame,char flagReverse)
{
    int xAxis,yAxis,xAxis1,yAxis1,yLongTick,yLongTick1,yShortTick,yShortTick1,xLongTick,xLongTick1,xShortTick,xShortTick1,xStrTick,yStrTick,x0,y0;
	int i;
	char tempStr[80];
	int hstrMode,vstrMode;
	
	
    /*
	 * Draw in the x-axis
	 */

    /* Draw the axis line */
    if (flagReverse & YFlagReverse) {
      yAxis = y+h-1+pixelMargin;
      yAxis1 = y-pixelMargin;
    }
    else {
      yAxis = y-pixelMargin;
      yAxis1 = y+h-1+pixelMargin;
    }
	WDrawLine((GOBJECT) win,x-pixelMargin, yAxis, x-1+w+pixelMargin, yAxis);
	if (flagFrame) 	WDrawLine((GOBJECT) win,x-pixelMargin, yAxis1, x-1+w+pixelMargin, yAxis1);

	
	/* Draw the long ticks and label them */
	if (flagReverse & YFlagReverse) {
	  vstrMode = VPositionUpStr;
	  if (!flagTicksIn) {
	    yLongTick = yAxis+2*PXTICK;
	    yLongTick1 = yAxis1-2*PXTICK;
	    yStrTick = yLongTick + 2;
	    yShortTick = yAxis+PXTICK;
	    yShortTick1 = yAxis1-PXTICK;
	  }
	  else {
	    yLongTick = yAxis-2*PXTICK;
	    yLongTick1 = yAxis1+2*PXTICK;
	    yStrTick = yAxis + 2;
	    yShortTick = yAxis-PXTICK;
	    yShortTick1 = yAxis1+PXTICK;
	  }
	} 
	else {
	  vstrMode = VPositionBaseStr;
	  if (!flagTicksIn) {
	    yLongTick = yAxis-2*PXTICK;
	    yLongTick1 = yAxis1+2*PXTICK;
	    yStrTick = yLongTick - 2;
	    yShortTick = yAxis-PXTICK;
	    yShortTick1 = yAxis1+PXTICK;
	  }
	  else {
	    yLongTick = yAxis+2*PXTICK;
	    yLongTick1 = yAxis1-2*PXTICK;
	    yStrTick = yAxis - 2;
	    yShortTick = yAxis+PXTICK;
	    yShortTick1 = yAxis1-PXTICK;
	  }
	}
	for (i = 0; i < nXLongTicks + 1; i++){
      sprintf(tempStr, "%.8g", theXLongTicks[i]);
	  if (flagReverse & XFlagReverse) x0 = (int) ((xMax-theXLongTicks[i]) *(w-1)/(xMax-xMin) + x + 0.5);
      else x0 = (int) ((theXLongTicks[i] - xMin) *(w-1)/(xMax-xMin) + x + 0.5);
	    
	  /* Draw the string */
	  WDrawString((GOBJECT) win,tempStr,HPositionMiddleNStr,x0,vstrMode,yStrTick);
	    
	  /* And the tick */
	  WDrawLine((GOBJECT) win,x0,yLongTick,x0,yAxis);
	  if (flagFrame) WDrawLine((GOBJECT) win,x0,yLongTick1,x0,yAxis1);
	}
	
	/* draw the short ticks */
	for (i = 0; i < nXShortTicks + 1; i++){
	  if (flagReverse & XFlagReverse) x0 = (int) ((xMax-theXShortTicks[i]) *(w-1)/(xMax-xMin) + x + 0.5);
      else x0 = (int) ((theXShortTicks[i] - xMin) *(w-1)/(xMax-xMin) + x + 0.5);
	  WDrawLine((GOBJECT) win,x0,yShortTick,x0,yAxis);
	  if (flagFrame) WDrawLine((GOBJECT) win,x0,yShortTick1,x0,yAxis1);
	}

	/* Label the x axis */
    if (xText) {
	  if (flagReverse & YFlagReverse) y0 = yStrTick+4;
	  else y0 = yStrTick-4;
	  if (flagReverse & XFlagReverse) {
	    hstrMode = HPositionLeftStr;
	    x0 = x +10;
	  }
	  else {
	    hstrMode = HPositionRightNStr;
	    x0 = x+w -10;
	  }
	  WDrawString((GOBJECT) win,xText,hstrMode,x0,vstrMode,y0); 
	}


    /*
	 * Draw in the y-axis
	 */

    /* Draw the axis line */
    if (flagReverse & XFlagReverse) {
      xAxis = x+w-1+pixelMargin;
      xAxis1 = x-pixelMargin;
    }
    else {
      xAxis = x-pixelMargin;
      xAxis1 = x+w-1+pixelMargin;
    }
	WDrawLine((GOBJECT) win,xAxis,y-pixelMargin, xAxis, y+h-1+pixelMargin);
	if (flagFrame) 	WDrawLine((GOBJECT) win,xAxis1,y-pixelMargin, xAxis1, y+h-1+pixelMargin);

	/* Draw the long ticks and label them */
	if (flagReverse & XFlagReverse) {
	  hstrMode = HPositionLeftStr;
	  if (!flagTicksIn) {
	    xLongTick = xAxis+2*PXTICK;
	    xShortTick = xAxis+PXTICK;
	    xLongTick1 = xAxis1-2*PXTICK;
	    xShortTick1 = xAxis1-PXTICK;
	  }
	  else {
	    xLongTick = xAxis-2*PXTICK;
	    xShortTick = xAxis-PXTICK;
	    xLongTick1 = xAxis1+2*PXTICK;
	    xShortTick1 = xAxis1+PXTICK;
      }	  
	} 
	else {
	  hstrMode = HPositionRightNStr;
	  if (!flagTicksIn) {
	    xLongTick = xAxis-2*PXTICK;
	    xShortTick = xAxis-PXTICK;
	    xLongTick1 = xAxis1+2*PXTICK;
	    xShortTick1 = xAxis1+PXTICK;
	  }
	  else {
	    xLongTick = xAxis+2*PXTICK;
	    xShortTick = xAxis+PXTICK;
	    xLongTick1 = xAxis1-2*PXTICK;
	    xShortTick1 = xAxis1-PXTICK;
	  }
	}
	for (i = 0; i < nYLongTicks + 1; i++){
     
      sprintf(tempStr, "%.8g", theYLongTicks[i]);
	 
	  if (!flagTicksIn) {
	    if (flagReverse & XFlagReverse) xStrTick = xLongTick + 2;
	    else xStrTick = xLongTick - 2;
	  } 
	  else {
	    if (flagReverse & XFlagReverse) xStrTick = xAxis + 2;
	    else xStrTick = xAxis - 2;
      }
           
	  if (flagReverse & YFlagReverse) y0 = (int) ((yMax-theYLongTicks[i]) *(h-1)/(yMax-yMin) + y + 0.5);
      else y0 = (int) ((theYLongTicks[i] - yMin) *(h-1)/(yMax-yMin) + y + 0.5);
	  yStrTick = y0;	
	    
	  /* Draw the string */
	  WDrawString((GOBJECT) win,tempStr,hstrMode,xStrTick,VPositionMiddleUpStr, yStrTick);
	    
	  /* And the tick */
	  WDrawLine((GOBJECT) win,xLongTick,y0,xAxis,y0);
	 if (flagFrame) WDrawLine((GOBJECT) win,xLongTick1,y0,xAxis1,y0);
	}
	
	/* draw the short ticks */
	for (i = 0; i < nYShortTicks + 1; i++){
	  if (flagReverse & YFlagReverse) y0 = (int) ((yMax-theYShortTicks[i]) *(h-1)/(yMax-yMin) + y + 0.5);
      else y0 = (int) ((theYShortTicks[i] - yMin) *(h-1)/(yMax-yMin) + y + 0.5);
	  WDrawLine((GOBJECT) win,xShortTick,y0,xAxis,y0);
	  if (flagFrame) WDrawLine((GOBJECT) win,xShortTick1,y0,xAxis1,y0);
	}
	
	
	/* Label the y axis */
    if (yText) {
	  if (flagReverse & XFlagReverse) x0 = xStrTick+2;
	  else x0 = xStrTick-2;
	  if (flagReverse & YFlagReverse) {
	    vstrMode = VPositionUpStr;
	    y0 = y + 10;
	  }
	  else {
	    vstrMode = VPositionDownStr;
	    y0 = y+h - 10;
	  }
	  WDrawString((GOBJECT) win,yText,hstrMode,x0,vstrMode,y0); 
	}

	
	/*
	 * Now do the title
	 */
	
    if (titleText) {
      if (flagReverse & YFlagReverse) y0 = y-5;
      else y0 = y+h+5;	
	  WDrawString((GOBJECT) win,titleText,HPositionMiddleNStr,x+w/2,VPositionMiddleStr, y0); 
	}
}



void DrawAxis(GOBJECT o, float x, float y, float dx, float dy, float xMin, float xMax, float yMin, float yMax,
              char *xTitle, char *yTitle, char *title, int pixelMargin, char flagTicksIn, char flagFrame,char flagReverse)
{
  int i,j,m,n;
  WINDOW win = GetWin(o);
  
  if (win->flagHide) return;
  if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");

  Local2GlobalRect(o,x,y,dx,dy,NormalRect,&i,&j,&m,&n);
    
  ComputeTicks(xMin,xMax,yMin,yMax);
  DrawAxis_(win, i,j,m,n,xMin,xMax,yMin,yMax,xTitle,yTitle,title,pixelMargin,flagTicksIn,flagFrame,flagReverse);
}

 
 /*
  * Function to take care of the options of the C_Draw command
  *    It sets the color, the pensize, the linestyle, the clipRect and the pen mode
  *    If argv == NULL then it resets everything
  * This function must be called twice : one time for setting the graphic attributes and the
  * second time for resetting them.
  * Both times it must be called with the SAME arguments for clipWin, clipX, clipY, clipW, clipH.
  * It returns YES if the flag '-n' is set otherwise it returns NO.
  */

#define FlagSizeIsInPixel 256
#define FlagFill 512
#define FlagCentered 1024


static unsigned long DoDrawOptions(WINDOW win, GOBJECT o, char **argv,RectType *rectType,WINDOW *clipWin,int *clipX,int *clipY, int *clipW, int *clipH)
{
  char *str;
  int pen;
  unsigned long flag;
  unsigned long color;
  int x,y,w,h,l,t,r,b;
  FONT font;
  VALUE val;
  
  flag = 0;
  *rectType = NormalRect;
  
  /* Set the graphic attributes */
  if (argv != NULL) {
    
    *clipWin = NULL;
    
    while (*argv != NULL) {

      if (!strcmp(*argv,"-pen")) {
        argv++;
        argv = ParseArgv(argv,tINT,&pen,-1);
        if (pen <= 0) Errorf("DoDrawOptions() : Bad pen size '%d'",pen);
        WSetPenSize(win,pen);
        continue;
      }
  
      if (!strcmp(*argv,"-line")) {
        argv++;
        argv = ParseArgv(argv,tSTR,&str,-1);
        if (!strcmp(str,"solid")) WSetLineStyle(win,LinePlain);
        else if (!strcmp(str,"dash")) WSetLineStyle(win,LineDash);
        else Errorf("DoDrawOptions() : Bad line style '%s'",str);
        continue;
      }

      if (!strcmp(*argv,"-mode")) {
        argv++;
        argv = ParseArgv(argv,tSTR,&str,-1);
        if (!strcmp(str,"normal")) WSetPenMode(win,PenPlain);
        else if (!strcmp(str,"inverse")) WSetPenMode(win,PenInverse);
        else Errorf("DoDrawOptions() : Bad pen mode '%s'",str);
        continue;
      }

      if (!strcmp(*argv,"-color")) {
        argv++;
        argv = ParseArgv(argv,tCOLOR,&color,-1);
        WSetColor(win,color);
        continue;
      }

      if (!strcmp(*argv,"-font")) {
        argv++;
        argv = ParseArgv(argv,tFONT,&font,-1);
        WSetFont(win,font);
        continue;
      }

      if (!strcmp(*argv,"-clip")) {
        argv++;
        WGetClipRect(clipWin,clipX,clipY,clipW,clipH);
        x = 0; y = 0; w = INT_MAX/2; h = INT_MAX/2;
        GetVisibleRect(o,&x,&y,&w,&h);
        WSetClipRect(win,x,y,w,h);
        continue;
      }
  
      if (!strcmp(*argv,"-pixel")) {
        argv++;
        flag = flag | FlagSizeIsInPixel;
        continue;
      }

      if (!strcmp(*argv,"-centered")) {
        argv++;
        flag = flag | FlagCentered;
        continue;
      }
      
      if (!strcmp(*argv,"-rectType")) {
        argv++;
        ParseArgv(argv,tVAL,&val,-1);
        if (GetTypeValue(val) == strType) {
          argv++;
          str = CastValue(val,STRVALUE)->str;
          if (!strcmp(str,"normal")) *rectType = NormalRect;
          else if (!strcmp(str,"large")) *rectType = LargeRect;
          else if (!strcmp(str,"small")) *rectType = SmallRect;
          else Errorf("Bad value '%s' for -recType",str);
        }
        else {
          argv = ParseArgv(argv,tINT,&l,tINT,&t,tINT,&r,tINT,&b,-1);
          rectType->left = l;
          rectType->right = r;
          rectType->top = t;
          rectType->bottom = b;
        }
        continue;
      }

      if (!strcmp(*argv,"-fill")) {
        argv++;
        flag = flag | FlagFill;
        continue;
      }

 
      Errorf("DoDrawOptions() : Bad option '%s'",*argv);
    }
    
    return(flag);
  }

  /* or Unset them */
  WSetLineStyle(win,o->lineStyle);
  WSetPenSize(win,o->penSize);
  WSetColor(win,o->fgColor);
  WSetFont(win,o->font);
  WSetPenMode(win,o->penMode);
  if (*clipWin != NULL) WSetClipRect(*clipWin,*clipX,*clipY,*clipW,*clipH);

  return(NO);
} 

/*   
 * General Command to Draw different things on a gobjects
 */
 
void C_Draw (char **argv)
{
  static float oldX,oldY;
  static GOBJECT oldObj = NULL;
  GOBJECT o,o1;
  GLIST list;
  GCLASS class;
  WINDOW w;
  char *action,name[50];
  float x,y,dx,dy,x1,y1;
  float xMin,xMax,yMin,yMax;
  char *str,*title,*xlabel,*ylabel;
  char flagTicksIn,flagFrame,flagReverse;
  int pixelMargin;  
  unsigned long flag;
  RectType rectType;
  WINDOW clipWin;
  int clipX,clipY,clipW,clipH;
  char flagClip;
  char *hModeStr,*vModeStr;
  int hMode,vMode;
  
  argv = ParseArgv(argv,tWORD,&action,tGOBJECT,&o,-1);

  /* Should we display it ? */
  for (o1 = o;o1 !=NULL; o1 = (GOBJECT) o1->father) {
    if (o1->flagHide) return;
  }
  
  /* Set some default graphic attributes */  
  w = GetWin(o);
  WSetLineStyle(w,o->lineStyle);
  WSetPenSize(w,o->penSize);
  WSetColor(w,o->fgColor);
  WSetPenMode(w,o->penMode);

  /* rect */
  if (!strcmp(action,"rect")) {
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,tFLOAT,&dx,tFLOAT,&dy,-1);
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    if (flag & FlagCentered) {
      if (flag & FlagFill) WFillCenteredRect(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel);    
      else  WDrawCenteredRect(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel);    
    }
    else {
      if (flag & FlagFill) WFillRect(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel, rectType);    
      else  WDrawRect(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel, rectType);    
    }
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldObj = NULL;
    return;
  }  

  /* ellipse */
  if (!strcmp(action,"ellipse")) {
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,tFLOAT,&dx,tFLOAT,&dy,-1);
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    if (flag & FlagCentered) {
      if (flag & FlagFill) WFillCenteredEllipse(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel);    
      else  WDrawCenteredEllipse(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel);    
    }
    else {
      if (flag & FlagFill) WFillEllipse(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel,rectType);    
      else  WDrawEllipse(o,x,y,dx,dy,(flag & FlagSizeIsInPixel)==FlagSizeIsInPixel, rectType);    
    }
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldObj = NULL;
    return;
  }  

  /* cross */
  if (!strcmp(action,"cross")) {
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,tFLOAT,&dx,-1);
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    WDrawCenteredCross(o,x,y,dx,YES);    
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldObj = NULL;
    return;
  }  

  /* line */
  if (!strcmp(action,"line")) {
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,tFLOAT,&x1,tFLOAT,&y1,-1);
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    WDrawLine(o,x,y,x1,y1);    
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldX = x1;
    oldY = y1;
    oldObj = o;
    return;
  }  

  /* Draw a lineto */
  if (!strcmp(action,"lineto")) {
    if (o != oldObj) Errorf("No former point in the same gobject to draw a line from");
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,-1);
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    WDrawLine(o,oldX,oldY,x,y);    
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldX = x;
    oldY = y;
    oldObj = o;
    return;
  }  
    
  /* Draw a point */
  if (!strcmp(action,"point")) {
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,-1);
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    WDrawPoint(o,x,y);    
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldX = x;
    oldY = y;
    oldObj = o;
    return;
  }  

  /* Draw a string */
  if (!strcmp(action,"string")) {
    argv = ParseArgv(argv,tSTR,&str,-1);

    if (ParseFloat_(*argv,0.0,&x)) {
      hMode = HPositionLeftStr;
      argv++;
    }
    else {
      argv = ParseArgv(argv,tWORD,&hModeStr,tFLOAT,&x,-1);
      if (!strcmp(hModeStr,"left")) hMode = HPositionLeftStr;
      else if (!strcmp(hModeStr,"right1")) hMode = HPositionRight1Str;
      else if (!strcmp(hModeStr,"rightN") || !strcmp(hModeStr,"right")) hMode = HPositionRightNStr;
      else if (!strcmp(hModeStr,"middle1")) hMode = HPositionMiddle1Str;
      else if (!strcmp(hModeStr,"middleN") || !strcmp(hModeStr,"middle")) hMode = HPositionMiddleNStr;
      else Errorf("Bad horizontal string position mode '%s'",hModeStr);
    }
    
    if (ParseFloat_(*argv,0.0,&y)) {
      vMode = VPositionBaseStr;
      argv++;
    }
    else {
      argv = ParseArgv(argv,tWORD,&vModeStr,tFLOAT,&y,-1);    
      if (!strcmp(vModeStr,"up")) vMode = VPositionUpStr;
      else if (!strcmp(vModeStr,"down")) vMode = VPositionDownStr;
      else if (!strcmp(vModeStr,"base")) vMode = VPositionBaseStr;
      else if (!strcmp(vModeStr,"middle")) vMode = VPositionMiddleStr;
      else if (!strcmp(vModeStr,"middleUp")) vMode = VPositionMiddleUpStr;
      else Errorf("Bad vertical string position mode '%s'",vModeStr);
    }
    
    flag = DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    WDrawString(o,str,hMode,x,vMode,y);    
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldObj = NULL;
    return;
  }
  
  /* axis */
  flagReverse = YFlagReverse;
  if (!strcmp(action,"axis")) {
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,tFLOAT,&dx,tFLOAT,&dy,tFLOAT,&xMin,tFLOAT, &xMax,tFLOAT, &yMin,tFLOAT, &yMax,-1);
    xlabel = ylabel = title = NULL;
    pixelMargin = 0;
    flagTicksIn = flagFrame = NO;
    while (*argv != NULL) {

/*      if (!strcmp(*argv,"-title")) {
        argv++;
        argv = ParseArgv(argv,tSTR,&title,-1);
        continue;
      }
      if (!strcmp(*argv,"-xlabel")) {
        argv++;
        argv = ParseArgv(argv,tSTR,&xlabel,-1);
        continue;
      }

      if (!strcmp(*argv,"-ylabel")) {
        argv++;
        argv = ParseArgv(argv,tSTR,&ylabel,-1);
        continue;
      }
*/
      if (!strcmp(*argv,"-margin")) {
        argv++;
        argv = ParseArgv(argv,tINT,&pixelMargin,-1);
        continue;
      }

      if (!strcmp(*argv,"-reverse")) {
        argv++;
        argv = ParseArgv(argv,tSTR,&str,-1);
        flagReverse = 0;
        if (!strcmp(str,"xy")) flagReverse = YFlagReverse + XFlagReverse;
        else if (!strcmp(str,"y")) flagReverse = YFlagReverse;
        else if (!strcmp(str,"x")) flagReverse = XFlagReverse;
        else if (!strcmp(str,"none")) flagReverse = 0;
        else Errorf("Bad value '%s' for option '-reverse'",str);
        continue;
      }

      if (!strcmp(*argv,"-in")) {
        argv++;
        flagTicksIn = YES;
        continue;
      }

      if (!strcmp(*argv,"-frame")) {
        argv++;
        flagFrame = YES;
        continue;
      }
      
      break;
    }
    
    DoDrawOptions(w,o,argv,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    DrawAxis(o,x,y,dx,dy,xMin,xMax,yMin,yMax,xlabel,ylabel,title,pixelMargin,flagTicksIn,flagFrame,flagReverse);
    DoDrawOptions(w,o,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    oldObj = NULL;
    return;
  } 
  
  /* Draw a gobject (without adding it) */
  if (!strcmp(action,"gobject")) {  
    argv = ParseArgv(argv,tWORD,&str,-1);
    if (!IsGList(o)) Errorf("The first argument should be a glist not just a gobject");
    list = (GLIST) o;
    class = (GCLASS) GetElemHashTable(theGClasses,str);    
    if (class == NULL) Errorf("'%s' does not correspond to a gclass",str);
    sprintf(name,"_%d_",clock());
    WGetClipRect(&clipWin,&clipX,&clipY,&clipW,&clipH);
    flagClip = NO;
    if (!strcmp(*argv,"-clip")) {
      argv++;
      flagClip = YES;
    }
    o = NewGObject(class,list,name,argv,NO);
    if (clipWin && clipWin == GetWin((GOBJECT) list) && !flagClip) DrawGObject(o,clipX,clipY,clipW,clipH,YES);
    else DrawWholeGObject(o,YES);
    o->flagHide = YES;
    DeleteGObject(o);
    DoDrawOptions(w,(GOBJECT) w,NULL,&rectType,&clipWin,&clipX,&clipY,&clipW,&clipH);
    return;
  } 
  
  Errorf("Unknown action '%s'",action);
}




/********************************
 *
 * Managing Strings
 *
 ********************************/

/*
 *
 * Main routine for Drawing a string or getting its bounding rectangle (if rx != NULL)
 *
 */
 
 /* Pb : si postscript sans graphique */
 
static void _WDrawStringOrGetBoundRect(GOBJECT o,FONT font,char *str, char hPositionMode, float x, char vPositionMode,  float y, float *rx, float *ry, float *rw, float *rh)
{
  int i0,j0,i,j,ri1,rj1,ri,rj,dj;
  WINDOW win;
  char *str1,*str2;
  int nLines,n;
  int vSize;
  int ascent,descent,interline;
  int strWidth,maxStrWidth;
    
  if (o == NULL && rx == NULL) Errorf("WDrawStringOrGetBoundRect() : Do not know whether you want to getthe BoundRect or draw the string");
  
  /* If not getting the boundrect then we must test whether the object is visible or not */  
  if (rx == NULL) {
    if (flagInvisible) return;
    win = GetWin(o);
    if (win->flagHide) return;
    if (win->frame == (FRAME) NULL && !PSMode) Errorf("Weird error");
    if (str == NULL) return;
    if (!GraphicMode && !PSMode) return;
  }
  
  /* Get local coordinates */
  if (o != NULL) Local2Global(o,x,y,&i0,&j0);
  else {
    i0 = (int) (x+.5); 
    j0 = (int) (y+.5); 
  }
  
  /* Let us first compute the number of lines */
  nLines = 1;
  str1 = str;
  while (1) {
    str2 = strchr(str1,'\n');
    if (str2 == NULL) break;
    str1 = str2+1;
    nLines++;
  }
  
  /* Then we compute the vertical size of the whole block */
  ascent = font->ascent;
  descent = font->descent;
  interline = font->interline;
  vSize = nLines*(ascent+descent+interline) -interline;
  
  /* In case HPositionMiddle1Str or HPositionRight1Str we must compute the maximum string width */
  if (hPositionMode == HPositionMiddle1Str || hPositionMode == HPositionRight1Str) {
    maxStrWidth = -1;
    str1 = str;
    while (1) {
      str2 = strchr(str1,'\n');
      if (str2 != NULL) *str2 = '\0';
      strWidth = XXGetStringWidth(font,str1);
      if ((strWidth > maxStrWidth) && PSMode) PSMaxString(str1);
      maxStrWidth = MAX(maxStrWidth,strWidth);
      if (str2 == NULL) break;
      *str2 = '\n';
      str1 = str2+1;
    }
  }
  
  /* Then we loop on all the lines */
  str1 = str;
  n = 0;
  ri = rj= INT_MAX/2;
  ri1 = rj1 = -INT_MAX/2;
  while (1) {
    n++;
    str2 = strchr(str1,'\n');
    if (str2 != NULL) *str2 = '\0';
    
    if (PSMode && rx == NULL) {
      dj = 0;
      if (vPositionMode == VPositionDownStr) dj = vSize-(ascent+descent);
      else if (vPositionMode == VPositionMiddleStr || vPositionMode == VPositionMiddleUpStr) dj = (vSize-ascent-descent)/2;
      PSDrawString(str1,hPositionMode,i0,vPositionMode,j0-dj+(n-1)*(ascent+descent+interline));
      if (str2 == NULL) break;
      *str2 = '\n';
      str1 = str2+1;
      continue;
    }
        
    /* Let us fix the vertical position */
    switch (vPositionMode) {
    
      case VPositionBaseStr: j = j0+(n-1)*(ascent+descent+interline); break;

      case VPositionUpStr:
        j = j0+ascent+1+(n-1)*(ascent+descent+interline);
        break;

      case VPositionDownStr:
        j = j0-vSize+ascent+(n-1)*(ascent+descent+interline);
        break;

      case VPositionMiddleStr:
        j = j0-vSize/2+ascent+1+(n-1)*(ascent+descent+interline);
        break;

      case VPositionMiddleUpStr:
        j = j0-vSize/2+descent/2+ascent+1+(n-1)*(ascent+descent+interline);
        break;
    }
 
    /* Let us then fix the horizontal position */
    switch (hPositionMode) {
  
      /* Justify left */
      case HPositionLeftStr : 
        i = i0; 
        strWidth = XXGetStringWidth(font,str1);
        break;

      /* Justify right each line separately */
      case HPositionRightNStr : 
        strWidth = XXGetStringWidth(font,str1);
        i = i0-strWidth;
        break;

      /* Justify right all the lines as a block */
      case HPositionRight1Str : 
        strWidth = XXGetStringWidth(font,str1);
        i = i0-maxStrWidth;
        break;

      /* Justify middle each line separately */
      case HPositionMiddleNStr : 
        strWidth = XXGetStringWidth(font,str1);
        i = i0-strWidth/2;
        break;

      /* Justify middle all the lines as a block */
      case HPositionMiddle1Str : 
        i = i0-maxStrWidth/2;
        break;
    }
        
    /* Computing the bounding rect if necessary */
    if (rx != NULL) {
      ri = MIN(i,ri); 
      rj = MIN(rj,j-ascent-1);
      rj1 = MAX(rj1,j+descent);
      if (hPositionMode == HPositionMiddle1Str) ri1 = i0+maxStrWidth/2;
      else if (hPositionMode == HPositionRight1Str) ri1 = i+maxStrWidth;
      else ri1 = MAX(ri1,i+strWidth); 
    }
    
    /* Draw the string if asked */
    if (rx == NULL) XXDrawString(win->frame,i,j,str1);
  
    if (str2 == NULL) break;
    *str2 = '\n';
    str1 = str2+1;
  }
  
  if (rx != NULL) {
    if (o !=NULL) Global2LocalRect(o,ri,rj,ri1-ri+1,rj1-rj+1,rx,ry,rw,rh,NormalRect);
    else {
      *rx = ri; 
      *ry = rj;
      *rw = ri1-ri+1;
      *rh = rj1-rj+1;
    }
  }
  
}


void WDrawString(GOBJECT o,char *str, char hPositionMode, float x, char vPositionMode, float y)
{
  _WDrawStringOrGetBoundRect(o,currentFont,str,hPositionMode,x,vPositionMode,y,NULL,NULL,NULL,NULL);
}

void WGetBoundRectString(GOBJECT o,FONT font,char *str, char hPositionMode, float x, char vPositionMode, float y,float *rx, float *ry, float *rw, float *rh)
{
  _WDrawStringOrGetBoundRect(o,font, str,hPositionMode,x,vPositionMode,y,rx,ry,rw,rh);
}



/********************************
 *
 * Managing Fonts
 *
 ********************************/

HASHTABLE theFontHashTable;
FONT defaultFont = NULL;


/* 
 * Miscellenaous funtions
 */
 
/* Get the name of the styme from 'style' */ 
static char *GetFontStyleNameFromStyle(unsigned char style)
{
  static char str[255];
  
  str[0] ='\0';
  
  if (style & FontBold) strcat(str,"bold");
  if (style & FontItalic) {
    if (*str == '\0') strcat(str,"italic");
    else strcat(str,"Italic");
  }
  if (style == FontPlain) strcat(str,"plain");
  
  return(str);
}

/* Reverse the above function */
static unsigned char GetFontStyleFromStyleName(char *str)
{
  unsigned char style;

  if (!strcmp(str,"plain")) style = FontPlain;
  else if (!strcmp(str,"bold")) style = FontBold;
  else if (!strcmp(str,"italic")) style = FontItalic;
  else if (!strcmp(str,"boldItalic")) style = FontItalic | FontBold;
  else {
    SetErrorf("GetFontStyleFromStyleName() : Bad style name '%s'",str);
    return(FontError);
  }
  
  return(style);
}

/* Get the name of the font given the 'name', 'size' and 'style' */
char *GetFontFullName(char *name, int size, unsigned char style)
{
  static char str[200];

  sprintf(str,"%s-%d-%s",name,size,GetFontStyleNameFromStyle(style));
  return(str);
}

/* Reverse of the above procedure */
static char GetFontNameSizeStyle(char *str, char **name, int *size, unsigned char *style)
{
  static char str0[200];
  char *str1,*str2,*str3,*tempStr;
  char *endStr1,*endStr2,*endStr3;
  char *endp;
  long l;
  
  if (str == NULL) Errorf("GetFontNameSizeStyle() : Weird error");
  
  /* Looking for the first string */
  str1 = tempStr = str;
  tempStr = str;
  while(*tempStr != '\0' && *tempStr != '-') tempStr++;
  endStr1 = tempStr;
  if (*str1 == *endStr1) {
    SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
    return(NO);
  }
 
  /* Looking for the second string */
  if (*endStr1 == '\0') str2 = endStr2 = NULL;
  else {
    str2 = tempStr = endStr1+1;
    while(*tempStr != '\0' && *tempStr != '-') tempStr++;
    endStr2 = tempStr;
    if (*str2 == *endStr2) {
      SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
      return(NO);
    }
  }

  /* Looking for the third string */
  if (str2 == NULL || *endStr2 == '\0') str3 = endStr3 = NULL;
  else {
    str3 = tempStr = endStr2+1;
    while(*tempStr != '\0' && *tempStr != '-') tempStr++;
    endStr3 = tempStr;
    if (*str3 == *endStr3) {
      SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
      return(NO);
    }
    /* If some more strings are left then error */
    if (*endStr3 != '\0') {
      SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'",str);
      return(NO);
    }
  }
  
 
  /*
   * Case there is just one string 
   */
  if (str2 == NULL) {
      
    /* Try to read a size */  
    l = strtol(str1,&endp,0);
    if (*endp == '\0') {
      if (l<=0) {
        SetErrorf("GetFontNameSizeStyle() : Bad font size '%d'",l);
        return(NO);
      }
      if (currentFont == NULL) {
        SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'. You must specify the name of the font.",str);
        return(NO);
      }
      *size = l;
      *style = FontPlain;
      *name = currentFont->fontName;
      return(YES);
    }

    /* Or just a type */  
    *style = GetFontStyleFromStyleName(str1);
    if (*style != FontError) {
      if (currentFont == NULL) {
        SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'. You must specify the name of the font and its size.",str);
        return(NO);
      }
      *size = currentFont->size;
      *name = currentFont->fontName;
      return(YES);
    }

    /* Or just a name */  
    if (currentFont == NULL) {
      SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'. You must specify the size of the font",str);
      return(NO);
    }
    *size = currentFont->size;
    *style = FontPlain;
    strcpy(str0,str1);
    *name = str0;
    return(YES);
  }
    
    
  /*
   * Case there are only two strings : string-string 
   */
  if (str3 == NULL) {

    /* Try to read a size */  
    l = strtol(str1,&endp,0);
    if (*endp == '-') {
      if (l<=0) {
        SetErrorf("GetFontNameSizeStyle() : Bad font size '%d'",l);
        return(NO);
      }
      if (currentFont == NULL) {
        SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'. You must specify the name of the font.",str);
        return(NO);
      }
      /* And the type */
      *style = GetFontStyleFromStyleName(str2);
      if (*style != FontError) {
        *size = l;
        *name = currentFont->fontName;
        return(YES);
      }
      SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
      return(NO);
    }
 
    /* Or a font name */  
    *endStr1 = '\0';
    strcpy(str0,str1);
    *name = str0;
    *endStr1 = '-';
    
    /* And a size */
    l = strtol(str2,&endp,0);
    if (*endp == '\0') {
      if (l<=0) {
        SetErrorf("GetFontNameSizeStyle() : Bad font size '%d'",l);
        return(NO);
      }
      *style = FontPlain;
      *size = l;
      return(YES);
    }

    /* Or a style */
    *style = GetFontStyleFromStyleName(str2);
    if (*style != FontError) {
      if (currentFont == NULL) {
        SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'. You must specify the size of the font.",str);
        return(NO);
      }
      *size = currentFont->size;
      return(YES);
    }
    SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
    return(NO);
  }
  
  /*
   * Case there are three strings : string-string-string 
   */
  
  /* Read a font name */  
  *endStr1 = '\0';
  strcpy(str0,str1);
  *name = str0;
  *endStr1 = '-';

  /* And a size */
  l = strtol(str2,&endp,0);
  if (*endp == '-') {
    if (l<=0) {
      SetErrorf("GetFontNameSizeStyle() : Bad font size '%d'",l);
      return(NO);
    }
  }
  else {
    SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
    return(NO);
  }
  *size = l;
  
  /* And a style */
  *style = GetFontStyleFromStyleName(str3);
  if (*style == FontError) {
    SetErrorf("GetFontNameSizeStyle() : Bad font name '%s'.",str);
    return(NO);
  }
  
  return(YES);
}   
    
/* Is the font exist ? */
char WExistFont(char *str,FONTID *id)
{ 
  char *name;
  int size;
  unsigned char style;

  if (GetFontNameSizeStyle(str,&name,&size,&style) == NO) Errorf1("");;
  
  if (GraphicMode) return(XXExistFont(name,size,style,id));
  else return(1);
}


/*
 * Allocation and desallocation of a font
 */

static FONT NewFont(FONTID id,char *name,int size, unsigned char style)
{
  FONT font;

  font = (FONT) Malloc(sizeof (struct font));
  
  font->fontName = CopyStr(name);
  font->size = size;
  font->style = style;

  font->postscriptName = NULL;
  font->postscriptSize = -1;
  
  font->id = id;
  font->nRef = 0;
  
  font->name = CopyStr(GetFontFullName(name,size,style));
  
  if (GraphicMode) XXGetFontInfo(font,&(font->ascent),&(font->descent),&(font->interline));
  else font->ascent = font->descent = font->interline = 8;

  AddElemHashTable(theFontHashTable,(AHASHELEM) font);

  return(font);
}


/* Returns the font or create it if it does not exist or returns NULL */
static FONT CGetFont(char *str)
{
  FONT f;
  FONTID id;
  char *name;
  int size;
  unsigned char style;
  char *str1;
  
  f = (FONT) GetElemHashTable(theFontHashTable,str);
  if (f != NULL) return (f);

  if (GetFontNameSizeStyle(str, &name,&size, &style) == NO) {
    SetErrorf("CGetFont() : Bad font syntax '%s'",str);
    return(NULL);
  }
  
  str1 = GetFontFullName(name,size,style);
  f = (FONT) GetElemHashTable(theFontHashTable,str1);
  if (f != NULL) return (f);
  
  if (!WExistFont(str,&id)) {
    SetErrorf("CGetFont() : font '%s' does not exist",str);
    return(NULL);
  }
  
  return(NewFont(id,name,size,style));  
}


/*
 * Parse a font name
 */
 
char ParseFont_(char *arg, FONT def, FONT *f)
{ 
  char *str;
   
  *f = def;
  if (arg == NULL) {
    SetErrorf("ParseFont_() : NULL string cannot be converted to a font");
    return(NO);
  }

  if (*arg == '\0') {
    SetErrorf("ParseFont_() : empty string cannot be converted to a font");
    return(NO);
  }
  
  if (ParseStr_(arg,NULL,&str) == NO) return(NO);
  
  if (!strcmp("default",str)) {
    *f = defaultFont;
    return(YES);
  }

  *f = CGetFont(str);
  if (*f == NULL) {
    *f = def;
    return(NO);
  }
  
  return(YES);
}

void ParseFont(char *arg,  FONT *f) 
{
  if (ParseFont_(arg,0,f) == NO) Errorf1("");
}



/* Change the Font of the text */
void WSetFont(WINDOW win,FONT font)
{ 
  if (win->flagHide) return;
  currentFont = font;
  if (PSMode) PSSetFont(font);
  if (GraphicMode) XXSetFont(win->frame,font);
}

/* Get the current Font of the text */
FONT WGetFont(WINDOW win)
{ 
  if (win->flagHide) return(NULL);
  return(currentFont);
}
  
/*
 * The font command 
 */
 
void C_Font(char **argv)
{
  char *action,*str,*str1,*str2,*hModeStr,*vModeStr;
  float x,y,rx,rw,ry,rh;
  int hMode,vMode;
  GOBJECT o;
  FONT font;
  int size;
  unsigned char style;
  char flagSize,flagStyle;
  LISTV lv;
    
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if (!strcmp(action,"exist")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    if (WExistFont(str,NULL)) SetResultInt(1);
    else SetResultInt(0);
    return;
  }

  if (!strcmp(action,"list")) {
    argv = ParseArgv(argv,tSTR_,"*",&str,tSTR_,"*",&str1,tSTR_,"*",&str2,0);
    if (!strcmp(str1,"*")) flagSize = NO;
    else {
      ParseInt(str1,&size);
      flagSize = YES;
    }
    if (!strcmp(str2,"*")) flagStyle = NO;
    else {
      style = GetFontStyleFromStyleName(str2);
      if (style == FontError) Errorf1("");
      flagStyle = YES;
    }
    XXFontMatch(str,flagSize,size,flagStyle,style);
    return;
  }

  if (!strcmp(action,"default")) {
    argv = ParseArgv(argv,tFONT_,NULL,&font,0);
    if (font != NULL) currentFont = defaultFont = font;
    SetResultStr(defaultFont->name);
    return;
  }

  argv = ParseArgv(argv,tFONT,&font,-1);

  if (!strcmp(action,"info")) {
    NoMoreArgs(argv);
    lv = TNewListv();
    AppendInt2Listv(lv,font->ascent);
    AppendInt2Listv(lv,font->descent);
    AppendInt2Listv(lv,font->interline);
    SetResultValue(lv);
    return;
  }

  if (!strcmp(action,"size")) {
    NoMoreArgs(argv);
    SetResultInt(font->size);
    return;
  }

  if (!strcmp(action,"style")) {
    NoMoreArgs(argv);
    SetResultStr(GetFontStyleNameFromStyle(font->style));
    return;
  }
 
  if (!strcmp(action,"name")) {
    NoMoreArgs(argv);
    SetResultStr(font->fontName);
    return;
  }

  if (!strcmp(action,"rect")) {
    argv = ParseArgv(argv,tSTR,&str,tWORD,&hModeStr,tFLOAT,&x,tWORD,&vModeStr,tFLOAT,&y,tGOBJECT_,NULL,&o,0);
    if (!strcmp(hModeStr,"left")) hMode = HPositionLeftStr;
    else if (!strcmp(hModeStr,"right1")) hMode = HPositionRight1Str;
    else if (!strcmp(hModeStr,"rightN") || !strcmp(hModeStr,"right")) hMode = HPositionRightNStr;
    else if (!strcmp(hModeStr,"middle1")) hMode = HPositionMiddle1Str;
    else if (!strcmp(hModeStr,"middleN") || !strcmp(hModeStr,"middle")) hMode = HPositionMiddleNStr;
    else Errorf("Bad horizontal string position mode '%s'",hModeStr);
    if (!strcmp(vModeStr,"up")) vMode = VPositionUpStr;
    else if (!strcmp(vModeStr,"down")) vMode = VPositionDownStr;
    else if (!strcmp(vModeStr,"base")) vMode = VPositionBaseStr;
    else if (!strcmp(vModeStr,"middle")) vMode = VPositionMiddleStr;
    else if (!strcmp(vModeStr,"middleUp")) vMode = VPositionMiddleUpStr;
    else Errorf("Bad vertical string position mode '%s'",vModeStr);
    WGetBoundRectString(o,font,str,hMode,x,vMode,y,&rx,&ry,&rw,&rh);
    lv = TNewListv();
    AppendFloat2Listv(lv,rx);
    AppendFloat2Listv(lv,ry);
    AppendFloat2Listv(lv,rw);
    AppendFloat2Listv(lv,rh);
    SetResultValue(lv);    
    return;
  }
  
  Errorf("Bad action '%s'",action);
}
  

/*
 * Initializations
 */

static void WInitFont(void) 
{
  FONTID id;
  char *name;
  int size;
  
  theFontHashTable = NewHashTable(40);
  
  if (GraphicMode) name = XXGetDefaultFont(&id,&size);  
  else name = "fixed";
  
  defaultFont =  NewFont(id,name,size,FontPlain);
  currentFont = defaultFont;
}



/* ANSI changed */

