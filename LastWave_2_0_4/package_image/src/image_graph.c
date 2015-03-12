/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0                        */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry, Jerome Fraleu.              */
/*      emails : fraleu@cmap.polytechnique.fr                               */
/*               lastwave@cmap.polytechnique.fr                             */
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
/*  image_graph.c   Functions for displaying image                          */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "images.h"


  
/* The GOBJECT structure for displaying an image */
typedef struct graphImage {

  GObjectFields; 
  
  /* The image to be displayed */
  IMAGE image;
  
  /* The colormap used */
  unsigned long cm;
 
  /* The norm field */
  float norm;
  char flagAbsVal;
  
} GraphImage      , *GRAPHIMAGE;

/* The corresponding class */      
GCLASS theGraphImageClass = NULL;



/* Values for ->norm field */
enum {
  NormNone  = 0,
  NormMax  = -1
}; 
  
  
/* Initialization of the GraphImage structure */
static void _InitGraphImage(GOBJECT o)
{
  GRAPHIMAGE graph;

  graph = (GRAPHIMAGE) o;

  graph->image = NewImage();
  
  graph->cm = GetColorMapCur();
  
  graph->bgColor = invisibleColor;

  graph->rectType = SmallRect;
  
  graph->norm = NormMax;
  graph->flagAbsVal = YES;
}


/* Deleting the content of a GraphImage */
static void _DeleteContentGraphImage(GOBJECT o)
{
  GRAPHIMAGE graph;

  graph = (GRAPHIMAGE) o;

  DeleteImage(graph->image);
}


/* The setg method */
static int _SetGraphImage (GOBJECT o, char *field, char**argv)
{
  GRAPHIMAGE graph;
  IMAGE image;
  char *str;
  float f;
  LISTV lv;
  VALUE val;

  /* The help command */
  if (o == NULL) {
      SetResultStr("{{{graph [<image>]} {Gets/Sets the image to be displayed by the GraphImage. (The '-cgraph' field \
is equivalent to that field).}} \
{{cm [<colormap>]} {Sets/Gets the colormap that will be used to display the image.}} \
{{norm '[+][none' | 'max' | <number>]} {Sets/Gets the normalization mode. The '+' sign indicates whether the absolute values \
are coded (if '+' is specified) or the signed values. If the mode is 'none', then \
no normalization is done, i.e., the values of the image are taken as color indexes in the colormap. Whatever index \
is out of range is replaced by either the first color of the colormap (for negative indexes) or the last one. If it is 'max' \
then the values (resp. absolute values) are normalized between -<max> (resp. 0) (first color of the colormap) and <max> (last color), \
where <max> stands for the maximum absolute value. \
If it is a positive number \
<number> then the values (resp. absolute values) are normalized between -<number> (resp. 0) (first color) and <number> (last color). Default value is 'max'.}}}");
    return(YES);
  }
     
      
  graph = (GRAPHIMAGE) o;

  
  /* the 'graph' and 'cgraph' fields */
  if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) {
    if (*argv == NULL) {
      SetResultValue(graph->image);
      return(YES);
    }
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    if (image->ncol == 0 || image->nrow == 0) Errorf("_SetGraphImage() : You cannot display an empty image");
    DeleteImage(graph->image);
    
  
    if (!strcmp(field,"graph")) {
      graph->image = image;
      AddRefValue(image);
    }
    else {
      graph->image = NewImage();
      CopyImage(image,graph->image);
    }
  
  o->rx = -.5;
    o->rw = image->ncol;
    o->ry = -.5;
    o->rh = image->nrow;
    UpdateGlobalRectGObject(o);   
    return(YES);
  }

  /* The colormap field */
  if (!strcmp(field,"cm")) {
    if (*argv == NULL) {
      SetResultStr(GetColorMapName(graph->cm));
      return(YES);  
    }    
    argv = ParseArgv(argv,tCOLORMAP,&(graph->cm),0);
    return(YES);
  }

  /* The norm field */
  if (!strcmp(field,"norm")) {
    if (*argv == NULL) {
      if (graph->norm == NormNone && graph->flagAbsVal) SetResultStr("+none");
      else if (graph->norm == NormNone) SetResultStr("none");
      else if (graph->norm == NormMax && graph->flagAbsVal) SetResultStr("+max");
      else if (graph->norm == NormMax) SetResultStr("max");
      else if (graph->flagAbsVal) SetResultf("+%g",graph->norm);
      else SetResultFloat(graph->norm);
      return(YES);  
    }    
    ParseVal(*argv,&val);
    if (GetTypeValue(val) == strType) {
      str = CastValue(val,STRVALUE)->str;
      if (*str=='+') {graph->flagAbsVal = YES; str++;}
      else {graph->flagAbsVal = NO;}
      if (!strcmp(str,"none")) graph->norm = NormNone; 
      else if (!strcmp(str,"max")) graph->norm = NormMax; 
    }
    else {
      str = *argv;
      if (*str == '+') {graph->flagAbsVal = YES; str++;}
      else {graph->flagAbsVal = NO;}
      ParseFloat(str,&f);
      if (f <= 0) Errorf("_SetGraphImage() : Bad '-norm' value '%s'",*argv);
      graph->norm = f;
    }
    argv++;
    NoMoreArgs(argv);
    return(YES);
  }
  
  /* The 'rect' field */
  if (!strcmp(field,"rect")) { 
    NoMoreArgs(argv);
    o->rx = -.5;
    o->rw = graph->image->ncol;
    o->ry = -.5;
    o->rh = graph->image->nrow;
    lv = TNewListv();
    SetResultValue(lv);
    AppendFloat2Listv(lv,o->rx);
    AppendFloat2Listv(lv,o->ry);
    AppendFloat2Listv(lv,o->rw);
    AppendFloat2Listv(lv,o->rh );

    UpdateGlobalRectGObject(o);
    return(YES);
  }

  return(NO);
}


/* The drawing procedure */
static void _DrawGraphImage (WINDOW win, GOBJECT obj, int x, int y,int w,int h)
{
  GRAPHIMAGE graph;
  GOBJECT o1;
  IMAGE image;
  
  int nColors;
  long c;
  unsigned long color,bg;
  int nRow,nCol;
  int indexY,indexX;
  float valMin,valMax,val;
  float fx,fy;
  int i,j;

   /* Some inits */
  graph = (GRAPHIMAGE) obj;
  image = graph->image;
  if (image == NULL) return;
  if (image->ncol == 0 || image->nrow == 0) return;


  if (graph->norm == NormMax) {
    MinMaxImage(image,NULL,NULL,&valMin,NULL,NULL,&valMax);
    valMax = MAX(fabs(valMin), fabs(valMax));
  }
  else if (graph->norm == NormNone) valMax = -1;
  else valMax = graph->norm;
    
  /* Get the number of colors of the colormap */
  nColors = ColorMapSize(graph->cm);

  bg = graph->bgColor;
  o1 = obj;
  while(bg & invisibleColor) {
    o1 = (GOBJECT) o1->father;
    if (o1 == NULL) break;
    bg = o1->bgColor;
  }
  if (bg &invisibleColor) bg = bgColor;
  
  /* The size of the image to be drawn */
  nRow = h+1;  
  nCol = w+1;
  
 
  /* Allocation of the pixmap */
  WInitPixMap(nRow,nCol);
 
  /*
   * Loop on the rows 
   */
  for (i=y;i<=y+h;i++) {
  
    /* Get local ordinate fy */
    Global2Local(obj,0,i,&fx,&fy);
    
    /* Transform it in the number of the row */
       fy = (int) (fy+.5);
      if (fy < 0) fy = 0;
      else if (fy >= image->nrow) fy = image->nrow-1;
      indexY = fy;
   /*
     * Loop on the columns 
     */
    for (j=x;j<=x+w;j++) {

      /* Get local abscissa */
      Global2Local(obj,j,0,&fx,&fy);
      
      /* Transform it into an index for the signal */
      
      fx = (int) (fx+.5);
      if (fx < 0) fx = 0;
      else if (fx >= image->ncol) fx = image->ncol-1;
      indexX = fx;
     

      val = image->pixels[indexY*image->ncol+indexX];
      if (graph->flagAbsVal) val = fabs(val);
      if (graph->norm == NormNone) c = (int) (val+.5);
      else if (graph->flagAbsVal) c = (nColors * val)/valMax; 
      else c = (nColors * (val/(2*valMax)+.5)); 
      c = (c>=nColors ? nColors-1 : c);
	  c = (c<0 ? 0 : c);
	  color = c+graph->cm;

      /* Then set the point */ 
      WSetPixelPixMap(i-y,j-x,color);       
    } 
  }

  /* Display the image */
  WDisplayPixMap(win,x,y);
}


/* Defining the GraphImage gclass */  
void DefineGraphImage(void)
{
  theGraphImageClass = NewGClass("GraphImage",theGObjectClass,"image"); 
  theGraphImageClass->nbBytes = sizeof(GraphImage);
  theGraphImageClass->init = _InitGraphImage;
  theGraphImageClass->deleteContent = _DeleteContentGraphImage;
  theGraphImageClass->set = _SetGraphImage;
  theGraphImageClass->draw = _DrawGraphImage;  
  theGraphImageClass->varType = imageiType;
  theGraphImageClass->flags &= ~(GClassMoveResize+GClassLocalCoor);
  theGraphImageClass->info = "Graphic Class that allows to display image";
  
}

