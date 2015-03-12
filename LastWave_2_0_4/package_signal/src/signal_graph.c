/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0                       */
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





/****************************************************************************/
/*                                                                          */
/*  signal_graph.c   Functions for displaying signals                       */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "signals.h"


/* The GOBJECT structure for displaying a signal */
typedef struct graphSignal {

  GObjectFields; 
  
  /* The signal to be displayed */
  SIGNAL signal;
  
  /* The way to display it (see enum below) */
  char curve;
  
  /* And a parameter to specify the latter field */
  float parameterCurve;
  
  /* Is the display causal ? */
  char flagCausal;
  
} GraphSignal, *GRAPHSIGNAL;


/* The corresponding class */            
GCLASS theGraphSignalClass = NULL;


/* The types of curve */
enum {
  LineGraphType = 0,
  DLineGraphType,
  DotGraphType,
  CircleGraphType,
  BarGraphType,
  CrossGraphType
};

/* Initialization of the SignalWtrans structure */
static void _InitGraphSignal(GOBJECT o)
{
  GRAPHSIGNAL graph;

  graph = (GRAPHSIGNAL) o;

  graph->signal = NewSignal();
  
  graph->curve = LineGraphType;
  graph->parameterCurve = 0;

  graph->flagCausal = NO;
  
  graph->bgColor = invisibleColor;

  graph->rectType.left = graph->rectType.right = graph->rectType.bottom = graph->rectType.top = 20;
}

/* Deleting the content of a GraphSignal */
static void _DeleteContentGraphSignal(GOBJECT o)
{
  GRAPHSIGNAL graph;

  graph = (GRAPHSIGNAL) o;

  DeleteSignal(graph->signal);
}


/* The setg method */
static int _SetGraphSignal (GOBJECT o, char *field, char**argv)
{
  GRAPHSIGNAL graph;
  char c;
  SIGNAL signal;
  int i,j;
  float xMin,xMax,yMin,yMax,f;
  LISTV lv;
  
  
  /* The help command */
  if (o == NULL) {
    SetResultStr("{{{graph [<signal>]} {Gets/Sets the signal to be displayed by the GraphSignal with <signal>.}} \
{{cgraph [<signal>]} {Gets/Sets the signal to be displayed by the GraphSignal with a copy of <signal>.}} \
{{causal [<flagOnOff>]} {Sets/Gets the causal flag. If 1 then it will not display all the values which were \
affected by border effects.}} \
{{curve [<symbol> [<parameter>]]} {Sets/Gets the symbol which is used to draw the signal. There are several choices \n\
- '_' : A plain line is used. \n\
- '-' : A dashed line is used (since this symbols is also used for specifying fields, for using it in a \
'setg' command, you must escape it using two successive '-', e.g., 'setg ..signal -curve - -').\n\
- '|' : A histogram-type display will be used. The argument <parameter> specifies the y-value the boxes of the histogram \
will start at (default is 0). \n\
- '+' : Crosses of size <parameter> will be used. \n\
- 'o' : Circles of size <parameter> will be used. If <parameter> < 0 then the cirlces will be filled.}}}");
    return(YES);
  }
  
  graph = (GRAPHSIGNAL) o;
  
  /* the 'graph' and 'cgraph' field */
  if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) {
    if (*argv == NULL) {
      SetResultValue(graph->signal);
      return(YES);
    }
    argv = ParseArgv(argv,tSIGNALI,&signal,0);
    DeleteSignal(graph->signal);
    if (!strcmp(field,"graph")) {
      graph->signal = signal;
      AddRefValue(signal);
    }
    else {
      graph->signal = NewSignal();
      CopySig(signal,graph->signal);
    }
    xMin = 1;
    xMax = -1;
    MinMaxSig(graph->signal,&xMin,&xMax,&yMin,&yMax,&i,&j,NO);
    o->rx = xMin;
    o->ry = yMin;
    o->rw = xMax-xMin;
    o->rh = yMax-yMin;
    UpdateGlobalRectGObject(o);
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

  /* the 'curve' field */
  if (!strcmp(field,"curve")) {
    if (*argv == NULL) {
      switch(graph->curve) {
        case LineGraphType : SetResultStr("_"); break;
        case DLineGraphType : SetResultStr("-"); break;
        case CircleGraphType : lv = TNewListv(); SetResultValue(lv); AppendStr2Listv(lv,"o"); AppendInt2Listv(lv,(int) graph->parameterCurve); break;
        case CrossGraphType : lv = TNewListv(); SetResultValue(lv); AppendStr2Listv(lv,"+"); AppendInt2Listv(lv,(int) graph->parameterCurve); break;
        case DotGraphType : SetResultStr("."); break;
        case BarGraphType : lv = TNewListv(); SetResultValue(lv); AppendStr2Listv(lv,"|"); AppendFloat2Listv(lv,graph->parameterCurve); break; 
      }
      return(YES);
    }
    argv = ParseArgv(argv,tCHAR,&c,-1);
    switch(c) {
      case '_' : NoMoreArgs(argv); graph->curve = LineGraphType; graph->lineStyle = LinePlain; break;
      case '-' : NoMoreArgs(argv); graph->curve = DLineGraphType; graph->lineStyle = LineDash; break;
      case '.' : NoMoreArgs(argv); graph->curve = DotGraphType; graph->lineStyle = LinePlain; break;
      case 'o' : 
        argv = ParseArgv(argv,tINT_,4,&i,0);
        if (i == 0) Errorf("_SetGraphSignal() : Size of circles must be different from 0");
        graph->parameterCurve = i;
        graph->curve = CircleGraphType; 
        graph->lineStyle = LinePlain;
        break;
      case '+' : 
        argv = ParseArgv(argv,tINT_,4,&i,0);
        if (i <= 0) Errorf("_SetGraphSignal() : Size of crosses must be strictly greater than 0");
        graph->parameterCurve = i;
        graph->curve = CrossGraphType; 
        graph->lineStyle = LinePlain;
        break;
      case '|' : 
        argv = ParseArgv(argv,tFLOAT_,0.,&f,0);
        graph->parameterCurve = f;
        graph->curve = BarGraphType; 
        graph->lineStyle = LinePlain;
        break;
      default : Errorf("_SetGraphSignal() : Bad value '%c' for field 'curve'",c);
    }
    return(YES);
  }
  
  /* The 'rect' field */
  if (!strcmp(field,"rect")) {
    NoMoreArgs(argv);
    xMin = 1;
    xMax = -1;
    MinMaxSig(graph->signal,&xMin,&xMax,&yMin,&yMax,&i,&j,NO);
    lv = TNewListv();
    SetResultValue(lv);
    AppendFloat2Listv(lv,xMin);
    AppendFloat2Listv(lv,yMin);
    AppendFloat2Listv(lv,xMax-xMin);
    AppendFloat2Listv(lv,yMax-yMin);
    o->rx = xMin;
    o->ry = yMin;
    o->rw = xMax-xMin;
    o->rh = yMax-yMin;
    UpdateGlobalRectGObject(o);
    return(YES);
  }

  /* The 'rectx' field */
  if (!strcmp(field,"rectx")) {
    argv = ParseArgv(argv,tFLOAT,&xMin,tFLOAT,&xMax,0);
    MinMaxSig(graph->signal,&xMin,&xMax,&yMin,&yMax,&i,&j,NO);
    lv = TNewListv();
    SetResultValue(lv);
    AppendFloat2Listv(lv,xMin);
    AppendFloat2Listv(lv,yMin);
    AppendFloat2Listv(lv,xMax-xMin);
    AppendFloat2Listv(lv,yMax-yMin);
    return(YES);
  }

  return(NO);
}


/* The drawing procedure */
static void _DrawGraphSignal (WINDOW win, GOBJECT o, int x, int y,int w,int h)
{
  GRAPHSIGNAL graph;
  SIGNAL signal;
  int iMin,iMax;
  int i;
  char flagLastPointPlotted;
  int xCur, yCur, xLast, yLast,xNext,yNext,yBase;
  int yInf, ySup;
  float x0,y0,x1,y1;
  int rightSizeBox,leftSizeBox;

  /* Some init */    
  graph = (GRAPHSIGNAL) o;
  signal = graph->signal;
  if (signal->size == 0) return;
  
  /* Special case we should not draw anything */
  if (signal == NULL) Errorf("_DrawGraphSignal() : No signal specified in GraphSignal object");
  if (signal->size == 0) return;

  /* Getting the minimum and maximum indexes the drawing should be perform on */
  Global2LocalRect(o,x,y,w,h,&x0,&y0,&x1,&y1,LargeRect);
  x1 += x0;
  y1 += y0;
  iMin = ISig(signal,x0)-1;  
  iMax = ISig(signal,x1)+1;
  iMin = MAX(0,iMin);
  iMax = MIN(signal->size-1,iMax);

  if (graph->flagCausal) {
    iMin = MAX(iMin,signal->firstp);
    iMax = MIN(iMax,signal->lastp);
  }

  if (graph->curve == BarGraphType)
     Local2Global(o,0,graph->parameterCurve,&xCur,&yBase);

         
  /*
   * Let's start the loop !
   */
     
  flagLastPointPlotted = NO;
  yInf = INT_MAX;
  ySup = INT_MIN;
   
  for (i = iMin ; i <= iMax; i++) {
  
    /* Computing the point coordinates */    
    Local2Global(o,XSig(signal,i),signal->Y[i],&xCur,&yCur);
        
    switch(graph->curve) {

    /* Drawing lines */    
    case LineGraphType : case DLineGraphType :
      if (i != iMin && xLast != xCur) {
        if (yInf < ySup-1) WDrawLine((GOBJECT) win,xLast, yInf, xLast, ySup);
	    if (flagLastPointPlotted == NO) WDrawPoint((GOBJECT) win,xCur,yCur);
        else WDrawLine((GOBJECT) win,xLast, yLast, xCur, yCur);
        yInf = yCur;
        ySup = yCur;
      }
      else {
        yInf = MIN(yInf,yCur);
        ySup = MAX(ySup,yCur);
        if (i == iMax) {
          if (i == iMin || flagLastPointPlotted == NO) WDrawPoint((GOBJECT) win,xCur, yCur);
          else if (yInf < ySup-1)  WDrawLine((GOBJECT) win,xLast, yInf, xLast, ySup);
        }
      }       

      flagLastPointPlotted = YES;
      xLast = xCur;
      yLast = yCur;
      
      break;

    /* Drawing bars */    
    case BarGraphType :
      
      /* We first compute the rightSizeBox */
      if (i+1 < signal->size) {
        Local2Global(o,XSig(signal,i+1),signal->Y[i+1],&xNext,&yNext);
        rightSizeBox = (xNext-xCur)/2;
      }
      else {
        if (i != iMin) rightSizeBox = (xCur-xLast)/2;
        else if (i > 0) {
          Local2Global(o,XSig(signal,i-1),signal->Y[i-1],&xLast,&yLast);
          rightSizeBox = (xCur-xLast)/2;
        } else rightSizeBox = 1;
      }
  
      /* We then compute the leftSizeBox */
      if (i != iMin) leftSizeBox = (xCur-xLast)/2;
      else if (i > 1) {
        Local2Global(o,XSig(signal,i-1),signal->Y[i-1],&xLast,&yLast);
        leftSizeBox = (xCur-xLast)/2;
      } else leftSizeBox = rightSizeBox;
      
      rightSizeBox--;
      leftSizeBox--;
      
      /* Then Filling the rect */ 
	  WFillRect((GOBJECT) win,xCur-leftSizeBox,yBase,leftSizeBox+rightSizeBox,yCur-yBase,YES,LargeRect); 

      xLast = xCur;
      yLast = yCur;

      
      break;

	          
    /* Drawing Dots */   
    case DotGraphType :
      WDrawPoint((GOBJECT) win,xCur,yCur);
      break;
   
    /* Drawing Circles */   
    case CircleGraphType :
      if (graph->parameterCurve > 0) WDrawCenteredEllipse((GOBJECT) win,xCur,yCur,graph->parameterCurve,graph->parameterCurve,YES);
      else WFillCenteredEllipse((GOBJECT) win,xCur,yCur,-graph->parameterCurve,-graph->parameterCurve,YES);
      break;

    /* Drawing Crosses */   
    case CrossGraphType :
      WDrawCenteredCross((GOBJECT) win,xCur,yCur,graph->parameterCurve,YES);
      break;
     
    }
  } 
}


/* The isin procedure */
static float _IsInGraphSignal(GOBJECT o, GOBJECT *o1, int x, int y)
{
  float rx, ry,rx1,ry1,d;
  GRAPHSIGNAL g;
  int i;
  float index;

  g = (GRAPHSIGNAL) o;
  *o1 = NULL;
  
  /* Get the local coordinate */
  Global2Local(o,x,y,&rx,&ry);
  
  /* Get the corresponding index in the signal */  
  index = X2FIndexSig(g->signal,rx);
  
  /* If the index is out of the bounds of the signal then we consider the point is not in the signal */
  if (index < 0 || index > g->signal->size-1 || 
      (index == g->signal->size-1 && g->signal->type == XYSIG && g->signal->X[g->signal->size-1] < rx)) return(-1);
  
  /* Otherwise we must compute the distance of the signal to the point and return it */
  i = (int) (index+.5);
  rx1 = XSig(g->signal,i);
  ry1 = g->signal->Y[i];
  rx -= rx1;
  ry -= ry1;
  d = sqrt(rx*rx + ry*ry);
  
  *o1 = o;
  
  return(d);
}

/* Defining the GraphSignal gclass */  
void DefineGraphSignal(void)
{
  theGraphSignalClass = NewGClass("GraphSignal",theGObjectClass,"signal"); 
  theGraphSignalClass->nbBytes = sizeof(GraphSignal);
  theGraphSignalClass->init = _InitGraphSignal;
  theGraphSignalClass->deleteContent = _DeleteContentGraphSignal;
  theGraphSignalClass->set = _SetGraphSignal;
  theGraphSignalClass->draw = _DrawGraphSignal; 
  theGraphSignalClass->isIn = _IsInGraphSignal; 
  theGraphSignalClass->varType = signaliType;
  theGraphSignalClass->flags &= ~(GClassMoveResize+GClassLocalCoor);
  theGraphSignalClass->info = "Graphic Class that allows to display signals";
}

