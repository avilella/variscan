/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class JavaTerminal */

#ifndef _Included_JavaTerminal
#define _Included_JavaTerminal
#ifdef __cplusplus
extern "C" {
#endif
/* Inaccessible static: LOCK */
/* Inaccessible static: dbg */
/* Inaccessible static: isInc */
/* Inaccessible static: incRate */
#undef JavaTerminal_TOP_ALIGNMENT
#define JavaTerminal_TOP_ALIGNMENT 0.0f
#undef JavaTerminal_CENTER_ALIGNMENT
#define JavaTerminal_CENTER_ALIGNMENT 0.5f
#undef JavaTerminal_BOTTOM_ALIGNMENT
#define JavaTerminal_BOTTOM_ALIGNMENT 1.0f
#undef JavaTerminal_LEFT_ALIGNMENT
#define JavaTerminal_LEFT_ALIGNMENT 0.0f
#undef JavaTerminal_RIGHT_ALIGNMENT
#define JavaTerminal_RIGHT_ALIGNMENT 1.0f
#undef JavaTerminal_serialVersionUID
#define JavaTerminal_serialVersionUID -7644114512714619750LL
/* Inaccessible static: metrics */
/* Inaccessible static: class_00024java_00024awt_00024Component */
/* Inaccessible static: class_00024java_00024awt_00024event_00024ComponentListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024FocusListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024HierarchyListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024HierarchyBoundsListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024KeyListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024MouseListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024MouseMotionListener */
/* Inaccessible static: class_00024java_00024awt_00024event_00024InputMethodListener */
#undef JavaTerminal_serialVersionUID
#define JavaTerminal_serialVersionUID 4613797578919906343LL
/* Inaccessible static: dbg */
/* Inaccessible static: class_00024java_00024awt_00024Container */
/* Inaccessible static: class_00024java_00024awt_00024event_00024ContainerListener */
#undef JavaTerminal_OPENED
#define JavaTerminal_OPENED 1L
/* Inaccessible static: nameCounter */
#undef JavaTerminal_serialVersionUID
#define JavaTerminal_serialVersionUID 4497834738069338734LL
/* Inaccessible static: dbg */
/* Inaccessible static: class_00024java_00024awt_00024Container */
/* Inaccessible static: class_00024java_00024awt_00024event_00024WindowListener */
#undef JavaTerminal_DEFAULT_CURSOR
#define JavaTerminal_DEFAULT_CURSOR 0L
#undef JavaTerminal_CROSSHAIR_CURSOR
#define JavaTerminal_CROSSHAIR_CURSOR 1L
#undef JavaTerminal_TEXT_CURSOR
#define JavaTerminal_TEXT_CURSOR 2L
#undef JavaTerminal_WAIT_CURSOR
#define JavaTerminal_WAIT_CURSOR 3L
#undef JavaTerminal_SW_RESIZE_CURSOR
#define JavaTerminal_SW_RESIZE_CURSOR 4L
#undef JavaTerminal_SE_RESIZE_CURSOR
#define JavaTerminal_SE_RESIZE_CURSOR 5L
#undef JavaTerminal_NW_RESIZE_CURSOR
#define JavaTerminal_NW_RESIZE_CURSOR 6L
#undef JavaTerminal_NE_RESIZE_CURSOR
#define JavaTerminal_NE_RESIZE_CURSOR 7L
#undef JavaTerminal_N_RESIZE_CURSOR
#define JavaTerminal_N_RESIZE_CURSOR 8L
#undef JavaTerminal_S_RESIZE_CURSOR
#define JavaTerminal_S_RESIZE_CURSOR 9L
#undef JavaTerminal_W_RESIZE_CURSOR
#define JavaTerminal_W_RESIZE_CURSOR 10L
#undef JavaTerminal_E_RESIZE_CURSOR
#define JavaTerminal_E_RESIZE_CURSOR 11L
#undef JavaTerminal_HAND_CURSOR
#define JavaTerminal_HAND_CURSOR 12L
#undef JavaTerminal_MOVE_CURSOR
#define JavaTerminal_MOVE_CURSOR 13L
#undef JavaTerminal_NORMAL
#define JavaTerminal_NORMAL 0L
#undef JavaTerminal_ICONIFIED
#define JavaTerminal_ICONIFIED 1L
/* Inaccessible static: nameCounter */
#undef JavaTerminal_serialVersionUID
#define JavaTerminal_serialVersionUID 2673458971256075116LL
/* Inaccessible static: class_00024java_00024awt_00024Frame */
#undef JavaTerminal_EXIT_ON_CLOSE
#define JavaTerminal_EXIT_ON_CLOSE 3L
/*
 * Class:     JavaTerminal
 * Method:    lw_main
 * Signature: ([Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_JavaTerminal_lw_1main
  (JNIEnv *, jobject, jobjectArray);

/*
 * Class:     JavaTerminal
 * Method:    initJNITerminal
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_JavaTerminal_initJNITerminal
  (JNIEnv *, jobject);

/*
 * Class:     JavaTerminal
 * Method:    sendKeyChar
 * Signature: (CI)V
 */
JNIEXPORT void JNICALL Java_JavaTerminal_sendKeyChar
  (JNIEnv *, jobject, jchar, jint);

/*
 * Class:     JavaTerminal
 * Method:    sendKeyCode
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_JavaTerminal_sendKeyCode
  (JNIEnv *, jobject, jint, jint);

#ifdef __cplusplus
}
#endif
#endif