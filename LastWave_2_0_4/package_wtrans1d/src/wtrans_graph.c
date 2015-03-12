/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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
/*  wtrans.c   Functions for displaying wtrans                              */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "wtrans1d.h"



/* The GOBJECT structure for displaying a wtrans */
typedef struct graphWtrans {

  GObjectFields; 
  
  /* The wtrans to be displayed */
  WTRANS wtrans;
  
  /* The colormap used */
  unsigned long cm;

  /* The normalization mode */
  char norm;

  /* The causal flag */
  char flagCausal;
    
} GraphWtrans, *GRAPHWTRANS;

/* The corresponding class */      
GCLASS theGraphWtransClass = NULL;

/* The different modes of normalization */
enum {
  WRLineLocal = 0,
  WRLineGlobal,
  WRLocal,
  WRGlobal
};



/* Initialization of the GraphWtrans structure */
static void _InitGraphWtrans(GOBJECT o)
{
  GRAPHWTRANS graph;

  graph = (GRAPHWTRANS) o;

  graph->wtrans = NULL;
  
  graph->cm = GetColorMapCur();
  
  graph->bgColor = invisibleColor;

  graph->rectType = SmallRect;
  
  graph->norm = WRLineGlobal;
  graph->flagCausal = NO;
}


/* Deleting the content of a GraphWtrans */
static void _DeleteContentGraphWtrans(GOBJECT o)
{
  GRAPHWTRANS graph;

  graph = (GRAPHWTRANS) o;

  if (graph->wtrans != NULL) DeleteWtrans(graph->wtrans);
}


/* The setg method */
static int _SetGraphWtrans (GOBJECT o, char *field, char**argv)
{
  GRAPHWTRANS graph;
  WTRANS wtrans;
  int norm;
  char *str;
  int i;
  LISTV lv;

   /* The help command */
  if (o == NULL) {
    SetResultStr("{{{graph [<wtrans>]} {Gets/Sets the wavelet transform to be displayed by the GraphWtrans. (The '-cgraph' field \
is equivalent to that field).}} \
{{cm [<colormap>]} {Sets/Gets the colormap that will be used to display the wavelet transform.}} \
{{norm [<type>]} {Sets/Gets the normalization used for displaying a wavelet transform. In any case, \
the absolute value of the wavelet transform is coded. However, there are 4 choices : \n\
- 'global' : The colors are coded from the global minimum of the absolute value and the global maximum \n\
- 'local' : The colors are coded from the local (among all the values displayed in the View)  minimum of the absolute \
value and the local maximum. Let us note that this mode is different from 'global' only when the image is zoomed \n\
- 'lglobal' : The colors are coded separately at each scale from the global minimum of the absolute value (at each scale) \
and the global maximum \n\
- 'llocal' : The colors are coded separately at each scale from the local (among all the values displayed in the View) \
minimum of the absolute value (at each scale) and the local maximum.}} \
{{causal [<flagOnOff>]} {Sets/Gets the causal flag. If 1 then it will not display all the values which were \
affected by border effects.}}}");
    return(YES);
  }
   
      
  graph = (GRAPHWTRANS) o;
  
  
  /* the 'graph' and 'cgraph' fields */
  if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) {
    if (*argv == NULL) {
      SetResultValue(graph->wtrans);
      return(YES);
    }
    argv = ParseArgv(argv,tWTRANS,&wtrans,0);
    if (wtrans->nOct == 0) Errorf("_SetGraphWtrans() : You cannot display an empty 'wtrans'");
    if (graph->wtrans != NULL) DeleteWtrans(graph->wtrans);
    graph->wtrans = wtrans;
    AddRefValue(wtrans);
    o->rx = graph->wtrans->x0-graph->wtrans->dx/2;
    o->rw = graph->wtrans->dx*(graph->wtrans->size);
    o->ry = -.5;
    o->rh = graph->wtrans->nOct*graph->wtrans->nVoice;
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

  /* The renorm field */
  if (!strcmp(field,"norm")) {
    if (*argv == NULL) {
      switch(graph->norm) {
      case WRLineLocal : SetResultStr("llocal"); break;
      case WRLineGlobal : SetResultStr("lglobal"); break;
      case WRLocal : SetResultStr("local"); break;
      case WRGlobal : SetResultStr("global"); break;
      }
      return(YES);  
    }    
    argv = ParseArgv(argv,tSTR,&str,0);
    if (!strcmp(str,"local")) norm = WRLocal;
    else if (!strcmp(str,"global")) norm = WRGlobal;
    else if (!strcmp(str,"lglobal")) norm = WRLineGlobal;
    else if (!strcmp(str,"llocal")) norm = WRLineLocal;
    else Errorf("_SetGraphWtrans() : Bad value '%s' for 'renorm' field");
    if (norm == graph->norm) return(-1);
    graph->norm = norm;
    return(YES);
  }

  /* the 'causal' field */
  if (!strcmp(field,"causal")) {
    if (*argv == NULL) SetResultInt((int) graph->flagCausal);
    else {
      argv = ParseArgv(argv,tINT,&i,0);
      graph->flagCausal = i != 0;
    }
    return(YES);
  }
  
  /* The 'rect' field */
  if (!strcmp(field,"rect")) {
    NoMoreArgs(argv);
    if (graph->wtrans == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0); 
      return(YES);
    }
    o->rx = graph->wtrans->x0-graph->wtrans->dx/2;
    o->rw = graph->wtrans->dx*(graph->wtrans->size);
    o->ry = -.5;
    o->rh = graph->wtrans->nOct*graph->wtrans->nVoice;
    UpdateGlobalRectGObject(o);

    lv = TNewListv();
    SetResultValue(lv);
    
    AppendFloat2Listv(lv,o->rx);
    AppendFloat2Listv(lv,o->ry);
    AppendFloat2Listv(lv,o->rw);
    AppendFloat2Listv(lv,o->rh);
    
    return(YES);
  }

  return(NO);
}


/* The drawing procedure */
static void _DrawGraphWtrans (WINDOW win, GOBJECT obj, int x, int y,int w,int h)
{
  GRAPHWTRANS graph;
  GOBJECT o1;
  WTRANS wtrans;
  SIGNAL sig;
  int nColors;
  unsigned long color,bg;
  int grey;
  int nRow,nCol;
  int i1,i2;
  float boundX1,boundX2,boundY1,boundY2,f;
  int o,v,index;
  float xMin,xMax,yMin,yMax,valMin,valMax;
  float fx,fy;
  int i,j;

  /* Some inits */
  graph = (GRAPHWTRANS) obj;
  wtrans = graph->wtrans;
  if (wtrans == NULL) return;
  if (wtrans->nOct == 0) return;

  /* Get the number of colors of the colormap */
  nColors = ColorMapSize(graph->cm);

  /* Get the BackGroundColor (not invisible) for using when drawing in case flagCausal == 1 */
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
  
  /* Get the bounding limits in x (and corresponding index) for renormalization */
  if (graph->norm == WRLocal || graph->norm == WRLineLocal) {
    Global2Local((GOBJECT) graph->father,graph->father->x,graph->father->y,&boundX1,&boundY1);
    Global2Local((GOBJECT) graph->father,graph->father->x+graph->father->w,graph->father->y+graph->father->h,&boundX2,&boundY2);
    if (boundX1 > boundX2) {
      f = boundX2;
      boundX2 = boundX1;      
      boundX1 = f;
    }
  }
  else {
    boundX1 = wtrans->x0;
    boundX2 = wtrans->dx*wtrans->size+wtrans->x0;
  }
  
  /* Compute the global renormalization factor if necessary */
  valMin = FLT_MAX/2;
  valMax = -FLT_MAX/2;
  if (graph->norm == WRLocal ||  graph->norm == WRGlobal) {
    for (o = 1;o<=wtrans->nOct;o++) {
      for (v = 0;v<wtrans->nVoice;v++) {
        xMin = boundX1;
        xMax = boundX2;
        sig = wtrans->D[o][v];
        MinMaxSig(sig,&xMin,&xMax,&yMin,&yMax,&i1,&i2,graph->flagCausal);
        valMin = MIN(valMin,yMin);
        valMax = MAX(valMax,yMax);
      }
    }
  }
    
  /* Allocation of the pixmap */
  WInitPixMap(nRow,nCol);
 
  /*
   * Loop on the rows 
   */
  for (i=y;i<=y+h;i++) {
  
    /* Get local ordinate fy */
    Global2Local(obj,0,i,&fx,&fy);
    
    /* Transform it in octave number and voice number */
    if (fy < 0) fy  = 0;
    fy = (int) (fy+.5);
    o = ((int) fy) / wtrans->nVoice + 1;
    v = ((int) fy) % wtrans->nVoice;
    if (o > wtrans->nOct) {
      o = wtrans->nOct;
      v = wtrans->nVoice-1;
    }
    else if (o <= 0) {
      o  = 1;
      v = 0;
    }
    sig = wtrans->D[o][v];


    /* Compute line renormalization factors if necessary */
    if (graph->norm == WRLineLocal || graph->norm == WRLineGlobal) {
      sig = wtrans->D[o][v];
      xMin = boundX1;
      xMax = boundX2;
      MinMaxSig(sig,&xMin,&xMax,&yMin,&yMax,&i1,&i2,graph->flagCausal);
      valMin = yMin;
      valMax = yMax;
    }

    /* Symmetry between negative and positive values for renormalization */
    valMax = MAX(fabs(valMax),fabs(valMin));
    valMin = -valMax;

    /*
     * Loop on the columns 
     */
    for (j=x;j<=x+w;j++) {

      /* Get local abscissa */
      Global2Local(obj,j,0,&fx,&fy);
      
      /* Transform it into an index for the signal */
      fx -= sig->x0;
      fx /= sig->dx;
      fx = (int) (fx+.5);
      if (fx < 0) fx = 0;
      else if (fx >= sig->size) fx = sig->size-1;
      index = fx;
      
      /* If flagCausal is set and the point is not in the causal boundaries then use color 'bg' */
      if (graph->flagCausal && (index < sig->firstp || index > sig->lastp)) color = bg;
      
      /* Otherwise just compute the color */
      else {
      
        /* Get the color and the grey level */
        color = (int) (nColors * fabs(sig->Y[index])/valMax); 
        grey = (int) (256 * fabs(sig->Y[index])/valMax); 
      
        color = (color>=nColors ? nColors-1 : color);
        grey = (grey>=256 ? 255 : grey);
	  
	    color = (color<0 ? 0 : color);
	    grey = (grey<0 ? 0 : grey);
	    
	    color += graph->cm;
      }  

      /* Then set the point */ 
      WSetPixelPixMap(i-y,j-x,color);       
    } 
  }

  /* Display the image */
  WDisplayPixMap(win,x,y);
}


/* Defining the GraphWtrans gclass */  
void DefineGraphWtrans(void)
{
  theGraphWtransClass = NewGClass("GraphWtrans",theGObjectClass,"wtrans1d"); 
  theGraphWtransClass->nbBytes = sizeof(GraphWtrans);
  theGraphWtransClass->init = _InitGraphWtrans;
  theGraphWtransClass->deleteContent = _DeleteContentGraphWtrans;
  theGraphWtransClass->set = _SetGraphWtrans;
  theGraphWtransClass->draw = _DrawGraphWtrans;   
  theGraphWtransClass->varType = wtransType;
  theGraphWtransClass->flags &= ~(GClassMoveResize+GClassLocalCoor);
  theGraphWtransClass->info = "Graphic Class that allows to display 1d wavelet transform";
  
}

