/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   M a c i n t o s h   2 . 0 . 1                     */
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
/*  mac_graphics.c      The Machine dependent Graphic functions                */
/*                                                                          */    
/****************************************************************************/

#include "computer.h"

#ifdef LASTWAVE_MAC_GRAPHICS

#include "macwindow.h"
#include "lastwave.h"

#include <ctype.h>
#include <stdarg.h>
#include <float.h>



#include <Controls.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>
#include <Fonts.h>
#include <DiskInit.h>
#include <Script.h>

#include <Traps.h>
#include <Palettes.h>
#include <QDOffscreen.h>
#include <OSUtils.h>
#include <Resources.h>
#include <Sound.h>
#include <ToolUtils.h>

#include <FixMath.h>

#include "MacTextWindow.h"
#include "MacMenu.h"
#include "Mac.h"


/* Some global variables for the graphics */

static GDHandle theDevice;
Str63	AppName;
static PaletteHandle thePalette; /* This is the shared colormap */
RgnHandle grayRgnHandle;         /* For multiple screen drag */
static PixPatHandle theColorPattern; /* This is the current pattern if color */
static unsigned long currentColor;
static Pattern *theBWPattern; /* This is a pointer to the current Black/White pattern*/
static int flagBW; /* is set whenever the current color is black or white */
static int theDepth; /* Depth of the current screen */
static int hasWNE; /* Set if the function WaitNextEvent exists in the Rom */ 
static char str1[255];  /* A very useful string */
static CursHandle iBeamCursorH = NULL;
static GrafPtr curPort;



static char Is3MouseButtons = NO;
static char ForceKeyUp = YES;


/*
 * The Current Terminal Window 
 */
TEXTWINDOW XXTerminalWindow  = NULL;


/*
 * If there is no routine for converting C string to Pascal string we must add our own
 */
 
#ifndef CtoPstr

static Str255 thePascalString;

static void _C2Pstr(char *cstring, Str255 pstring)
{
  unsigned char *p = (unsigned char *) pstring;
  char *c = cstring;
  int l;
  
  strncpy((char *) (p+1),c,254);
  l = strlen(c);
  if (l>255) l = 255;
  
  p[0] = l;  
}

#define CtoPstr(c) (_C2Pstr(c,thePascalString),thePascalString)

#endif

/*
 * If there is no routine for converting Pascal string to C string we must add our own
 */
 
#ifndef PtoCstr

static char theCString[256];

static char *PtoCstr(Str255 pstring)
{
  unsigned char *p = (unsigned char *) pstring;
  int len;
  
  len = p[0];
  
  strncpy(theCString,(char*) (p+1),len);
  theCString[len] = '\0';
  
  return(theCString);
}

#endif

/******************************************************************
 *         Test whether the WaitNextEvent function exists         *
 *         in the Rom (and set the hasWNE flag if yes)            *
 ******************************************************************/

static Boolean	TrapAvailable(SysEnvRec *gMac,short	trapNum, short tType)
{
  return(true);
}


/******************************************************************
 *         Get the depth of the main screen  (set theDepth)       *
 ******************************************************************/

static int GetPixelDepth(GDHandle theDevice) 
{
  PixMapHandle screenPMapH = (**theDevice).gdPMap;
  return((int) ((**screenPMapH).pixelSize));
}


/******************************************************************
 *           Open/Close the Graphics !!!!                         *
 ******************************************************************/
 
void XXOpenGraphics(void)
{	
    SysEnvRec gMac;
	ProcessSerialNumber psn;
	ProcessInfoRec pinfo;
	OSErr err;
  
   	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0L);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	
	MaxApplZone(); 
/*	for (i=0;i<10000;i++) MoreMasters();  */
	
/*	
 * 68K-WARNING :	
 *	FOR 68K USERS replace the last block ockline by the following line 
 *
 */
 
/*  SetApplLimit(GetApplLimit()-16384*8); */


/*	MoreMasters();*/	
	
	/* Test if WaitNextEvent  exist */
	SysEnvirons(1, &gMac);  
	hasWNE = TrapAvailable(&gMac,_WaitNextEvent, ToolTrap);
	
	SysEnvirons(2, &gMac);

    /* Get the main device and get its depth */
	theDevice = GetDeviceList();
    theDepth = GetPixelDepth(theDevice);


   /* Initialize the colormap and pattern */
   thePalette = NULL;
   theColorPattern = NULL;

  /* For multiple screen drag */
   grayRgnHandle = GetGrayRgn();

  /* SetUp the Menus */
  SetupMenus();
  
  /* Get the ibeam cursor */
  iBeamCursorH = GetCursor(iBeamCursor);
  
  err = GetCurrentProcess(&psn);
  pinfo.processName = AppName;
  pinfo.processInfoLength = sizeof(pinfo);
  pinfo.processAppSpec = NULL;
  err = GetProcessInformation(&psn,&pinfo);

  if (pinfo.processName[0] <= 59) {
	pinfo.processName[++(pinfo.processName[0])] = '.';
	pinfo.processName[++(pinfo.processName[0])] = 'o';
	pinfo.processName[++(pinfo.processName[0])] = 'u';
	pinfo.processName[++(pinfo.processName[0])] = 't';
  }

  if (*AppName == 0) BlockMove("\pMT TextWindow", AppName, 17L);
  
  SetOutlinePreferred(false);
		
  /* SetUp the terminal window */
  XXInitTextWindows();
  XXTerminalWindow = NewTextWindow(nRowsDefault,nColsDefault,"Listener",TERMINAL);
  
  GetPort(&curPort);   
  

}

void XXCloseGraphics()
{
}

/************************************************************************/
/************************************************************************/
long myMenuKey(char key)
{
/*	short theMenu = 0;
	short theMenuItem = 0;

	switch (key) {
		case 's': case 'S':
			theMenu = FILEID;
			theMenuItem = FILESAVE;
			break;
		case 'p': case 'P':
			theMenu = FILEID;
			theMenuItem = FILEPRINT;
			break;
		case 'q': case 'Q':
			theMenu = FILEID;
			theMenuItem = FILEQUIT;
			break;
		case 'x': case 'X':
			theMenu = EDITID;
			theMenuItem = EDITCUT;
			break;
		case 'c': case 'C':
			theMenu = EDITID;
			theMenuItem = EDITCOPY;
			break;
		case 'v': case 'V':
			theMenu = EDITID;
			theMenuItem = EDITPASTE;
			break;
		case 'a': case 'A':
			theMenu = EDITID;
			theMenuItem = EDITSELECTALL;
			break;
	}

	return (((long)theMenu << 16) | theMenuItem);
*/
}

/******************************************
 * Draw the Grow box of a window 
 ******************************************/
void DrawGrowBox(WindowPtr theWindow)
{
	Rect aRect;
	short width, height;
	GrafPtr savePort;
    Rect  bigRect =	{-32000, -32000, 32000, 32000};


	GetPort(&savePort);
	SetPort(theWindow);

	width = theWindow->portRect.right - theWindow->portRect.left;
	height = theWindow->portRect.bottom - theWindow->portRect.top;
	
    SetRect(&aRect, width - 15, -1, width, height);
    ClipRect(&aRect);
    DrawGrowIcon(theWindow);
    ClipRect(&bigRect);

	SetPort(savePort);
}				


/*************************************************************************/
/** These two routines avoid saving the graphic port each time          **/
/** If it has already been opened it considers it shouldn't open it again **/
/** It will close it only if it was not open when it tried to open it   **/
/*************************************************************************/

static XXOpenPortIfNeeded(FRAME frame)
{  
  GetPort(&curPort); 
  if (((GrafPtr) frame) == curPort) return; 
  
  SetGWorld((CGrafPtr) frame,theDevice);
}


static XXClosePortIfNeeded(FRAME frame)
{
/*  SetPort(savePortIfNeeded); */
}

void XXScreenRect(int *x, int *y, int *w, int *h)
{
  *x = *y = 0;
  *h = qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - GetMBarHeight() - 24;	/*	screen height ... */
  *w = qd.screenBits.bounds.right - qd.screenBits.bounds.left;
}



/* Set the style of the line */
void XXSetLineStyle(FRAME frame,int flag)
{
  static Pattern thePattern = {0xAA,0x55,0xAA,0x55,0xAA,0x55,0xAA,0x55}; 
/*  static Pattern thePattern = {0xAA,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; */
  XXOpenPortIfNeeded(frame);
  if (flag == LinePlain) PenNormal();
  else PenPat(&thePattern);
  XXClosePortIfNeeded(frame);
}
	
void XXSetPenMode(FRAME frame,int mode)
{ 
  XXOpenPortIfNeeded(frame);
  if (mode == PenInverse) {
    PenMode(patXor);
    TextMode(srcXor);
  }
  else {
    PenMode(patCopy);
    TextMode(srcOr);
  }
  XXClosePortIfNeeded(frame);
}

static void _XXPattern(void); 

/* Change the size of the pen of the Graphic Port */
void XXSetPenSize(FRAME frame,int size)
{
  XXOpenPortIfNeeded(frame);
  PenSize((short) size, (short) size);
  XXClosePortIfNeeded(frame);
}


/* Draw a line in the graphic port */
void XXDrawLine(FRAME frame,int x,int y,int x1,int y1)
{
	XXOpenPortIfNeeded(frame);
	MoveTo((short) x,(short) y);
	LineTo((short) x1,(short) y1);
	XXClosePortIfNeeded(frame);
}

/* Draw a point in the Graphic Port */
void XXDrawPoint(FRAME frame,int x,int y)
{
    Rect box;
	
	box.left = x-1;
	box.right = x+1;
	box.top = y-1;
	box.bottom = y+1;
	
	XXOpenPortIfNeeded(frame);
	MoveTo((short) x,(short) y);
	LineTo((short) x,(short) y);
	XXClosePortIfNeeded(frame);
}

/* Draw an ellipse in a rect */
void XXDrawEllipse(FRAME frame,int x,int y,int dx, int dy)
{
  Rect box;

  box.left = x;
  box.right = x+dx;
  box.top = y;
  box.bottom = y+dy;
	
  XXOpenPortIfNeeded(frame);
  FrameOval(&box);
  XXClosePortIfNeeded(frame);
}

/* Fill an ellipse in a rect */
void XXFillEllipse(FRAME frame,int x,int y, int dx,int dy)
{

  Rect box;

  box.left = x;
  box.right = x+dx;
  box.top = y;
  box.bottom = y+dy;
		
  XXOpenPortIfNeeded(frame);
  if (frame->pnMode == patXor) PaintOval(&box);
  else {
    _XXPattern();
     FillCOval(&box,theColorPattern);
  }
  XXClosePortIfNeeded(frame);
}

/* Draw a string in the graphic port */
void XXDrawString(FRAME frame,int x,int y,char *str)
{
	strcpy(str1,str);
	XXOpenPortIfNeeded(frame);
	MoveTo((short) x,(short) y);
	DrawString(CtoPstr(str1)); 
	XXClosePortIfNeeded(frame);
}


/* Draw a rectangle */
void XXDrawRect(FRAME frame,int x,int y,int dx,int dy)
{
  Rect box;
	
  box.left = x;
  box.right = x+dx;
  box.top = y;
  box.bottom = y+dy;
	
  XXOpenPortIfNeeded(frame);
  FrameRect(&box); 
  XXClosePortIfNeeded(frame);
}
	
/* fill a rectangle */
void XXFillRect(FRAME frame,int x,int y,int dx,int dy)
{
  Rect box;
	
  box.left = x;
  box.right = x+dx;
  box.top = y;
  box.bottom = y+dy;
	
  XXOpenPortIfNeeded(frame);
	
  if (frame->pnMode == patXor) PaintRect(&box);
  else {
	_XXPattern();
    FillCRect(&box,theColorPattern); 
  }
  XXClosePortIfNeeded(frame);
} 

/* Set the clip rect */
void XXSetClipRect(FRAME frame, int x, int y, int w, int h)
{
  Rect r;
   
  r.left = x;
  r.right = x+w;
  r.top = y;
  r.bottom = y+h;
   
  XXOpenPortIfNeeded(frame);

  ClipRect(&r);
}


/* Copy a pixmap in a window */
static void CopyPixMapToWindow(GWorldPtr dSrc,WindowPtr dDst,
                     int x1,int y1,int w1,int h1,int x2,int y2,int w2,int h2)
{
  CGrafPtr savePort;
  GDHandle device;  
  Rect dst;
  Rect src;
  
  RGBColor blackMac ,whiteMac;
  src.top = y1;
  src.left = x1;
  src.bottom = y1+h1;
  src.right = x1+w1;
  
  dst.top = y2;
  dst.left = x2;
  dst.bottom = h2+y2;
  dst.right = w2+x2;
  
  blackMac.red = blackMac.green = blackMac.blue = 0;
  whiteMac.red = whiteMac.green = whiteMac.blue = 65535;
  
  
  GetGWorld(&savePort,&device);
  SetGWorld((CGrafPtr) dDst,device);
  RGBForeColor(&blackMac);
  RGBBackColor(&whiteMac);
  CopyBits(&((GrafPtr) dSrc)->portBits,&((GrafPtr) dDst)->portBits,&src,&dst,srcCopy,NULL);
  SetGWorld(savePort,device);
}


/* Allocate the current pixmap */
static GWorldPtr thePixMap;
static int theW,theH;

void XXAllocPixMap(int w,int h,unsigned char **pData,int *pRowBytes)
{
  int x = 0;
  int y = 0;
  GWorldPtr myWorld;
  Rect winBounds;
  CGrafPtr savePort;
  GDHandle device;  
  PixMapHandle pp;
  
  /* This is the Rectangle defining the pixmap */
  winBounds.left = x;
  winBounds.right = x+w;
  winBounds.top = y;
  winBounds.bottom = y+h;
  
  /* We then create the pixmap */
  NewGWorld(&myWorld, 0, &winBounds, NULL,NULL,0);
  
  /* ?? Is that bad ? */
  LockPixels(GetGWorldPixMap(myWorld));

  /* We associate the palette to the window if there is one */
  if (thePalette != NULL) SetPalette((GrafPtr) myWorld,thePalette,false);

  /* We clear the PixMap */
  GetGWorld(&savePort,&device);
  SetGWorld(myWorld,NULL);
  EraseRect(&winBounds);
  SetGWorld(savePort,device);

  thePixMap = myWorld;
  theH = h;
  theW = w;
  
  pp = GetGWorldPixMap(thePixMap);
  *pData = (unsigned char *) GetPixBaseAddr(pp);
  *pRowBytes = ((unsigned long) (*pp)->rowBytes) & 0x3FFF;      
}

/* Delete the current pixmap */
void XXDeletePixMap(void)
{
  DisposeGWorld((CGrafPtr) thePixMap);
}

/* Display the current pixmap in a frame */
void XXDisplayPixMap(FRAME frame,int winX,int winY)
{
   CopyPixMapToWindow(thePixMap,frame,0,0,theW,theH,winX,winY,theW,theH);
}

char XXIsDisplayBLittle(void)
{
  return(NO);
}

void XXFlush(void)
{
}
/********************************************************/
/*    Create/Delete/Change a window                     */
/********************************************************/


static FRAME frames[100];
static nFrames = 0;

void AddNoUpdate(FRAME frame)
{  
  if (nFrames >= 99) return;
  
  frames[nFrames++] = frame;
  frames[nFrames] = NULL;
}


char RemoveNoUpdate(FRAME frame, char flagAll)
{
  int i;  
  char answer = NO;
  
  if (nFrames == 0) return(NO);
  
  for (i=nFrames-1;i>=0;i--) {
    if (frames[i] == frame) {
      answer = YES;
      memmove(frames+i,frames+i+1,sizeof(FRAME)*(nFrames-i));
      nFrames--;
      if (!flagAll) break;
    }
  }
  
  return(answer);
}


/* Delete a window */
void XXDeleteFrame(FRAME frame)
{
  RemoveNoUpdate(frame,YES);
  DisposeWindow(frame);
}


/* Create a window */
FRAME XXNewFrame(char *title, int x,int y,int w,int h)
{
  WindowPtr myWindow;
  Rect winBounds;
  WindowPtr frontFrame = FrontWindow();  
  
  
  /* This is the Rectangle defining the window */
  winBounds.left = x;
  winBounds.right = x+w;
  winBounds.top = y;
  winBounds.bottom = y+h;
  
  /* We prepare the title */
  strcpy(str1,title);

  /* We then create the window */
  myWindow = NewCWindow(0L, &winBounds, CtoPstr(str1), true, 
  					    documentProc,(WindowPtr) -1L, true, 0);
  					
  /* We associate the palette to the window if there is one */
  if (thePalette != NULL) SetPalette(myWindow,thePalette,false);

  SelectWindow(frontFrame);
  
  XXOpenPortIfNeeded(myWindow);	

  AddNoUpdate((FRAME) myWindow);
  
  return((FRAME) myWindow);
}


/* Change the size the position and the title of a window */
void XXChangeFrame(FRAME frame,char *title, int x,int y,int w,int h)
{
  if (title != NULL) {
	strcpy(str1,title);
	SetWTitle(frame,CtoPstr(str1));
  }
  MoveWindow(frame,(short) x,(short) y,false);
  SizeWindow(frame,(short) w,(short) h,true);	
}

FRAME XXGetFrontFrame(void)
{
  return(FrontWindow());  
}

void XXFrontFrame(FRAME frame)
{
  SelectWindow(frame);
}

static void GetKeyCode(EventRecord *macEvent, EVENT event)
{
  unsigned int charCode,keyCode,modifiers;
  Ptr KCHRPtr;
  static unsigned long state = 0;
  long c,c1;

  event->key = 0;
    
  /* Get the character generated by the keys */  
  charCode = macEvent->message & charCodeMask;
  
  /* Get the virtual code of the key (with no modifier keys) */
  keyCode = (macEvent->message & keyCodeMask) >> 8;
  
  /* Case of the return key */
  if ((charCode == '\n' || charCode == '\r') && 
      !(macEvent->modifiers & shiftKey) && !(macEvent->modifiers & optionKey) && !(macEvent->modifiers & controlKey)) {
    event->key = NewlineKC;
    return;
  }
  
  /* Case of a regular character */
  if (!(macEvent->modifiers & controlKey) && (charCode>=' ' && charCode <= '~')) {
    event->key = charCode;
    return;
  }

  /* Get the character associated to the key with no modifier keys */
  KCHRPtr = (Ptr) GetScriptManagerVariable(smKCHRCache);
  c = KeyTranslate(KCHRPtr, keyCode, &state);
  
  /* Case of a 'ctrl d' (corresponding to an 'eof' character) */
  if (c == 'd' && (macEvent->modifiers & controlKey) && 
      !(macEvent->modifiers & shiftKey) && !(macEvent->modifiers & optionKey)) {
    event->key = EofKC;
    return;
  }

  /* The function keys */
  event->key = 0;
  if (charCode == 0x10) {
    switch(keyCode) {
    case 122 : event->key = F1KC; break;  
    case 120 : event->key = F2KC; break;  
    case 99 : event->key = F3KC; break;  
    case 118 : event->key = F4KC; break;  
    case 96 : event->key = F5KC; break;  
    case 97 : event->key = F6KC; break;  
    case 98 : event->key = F7KC; break;  
    case 100 : event->key = F8KC; break;  
    case 101 : event->key = F9KC; break;  
    case 109 : event->key = F10KC; break;  
    case 103 : event->key = F11KC; break;  
    case 111 : event->key = F12KC; break;  
    case 105 : event->key = F13KC; break;  
    case 107 : event->key = F14KC; break;  
    case 113 : event->key = F15KC; break; 
    } 
  }

  /* the other special keys */
  if (event->key == 0) {
    switch(keyCode) {
      case 53 : event->key = EscapeKC; break;  
      case 0x7B : event->key = LeftKC; break;  
      case 0x30 : event->key = TabKC; break;  
      case 0x7C : event->key = RightKC; break;  
      case 0x7E : event->key = UpKC; break;  
      case 0x7D : event->key = DownKC; break;  
      case 0x01 : event->key = HomeKC; break;  
      case 0x77 : event->key = EndKC; break;  
      case 0x33 : event->key = DeleteKC; break;  
      case 0x1B : event->key = ClearKC; break;
    }
  }
  
  /* The other cases */
  if (event->key == 0) {  
    /* We first test whether the key without the 'ctrl' key is a regular ascii */
    if (macEvent->modifiers & controlKey) {
      modifiers = 0;
      if (macEvent->modifiers & optionKey) modifiers = optionKey;
      if (macEvent->modifiers & shiftKey) modifiers += shiftKey;
      c1 = KeyTranslate(KCHRPtr, keyCode+modifiers, &state);
      if (c1>=' ' && c1 <= '~') {
        event->key = c1+ModCtrl;
        return;
      }
    }

    /* We then test whether the key without the 'opt' and the 'ctrl' key is a regular ascii */
    if ((macEvent->modifiers & optionKey) && (macEvent->modifiers & controlKey)) {
      modifiers = 0;
      if (macEvent->modifiers & shiftKey) modifiers = shiftKey;
      c1 = KeyTranslate(KCHRPtr, keyCode+modifiers, &state);
      if (c1>=' ' && c1 <= '~') {
        event->key = c1+ModOpt+ModCtrl;
        return;
      }
    }

    /* We then test whether the key without the 'opt' key is a regular ascii */
    if (macEvent->modifiers & optionKey) {
      modifiers = 0;
      if (macEvent->modifiers & controlKey) modifiers = controlKey;
      if (macEvent->modifiers & shiftKey) modifiers += shiftKey;
      c1 = KeyTranslate(KCHRPtr, keyCode+modifiers, &state);
      if (c1>=' ' && c1 <= '~') {
        event->key = c1+ModOpt;
        return;
      }
    }


    /* The other cases */    
    if (c >=' ' && c <= '~') event->key = c;
    else event->key = keyCode+(1<<9); 
  }

  if (macEvent->modifiers & optionKey) event->key += ModOpt;
  if (macEvent->modifiers & controlKey) event->key  += ModCtrl;
  if (macEvent->modifiers & shiftKey) event->key  += ModShift;
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
void XXAutoRepeatOn(void)
{
}

/* Turn the autorepeat off */
void XXAutoRepeatOff(void)
{
}


static int XXWNE(EventRecord *pMyEvent,int eventMask)
{
	if (hasWNE) 
		return(WaitNextEvent((short) eventMask, pMyEvent, 0, nil));
	else {
		SystemTask();
	 	return(GetNextEvent((short) eventMask, pMyEvent));
	}
}



void XXGetNextEvent(EVENT event, int flagWait)
{
  WindowPeek wp;
  EventRecord myEvent;  
  long eventType = 0;
  long where,theResult;
  Rect rect;
  GrafPtr savePort;
  Point pt = {100,100};

  /*
   * Variables that store the front window, the window the mouse is in and 
   * the coordinates of the mouse relative to both windows
   */
  Point gMouse,lMouse;
  WindowPtr mouseFrame,frontFrame,frame;
  WINDOW mouseWindow,frontWindow,window;
  TEXTWINDOW mouseTextWindow,frontTextWindow,textWindow;

  /* Variables used for key events */

  /* This flag is set if the next event to be sent is 'nextEvent' */
  static int sendEventsN = 0;
  static struct event sendEvents[10];
  
  /* This flag indicates that the next update event on the corresponding window should not be performed */
  static FRAME flagNoUpdate = NULL;
  
  /*
   * The old mouse window, the old local coordinates relative to this window.
   */
  static WINDOW oldMouseWindow = NULL;
  static Point oldLMouse;
  
  /*
   * The last button down event that was received
   */
  static unsigned long buttonDown = NoButton;     
  static WINDOW buttonDownWindow = NULL;     

   
  /*
   * The old and the new coordinates of the mouse relative to the buttonDownWindow   
   */
   static Point oldButtonDownMouse;
   Point buttonDownMouse;


  SetEventMask(everyEvent);  /* Allow all the events to be received */


  /* Init ... */
  event->type = NoEvent;
  
   
  /*
   * Should we send the 'nextEvent' ?
   */
  if (sendEventsN != 0) {
    *event = sendEvents[--sendEventsN];
    if (event->type == ButtonUp || event->type == KeyUp) {
      mouseWindow = (WINDOW) event->object;
      lMouse.h = event->i;
      lMouse.v = event->j;
      goto jump;
    }
    return;
  }    
  
  /* Get the next event in the queue */  
  XXWNE(&myEvent,everyEvent);

  /* Get the window and the frame the mouse is in. Check if it is a Text Window */ 
  gMouse = myEvent.where;
  where = FindWindow(gMouse, &mouseFrame);
  mouseWindow = Frame2Window(mouseFrame);
  mouseTextWindow = IsTextWindow(mouseFrame);

  /* Get the local coordinate of the mouse */
  if (mouseFrame != NULL) {
    GetPort(&savePort);
    SetPort(mouseFrame);
    lMouse = gMouse;
	GlobalToLocal(&lMouse);
	SetPort(savePort);
  }
  
  /* Get the front window and the front frame. Check if it is a Text Window */ 
  frontFrame = FrontWindow();  
  frontWindow = Frame2Window(frontFrame);
  frontTextWindow = IsTextWindow(frontFrame);

  /* If a mouse button is down we compute the local coordinates of the mouse relative to the buttonDownWindow */
  if ((buttonDown & ButtonMask) != NoButton) {
    GetPort(&savePort);
    SetPort(buttonDownWindow->frame);
    buttonDownMouse = gMouse;
	GlobalToLocal(&buttonDownMouse);
	SetPort(savePort);
  }
  
  /* We set the 'x','y' and 'window' fields of 'event' */
  event->key = 0;
  if ((buttonDown & ButtonMask) != NoButton) {
    event->object = (GOBJECT) buttonDownWindow;
    event->i = buttonDownMouse.h;
    event->j = buttonDownMouse.v;
  }
  else {
    if (mouseWindow != NULL) event->object = (GOBJECT) mouseWindow;
    else event->object = NULL;
    event->i = lMouse.h;
    event->j = lMouse.v;
  }
  
  
  /*
   * The different cases of events
   */
   
  switch(myEvent.what ) {
    
    case nullEvent: 
      /* Scrollbar updating of the console ? */
      if (XXTerminalWindow->scrollbarDirty == true) TWUpdateScrollbar(XXTerminalWindow);
            	
	  /* We take care of the cursor icon and the cursor flashing if the mouse is in a text window */
	  if (mouseTextWindow != NULL) {
        GetPort(&savePort);
 	    SetPort((WindowPtr) mouseFrame);
   	    if (PtInRect(lMouse, &(*mouseTextWindow->textEdit)->viewRect) && iBeamCursorH)
   		  SetCursor(*iBeamCursorH);
	    else  SetCursor(&qd.arrow);
	    TEIdle(mouseTextWindow->textEdit);
		SetPort(savePort);
	   } 
	   else if (mouseWindow != NULL) {
	     GetPort(&savePort);
 	     SetPort((WindowPtr) mouseFrame);
  	     SetCursor(&qd.arrow);
		 SetPort(savePort);
	   }  
	   break;
	  
	case keyDown: case autoKey: case keyUp:

      /* Set the key and the modifiers */
      GetKeyCode(&myEvent,event); 

      /* Menu events */
      if ((myEvent.modifiers & cmdKey) && (KeyMask & event->key) != LeftKC && (KeyMask & event->key) != RightKC) {
        if (myEvent.what == keyUp) return;
	    UpdateMenuItems();
	    DoMenuChoice(MenuKey((myEvent.message & charCodeMask)));
		return;
	  } 
	  	
	  /*
	   * Case we have a mouse with 3 buttons :
	   *    - left one should be assigned to mouse button
	   *    - middle should be assign to Cmd Left arrow
	   *    - right should be assign to Cmd  arrow
	   */
      if (Is3MouseButtons && (myEvent.modifiers & cmdKey) && ((KeyMask & event->key) == LeftKC || (KeyMask & event->key) == RightKC)) {
        if ((KeyMask & event->key) == LeftKC) event->button = MiddleButton;
	    else event->button = RightButton;
	    event->button += (ModMask & event->key);
        if (myEvent.what == keyDown) event->type = ButtonDown;
        else if (myEvent.what == keyUp && !ForceKeyUp) 
          event->type = ButtonUp;
        else return;
        if (ForceKeyUp) {
          sendEvents[sendEventsN] = *event; 
          sendEvents[sendEventsN].type = ButtonUp;
          sendEventsN++;
        }
      }
       
      /*
       * Case of a regular key event
       */
      else {  
	    if (myEvent.what == keyUp) {
	      if (!ForceKeyUp) event->type = KeyUp;
	      else return;
	    }
        else {
          event->type = KeyDown;
          if (ForceKeyUp) {
            sendEvents[sendEventsN] = *event; 
            sendEvents[sendEventsN].type = KeyUp;
            sendEventsN++;
          }
        } 
      }
      
      break;
		
	case activateEvt: case updateEvt:
	  frame = (WindowPtr) (myEvent.message);
	  window = Frame2Window(frame);
	  textWindow = IsTextWindow(frame);
      if (window != NULL) {      
        if (myEvent.what == updateEvt) {
          wp = ((WindowPeek) frame);
/*	      Printf("Update %s\n",window->title); */
         if (flagNoUpdate == frame || RemoveNoUpdate(frame,NO)) {
            flagNoUpdate = NULL;
	        BeginUpdate(frame);
	        EndUpdate(frame);
            return;
          }
          event->i = (*(wp->updateRgn))->rgnBBox.left-(*(wp->contRgn))->rgnBBox.left;
          event->j = (*(wp->updateRgn))->rgnBBox.top-(*(wp->contRgn))->rgnBBox.top;
          event->m = (*(wp->updateRgn))->rgnBBox.right-(*(wp->contRgn))->rgnBBox.left-event->i;
          event->n = (*(wp->updateRgn))->rgnBBox.bottom-(*(wp->contRgn))->rgnBBox.top-event->j;
          XXOpenPortIfNeeded(frame);
	      BeginUpdate(frame);
	      EndUpdate(frame);
	      event->type = Draw;
	      event->object = (GOBJECT) window;
	      return;
	    }
	  }
	  else if (textWindow != NULL) {
	    HandleTWUpdateActivateEvent(textWindow,&myEvent);
	    return;
	  }
	  else Printf("Merde\n");
	  break;
	 
	case kHighLevelEvent:
 	  AEProcessAppleEvent(&myEvent);
  	  break;

	case diskEvt:
	  if (HiWord(myEvent.message) != noErr) DIBadMount(pt, myEvent.message);
	  break;

	case mouseUp:
	  if (mouseWindow != NULL || (ButtonMask & buttonDown) != NoButton) {
	    if (Is3MouseButtons == NO) {
	      if (myEvent.modifiers & cmdKey) event->button = RightButton;
	      else if (myEvent.modifiers & optionKey) event->button = MiddleButton;
	      else event->button = LeftButton;
	    }
	    else event->button = LeftButton;

        if (myEvent.modifiers & shiftKey) event->button += ModShift;
        if (myEvent.modifiers & controlKey) event->button += ModCtrl;
        if (Is3MouseButtons && (myEvent.modifiers & optionKey)) event->button += ModOpt;

	    /*Printf("mouseDown %d %d %d\n",*p1,*p2,*p3);*/
	    event->type = ButtonUp;
	  }
	  break;
	  
	case mouseDown:
	  switch (where) { 
		case inMenuBar:
  	      UpdateMenuItems();
		  DoMenuChoice(MenuSelect(myEvent.where));
		  return;
		case inSysWindow: 
		  SystemClick(&myEvent, mouseFrame); /*?? */
		  return;
		case inGoAway: 
 		  if (mouseWindow != NULL && mouseWindow == frontWindow) {
 		     if (oldMouseWindow == mouseWindow) oldMouseWindow = NULL;
 		     event->type = Del;
 		     event->object = (GOBJECT) mouseWindow;
		     return;
		  }
		  break;
		case inDrag:
          rect = (**grayRgnHandle).rgnBBox;
		  DragWindow(mouseFrame, myEvent.where, &rect);
		  if ((myEvent.modifiers & optionKey) && frontFrame != mouseFrame)
		     SelectWindow(frontFrame);
		  if (mouseTextWindow) break;
		  wp = (WindowPeek) mouseFrame;
	      event->type = Resize;
 		  event->object = (GOBJECT) mouseWindow;
          event->i = (*(wp->contRgn))->rgnBBox.left;
          event->j = (*(wp->contRgn))->rgnBBox.top;
          event->m = mouseWindow->w;
          event->n = mouseWindow->h;
		  /*Printf("drag\n");*/
          return;
		case inGrow:
	      if (mouseTextWindow!=NULL) TWGrowWindow(mouseTextWindow, myEvent.where);
          else if (mouseWindow != NULL) {
		    GetPort(&savePort);
		    SetPort(mouseFrame);
		    rect.top = rect.left = 50;
		    rect.bottom = rect.right = 800;
	        theResult = GrowWindow(mouseFrame, myEvent.where, &rect);
	        if (theResult == 0) break;
	        SizeWindow(mouseFrame, LoWord(theResult), HiWord(theResult), false);
	        event->type = Resize;
 		    event->object = (GOBJECT) mouseWindow;
            event->i = mouseWindow->x;
            event->j = mouseWindow->y;
            event->m = LoWord(theResult);
            event->n = HiWord(theResult);
            flagNoUpdate = mouseFrame;
	        InvalRect(&mouseFrame->portRect);
	        SetPort(savePort);
	      }
	      return;
		case inContent:
  	      if (mouseWindow != NULL) {
	        if (Is3MouseButtons == NO) {
	          if (myEvent.modifiers & cmdKey) event->button = RightButton;
	          else if (myEvent.modifiers & optionKey) event->button = MiddleButton;
	          else event->button = LeftButton;
	        }
	        else event->button = LeftButton;
	        if (myEvent.modifiers & shiftKey) event->button += ModShift;
	        if (myEvent.modifiers & controlKey) event->button += ModCtrl;
	        if (Is3MouseButtons && (myEvent.modifiers & optionKey)) event->button += ModOpt;
		    /*Printf("mouseDown %d %d %d\n",*p1,*p2,*p3);*/
	        event->type = ButtonDown;
	      }
	      else if (mouseTextWindow != NULL) {
	        SelectWindow(mouseFrame);
		    UpdateMenuItems();
  		    if (mouseTextWindow->state == PRINTFING) {
  		      if (StillDown()) while (WaitMouseUp());
  		    }
		    else TWDoContentClick(mouseTextWindow, &myEvent);
	      }
		  break;
		
		default:
		  break;
	  }
	 
   default:
      break;
 }

 
jump :

 /* If 'buttonDown' then we don't accept button events of other buttons except for buttonUp in 1 button mouse mode */
 if ((ButtonMask & buttonDown) != NoButton && (ButtonMask & event->button) != (ButtonMask & buttonDown) && 
     (event->type == ButtonDown || (event->type == ButtonUp && Is3MouseButtons))) 
   event->type = NoEvent;
 
 /* In the case of buttonUp and 1 button mose mode we send the same button */
 if ((ButtonMask & buttonDown) != NoButton && (ButtonMask & event->button) != (ButtonMask & buttonDown) && 
     event->type == ButtonUp && !Is3MouseButtons)
     event->button = buttonDown;
   

 /* Manage 'ButtonDown' events */   
 if (event->type == ButtonDown) {
   /*if (event->button == LeftButton) {  this line is for Apple design keyboards only */
     buttonDown = event->button;
     buttonDownWindow = (WINDOW) event->object;
     buttonDownMouse = lMouse;
     oldButtonDownMouse = buttonDownMouse;
     return;
 }
 
 /* Manage 'ButtonUp' event */
 if (event->type == ButtonUp && ((ButtonMask & buttonDown) == (ButtonMask & event->button))) {
   /* Should we send enter/leave events right after the ButtonUp ? */
   if (buttonDownWindow != mouseWindow) {
     if (mouseWindow != NULL) {
       sendEvents[sendEventsN].type = Enter;
       sendEvents[sendEventsN].object = (GOBJECT) mouseWindow;
       sendEvents[sendEventsN].x = lMouse.h;
       sendEvents[sendEventsN].y = lMouse.v;
       sendEventsN++;
     }
     sendEvents[sendEventsN].type = Leave;
     sendEvents[sendEventsN].object = (GOBJECT) buttonDownWindow;
     sendEvents[sendEventsN].x = event->i;
     sendEvents[sendEventsN].y = event->j;
     sendEventsN++;
   }     
   event->button = buttonDown;
   buttonDown = NoButton;
   buttonDownWindow = NULL;
   oldMouseWindow = mouseWindow;
   oldLMouse = lMouse; 
   return;
 }

 /* Then we check for motion events with button down */
 if ((ButtonMask & buttonDown) != NoButton) {
  if  (oldButtonDownMouse.v != buttonDownMouse.v  || oldButtonDownMouse.h != buttonDownMouse.h) {
    event->button = buttonDown;
    oldButtonDownMouse = buttonDownMouse;
    event->type = MouseMotion;
    return;
  }
  else return;
 } 
   
 /* if an event has been found then it's over */
 if (event->type != NoEvent) return;


 /* If in the drag area then consider that it is out of the window */
 if (mouseFrame != NULL && lMouse.v < 0) {
   mouseWindow = NULL;
   mouseFrame = NULL;
 }

 /* We check for leave events */
 if (oldMouseWindow != NULL && oldMouseWindow != mouseWindow) {
   event->type = Leave;
   event->object = (GOBJECT) oldMouseWindow;
   /* Test if the next event should be an enter event */
   if (mouseWindow != NULL) {
     sendEvents[sendEventsN].type = Enter;
     sendEvents[sendEventsN].object = (GOBJECT) mouseWindow;
     sendEvents[sendEventsN].x = lMouse.h;
     sendEvents[sendEventsN].y = lMouse.v;
     sendEventsN++;
   }
   oldMouseWindow = mouseWindow;
   oldLMouse = lMouse; 
   return;
 }
 
 /* Check enter events */
 if (mouseWindow != NULL && oldMouseWindow != mouseWindow) {
   event->type = Enter;
   event->object = (GOBJECT) mouseWindow;
   oldMouseWindow = mouseWindow;
   oldLMouse = lMouse; 
   return;
 }
  
 /* Then we check for mouse motion events */
 if (mouseWindow != NULL && oldMouseWindow == mouseWindow) {
    if (oldLMouse.v == lMouse.v && oldLMouse.h == lMouse.h) {  /* Did not move */
      event->type = NoEvent;
      return;
    }
    event->type = MouseMotion;
    /* Set the modifiers */
    event->button = NoButton;
    if (myEvent.modifiers & shiftKey) event->button += ModShift;
    if (myEvent.modifiers & controlKey) event->button += ModCtrl;
    if (myEvent.modifiers & optionKey) event->button += ModOpt;
    oldMouseWindow = mouseWindow;
    oldLMouse = lMouse; 
    return;
 }
}



/****************************************
 *
 *            Managing Colors             
 *
 ****************************************/

/*
 * Set the current colormap according to the values of red, green and blue. 
 * Specifies the index of the color of the cursor and of the colors that must
 * behave 'properly' during a PenInverse mode.
 * Returns the number of color cells that are used.
 */

int XXSetColormap(unsigned short red[],unsigned short green[],unsigned short blue[],
                  unsigned long pixels[], int nCols, int flagSharedColormap, int mouseMode,
                  unsigned short mouseRed, unsigned short mouseGreen, unsigned short mouseBlue)
{
   static RGBColor aColor;
   PaletteHandle theNewPalette;
   int r,i;
   AHASHELEM e;
   
   /* If the depth is <= 8 we use the Palette manager if just need to fill the pixel array */
   if (theDepth <= 8) {
     
     /* Create a new palette */
     theNewPalette = NewPalette(nCols,NULL,pmTolerant,0);
     if (theNewPalette == NULL) Errorf("Didn't succeed in creating a colormap");
   
     /* Set the different colors in this palette */
     for (i=0;i<nCols;i++) {
       aColor.red = red[i];
       aColor.blue = blue[i];
       aColor.green = green[i];
     
       SetEntryColor(theNewPalette,(short) i,&aColor);
       pixels[i] = i<<8;
       pixels[i] += Color2Index(&aColor);
     }
   
     /* Replace the old palette */
     if (thePalette != NULL) DisposePalette(thePalette);
     thePalette = theNewPalette;     
  
       
    /* Assign the new palette to all the windows */
    if (XXTerminalWindow) SetPalette((WindowPtr) XXTerminalWindow ,thePalette,false);
    if (theWindowsHT) {
      for (r = 0; r<theWindowsHT->nRows;r++) {
        for (e = theWindowsHT->rows[r]; e != NULL; e = e->next) { 
          SetPalette(((WINDOW) e)->frame ,thePalette,false);
        }
      }
    }
  }
  
  else {
    for (i=0;i<nCols;i++) {
      aColor.red = red[i];
      aColor.blue = blue[i];
      aColor.green = green[i];
      pixels[i] = Color2Index(&aColor); 
    }
  }   
   
   return(nCols);
}

void XXBgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  *r = *g = *b = 65535;
}

void XXFgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  *r = *g = *b = 0;
}

/* Returns the number of colors available */
int XXNumOfColors(void)
{
  if (theDepth == 32) return(1<<24);
  return(1<<theDepth);
}

void XXAnimateColor(unsigned long c, unsigned short r, unsigned short g, unsigned short b)
{
}

int XXDepth(void)
{
  return(theDepth);
}

/* Change the color of the Graphic port */
void XXSetColor(FRAME frame,unsigned long pixel)
{ 
  RGBColor c;
     
  XXOpenPortIfNeeded(frame);
  if (theDepth <= 8)  PmForeColor(pixel>>8);
  else {
    Index2Color(pixel,&c);
    RGBForeColor(&c);
  }
  currentColor = pixel;
  XXClosePortIfNeeded(frame);
}


/* Procedure to set the colorPattern to the currentColor if not yet done */
static void _XXPattern() 
{
  PixMapHandle pmh;
  Handle patData;
  static unsigned long currentPatColor;
  RGBColor c;
    
  if (theColorPattern != NULL && currentPatColor == currentColor) return;

  if (theColorPattern) DisposePixPat(theColorPattern);
   
  if (theDepth <= 8) {        

    if (theDepth < 8) Errorf("_XXPattern() : does not work with this screen depth");
      
    theColorPattern = NewPixPat();
    (**theColorPattern).patType = 1;
    SetHandleSize((**theColorPattern).patData,1);
    pmh = (**theColorPattern).patMap ;
    (**pmh).rowBytes = 1;
    (**pmh).bounds.top = 0;
    (**pmh).bounds.left = 0;
    (**pmh).bounds.bottom = 1;
    (**pmh).bounds.right = 1;
    (**pmh).cmpSize = (**pmh).pixelSize; 
    (**(**theColorPattern).patData) = currentColor>>8;
    Palette2CTab(thePalette,(**pmh).pmTable);
  }

  else if (theDepth == 16) {            
    theColorPattern = NewPixPat();
    (**theColorPattern).patType = 1;
    SetHandleSize((**theColorPattern).patData,2);
    pmh = (**theColorPattern).patMap ;
    (**pmh).rowBytes = 2;
    (**pmh).bounds.top = 0;
    (**pmh).bounds.left = 0;
    (**pmh).bounds.bottom = 1;
    (**pmh).bounds.right = 1;
    (*(**theColorPattern).patData)[1] = currentColor & 0xFF; 
    (*(**theColorPattern).patData)[0] = (currentColor & 0xFF00)>>8; 
  }
  
  else if (theDepth == 32) {            
  theColorPattern = NewPixPat();
    Index2Color(currentColor,&c);
     MakeRGBPat(theColorPattern,&c);    

/*    theColorPattern = NewPixPat();
    (**theColorPattern).patType = 1;
    patData = (**theColorPattern).patData;
    SetHandleSize(patData,32);
    pmh = (**theColorPattern).patMap ;
    (**pmh).rowBytes = 4;
    (**pmh).bounds.top = 0;
    (**pmh).bounds.left = 0;
    (**pmh).bounds.bottom = 8;
    (**pmh).bounds.right = 1;
    (*patData)[28] = (*patData)[24] = (*patData)[20] = (*patData)[16] = (*patData)[12] = (*patData)[8] = (*patData)[4] = (*patData)[0] = (currentColor & 0xFF000000)>>24;
    (*patData)[29] = (*patData)[25] = (*patData)[21] = (*patData)[17] = (*patData)[13] = (*patData)[9] = (*patData)[5] = (*patData)[1] = (currentColor & 0xFF0000)>>16;  
    (*patData)[30] = (*patData)[26] = (*patData)[22] = (*patData)[18] = (*patData)[14] = (*patData)[10] = (*patData)[6] = (*patData)[2] = (currentColor & 0xFF00)>>8;  
    (*patData)[31] = (*patData)[27] = (*patData)[23] = (*patData)[19] = (*patData)[15] = (*patData)[11] = (*patData)[7] = (*patData)[3] = currentColor & 0xFF;     */
  }
  
  else {
   Warningf ("_XXPattern () : Does not know how to manage this screen depth");
  }
  
  currentPatColor = currentColor;
}

int XXIsBWScreen(void)
{
  return(NO); /* ?? */
}

char *XXScreenType(void)
{
  if (theDepth > 8) return("TrueColor");
  else return("PseudoColor");
}


/****************************************
 *
 *            Managing Fonts and Strings
 *
 ****************************************/

static void XXSetStyle(unsigned char style)
{
  Style face = normal;
  
  if (style & FontBold) face |= bold;
  if (style & FontItalic) face |= italic;
  TextFace(face);
}
  
/* Change the Font of the text for the graphic port */
void XXSetFont(FRAME frame,FONT font)
{

  XXOpenPortIfNeeded(frame);
  TextFont(font->id);
  TextSize(font->size);
  XXSetStyle(font->style);
}
 

static Str255 systemFont1;

char *XXGetDefaultFont(FONTID *id,int *size)
{
  *id = 0;
  GetFontName(*id,systemFont1);

#ifdef PtoCstr
  PtoCstr(systemFont1);
#endif
  
  *size = 11;
   
  return((char *) systemFont1);
}

void XXGetFontInfo(FONT font, int *ascent, int *descent, int *interline)
{
  FontInfo info;
  FONTID idCur;
  short size;
  Style face;
  
  idCur = curPort->txFont;
  size = curPort->txSize;
  face = curPort->txFace;

  TextFont(font->id);
  TextSize(font->size);  
  XXSetStyle(font->style);
  
  GetFontInfo(&info);
/*  *ascent = info.ascent-3;
  *descent = info.descent;
  *interline = info.leading+3;
*/
  *ascent = info.ascent;
  *descent = info.descent;
  *interline = info.leading;

  TextFont(idCur);
  TextSize(size);
  TextFace(face);

}


/* Test whether a font exist and get its FontId */
char XXExistFont(char *name,int size, unsigned char style,FONTID *pid) 
{
  FONTID id;
  static char str[255];

  strcpy(str,name);  
  GetFNum(CtoPstr(str),&id);
  if (id == 0) {
    if (strcmp(name,(char *) systemFont1)) return(0); 
  }
  
  if (pid != NULL) *pid = id;
 
  return(1);
}

int XXGetStringWidth(FONT font, char *text)
{
  static char str[255];
  FONTID idCur;
  short size;
  Style face;
  int width;
  
  idCur = curPort->txFont;
  size = curPort->txSize;
  face = curPort->txFace;

  TextFont(font->id);
  TextSize(font->size);  
  XXSetStyle(font->style);


  strcpy(str,text);
  
  width =  StringWidth(CtoPstr(str));

  TextFont(idCur);
  TextSize(size);
  TextFace(face);

  return(width);
}

void XXFontMatch(char *name,char flagSize,int size,char flagStyle,unsigned char style)
{
  short	rCount, rID, j;
  Handle	rHandle;
  ResType	rType;
  Str255	rName;
  char  firstByte;
  char *str;
  
  rCount = CountResources('FOND');
  SetResLoad( false ); /* do not need resource, just info */
  
  for( j=1; j <= rCount; j++ ) {
	rHandle = GetIndResource('FOND',j);
	GetResInfo(rHandle,&rID,&rType,rName);
    str = PtoCstr(rName);

/*    rHandle1 = GetResource('FOND',rID);
    if (rHandle1 == NULL) continue;
    HLock(rHandle1);
    cp = *rHandle;		   cp now points to the data 
    firstByte = *cp;	   use (*cp) or (**rHandle) to refer to first byte of data 

    HUnlock(rHandle1);	   let it float again 
    ReleaseResource(rHandle1);  free up memory when ALL done 
  */  
  
    if (MatchStr(str,name)) AppendListResultStr(str);
  }
  
  SetResLoad(true);  /* better do this! */
}




/* 
 * Command which is system dependent
 */
 
void C_System(char **argv) 
{
  char *str;
  
  argv = ParseArgv(argv,tWORD,&str,0);
  
  
  
  if (!strcmp(str,"mouse1")) Is3MouseButtons = NO;
  else if (!strcmp(str,"mouse3")) Is3MouseButtons = YES;
  else if (!strcmp(str,"forceKeyUp")) ForceKeyUp = YES;
  else if (!strcmp(str,"noForceKeyUp")) ForceKeyUp = NO;
  else if (!strcmp(str,"nEvents")) SetResultInt(toplevelCur->nEvents+1);
  else ErrorUsage1();
}




#endif


