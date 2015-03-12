#include "computer.h"

#ifdef LASTWAVE_JAVA_GRAPHICS

#include <stdio.h>
#include "lastwave.h"
// REMI __int64 is not always defined, especially on Cygwin
//#ifndef __int64
//#define __int64 long long
//#endif
#include "JavaTerminal.h"
#include "java_keysyms.h"

JNIEnv *cachedGraphicsEnv = NULL;
jobject cachedGraphicsObject = NULL;
jclass JavaGraphicsClass = NULL;//Not yet graphics class, just method in terminal class

void initJavaGraphics(JNIEnv *env, jobject obj)
{
  jclass cls;
  printf("initJavaGraphics called \n");fflush(stdout);
  cachedGraphicsEnv = env;
  cachedGraphicsObject = obj;
  cls = (*env)->FindClass(env,"JavaTerminal");

  JavaGraphicsClass = (*env)->NewWeakGlobalRef(env,cls);
}

void XXOpenGraphics(void)
{ // Open a new class JavaGraphics
  JNIEnv *env = cachedGraphicsEnv;
	jclass cls = JavaGraphicsClass;

	jobject obj = cachedGraphicsObject;
	jmethodID mid;
  

	mid = (*env)->GetMethodID(env,cls,"javaGraphics","()V");
	if(mid == NULL){
		printf("Warning :Cannot find JMethod javaGraphics!!!");
		return;
	}
     
	    printf("XXOpenGraphics - trying to call java");
	   
		(*env)->CallVoidMethod(env,obj,mid);
	       
	
	//printf("XXOpenGraphics called\n");fflush(stdout);
}

void XXCloseGraphics(void)
{
  printf("XXCloseGraphics called\n");fflush(stdout);
}


/* Set the style of the line */
void XXSetLineStyle(FRAME frame,int flag)
{
  printf("XXSetLineStyle called\n");fflush(stdout);
}

/* Change the size of the pen of the Graphic Port */
void XXSetPenSize(FRAME frame,int size)
{
  printf("XXSetPenSize called\n");fflush(stdout);
}

/* Set the pen mode of the Graphic Port */
void XXSetPenMode(FRAME frame,int mode)
{ 
  printf("XXSetPenMode called\n");fflush(stdout);
}


/* Draw a line in the graphic port */
void XXDrawLine(FRAME frame,int x,int y,int x1,int y1)
{
  printf("XXDrawLine called\n");fflush(stdout);
}

/* Draw a point in the Graphic Port */
void XXDrawPoint(FRAME frame,int x,int y)
{
  printf("XXDrawPoint called\n");fflush(stdout);
}

/* Draw an ellipse in a rect */
void XXDrawEllipse(FRAME frame,int x,int y,int dx, int dy)
{
  printf("XXDrawEllipse called\n");fflush(stdout);
}

/* Fill an ellipse in a rect */
void XXFillEllipse(FRAME frame,int x,int y,int dx, int dy)
{
  printf("XXFillEllipse called\n");fflush(stdout);
}


/* Draw a rectangle */
void XXDrawRect(FRAME frame,int x,int y,int dx,int dy)
{
  printf("XXDrawRect called\n");fflush(stdout);
}

/* Fill a rectangle */
void XXFillRect(FRAME frame,int x,int y,int dx,int dy)
{
  printf("XXFillRect called\n");fflush(stdout);
} 

/* Set the clip rect */
void XXSetClipRect(FRAME frame, int x, int y, int w, int h)
{
  printf("XXSetClipRect called\n");fflush(stdout);
}


/*
 * Managing Pixmaps
 */

void XXAllocPixMap(int w,int h,unsigned char **pData,int *pRowBytes)
{
  printf("XXAllocPixMap called\n");fflush(stdout);
}

void XXDeletePixMap(void)
{  
  printf("XXDeletePixMap called\n");fflush(stdout);
}
  
/* Display an image in a frame */
void XXDisplayPixMap(FRAME frame,int winX,int winY)
{
  printf("XXDisplayPixMap called\n");fflush(stdout);
}

char XXIsDisplayBLittle(void)
{ printf("XXIsDisplayBLittle called\n");fflush(stdout);
  return(NO);
}

void XXFlush(void)
{
  printf("XXFlush called\n");fflush(stdout);
}

/********************************************************/
/*                Other functions                       */
/********************************************************/

/* Delete a window */
void XXDeleteFrame(FRAME frame)
{
  printf("XXDeleteFrame called\n"); fflush(stdout);
}

/* Create a window */
FRAME XXNewFrame(char *title,int x, int y, int w,int h)
{ printf("XXNewFrame called\n");fflush(stdout);
  return(NULL);
}

/* Quand c'est juste la position ou le titre qui change ne pas faire d'expose ni configure */
/* Quand la taille change avec enlever juste le configure et garder l'expose */
/* Change the size the position and the title of a window */
void XXChangeFrame(FRAME frame,char *title,int x,int y,int w, int h)
{
  printf("XXChangeFrame called\n");fflush(stdout);
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
  printf("XXAutoRepeatOn called\n");fflush(stdout);
}

/* Turn the autorepeat off */
void XXAutoRepeatOff(void)
{
  printf("XXAutoRepeatOff called\n");fflush(stdout);
}

/*
void ANSIXXGetNextEvent(EVENT event,int flagWait)
{
  static char str[10000];
  static n = 0;
  static flagUp = NO;
  char *str1;
  
  // DEBUG
  printf("C: XXGetNextEvent is being called\n");fflush(stdout);
  if (flagUp == YES) {
    event->type = KeyUp;
    event->object = NULL;
    event->key = str[n];
    flagUp = NO;
    n++;
    printf("keyNO= '%c'\n",event->key);fflush(stdout);
    return;
  }
    
  if (n==0 || str[n] == '\0') {
    printf("C: gets is being called ...");fflush(stdout);
    gets(str);
    printf("done str = %s\n",str);fflush(stdout);
    strcat(str,"\n");
    n = 0;
  }
  
  event->type = KeyDown;
  event->object = NULL;
  event->key = str[n];
  flagUp = YES;
  printf("keyYES= '%c'\n",event->key);fflush(stdout);
  printf("C: leaving XXGetNextEvent\n");fflush(stdout);
}
*/


/* Change the color of the Graphic port */
void XXSetColor(FRAME frame, unsigned long pixel)
{
  printf("XXSetColor called\n");fflush(stdout);
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
  printf("XXSetColorMap called\n");fflush(stdout);
 return(100);
}


/*
 * Animate one color of the colormap
 */

void XXAnimateColor(unsigned long pixel, unsigned short r, unsigned short g, unsigned short b)
{
  printf("XXAnimateColor called\n");fflush(stdout);
}


/*
 * Returns the number of colors available 
 */
int XXNumOfColors(void)
{ printf("XXNumOfColors called\n");fflush(stdout);
return(65000);

}

/*
 * Returns the depth
 */
int XXDepth(void)
{printf("XXDepth being called\n");fflush(stdout);
return(16);
}

/*
 * Is the screen BW ?
 */

int XXIsBWScreen(void)
{printf("XXIsBWScreen called\n");fflush(stdout);
return(NO);
}

/*
 * Type of the screen
 */

char * XXScreenType(void)
{printf("XXScreenType called\n");fflush(stdout);
  return("ansi");
}

void XXScreenRect(int *x, int *y, int *w, int *h)
{printf("XXScreenRect called\n");fflush(stdout);
  *x = *y = 0;
  *w = *h = 1000;
}

/*
 * Get the foreground and background colors of the terminal window 
 */

void XXBgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  printf("XXBgColor called\n");fflush(stdout);
}

void XXFgColor(unsigned short *r,unsigned short *g,unsigned short *b)
{
  printf("XXFgColor called\n");fflush(stdout);
}




/* Draw a string in the graphic port */
void XXDrawString(FRAME frame,int x,int y,char *str)
{
  printf("XXDrawString called\n");fflush(stdout);
}


char *XXGetDefaultFont(FONTID *fontStruct,int *size) 
{printf("XXGetDefaultFont called\n");fflush(stdout);
  return("none");
}


void XXGetFontInfo(FONT font, int *ascent, int *descent, int *interline)
{
  printf("XXGetFontInfo called\n");fflush(stdout);
}

int XXGetStringWidth(FONT font, char *text)
{printf("XXGetStringWidth called\n");fflush(stdout);
return(10);
}

void XXSetFont(FRAME frame,FONT font)
{
  printf("XXSetFont called\n"); fflush(stdout);
}


void XXFontMatch(char *name,char flagSize, int size,char flagStyle,unsigned char style)
{
  printf("XXFontMatch called\n");fflush(stdout);
}


char XXExistFont(char *name,int size, unsigned char style,FONTID *fontStruct)
{
  printf("XXExistFont called\n");fflush(stdout);
  return(YES);
}


void C_System(char **argv)
{
  printf("C_System called\n");fflush(stdout);
}

#endif //LASTWAVE_JAVA_GRAPHICS

