
#include "computer.h"

//#undef LASTWAVE_WIN32_GRAPHICS
#ifdef LASTWAVE_WIN32_GRAPHICS

#include <windows.h>
#include "wincon.h"

//#undef   GetCommandLine 
//#include "lastwave.h"
//#include "gobject.h"
#ifndef FRAME
#define FRAME int *
#endif
#ifndef FONTID
#define FONTID int *
#endif
#ifndef FONT
#define FONT int *
#endif
#ifndef NO
#define NO 0
#endif
#ifndef YES
#define YES 1
#endif
//extern int tWORD
//extern char **ParseArgv(char **argv,...);

extern void Errorf(char *format,...);
extern void Warningf(char *format,...);
extern void Printf(char *format,...);
extern void Flush(void);
extern char* CharAlloc(int size);
extern float *FloatAlloc(int size);
extern void Free(void * ptr);

/*
 * The event structure 
 */
typedef struct event {

  /* the type of the event */  
  unsigned long type;
  
  /* The object it happened (or NULL if in terminal) */
  void * object;
    
  /* The button that was (un)pressed (along with the modifiers) */
  unsigned long button;
      
  /* The key that was (un)pressed (along with the modifiers) */
  unsigned long key;

  /* local coordinates */
  float x,y,w,h;

  /* global coordinates */
  int i,j,m,n;
  
} *EVENT;

#define NoEvent 0
#define ButtonDown (1l<<0)
#define ButtonUp (1l<<1)

#define KeyDown (1l<<2)
#define KeyUp (1l<<3)

#define Enter (1l<<4)
#define Leave (1l<<5)  
#define Draw (1l<<6)
#define Del (1l<<7)
#define Resize (1l<<8)
  
#define MouseMotion (1l<<9)
  
#define ErrorEvent (1l<<10)
#define TimeEvent (1l<<11)
#define DelayEvent (1l<<12)

/*
 * Each EventCategory correspond to a set of event types.
 */
 
enum { 
  ButtonEventCategory = 0,
  KeyEventCategory,
  EnterLeaveEventCategory,
  MotionEventCategory,
  OtherEventCategory,
  LastEventCategory
};

/*
 * Warning : The number of event categories must be used for the size
 * of the 'theBindings' array of a GCLASS
 */

/*
 * The different buttons (these definitions are used both as values and as masks) 
 */
 
#define LeftButton (1l<<0)
#define MiddleButton (1l<<1)
#define RightButton (1l<<2)
#define NoButton (1l<<3)

#define ButtonMask 0x000F

/*
 * The different modifier keys (these definitions are used both as values and as masks)
 */
#define ModShift (1l<<10)
#define ModCtrl (1l<<11)
#define ModOpt (1l<<12)
#define ModAny (1l<<13)

#define ModMask 0xFC00

/*
 * The different key codes of 'special keys'
 *
 *   WARNING : If you add one key code don't forget to add its name in the SpecialKeyNames array
 *             in window_event.s
 *
 */

#define KeyMask 0x03FF

#define EscapeKC 27              /* For xterm compatibilities we choose the ascii code */
#define NewlineKC ((int) '\n')   /* For xterm compatibilities we choose the ascii code */

#define EofKC 256     /* A key code that will nether conflict with the other ones */

#define FirstKC 0x0101

enum {

 RightKC = FirstKC,
 UpKC,LeftKC,DownKC,
 
 HomeKC,EndKC,ClearKC,
 
 DeleteKC,TabKC,
 
 F1KC,F2KC,F3KC,F4KC,F5KC,F6KC,F7KC,F8KC,F9KC,F10KC,F11KC,F12KC,F13KC,F14KC,F15KC,
 
 AnyKC,
 
 LastKC
 
};


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

static   int KeyEventProc(KEY_EVENT_RECORD KeyEvent,EVENT event);


void XXGetNextEvent(EVENT event,int flagWait)
{
  INPUT_RECORD irInBuf[128]; 
  static DWORD cNumRead = 0;
  static DWORD n = 0;
  HANDLE stdHandle;
  short read =0;	 
  event->type = NoEvent;
  
	
  if((stdHandle = GetStdHandle(STD_INPUT_HANDLE))== INVALID_HANDLE_VALUE){
    Errorf("XXGetNextEvent::bad console handle !\n");			
  }	
  while(!read){
	if(!cNumRead){
		if (! ReadConsoleInput( 
		       stdHandle,      // input buffer handle 
			   irInBuf,     // buffer to read into 
			   128,         // size of read buffer 
			   &cNumRead) ) {// number of records read 					
		Errorf("XXGetNextEvent::can't read the console I !!!\n");
	}
	n=0;
	}
  

	if(!cNumRead)
		return;	
    switch(irInBuf[n].EventType) 
	  { 
       case KEY_EVENT: // keyboard input 
	    	KeyEventProc(irInBuf[n].Event.KeyEvent,event);				 
			read=1;
			break; 
        case MOUSE_EVENT: // mouse input                   
        case WINDOW_BUFFER_SIZE_EVENT:              
        case FOCUS_EVENT:  
        case MENU_EVENT:   // disregard menu events 						
             break; 
		default: 
             Printf("eventType :: %d - n=%lu - cNum = %lu !!!\n",irInBuf[n].EventType,n,cNumRead);
             break; 
      }//switch 
	
	
	n++;	
	cNumRead--;
  }
    return;
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
//  char *str;
  
//  argv = ParseArgv(argv,tWORD,&str,0);
  
//  if (!strcmp(str,"nEvents")) SetResultInt(toplevelCur->nEvents+1);
//  else ErrorUsage();
}

static int KeyEventProc(KEY_EVENT_RECORD KeyEvent,EVENT event){				
	char ShiftPressed = 0,ControlPressed = 0, AltPressed = 0;
	DWORD ckState = KeyEvent.dwControlKeyState;

	event->button = NoButton;
	if(KeyEvent.bKeyDown)
		event->type = KeyDown;
	else
		event->type = KeyUp;
	event->object = NULL;

	// Get Modifiers
	if((ckState & RIGHT_CTRL_PRESSED) || (ckState & LEFT_CTRL_PRESSED)) 
		ControlPressed = YES;
	if((ckState & SHIFT_PRESSED)) 
		ShiftPressed = YES;
	if((ckState & RIGHT_ALT_PRESSED) ||	(ckState & LEFT_ALT_PRESSED))
		AltPressed = YES;
	
	/* Is it a special  key? */
	switch(KeyEvent.wVirtualKeyCode) {
	case  VK_ESCAPE         :
			event->key = EscapeKC;
			break;
		// Arrow keys
	case  VK_LEFT        :   
			event->key = LeftKC;
			break;
	case  VK_UP             :
			event->key = UpKC;
			break;
	case  VK_RIGHT :         
			event->key = RightKC;
			break;
	case  VK_DOWN           :
			event->key = DownKC;
			break;
		// Home/End/Clear
	case  VK_HOME     :      
			event->key = HomeKC;
			break;
	case  VK_END         :   
			event->key = EndKC;
			break;
	case  VK_CLEAR:
			event->key = ClearKC;
			break;
		// Delete/Tab
	case  VK_BACK:
	case  VK_DELETE:         
			event->key = DeleteKC;
			break;
	case  VK_TAB:
			event->key = TabKC;
			break;
		// F1 to F15
	case  VK_F1:
			event->key = F1KC;
			break;
	case  VK_F2:
			event->key = F2KC;
			break;
	case  VK_F3:
			event->key = F3KC;
			break;
	case  VK_F4:
			event->key = F4KC;
			break;
	case  VK_F5:
			event->key = F5KC;
			break;
	case  VK_F6:
			event->key = F6KC;
			break;
	case  VK_F7:
			event->key = F7KC;
			break;
	case  VK_F8:
			event->key = F8KC;
			break;
	case  VK_F9:
			event->key = F9KC;
			break;
	case  VK_F10:
			event->key = F10KC;
			break;
	case  VK_F11:
			event->key = F11KC;
			break;
	case  VK_F12:
			event->key = F12KC;
			break;
	case  VK_F13:
			event->key = F13KC;
			break;
	case  VK_F14:
			event->key = F14KC;
			break;
	case  VK_F15:
			event->key = F15KC;
			break;
	// We must filter when some keys are pressed :
	// CapsLock/ScrollLock/NumLock 
	case  VK_CAPITAL:
	case VK_NUMLOCK :
	case VK_SCROLL :
			event->key = 0;
			event->type = NoEvent;
			break;
	// LEFT_WIN/RIGHT_WIN/APPS
	case VK_LWIN :
	case VK_RWIN :
	case VK_APPS :
			event->key = 0;
			event->type = NoEvent;
			break;
	// INSERT/PAGE_UP/PAGE_DOWN
	case VK_INSERT :
	case VK_PRIOR  :
	case VK_NEXT   :
			event->key = 0;
			event->type = NoEvent;
			break;
	default :
		// Case of the newline characters
		if((unsigned long)KeyEvent.uChar.AsciiChar==(unsigned long)'\n'
			|| (unsigned long)KeyEvent.uChar.AsciiChar==(unsigned long)'\r') {
			event->key = NewlineKC;
			break;
		}
		// Standard characters with no control/alt modifiers
		if(!ControlPressed && !AltPressed &&
		(KeyEvent.uChar.AsciiChar>=' ') &&
		(KeyEvent.uChar.AsciiChar<='~')) {
			event->key = (unsigned long)KeyEvent.uChar.AsciiChar;
			break;
		}

		// Case of the EOF (CONTROL-D)
		if((unsigned long)KeyEvent.uChar.AsciiChar==(unsigned long) 0x4) {
			event->key = EofKC;
			break;
		}


		//event->key = (unsigned long)KeyEvent.uChar.AsciiChar;
		event->key = KeyEvent.wVirtualKeyCode;
		// We must filter when the Ctrl/Shift/Alt keys are pressed !
		if(event->key == VK_CONTROL || event->key == VK_SHIFT || event->key == VK_MENU) {
			event->key = 0;
			event->type = NoEvent;
			break;
		}
		if(ControlPressed)
			event->key += ModCtrl;
		if(ShiftPressed)
			event->key += ModShift;
		if(AltPressed)
			event->key += ModOpt;
		break;
	}
		
//	printf("EVT:%ld %c\n",(unsigned long int)KeyEvent.uChar.AsciiChar,KeyEvent.uChar.AsciiChar);
	return 0;
}

#endif//LASTWAVE_WIN32_GRAPHICS

