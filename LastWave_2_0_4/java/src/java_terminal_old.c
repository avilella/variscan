#include "lastwave.h" 

#ifdef LASTWAVE_JAVA_GRAPHICS

#include <stdio.h>
#include "java_main.h"

static char* callname="lastwave";
// The function called by Java to start LastWave main loop
JNIEXPORT void JNICALL Java_java_1main_lw_1main(JNIEnv *env, jobject obj, jobjectArray args) {
  int argc;
  char **argv;
  //  lw_main(0,NULL);
  main(1,&callname);
}

// Some global variables, we need to get them!
JNIEnv *theEnv;
jobject theObj;

// Called whenever the JAVA terminal receives a new character input
//static void  JavaTerminalGetNewChar(unsigned char c)
//{
//  XKeyEvent event;
//
//  event.keycode = c;
//  event.window = myEventWindow;
//  event.send_event = True;
//  event.type = KeyPress;
/*   event.display = myDisplay; */
   /* Then send the event */ 
/*   XSendEvent(myDisplay,myEventWindow,False,KeyPressMask,(XEvent *) (&event)); */
/*   XFlush(myDisplay);  */
/* } */

// This function is only called by Java!
JNIEXPORT void JNICALL Java_java_1main_terminalPushChar(JNIEnv *env, jobject object, jchar c) {
  TermBufferPushKey((unsigned long)c);
  printf("%c",c);Flush();
  //  *theEnv = *env;
  //  theObj = object;
}


void XXInitTerminal(void)
{}

void XXCloseTerminal(void)
{}
/*
 * Make a beep
 */
void XXTerminalBeep(void)
{}
/*
 * Just print a string (not editable ==> no track of the cursor)
 */
void  XXTerminalPrintStr(char *str)
{
  printf("[XXTerminalPrintStr]%s", str);
}

void  XXTerminalPrintErrStr(char *str)
{}
/*
 * Same thing as above but with a character
 */
JNIEXPORT jchar JNICALL Java_java_1main_terminalPrintChar(JNIEnv *env, jobject obj, jchar input)
{	
  return input;
}
void  XXTerminalPrintChar(char c)
{
  //  Java_java_1main_terminalPrintChar(theEnv,theObj,c);
  printf("[XXTerminalPrintChar]%c",c);fflush(stdout);
}
//// When Java needs to call XXTerminalPrintChar, 
//// it uses the following interface 
//JNIEXPORT
//void JNICALL Java_java_1main_XXTerminalPrintChar(JNIEnv *env, jobject obj, jchar ch)
//{	
//  XXTerminalPrintChar(ch);	
//}


/*
 * Just flushes the terminal output
 */
void XXTerminalFlush(void)
{}
/*
 * Make the cursor go forward by n space
 */
void  XXTerminalCursorGoForward(int n)
{}
/*
 * Make the cursor go backward by one space
 */
void  XXTerminalCursorGoBackward(int n)
{}
/*
 * Print an editable character from the cursor position
 * The 'str' corresponds to what is already displayed on the screen
 * and should be redrawn (shifted by 1 character).
 * The cursor should stay at the same position.
 */
void  XXTerminalInsertChar(char c,char *str)
{}
/*
 * Same as above but "inserts" a delete character n times
 */
void  XXTerminalDeleteInsertChar(int n, char *str)
{}
/* Move the terminal window */
void XXTerminalMoveWindow(int x,int y)
{}
/* Resize the terminal window */
void XXTerminalResizeWindow(int w,int h)
{
}



//JNIEXPORT jobject JNICALL
//Java_java_1main_specialKeyCode(JNIEnv *env, jobject obj, jobject keyEvent)
//{
//	return keyEvent;
//}

#endif // LASTWAVE_JAVA_GRAPHICS
