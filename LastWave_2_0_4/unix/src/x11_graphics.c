/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   U n i x   2 . 0 . 4                               */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
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



/****************************************************************************/
/*                                                                          */
/*  x11_graphics.c      The Machine dependent Graphic functions             */
/*                                                                          */    
/****************************************************************************/


#include "computer.h"

#ifdef LASTWAVE_X11_GRAPHICS

#include "x11_graphics.h"
#include "lastwave.h"

#include <unistd.h>
#include <time.h>


#define BORDER_WIDTH  2  /* Width of the window border */


/*************************************/
/*      Some useful variables        */
/*************************************/

static Display 	*theDisplay=NULL;        /* -- The display                    */
static int		theScreen;	 /* -- Which screen on the display    */
static int		theDepth;	 /* -- Number of color planes         */
static Colormap	theColormap;	         /* -- default System color map       */
static Cursor		theCursor;	 /* -- Application program's cursor   */
static GC              theGC;            /* -- Graphic content to be used     */
static Visual          *theVisual;       /* -- Visual color structure         */
static int 	       theVisualClass;   /* -- the class of the visual        */

static Colormap myColormap; /* the colormap used for all the windows */
static XColor fgDefaultColor,bgDefaultColor;
static int planeMask;

static unsigned long	theBlackPixel;	 /* -- "Black" color                  */
static unsigned long	theWhitePixel;	 /* -- "White" color                  */
  

static FRAME theEventWindow; /* The window used by the terminal to send key events */
extern int fildes[]; /* The pipe the two processes are communicating through */

static Atom WM_DELETE_WINDOW;
static Atom WM_PROTOCOLS;

#define DEFAULT_DELAY .3
static float _resizeDelay = DEFAULT_DELAY;
static float resizeDelay = DEFAULT_DELAY*CLOCKS_PER_SEC;


/***************************************/
/* Create the event window             */
/***************************************/

static void XXNewEventWindow(void)
{
  XSetWindowAttributes	theWindowAttributes;
  unsigned long	        theWindowMask;
  XSizeHints		theSizeHints;
  FILE *stream;
  int temp;
  unsigned long window;
  FRAME frame;

  theWindowAttributes.override_redirect = False; 

  theWindowMask = CWOverrideRedirect;

  theEventWindow = XCreateWindow(theDisplay,
			RootWindow( theDisplay , theScreen ),
			1, 1,	
			10, 10,	
			1,
			theDepth,
			InputOutput, 
			CopyFromParent,
			theWindowMask,
			&theWindowAttributes );
  
  XSelectInput(theDisplay,theEventWindow,KeyPressMask); 

  XFlush(theDisplay);

  /* Send the id of the Event window to the terminal process */
  window = theEventWindow;
  write(fildes[1],&window,sizeof(unsigned long));
}


/***************************************/
/*     Initialization of X11           */
/***************************************/

void _XXInfo(void)
{
  Printf("%s version %d of the X Window System, X%d R%d\n",
     ServerVendor(theDisplay),VendorRelease(theDisplay),
     ProtocolVersion(theDisplay),
     ProtocolRevision(theDisplay)
  );

  Printf("Color plane depth is %d\n",theDepth);
  Printf("The Display %s\n",(char *) XDisplayName(NULL));
}


void XXOpenGraphics(void)
{
  int stup;
  int nitem;
  XVisualInfo *pvinfo,vinfo;
  char *str;
  unsigned long mask;
  XGCValues gcValues;
  XColor color1,color2;
  XWindowAttributes	theWindowAttributes;

  theDisplay = XOpenDisplay(NULL);
  if ( theDisplay == NULL ) {
    printf("Cannot establish a connection to the X Server\n");
    exit(0);
  }

  /* Default Parameters */
  theScreen = XDefaultScreen( theDisplay );  
  theDepth = XDefaultDepth( theDisplay, theScreen );
  theColormap = XDefaultColormap( theDisplay, theScreen );
  theGC = XDefaultGC(theDisplay, theScreen);

  theVisual = XDefaultVisual(theDisplay, theScreen);

  theBlackPixel = XBlackPixel( theDisplay, theScreen );
  theWhitePixel = XWhitePixel( theDisplay, theScreen );

  /* Create a cursor for all the program's windows */
  theCursor = XCreateFontCursor( theDisplay, XC_crosshair );

  /* Default parameters for the colormap */
  myColormap = theColormap;

  /* Get the visual type */
  vinfo.visualid = XVisualIDFromVisual(theVisual);
  pvinfo = XGetVisualInfo(theDisplay,VisualIDMask,&vinfo,&nitem);
  theVisualClass = pvinfo->class;
  XFree(pvinfo);

  /* Reading bgcolor from default file if possible otherwise it will be black */
/*  str = XGetDefault(theDisplay,"xterm","Background");
  if (str[0] == '\0') str = XGetDefault(theDisplay,"xterm","background");
  if (str[0] == '\0') str = XGetDefault(theDisplay,"","background");
  if (str[0] != '\0') XParseColor(theDisplay,theColormap,str,&bgDefaultColor); 
  else */ bgDefaultColor.red = bgDefaultColor.blue = bgDefaultColor.green = 65535;

  /* Reading fgcolor from default file if possible otherwise it will be white */
/*  str = XGetDefault(theDisplay,"xterm","Foreground");
  if (str[0] == '\0') str = XGetDefault(theDisplay,"xterm","foreground");
  if (str[0] == '\0') str = XGetDefault(theDisplay,"","foreground");
  if (str[0] != '\0') XParseColor(theDisplay,theColormap,str,&fgDefaultColor); 
  else */ fgDefaultColor.red = fgDefaultColor.blue = fgDefaultColor.green = 0;

  /* Add SubstructureNotifyMask to event mask of the root window in order to receive deleteNotify */
  XGetWindowAttributes(theDisplay,RootWindow(theDisplay,theScreen),&theWindowAttributes);
  /*    XSelectInput(theDisplay,RootWindow(theDisplay,theScreen),theWindowAttributes.your_event_mask | SubstructureNotifyMask);    */
 
  if (GraphicMode) {
    XXNewEventWindow();
    WM_DELETE_WINDOW = XInternAtom(theDisplay,"WM_DELETE_WINDOW",False);
    WM_PROTOCOLS = XInternAtom(theDisplay,"WM_PROTOCOLS",False);
  }

}

void XXCloseGraphics(void)
{
  if (GraphicMode) XDestroyWindow(theDisplay,theEventWindow);
}


/*************************************************************************/
/* This routine avoids flushing the buffer after each graphics operation */
/*************************************************************************/

static int flagStartStop = NO;

static void _XXFlushIfNeeded(FRAME frame)
{
}

void XXFlush(void)
{
  XFlush(theDisplay);
}


/*************************************************************************/
/*                                                                       */
/*            The Following functions are called by the program          */
/*                                                                       */
/*************************************************************************/

/* Set the style of the line */
void XXSetLineStyle(FRAME frame,int flag)
{
  XGCValues values;

  if (flag == LinePlain) values.line_style = LineSolid;
  else values.line_style = LineOnOffDash;

  XChangeGC(theDisplay,theGC,GCLineStyle,&values);

}

/* Change the size of the pen of the Graphic Port */
static int penSize;
void XXSetPenSize(FRAME frame,int size)
{ 
  XGCValues values;

  values.line_width = size;
  penSize = size;
  XChangeGC(theDisplay,theGC,GCLineWidth,&values); 

}

/* Set the pen mode of the Graphic Port */
void XXSetPenMode(FRAME frame,int mode)
{ 
  XGCValues values;

  if (mode == PenInverse) {
    values.function = GXinvert;
    XChangeGC(theDisplay,theGC,GCFunction,&values);
    if (planeMask != 0) XSetPlaneMask(theDisplay,theGC,planeMask);
  }
  else {
    values.function = GXcopy;
    XChangeGC(theDisplay,theGC,GCFunction,&values);
    XSetPlaneMask(theDisplay,theGC,AllPlanes);
  }
}


/* Draw a line in the graphic port */
void XXDrawLine(FRAME frame,int x,int y,int x1,int y1)
{
  XDrawLine(theDisplay,frame,theGC,x,y,x1,y1);
  _XXFlushIfNeeded(frame);
}

/* Draw a point in the Graphic Port */
void XXDrawPoint(FRAME frame,int x,int y)
{
  XDrawLine(theDisplay,frame,theGC,x,y,x+penSize,y);
  _XXFlushIfNeeded(frame);
}

/* Draw an ellipse in a rect */
void XXDrawEllipse(FRAME frame,int x,int y,int dx, int dy)
{
  XDrawArc(theDisplay,frame,theGC,x,y,dx-1,dy-1,0,360*64);
  _XXFlushIfNeeded(frame);
}

/* Fill an ellipse in a rect */
void XXFillEllipse(FRAME frame,int x,int y,int dx, int dy)
{
  XFillArc(theDisplay,frame,theGC,x,y,dx,dy,0,360*64);
  _XXFlushIfNeeded(frame);
}


/* Draw a rectangle */
void XXDrawRect(FRAME frame,int x,int y,int dx,int dy)
{
  XDrawRectangle(theDisplay,frame,theGC,x,y,dx-1,dy-1);
  _XXFlushIfNeeded(frame);
}

/* Fill a rectangle */
void XXFillRect(FRAME frame,int x,int y,int dx,int dy)

{
  XFillRectangle(theDisplay,frame,theGC,x,y,dx,dy);
  _XXFlushIfNeeded(frame);
} 

/* Set the clip rect */
void XXSetClipRect(FRAME frame, int x, int y, int w, int h)
{
  XRectangle rect;

  rect.x = x;
  rect.y = y;
  rect.width = w; /* -1 ?? */
  rect.height = h;
  
  XSetClipRectangles(theDisplay,theGC,0,0,&rect,1,Unsorted);
}


/*
 * Managing Pixmaps
 */

static char *theData = NULL;
static int nCol = 0;
static int nRow = 0;

void XXAllocPixMap(int w,int h,unsigned char **pData,int *pRowBytes)
{
  if (theData != NULL) {
    free(theData);
    theData = NULL;
  } 

  switch(theDepth) {
    case 8 :  
      *pData = (unsigned char *) calloc(w*h,sizeof(char)); 
      *pRowBytes = w;
      break;
    case 16 : 
      *pData = (unsigned char *) calloc(2*w*h,sizeof(char)); 
      *pRowBytes = 2*w;
      break;
/*    case 24 : 
      *pData = (unsigned char *) calloc(3*w*h,sizeof(char)); 
      *pRowBytes = 3*w;
      break;    */
    case 32 : case 24 :
      *pData = (unsigned char *) calloc(4*w*h,sizeof(char)); 
      *pRowBytes = 4*w;
      break;
    default : 
      Errorf("XXAllocPixMap() : Sorry this function has not been impleted for screen depth %d",theDepth);
  }

  theData = (char *) *pData;
  nCol = w;
  nRow = h;
}

void XXDeletePixMap(void)
{  
}
  
/* Display an image in a frame */
void XXDisplayPixMap(FRAME frame,int winX,int winY)
{
  int i,j;
  XImage *ximage;
  int bitmap_pad;

  switch(theDepth) {
    case 8 :  
      bitmap_pad = 8; 
      break;
    case 16 :  
      bitmap_pad = 16; 
      break;
    case 24 : case 32 :  
      bitmap_pad = 32; 
      break;
    default :
      Errorf("XXDisplayPixMap() : Sorry this function has not been impleted for screen depth %d",theDepth);
  }

  ximage = XCreateImage(theDisplay,theVisual,theDepth,ZPixmap,0,theData,nCol,nRow,bitmap_pad,0);

  if (ximage == NULL) Errorf("Allocation failed for 'ximage'");

  XPutImage(theDisplay,frame,theGC,ximage,0,0,winX,winY,nCol,nRow);
  _XXFlushIfNeeded(frame);
  XDestroyImage(ximage);

  nCol = 0; 
  nRow = 0;
  theData = NULL;
}

char XXIsDisplayBLittle(void)
{
  return(ImageByteOrder(theDisplay)==LSBFirst);
}

/********************************************************/
/*                Other functions                       */
/********************************************************/

/* Delete a window */
void XXDeleteFrame(FRAME frame)
{
  if (frame == 0) return;
  XDestroyWindow(theDisplay,frame);
  _XXFlushIfNeeded(frame);
}

/* Create a window */
FRAME XXNewFrame(char *title,int x, int y, int w,int h)
{
  int flagConnect;
  XSetWindowAttributes	theWindowAttributes;
  XSizeHints		theSizeHints;
  unsigned long	        theWindowMask;
  FRAME frame;
  unsigned long x1,x2,x3,x4;
  XEvent event;
  unsigned long xEventMask;
  int n;

  theWindowAttributes.border_pixel      = theWhitePixel;
  theWindowAttributes.cursor            = theCursor;    
  theWindowAttributes.override_redirect = False; 

  theWindowMask = CWBorderPixel | CWCursor | CWOverrideRedirect;

  frame = XCreateWindow(theDisplay,
			RootWindow( theDisplay , theScreen ),
			x, y,	
			w, h,	
			BORDER_WIDTH,
			theDepth,
			InputOutput, 
			CopyFromParent,
			theWindowMask,
			&theWindowAttributes );

   XStoreName( theDisplay, frame, title );


  theSizeHints.flags      = USPosition | USSize;
  theSizeHints.x          = x;
  theSizeHints.y          = y;
  theSizeHints.width      = w;
  theSizeHints.height     = h;
  XSetNormalHints( theDisplay, frame, &theSizeHints ); 

  XSelectInput(theDisplay,frame,ButtonPressMask | ButtonReleaseMask | KeyPressMask | 
	       KeyReleaseMask | ExposureMask | StructureNotifyMask | 
	       EnterWindowMask | LeaveWindowMask | PointerMotionMask);


  XSetWindowColormap(theDisplay,frame,myColormap); 
  
  XSetWMProtocols(theDisplay,frame,&WM_DELETE_WINDOW,1);

  XMapWindow( theDisplay, frame); 

  XClearWindow(theDisplay, frame); 
  XFlush( theDisplay );

  /* We must take out the corresponding map event on the event queue */
  while (1) {
    if (!XCheckWindowEvent(theDisplay,frame, StructureNotifyMask,&event)) continue;
    if (event.type == MapNotify) break;
  }

  /* We must take out the corresponding expose event on the event queue */
  while (1) {
    if (!XCheckWindowEvent(theDisplay,frame, ExposureMask,&event)) continue;
    if (event.type == Expose) break;
  }

  return(frame);
}

/* Quand c'est juste la position ou le titre qui change ne pas faire d'expose ni configure */
/* Quand la taille change avec enlever juste le configure et garder l'expose */
/* Change the size the position and the title of a window */
void XXChangeFrame(FRAME frame,char *title,int x,int y,int w, int h)
{
  XWindowAttributes att;
  int x1,y1;
  unsigned int w1,h1,b,depth;
  XEvent event;
  Window root;

  XGetGeometry(theDisplay,frame,&root,&x1,&y1,&w1,&h1,&b,&depth);

//Printf("UNIX: %d %d %d %d %d %d %d %d\n",x,y,w,h,x1,y1,w1,h1);

  if (w == w1 && h1 == h) {
    if (title != NULL) XStoreName( theDisplay, frame, title );
    XMoveResizeWindow(theDisplay,frame,x,y,(unsigned int) w, (unsigned int) h);
  }
  else {
    if (title != NULL) XStoreName( theDisplay, frame, title );
    XMoveResizeWindow(theDisplay,frame,x,y,(unsigned int) w, (unsigned int) h);
    /* We must take out the corresponding expose event on the event queue */
    while (1) {
      if (!XCheckWindowEvent(theDisplay,frame, ExposureMask,&event)) continue;
      if (event.type == Expose) break;
    }
  }
}

FRAME XXGetFrontFrame(void)
{
}

void XXFrontFrame(FRAME frame)
{
  unsigned int mask;
  XWindowChanges values;
  /* XRaiseWindow(theDisplay,frame); */
  mask = CWStackMode;
  values.stack_mode = Above;
  XConfigureWindow(theDisplay,frame,mask,&values);
  XSetInputFocus(theDisplay,frame,RevertToNone,CurrentTime); 
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
  XAutoRepeatOn(theDisplay);
}

/* Turn the autorepeat off */
void XXAutoRepeatOff(void)
{
  XAutoRepeatOff(theDisplay);
}



void XXGetNextEvent(EVENT event,int flagWait)
{
  XWindowAttributes att;
  XEvent xevent;
  XButtonEvent *pButtonEvent;
  XConfigureEvent *pConfigureEvent;
  XExposeEvent *pExposeEvent;
  XCrossingEvent *pCrossingEvent;
  XMotionEvent *pMotionEvent;
  XKeyEvent *pKeyEvent;
  XComposeStatus status;
  KeySym keysym;
  int i,n,x1,y1,x2,y2;
  unsigned int w1,h1,b,depth;
  char key,flagNoEvent;
  unsigned long xEventMask;
  Window win1,win2,root;
  static unsigned int buttonDown = 0;
  static unsigned long buttonDownModifiers= 0;
  static WINDOW buttonDownWindow = NULL;
  WINDOW window;
  extern char UnionRect1(int x,int y,int w,int h,int *x1,int *y1,int *w1,int *h1);
  clock_t now;

  event->type = NoEvent;
  event->key = 0;
  event->button = NoButton;

  flagNoEvent = YES;

  while (1) {

    if (!flagWait && XPending(theDisplay) == 0) return;
    XNextEvent(theDisplay,&xevent);

    flagNoEvent = NO;

    if (xevent.type == UnmapNotify || xevent.type == DestroyNotify) continue;

    if (xevent.xkey.window == theEventWindow) {
      switch(xevent.type) {
	  
	/* The terminal window sent a key press event */
      case KeyPress: 
	pKeyEvent = (XKeyEvent *) &xevent;
	event->object = NULL;
	switch(pKeyEvent->keycode) {
	case 0x8:case 0x7f: event->key = DeleteKC; break;
	case 0x4: event->key = EofKC; break;
	case 0x9: event->key = TabKC; break;
	case '\n': case '\r' : event->key = NewlineKC; break;
	case 27: event->key = EscapeKC; break;
	default :  event->key = pKeyEvent->keycode;
	}
	event->x = event->y = 0;
	event->type = KeyDown;
	return;
      }
      continue;
    }
    
    //    Printf("Event %d %d\n",xevent.xkey.window,xevent.type); 

    window = Frame2Window(xevent.xkey.window);
    if (window == NULL) continue;
    event->object = (GOBJECT) window;

    /* Different types of event */
    switch(xevent.type) {
    case ButtonRelease:
      pButtonEvent = (XButtonEvent *) &xevent;
      event->i = pButtonEvent->x;
      event->j = pButtonEvent->y;
      event->button = (pButtonEvent->button == Button1 ? LeftButton :
	     (pButtonEvent->button == Button2 ? MiddleButton : RightButton));
      if (buttonDown == 0 || buttonDown != event->button) return;
      event->type = ButtonUp;
      event->button += buttonDownModifiers;
      buttonDown = 0;
      XUngrabPointer(theDisplay,CurrentTime); 
      /*            Printf("ButtonRelease %d %d %d %d\n",event->i,event->j,event->button,event->key);  */
      return;

    case ButtonPress:
      if (buttonDown != 0) return;
      pButtonEvent = (XButtonEvent *) &xevent;
      event->type = ButtonDown;
      event->i = pButtonEvent->x;
      event->j = pButtonEvent->y;
      buttonDown = (pButtonEvent->button == Button1 ? LeftButton :
	     (pButtonEvent->button == Button2 ? MiddleButton : RightButton));
      buttonDownWindow = window;
      buttonDownModifiers = 0;
      if (pButtonEvent->state & ShiftMask) buttonDownModifiers += ModShift;
      if (pButtonEvent->state & ControlMask) buttonDownModifiers += ModCtrl;
      if (pButtonEvent->state & Mod1Mask) buttonDownModifiers += ModOpt;
      XGrabPointer(theDisplay,window->frame,False,PointerMotionMask | ButtonReleaseMask,GrabModeAsync,GrabModeAsync,
		   window->frame,None,CurrentTime); 
      event->button = buttonDown + buttonDownModifiers;
      /*      Printf("ButtonPress %d %d\n",event->i,event->j);    */
      return;

    case Expose:
      pExposeEvent =  (XExposeEvent *) &xevent;
      event->type = Draw;
      event->i = pExposeEvent->x;
      event->j = pExposeEvent->y;
      event->m = pExposeEvent->width;
      event->n = pExposeEvent->height;
      while (1) {
	if (XCheckWindowEvent(theDisplay,window->frame, ExposureMask, &xevent) == False) break;
	pExposeEvent = (XExposeEvent *) &xevent;
	UnionRect1(pExposeEvent->x,pExposeEvent->y,pExposeEvent->width,pExposeEvent->height,&(event->i),&(event->j),&(event->m),&(event->n));
      } 
	
//            Printf("Expose %d %d %d %d\n",event->i,event->j,event->m,event->n);      
      return;

    case ConfigureNotify:
      pConfigureEvent = (XConfigureEvent *) &xevent;

      // Take borders into account for position
      XGetGeometry(theDisplay,window->frame,&root,&x1,&y1,&w1,&h1,&b,&depth);

      window->x = pConfigureEvent->x-x1;
      window->y = pConfigureEvent->y-y1;
      event->i = window->x;
      event->j = window->y;
      event->m = pConfigureEvent->width;
      event->n = pConfigureEvent->height;

  //    Printf("configure %d %d %d %d\n",event->i,event->j,event->m,event->n); 

      if (event->m == window->w && event->n == window->h) return;
      event->type = Resize;
      now = clock();
      while(clock()<=now+resizeDelay) {
	while (1) {
	  if (XCheckWindowEvent(theDisplay,window->frame, StructureNotifyMask | ExposureMask, &xevent) == False) break;
	  if (xevent.type == Expose) continue;
	  now = clock();
	  pConfigureEvent = (XConfigureEvent *) &xevent;
	  event->m = pConfigureEvent->width;
	  event->n = pConfigureEvent->height;
	} 
      }
      /* removing the spurious expose events */
      while (XCheckWindowEvent(theDisplay,window->frame, ExposureMask, &xevent) != False); 

      //                       Printf("configure\n");  
      return;

    case MotionNotify:
      /* Get the last motion event */
      while(XCheckTypedWindowEvent(theDisplay,window->frame,MotionNotify,&xevent) == True);

      pMotionEvent = (XMotionEvent *) &xevent;
      event->i = pMotionEvent->x;
      event->j = pMotionEvent->y;
      event->button = NoButton;
      if (buttonDown != 0) event->button = buttonDown+buttonDownModifiers;
      else {
	    if (pMotionEvent->state & ShiftMask) event->button += ModShift;
	    if (pMotionEvent->state & ControlMask) event->button += ModCtrl;
	    if (pMotionEvent->state & Mod1Mask) event->button += ModOpt;
      }
      event->type = MouseMotion;
      return;

    case ClientMessage:

      if (xevent.xclient.message_type == WM_PROTOCOLS && 
	  xevent.xclient.format == 32 &&
	  xevent.xclient.data.l[0] == WM_DELETE_WINDOW) {
	event->type = Del;
      }
      return;

    case FocusIn:
      /* XSetInputFocus(theDisplay,theTerminalWindow,RevertToNone,CurrentTime); */
      /* printf("FocusIn\n"); */
      return;
      
    case FocusOut:
    /*  printf("FocusOut\n"); */
      return;

    case EnterNotify:
      pCrossingEvent = (XCrossingEvent *) &xevent;
      event->type = Enter;
      event->i = pCrossingEvent->x;
      event->j = pCrossingEvent->y;
      /*      Printf("enter\n");   */
      return;

    case LeaveNotify:
      event->type = Leave;
      /*      Printf("leave\n");   */
      return;

    case KeyPress:case KeyRelease:
      pKeyEvent = (XKeyEvent *) &xevent;
      if (pKeyEvent->type == KeyPress) event->type = KeyDown;
      else event->type = KeyUp;

      event->key = 0;
      if (pKeyEvent->state & ShiftMask) event->key += ModShift;
      if (pKeyEvent->state & ControlMask) event->key += ModCtrl;
      if (pKeyEvent->state & Mod1Mask) event->key += ModOpt;

      XLookupString(pKeyEvent,&key,1,&keysym,&status);

      switch(keysym) {

      case XK_Shift_L:case XK_Shift_R:
      case XK_Control_L:case XK_Control_R:
      case XK_Alt_L:case XK_Alt_R: event->type = NoEvent; return;
      case XK_Escape: event->key += EscapeKC; break;
      case XK_Left: event->key += LeftKC; break;
      case XK_Right: event->key += RightKC; break;
      case XK_Up: event->key += UpKC; break;
      case XK_Down: event->key += DownKC; break;
      case XK_Home: event->key += HomeKC; break;
      case XK_End: event->key += EndKC; break;
      case XK_Clear: event->key += ClearKC; break;
      case XK_Delete:case XK_BackSpace: event->key += DeleteKC; break;
      case XK_Tab: event->key += TabKC; break;
      case XK_F1: event->key += F1KC; break;
      case XK_F2: event->key += F2KC; break;
      case XK_F3: event->key += F3KC; break;
      case XK_F4: event->key += F4KC; break;
      case XK_F5: event->key += F5KC; break;
      case XK_F6: event->key += F6KC; break;
      case XK_F7: event->key += F7KC; break;
      case XK_F8: event->key += F8KC; break;
      case XK_F9: event->key += F9KC; break;
      case XK_F10: event->key += F10KC; break;
      case XK_F11: event->key += F11KC; break;
      case XK_F12: event->key += F12KC; break;
      case XK_F13: event->key += F13KC; break;
      case XK_F14: event->key += F14KC; break;
      case XK_F15: event->key += F15KC; break;
	
      default:
        if (key == '\n' || key == '\r' || (key >= ' ' && key <= '~')) event->key = key;
	else {
	  pKeyEvent->state = 0;
	  XLookupString(pKeyEvent,&key,1,&keysym,&status);
	  if (key == '\n' || key == '\r' || (key >= ' ' && key <= '~')) event->key += key;
	  else {
	    event->type = NoEvent;
	    return;
	  }
	}
      }
      event->i = pKeyEvent->x;
      event->j = pKeyEvent->y;
     if (event->i < 0 || event->j < 0 || event->i >= event->object->w || event->j >= event->object->h) event->type = NoEvent;
/*            printf("KeyPress <%c> %d %d %s\n",key,event->i,event->j,event->object->name); */
      return;

    default: 
	continue;
    }
    
  }
  
  if (flagNoEvent && flagWait) XNextEvent(theDisplay,&xevent);

}


/****************************************/
/*               Color stuff            */
/****************************************/

/* Change the color of the Graphic port */
void XXSetColor(FRAME frame, unsigned long pixel)
{
  XSetForeground(theDisplay,theGC,pixel); 
}


static XColor *xColors = NULL;
static int nXColors = 0;

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
  int    i,j;
  Status stat;
  unsigned long plane_masks[1];
  WINDOW window;

  /*
   * Free any former colormap or color cells allocated
   * during a former call to this function 
   */
  if (myColormap != theColormap)
      XFreeColormap(theDisplay,myColormap);
  else {
    if (nXColors != 0) 
      for(i=0;i<nXColors;i++) XFreeColors(theDisplay,myColormap,&(xColors[i].pixel),1,0);
  }
  if (nXColors != 0) {
    nXColors = 0;
    Free(xColors);
  }

  /* Build a new colormap if necessary */
  if (flagSharedColormap == NO) myColormap = XCopyColormapAndFree(theDisplay,theColormap);
  else myColormap = theColormap;

  /*
   * Case of read/write color cells : GrayScale, PseudoColor or DirectColor screens
   */

  if (theVisualClass == GrayScale || theVisualClass == PseudoColor || theVisualClass == DirectColor) {

    /*
     * Allocating the cells for the colorIndexes[] colors and their 'Inversed' colors
     */
    if (mouseMode == MouseInverse) {
      stat = XAllocColorCells(theDisplay,myColormap,False,plane_masks,0,pixels,nCols);
      if (stat == 0) Errorf(" XXSetColormap() : Sorry, cannot allocate %d colors",nCols);
      planeMask = 0;
    }
    else if ((stat = XAllocColorCells(theDisplay,myColormap,False,plane_masks,1,pixels,nCols)) == 0) {
      PrintfErr("*** Failed to reserve %d colors for inverse cursor mode\n",nCols);
      PrintfErr("*** ---> Assumed to be 0\n");
      mouseMode = MouseInverse;
      stat = XAllocColorCells(theDisplay,myColormap,False,plane_masks,0,pixels,nCols);
      if (stat == 0) Errorf(" XXSetColormap() : Sorry, cannot allocate %d colors",nCols);
      planeMask = 0;
    }
    else planeMask = plane_masks[0];

    /* 
     * We allocate all the colors
     */
    
    if (mouseMode != MouseInverse) nXColors = 2*nCols;
    else nXColors = nCols;
    xColors = Malloc(nXColors*sizeof(XColor));

    for (i=0; i<nCols ; i++) {
      xColors[i].red   = red[i];
      xColors[i].green = green[i];
      xColors[i].blue  = blue[i];
      xColors[i].flags = DoRed | DoGreen | DoBlue;
      xColors[i].pixel = pixels[i];

      if (mouseMode != MouseInverse) {
        if (mouseMode == Mouse1Color) {
          xColors[i+nCols].red   = mouseRed;
	      xColors[i+nCols].green = mouseGreen;
	      xColors[i+nCols].blue  = mouseBlue;
	      xColors[i+nCols].flags = DoRed | DoGreen | DoBlue;
	      xColors[i+nCols].pixel = pixels[i] | plane_masks[0];
        }
        else {
          xColors[i+nCols].red   = mouseRed;
	      xColors[i+nCols].green = mouseGreen;
	      xColors[i+nCols].blue  = mouseBlue;
	      xColors[i+nCols].flags = DoRed | DoGreen | DoBlue;
	      xColors[i+nCols].pixel = pixels[i] | plane_masks[0];
        }
      }
    }
  
    if (nXColors != 0)  XStoreColors(theDisplay,myColormap,xColors,nXColors);
  }

  /*
   * Case of read-only color cells : StaticGray, StaticColor or TrueColor screens
   */

  else {
    
    nXColors = nCols;
    xColors = Malloc(nXColors*sizeof(XColor));
    
    for (i=0; i<nCols ; i++) {
      xColors[i].red   = red[i];
      xColors[i].green = green[i];
      xColors[i].blue  = blue[i];
      xColors[i].flags = DoRed | DoGreen | DoBlue;
      stat = XAllocColor(theDisplay,myColormap,xColors+i);
      pixels[i] = xColors[i].pixel;
    }

    planeMask = 0;
  }

  for(window = theWindows; window != NULL; window = (WINDOW) window->front)
      XSetWindowColormap(theDisplay,window->frame,myColormap);   
   

  return(nXColors);
}


/*
 * Animate one color of the colormap
 */

void XXAnimateColor(unsigned long pixel, unsigned short r, unsigned short g, unsigned short b)
{
  XColor RGB;

  RGB.red   = r;
  RGB.green = g;
  RGB.blue  = b;
  RGB.pixel = pixel;
  RGB.flags = DoRed | DoGreen | DoBlue;

  XStoreColors(theDisplay,myColormap,&RGB,1);
}


/*
 * Returns the number of colors available 
 */
int XXNumOfColors(void)
{
  int nCols = 1<<theDepth;
  unsigned long plane_masks[1];
  unsigned long pixels[256];

  if (theDepth > 8) return(nCols);

  while  (nCols != 0) {
    if (XAllocColorCells(theDisplay,theColormap,False,plane_masks,0,pixels,nCols)) break;
    nCols--;
  }
  if (nCols != 0) XFreeColors(theDisplay,theColormap,pixels,nCols,0);  

  return(nCols+nXColors);
}

/*
 * Returns the depth
 */
int XXDepth(void)
{
  return(theDepth);
}

/*
 * Is the screen BW ?
 */

int XXIsBWScreen(void)
{
  switch(theVisualClass) {
  case GrayScale: case StaticGray: return(YES);
  case PseudoColor: case DirectColor: return(NO);
  case StaticColor: case TrueColor:return(NO);
  }
}

/*
 * Type of the screen
 */

char * XXScreenType(void)
{
  switch(theVisualClass) {
  case GrayScale: return("GrayScale");
  case StaticGray: return("StaticGray");
  case PseudoColor: return("PseudoColor");
  case DirectColor: return("DirectColor");
  case StaticColor: return("StaticColor");
  case TrueColor: return("TrueColor");
  }
}

void XXScreenRect(int *x, int *y, int *w, int *h)
{
  *x = *y = 0;
  *w = DisplayWidth(theDisplay,theScreen);
  *h = DisplayHeight(theDisplay,theScreen);
}

/*
 * Get the foreground and background colors of the terminal window 
 */

void XXBgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  *r = bgDefaultColor.red;
  *g = bgDefaultColor.green;
  *b = bgDefaultColor.blue;
}

void XXFgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  *r = fgDefaultColor.red;
  *g = fgDefaultColor.green;
  *b = fgDefaultColor.blue;
}




/***************************************
 *
 * Dealing with strings and fonts
 *
 ***************************************/

/* Draw a string in the graphic port */
void XXDrawString(FRAME frame,int x,int y,char *str)
{
  XDrawString(theDisplay,frame,theGC,x,y,str,strlen(str));
  _XXFlushIfNeeded(frame);
}


char *XXGetDefaultFont(FONTID *fontStruct,int *size) 
{
  static char *str = "fixed";
 
  for (*size=13;*size<=20;(*size)++) {
    if (XXExistFont(str,*size, FontPlain,fontStruct)) break;
  }

  if (*size == 21) Errorf("XXGetDefaultFont() : Cannot localize the X11font 'fixed'.");
 
  return(str);
}


void XXGetFontInfo(FONT font, int *ascent, int *descent, int *interline)
{
  XFontStruct *font_info;

  font_info = font->id;
  if (font_info == NULL) Errorf("XXGetFontInfo() : Weird error");
  
  font->ascent = font_info->max_bounds.ascent;
  font->descent = font_info->max_bounds.descent;;
  font->interline = font_info->ascent+font_info->descent-font->ascent-font->descent;
}

int XXGetStringWidth(FONT font, char *text)
{
  XFontStruct *font_info;
  int width;

  font_info = font->id;
  width= XTextWidth(font_info,text,strlen(text));

  return(width);
}

void XXSetFont(FRAME frame,FONT font)
{
  XGCValues values;

  values.font = font->id->fid;

  XChangeGC(theDisplay,theGC,GCFont,&values);  
}


void XXFontMatch(char *name,char flagSize, int size,char flagStyle,unsigned char style)
{
  char **list,**list1;
  char *weight,c;
  char str[100], str1[50],str2[50],str3[50];
  char *oldName;
  unsigned char oldStyle;
  int oldSize;
  int i,k;
  int n;

  if (flagStyle) {
    if (style & FontBold) weight = "bold";
    else weight = "medium";
  }
  else weight = "*";

  if (flagStyle) {
    if (style & FontItalic) c = 'o';
    else c = 'r';
  } else c = '*';

  /* Get the list of font that match the requirements */
  if (flagSize)   sprintf(str,"-*-%s-%s-%c-normal-*-%d-*-*-*-*-*-*-*",name,weight,c,size);
  else sprintf(str,"-*-%s-%s-%c-normal-*-*-*-*-*-*-*-*-*",name,weight,c);

  list = XListFonts(theDisplay,str,300,&n);

  if (list == NULL) return;

  /* We must return the list of the fonts */
  oldName = NULL;
  list1 = list;
  for (;n!=0;n--,list++) {
 if (sscanf(*list,"-%*[^-]-%[^-]-%[^-]-%c-normal-%*[^-]-%d",str1,str2,&c,&i) != 4 &&
     sscanf(*list,"-%*[^-]-%[^-]-%[^-]-%c-normal--%d",str1,str2,&c,&i) != 4) continue;
    if (c == 'o') style = FontItalic;
    else if (c == 'r') style = FontPlain;
    else continue;
    if (!strcmp("bold",str2)) style |= FontBold;
    else if (strcmp("medium",str2)) continue;
    
    if (oldName != NULL && !strcmp(str1,oldName) && oldStyle == style &&  oldSize==i) continue;
    oldName = str1;
    oldStyle = style;
    oldSize = i;
    AppendListResultStr(GetFontFullName(oldName,i,style));
  }    

  XFreeFontNames(list1);  
}


char XXExistFont(char *name,int size, unsigned char style,FONTID *fontStruct)
{
  char **list;
  char *weight,c;
  char str[200];
  int n;

  if (style & FontBold) weight = "bold";
  else weight = "medium";
  if (style & FontItalic) c = 'o';
  else c = 'r';

  sprintf(str,"-*-%s-%s-%c-normal-*-%d-*-*-*-*-*-*-*",name,weight,c,size);
  list = XListFonts(theDisplay,str,1,&n);

  if (list == NULL) {
    return(NO);
  }

  if (fontStruct == NULL) {
    XFreeFontNames(list);  
    return(YES);
  }

  *fontStruct = XLoadQueryFont(theDisplay,*list);
  XFreeFontNames(list);  

  return(YES);
}

 
void C_System(char **argv)
{
  char *str;
  float f;

  argv = ParseArgv(argv,tWORD,&str,-1);
  
  if (!strcmp(str,"nEvents")) {
    NoMoreArgs(argv);
    SetResultInt(toplevelCur->nEvents+1);
  }
  else if (!strcmp(str,"resizeDelay")) {
    if (*argv == NULL) {
      SetResultFloat(_resizeDelay);
      return;
    }
    argv = ParseArgv(argv,tFLOAT,&f,0);
    if (f <0) Errorf("Delay cannot be negative");
    _resizeDelay = f;
    resizeDelay = _resizeDelay*CLOCKS_PER_SEC;
    return;
  }
  else ErrorUsage();
}


#endif

