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


/********************************************************************/
/*                                                                  */
/*   window_manager.h :               The window manager interface  */
/*                                                                  */
/********************************************************************/




/****************************************************************/
/* The following are all the graphic functions called by the     */
/* program                                                      */
/****************************************************************/


extern void WOpenGraphics(void);  /* Initialization */
extern void WCloseGraphics(void);
extern void WFlush(void);

extern void WSetColor(WINDOW win,unsigned long color); /* Change the color */
extern void WSetLineStyle(WINDOW win,int flag); /* Set the style of the line */
extern void WSetPenSize(WINDOW win,int size);   /* Change the size of the pen */
extern void WSetPenMode(WINDOW win, int mode);


extern void WDrawLine(GOBJECT o,float x,float y,float x1,float y1);  /* Draw a line */
extern void WDrawPoint(GOBJECT o,float x,float y);               /* Draw a point */
extern void WDrawRect(GOBJECT o,float x,float y,float dx,float dy,char flagSizeIsInPixel,RectType rectType);  /* Draw a rectangle */
extern void WFillRect(GOBJECT o,float x,float y,float dx,float dy,char flagSizeIsInPixel,RectType rectType);  /* Fill a rectangle */
extern void WDrawCenteredRect(GOBJECT o,float x,float y,float r1,float r2,char flagSizeIsInPixel);  /* Draw a rectangle */
extern void WFillCenteredRect(GOBJECT o,float x,float y,float r1,float r2,char flagSizeIsInPixel);  /* Fill a rectangle */
extern void WClearRect(GOBJECT o,unsigned long color, float x,float y,float dx,float dy,char flagSizeIsInPixel,RectType rectType);  /* Clear a rectangle */
extern void WDrawEllipse(GOBJECT o,float x,float y,float r1,float r2,char flagSizeIsInPixel,RectType rectType);             /* Draw an ellipse */
extern void WFillEllipse(GOBJECT o,float x,float y,float r1, float r2,char flagSizeIsInPixel,RectType rectType);        /* Fill an ellipse */
extern void WDrawCenteredEllipse(GOBJECT o,float x,float y,float r1,float r2,char flagSizeIsInPixel);             /* Draw an ellipse */
extern void WFillCenteredEllipse(GOBJECT o,float x,float y,float r1, float r2,char flagSizeIsInPixel);        /* Fill an ellipse */
extern void WDrawCenteredCross(GOBJECT o,float x,float y,float r,char flagSizeIsInPixel);         /* Draw a cross */
extern void DrawAxis(GOBJECT o, float x, float y, float dx, float dy, float xMin, float xMax, float yMin, float yMax,
              char *title, char *xTitle, char *yTitle, int pixelMargin, char flagTicksIn, char flagFrame, char flagReverse);
extern void WSetClipRect(WINDOW win, int x, int y, int w, int h);
extern void WGetClipRect(WINDOW *win, int *x, int *y, int *w, int *h);

extern void WGetNextEvent(EVENT event,int flagWait);    /* Return the next window to be refreshed */
extern void WAutoRepeatOn(void);                   /* Turn the autorepeat On */
extern void WAutoRepeatOff(void);                  /* Turn it Off */

extern FRAME WNewFrame(char *title,int x,int y,int w,int h);    /* Create a frame */
extern void WChangeFrame(FRAME frame,char  *title, int x,int y,int w,int h);
extern void WDeleteFrame(FRAME frame);
extern WINDOW Frame2Window(FRAME frame);
extern WINDOW WGetFrontWindow(void);
extern void WFrontWindow(WINDOW win);

extern int WSetColormap(unsigned short red[],unsigned short green[],
                        unsigned short blue[],unsigned long pixels[],int nCols,int flagShared,int mouseMode);

extern void WAnimateColor(unsigned long c, unsigned short r, unsigned short g, unsigned short b);

extern int WNumOfColors(void);                 /* Returns the maximum number of colors */
extern int WDepth(void);                 
extern int WIsBWScreen(void);
extern char *WScreenType(void);
extern void WScreenRect(int *x, int *y, int *w, int *h);

extern void WBgColor(unsigned short *r,unsigned short *g,unsigned short *b);
extern void WFgColor(unsigned short *r,unsigned short *g,unsigned short *b);


extern void WDisplayPixMap(WINDOW win,int winX,int winY); /* Display the current pixmap */
extern void WSetPixelPixMap(int i, int j, unsigned long color);
extern void WInitPixMap(int nRows, int nCols);
extern char WIsDisplayBLittle(void);

/* The pen modes */
enum {
  PenPlain = 0,
  PenInverse
};

/* The line styles */
enum {
  LinePlain = 0,
  LineDash
};

/* Mouse color mode */
enum {
  MouseInverse = 0,
  MouseTransparent,
  Mouse1Color
};




/********************************
 *
 * Managing Fonts and strings
 *
 ********************************/

/*
 * the Font structure 
 */

typedef struct font {

  AHashElemFields;

  FONTID id;
  
  char *fontName;
  int size;
  
  int ascent,descent,interline;
  
  unsigned char style;
  
  char *postscriptName;
  float postscriptSize;
  
} *FONT;

extern HASHTABLE theFontHashTable;
extern FONT defaultFont;

/* The different style of fonts */
enum {
  FontPlain = 0,
  FontItalic = 1,
  FontBold = 2,
  FontError = 4
};

/* Mode for drawing strings */
enum {
  HPositionLeftStr = 0,
  HPositionRightNStr,
  HPositionRight1Str,
  HPositionMiddleNStr,
  HPositionMiddle1Str,
  VPositionUpStr,
  VPositionDownStr,
  VPositionBaseStr,
  VPositionMiddleStr,
  VPositionMiddleUpStr
};
  
  

  
extern void WSetFont(WINDOW win,FONT font);  /* Change the Font of the text */
extern FONT WGetFont(WINDOW win);
extern void WGetBoundRectString(GOBJECT o,FONT font,char *str, char hPositionMode, float x, char vPositionMode, float y,float *rx, float *ry, float *rw, float *rh);
extern void WDrawString(GOBJECT o,char *str, char hPositionMode, float x, char vPositionMode, float y);
extern char WExistFont(char *str,FONTID *id);
extern char *GetFontFullName(char *name, int size, unsigned char style);


/* ANSI changed */
 
  
