#include "computer.h"

#ifdef LASTWAVE_JAVA_GRAPHICS
#include "lastwave.h"
/*	This listing is a test for LastWave...
**	It creates a Java String object
**	from a chain of characters,
**	arbitrarily obtains the JNIEnv pointer
**	and calls printString(String) in
**	JavaTerminal.java
*/


#include <stdio.h>
#include "JavaTerminal.h"
#include "java_keysyms.h"

/* 	Global variables for caching
 * - the Java Virtual Machine,
 * - the Java Class JavaTerminal
 */
JavaVM *cached_pJVM = NULL;



/* 	This function is called when the native library
**	is loaded by Java - i.e. called before the native main method
*/

JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *pJVM, void *reserved)
{	JNIEnv *env;

// DEBUG
	printf("C: on load function is being called\n");


	if((*pJVM)->GetEnv(pJVM,(void **)&env,JNI_VERSION_1_2)){
		printf("jni version not supported");
		return JNI_ERR; //JNI version not supported
	}

	cached_pJVM = pJVM; /*cache the JVM pointer*/

	return JNI_VERSION_1_2;
}
/* 	Global variables for caching
 * - the Java Env interface pointer for the terminal,
 * - the Java Class JavaTerminal,
 * - the Java Instance object
 */

jobject cachedObject = NULL;
JNIEnv *cachedEnv = NULL;
jclass JavaTerminalClass = NULL;
BUFFER terminalInputBuffer = NULL;
BUFFER terminalKeyCodeBuffer = NULL;
#define TERMINPUT_BUFFERSIZE 100
jint JVK_F1  =  0;
jint JVK_F2  =  0;
jint JVK_F3  =  0;
jint JVK_F4  =  0;
jint JVK_F5  =  0;
jint JVK_F6  =  0;
jint JVK_F7  =  0;
jint JVK_F8  =  0;
jint JVK_F9  =  0;
jint JVK_F10 =  0;
jint JVK_F11 =  0;
jint JVK_F12 =  0;
jint JVK_BACKSPACE = 0;
jint JVK_DELETE = 0;
jint JVK_NEWLINE = 0;
jint JVK_ESCAPE = 0;
jint JVK_SPACE = 0;
jint JVK_SHIFT = 0;
jint JVK_CONTROL = 0;
jint JVK_LEFT = 0;
jint JVK_RIGHT = 0;
jint JVK_UP = 0;
jint JVK_DOWN = 0;

JNIEXPORT void JNICALL Java_JavaTerminal_initJNITerminal
  (JNIEnv *env, jobject obj)
{
	jclass cls;
        jclass keyEventCls;
        jfieldID keyEventID;
	jmethodID constructorID;
	printf("C: initJNITerminal being called");

	cachedEnv = env;
	cachedObject = obj;
	cls = (*env)->FindClass(env, "JavaTerminal");
	if(cls == NULL){
		printf("Cannot find JavaTerminal.class");
		return;
	}

	// Use weak global ref to allow C class to be unloaded
	JavaTerminalClass = (*env)->NewWeakGlobalRef(env, cls);
	if(JavaTerminalClass == NULL){

		return;
	}
        keyEventCls = (*env)->FindClass(env, "java/awt/event/KeyEvent");
	if(keyEventCls == NULL){
	  printf("Can't find keyEventCls\n");
	}


	if(cachedObject == NULL){
		printf("Cannot find JObject theObject relating to class JavaTerminal!!!");
		return;
	}
        // Gets the field ID for the KeyEvent class
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F1", "I");
	JVK_F1     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("\n#define JK_F1 %#x\n",JVK_F1);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F2", "I");
	JVK_F2     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F2 %#x\n",JVK_F2);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F3", "I");
	JVK_F3     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F3 %#x\n",JVK_F3);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F4", "I");
	JVK_F4     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F4 %#x\n",JVK_F4);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F5", "I");
	JVK_F5     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F5 %#x\n",JVK_F5);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F6", "I");
	JVK_F6     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F6 %#x\n",JVK_F6);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F7", "I");
	JVK_F7     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F7 %#x\n",JVK_F7);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F8", "I");
	JVK_F8     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F8 %#x\n",JVK_F8);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F9", "I");
	JVK_F9     = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F9 %#x\n",JVK_F9);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F10", "I");
	JVK_F10    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F10 %#x\n",JVK_F10);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F11", "I");
	JVK_F11    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F11 %#x\n",JVK_F11);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_F12", "I");
	JVK_F12    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_F12 %#x\n",JVK_F12);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_BACK_SPACE", "I");
	JVK_BACKSPACE    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_BACKSPACE %#x\n",JVK_BACKSPACE);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_DELETE", "I");
	JVK_DELETE    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_DELETE %#x\n",JVK_DELETE);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_ENTER", "I");
	JVK_NEWLINE    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_NEWLINE %#x\n",JVK_NEWLINE);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_ESCAPE", "I");
	JVK_ESCAPE    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_ESCAPE %#x\n",JVK_ESCAPE);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_SPACE", "I");
	JVK_SPACE    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_SPACE %#x\n",JVK_SPACE);
        keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_SHIFT", "I");
	JVK_SHIFT    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_SHIFT %#x\n",JVK_SHIFT);
        keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_CONTROL", "I");
	JVK_CONTROL    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_CONTROL %#x\n",JVK_CONTROL);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_LEFT", "I");
	JVK_LEFT    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_LEFT %#x\n",JVK_LEFT);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_RIGHT", "I");
	JVK_RIGHT    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_RIGHT %#x\n",JVK_RIGHT);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_UP", "I");
	JVK_UP    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_UP %#x\n",JVK_UP);
	keyEventID = (*env)->GetStaticFieldID(env, keyEventCls, "VK_DOWN", "I");
	JVK_DOWN    = (*env)->GetStaticIntField(env, keyEventCls, keyEventID);
	printf("#define JK_DOWN %#x\n",JVK_DOWN);



	// Allocates the input buffer
	terminalInputBuffer = NewBuffer(TERMINPUT_BUFFERSIZE);
	terminalKeyCodeBuffer = NewBuffer(TERMINPUT_BUFFERSIZE);
}

/*
**	This function releases the WeakGlobalRef to JavaTerminal when the library is unloaded
*/

JNIEXPORT void JNICALL
JNI_OnUnload(JavaVM *pJVM, void *reserved){
	JNIEnv *env;

        printf("C: on unload function is being called\n");

	if((*pJVM)->GetEnv(pJVM, (void **)&env, JNI_VERSION_1_2)){
		return;
	}
	(*env)->DeleteWeakGlobalRef(env, JavaTerminalClass);

	return;
}

/*
**	This is a utility function to get a JNIEnv pointer for the current thread
*/
JNIEnv *JNU_GetEnv()
{
	JNIEnv *env;
	//(*cached_pJVM)->GetEnv(cached_pJVM,(void **)&env,JNI_VERSION_1_2);
	(*cached_pJVM)->AttachCurrentThread(cached_pJVM,(void **)&env,NULL);
	return env;
}

/*
**	This utility function converts a chain of characters into a jstring
**	- jstring is equivalent to java/lang/String
*/
jstring Newjstring(char *str)
{

	JNIEnv *env = cachedEnv;
	jstring result;

	result =  (*env)->NewStringUTF(env, str);
	if(result == NULL)
	{
	    		printf("Warning: (Newjstring) result jstring = NULL!!!!\n");fflush(stdout);
		return NULL;
	}
	else{
	    //		printf("result jstring is OK !!!\n");fflush(stdout);
		return result;
	}
}

/****This is a simple test for sending a character to Java****
*/
void XXTerminalPrintChar(char c)
{
	JNIEnv *env;
	jclass cls = JavaTerminalClass;

	jobject obj = cachedObject;
	jmethodID mid;

	// DEBUG
	//	printf("C: XXTerminalPrintChar is being called\n");
	env = cachedEnv;
	mid = (*env)->GetMethodID(env,cls,"printChar", "(C)V");
	if(mid == NULL){
		printf("Warning: Cannot find JMethod printChar!!!");
		return;
	}
	if(((*cached_pJVM)->GetEnv(cached_pJVM, (void **)&env, JNI_VERSION_1_2)) == JNI_OK)
	{
	  // printf("XXTerminalPrintChar : '%c'\n",c);
		(*env)->CallVoidMethod(env,obj,mid, c);
	}
}


/* 	This method does the following :
**	*	Obtains a JNIEnv pointer and object reference
**	*	Obtains method ID for JavaTerminal/printString(String myString)
**	*	Calls printString with parameter jstring
****** Will this implementation just slot into LastWave? ******
*/
void XXTerminalPrintStr(char *str)
{	JNIEnv *env;
	jclass cls = JavaTerminalClass;

	jobject obj = cachedObject;
	jmethodID mid;
	jstring result = NULL;

	// DEBUG
		printf("C: XXTerminalPrintStr is being called\n");

	result = Newjstring(str);

	env = cachedEnv;
	mid = (*env)->GetMethodID(env,cls,"printString","(Ljava/lang/String;)V");
	if(mid == NULL){
		printf("Warning :Cannot find JMethod printString!!!");
		return;
	}
	if(((*cached_pJVM)->GetEnv(cached_pJVM, (void **)&env, JNI_VERSION_1_2)) == JNI_OK)
	{
	  // printf("XXTerminalPrintStr : '%s'\n",str);
	    //		printf("C: calling CallVoidMethod");fflush(stdout);
		(*env)->CallVoidMethod(env,obj,mid, result);
		//		printf("Done\n");fflush(stdout);

		//		printf("C: Releasing chars");fflush(stdout);
		//		(*env)->ReleaseStringUTFChars(env, result, str);
		//		printf("Done\n");fflush(stdout);
	}

}


void XXInitTerminal(void)
{
   printf("C: XXInitTerminal is being called\n");
}

void XXCloseTerminal(void)
{
  printf("C: XXCloseTerminal is being called\n");
}
/*
 * Make a beep
 */
void XXTerminalBeep(void)
{	JNIEnv *env = cachedEnv;
	jclass cls = JavaTerminalClass;

	jobject obj = cachedObject;
	jmethodID mid;


	mid = (*env)->GetMethodID(env,cls,"beep","()V");
	if(mid == NULL){
		printf("Warning :Cannot find JMethod beep!!!");
		return;
	}
	if(((*cached_pJVM)->GetEnv(cached_pJVM, (void **)&env, JNI_VERSION_1_2)) == JNI_OK)
	{
	    printf("XXTerminalBeep - trying to call java");

		(*env)->CallVoidMethod(env,obj,mid);

	}

}

void  XXTerminalPrintErrStr(char *str)
{
  //printf("C: XXTerminalPrintErrStr is being called\n");
  XXTerminalPrintStr(str);
}
/*
 * Just flushes the terminal output
 */
void XXTerminalFlush(void)
{
  // printf("C: XXTerminalFlush is being called\n");
}
/*
 * Make the cursor go forward by n space
 */
void  XXTerminalCursorGoForward(int n)
{
  //printf("C: XXTerminalCursorGoForward is being called\n");
}
/*
 * Make the cursor go backward by one space
 */
void  XXTerminalCursorGoBackward(int n)
{
  //printf("C: XXTerminalCursorGoBackward is being called\n");
}
/*
 * Print an editable character from the cursor position
 * The 'str' corresponds to what is already displayed on the screen
 * and should be redrawn (shifted by 1 character).
 * The cursor should stay at the same position.
 */
void  XXTerminalInsertChar(char c,char *str)
{
  // Quick and Dirty implementation that does NOT do what it is expected to do
  //char mystring[1000];
  //printf("C: XXTerminalInsertChar is being called\n");
  //sprintf(mystring,"%c%s",c,str);
  // XXTerminalPrintStr(mystring);
  XXTerminalPrintChar(c);
}
/*
 * Same as above but "inserts" a delete character n times
 */
void  XXTerminalDeleteInsertChar(int n, char *str)
{

  JNIEnv *env = cachedEnv;
  jclass cls = JavaTerminalClass;

  jobject obj = cachedObject;
  jmethodID mid;
  mid = (*env)->GetMethodID(env,cls,"deleteChar","()V");
	if(mid == NULL){
		printf("Warning :Cannot find JMethod deleteChar!!!");
		return;
	}
	if(((*cached_pJVM)->GetEnv(cached_pJVM, (void **)&env, JNI_VERSION_1_2)) == JNI_OK)
	{
	    printf("XXTerminalDeleteInsertChar - trying to call java");

		(*env)->CallVoidMethod(env,obj,mid);

	}


}
/* Move the terminal window */
void XXTerminalMoveWindow(int x,int y)
{
  printf("C: XXTerminalMoveWindow is being called\n");
}
/* Resize the terminal window */
void XXTerminalResizeWindow(int w,int h)
{
  printf("C: XXTerminalResizeWindow is being called\n");
}


static char* callname="lastwave";
// The function called by Java to start LastWave main loop
JNIEXPORT void JNICALL Java_JavaTerminal_lw_1main(JNIEnv *env, jobject obj, jobjectArray args) {
  int argc;
  char **argv;
  //  lw_main(0,NULL);
  // DEBUG
  printf("C: main function is being called\n");
  main(1,&callname);
}

JNIEXPORT void JNICALL Java_JavaTerminal_sendKeyChar(JNIEnv *env, jobject obj, jchar c,jint flagUp) {
  /* if(flagUp) {
    printf("C: sendKeyChar : '%c' UP\n",c);

  }
  else {
   printf("C: sendKeyChar : '%c' DOWN\n",c);
   }*/
  PushBuffer(terminalInputBuffer,c);
}

JNIEXPORT void JNICALL Java_JavaTerminal_sendKeyCode(JNIEnv *env, jobject obj, jint k,jint flagUp) {
  /*if(flagUp) {
    printf("C: sendKeyCode : '%ld' UP\n",k);
   }
  else {
   printf("C: sendKeyCode : '%ld' DOWN\n",k);
    }*/
    printf("In sendkeycode\n");
      PushBuffer(terminalKeyCodeBuffer,k);
}

void XXGetNextEvent(EVENT event,int flagWait)
{ jint javaKeyChar;
  jint javaKeyCode;
  static char flagUp = NO;

  event->type = NoEvent;
  event->key = 0;
  event->button = NoButton;

  // DEBUG
  //printf("C: XXGetNextEvent is being called\n");fflush(stdout);


  // Wait for the inout buffer to be non empty
  while((javaKeyChar = PullBuffer(terminalInputBuffer)) == 0) {}

  javaKeyCode = PullBuffer(terminalKeyCodeBuffer);
  event->object = NULL;


    switch(javaKeyCode){
    case JK_BACKSPACE  : case JK_DELETE : event->key = DeleteKC; break;
    case JK_NEWLINE : event->key = NewlineKC; break;
    case JK_ESCAPE : event->key = EscapeKC; break;
    case JK_SPACE :  event->key = ' '; break;
    case JK_F1: event->key = F1KC; break;
    case JK_F2: event->key = F2KC; break;
    case JK_F3: event->key = F3KC; break;
    case JK_F4: event->key = F4KC; break;
    case JK_F5: event->key = F5KC; break;
    case JK_F6: event->key = F6KC; break;
    case JK_F7: event->key = F7KC; break;
    case JK_F8: event->key = F8KC; break;
    case JK_F9: event->key = F9KC; break;
    case JK_F10: event->key = F10KC; break;
    case JK_F11: event->key = F11KC; break;
    case JK_F12: event->key = F12KC; break;
    case JK_SHIFT: case JK_CONTROL:  break;
    case JK_LEFT:case JK_RIGHT: case JK_UP: case JK_DOWN: break;
    default: event->key = javaKeyChar;




  }
  event->type = KeyDown;
    event->x = event->y =  0;
    return;

    //printf("C: leaving XXGetNextEvent\n");fflush(stdout);
}


#endif // LASTWAVE_JAVA_GRAPHICS
