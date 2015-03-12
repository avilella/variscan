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
/*   gobject.h :                  The graphic structures            */
/*                                                                  */
/********************************************************************/


/*******************************************************/
/* This is the machine dependant definition of a frame */
/*******************************************************/

#ifndef FRAME
#define FRAME int *
#endif

#ifndef FONTID
#define FONTID int *
#endif


/* 
 * The structure for rectType
 */
 
 typedef struct rectType {
   short left;  
   short top;  
   short right;  
   short bottom;  
 } RectType;
 
  
/* 
 * The abstract structure of a graphic object
 */

#define GObjectFields \
\
  /* GObjects will be put in a Hash Table thus we must include the corresponding fields */ \
  AHashElemFields; \
\
  /* They will also be linked in order to take into account the front/back */ \
  struct gobject *front; \
\
  /* the class the object belongs to */ \
  struct gClass *gclass; \
  struct gClass *classCur; \
  /* The father gobject */ \
  struct gList *father; \
\
  /* relative position */ \
  float rx, ry, rw, rh; \
\
  /* Absolute position */ \
  int x, y, w, h; \
  char flagGrid; \
  unsigned char i,j,m,n; \
  /* the font */ \
  struct font *font; \
  /* Is it hidden ? */ \
  char flagHide; \
\
  /* The background and foreGround colors */ \
  unsigned long fgColor, bgColor; \
\
  /* The pen size */ \
  unsigned char penSize; \
\
  /* The pen mode */ \
  unsigned char penMode; \
\
  /* The clip flag */ \
  unsigned char flagClip; \
\
  /* The line style */ \
  unsigned char lineStyle; \
\
  RectType rectType; \
  /* Should we draw a frame to the object ? */ \
  unsigned char flagFrame

 
typedef struct gobject {

 GObjectFields; 

} GObject, *GOBJECT;


extern const RectType LargeRect,SmallRect,NormalRect;

/*
 * The glist structure
 */

#define GListFields \
  GObjectFields;  \
    /* They will also be linked in order to take into account the front/back */ \
  struct gobject *back; \
  HASHTABLE theGObjects 
    
typedef struct gList {

  GListFields;
    
} GList, *GLIST;



/*
 * The view structure
 */

enum{
  YFlagReverse = 1,
  XFlagReverse = 2
};

#define ViewFields \
  GListFields; \
  unsigned char flagReverse; \
  float xMin,xMax,yMin,yMax 
   
typedef struct view {

  ViewFields;
  
} View, *VIEW;


/*
 * The Grid structure
 */

#define GridFields \
  GListFields; \
  unsigned char gridN,gridM; \
  short topMargin,bottomMargin,leftMargin,rightMargin; \
  short dx, dy
 
typedef struct grid {

  GridFields;
  
} Grid, *GRID;



/*
 * The window structure
 */

#define WindowFields \
  GridFields;  \
  FRAME frame; \
  char *title;  \
  char flag
 
typedef struct window {

  WindowFields;
  
} *WINDOW;
   

enum {
  WindowNoFlag,
  WindowFlagNoUpdate = 1,
  WindowFlagNoChangeFrame = 2
};

extern HASHTABLE theWindowsHT;
extern WINDOW theWindows;


/* 
 * The structure of a graphic object class
 */


#define GClassProtected (1l<<0)
#define GClassMoveResize (1l<<1)
#define GClassLocalCoor (1l<<2)
 
 
typedef struct gClass {

  /* GObjectClasses will be put in a Hash Table thus we must include the corresponding fields */
  AHashElemFields;
  
  /* The number of bytes of the asssociated GObjects */
  size_t nbBytes;
  
  /* The father class in the class hierarchy */
  struct gClass *fatherClass;

  /* Could be either NULL (for gobjects) GList or Window */
  struct gClass *typeClass;
  
  /* Some flags */
  unsigned long flags;
     
  /* Routine for drawing the gobject */
  void (*draw) (WINDOW, GOBJECT, int, int,int,int);
  
  /* Routine for deleting the content of a gobject */
  void (*deleteContent)(GOBJECT);  
  
  /* Routine for initializing the content of a gobject */
  void (*init)(GOBJECT);
  
  /* Routine for testing whether a point is in a gobject */
  float (*isIn)(GOBJECT,GOBJECT *, int, int);
  
  /* Routine for answering set/get messages */
  int (*set) (GOBJECT, char *field, char**argv);

  /* Routine for notifying that a gobject son will be deleted */
  char (*deleteNotify)(GOBJECT);  
  
  /* The eventual scommand name for receiving set/get messages */
  PROC setSCommand;

  /* Routine for answering messages */
  char (*msge) (GOBJECT, char *msge, char**argv);

  /* The eventual scommand name for receiving messages (except draw and isin messages) */
  PROC msgeSCommand;

  /* The eventual scommand name for receiving draw messages */
  PROC drawSCommand;

  /* The eventual scommand name for receiving isin messages */
  PROC isInSCommand;

  /* The eventual variable type the class is associated */
  char *varType;

  /* The eventual package name this class is attached to */
  char *packageName;
  
  /* Some info about the class */
  char *info;
  
  /* The binding of this class */
  struct binding * theBindings[8] /*???????????????????????*/;
  
} GClass, *GCLASS;

  
  
extern HASHTABLE theGClasses;

extern GCLASS theGObjectClass;
extern GCLASS theGListClass;
extern GCLASS theWindowClass;
extern GCLASS theViewClass;
extern GCLASS theGridClass;

#define MaxClassHierarchyDepth 100



extern GOBJECT NewGObject(GCLASS gclass, GLIST father, char *name, char **argv,char flagDraw);
extern char DeleteGObject(GOBJECT o);
extern void DrawGObject(GOBJECT o, int x,int y,int w,int h,char);
extern void DrawWholeGObject(GOBJECT o,char);
extern void DrawGObjectList(GOBJECT *gobjects,int x0, int y0, int w0 , int h0);
extern void DrawWholeGObjectList(GOBJECT *gobjects);
extern void BackGObject(GOBJECT o,char flagDraw);
extern void FrontGObject(GOBJECT o,char flagDraw);
extern float IsInGObject(GOBJECT o, GOBJECT *o1, int x, int y);
extern char  SetGObject(GOBJECT o, char **argv, char flagUpdate);
extern void ShowGObject(GOBJECT o);
extern void HideGObject(GOBJECT o);
extern void MoveResizeDrawGObject(GOBJECT o,float x, float y, float w, float h);
extern void Local2Global(GOBJECT o,float x, float y, int *mx, int *my);
extern void Global2Local(GOBJECT o,int mx, int my, float *x, float *y);

extern void Local2GlobalRect(GOBJECT o,float x, float y, float w, float h,  RectType rectType, int *mx, int *my, int*mw, int*mh);
extern void Global2LocalRect(GOBJECT o,int mx, int my, int mw, int mh, float *x, float *y, float *w, float *h, RectType rectType);
extern char *GetNameGObject(GOBJECT o);
extern void Rect2GRect(int *x, int *y, int *w, int *h,RectType rectType,int signW, int signH); 
extern void GRect2Rect(int *x, int *y, int *w, int *h, RectType rectType);
extern void UpdateLocalRectGObject(GOBJECT o);
extern void UpdateGlobalRectGObject(GOBJECT o);
extern char GetVisibleRect(GOBJECT o,int *x, int *y, int *w, int *h);
extern char UnionRect1(int x,int y,int w,int h,int *x1,int *y1,int *w1,int *h1);

extern char IsGList(GOBJECT o);
extern void AddGObject2GList(GOBJECT o,  GLIST list);
extern void RemoveGObject2GList(GOBJECT o, GLIST list);

extern char IsWin(GOBJECT o);
extern WINDOW NewWin(GCLASS gclass, char *name, char **argv);
extern WINDOW GetWin(GOBJECT o);

extern char IsView(GOBJECT o);

extern char IsSubClass(GCLASS subClass, GCLASS gclass);
extern GCLASS NewGClass(char *name,GCLASS fatherClass,char *packageName);


extern void InitGraphics(void);
extern void CloseGraphics(void);

extern void InitGUpdates(void);




#include "event.h"

extern int ProcessNextEvent(int flagWait);


/*
 * Color variables and functions (in WINDOW_COLOR.c) 
 */


extern unsigned long fgColor,bgColor,mColor;
extern const unsigned long invisibleColor;


extern unsigned long GetColorMapCur(void);
extern int ColorMapSize(unsigned long colorMap);
extern char *GetColorMapName (unsigned long cm); 

extern unsigned long Color2Pixel(unsigned long color);
extern char *GetColorName(unsigned long color);
extern void Color2RGB(unsigned long color, unsigned short *r,unsigned short *g,unsigned short *b);
extern int BuildColormap(char flagShared,char mouseMode);
extern void DefineColorRGB(int cm, int n, float r, float g, float b);
extern void DefineColorHSV(int cm, int n, float h, float s, float v);
extern void DefineNamedColorRGB(char *name, float r, float g, float b);
extern void DefineNamedColorHSV(char *name, float h, float s, float v);


#include "window_manager.h"
#include "xx_graphics.h"
#include "postscript.h"



