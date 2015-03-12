/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'extrema1d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1999-2002 Emmanuel Bacry,                             */
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
/*  ext_graph.c   Functions for displaying extrema                          */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "extrema1d.h"



/* The GOBJECT structure for displaying a extrep */
typedef struct graphExtrep {

  GObjectFields; 
  
  /* The EXTREP to be displayed */
  EXTREP extrep;
  
  /* The chained flag */
  char flagChained;
    
} GraphExtrep, *GRAPHEXTREP;

/* The corresponding class */      
GCLASS theGraphExtrepClass = NULL;


/* Initialization of the GraphExtrep structure */
static void _InitGraphExtrep(GOBJECT o)
{
  GRAPHEXTREP graph;

  graph = (GRAPHEXTREP) o;

  graph->extrep = NULL;
  
  graph->bgColor = invisibleColor;

  graph->rectType = LargeRect;
  
  graph->flagChained = NO;
}


/* Deleting the content of a GraphExtrep */
static void _DeleteContentGraphExtrep(GOBJECT o)
{
  GRAPHEXTREP graph;

  graph = (GRAPHEXTREP) o;

  if (graph->extrep != NULL) DeleteExtrep(graph->extrep);
}


/* The setg method */
static int _SetGraphExtrep (GOBJECT o, char *field, char**argv)
{
  GRAPHEXTREP graph;
  EXTREP extrep;
  int i;
  LISTV lv;
 
   /* The help command */
  if (o == NULL) {
    SetResultStr("{{{graph [<extrep>]} {Gets/Sets the extrema representation to be displayed by the GraphExtrep with <extrep>.}} \
{{chained [<flagOnOff>]} {Sets/Gets the chained flag. If 1 then it will link on the display the chained extrema.}}}");
    return(YES);
  }
   
  graph = (GRAPHEXTREP) o;

  
  /* the 'graph' and 'cgraph' fields */
  if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) {
    if (*argv == NULL) {
      SetResultValue(graph->extrep);
      return(YES);
    }
    argv = ParseArgv(argv,tEXTREP,&extrep,0);
    if (extrep->nOct == 0) Errorf("_SetGraphExtrep() : You cannot display an empty 'extrep'");
    if (graph->extrep != NULL) DeleteExtrep(graph->extrep);
    graph->extrep = extrep;
    AddRefValue(extrep);
    o->rx = graph->extrep->x0;
    o->rw = graph->extrep->dx*(graph->extrep->size-1);
    o->ry = 0;
    o->rh = graph->extrep->nOct*graph->extrep->nVoice-1;
    UpdateGlobalRectGObject(o);    
    /* Should we chain ? */
    if (extrep->D[2][0]->first != NULL && extrep->D[2][0]->first->finer != NULL) graph->flagChained = YES;
    else graph->flagChained = NO;
    return(YES);
  }
  
  /* the 'chained' field */
  if (!strcmp(field,"chained")) {
    if (*argv == NULL) SetResultInt((int) graph->flagChained);
    else {
      argv = ParseArgv(argv,tINT,&i,0);
      graph->flagChained = i != 0;
    }
    return(YES);
  }
  
  /* The 'rect' field */
  if (!strcmp(field,"rect")) {
    NoMoreArgs(argv);
    if (graph->extrep == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0);
      AppendFloat2Listv(lv,0); 
      return(YES);
    }
    o->rx = graph->extrep->x0;
    o->rw = graph->extrep->dx*(graph->extrep->size-1);
    o->ry = 0;
    o->rh = graph->extrep->nOct*graph->extrep->nVoice-1;
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
static void _DrawGraphExtrep (WINDOW win, GOBJECT obj, int x, int y,int w,int h)
{
  GRAPHEXTREP graph;
  EXTREP extrep;
  float xx,yy,ww,hh;
  EXTLIS extlis;
  EXT ext,ext1,extOld;
  int o,v;
  
  /* Some inits */
  graph = (GRAPHEXTREP) obj;
  extrep = graph->extrep;
  if (extrep == NULL) return;
  if (extrep->nOct == 0) return;
  
  /* Get the local boundaries */
  Global2LocalRect(obj,x,y,w,h,&xx,&yy,&ww,&hh,LargeRect);    
  
  /* If not chained */
  if (!graph->flagChained) {
    /* Loop on the extrema */
    for (o=1;o<=extrep->nOct;o++) {
      for (v = 0;v<extrep->nVoice;v++) {
        extlis = extrep->D[o][v];
        if (extlis->first == NULL) continue;
        if (extlis->first->scale < yy) continue;
        if (extlis->first->scale > yy+hh) break;
        for (ext = extlis->first;ext !=NULL; ext = ext->next) {
          if (ext->abscissa < xx) continue;
          if (ext->abscissa > xx+ww) break;
          WDrawPoint(obj,ext->abscissa,ext->scale);
        }
      }
    }
  }
  
  /* If chained */
  else {
    /* Loop on the extrema from the smallest scale */
    extlis = extrep->D[1][0];
    for (ext = extlis->first;ext!= NULL; ext=ext->next) {
      if (ext->abscissa < xx) continue;
      if (ext->abscissa > xx+ww) break;
      extOld = ext;
      for (ext1 = ext->coarser; ext1 != NULL; ext1 = ext1->coarser) {
        WDrawLine(obj,extOld->abscissa,extOld->scale,ext1->abscissa,ext1->scale);
        extOld = ext1;
      }
    }
  } 
}


/* Defining the GraphExtrep gclass */  
void DefineGraphExtrep(void)
{
  theGraphExtrepClass = NewGClass("GraphExtrep",theGObjectClass,"extrema1d"); 
  theGraphExtrepClass->nbBytes = sizeof(GraphExtrep);
  theGraphExtrepClass->init = _InitGraphExtrep;
  theGraphExtrepClass->deleteContent = _DeleteContentGraphExtrep;
  theGraphExtrepClass->set = _SetGraphExtrep;
  theGraphExtrepClass->draw = _DrawGraphExtrep;   
  theGraphExtrepClass->varType = extrepType;
  theGraphExtrepClass->flags &= ~(GClassMoveResize+GClassLocalCoor);
  theGraphExtrepClass->info = "Graphic Class that allows to display 1d extrema representation";
}

