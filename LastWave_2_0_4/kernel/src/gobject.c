/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 1                           */
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



#include "lastwave.h"
#include <stdarg.h>


#define MaxSizeGObjectList 3000


/************************************************
 *
 *  Miscellenaous functions
 *
 ************************************************/

extern void ApplyProc2Listv(PROC proc, LISTV lv);
extern void ApplyProc2List(PROC proc, char **argv);

static char DrawMessage(GOBJECT o, WINDOW win, GCLASS c, int x, int y, int w, int h)
{
  char flag;
  LISTV lv1;

  if (c->draw == NULL && c->drawSCommand == NULL) return(NO);

  flag = toplevelCur->flagInDrawMessage;
  toplevelCur->flagInDrawMessage = YES;

  /* C function drawing ? */
  if (c->draw) (*(c->draw))(win,o,x,y,w,h);


  /* Script drawing ? */
  else {
  
    lv1 = TNewListv();

    SetLengthListv(lv1,5);

    lv1->values[0] = (VALUE) NewStrValue();
    SetStrValue((STRVALUE) lv1->values[0],GetNameGObject(o));

    lv1->f[1] = x;
    lv1->f[2] = y;
    lv1->f[3] = w;
    lv1->f[4] = h;
  
    ApplyProc2Listv(c->drawSCommand,lv1);
  }
  
  toplevelCur->flagInDrawMessage = flag;
}


/* Executing the script command 'scommand' with arguments being the 'object' name, the 'message' and 'listv' */
static void DoScriptListv(GOBJECT o,PROC scommand, char *message, LISTV lv, char flagStoreResult)
{
  LISTV lv1;
  int i;
  char oldFlagStoreResult;
  int n;
  
  if (message) n = 2; else n = 1;
  
  lv1 = TNewListv();

  if (lv != NULL) {
    SetLengthListv(lv1,lv->length+n);

    for (i=0;i<lv->length;i++) {
      if (lv->values[i] != NULL) {
        lv1->values[i+n] = lv->values[i];
        AddRefValue(lv->values[i]);
      }
      else lv1->f[i+n] = lv->f[i];
    }
  }
  else SetLengthListv(lv1,n);

  lv1->values[0] = (VALUE) NewStrValue();
  SetStrValue((STRVALUE) lv1->values[0],GetNameGObject(o));

  if (n == 2) {
    lv1->values[1] = (VALUE) NewStrValue();
    SetStrValue((STRVALUE) lv1->values[1],message);
  }
  
  switch (flagStoreResult) {
  case 0 :
    oldFlagStoreResult = toplevelCur->flagStoreResult;
    toplevelCur->flagStoreResult = 0;
    break;
  case 1 :
    oldFlagStoreResult = toplevelCur->flagStoreResult;
    toplevelCur->flagStoreResult = 1;
    break;
  }
  
  ApplyProc2Listv(scommand,lv1);

  switch (flagStoreResult) {
  case 0 : case 1:
    toplevelCur->flagStoreResult = oldFlagStoreResult;
    break;
  }
    
}


/* Executing the script command 'scommand' with arguments being the 'object' name, the 'message' and 'argv' */
static void DoScriptArgv(GOBJECT o, PROC scommand, char **argv)
{
  char str[200];
  char str1[200];
  char *list[100];
  int n;
   
  n= 0;
  str[0] = '"';
  strcpy(str+1,GetNameGObject(o));
  strcat(str,"\"");
  list[n++] = str;
  str1[0] = '"';
  strcpy(str1+1,argv[0]);
  strcat(str1,"\"");
  list[n++] = str1;
  argv++;
  while (1) {
    if (*argv == NULL) break;
    list[n++] = *argv;
    argv++;
  }

  list[n] = NULL;

  ApplyProc2List(scommand,list);
}



/**********************************************************************************
 *
 * Basic procedures to deal with local and global coordinates of points
 *
 **********************************************************************************/

/*
 * Conversion of a point from local (x,y) to global (*mx,*my) coordinates 
 */
void Local2Global(GOBJECT o,float x, float y, int *mx, int *my)
{
  VIEW view;
  
  /* Case the object is the root window */
  if (o == NULL) {
    *mx = x;
    *my = y;
    return;
  }
  
  /* Get the first object in the hierarchy which has local coordinates */
  while (o != NULL && !(o->gclass->flags & GClassLocalCoor)) o = (GOBJECT) o->father;
  if (o == NULL) Errorf("Local2Global() : Weird error");
  
  /* Case of a window */
  if (IsWin(o)) {
    *mx = x;
    *my = y;
    return;
  }

  /* Case of a non view object */
  if (!IsView(o)) {    
    if (o->rw == o->w) *mx = x+o->x;
    else *mx = x*o->w/o->rw+o->x;

    if (o->rh == o->h) *my = y+o->y;
    else *my = y*o->h/o->rh+o->y;
    return;
  }
  
  /* Case of a view */
  view = (VIEW) o;
  if (view->flagReverse & YFlagReverse) *my = (view->yMax-y)*(o->h-1)/(view->yMax-view->yMin)+o->y + .5;
  else *my = (y-view->yMin)*(o->h-1)/(view->yMax-view->yMin)+o->y + .5;
  if (view->flagReverse & XFlagReverse)   *mx = (view->xMax-x)*(o->w-1)/(view->xMax-view->xMin)+o->x + .5;
  else  *mx = (x-view->xMin)*(o->w-1)/(view->xMax-view->xMin)+o->x + .5;
}


/*
 * Conversion of a point from global (mx,my) to local (*x,*y) coordinates 
 */
void Global2Local(GOBJECT o,int mx, int my, float *x, float *y)
{
  VIEW view;

  /* Case the object is the root window */
  if (o == NULL) {
    *x = mx;
    *y = my;
    return;
  }

  /* Get the first object in the hierarchy which has a bounding box */
  while (o != NULL && !(o->gclass->flags & GClassLocalCoor)) o = (GOBJECT) o->father;
  if (o == NULL) Errorf("Global2Local() : Weird error");

  /* Case of a window */
  if (IsWin(o)) {
    *x = mx;
    *y = my;
    return;
  }

  /* Case of a non view object */
  if (!IsView(o)) {
    if (o->rw == o->w) *x = mx - o->x;
    else *x = (mx-o->x)*o->rw/o->w;

    if (o->rh == o->h) *y = my - o->y;
    else *y = (my-o->y)*o->rh/o->h;
    return;
  }
  
  /* Case of a view object */
  view = (VIEW) o;
  if (view->flagReverse & YFlagReverse)   *y = -(my-o->y)*(view->yMax-view->yMin)/(o->h-1)+view->yMax;
  else  *y = (my-o->y)*(view->yMax-view->yMin)/(o->h-1)+view->yMin;
  if (view->flagReverse & XFlagReverse)  *x = -(mx-o->x)*(view->xMax-view->xMin)/(o->w-1)+view->xMax;
  else  *x = (mx-o->x)*(view->xMax-view->xMin)/(o->w-1)+view->xMin;
}

/**********************************************************************************
 *
 * Basic procedures to deal with rectangles.
 *
 * Notice that a 'regular' rectangle in Lastwave is defined by
 * four integers : the position (x,y) of the top left corner and the
 * (w,h) width and the height (>0).
 * Such a rectangle is drawn using the basic XXDrawRectangle or is stored
 * in the (x,y,w,h) field of a gobject.
 * However LastWave allows the user to deal with more general definitions of rectangles
 * referred to as 'grect' (for general rectangles)
 * for which (w,h) are no longer necessary positive integers and for which boundaries can be 
 * extended in any directions. 
 * This is crucial for drawing using float coordinates (i.e, relative position in a view).
 * The type of a rectangle is coded in structure of tyupe RectType with four fields left,top,right,bottom
 * codind coding 
 * (using a short) how many points it is extended by respectively : left,top,right,bottom.
 * There are 3 predefined rectangleTypes :
 *    - LargeRect (=0,0,1,1) : both the points (x,y) and (x+w,y+h) belong to the rectangle
 *    - SmallRect (= -1,-1,0,0) : none of the points (x,y) and (x+w,y+h) belong to the rectangle
 *    - NormalRect (=0,0,0,0)  : the point (x,y) belong to the rectangle whereas the 
 *      point (x+w,y+h) do not belong to the rectangle 
 *
 * Warning : Let us note that, in a View, if the y-axis (resp. the x-axis) is reversed \
 * then the top-bottom (resp. left-right) are reversed.
 *
 **********************************************************************************/

/*
 * Initializes the different rectType constants 
 */
const RectType LargeRect = {0,0,1,1};
const RectType NormalRect = {0,0,0,0};
const RectType SmallRect = {-1,-1,0,0};


/*
 * Compute the intersection of two 'regular' rectangles
 * The result is in *x1,*y1,*w1,*h1
 * It return YES if intersection is not empty and NO otherwise 
 * (in that case *x1,*y1,*w1,*h1 values are not supposed to be used)
 */
 
static char IntersectRect(int x,int y,int w,int h,int *x1,int *y1,int *w1,int *h1)
{
  int xx,yy,ww,hh;
  
  xx = MAX(x,*x1);
  yy = MAX(y,*y1);
  ww = MIN(x+w,*x1+*w1)-xx;
  hh = MIN(y+h,*y1+*h1)-yy;

  if (hh <=0 || ww <= 0) {
    *w1 = *h1 = -1;
    return(NO);
  }
  
  *x1 = xx;
  *y1 = yy;
  *w1 = ww;
  *h1 = hh;
  
  return(YES);  
}

/*
 * Compute the union of two 'regular' rectangles
 * The result is in *x1,*y1,*w1,*h1
 * It return YES if union is not empty and NO otherwise 
 * (in that case *x1,*y1,*w1,*h1 values are not supposed to be used)
 */
 
char UnionRect1(int x,int y,int w,int h,int *x1,int *y1,int *w1,int *h1)
{
  int xx,yy,ww,hh;
  
  xx = MIN(x,*x1);
  yy = MIN(y,*y1);
  ww = MAX(x+w,*x1+*w1)-xx;
  hh = MAX(y+h,*y1+*h1)-yy;

  if (hh <=0 || ww <= 0) {
    *w1 = *h1 = -1;
    return(NO);
  }
  
  *x1 = xx;
  *y1 = yy;
  *w1 = ww;
  *h1 = hh;
  
  return(YES);  
}


/*
 * Get the intersection of a regular rectangle with the clip rect of a gobject
 */

static char IntersectClipRect(GOBJECT o,int *x, int *y, int *w, int *h)
{
  int x1,y1,w1,h1;

  if (o->flagClip==0 || (o->flagClip == 2 && PSMode)) return(YES);
  
  if (IsWin(o)) {
    x1 = 0;
    y1 = 0;
    w1 = o->w;
    h1 = o->h;
  }
  else {
    x1 = o->x;
    y1 = o->y;
    w1 = o->w;
    h1 = o->h;
  }
  
  return(IntersectRect(x1,y1,w1,h1,x,y,w,h));
}


/*
 * Get the visible rect of an object (intersected with the original rect value *x,*y,*w,*h)
 * The result is in *x1,*y1,*w1,*h1
 * It return YES if intersection is not empty and NO otherwise 
 * (in that case *x,*y,*w,*h values are not supposed to be used)
 */
 
char GetVisibleRect(GOBJECT o,int *x, int *y, int *w, int *h)
{
  while (o != NULL) {
    if (o->flagHide) return(NO);
    if (IntersectClipRect(o,x,y,w,h) == NO)  return(NO);
    o = (GOBJECT) o->father;
  }
  return(YES);
}


/*
 * The following routine transform a general rectangle into a regular one using
 * its type.
 */
void GRect2Rect(int *x, int *y, int *w, int *h, RectType rectType)
{
  short temp;
      
  if (*w < 0) {
    temp = rectType.left;
    rectType.left = rectType.right;
    rectType.right = temp;
  }

  if (*h < 0) {
    temp = rectType.top;
    rectType.top = rectType.bottom;
    rectType.bottom = temp;
  }
  
  if (*w<0) {
    *x += *w+1;
    *w = -*w;
  }
  if (*h<0) {
    *y += *h+1;
    *h = -*h;
  }

  *x -= rectType.left;
  *w += rectType.left+rectType.right;        
  *y -= rectType.top;
  *h += rectType.bottom+rectType.top;     
}

/*
 * The following routine transform a regular rectangle into a general rectangle one using
 * its type. However there are several possibilities (since w and h can be negative).
 * Thus signW and signH are the respective signs of h and w.
 */
void Rect2GRect(int *x, int *y, int *w, int *h,RectType rectType, int signW, int signH) 
{
  *x += rectType.left;
  *y += rectType.top;
  *w -= rectType.left+rectType.right;      
  *h -= rectType.top+rectType.bottom;
  
  if (signH < 0) {
    *y += *h-1;
    *h = -(*h);
  }

  if (signW < 0) {
    *x += *w-1;
    *w = -(*w);
  }
}


/*
 * Conversion of a 'generalized' rectangle of local coordinates (x,y,w,h) to global (*mx,*my,*mw,*mh) coordinates
 */
void Local2GlobalRect(GOBJECT o,float x, float y, float w, float h, RectType rectType,int *mx, int *my, int*mw, int*mh)
{
  /* We convert each extremity of the rectangle */
  Local2Global(o,x,y,mx,my);
  Local2Global(o,x+w,y+h,mw,mh);
  *mw -= *mx;
  *mh -= *my;
  
  /* Then convert the rectangle from generalized to regular */
  GRect2Rect(mx,my,mw,mh,rectType);
}

/*
 * Conversion of a 'regular' rectangle from global (mx,my,mw,mh) to 'generalized' local (*x,*y,*w,*h)  
 */
void Global2LocalRect(GOBJECT o,int mx, int my, int mw, int mh, float *x, float *y, float *w, float *h,RectType rectType)
{
  int signW,signH;
  GOBJECT o1;
  
  /* Getting the signW and signH */
  signW = signH = 1;
  for (o1 = o; o1 != NULL; o1 = (GOBJECT) o1->father) {
    if (IsView(o1)) {
      if (((VIEW) o1)->flagReverse & YFlagReverse) signH -=signH;
      if (((VIEW) o1)->flagReverse & XFlagReverse) signW -=signW;
    }
  }
  
  /* Convert the rectangle from regular to generalized */
  Rect2GRect(&mx,&my,&mw,&mh,rectType,signW,signH);

  /* We convert each extremity of the rectangle */
  Global2Local(o,mx,my,x,y);
  Global2Local(o,mx+mw,my+mh,w,h);
  *w -= *x;
  *h -= *y;
  if (*w < 0) {
    *x += *w;
    *w = -*w;
  }
  if (*h < 0) {
    *y += *h;
    *h = -*h;
  }
}


/* 
 * Update the local rect of an object from its global rect 
 */
void UpdateLocalRectGObject(GOBJECT o)
{
  Global2LocalRect((GOBJECT) (o->father),o->x,o->y,o->w,o->h,&(o->rx),&(o->ry),&(o->rw),&(o->rh),o->rectType);
}

/* Update the global rect of an object from its local rect */
void UpdateGlobalRectGObject(GOBJECT o)
{
  Local2GlobalRect((GOBJECT) (o->father),o->rx,o->ry,o->rw,o->rh,o->rectType,&(o->x),&(o->y),&(o->w),&(o->h));
}


/**********************************************************************************
 *
 * Some other misc functions
 *
 **********************************************************************************/

/* Get the name of an object (returns a temporary string) */
char *GetNameGObject(GOBJECT o)
{
  GOBJECT array[200];
  char *name;
  int s,i,n;
  
  n = 0;
  s = 0;
  while (o != NULL) {
    array[n++] = o;     
    s += strlen(o->name);
    o = (GOBJECT) o->father;
  }
  
  name = CharAlloc(s+n);
  TempStr(name);
  
  name[0] = '\0';
  for(i=n-1;i>=0;i--) {
    strcat(name,array[i]->name);
    if (i != 0) strcat(name,".");
  }
  
  return(name);
}

/*
 * Compute the positions (absolute and relative) of an object using its grid position
 */
static void ComputeGridRect(GOBJECT obj)
{
  GRID grid;
  int x,y,w,h,xGrid,yGrid;
  int dx,dy;
  char *l[4];
  char str1[20],str2[20];
    
  /* Basic checking */  
  if (!IsSubClass(obj->father->gclass,theGridClass)) Errorf("ComputeGridRect() : Weird error 1");

  grid = (GRID) obj->father;

  /* More checking */    
  if (!(obj->flagGrid)) Errorf("ComputeGridRect() : Weird error 2");
  
  /* Get the absolute position of the grid */
  if (IsWin((GOBJECT) grid)) {
    xGrid = 0;
    yGrid = 0;
  }
  else {
    xGrid = grid->x;
    yGrid = grid->y;
  }
    
  /* Compute the number of pixels for a grid unit */
  dx = (grid->w-grid->leftMargin-grid->rightMargin+grid->dx)/((float) grid->gridM)-grid->dx+.5;
  dy = (grid->h-grid->topMargin-grid->bottomMargin+grid->dy)/((float) grid->gridN)-grid->dy+.5;
  if (dx <=0) dx = 1;
  if (dy <=0) dy = 1;
  
  /* Compute the absolute position of the object */
  x = xGrid + grid->leftMargin + (obj->i-1)*(dx+grid->dx);
  y = yGrid + grid->topMargin  + (obj->j-1)*(dy+grid->dy);
  
  /* Compute the size of the object */
  w = obj->m*(dx+grid->dx)-grid->dx;
  h = obj->n*(dy+grid->dy)-grid->dy;
  
  /* If it is the last object of a row or column, we must correct the size so that it fits the grid */
  if (obj->i+obj->m-1 == grid->gridM) w = xGrid+grid->w-grid->rightMargin-x;
  if (obj->j+obj->n-1 == grid->gridN) h = yGrid+grid->h-grid->bottomMargin-y;
  if (w <= 0) w =  obj->m*dx +1;
  if (h <= 0) h =  obj->n*dy +1;
  
  /* Then we set the position MERDE */
  sprintf(str1,"%d",x);      
  sprintf(str2,"%d",y);      
  l[0] = "-apos";
  l[1] = str1;
  l[2] = str2;
  l[3] = NULL;
  SetGObject(obj,l,NO);

  /* Then we set the size MERDE */
  sprintf(str1,"%d",w);      
  sprintf(str2,"%d",h);      
  l[0] = "-asize";
  l[1] = str1;
  l[2] = str2;
  l[3] = NULL;
  SetGObject(obj,l,NO);
}

/* 
 * Send (varcontent) message to a gobject (from a hash table ONLY !) 
 * Actually, this routine should never be called !
 */
static void * SendMessage2AGObject(void *content,int message,void **arg)
{
  Errorf("SendMessage2AGObject() : Weired error");
  return(NULL);
}




/***************************************************
 *
 *
 * (Des)Allocation of gobjects functions
 *
 *
 ***************************************************/
 
/*
 * Function for creating a gobject named 'name' of a given 'class' and adding it to the glist 'father'.
 * 'argv' is a list of optional initialization fields that will be set after creating the gobject.
 * 'flagDraw' indicates whether the newly created object must be drawn or not 
 */

GOBJECT NewGObject(GCLASS class, GLIST father, char *name, char **argv,char flagDraw)
{
  GOBJECT o;
  GCLASS c[MaxClassHierarchyDepth];
  char flagWindow;
  int n;
    
  /* Is the class a window class ? */
  flagWindow = IsSubClass(class,theWindowClass);    
  
  /* Cannot specify a father for a window */
  if (flagWindow && father != NULL) Errorf("NewGObject() : Cannot specify a father for a window");

  /* Must specify a father for non window */
  if (!flagWindow && father == NULL) Errorf("NewGObject() : Must specify a father for a gobject which is not a window");
  
  /* Print a warning if the object already exists */
  if (father != NULL && (o = (GOBJECT) GetElemHashTable(father->theGObjects,name)) != NULL) {
    if (DeleteGObject(o) == NO) {
      Warningf("Cannot add object '%s' in glist '%s' since object with the same name already exists and is protected",name,father->name);
      return(NULL);
    }
    else 
      Warningf("Adding object '%s' (which already exists) in glist '%s' -> delete former version",name,father->name);
  }
  
  /* Alloc the object */
 #ifdef DEBUGALLOC
DebugType = "GObject";
#endif
  o = (GOBJECT) Malloc(class->nbBytes);

  InitValue(o,NULL);
  o->nRef = 0;

  /* Copying the name */
  o->name = CopyStr(name);
  
  /* Setting the class */  
  o->gclass = class;
  
  /* Adding the object to the 'father' */
  o->father = NULL;
  o->front = NULL;
  o->sendMessage = SendMessage2AGObject;

  if (father != NULL) AddGObject2GList(o,father); 
      
  /* Get the class hierarchy in the c array */
  n = 0;
  c[n] = class;
  while (c[n]->fatherClass != NULL) {
    c[n+1] = c[n]->fatherClass;
    n++;
  }

  /* Perform the initializations : from the top class initialization to the initialization of the 'class' */
  for (;n>=0;n--) {
    o->classCur = c[n];
    if (c[n]->init) (*(c[n]->init))(o);
    if (c[n]->msgeSCommand != NULL) DoScriptListv(o,c[n]->msgeSCommand,"init",NULL,-1);
  }
          
  /* Optional initializations of fields */  
  o->classCur = o->gclass;
  SetGObject(o,argv,NO);
  
  /* Draw the gobject if asked */
  o->classCur = o->gclass;
  if (flagDraw) DrawWholeGObject(o,YES);
      
  /* Then return */
  return(o);
}


/*
 * Basic Function to delete the gobject 'o' (not to be called directly)
 *
 * No hiding is performed before
 *
 * It returns YES if deletion was made and NO if not 
 * (NOTA : for now the o->nRef is never greater than 1 thus deletion is always performed when
 * this function is called)
 */

static void RemoveGUpdate(GOBJECT o);

static char DeleteGObject_(GOBJECT o)
{
  struct event event;
  GCLASS class;
  GLIST list;
  WINDOW w,w1;
  int r;
  AHASHELEM e;
  GOBJECT obj;

  /* If there still exists a reference then return otherwise we have to destroy the gobject */
  if (o->nRef > 1) {
    o->nRef--;
    return(NO);
  }
    
  /* Force the classCur of the object to be its class */
  o->classCur = o->gclass;
  
  /* We first send all the delete events */
  class = o->gclass;
  while (class != NULL) {

    o->classCur = class;
    event.object = o;
    event.type = Del;
    SendEvent(&event);

    /* Then loop on the class */
    class = class->fatherClass;
  }
  o->classCur = o->gclass;


  /* If it is a GLIST then we MUST delete all the gobjects in it first (because they might adress the GList) */
  if (IsGList(o)) {
    list = (GLIST) o;
    /* We loop on the gobjects */
    for (r = 0; r<list->theGObjects->nRows;r++) {
      for (e = list->theGObjects->rows[r]; e != NULL;) {    
        obj = (GOBJECT) e;
        e = e->next;
        obj->flagHide = YES;
        DeleteGObject_(obj);
      }
    }
    list->theGObjects->nElems = 0;
  }

  /* We perform deletion methods from the class of the object to the topclass */
  class = o->gclass;
  while (class != NULL) {

    /* Then call the specific script delete method if any */
    if (class->msgeSCommand != NULL) DoScriptListv(o,class->msgeSCommand,"delete",NULL,NO);
      

    /* Then call the specific delete function if any */
    if (class->deleteContent) (*(class->deleteContent))(o);      
    
    /* Then loop on the class */
    class = class->fatherClass;
  }
  o->classCur = o->gclass;
  
  /* We must remove the gobject from the update list if it is in it */
  RemoveGUpdate(o);
  
  /* Remove the object from its father GList */
  if (o->father != NULL) {
    RemoveGObject2GList(o,o->father);
    o->father = NULL;
  }
  
  /* If it is a window then we remove it from the window hash table */
  if (IsWin(o)) {
    GetRemoveElemHashTable(theWindowsHT,o->name);
    w = (WINDOW) o;
    if (theWindows == w) theWindows = (WINDOW) w->front;
    else {
      w1 = theWindows;
      while(w1->front != o) w1 = (WINDOW) w1->front;
      w1->front = w->front;
      w->front = NULL;
    }
  }
  
  /* Then free the non specific content of the object */
  Free(o->name);
  
  /* Then the object */
 #ifdef DEBUGALLOC
DebugType = "GObject";
#endif
  Free(o);   
  
  return(YES);
}



/*
 * Function to delete the gobject 'o' after hiding it
 *
 * Before trying to delete it, it notifies its glist that it will be
 * deleted. If the notify method sends back YES, then deletion is allowed
 * otherwise it is not allowed and the object is not deleted.
 */

char DeleteGObject(GOBJECT o)
{
  GCLASS class;
  LISTV lv;

  /* This line is useless for now since the 'nRef' field is never greater than 1 */
  if (o->nRef > 1) return(DeleteGObject_(o));

  /* If the object is not a window we must notify the glist it belongs to */
  if (!IsWin(o)) {

    /* The notification method is sent to the father and is inherited thus we must loop on the top classes */  
    class = o->father->gclass;
    while (class != NULL) {

      /* Then call the specific script deleteNotify method if any */
      if (class->msgeSCommand != NULL) {
        lv = TNewListv();
        AppendStr2Listv(lv,o->name);        
        DoScriptListv((GOBJECT) o->father,class->msgeSCommand,"deleteNotify",lv,YES);
        
        if (toplevelCur->flagReturn) {
          /* if it returned 0 then we forbid deletion ?????? */
          if (GetResultType() == numType && GetResultFloat() == 0);
        
          /* Otherwise let's do the deletion */
          break;
        }
      }
      
      /* Then call the specific deleteNotify function if any */
      if (class->deleteNotify) {
        if ((*(class->deleteNotify))((GOBJECT) o->father) == 0) return(NO);
        break;
      }
    
      /* Then loop on the class */
      class = class->fatherClass;
    }
  }
  
  HideGObject(o);
  return(DeleteGObject_(o));
}




/***************************************************
 *
 *
 * Drawing functions
 *
 *
 ***************************************************/

/*
 * Basic function to draw the object 'o' in the window 'win' 
 * (that is the window 'o' is displayed in) and only in the
 * rectangle specified by x,y,w,h.
 * Just the object 'o' is drawn. No other objects are redrawn (event those in front of 'o').
 */
static void DrawGObject_(WINDOW win, GOBJECT o, int x,int y,int w,int h)
{
  float lx,ly,lw,lh; 
  struct event event;
  GCLASS class,class1;
  WINDOW clipWin;
  int clipX,clipY,clipW,clipH;
  FONT font;
    
        
  /* If window or object is hidden then return */  
  if (win->flagHide) return;
  if (o->flagHide) return;
  
  /* Get the current class of the object */
  class = o->classCur;
  
  /* If rect empty then return */
  if (w <= 0 || h <= 0) return;
    
  /* Get the current clip rect and set it to the new one */
  WGetClipRect(&clipWin,&clipX,&clipY,&clipW,&clipH);
  WSetClipRect(win, x, y, w, h);

  /* Clear the corresponding rect */
  WClearRect((GOBJECT) win,o->bgColor,x,y,w,h,YES,NormalRect);
  
  /* Set some graphic attributes */
  WSetColor(win,o->fgColor);
  font = WGetFont(win);
  WSetFont(win,o->font);
  WSetLineStyle(win,o->lineStyle);
  WSetPenMode(win,o->penMode);
  WSetPenSize(win,o->penSize);

  /* Remember the last window we drawn into */
  toplevelCur->lastWindow = win;
  
  /* Local coordinates of the rectangle */
  Global2LocalRect(o,x,y,w,h,&lx,&ly,&lw,&lh,NormalRect); 
  
  /* We must find the first drawing method in the class hierarchy */
  class1 = class;
  while (class1 != NULL) {
    /* DrawMessage */
    if (DrawMessage(o,win,class1,x,y,w,h)) break;
    class1 = class1->fatherClass;
  }
  
  /* Send a draw event */
  event.object = o;
  event.object->classCur = class;
  event.type = Draw;
  event.x = lx;
  event.y = ly;
  event.w = lw;
  event.h = lh;
  event.i = x;
  event.j = y;
  event.m = w;
  event.n = h;
  SendEvent(&event);

  /* Should we draw a frame around the object */  
  if (o->flagFrame) {
    WSetColor(win,o->fgColor);
    WSetClipRect(win, x, y, w, h);
    WSetLineStyle(win,o->lineStyle);
    WSetPenSize(win,o->penSize);
    if (!IsWin(o)) WDrawRect((GOBJECT) win,o->x,o->y,o->w,o->h,YES,NormalRect);
    else WDrawRect((GOBJECT) win,0,0,o->w,o->h,YES,NormalRect);
  }

  /* Restore the clip rect */
  WSetClipRect(clipWin,clipX,clipY,clipW,clipH);

  /* Restore the font */
  if (font != o->font) WSetFont(win,font);

  /* Flush the graphics */
  WFlush();
}


/*
 * Function to draw the object 'o' and only in the
 * rectangle specified by x,y,w,h.
 *
 * if 'flagJustTheObject' == NO then all the eventual objects intersecting the rect x,y,w,h
 * will be redrawn. Otherwise just this object is drawn.
 *
 */
 
void DrawGObject(GOBJECT o, int x,int y,int w,int h, char flagJustTheObject)
{
  WINDOW win;
  GCLASS class;
  GOBJECT o1;
  int x1, y1, w1, h1;
  
  /* If object is hidden then return */
  if (o->flagHide) return;

  /* We must compute the part which is visible */
  if (GetVisibleRect(o,&x,&y,&w,&h) == NO) return;

  /* Get the window the object is in */
  win = GetWin(o);
    
  /* Then just call the DrawGObject_ function */
  if (!flagJustTheObject) {
    o1 = o;
    while (!IsWin(o1) && o1->bgColor == invisibleColor) o1 = (GOBJECT) o1->father;
    win = GetWin(o1);
    for (;o1 != NULL; o1 = o1->front) {
      class = o1->classCur;
      o1->classCur = o1->gclass;
      x1 = x; y1 = y; w1 = w; h1 = h;
      if (GetVisibleRect(o1,&x1,&y1,&w1,&h1) == NO) continue;
      DrawGObject_(win,o1,x1,y1,w1,h1);
      o1->classCur = class;
      if (IsWin(o1)) break;
    }
  }
  else DrawGObject_(win,o,x,y,w,h);
}

/* Same as above but draw the whole object */
void DrawWholeGObject(GOBJECT o, char flagJustTheObject)
{
  DrawGObject(o,0,0,INT_MAX/2,INT_MAX/2,flagJustTheObject);
}


/*
 * Function to draw a list of objects 'gobjects'  only in the
 * rectangle specified by x0,y0,w0,h0.
 *
 * All the eventual objects intersecting the rect x,y,w,h
 * will be redrawn.
 *
 * All the gobjects must belong to the same window.
 */
void DrawGObjectList(GOBJECT *gobjects,int x0, int y0, int w0 , int h0)
{
  WINDOW win;
  int x[1];
  int y[1];
  int w[1];
  int h[1];
  int x1,y1,w1,h1,x2,y2,w2,h2;
  int nRect,nGObj,nGObj1;
  int i;
  char flag;

  /* If no object then just return */
  if (*gobjects == NULL) return;

  /* If just one object then call DrawGObject */
  if (gobjects[1] == NULL) {
    DrawGObject(*gobjects,x0,y0,w0,h0,toplevelCur->flagInDrawMessage);
    return;
  }
  
  /* If flagInDrawMessage then we cannot daw (for now) several objects */
  if (toplevelCur->flagInDrawMessage) Errorf("DrawGObjectList() : Sorry, cannot draw several gobjects while executing a drawing message");
 
  /* If window is hidden then return */
  win = GetWin(*gobjects);
  if (win->flagHide) return;

  /* Check that all gobjects belong to the same window */
  for (nGObj = 0;gobjects[nGObj];nGObj++) {
    if (GetWin(gobjects[nGObj]) != win) Errorf("DrawGObjectList() : The gobjects must belong to the same window");
  }


  /*
   * We are going to loop on the gobjects in order to get a list of rectangles using absolute coordinates 
   */
  
  /* Get the first rect */
  nRect = 0;
  nGObj = 0;
  
  for (;gobjects[nGObj] != NULL;nGObj++) {
    x[0] = x0; y[0] = y0; w[0] = w0; h[0] = h0;
    if (GetVisibleRect(gobjects[nGObj],&x[0],&y[0],&w[0],&h[0])) {
      nRect++;
      break;
    }
  }
  
  /* If no rect then return */
  if (nRect == 0) return;
  nGObj++;
  
  /* Loop on all the other rects */
  for (;gobjects[nGObj];nGObj++) {
    x1 = x0; y1 = y0; w1 = w0; h1 = h0;
    if (!GetVisibleRect(gobjects[nGObj],&x1,&y1,&w1,&h1)) continue; 
    UnionRect1(x1,y1,w1,h1,&x[0],&y[0],&w[0],&h[0]);
  }
  
  /* Then Draw !!! */
  for(i=0;i<nRect;i++) {
    DrawGObject_(win,(GOBJECT) win,x[i],y[i],w[i],h[i]);
  }
}

/* Same as above but draw the whole of each object in the object list 'gobjects' */
void DrawWholeGObjectList(GOBJECT *gobjects)
{
  DrawGObjectList(gobjects,0,0,INT_MAX/2,INT_MAX/2);
}



/***************************************************
 *
 *
 * Hide/show functions
 *
 *
 ***************************************************/

/* Function to show a gobject */
void ShowGObject(GOBJECT o)
{
  WINDOW w;

  /* If it was not hidden then return */    
  if (o->flagHide == NO) return;
  
  /* Special treatment for windows (creating a frame) */
  if (IsWin(o)) {
    w = (WINDOW) o;
    if (w->frame != (FRAME) NULL) Errorf("ShowGObject() : Weird error");
    w->frame =  WNewFrame(w->title,w->x,w->y,w->w,w->h); 
    w->flag = 0;
    //    WindowFlagNoUpdate;
  }

  o->flagHide = NO;

  /* Let's draw it ! */
  DrawWholeGObject(o,NO);
}

/* Function to hide a gobject */
void HideGObject(GOBJECT o)
{
  WINDOW win;
  GCLASS class;
  int x,y,w,h;

  /* If it was hidden then return */    
  if (o->flagHide == YES) return;

  /* Special treatments for windows (deleting the frame) */
  if (IsWin(o)) {
    win = (WINDOW) o;  
    if (win->frame == (FRAME) NULL) Errorf("HideGObject() : Weird error");
    WDeleteFrame(win->frame);
    win->frame = (FRAME) NULL;
    win->flagHide = YES;
  }
  
  /* And for non window objects */
  else {
    /* We must compute the part which is visible */
    x = 0;
    y = 0;
    w = INT_MAX/2;
    h = INT_MAX/2;
    if (GetVisibleRect(o,&x,&y,&w,&h) == NO) {
      o->flagHide = YES;
      return;
    }
    
    o->flagHide = YES; 
    
    win = GetWin(o);
    class = win->classCur;
    win->classCur = win->gclass;
    DrawGObject_(win,(GOBJECT) win,x,y,w,h);
    win->classCur = class;
  }
}


/***************************************************
 *
 *
 * Back/Front functions
 *
 *
 ***************************************************/

/* Function to make a gobject move to the front */
void FrontGObject(GOBJECT o, char flagDraw)
{
  GLIST list;
  GOBJECT o1;
  
  if (IsWin(o)) {
    WFrontWindow((WINDOW) o);
    return;
  }
  
  /* Get the glist */
  list = (GLIST) o->father;
  
  /* If object is in the back, just remove it */
  if (list->back == o) {
    if (o->front == NULL) return;
    o1 = list->back = o->front;
  }
  
  /* Otherwise look for it and remove it */
  else {
    for (o1 = list->back;o1->front != o;o1 = o1->front);
    o1->front = o->front;
  }

  /* Then add it to the end */
  while (o1->front != NULL) o1 = o1->front;
  o1->front = o;
  o->front = NULL;
  
  /* then redraw the object */
  if (flagDraw)  DrawWholeGObject(o,NO);
}


/* Function to make a gobject move to the back */
void BackGObject(GOBJECT o,char flagDraw)
{
  GLIST list;
  GOBJECT o1;

  /* this function does not work for windows yet */
  if (IsWin(o)) return;
  
  /* Get the glist */
  list = (GLIST) o->father;

  /* If object is in the back then just return */
  if (list->back == o) return;
  
  /* Look for the object */
  for (o1 = list->back;o1->front != o;o1 = o1->front);
  
  /* Then put it in the back */
  o1->front = o->front;
  o->front = list->back;
  list->back = o;

  /* then redraw the object */
  if (flagDraw) DrawWholeGObject(o,NO);
}



/***************************************************
 *
 *
 * Move and/or resize an object 
 *
 *
 ***************************************************/

void MoveResizeDrawGObject(GOBJECT o,float x, float y, float w, float h)
{
  char *list[4],*list1[4];
  char str1[20],str2[20],str3[20],str4[20];
  char flagDraw;
   
  if (!(o->gclass->flags & GClassMoveResize)) return;

  list[0] = NULL;
  if (o->x != x || o->y != y) {
    list[0] = "-pos";

    sprintf(str1,"%g",x);
    list[1] = str1;

    sprintf(str2,"%g",y);
    list[2] = str2;

    list[3] = NULL;
  }

  list1[0] = NULL;
  if (o->w != w || o->h != h) {
    list1[0] = "-size";
   
     sprintf(str3,"%g",w);
     list1[1] = str3;

     sprintf(str4,"%g",h);
     list1[2] = str4;
   
     list1[3] = NULL;
   }
   
   if (IsWin(o)) {
     if (list[0] != NULL) SetGObject(o,list,NO);
     if (list1[0] != NULL) SetGObject(o,list1,NO);
     return;
   }
   
   if (o->flagHide) flagDraw = NO;
   else flagDraw = YES;
   
   if (flagDraw) HideGObject(o);
   if (list[0] != NULL) SetGObject(o,list,NO);
   if (list1[0] != NULL)  SetGObject(o,list1,NO);
   if (flagDraw) ShowGObject(o);
 }
 
 


/******************************************************
 *
 * Function to test whether a point (global coordinates) is within a gobject.
 *   It returns a negative number if it is not
 *   Otherwise it returns a positive float that indicates the "distance" to
 *   the object. If this distance is 0 it means that the point is in the object.
 *   The object the distance corresponds to is returned in *o1
 *   (*o1 is just o in case of simple gobjects but could be a different object 
 *   in case 'o' is a glist).
 *
 ***************************************************/
 
float IsInGObject(GOBJECT o, GOBJECT *o1, int x, int y)
{
  GCLASS class,class1;
  float rx,ry;
  LISTV lv;
  
  /* Get the class */  
  class = o->classCur;
  
  /* Init of the returned value */
  *o1 = NULL;

  /* If the object is hidden then return */  
  if (o->flagHide) return(-1);
  
  /*
   * Is the point in the object bounding box ? 
   * If not then we just return -1
   */
  if (!IsWin(o) && (o->x > x || o->x+o->w < x || o->y > y || o->y+o->h < y)) return(-1);
  if (IsWin(o) && (0 > x || o->w < x || 0 > y || o->h < y)) return(-1);
  
  /* A priori the object the point is in is the object 'o' itself */
  *o1 = o;
  
  /* We must call the IsIn C function or script if there is any in the class hierarchy */
  class1 = class;
  while (class1 != NULL) {
  
    /* Should we call a script command ? */
    if (class1->isInSCommand) {
      Global2Local(o,x,y,&rx,&ry);
      lv = TNewListv();
      AppendFloat2Listv(lv,rx);
      AppendFloat2Listv(lv,ry);
      DoScriptListv(o,class1->isInSCommand,NULL,lv,YES);
      return(GetResultFloat());
    }
  
    /* Should we call a C function ? (????? pourquoi pas en local??) */
    if (class1->isIn != NULL) return((*(class1->isIn))(o,o1,x,y));
  
    /* Loop on the class */  
    class1 = class1->fatherClass;
  }
  
  /* If no specific command were found, it means we are in the object 'o' */
  return(0);
}


/***************************************************
 *
 *
 * Functions that deal with gupdates
 *
 *
 ***************************************************/

/* 
 * The following variables are used in order to remember which objects should be updated on the screen.
 * Whenever a 'setgd' is called or 'update start' is explicitely called, all the gobjects
 * whose fields are changed will be put in the 'gupdateList' table (of size 'sizeGUpdateList') to be redrawn later
 * (i.e., at the end of the setgd or when 'update do' is explicitely called).
 *
 * Actually whenever the 'setg' command is applied to a certain field <field> in a script and whenever it returns -1, it means
 * that the change of the field <field> did not actually affect the display of the object,
 * Thus the object should not be redrawn (it is not put in the 'gupdateList' table.
 *
 * If it returns 0 it means that the object does not know about the field <field>
 *
 * If it returns 1, it means pdate should be perform
 *
 * The  same convention is used for C Set functions. 
 *
 * Since 'setgd' calls can be imbricated, there is a stack of gupdates.
 * 'nGUpdates' is the number of imbricated gupdates and 'indexGUpdate' are the indexes where
 * each of these gupdates start in the 'gupdateList' table.
 */
 

static GOBJECT gupdateList[MaxSizeGObjectList];
static int sizeGUpdateList = 0;
static int indexGUpdate[MaxSizeGObjectList];
static int nGUpdates = 0;


/* Start a new gupdate */
static void StartGUpdate(void)
{
  if (nGUpdates == MaxSizeGObjectList) Errorf("StartGUpdate() : Too many imbricated gupdates");
  
  indexGUpdate[nGUpdates] = sizeGUpdateList;
  gupdateList[sizeGUpdateList] = NULL;
  nGUpdates++;
}  

/* Remove object 'o' from the 'updateList' if it is in it */
static void RemoveGUpdate(GOBJECT o)
{
  int i;
  
  for (i=0;i<sizeGUpdateList;i++) {
    if (gupdateList[i] == o) {
      gupdateList[i] = NULL;
    }
  }
}

/* Add a new object to the update list */
static void AddGUpdate(GOBJECT o)
{
  GOBJECT o1;
  int i;
  
  /* Some basic tests */
  if (nGUpdates == 0) return;
  if (sizeGUpdateList == MaxSizeGObjectList-1) Errorf("AddGUpdate() : Too many objects to update");
  
  if (indexGUpdate[nGUpdates-1] != sizeGUpdateList) {
  
    o1 = gupdateList[sizeGUpdateList-1];

    /* Check that it does not exist already */
   if (o1 == o) return;
  
    /* Check that it is not included in the former one */
    if (!IsWin(o)) {
      if (o1 != NULL && IsWin(o1)) {
        if (GetWin(o) == (WINDOW) o1) return;
      }
      else if (o1->x < o->x && o1->y < o->y && o1->x+o1->w >= o->x+o->w && o1->y+o1->h >= o->y+o->h) return;
    }
  }

  /* Otherwise just add the gobject */
  gupdateList[sizeGUpdateList++] = o;
  gupdateList[sizeGUpdateList] = NULL;
}


/* Perform the last gupdate (Call 'DrawWholeGObjectList') */
static void DoGUpdate(void)
{
  GOBJECT *list;
  int first,last,i,j;
  
  if (nGUpdates == 0) Errorf("DoGUpdate() : No gupdates to do");
  
  /* We copy the list of gobjects to update */
  first = indexGUpdate[nGUpdates-1];
  last = sizeGUpdateList;
  list = (GOBJECT *) Malloc(sizeof(GOBJECT)*(last-first+1));
  TempPtr(list);
  for (j=0,i=first;i<last;i++) {
    if (gupdateList[i] == NULL) continue;
    list[j] = gupdateList[i];
    j++;
  }
  list[j] = NULL;
  
  /* We update the gupdate counter */
  sizeGUpdateList = first;
  nGUpdates--;
  gupdateList[sizeGUpdateList] = NULL;
  
  /* Then we draw the list of gobjects */
  DrawWholeGObjectList(list);
}  

/* Init the gupdates mechanism */
void InitGUpdates(void)
{
  sizeGUpdateList = 0;
  nGUpdates = 0;
}

/* The gupdate command */
void C_GUpdate(char **argv)
{
  char *msge;
  GOBJECT *list;
  
  if (levelCur == levelFirst) Errorf("This command must be used in a script");
  
  argv = ParseArgv(argv,tWORD,&msge,-1);
  
  if (!strcmp(msge,"start")) {
    NoMoreArgs(argv);
    StartGUpdate();
    return;
  }
  
  if (!strcmp(msge,"do")) {
    NoMoreArgs(argv);
    DoGUpdate();
    return;
  }

  if (!strcmp(msge,"add")) {
    argv = ParseArgv(argv,tGOBJECTLIST,&list,0);
    while(*list) {
      AddGUpdate(*list);
      list++;
    }
    return;
  }
  
  Errorf("Unknown action'%s'",msge);
}




/***************************************************
 *
 *
 * Functions that deal with setting and getting fields of a gobject
 *
 *
 ***************************************************/


/* A function defined below (used for parsing a gobject list) */
static char _ParseGObjectList_(char *arg, GLIST glist, GOBJECT *def, GOBJECT **objlist);


/*
 * Basic function to perform a set/get field message specified by 'argv on a gobject 'o' considered
 * as an object of class 'class'
 *
 * It returns 0 if the field name was not found 1 if it exists and if the corresponding gobject must be updated
 * and -1 if it exists and the corresponding gobject must not be updated.
 */ 
 
static int Set1GObjectClass(GCLASS class, GOBJECT o, char **argv)
{
  char flagSet;
  int n;  
  LISTV lv;
  VALUE val;
  
  /* Is it a 'set' or a 'get' */
  if (argv[1] != NULL) flagSet = YES;
  else flagSet = NO;
  
  /* Should we perform a script ? */    
  if (class->setSCommand && !(*argv != NULL && argv[1] == NULL && (!strcmp(*argv,"-rect") || !strcmp(*argv,"-arect")))) {
    lv = TNewListv();
    AppendStr2Listv(lv,*argv+1);
    argv++;
    while (*argv) {
      ParseVal(*argv,&val);
      AppendValue2Listv(lv,val);
      argv++;
    }
    DoScriptListv(o,class->setSCommand,NULL,lv,YES);
        
    if (!toplevelCur->flagReturn) return(NO);
    if (!flagSet) return(YES);
    if (GetResultType() == numType) {
      if (GetResultFloat() == 1) return(YES);
      if (GetResultFloat() == -1) return(-1);
      Errorf("Set1GObjectClass() : Bad value returned '%g' by the 'set' method of gobject '%s'",GetResultFloat(),GetNameGObject(o));     
    }
    Errorf("Set1GObjectClass() : Bad value type '%s' returned by the 'set' method of gobject '%s'",GetResultType(),GetNameGObject(o));     
  }  

  /* Should we perform a C-function */
  if (class->set) {
    if (argv == NULL || *argv == NULL) n = (*(class->set))(o,NULL,NULL);
    else n = (*(class->set))(o,*argv+1,argv+1);
    if (n == NO) return(NO);
    if (!flagSet) return(YES);
    else return(n);    
  }
    
  return(NO);
}


/*
 * General function to perform a list of set/get field messages specified by 'argv' on the object 'o'
 *
 * if flagGUpdate == YES then it means that a gupdate is on
 * 
 * This function returns NO if the last field name (in argv) was not found and YES otherwise
 */

char SetGObject(GOBJECT o, char **argv,char flagGUpdate)
{
  char **argv1;
  char *str;
  GCLASS class;
  char answer;
  GOBJECT objCur,objList[2],*objlist,*list,o1;
  int n;
  char flagStop;
  LEVEL levelVar;

  /* If no message then just return */   
  if (argv == NULL) return(YES);
        
  /* init */
  answer = NO;
  objList[0] = o;
  objList[1] = NULL;
  objlist = objList;
  
  /* We loop on the list to separate each set/get message */ 
  flagStop = NO;
  while (!flagStop && *argv != NULL) {
 
    /* The first word should be of type '-field' */   
    if (**argv != '-') Errorf("SetGObject() : Bad set/get field '%s' (should start with a '-')",*argv);
  
    /* Let's loop to get the end of the first message */
    argv1 = argv;
    argv++;
    while (*argv != NULL) {
      if (**argv != '-' || (*argv)[1] == '-' || isdigit((*argv)[1]) || ((*argv)[1] == '.' && isdigit((*argv)[2]))) {
        if (**argv == '-' && (*argv)[1] == '-') (*argv)++;
        argv++; 
        continue;
      }
      break;
    }

    /* We got the first message (it is pointed to by 'argv1' and the remaining is pointed by 'argv') */
    str = *argv;
    *argv = NULL;
    
    /* If there is a '-.' message then we must have a glist  and send all the following messages to the corresponding objects */
    if ((*argv1)[1] == '.' && !isdigit((*argv1)[2])) {
      if (!IsGList(o)) Errorf("SetGObject() : '%s' message cannot be used with object '%s' which is a not a GList",*argv1,GetNameGObject(o));
      if (argv1[1] != NULL) Errorf("SetGObject() : '%s' message must be followed by other '-' messages",*argv1);
      if (_ParseGObjectList_(*argv1+1,(GLIST) o, NULL,&objlist) == NO) Errorf1(""); 
      if (*objlist == NULL) Errorf("SetGObject() : '%s' message failed since I cannot find '%s' object",*argv1,*argv1+3);
      *argv = str;
      continue;
    }
       
    /* Send the set/get message we found to each object of the list and to its class hierarchy (from bottom to top) till we get an answer */
    for (list = objlist;*list;list++) {
      objCur = *list;
      class = objCur->classCur;
      InitResult();
      while (class != NULL) {
        n = Set1GObjectClass(class,objCur,argv1);
        if (!strcmp(*argv1,"-add")) {
          flagStop = YES;
          *argv = str;
          if (IsGList(objCur)) {
            o1 = (GOBJECT) GetElemHashTable(((GLIST) objCur)->theGObjects,argv1[1]);
            if (o1!=NULL) {
              SetGObject(o1,argv,flagGUpdate);
            }
          }
        }
        if (n != NO) {
          /* Should we add this object to the update list ? */
          if (n == YES && flagGUpdate && objCur->flagHide == NO && 
              *argv1 != NULL && argv1[1] != NULL && (*argv1)[1] != '?') AddGUpdate(objCur);
          answer = YES;
          break;
        }
        answer = NO;
        class = class->fatherClass;
      }
    }
        
    for (list = objlist;*list;list++) {
      if (*list != o) (*list)->classCur = (*list)->gclass;
    }

    *argv = str;
  }    
  
  return(answer);     
}
 
 
/*
 * Basic subroutine for the setg and setgd commands
 */
static char _C_SetGObject(GOBJECT *objlist, char **argv, char flagDraw,char flagUpdate)
{
  char answer;
  GOBJECT *list;
  
  /* Start the GUpdate counter if it is a 'setgd' call */
  if (flagDraw && flagUpdate) StartGUpdate();

  /* Then we loop on the object list */
  list = objlist;
  while (*list) {
    answer = SetGObject(*list,argv,flagUpdate);
    list++;
  }
  
  /* Then we end the update if necessary */
  if (flagDraw && flagUpdate) DoGUpdate();
  
  return(answer);
}


/*
 * The setg command 
 */
void C_Setg(char **argv)
{
  GOBJECT *objlist;
  char answer,flagUpdate;
 
  /* If the first argument is '-' then no update will be performed */
  flagUpdate = YES;
  if (*argv != NULL && !strcmp(*argv,"-")) {
    flagUpdate = NO;
    argv++;
  }
  
  argv = ParseArgv(argv,tGOBJECTLIST,&objlist,-1);
    
  /* Set/Get the fields */ 
  answer = _C_SetGObject(objlist,argv,NO,flagUpdate);
  
  while (*objlist) {(*objlist)->classCur = (*objlist)->gclass;objlist++;}

}


/*
 * The setgu command 
 */
void C_SetgU(char **argv)
{
  GOBJECT *objlist;

  argv = ParseArgv(argv,tGOBJECTLIST,&objlist,-1);

  /* Set/Get the fields */ 
  _C_SetGObject(objlist,argv,YES,YES);
  
  while (*objlist) {(*objlist)->classCur = (*objlist)->gclass;objlist++;}
}




/***************************************************
 *
 *
 * Functions that deal with sending messages to gobjects
 *
 *
 ***************************************************/

/*
 * The msge command (for sending messages) 
 */
void C_Msge(char **argv)
{
  LISTV lv;
  char *msge,*str,**argv1;
  GOBJECT o,o1;
  GCLASS class;
  int x,y,w,h,cx,cy,cw,ch;
  float fx,fy,rx,ry,rw,rh;
  WINDOW cwin;
  GOBJECT *objlist,*objlist1;
  char flagLocal;
  
  /* Basic checkings */
  if (*argv == NULL || *(argv+1) == NULL) ErrorUsage();
  
  
  /* Special treatment for the 'exist' message */
  if (!strcmp(*(argv+1),"exist")) {
    class = NULL;
    if (*(argv+2) != NULL) {
      if (*(argv+3) != NULL) ErrorUsage();
      str = *(argv+2);
      class = (GCLASS) GetElemHashTable(theGClasses,str);
      if (class == NULL) Errorf("Class '%s' does not exist",str);
    }
    argv = ParseArgv(argv,tGOBJECTLIST_,NULL,&objlist,-1);
    if (objlist == NULL) SetResultInt(0);
    else {
      if (class != NULL) {
        objlist1 = objlist;
        while(*objlist1) {
          if (!IsSubClass((*objlist1)->gclass,class)) break;
          objlist1++;
        }
        SetResultInt((int) (*objlist1 == NULL));
        while(*objlist) {
          (*objlist)->classCur = (*objlist)->gclass;
          objlist++;
        }
      }
      else SetResultInt(1);
    }
    return;
  }

  /* Get the message and the objectlist */
  argv = ParseArgv(argv,tGOBJECTLIST,&objlist,tWORD,&msge,-1);
  argv1 = argv-1;
  
  /* Special treatment for the 'name' message */
  if (!strcmp(msge,"name")) {
    NoMoreArgs(argv);
    objlist1 = objlist;
    while(*objlist1) {
      AppendListResultStr(GetNameGObject(*objlist1));
      objlist1++;
    }
    while(*objlist) {
      (*objlist)->classCur = (*objlist)->gclass;
      objlist++;
    }
    return;
  }

  /* Special treatment for the 'draw' message */
  if (!strcmp(msge,"draw")) { 
   
    /* Case there is no rect specified */
    if (*argv == NULL || *(argv+1) == NULL) {
      if (*argv != NULL) {
        /* Case we do not use the current clip */
        if (strcmp("-clip",*argv)) ErrorUsage();
        DrawWholeGObjectList(objlist);
      }
      else {
       /* Case we should use the current clip */
        WGetClipRect(&cwin,&cx,&cy,&cw,&ch);
        if (cwin != GetWin(objlist[0])) DrawWholeGObjectList(objlist);
        else DrawGObjectList(objlist,cx,cy,cw,ch);
      }
      
      while(*objlist) {
        (*objlist)->classCur = (*objlist)->gclass;
        objlist++;
      }
      return;
    }
    
    /* Case a rect (local/global coordinate) is specified */    
    if (!strcmp(*argv,"-l")) {
      flagLocal = YES;
      if (objlist[1] != NULL) Errorf("Cannot use local coordinates if more than 1 gobject");
    }
    else if (!strcmp(*argv,"-g")) flagLocal = NO;
    else ErrorUsage();
    argv++; 
    if (flagLocal) {
      argv = ParseArgv(argv,tFLOAT,&rx,tFLOAT,&ry,tFLOAT,&rw,tFLOAT,&rh,-1);
      Local2GlobalRect(*objlist,rx,ry,rw,rh,LargeRect,&x,&y,&w,&h);
    }
    else {
      argv = ParseArgv(argv,tINT,&x,tINT,&y,tINT,&w,tINT,&h,-1);
    }    
    
    /* Case we do not use the clip */
    if (*argv != NULL) {
      if (*(argv+1) != NULL) ErrorUsage();
      if (strcmp("-clip",*argv)) ErrorUsage();
      DrawGObjectList(objlist,x,y,w,h);
    }
    else {
      /* case we should use the clip */
      WGetClipRect(&cwin,&cx,&cy,&cw,&ch);
      if (cwin != GetWin(objlist[0])) {
        DrawGObjectList(objlist,x,y,w,h);
      }
      else {
        IntersectRect(cx,cy,cw,ch,&x,&y,&w,&h);
        DrawGObjectList(objlist,x,y,w,h);
      }
    }
    
    /* Reset the current class */
    while(*objlist) {
      (*objlist)->classCur = (*objlist)->gclass;
      objlist++;  
    }
    return;
  }
          
  /* The 'init' message is forbidden */
  if (!strcmp(msge,"init")) return;

  /*
   * Then we loop on all the objects of the list 
   */
  while(*objlist) {
    
    o = *objlist;
    objlist++;
    
    /*  
     * First we perform the messages that cannot be redifined by class messages
     */  
  
    /* Delete message */
    if (!strcmp(msge,"delete")) { 
      SetResultInt((int) DeleteGObject(o));
      continue;
    }

    /* Back message */
    if (!strcmp(msge,"back")) { 
      BackGObject(o,YES);      
      SetResultInt(1);
      continue;
    }

    /* Front message */
    if (!strcmp(msge,"front")) { 
      FrontGObject(o,YES);      
      SetResultInt(1);
      continue;
    }
  
    /* IsIn message */
    if (!strcmp(msge,"isin")) { 
      argv = ParseArgv(argv,tFLOAT,&fx,tFLOAT,&fy,0);
      Local2Global(o,fx,fy,&x,&y);
      SetResultInt(IsInGObject(o,&o1,x,y)>=0);
      o->classCur = o->gclass;
      continue;
    }

    /* move message */
    if (!strcmp(msge,"move")) { 
      argv = ParseArgv(argv,tFLOAT,&rx,tFLOAT,&ry,0);
      MoveResizeDrawGObject(o,rx,ry,o->rw,o->rh);
      SetResultInt(1);
      o->classCur = o->gclass;
      continue;
    }

    /* pmove message */
    if (!strcmp(msge,"pmove")) { 
      argv = ParseArgv(argv,tINT,&w,tINT,&h,0);
      if (!IsWin(o)) {
        Local2Global((GOBJECT) o->father,o->rx,o->ry,&x,&y);
        Global2Local((GOBJECT) o->father,x+w,y+h,&rx,&ry);
      }
      else {
        rx = o->x+w;
        ry = o->y+h;
      }
      MoveResizeDrawGObject(o,rx,ry,o->rw,o->rh);
      SetResultInt(1);
      o->classCur = o->gclass;
      continue;
    }

    /* class message */
    if (!strcmp(msge,"class")) { 
      argv = ParseArgv(argv,tSTR_,NULL,&str,0);
      if (str != NULL) {
        class = (GCLASS) GetElemHashTable(theGClasses,str);
        if (class == NULL) Errorf("Class '%s' does not exist",str);
        SetResultInt((int) IsSubClass(o->gclass,class));
      }
      else SetResultStr(o->gclass->name);
      o->classCur = o->gclass;
      continue;
    }

    /* id message */
    if (!strcmp(msge,"id")) { 
      NoMoreArgs(argv);
      SetResultf("%p",o);
      o->classCur = o->gclass;
      continue;
    }
    
    /* resize message */
    if (!strcmp(msge,"resize")) { 
      argv = ParseArgv(argv,tFLOAT,&rw,tFLOAT,&rh,0);
      MoveResizeDrawGObject(o,o->rx,o->ry,rw,rh);
      SetResultInt(1);
      o->classCur = o->gclass;
      continue;
    }

    /* g2l message */
    if (!strcmp(msge,"g2l")) { 
      argv = ParseArgv(argv,tINT,&x,tINT,&y,tINT_,-1,&w,tINT_,-1,&h,0);
      if (w < 0 || h < 0)  {
        Global2Local(o,x,y,&rx,&ry);
        lv = TNewListv();
        AppendFloat2Listv(lv,rx);
        AppendFloat2Listv(lv,ry);
        SetResultValue(lv);
      }
      else {
        Global2LocalRect(o,x,y,w,h,&rx,&ry,&rw,&rh,NormalRect);
        lv = TNewListv();
        AppendFloat2Listv(lv,rx);
        AppendFloat2Listv(lv,ry);
        AppendFloat2Listv(lv,rw);
        AppendFloat2Listv(lv,rh);
        SetResultValue(lv);
      }
      o->classCur = o->gclass;
      continue;
    }

    /* l2g message */
    if (!strcmp(msge,"l2g")) { 
      argv = ParseArgv(argv,tFLOAT,&rx,tFLOAT,&ry,tFLOAT_,-1.,&rw,tFLOAT_,-1.,&rh,0);
      if (rw < 0 || rh < 0)  {
        Local2Global(o,rx,ry,&x,&y);
        lv = TNewListv();
        AppendInt2Listv(lv,x);
        AppendInt2Listv(lv,y);
        SetResultValue(lv);
      }
      else {
        Local2GlobalRect(o,rx,ry,rw,rh,LargeRect,&x,&y,&w,&h);
        lv = TNewListv();
        AppendInt2Listv(lv,x);
        AppendInt2Listv(lv,y);
        AppendInt2Listv(lv,w);
        AppendInt2Listv(lv,h);
        SetResultValue(lv);
      }
      o->classCur = o->gclass;
      continue;
    }

    /* father message */
    if (!strcmp(msge,"father")) { 
      NoMoreArgs(argv);
      if (o->father == NULL) SetResultValue(nullValue);
      else {
        SetResultStr(GetNameGObject((GOBJECT) (o->father)));
        o->classCur = o->gclass;
      }
      continue;
    }

    /* Hide message */
    if (!strcmp(msge,"hide")) { 
      NoMoreArgs(argv);
      HideGObject(o);
      SetResultInt(1);
      o->classCur = o->gclass;
      continue;
    }

    /* Show message */
    if (!strcmp(msge,"show")) { 
      NoMoreArgs(argv);
      ShowGObject(o);
      SetResultInt(1);
      o->classCur = o->gclass;
      continue;
    }

    /* Class dependent messages */
    class = o->classCur;
    while (class != NULL) {
      /* Should we execute a script ? */
      if (class->msgeSCommand && strcmp(*argv1,"add")) {
        DoScriptArgv(o,class->msgeSCommand,argv1);
        if (toplevelCur->flagReturn) break;
      }
      /* Should we execute a C function ? */
      else if (class->msge) {
        if ((*(class->msge))(o,msge,argv)) break;
      }
      /* Try the fatherClass */
      class = class->fatherClass;
    }    
    o->classCur = o->gclass;
  }
}


/***************************************************
 *
 *
 * Functions that deal with parsing GObjects
 *
 *
 ***************************************************/

/*
 * Search for a list  of  gobjects whose name matches the pattern 'filter'
 * The gobjects must be direct son (if flagDeepSearch == NO) of 'o' or any 
 * son of 'o' (if flagDeepSearch == YES).
 * If list == NULL then the gobjects we are searching for are windows.
 * The gobjects must be of class 'class' (unless class == NULL)
 * The result list is set in the array 'gobjects' and the length of this list is set in '*nGObjects'.
 * The list must have a maximum length of 'maxNGObjects'
 *
 * It returns NO if an error occurred otherwise it returns YES.
 */  


static char SearchGObjectList(GOBJECT o, char *filter, GCLASS class, int maxNGObjects, GOBJECT *gobjects,int *nGObjects, char flagDeepSearch) 
{
  GOBJECT obj;
  GLIST list;
  HASHTABLE hash;
  GOBJECT back;
  char *str;
  char flagWild;
    
  /* If name is ^ then we must go 1 level up */
  if (!strcmp(filter,"^")) {
    if (o == NULL) return(YES);
    if (class && !IsSubClass(o->father->gclass,class)) return(YES);
    gobjects[(*nGObjects)++] = (GOBJECT) o->father;
    return(YES);
  }
  
  if (o && !IsGList(o)) Errorf("SearchGObjectList() : Weird error");
  list = (GLIST) o;
  
  /* If list  is NULL we must use the window hashtable otherwise we use the glist hashtable */
  if (list == NULL) {
    hash = theWindowsHT;
    back = (GOBJECT) theWindows;
  }
  else {
    hash = list->theGObjects;
    back = list->back;
  }
  
  /* Does the filter contain a wild card character ? */
  flagWild = NO;
  str = filter;
  while (*str != '\0') {
    if (*str == '*' || *str == '[' || *str == '?') {
      flagWild = YES;
      break;
    }
    str++;
  }
     
  /* Look at the first level of the glist */
  /* If there is no wild card character we just use the hash table */
  if (!flagWild) {
    obj = (GOBJECT) GetElemHashTable(hash,filter);
    if (obj != NULL) {
      if (*nGObjects>=maxNGObjects) return(YES);
      if (!class || IsSubClass(obj->gclass,class)) {
        gobjects[(*nGObjects)++] = obj;
      }
    }
  }
  else {
    /* Otherwise we have to go through the whole 'back' list */
    for (obj = back; obj != NULL; obj = obj->front) {
      if (class && !IsSubClass(obj->gclass,class)) continue;
      if (MatchStr(obj->name,filter)) {
        if (*nGObjects>=maxNGObjects) return(YES);
        gobjects[(*nGObjects)++] = obj;
      }
    }
  }
  
  if (!flagDeepSearch) return(YES);
  
  /* Recursive search */
  for (obj = back; obj != NULL; obj = obj->front) {
    if (IsGList(obj)) {
      if (!SearchGObjectList(obj,filter,class,maxNGObjects,gobjects,nGObjects,flagDeepSearch)) return(NO);
    }
  }
  
  return(YES);
}


/*
 * Parse the string 'str0' which must be a valid gobject filter name (using wild cards) and returns the list of the matching gobjects
 *
 * The result list is set in the array 'gobjects' and the length of this list is set in '*nGObjects'.
 * The list must have a maximum length of 'maxNGObjects'
 * If 'glist' is not NULL then 'str0' starts with a dot and the objects should be sons (direct or not) of 'glist'.
 *
 * It returns NO if an error occurred otherwise it returns YES.
 */  

static char ParseGObjectList__(char *str0, GLIST glist, int maxNGObjects, GOBJECT *gobjects,int *nGObjects)
{
  WINDOW win;
  GOBJECT *list1,*list2,*list;
  int nList1,nList2,n,i;
  char c;
  char *str1, *str2;
  char flagDeepSearch;
  GCLASS class,class1;
  static GOBJECT gobjList1[MaxSizeGObjectList],gobjList2[MaxSizeGObjectList];
  
  /* init the result */
  *nGObjects = 0;

  /* Some basic tests */  
  if (str0 == NULL) return(YES);
  if (*str0 == '\0') return(YES);
  
  /* If glist is not NULL then the begining is easy */
  if (glist != NULL) {
    if (*str0 != '.') {
      SetErrorf("ParseGObjectList__() : Bad name '%s' for gobject of glist '%s'",str0,GetNameGObject((GOBJECT) glist));
      return(NO);
    }
    str1 = str0;
    class = NULL;
    *gobjList1 = (GOBJECT) glist;
    nList1 = 1;
  }
  
  /* Otherwise it's a whole different story ! */
  else {
    
    /*
     * Getting an eventual class conversion 'class:a.b.c' 
     */
    class = NULL;
    str1 = str0;
    while (*str1 != '\0' && *str1 != ':') str1++;
    if (*str1 == ':') {
      *str1 = '\0';
      class = (GCLASS) GetElemHashTable(theGClasses,str0);
      if (class == NULL) {
        SetErrorf("ParseGObjectList__() : Bad class name in gobject name '%s'",str0);
        *str1 = ':';
        return(NO);
      }
      *str1 = ':';
      str0 = str1+1;
    }
             
    /* If it starts with a '.', we must get the current window */
    if (*str0 == '.') {
    
      if (ParseWindow_(".",NULL,&win) == NO) return(NO);
   
      /* If there is nothing else then we must check the class if any and return */
      if (str0[1] == '\0') {
        if (class && !IsSubClass(win->gclass,class)) {
          SetErrorf("ParseGObjectList__() : Bad class '%s' for current window '%s'",class->name,win->name);
          return (NO);
        } 
        *gobjects = (GOBJECT) win;
        (*nGObjects)++;
        if (class) gobjects[0]->classCur = class;
        else gobjects[0]->classCur = gobjects[0]->gclass;
        return(YES);
      }
      str1 = str0;
    
      gobjList1[0] = (GOBJECT) win;
      nList1 = 1;  
    }
  
    /* Otherwise we look for the first name */
    else {
      str1 = str0;
      while(*str1 != '\0' && *str1 != '.') str1++;
      c = *str1;
      *str1 = '\0';
    
      /* Search for @Variables first */
      if (*str0 == '@') { 
        if (toplevelCur->lastEvent == NULL || toplevelCur->lastEvent->type == NoEvent || toplevelCur->lastEvent->object == NULL) {
          SetErrorf("ParseGObjectList__() : No current gobject event for @ variable",str0);
          *str1 = c;
          return(NO);    
        }
        if (!strncmp(str0,"@object",7)) {
          str2 = str0+7;
          *gobjList1 = toplevelCur->lastEvent->object;
        }
        else if (!strncmp(str0,"@father",7)) {
          *gobjList1 = (GOBJECT) toplevelCur->lastEvent->object->father;
          if (*gobjList1 == NULL) {
            SetErrorf("ParseGObjectList__() : Current event gobject does not have any father",str0);
            *str1 = c;
            return(NO);
          }
          str2 = str0+7;
        }
        else if (!strncmp(str0,"window",7)) {
          *gobjList1 = (GOBJECT) GetWin(toplevelCur->lastEvent->object);
          str2 = str0+6;
        }
        else {
          SetErrorf("ParseGObjectList__() : Bad object name '%s'",str0);
          return(NO);
        }
    
        /* Are we done ? */ 
        *str1 = c;    
        if (c != '.' && c != '\0') {
          SetErrorf("ParseGObjectList__() : Bad event variable '%s'",str0);
          return(NO);
        }
        if (c == '\0') {
          if (class && !IsSubClass((*gobjList1)->gclass,class)) {
            SetErrorf("ParseGObjectList__() : Bad class '%s' for gobject '%s'",class->name,(*gobjList1)->name);
            return (NO);
          }
          *gobjects = *gobjList1;
          *nGObjects = 1;
          if (class) gobjects[0]->classCur = class;
          else gobjects[0]->classCur = gobjects[0]->gclass;
          return(YES);
        }
      
        /* Get ready for the following */
        nList1 = 1;
        str1 = str2;      
      }
  
      /* Regular variables */
      else {
        if (c == '\0') {
          SearchGObjectList(NULL,str0,class,maxNGObjects,gobjects,nGObjects,NO);
          *str1 = c;
          if (class) {
            for (i=0;i< (*nGObjects);i++) {
              gobjects[i]->classCur = class;
            }
          }
          return(YES);
        }
        else if (str1[1] != '^') {
          n = 0;
          SearchGObjectList(NULL,str0,theGListClass,MaxSizeGObjectList,gobjList1,&n,NO);
          nList1 = n;
        }
        else {
          n = 0;
          SearchGObjectList(NULL,str0,NULL,MaxSizeGObjectList,gobjList1,&n,NO);
          nList1 = n;
        }
        *str1 = c;
        if (n == 0) return(YES);
      }
    }
  }
  
  /* Some inits before the loop */
  list1 = gobjList1;
  list2 = gobjList2;
  nList2 = 0;

  if (*str1 != '.') Errorf("MERDE");  
  if (nList1 != 1) Errorf("MERDE1");  

  /* Now we start The Loop ! (str1 is pointing to a '.') */
  while(1) {
  
  if (*str1 != '.') Errorf("MERDE2");  

    /* If there is two dots then we should 'deep search' the next object */
    str1++;
    if (str1[0] == '.') {
      flagDeepSearch = YES;
      str1++;
    } else flagDeepSearch = NO;
    
    /* Look for the next name */
    str2 = str1;
    while(*str2 != '\0' && *str2 != '.') str2++;
    c = *str2;
    *str2 = '\0';
    
    /* Let's loop on the object of list1 and get list2 */
    if (c == '\0') class1 = class;
    else if (str2[1] != '^') class1 = theGListClass;
    else class1 = NULL;
    nList2 = 0;
    for (i=0;i<nList1;i++) {
      SearchGObjectList(list1[i],str1,class1,MaxSizeGObjectList,list2,&nList2,flagDeepSearch);
    }
    
    /* if we did not get any answer that's the end */
    if (nList2 == 0) return(YES);
    
    /* Is it the end ? */
    *str2 = c;
    if (c == '\0') {
      *nGObjects = MIN(nList2,maxNGObjects);
      for (i=0;i<*nGObjects;i++) {
        gobjects[i] = list2[i];
        if (class) gobjects[i]->classCur = class;
        else gobjects[i]->classCur = gobjects[i]->gclass;
      }
      return(YES);
    }

    /* Otherwise we loop again */
    str1 = str2;
    list = list1;
    list1 = list2;
    list2 = list;
    nList1 = nList2;
  }      
}

/*
 * Same as the regular parsing function ParseGObjectList_ (below) except that 
 * if 'glist' != NULL then arg starts with a '.' and 'glist' is used as the starting glist.
 */
static char _ParseGObjectList_(char *arg, GLIST glist, GOBJECT *def, GOBJECT **objlist)
{
  GOBJECT gobjects[MaxSizeGObjectList];
  GOBJECT *gobjects1;
  int n,i,maxSize,n1;
  char **list;
  
  /* Some basic tests */  
  if (arg == NULL) {
    SetErrorf("ParseGObjectList_() : NULL string cannot be converted to a gobject");
    return(NO);
  }
  if (*arg == '\0') {
    SetErrorf("ParseGObjectList_() : empty string cannot be converted to an gobject");
    return(NO);
  }
  
  /* Get the list of strings */
  if (!ParseWordList_(arg,NULL,&list) || *list == NULL) {
    *objlist = def;
    return(NO);
  }
  
  /* Loop on the parsing basic subroutine */
  maxSize = MaxSizeGObjectList;
  n = 0;
  gobjects1 = gobjects;
  while (*list) {
    if (!ParseGObjectList__(*list,glist,maxSize,gobjects1,&n1)) {
      *objlist = def;
      return(NO);
    }
    n+=n1;
    gobjects1+=n1;
    maxSize-=n1;
    list++;
  }
  
  if (n == 0) {
    SetErrorf("_ParseGObjectList_() : No matching gobject names to '%s'",arg);
    *objlist = def;
    return(NO);
  }

  *objlist = (GOBJECT *) Malloc((n+1)*sizeof(GOBJECT *));
  TempPtr(*objlist);
  for (i=0;i<n;i++) (*objlist)[i] = gobjects[i];
  (*objlist)[i] = NULL;
  
  return(YES);
}

char ParseGObjectList_(char *arg, GOBJECT *def, GOBJECT **objlist)
{
  if (_ParseGObjectList_(arg,NULL,def,objlist)) return(YES);
  *objlist = def;
  return(NO);
}

  
void ParseGObjectList(char *arg, GOBJECT **objlist)
{
  if (ParseGObjectList_(arg,NULL,objlist) == NO) Errorf1("");
}

char ParseGObject_(char *arg, GOBJECT def, GOBJECT *obj) 
{
  int n;
  GOBJECT o[2];
    
  /* Some basic tests */  
  if (arg == NULL) {
    *obj = def;
    SetErrorf("ParseGObject_() : NULL string cannot be converted to a gobject");
    return(NO);
  }
  if (*arg == '\0') {
    *obj = def;
    SetErrorf("ParseGObject_() : empty string cannot be converted to an gobject");
    return(NO);
  }
  

  /* Call the parsing basic subroutine */
  n = 0;
  if (!ParseGObjectList__(arg,NULL,2,o,&n)) {
    *obj = def;
    return(NO);
  }
  if (n == 0) {
    SetErrorf("ParseGObject_() : No gobject matching the name '%s'",arg);
    *obj = def;
    return(NO);
  }
  
  if (n == 2) Warningf("ParseGObject_() : There are more than one gobject corresponding to '%s' (I chose the first one I found)",arg);

  *obj = o[0];
  
  return(YES);
}


void ParseGObject(char *arg, GOBJECT *obj)
{
  if (ParseGObject_(arg,NULL,obj) == NO) Errorf1("");
}



/***************************************************
 *
 *
 * C functions that define the GObject class
 *
 *
 ***************************************************/

GCLASS theGObjectClass = NULL;

 static void _InitGObject(GOBJECT o) 
{
  o->font = defaultFont;
  o->flagHide = NO;
  o->bgColor = bgColor;
  o->fgColor = fgColor;
  o->penSize = 1;
  o->lineStyle = LinePlain;
  o->penMode = PenPlain;
  o->flagGrid = NO;
  o->flagFrame = NO;
  o->flagClip = 1;
  o->x = o->y = 0;
  o->w = o->h = 10;
  o->rx = o->ry = 0;
  o->rw = o->rh = 10;  
  o->rectType = NormalRect;  
}

static char _MsgeGObject(GOBJECT o, char *msge, char ** argv)
{
  /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{{exist} {Returns 1 if the gobject exists and 0 otherwise.}} \
{{name} {Returns the full name of the gobject.}} \
{{draw [(-l | -g) <x> <y> <w> <h>] [-clip]} {Draws the gobject. If '-l' (resp. '-g') is set , the gobject is drawn \
only in the local (resp. global) rectangle <x> <y> <w> <h>. If '-clip' is set the clip rectangle of the gobject is used \
otherwise the current clip rectangle is used.}} \
{{delete} {Deletes the gobject.}} \
{{back} {Sends the gobject to the back.}} \
{{front} {Sends the gobject to the front.}} \
{{isin <x> <y>} {Returns 1 if the local point <x> <y> belongs to the gobject and returns 0 otherwise.}} \
{{move <x> <y>} {Moves the gobject to local position <x> <y>.}} \
{{pmove <dx> <dy>} {Moves the gobject of <dx> <dy> pixels.}} \
{{class <class>]} {If <class> is not specified then it returns the class of the gobject. Otherwise it returns 1 if \
the gobject is of class <class> (possibly inherited) and 0 otherwise.}} \
{{id} {Returns a unique identificator string (alphanumeric string that can be used as an index of an array) that corresponds to the gobject.}} \
{{resize <w> <h>} {Resizes the gobject to the new (local) size <w> <h>.}} \
{{l2g <x> <y>} {Converts local (gobject) position <x> <y> to global position.}} \
{{g2l <x> <y>} {Converts global position <x> <y> to local (gobject) position.}} \
{{father} {Gets the name of the father of the gobject or the empty string '' if none.}} \
{{hide} {Hides the gobject.}} \
{{show} {Shows the gobject.}}}");
  return(YES);
  }
      
  return(NO);
}
  
static int _SetGObject(GOBJECT o, char *field, char **argv)
{
  char *str;
  int x,y,w,h,n,l,t,r,b;
   float rx,ry,rw,rh;
  unsigned long col;
  GLIST list;
  GOBJECT o1;
  LISTV lv;
  FONT font;
  
  
  /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{asize [<w> <h>]} {Sets/Gets the size of the gobject using absolute coordinates.}} \
{{font [<fontName>]} {Sets/Gets the name of the font used for drawing the gobject.}} \
{{apos [<x> <y>]} {Sets/Gets the position of the gobject using absolute coordinates.}} \
{{size [<w> <h>]} {Sets/Gets the size of the gobject using local coordinates.}} \
{{pos [<x> <y>]} {Sets/Gets the position of the gobject using local coordinates.}} \
{{name} {Gets the full name of the gobject.}} \
{{class} {Gets the gclass name of the gobject.}} \
{{rect} {Gets the bounding rectangle (x,y,w,h) of the gobject using local coordinates (this is equivalent to calling '-pos' and '-size').}} \
{{arect} {Gets the bounding rectangle (x,y,w,h) of the gobject using absolute coordinates (this is equivalent to calling '-apos' and '-asize').}} \
{{grid [<xGrid> <yGrid> <wGrid> <hGrid>]} {If used with no argument, it gets the grid coordinates of the gobject (if they are activated) or \
it returns the empty string ''. If used with the four arguments <xGrid> <yGrid> <wGrid> <hGrid>, \
it sets the gobject on the corresponding grid coordinates and activate automatically the -grid? flag.}} \
{{grid? [<flagOnOff>]} {Activates (<flagOnOff>=1) or deactivates (<flagOnOff>=0) the grid coordinates of the gobject.}} \
{{bg [<color>]} {Sets/Gets the background color of the gobject.}} \
{{fg [<color>]} {Sets/Gets the foreground color of the gobject.}} \
{{pen [<penSize>]} {Sets/Gets the pen size used for drawing the gobject.}} \
{{frame [<flagOnOff>]} {Sets/Gets the frame flag which allows (if <flagOnOff> =1) to draw a frame around the gobject.}} \
{{line [{dash plain}]} {Sets/Gets the type of line that will be used for drawing the gobject (for now only one type of dash is available).}} \
{{hide [<flagOnOff>]} {Sets/Gets the flag which specifies whether the gobject is visible or not.}} \
{{clip [no | yes | screen]} {Sets/Gets the field which specifies whether the gobject should be clipped (using its bounding rectangle) or not. \
If it is 'no' (or 0) it means that the gobject is never clipped. if it is 'yes' (or 1) then it is always clipped. If it is 'screen' \
it means that it is clipped only when displayed on the screen (not in postscript files). This last mode is used \
for Text gobjects (since the postscript fonts and screen fonts do not really correspond, the screen clipping rectangle \
is not appropriate for being used in postscript).}} \
{{depth [<depth>]} {Sets/Gets the 'depth' of the gobject. The possible values of this field are either 'back' \
(gobject is in the back) or 'front' (gobject is in the front) or any number between 1 (which is the same position as \
'back') up to the number of gobjects in the father glist.}} \
{{rectType [<rectType>]} {This field specifies how the absolute coordinate boundary rectangle of the gobject (i.e., '-arect') must be extended in each \
direction after doing local to global change of coordinates on the local rectangle (i.e., '-rect'). This is particularly important if you want \
to control if the boundaries of the local rectangle '-rect' belong or not to the gobject. \
At initialization, the <rectType> of a gobject is 'normal' in the sense that boundary rectangle includes the point (x,y) but not the point (x+w,y+h) \
(thus w and h do represent the width and height in pixels of the gobject). A <rectType> corresponds \
to four numbers extending the rectangle respectively to the left, the top, \
the right and the bottom. There are 3 predefined \
rectangle types (which are associated to names instead of 4 numbers) : \
LargeRect (=0,0,1,1) (both the points (x,y) and (x+w,y+h) belong to the rectangle), \
SmallRect (= -1,-1,0,0) (none of the points (x,y) and (x+w,y+h) belong to the rectangle) \
NormalRect (=0,0,0,0) (the point (x,y) belongs to the rectangle whereas the points (x+w,y+h) \
does not belong to the rectangle). Let us note that, in a View, if the y-axis (resp. the x-axis) is reversed \
then the top-bottom (resp. left-right) are reversed.}}");
    return(YES);
  }


  
  /* Set/Get the absolute size of the object */
  if (!strcmp(field,"asize")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,o->w);
      AppendInt2Listv(lv,o->h);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&w,tINT,&h,0);
    if (w <= 0 || h <=0) Errorf("_SetGObject() : GObject '-asize' of '%s' could not be negative",GetNameGObject(o));
/*    if (o->w == w && o->h == h) return(-1); */
    o->w = w;
    o->h = h;
    UpdateLocalRectGObject(o);
    return(YES);
  }

  /* Set/Get the relative size of the object */
  if (!strcmp(field,"size")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendFloat2Listv(lv,o->rw);
      AppendFloat2Listv(lv,o->rh);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tFLOAT,&rw,tFLOAT,&rh,0);
    if (rw < 0 || rh <0) Errorf("_SetGObject() : GObject '-size' of '%s' could not be negative",GetNameGObject(o));
/*    if (o->rw == rw && o->rh == rh) return(-1); */
    o->rw = rw;
    o->rh = rh;
    UpdateGlobalRectGObject(o);
    return(YES);
  }

  /* Set/Get the absolute position of the object */  
  if ((IsWin(o) && !strcmp(field,"pos")) || !strcmp(field,"apos")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,o->x);
      AppendInt2Listv(lv,o->y);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,tINT,&y,0);
/*    if (o->x == x && o->y == y) return(-1); */
    o->x = x;
    o->y = y;
    UpdateLocalRectGObject(o);
    return(YES);
  }

  /* Set/Get the relative position of the object */
  if (!strcmp(field,"pos")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendFloat2Listv(lv,o->rx);
      AppendFloat2Listv(lv,o->ry);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tFLOAT,&rx,tFLOAT,&ry,0);
/*    if (o->rx == rx && o->ry == ry) return(-1); */
    o->rx = rx;
    o->ry = ry;
    UpdateGlobalRectGObject(o);
    return(YES);
  }  
  
  /* Get the relative bounding box */
  if (!strcmp(field,"rect")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendFloat2Listv(lv,o->rx);
      AppendFloat2Listv(lv,o->ry);
      AppendFloat2Listv(lv,o->rw);
      AppendFloat2Listv(lv,o->rh);
      SetResultValue(lv);
      return(YES);
    }
    Errorf("_SetGObject() : The '-rect' field is a read only field");
  }

  /* Get the absolute bounding box */
  if (!strcmp(field,"arect")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,o->x);
      AppendInt2Listv(lv,o->y);
      AppendInt2Listv(lv,o->w);
      AppendInt2Listv(lv,o->h);
      SetResultValue(lv);
      return(YES);
    }
    Errorf("_SetGObject() : The '-arect' field is a read only field");
  }

  /* Get the name  */
  if (!strcmp(field,"name")) {
    if (*argv == NULL) {
      SetResultStr(GetNameGObject(o));
      return(YES);
    }    
    Errorf("_SetGObject() : The '-name' field is a read only field");

  }

  /* Get the class name  */
  if (!strcmp(field,"class")) {
    if (*argv == NULL) {
      SetResultStr(o->gclass->name);
      return(YES);
    }
    Errorf("_SetGObject() : The '-class' field is a read only field");
  }

  /* Set/Get the grid? field */
  if (!strcmp(field,"grid?")) {
    if (*argv == NULL) {
      SetResultInt(o->flagGrid);
      return(YES);
    }      
    if (IsWin(o)) Errorf("_SetGObject() : Cannot set the '-grid?' field of the window '%s'",GetNameGObject(o)); 
    if (!IsSubClass(o->father->gclass,theGridClass)) 
      Errorf("Cannot set the '-grid?' field for a gobject ('%s') which is not in a grid",GetNameGObject(o)); 
      
    /* Unset the grid flag */  
    if (argv[1] == NULL && !strcmp("0",*argv)) {
      if (o->flagGrid == NO) return(-1);
      o->flagGrid = NO;
      return(YES);
    }  
    /* Set the grid flag */  
    if (argv[1] == NULL && !strcmp("1",*argv)) {
      if (o->flagGrid == YES) return(-1);
      o->flagGrid = YES;
      return(YES);
    }  
    Errorf("_SetGObject() : Bad value '%s' for '-grid?' field",*argv);
  }
  
  /* Set/Get the grid position */
  if (!strcmp(field,"grid")) { 
    if (*argv == NULL) {
      lv = TNewListv();
      if (o->flagGrid) {
        AppendInt2Listv(lv,o->i);
        AppendInt2Listv(lv,o->j);
        AppendInt2Listv(lv,o->m);
        AppendInt2Listv(lv,o->n);
      }
      SetResultValue(lv);
      return(YES);
    }
    if (IsWin(o)) Errorf("Cannot set the -grid field of a window !"); 
    if (!IsSubClass(o->father->gclass,theGridClass)) 
      Errorf("Cannot set the -grid field for objects that are not in grids"); 
    /* Set the grid position */
    argv = ParseArgv(argv,tINT,&x,tINT,&y,tINT,&w,tINT,&h,0);
    if (o->i == x && o->j == y && o->m == w && o->n == h && o->flagGrid) return(-1);    
    o->i = x;
    o->j = y;
    o->m = w;
    o->n = h;
    o->flagGrid = YES;
    ComputeGridRect(o);
    return(YES);
  }

  /* Set/Get the foreground color */
  if (!strcmp(field,"fg")) {
    if (*argv == NULL) {
      SetResultStr(GetColorName(o->fgColor));
      return(YES);
    }
    argv = ParseArgv(argv,tCOLOR,&col,0);
    if (o->fgColor == col) return(-1);
    o->fgColor = col;
    return(YES);
  }

  /* Set/Get the background color */
  if (!strcmp(field,"bg")) {
    if (*argv == NULL) {
      SetResultStr(GetColorName(o->bgColor));
      return(YES);
    }
    argv = ParseArgv(argv,tCOLOR,&col,0);
    if (o->bgColor == col) return(-1);
    o->bgColor = col;
    return(YES);
  }

  /* Set/Get the font */
  if (!strcmp(field,"font")) {
    if (*argv == NULL) {
      SetResultStr(o->font->name);
      return(YES);
    }
    argv = ParseArgv(argv,tFONT,&font,0);
    if (o->font == font) return(-1);
    o->font = font;
    return(YES);
  }

  /* Set/Get the pen size */
  if (!strcmp(field,"pen")) {
    if (*argv == NULL) {
      SetResultInt(o->penSize);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,0);
    if (x <= 0) Errorf("Set1GObject() : Bad pen size '%d'",x);
    if (o->penSize == x) return(-1);
    o->penSize = x;
    return(YES);
  }

  /* Set/Get the frame flag */
  if (!strcmp(field,"frame")) {
    if (*argv == NULL) {
      SetResultInt(o->flagFrame);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,0);
    if (o->flagFrame == (x!=0)) return(-1);
    o->flagFrame = x!=0;
    return(YES);
  }

  /* Set/Get the clip flag */
  if (!strcmp(field,"clip")) {
    if (*argv == NULL) {
      if (o->flagClip == 0) SetResultStr("no");
      else if (o->flagClip == 1) SetResultStr("yes");
      else SetResultStr("screen");
      return(YES);
    }
    argv = ParseArgv(argv,tSTR,&str,0);
    if (!strcmp(str,"0") || !strcmp(str,"no")) o->flagClip = 0;
    else if (!strcmp(str,"1") || !strcmp(str,"yes")) o->flagClip = 1;
    else if (!strcmp(str,"screen")) o->flagClip = 2;
    else Errorf("Set1GObject() : Bad value '%s' for 'clip' field",str);
    return(YES);
  }

  /* Set/Get the rectType  */
  if (!strcmp(field,"rectType")) {
    if (*argv == NULL) {
      if (o->rectType.left == LargeRect.left && o->rectType.right == LargeRect.right &&
          o->rectType.top == LargeRect.top && o->rectType.bottom == LargeRect.bottom) SetResultStr("large");
      else if (o->rectType.left == SmallRect.left && o->rectType.right == SmallRect.right &&
          o->rectType.top == SmallRect.top && o->rectType.bottom == SmallRect.bottom) SetResultStr("small");
      else if (o->rectType.left == NormalRect.left && o->rectType.right == NormalRect.right &&
          o->rectType.top == NormalRect.top && o->rectType.bottom == NormalRect.bottom) SetResultStr("normal");
      else {
        lv = TNewListv();
        AppendInt2Listv(lv,o->rectType.left);
        AppendInt2Listv(lv,o->rectType.top);
        AppendInt2Listv(lv,o->rectType.right);
        AppendInt2Listv(lv,o->rectType.bottom);
        SetResultValue(lv);
      }
      return(YES);
    }
    if (argv[1] == NULL) {
      argv = ParseArgv(argv,tSTR,&str,0);
      if (!strcmp(str,"normal")) o->rectType = NormalRect;
      else if (!strcmp(str,"large")) o->rectType = LargeRect;
      else if (!strcmp(str,"small")) o->rectType = SmallRect;
      else Errorf("Set1GObject() : Bad value '%s' for 'rectType' field",str);
    } else {
      argv = ParseArgv(argv,tINT,&l,tINT,&t,tINT,&r,tINT,&b,0);
      if (o->rectType.left == l && o->rectType.right == r && o->rectType.top == t && o->rectType.bottom == b) return(-1);
      o->rectType.left = l;
      o->rectType.right = r;
      o->rectType.top = t;
      o->rectType.bottom = b;
    }
    UpdateGlobalRectGObject(o);
    return(YES);
  }

  /* Set/Get the line style */
  if (!strcmp(field,"line")) {
    if (*argv == NULL) {
      if (o->lineStyle == LinePlain) SetResultStr("plain");
      else SetResultStr("dash");
      return(YES);
    }
    argv = ParseArgv(argv,tSTR,&str,0);
    if (!strcmp(str,"plain")) {
      if (o->lineStyle == LinePlain) return(-1);
      o->lineStyle = LinePlain;
    }
    else if (!strcmp(str,"dash")) {
      o->lineStyle = LineDash;
      if (o->lineStyle == LineDash) return(-1);
    }
    else Errorf("Set1GObject() : Bad line type '%s'",str);
    return(YES);
  }

  /* Set/Get the pen mode */
  if (!strcmp(field,"mode")) {
    if (*argv == NULL) {
      if (o->penMode == PenPlain) SetResultStr("normal");
      else SetResultStr("inverse");
      return(YES);
    }
    argv = ParseArgv(argv,tSTR,&str,0);
    if (!strcmp(str,"normal")) {
      if (o->penMode == PenPlain) return(-1);
      o->penMode = PenPlain;
    }
    else if (!strcmp(str,"inverse")) {
      if (o->penMode == PenInverse) return(-1);
      o->penMode = PenInverse;
    }
    else Errorf("Set1GObject() : Bad pen mode '%s'",str);
    return(YES);
  }

  /* Set/Get the hide flag */
  if (!strcmp(field,"hide")) {
    if (*argv == NULL) {
      SetResultInt((int) o->flagHide);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,0);
    if (o->flagHide == (x == 1)) return(-1);
    o->flagHide = (x == 1);
    return(YES);
  }
  
  /* Set/Get the depth flag */
  if (!strcmp(field,"depth")) {
    if (IsWin(o)) return(NO);
    if (*argv == NULL) {
      list = o->father;
      if (o == list->back) SetResultStr("back"); 
      else if (o->front == NULL) SetResultStr("front");
      else {
        n = 0;
        o1 = list->back;
        while (o1 != NULL) {
          o1 = o1->front;
          n++;
        }
        SetResultInt(n);
      }
      return(YES);
    }
    argv = ParseArgv(argv,tSTR,&str,0);
    if (!strcmp(str,"back")) BackGObject(o,NO);
    else if (!strcmp(str,"front")) FrontGObject(o,NO);
    else {
      ParseInt(str,&x);
      Warningf("_SetGObject() : Sorry '-depth <number>' not implemented yet");
    }
    return(YES);
  }
   
  return(NO);
}



/***************************************************
 *
 *
 * C functions that define the GList class
 *
 *
 ***************************************************/

GCLASS theGListClass = NULL;

/*
 * Function to recompute the absolute positions/sizes of all the gobjects in a glist 
 * given their relative positions.
 */
static void RecomputeAbsolutePositions(GLIST l)
{
  GOBJECT obj;
  GCLASS class;
  char *list[4];
  char str1[20],str2[20];
  
  for (obj = l->back;obj != NULL;obj=obj->front) {
    class = obj->classCur;
    obj->classCur = obj->gclass;
    sprintf(str1,"%g",obj->rx);      
    sprintf(str2,"%g",obj->ry);      
    list[0] = "-pos";
    list[1] = str1;
    list[2] = str2;
    list[3] = NULL;
    SetGObject(obj,list,NO);
    sprintf(str1,"%g",obj->rw);      
    sprintf(str2,"%g",obj->rh);      
    list[0] = "-size";
    list[1] = str1;
    list[2] = str2;
    list[3] = NULL;
    SetGObject(obj,list,NO);
    obj->classCur = class;
  }
}

    
static int _SetGList(GOBJECT o, char *field, char **argv)
{
  GLIST list;
  GCLASS class;
  char *str,*name;
  LISTV lv;
  
    /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{{add <name> <class> [-<fields>...]} {Adds a gobject named <name> of class <class> to the glist. After creating it, it sets \
its field according to [-<fields>...].}}}");
    return(YES);
  }
  
  list = (GLIST) o;

  /* Set/Get the absolute/relative position of the glist (propagate the move on all the included objects */
  if (!strcmp(field,"apos") || !strcmp(field,"pos")) {
    if (*argv == NULL) {
      _SetGObject(o,field,argv);
      return(YES);
    }
    _SetGObject(o,field,argv);
    RecomputeAbsolutePositions(list);
    return(YES);
  }
  
  /* Add field */
  if (!strcmp(field,"add")) { 
    argv = ParseArgv(argv,tSTR,&name,tSTR,&str,-1);
    class = (GCLASS) GetElemHashTable(theGClasses,str);    
    if (class == NULL) Errorf("'%s' does not correspond to a gclass",str);
    NewGObject(class,list,name,argv,NO);
    return(YES);
  }
  
  /* bound field */
  if (!strcmp(field,"bound") || !strcmp(field,"?bound")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,0);
      AppendInt2Listv(lv,o->w-1);
      AppendInt2Listv(lv,0);
      AppendInt2Listv(lv,o->h-1);
      SetResultValue(lv);
      return(YES);
    } 
    Errorf("_SetGList() : You cannot set the -bound field of a GList");
  }

  
  return(NO);
}
  

static void _DrawGList (WINDOW win, GOBJECT o, int x, int y, int w,int h)
{
  GOBJECT obj;
  GLIST list = (GLIST) o; 
  int x1,y1,w1,h1;
    
  /* We loop on the gobjects */
  for (obj = list->back;obj != NULL;obj=obj->front) {
    x1 = x; y1 = y; w1 = w; h1 = h;
    if (IntersectClipRect(obj,&x1,&y1,&w1,&h1) == NO) continue;
    DrawGObject_(win,obj,x1,y1,w1,h1);
  }  
}

static void _DeleteContentGList (GOBJECT o)
{
  GLIST list = (GLIST) o;   
  
  /* The objects have already been removed, thus we just need to delete the hash table */
  DeleteHashTable(list->theGObjects);
}

static void _InitGList (GOBJECT o)
{
  GLIST list = (GLIST) o;
  
  list->back = NULL;
    
  list->theGObjects = NewHashTable(8);
} 

static float _IsInGList (GOBJECT o, GOBJECT *res, int x, int y)
{
  GOBJECT obj,obj1;
  GLIST list = (GLIST) o;   
  float dist,d;
  
  dist = -1;
   
  /* We loop on the gobjects (from the front one) */
  for (obj = list->back;obj != NULL;obj=obj->front) {
    d = IsInGObject(obj,&obj1,x,y);
    if (d < 0) continue;
    if (d == 0) {
      *res = obj1;
    }
    if ((dist > 0 && d < dist) || dist == -1) {
      dist = d;
      *res = obj1;
    }
  }
  
  if (dist == -1) {
    if (IsView(o)) dist = FLT_MAX/2;
    else dist = 0;
    *res = o;
  }
  
  return(dist);
} 

static char _MsgeGList(GOBJECT o, char *msge, char ** argv)
{
  AHASHELEM e;
  int r;
  GLIST list;
  GCLASS class;
  GOBJECT obj;
  char *str;
  char *name,flag;
  float x,y;
  int i,j;
  char flagDraw;

  /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{{list [<filterName>=*]} {Gets all the gobject names of the glist that matches <filterName>.}} \
{{object <x> <y>} {Gets the gobject name of the glist that is the closest to the (local coordinate) point <x> <y> (if none it returns the empty string '').}} \
{{remove [<filterName>=*]} {Removes all the gobjects of the glist whose name matches <filterName>.}} \
{{add <name> <class> [<list of field initializations>]} {Adds to the glist a gobject of class <class> and name <name>. \
After creating it, it sets \
its field according to [-<fields>...].}}}");
return(YES);
}
 
  /* List msge */
  if (!strcmp(msge,"list")) { 
    argv = ParseArgv(argv,tSTR_,"*",&str,0);
    if (!IsGList(o)) Errorf("The first argument should be a glist not just a gobject");
    list = (GLIST) o;
    /* We loop on the gobjects */
    for (obj = list->back;obj != NULL;obj=obj->front) {
      if (MatchStr(obj->name,str)) {   
        AppendListResultStr(obj->name);
      }
    }
    return(YES);
  }

  /* object msge */
  if (!strcmp(msge,"object")) { 
    argv = ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,0);
    if (!IsGList(o)) Errorf("The first argument should be a glist not just a gobject");
    list = (GLIST) o;
    Local2Global(o,x,y,&i,&j);
    obj = NULL;
    if (IsInGObject(o, &obj,i,j) < 0 || obj == NULL) return(YES);
    SetResultStr(GetNameGObject(obj));
    return(YES);
  }

  /* Remove msge */
  if (!strcmp(msge,"remove")) { 
    argv = ParseArgv(argv,tSTR_,"*",&str,0);
    list = (GLIST) o;
    flagDraw = NO;
    /* We loop on the gobjects */
    for (r = 0; r<list->theGObjects->nRows;r++) {
      for (e = list->theGObjects->rows[r]; e != NULL;) {    
        obj = (GOBJECT) e;
        e = e->next;
        if (MatchStr(obj->name,str)) {
          flag = obj->flagHide;
          obj->flagHide = YES;
          flagDraw = YES;
          if (!DeleteGObject(obj)) obj->flagHide = flag;
        }
      } 
    }
    if (flagDraw) DrawWholeGObject(o,NO);
    return(YES);
  }

  /* Add msge */
  if (!strcmp(msge,"add")) { 
    argv = ParseArgv(argv,tSTR,&name,tWORD,&str,-1);
    if (!IsGList(o)) Errorf("The first argument should be a glist not just a gobject");
    list = (GLIST) o;
    class = (GCLASS) GetElemHashTable(theGClasses,str);    
    if (class == NULL) Errorf("'%s' does not correspond to a gclass",str);
    NewGObject(class,list,name,argv,YES);
    SetResultInt(1);
    return(YES);
  }
  
  return(NO);
}


/* Is an object a glist ? */
char IsGList(GOBJECT o)
{
  return(o->gclass->typeClass == theGListClass || IsWin(o) || IsView(o));
}
 
/* Add an object to a GList */ 
void AddGObject2GList(GOBJECT o, GLIST list)
{
  GOBJECT o1;
  
  if (IsWin(o)) Errorf("AddGObject2GList() : Cannot add a window to a glist");
  
  AddElemHashTable(list->theGObjects, (AHASHELEM) o);

  o->father = list;
  
  if (list->back == NULL) list->back = o;
  else {
    o1 = list->back;
    while (o1->front != NULL) o1 = o1->front;
    o1->front = o;
  }
  
  o->front = NULL;
}

/* Remove an object to a GList */ 
void RemoveGObject2GList(GOBJECT o, GLIST list)
{
  GOBJECT o1;
  
  GetRemoveElemHashTable(list->theGObjects, o->name);
  
  if (list->back == o) list->back = o->front;
  else {
    o1 = list->back;
    while (o1->front != o) o1 = o1->front;
    o1->front = o->front;
  }
    
  o->nRef = 0;
  o->front = NULL;
}



/***************************************************
 *
 *
 * C functions that define the View class
 *
 *
 ***************************************************/

 
GCLASS theViewClass = NULL;

/*
 * Basic function to get the minimum bounding rectangle of a view 
 *   If *xMin < *xMax then it just returns the values yMin and yMax within [xMin,xMax]
 *   If *yMin < *yMax then it just returns the values xMin and xMax within [yMin,yMax]
 */
static void GetBoundRect(GLIST list, float *xMin, float *xMax, float *yMin, float *yMax)
{
  GOBJECT obj;
  char *l[4];
  char str1[20],str2[20];
  float x0,x1,y0,y1,w0,h0;
  char flagFail,flagX,flagY;
  char flag;
  VALUE val;
  LISTV lv;
  
  flag = toplevelCur->flagStoreResult;
  toplevelCur->flagStoreResult = YES;

  flagX = flagY = NO;
  if (*xMin < *xMax) {
    flagX = YES;
    *yMin = FLT_MAX;
    *yMax = -FLT_MAX;
  }
  else if (*yMin < *yMax) {
    flagY = YES;
    *xMin = FLT_MAX;
    *xMax = -FLT_MIN;
  } else {
    *xMin = *yMin = FLT_MAX;
    *xMax = *yMax = -FLT_MAX;
  }
  
  if (flagX) {
    sprintf(str1,"%g",*xMin);
    sprintf(str2,"%g",*xMax);
  } 
  else if (flagY) {
    sprintf(str1,"%g",*yMin);
    sprintf(str2,"%g",*yMax);
  }
  l[1] = str1;     
  l[2] = str2;     
  l[3] = NULL;

  for (obj = list->back;obj != NULL;obj=obj->front) {
  
    if (obj->flagHide) continue;

    /* No script defined gobject knows about this field */
    if (obj->gclass->setSCommand != NULL) continue;
    
    flagFail = YES;
     
    /* Case the x values are limited */
    if (flagX) {
      /* We try the -rectx message */
      l[0] = "-rectx";
      if (SetGObject(obj,l,NO)) {
        flagFail = NO;
        val = GetResultValue();     
        if (GetTypeValue(val) != listvType) Errorf("GetBoundRect() : Field 'rectx' of gobject '%s' is expected to be a '%s' and not a '%s'",GetNameGObject(obj),listvType,GetTypeValue(val));
        lv = CastValue(val,LISTV);
        if (lv->length != 4 || lv->values[0] != NULL || lv->values[1] != NULL || lv->values[2] != NULL || lv->values[3] != NULL) 
          Errorf("GetBoundRect() : Field 'rectx' of gobject '%s' should return a &listv made of 4 numbers",GetNameGObject(obj));
      }
    }
    else if (flagY) {      
      /* We try the -recty message */
      l[0] = "-recty";
      if (SetGObject(obj,l,NO)) {
        flagFail = NO;
        val = GetResultValue();     
        if (GetTypeValue(val) != listvType) Errorf("GetBoundRect() : Field 'recty' of gobject '%s' is expected to be a '%s' and not a '%s'",GetNameGObject(obj),listvType,GetTypeValue(val));
        lv = CastValue(val,LISTV);
        if (lv->length != 4 || lv->values[0] != NULL || lv->values[1] != NULL || lv->values[2] != NULL || lv->values[3] != NULL) 
          Errorf("GetBoundRect() : Field 'rectx' of gobject '%s' should return a &listv made of 4 numbers",GetNameGObject(obj));
      }
    }

    /* 
     * If no x values nor y values were specified or if the -rectx or -recty messages do not exist 
     * then we send a -rect message
     */
    if (flagFail) {
      l[0] = "-rect";
      l[1] = NULL;
      if (SetGObject(obj,l,NO)) {
        val = GetResultValue();     
        if (GetTypeValue(val) != listvType) Errorf("GetBoundRect() : Field 'rect' of gobject '%s' is expected to be a '%s' and not a '%s'",GetNameGObject(obj),listvType,GetTypeValue(val));
        lv = CastValue(val,LISTV);
        if (lv->length != 4 || lv->values[0] != NULL || lv->values[1] != NULL || lv->values[2] != NULL || lv->values[3] != NULL) 
          Errorf("GetBoundRect() : Field 'rect' of gobject '%s' should return a &listv made of 4 numbers",GetNameGObject(obj));
      }
      else Errorf("GetBoundRect() : gobject '%s' should have a field '-rect'",GetNameGObject(obj));
    }

    x0 = lv->f[0];
    y0 = lv->f[1];
    w0 = lv->f[2];
    h0 = lv->f[3];
    
    x1 = x0+w0;
    y1 = y0+h0;

    /* Set the xMin, xMax, yMin, yMax values */
    if (flagX) { 
      if (!flagFail || (x0 >= *xMin && x0 <= *xMax) || (x1 >= *xMin && x1 <= *xMax) || (x0 <= *xMin && x1 >= *xMax)) {
        *yMin = MIN(*yMin,y0);
        *yMax = MAX(*yMax,y1);
      }          
    }
    else if (flagY) {
      if (!flagFail || (y0 >= *yMin && y0 <= *yMax) || (y1 >= *yMin && y1 <= *yMax) || (y0 <= *yMin && y1 >= *yMax)) {
        *yMin = MIN(*yMin,y0);
        *yMax = MAX(*yMax,y1);
      }
    }
    else {
      *xMin = MIN(*xMin,x0);
      *xMax = MAX(*xMax,x1);
      *yMin = MIN(*yMin,y0);
      *yMax = MAX(*yMax,y1);
    }
  }
  
  toplevelCur->flagStoreResult = flag;
}
     

static int _SetView (GOBJECT o, char *field, char**argv)
{
  VIEW v;
  float xMin,xMax,yMin,yMax,y;
  char *strx0,*strx1,*stry1,*stry0;
  char flagX,flagYMin,flagYMax;
  LISTV lv;

  
  
  /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{{bound [<xMin> <xMax> <yMin> <yMax>]} {Sets/Gets the boundaries of the View. When setting \
the rectangle you can use the special values '*' or '?' for any of the 4 arguments. The value '*' means that the corresponding \
value should not be changed. The value '?' means that the corresponding value should be set in order to frame exactly all the \
gobjects in the View. Thus for instance, if signals are displayed in the View,  '-bound 0 1 ? ?' will display all the signals \
between abscissa <xMin>=0 and <xMax>=1 using their minimum and maximum values (between 0 and 1) to set the y-scale. \
Let us note that when you call '-bound' with arguments it changes the boundary rectangle of the View. If you just want \
to know what the boundary rectangle would be set to if '-bound' (with some specific arguments) was called (without \
changing the boundaries of the View), you must uset the '-?bound' field (e.g., '-?bound 0 1 ? ?')}} \
{{?bound [<xMin> <xMax> <yMin> <yMax>]} {Read only field that works exactly like '-bound' except that it does not change the fields <xMin> \
<xMax> <yMin> and <yMax> (even when arguments are specified). This allows, for instance, to question the view about the <yMin> and <yMax> within a given \
range <xMin> <xMax> without changing the view.}} \
{{reverse [x | y | xy | none]} {Gets/Sets the state that indicates which axis are reversed compared to the regular axis of GLists. \
If it is 'y' (the default value at initialization) then the y-axis will be going from bottom of the window to top, if 'x' the x-axis \
will be going from right to left, if 'xy' both will be combined and if 'none' the y-axis will be top to bottom and the x-axis left to right \
(as for GLists)}}}");
    return(YES);
  }
  
  v = (VIEW) o;
  
  if (!strcmp(field,"reverse")) {
    if (*argv == NULL) {
      if (v->flagReverse == YFlagReverse + XFlagReverse) SetResultStr("xy");
      else if (v->flagReverse == YFlagReverse) SetResultStr("y");
      else if (v->flagReverse == XFlagReverse) SetResultStr("x");
      else SetResultStr("none");      
      return(YES);
    }
    argv = ParseArgv(argv,tSTR,&strx0,0);
    if (!strcmp(strx0,"xy")) v->flagReverse = YFlagReverse + XFlagReverse;
    else if (!strcmp(strx0,"y")) v->flagReverse = YFlagReverse;
    else if (!strcmp(strx0,"x")) v->flagReverse = XFlagReverse;
    else if (!strcmp(strx0,"none")) v->flagReverse = 0;
    else Errorf("_SetView() : Bad value for field 'reverse' '%s'",strx0);
    return(1);
  }
  
  if (!strcmp(field,"bound") || !strcmp(field,"?bound")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendFloat2Listv(lv,v->xMin);
      AppendFloat2Listv(lv,v->xMax);
      AppendFloat2Listv(lv,v->yMin);
      AppendFloat2Listv(lv,v->yMax);
      SetResultValue(lv);
      return(YES);
    }
    
    strx0=NULL;
    if (!ParseFloatStrLevel_(levelCur,*argv,&xMin,&strx0)) Errorf1("");
    if (strx0 && strcmp(strx0,"?") && strcmp(strx0,"*")) Errorf("Bad value '%s' for -bound field",strx0);
    argv++;
    strx1=NULL;
    if (!ParseFloatStrLevel_(levelCur,*argv,&xMax,&strx1)) Errorf1("");
    if (strx1 && strcmp(strx1,"?") && strcmp(strx1,"*")) Errorf("Bad value '%s' for -bound field",strx1);
    argv++;
    stry0=NULL;
    if (!ParseFloatStrLevel_(levelCur,*argv,&yMin,&stry0)) Errorf1("");
    if (stry0 && strcmp(stry0,"?") && strcmp(stry0,"*")) Errorf("Bad value '%s' for -bound field",stry0);
    argv++;
    stry1=NULL;
    if (!ParseFloatStrLevel_(levelCur,*argv,&yMax,&stry1)) Errorf1("");
    if (stry1 && strcmp(stry1,"?") && strcmp(stry1,"*")) Errorf("Bad value '%s' for -bound field",stry1);
    argv++;
    NoMoreArgs(argv);
    
    flagX = flagYMin = flagYMax = NO;
    
    /*
     * Getting the 4 values 
     */
    if (strx0 && !strcmp(strx0,"*")) xMin = v->xMin;
    else if (strx0 && !strcmp(strx0,"?")) {
      xMin = FLT_MAX;
      flagX = YES;
    }

    if (strx1 && !strcmp(strx1,"*")) xMax = v->xMax;
    else if (strx1 && !strcmp(strx1,"?")) {
      xMax = -FLT_MAX;
      flagX = YES;
    }

    if (stry0 && !strcmp(stry0,"*")) yMin = v->yMin;
    else if (stry0 && !strcmp(stry0,"?")) {
      yMin = FLT_MAX;
      flagYMin = YES;
    }

    if (stry1 && !strcmp(stry1,"*")) yMax = v->yMax;
    else if (stry1 && !strcmp(stry1,"?")) {
      yMax = -FLT_MAX;
      flagYMax = YES;
    }
    
    /* Some basic checkings */
    if (xMin == FLT_MAX && xMax != -FLT_MAX || xMin != FLT_MAX && xMax == -FLT_MAX)
      Errorf("You must use at least two successive '?' in -bound field");
     
    /* If there was a '?' then we must call GetBoundRect */
    if (flagX || flagYMin || flagYMax) {
      if (!flagYMin && flagYMax) {
        y = yMin;
        yMin = FLT_MAX;
      }
      else if (flagYMin && !flagYMax) {
        y = yMax;
        yMax = FLT_MIN;
      }
      GetBoundRect((GLIST) v,&xMin,&xMax,&yMin,&yMax);
      if (!flagYMin && flagYMax) yMin = y;
      else if (flagYMin && !flagYMax) yMax = y;
    }
      

    if (xMin > xMax) Errorf("Cannot set xMin > xMax in -bound message");
    if (yMin > yMax) Errorf("Cannot set yMin > yMax in -bound message");
    
    if (fabs(xMin-xMax) < 1e-7) {
      xMin -= 0.5;
      xMax += 0.5;
    }       

    if (fabs(yMin-yMax) < 1e-7) {
      yMin -= 0.5;
      yMax += 0.5;
    }       

    if (!strcmp(field,"bound") && (v->xMin != xMin || v->xMax != xMax || v->yMin != yMin || v->yMax != yMax)) {
      v->xMin = xMin;
      v->xMax = xMax;    
      v->yMin = yMin;
      v->yMax = yMax;
    
      RecomputeAbsolutePositions((GLIST) v);
      lv = TNewListv();
      AppendFloat2Listv(lv,xMin);
      AppendFloat2Listv(lv,xMax);
      AppendFloat2Listv(lv,yMin);
      AppendFloat2Listv(lv,yMax);
      SetResultValue(lv);
      return(YES);
    }
    else {
      lv = TNewListv();
      AppendFloat2Listv(lv,xMin);
      AppendFloat2Listv(lv,xMax);
      AppendFloat2Listv(lv,yMin);
      AppendFloat2Listv(lv,yMax);
      SetResultValue(lv);
      return(-1);
    }
  }

  if (!strcmp(field,"asize") || !strcmp(field,"size")) {
    _SetGObject(o,field,argv);
    if (*argv != NULL) RecomputeAbsolutePositions((GLIST) v);
    return(YES);
  }    

  return(NO);
}  

static void _InitView (GOBJECT o)
{
  VIEW v;
  
  v = (VIEW) o;
  
  v->flagReverse = YFlagReverse;
  
  v->xMin = v->yMin = 0;
  v->xMax = v->yMax = 1;
} 

  
/* Is the object 'o' a view ? */  
char IsView(GOBJECT o)
{
  return(o->gclass->typeClass == theViewClass);
}



/***************************************************
 *
 *
 * C functions that define the Grid class
 *
 *
 ***************************************************/

GCLASS theGridClass = NULL;

static void _InitGrid (GOBJECT o)
{
  GRID grid;
  
  grid = (GRID) o;
  
  grid->gridM = grid->gridN = 1;
  
  grid->dx = grid->dy = 0;
  
  grid->topMargin = grid->bottomMargin = grid->leftMargin = grid->rightMargin = 0;  
} 


/* Function to updat the positions of all the objects included in a grid */
static void UpgradeGridObjectPositions(GRID grid)
{
  GOBJECT obj;
  
  for (obj = ((GLIST) grid)->back;obj != NULL;obj=obj->front) {
    if (!(obj->flagGrid)) continue;
    ComputeGridRect(obj);
  }
}

static int _SetGrid (GOBJECT o, char *field, char**argv)
{
  GRID grid;
  int x,y,w,h;
  int ans;
  LISTV lv;
  
   /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{{mn [<m> <n>]} {Sets/Gets the horizontal (<m>) and the vertical (<n>) size of the Grid. These two numbers must be strictly positive \
integers}} \
{{dxdy [<dx> <dy>]} {Sets/Gets the horizontal (<dx>) and the vertical (<dy>) margins between each cell of the grid.}} \
{{margin [<left> <top> <right> <bottom>]} {Sets/Gets the margins for the Grid. All the gobjects that will use grid \
coordinates will be placed inside the rectangle <left> <top> <right> <botttom>. Thus, for instance,  the grid coordinates \
1,1 corresponds to the point <left> <top>.}}}");
    return(YES);
  }
  
  grid = (GRID) o;
  
  /* Set the grid size */
  if (!strcmp(field,"mn")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,grid->gridM);
      AppendInt2Listv(lv,grid->gridN);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,tINT,&y,0);
    if (grid->gridM == x && grid->gridN == y) return(-1);
    grid->gridM = x;
    grid->gridN = y;
    UpgradeGridObjectPositions(grid);
    return(YES);
  }

  /* Set the margins */
  if (!strcmp(field,"margin")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,grid->leftMargin);
      AppendInt2Listv(lv,grid->topMargin);
      AppendInt2Listv(lv,grid->rightMargin);
      AppendInt2Listv(lv,grid->bottomMargin);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,tINT,&y,tINT,&w,tINT,&h,0);
    
    if (grid->leftMargin == x && grid->topMargin == y && grid->rightMargin == w && grid->bottomMargin == h) return(-1);

    grid->leftMargin = x;
    grid->topMargin = y;
    grid->rightMargin = w;
    grid->bottomMargin = h;
    UpgradeGridObjectPositions(grid);
    return(YES);
  }

  /* Set the dxdy */
  if (!strcmp(field,"dxdy")) {
    if (*argv == NULL) {
      lv = TNewListv();
      AppendInt2Listv(lv,grid->dx);
      AppendInt2Listv(lv,grid->dy);
      SetResultValue(lv);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&x,tINT,&y,0);
    if (grid->dx == x && grid->dy == y) return(-1);
    grid->dx = x;
    grid->dy = y;
    UpgradeGridObjectPositions(grid);
    return(YES);
  }
      
  /* Set/Get the absolute/relative size of the grid */
  if (!strcmp(field,"asize") || !strcmp(field,"size")) {
    ans = _SetGObject(o,field,argv);
    if (*argv != NULL && ans == YES) UpgradeGridObjectPositions(grid);
    return(ans);
  }
 
  return(NO);
}  
  



/***************************************************
 *
 *
 * C functions that define the Window class
 *
 *
 ***************************************************/

GCLASS theWindowClass = NULL;

/* The Hash table of all the windows */
HASHTABLE theWindowsHT = NULL;
WINDOW theWindows = NULL;
 
 
static void _DeleteContentWindow (GOBJECT o)
{
  WINDOW w = (WINDOW) o;
  
  if (w->title != NULL) {
    Free(w->title); 
    w->title = NULL;
  }
  if (w->frame != (FRAME) NULL) {
    WDeleteFrame(w->frame);  
    w->frame = (FRAME) NULL;
  }
  if (toplevelCur->lastWindow == w) toplevelCur->lastWindow = NULL;
}


static void _InitWindow (GOBJECT o)
{
  WINDOW w = (WINDOW) o;
  WINDOW w1;
  
  o->rw = o->w = 200;
  o->rh = o->h = 200;
  o->rx = o->x = 100;
  o->ry = o->y = 100;
    
  AddElemHashTable(theWindowsHT,(AHASHELEM) o);
  w->flagHide = YES;

  if (theWindows == NULL) theWindows = w;
  else {
    w1 = theWindows;
    while(w1->front != NULL) w1 = (WINDOW) w1->front;
    w1->front = (GOBJECT) w;
    w->front = NULL;
  }
    
  w->frame = (FRAME) NULL;
  w->title = CopyStr(o->name);
  w->flag = WindowNoFlag;
} 

static char _MsgeWindow(GOBJECT o, char *msge, char ** argv)
{
  char *filename;
  
  /* The help command */
  if (o == NULL) {
    SetResultStr("\
{{{drawps <file.ps>} {Creates a (color) postscript file of the window. See command 'ps' to control the postscript output.}}}");
return(YES);
}

  /* The 'drawps' message  */
  if (!strcmp(msge,"drawps")) {
    argv = ParseArgv(argv,tSTR,&filename,0);
    PSOpen(o,filename);
    DrawWholeGObject(o,YES);
    PSClose();
    return(YES);
  }
  
  return(NO);
}

static int _SetWindow (GOBJECT o, char *field, char **argv)
{
  char *str;
  int w,h;
  WINDOW win;

   /* The help command */
  if (o == NULL) {
    SetResultStr("{{{title [<title>]} {Sets/Gets the title of the Window. At initialization the title is the name of the gobject.}}}");
    return(YES);
  }
  
    
  win = (WINDOW) o;   
  
  /* Set/Get the absolute/relative position of a window */
  if (!strcmp(field,"apos") || !strcmp(field,"pos")) {
    _SetGObject(o,field,argv);
    if (*argv != NULL && (win->flagHide == NO || win->flag != WindowFlagNoChangeFrame)) WChangeFrame(win->frame,NULL,win->x,win->y,win->w,win->h); 
    return(-1);
  }

  /* Set/Get the absolute/relative size of a window */
  if (!strcmp(field,"asize") || !strcmp(field,"size")) {
    w = o->w;
    h = o->h;
    _SetGrid(o,field,argv);
    if ((o->w != w || o->h != h) && o->flagHide == NO) {
      if (win->flag != WindowFlagNoChangeFrame) { 
        WChangeFrame(win->frame,NULL,win->x,win->y,win->w,win->h);
        win->flag = 0;
// WindowFlagNoUpdate;
      }
      DrawWholeGObject(o,NO);
      return(YES);
    }
    return(-1);
  }

  /* Set/Get the hide flag */
  if (!strcmp(field,"hide")) {
    if (*argv == NULL) {
      SetResultInt((int) o->flagHide);
      return(YES);
    }
    argv = ParseArgv(argv,tINT,&h,0);
    if (h) HideGObject(o);
    else ShowGObject(o);
    return(YES);
  }
  
  /* Set/Get the title */
  if (!strcmp(field,"title")) {
    if (*argv == NULL) {
      if (win->title) SetResultStr(win->title);
      return(YES);
    }
    argv = ParseArgv(argv,tSTR,&str,0);
    Free(win->title);
    win->title = CopyStr(str);
    if (win->frame) WChangeFrame(win->frame,str,win->x,win->y,win->w,win->h);
    return(-1);
  }
  
  return(NO);
} 

  
/* Is an object a window ? */
char IsWin(GOBJECT o)
{
  return(o->gclass->typeClass == theWindowClass);
}

/* Get the window an object is displayed in */
WINDOW GetWin(GOBJECT o)
{
  while (o != NULL && IsWin(o) == NO)  o = (GOBJECT) o->father;
  
  if (o == NULL) 
    Errorf("GetWin() : Weird error");
  
  return((WINDOW) o);
}

/* Create a new window */
WINDOW NewWin(GCLASS class, char *name, char **argv)
{
  WINDOW win;
  
  win = (WINDOW) NewGObject(class,NULL, name, argv,NO);

  toplevelCur->lastWindow = win;
  
  
  return(win);
}


/*
 * Parsing Windows
 */

char ParseWindow_(char *arg, WINDOW def, WINDOW *w)
{
  WINDOW w1;

  *w = def;
    
  if (arg == NULL) {
    SetErrorf("ParseWindow_() : NULL string cannot be converted to a window");
    return(NO);
  }

  if (*arg == '\0') {
    SetErrorf("ParseWindow_() : empty string cannot be converted to an window");
    return(NO);
  }
  
  /* The current window ? */
  if (!strcmp(arg,".")) {
    *w = toplevelCur->lastWindow;
    if (*w == NULL) {
      SetErrorf("ParseWindow_() : Sorry, no current window available");
      *w = def;
      return(NO);
    }
    (*w)->classCur = (*w)->gclass;
    return(YES);
  }
    
  w1 = (WINDOW) GetElemHashTable(theWindowsHT,arg);

  if (w1 == NULL) {
    SetErrorf("ParseWindow_() : '%s' is not a window",arg);
    return(NO);
  }
  
  *w = w1;
  (*w)->classCur = (*w)->gclass;
  return(YES);
}

void ParseWindow(char *arg, WINDOW *w)
{
  if (ParseWindow_(arg,NULL,w) == NO) Errorf1("");
}


/* The window command */
void C_Window(char **argv)
{
  char *action,*str,*str1;
  GCLASS class;
  WINDOW w;
    
  argv = ParseArgv(argv,tWORD,&action,-1);
  

  /* New action */
  if (!strcmp(action,"new")) { 
    argv = ParseArgv(argv,tSTR,&str,-1);
    
    /* Get the window class */
    class  = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) class = theWindowClass;
    else {
      if (class->typeClass != theWindowClass) Errorf("You must specify a window class");
      argv = ParseArgv(argv,tSTR,&str,-1);
    }
    
    /* Check the window name */
    str1 = str;
    if (!IsValidSymbolChar1(*str1)) Errorf("Bad name for window '%s'",str);
    while (*str1 != '\0' && IsValidSymbolChar(*str1)) str1++;
    if (*str1 != '\0') Errorf("Bad name for window '%s'",str);

    /* Does window exist ? */    
    if (GetElemHashTable(theWindowsHT,str) != NULL) Errorf("Window '%s' already exists",str);

    /* Create it */
    NewWin(class,str,argv);
    
    return;
  }
  
  /* List action */
  if (!strcmp(action,"list")) { 
    ParseArgv(argv,tSTR_,"*",&str,0);
    SetResultStr("");
    /* We loop on the windows */
    for (w = (WINDOW) theWindows;w != NULL;w= (WINDOW) w->front) {
      if (MatchStr(w->name,str)) AppendListResultStr(w->name);
    }
    return;
  }    
  
  Errorf("Bad action '%s'",action);
}



/***************************************************
 *
 *
 * C functions that deals with class structures GClass
 *
 *
 ***************************************************/


/* The hash table of all the classes */
HASHTABLE theGClasses = NULL;


/* Send message to a class  */
static void * SendMessage2GObjectClass(void *content,int message,void **arg)
{
  Errorf("SendMessage2GObjectClass() : No message expected");
  return(NULL);
}

/* Is 'subclass' a sub class of 'class' */
char IsSubClass(GCLASS subClass, GCLASS class)
{
  if (subClass == NULL || class == NULL) Errorf("IsSubClass() : class should not be NULL");
  
  /* First some shortcuts */
  if (class == theWindowClass) return(subClass->typeClass == theWindowClass);
  if (class == theGListClass) return((subClass->typeClass == theWindowClass) || (subClass->typeClass == theGListClass) || (subClass->typeClass == theViewClass));
  if (class == theViewClass) return((subClass->typeClass == theViewClass));
  if (class == theGObjectClass) return(YES);
  
  /* Then the regular case */
  while (1) {
    if (subClass == class) return(YES);
    subClass = subClass->fatherClass;
    if (subClass == NULL) return(NO);
  }
}


/*
 * Creating or modifying a graphic class 
 *
 *    name : the class name
 *    fatherClass : the graphic class the class "name" will inherit from
 *
 * If the class 'name' does not exist it creates and initializes it.
 * If it exists and is not protected then it just initializes it.
 *
 * This function return the graphic class named 'name'
 */
 
GCLASS NewGClass(char *name,GCLASS fatherClass, char *packageName)
{
  GCLASS class;
  int i;

  /* Do not accept NULL father unless for GObject class */
  if (strcmp("GObject",name) && fatherClass == NULL) Errorf("NewGClass() : You must specify a father class");
  
  /* Get the class named 'name' if any and resets it */
  if ((class = (GCLASS) GetElemHashTable(theGClasses,name)) != NULL) {
    if (!(class->flags & GClassProtected))  {
      Printf("  ...redefining gclass '%s'\n",name);
      if (class->setSCommand) DeleteValue(class->setSCommand);
      if (class->msgeSCommand) DeleteValue(class->msgeSCommand);
      if (class->drawSCommand) DeleteValue(class->drawSCommand);
      if (class->isInSCommand) DeleteValue(class->isInSCommand);
      class->isInSCommand = class->drawSCommand = class->msgeSCommand = class->setSCommand = NULL;
      if (class->packageName) Free(class->packageName);
      if (strcmp(class->info,"")) Free(class->info);
    }
    else Errorf("NewGClass() : gclass '%s' cannot be modified", name);
  }
  
  /* If it does not exist just create it ! */
  if (class == NULL) {
    class = (GCLASS) Malloc(sizeof(struct gClass));
    class->name = CopyStr(name);
    class->nRef = 0;
    AddElemHashTable(theGClasses,(AHASHELEM) class);
    for (i=0;i<LastEventCategory;i++) class->theBindings[i] = NULL;
  }
  
  /* Default flags */
  class->flags = GClassProtected + GClassMoveResize + GClassLocalCoor;
  
  /* Sets the fatherClass */
  class->fatherClass = fatherClass;
  
  /* Basic initializations of the class fields */
  class->nbBytes = sizeof(GObject);
  class->typeClass = class;
  class->varType = NULL;
  class->draw = NULL;
  class->deleteContent = NULL;
  class->init = NULL;
  class->isIn = NULL;
  class->set = NULL;
  class->deleteNotify = NULL;
  class->msge = NULL;
  class->setSCommand = NULL;
  class->msgeSCommand = NULL;
  class->drawSCommand = NULL;
  class->isInSCommand = NULL;
  class->packageName = NULL;
  class->info = "";
  if (packageName) class->packageName = CopyStr(packageName);
  
    
  
  /* More initializations based on inheritance (if any) */  
  if (fatherClass != NULL) {
    class->nbBytes = fatherClass->nbBytes;
    class->typeClass = fatherClass->typeClass;
    class->draw = fatherClass->draw;
    class->isIn = fatherClass->isIn;
    class->deleteNotify = fatherClass->deleteNotify;
    class->varType = fatherClass->varType;
    class->flags  = fatherClass->flags;
  }
    
  return(class);
}


/*
 * The gclass command 
 */
 
void C_GClass(char **argv)
{
  char *str,*str1,*name,*action,**l,**l1;
  PROC msge,set,isin,draw;
  int r;
  AHASHELEM e;
  GCLASS class,class1,fatherClass;
  char opt,flagMoveResize,flagLocalCoor;
  char *type,*type1,*varType,*varType1;
  char *list[4];
  char *packageName;
  char *info;
  LISTV lv,lv1;
  VALUE val;
  GOBJECT *gobjList,o;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* The list action */
  if (!strcmp(action,"list")) {
    ParseArgv(argv,tSTR_,"*",&str,tSTR_,"*",&packageName,tSTR_,NULL,&varType,0);
    if (varType != NULL) {
      varType1 = GetArgType(varType);
      if (varType1 == NULL) Errorf("Bad variable type '%s'",varType);
    }
    else varType1 = NULL;
    
    lv = TNewListv();
    
    /* We loop on the classes */
    for (r = 0; r<theGClasses->nRows;r++) {
      for (e = theGClasses->rows[r]; e != NULL; e = e->next) { 
        if (MatchStr(e->name,str) && 
            ((!strcmp(packageName,"") && (((GCLASS) e)->packageName == NULL))  || !strcmp(packageName,"*") || (((GCLASS) e)->packageName &&  MatchStr(((GCLASS) e)->packageName,packageName))) && 
            (varType1 == NULL || varType1 == ((GCLASS) e)->varType)) AppendStr2Listv(lv,e->name);
      }
    }
    SetResultValue(lv);
    return;
  }
  
  /* The objlist action */
  if (!strcmp(action,"objlist")) {
    ParseArgv(argv,tSTR,&str,tGOBJECTLIST,&gobjList, 0);
    lv = TNewListv();
    if (!strcmp(str,"*")) class = NULL;
    else {
      class = (GCLASS) GetElemHashTable(theGClasses,str);
      if (class == NULL) Errorf("Class '%s' does not exist",str);
    }
    for (; *gobjList!= NULL ; gobjList++) {
      o = *gobjList;
      if (class == NULL || o->gclass == class || IsSubClass(o->gclass,class)) {
        AppendStr2Listv(lv,GetNameGObject(o));
      }
    }
    SetResultValue(lv);
    return;
  }
  

  /* The father action */
  if (!strcmp(action,"father")) {
    argv = ParseArgv(argv,tSTR,&str,tSTR_,NULL,&str1,0);
    class = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) Errorf("Class '%s' does not exist",str);
    if (str1 != NULL) {
      class1 = (GCLASS) GetElemHashTable(theGClasses,str1);
      if (class1 == NULL) Errorf("Class '%s' does not exist",str1);
      SetResultInt((int) IsSubClass(class->fatherClass,class1) || class==class1);
    }
    else  if (class->fatherClass != NULL) SetResultStr(class->fatherClass->name);
    else  SetResultStr("");
    return;
  }

  /* The type action */
  if (!strcmp(action,"type")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    class = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) Errorf("Class '%s' does not exist",str);
    if (class->varType) SetResultStr(class->varType);
    return;
  }
  
  /* The info action */
  if (!strcmp(action,"info")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    class = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) Errorf("Class '%s' does not exist",str);
    SetResultStr(class->info);
    return;
  }

  /* The package action */
  if (!strcmp(action,"package")) {
    argv = ParseArgv(argv,tSTR,&str,0);
    class = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) Errorf("Class '%s' does not exist",str);
    if (class->packageName) SetResultStr(class->packageName);
    return;
  }


  /* The help action */
  if (!strcmp(action,"help")) {
    argv = ParseArgv(argv,tSTR,&str,-1);
    class = (GCLASS) GetElemHashTable(theGClasses,str);
    if (class == NULL) Errorf("Class '%s' does not exist",str);
    argv = ParseArgv(argv,tSTR,&str,0);
    if (!strcmp("setg",str)) {   
      if (class->setSCommand) {
        list[0] = "'?'";
        list[1] = "''";
        list[2] = NULL;
        ApplyProc2List(class->setSCommand,list);
      }      
      else if (class->set) (*(class->set))(NULL,NULL,NULL);      
    }
    else if (!strcmp("msge",str)) {   
      if (class->msgeSCommand) {
        list[0] = "'?'";
        list[1] = "''";
        list[2] = NULL;
        ApplyProc2List(class->msgeSCommand,list);
      }      
      else if (class->msge) (*(class->msge))(NULL,NULL,NULL);
    }
    else  Errorf("Bad argument '%s'",str);

    if (GetResultType() != strType) str = TCopyStr("");
    else str = GetResultStr();

    lv = TNewListv();

    if (str[0] == '\0') {
      SetResultValue(lv);
      return;
    }
    
    ParseWordList(str,&l);
    for (;*l!= NULL;l++) {
      ParseWordList(*l,&l1);
      lv1 = TNewListv();
      for (;*l1!= NULL;l1++) AppendStr2Listv(lv1,*l1);
      AppendValue2Listv(lv,(VALUE) lv1);
    }
    
    SetResultValue(lv);

    return;
  }
  
  /* The new action */
  if (!strcmp(action,"new")) {
    argv = ParseArgv(argv,tWORD,&name,tWORD,&str,-1);

    ParseArgv(argv,tVAL,&val,-1);
    if (GetTypeValue(val) == nullType) set = NULL;
    else ParseArgv(argv,tPROC,&set,-1);
    argv++;

    ParseArgv(argv,tVAL,&val,-1);
    if (GetTypeValue(val) == nullType) msge = NULL;
    else ParseArgv(argv,tPROC,&msge,-1);
    argv++;

    ParseArgv(argv,tVAL,&val,-1);
    if (GetTypeValue(val) == nullType) draw = NULL;
    else ParseArgv(argv,tPROC,&draw,-1);
    argv++;

    argv = ParseArgv(argv,tSTR,&info,-1);
    if (*argv != NULL && **argv != '-') argv = ParseArgv(argv,tPROC,&isin,-1);
    else isin = NULL;   

    type1 = NULL;
    flagMoveResize = flagLocalCoor =  YES;
    while(opt = ParseOption(&argv)) { 
    switch(opt) {
      case 't': 
	    argv = ParseArgv(argv,tSTR,&type,-1);
	    type1 = GetArgType(type);
	    if (type1 == NULL) Errorf("Type '%s' does not exist",type);
	    break;
      case 'm':
        flagMoveResize = NO; 
	    break;
     case 'l':
        flagLocalCoor = NO; 
	    break;
      default: ErrorOption(opt);
    }
    }
    NoMoreArgs(argv);
  
    fatherClass = (GCLASS) GetElemHashTable(theGClasses,str);
    if (fatherClass == NULL) Errorf("GClass '%s' does not exist",str);
    class = NewGClass(name,fatherClass,NULL);
  
    if (set != NULL) {
      if (set->flagSP == NO) Errorf("The set procedure should be a script procedure");
      if (class->setSCommand != NULL) DeleteValue(class->setSCommand);
      class->setSCommand = set;
      if (set) AddRefValue( set);
      class->set = NULL;
    }
    if (msge != NULL) {
      if (msge->flagSP == NO) Errorf("The msge procedure should be a script procedure");
      if (class->msgeSCommand != NULL) DeleteValue(class->msgeSCommand);
      class->msgeSCommand = msge;
      if (msge) AddRefValue( msge);
      class->msge = NULL;
    }
    if (draw != NULL) {
      if (draw->flagSP == NO) Errorf("The draw procedure should be a script procedure");
      if (class->drawSCommand != NULL) DeleteValue(class->drawSCommand);
      class->drawSCommand = draw;
      if (draw) AddRefValue( draw);
      class->draw = NULL;
    }
    if (isin != NULL) {
      if (isin->flagSP == NO) Errorf("The isin procedure should be a script procedure");
      if (class->isInSCommand != NULL) DeleteValue(class->isInSCommand);
      class->isInSCommand = isin;
      if (isin) AddRefValue( isin);
      class->isIn = NULL;
    }

    class->info = CopyStr(info);

    if (type1 != NULL) {
      class->varType = type1;
    }
    
    if (!flagMoveResize) class->flags &= ~GClassMoveResize;
    if (!flagLocalCoor) class->flags &= ~GClassLocalCoor;
    class->flags &= ~GClassProtected;
    
    if (toplevelCur->packageName) class->packageName = CopyStr(toplevelCur->packageName);
    
    return;
  }  
  
  /* The delete action */
  if (!strcmp(action,"delete")) {
    Errorf("Action 'delete' of command 'gclass' is not implemented yet");
    return;
  }  
  
  Errorf("Unknown action '%s'",action);
} 



/***************************************************
 *
 *
 * Functions that must be called once at startup or at the end
 *
 *
 ***************************************************/

/* Creation of the basic classes) */
void CreateBasicGClasses(void)
{   
  extern void InitTerminalBindings(void);

  /* Creating the hash table of the gobject classes */
  theGClasses = NewHashTable(10);

  /* We must create the gobject class (the father of all the classes) */
  theGObjectClass = NewGClass("GObject",NULL,"kernel");  
  theGObjectClass->typeClass = theGObjectClass;
  theGObjectClass->init = _InitGObject;
  theGObjectClass->set = _SetGObject; 
  theGObjectClass->msge = _MsgeGObject; 
  theGObjectClass->info = "Basic Graphic Class which any other class inherits from";

    
  /* Then we must create a glist class */
  theGListClass = NewGClass("GList",theGObjectClass,"kernel");  
  theGListClass->nbBytes = sizeof(GList);
  theGListClass->draw = _DrawGList;
  theGListClass->deleteContent = _DeleteContentGList;
  theGListClass->init = _InitGList;
  theGListClass->isIn = _IsInGList;
  theGListClass->set = _SetGList;
  theGListClass->msge = _MsgeGList;
  theGListClass->typeClass = theGListClass;
  theGListClass->info = "Graphic Class which allows to group a list of gobjects";

  /* Then we must create a grid class */
  theGridClass = NewGClass("Grid",theGListClass,"kernel");  
  theGridClass->nbBytes = sizeof(Grid);
  theGridClass->init = _InitGrid;
  theGridClass->set = _SetGrid;
  theGridClass->info = "GList Graphic Class which allows to display its gobjects using grid coordinates. The glist is divided into 'm' columns \
and 'n' lines. The gobjects inside this glist can thus be placed and sized specifying column and row numbers (using the '-grid' field). \
Everything will be automatically resized and repositionned whenever the glist is resized or moved. Let us note that one can specify margins \
to the grid using the '-margin' fields";

  /* The View class */
  theViewClass = NewGClass("View",theGListClass,"kernel");
  theViewClass->nbBytes = sizeof(View);
  theViewClass->set = _SetView; 
  theViewClass->typeClass = theViewClass;
  theViewClass->init = _InitView;
  theViewClass->info = "GList Graphic Class which allows to display its gobjects using float coordinates. The bounding rectangle \
of a view corresponds to float bounds referred to as <xMin> <xMax> <yMin> and <yMax> that one can set using the '-bound' field";

  /* Then we must create a window class */
  theWindowClass = NewGClass("Window",theGridClass,"kernel");
  theWindowClass->nbBytes = sizeof(struct window);
  theWindowClass->deleteContent = _DeleteContentWindow;
  theWindowClass->init = _InitWindow;
  theWindowClass->set = _SetWindow;  
  theWindowClass->msge = _MsgeWindow;  
  theWindowClass->typeClass = theWindowClass;
  theWindowClass->info = "Basic Window Class";
  
  /* The hash table for storing all the windows */
  theWindowsHT  = NewHashTable(30);

  /* Init the terminal bindings */  
  InitTerminalBindings();  
}  
  

/*
 * Initialization of all the graphics (to be called once at startup)
 */
void InitGraphics(void)
{
  extern void ColorsInit(void);

  WOpenGraphics();
  ColorsInit();  
  CreateBasicGClasses();  
}

/*
 * Closing all the graphics (to be called once when quitting LastWave)
 */    
void CloseGraphics(void)
{
  WCloseGraphics();
}
  
  
