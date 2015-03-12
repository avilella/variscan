/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'circles'                          */
/*                                                                          */
/*      Copyright (C) 2002 Emmanuel Bacry                                   */
/*      email : lastwave@cmap.polytechnique.fr                              */
/*                                                                          */
/*..........................................................................*/

#include "lastwave.h"
#include "int_fsilist.h"


/************************************************************************
 * 
 * 
 * Defining the CIRCLES VALUE
 * 
 * 
 ************************************************************************/


/* A single circle */
typedef struct circle { 
  float x;
  float y;
  float r;
} Circle;

/* The CIRCLES VALUE */
typedef struct circles {

  /* The fields of the VALUE structure */
  ValueFields;

  Circle *array;
  
  int n;
  
  char *name;
} Circles, *CIRCLES;

/* The corresponding &type */
static char *circlesType = "&circles";



/*
 * The type structure for CIRCLES
 */

extern CIRCLES NewCircles(void);
extern void DeleteCircles(CIRCLES c);
extern void ClearCircles(CIRCLES c);
extern CIRCLES  CopyCircles(CIRCLES in, CIRCLES out);
extern char *ToStrCircles(CIRCLES c, char flagShort);
extern void PrintCircles(CIRCLES c);
extern void PrintInfoCircles(CIRCLES c);

extern struct field fieldsCircles[];

TypeStruct tsCircles = {

  "{{{&circles} {A description of the type '&circles'.}}}",  /* Documentation */

  &circlesType,       /* The basic (unique) type name */
  NULL,               /* The GetType function */                       
  
  DeleteCircles,     /* The Delete function */
  NewCircles,     /* The Delete function */
  
  CopyCircles,       /* The copy function */
  ClearCircles,       /* The clear function */
  
  ToStrCircles,       /* String conversion */
  PrintCircles,   /* The Print function : print the object when 'print' is called */
  PrintInfoCircles,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsCircles,      /* The list of fields */
};


/*
 * The field list
 */
static void * GetExtractCirclesV(CIRCLES c,void **arg);
static void * SetExtractCirclesV(CIRCLES c,void **arg);
static void * GetExtractOptionsCirclesV(CIRCLES c,void **arg);
static void * GetExtractInfoCirclesV(CIRCLES c,void **arg);
static void * GetNameCirclesV(CIRCLES c, void **arg);
static void * SetNameCirclesV(CIRCLES c, void **arg);
static void * GetNCirclesV(CIRCLES c, void **arg);
static void * SetNCirclesV(CIRCLES c, void **arg);
static void * GetExtractOptionsRXYCirclesV(CIRCLES c,void **arg);
static void * GetExtractInfoRXYCirclesV(CIRCLES c,void **arg);
static void * GetRCirclesV(CIRCLES c, void **arg);
static void * SetRCirclesV(CIRCLES c, void **arg);
static void * GetXCirclesV(CIRCLES c, void **arg);
static void * SetXCirclesV(CIRCLES c, void **arg);
static void * GetYCirclesV(CIRCLES c, void **arg);
static void * SetYCirclesV(CIRCLES c, void **arg);

struct field fieldsCircles[] = {

  "", GetExtractCirclesV, SetExtractCirclesV, GetExtractOptionsCirclesV, GetExtractInfoCirclesV,
  "r", GetRCirclesV, SetRCirclesV, GetExtractOptionsRXYCirclesV, GetExtractInfoRXYCirclesV,
  "x", GetXCirclesV, SetXCirclesV, GetExtractOptionsRXYCirclesV, GetExtractInfoRXYCirclesV,
  "y", GetYCirclesV, SetYCirclesV, GetExtractOptionsRXYCirclesV, GetExtractInfoRXYCirclesV,
  "name", GetNameCirclesV, SetNameCirclesV, NULL, NULL,
  "n", GetNCirclesV, SetNCirclesV, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};
  




CIRCLES NewCircles(void)
{
  CIRCLES c;
  extern TypeStruct tsCircles;

  c = Malloc(sizeof(Circles));
  
  InitValue(c,&tsCircles);
  
  c->array = NULL;
  c->n = 0;
  c->name = CopyStr("");
  
  return(c);
}

void DeleteCircles(CIRCLES c)
{
  RemoveRefValue(c);
  if (c->nRef > 0) return;
  
  if (c->n != 0) Free(c->array);
  if (c->name) Free(c->name);
  Free(c);
}



void ClearCircles(CIRCLES c)
{
  if (c->n != 0) {
    Free(c->array);
    c->array = NULL;
    c->n = 0;
  }
}

CIRCLES  CopyCircles(CIRCLES in, CIRCLES out)
{
  if (out == NULL) out = NewCircles();
  
  ClearCircles(out);
  
  out->array = Malloc(sizeof(Circles)*in->n);
  out->n = in->n;  

  memcpy(out->array,in->array,sizeof(Circles)*in->n);
  
  return(out);
}

char *ToStrCircles(CIRCLES c, char flagShort)
{
  static char str[30];

  if (!strcmp(c->name,"")) {
    sprintf(str,"<&circles[%d];%p>",c->n,c);
  }
  else if (strlen(c->name) < 15) {
    sprintf(str,"<&circles[%d];%s>",c->n,c->name);
  }
  else {
    sprintf(str,"<&circles[%d];...>",c->n);
  }
  
  return(str);

}

void PrintCircles(CIRCLES c)
{
  int i;

  if (c->n == 0) Printf("<empty>\n");
  else {  
    for (i=0;i<c->n;i++) 
      Printf("%d : x=%g, y=%g, r= %g\n",i,c->array[i].x,c->array[i].y,c->array[i].r);
  }
}


void PrintInfoCircles(CIRCLES c)
{
  Printf("  name  :  %s\n",c->name);
  Printf("  number of circles  :  %d\n",c->n);
}


/*
 * 'name' field
 */
static char *nameDoc = "{[= <name>]} {Sets/Gets the name of a circles}";


static void * GetNameCirclesV(CIRCLES c, void **arg)
{
  /* Documentation */
  if (c == NULL) return(nameDoc);
  
  return(GetStrField(c->name,arg));
}

static void * SetNameCirclesV(CIRCLES c, void **arg)
{
  
  
  /* doc */
  if (c == NULL) return(nameDoc);

  return(SetStrField(&(c->name),arg));
}

/*
 * 'n' field
 */
static char *nDoc = "{[= <n>]} {Sets/Gets the number of circles of a circles object}";


static void * GetNCirclesV(CIRCLES c, void **arg)
{
  /* Documentation */
  if (c == NULL) return(nDoc);
  
  return(GetIntField(c->n,arg));
}

static void * SetNCirclesV(CIRCLES c, void **arg)
{
  
  int n,i;
    
  /* doc */
  if (c == NULL) return(nDoc);

  n = c->n;

  if (SetIntField(&n,arg,FieldSPositive)==NULL) return(NULL);

  if (n<c->n) c->n = n;
  else {
    ClearCircles(c);
    c->array = Malloc(sizeof(Circles)*n);
    c->n = n;
    for (i= 0;i<n;i++) {
      c->array[i].x = 0;
      c->array[i].y = 0;
      c->array[i].r = 0;
    }
  }
    
  return(numType);
}


/* 
 *
 * Dealing with extractions of the type circles[...]
 *
 */
 
static char *doc = "{[*nolimit,...] [:]= list of <x,y,r>} {Get/Set the circle values}"; 

static char *optionDoc = "{{*nolimit} {*nolimit : indexes can be out of range}}";
static char *extractOptionsCircles[] = {"*nolimit",NULL};
enum {
  FSIOptCirclesNoLimit = FSIOption1
};


static void *GetExtractOptionsCirclesV(CIRCLES c, void **arg)
{ 
  
  
   /* doc */
  if (c == NULL) return(optionDoc);
  
  return(extractOptionsCircles);
}

static void *GetExtractInfoCirclesV(CIRCLES c, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  
  unsigned long *options = ARG_EI_GetPOptions(arg);

  if (flagInit == YES) {
    extractInfo.nSignals = 1;
    extractInfo.xmin = 0;
    extractInfo.dx = 1;
  }
  
  if (c->n == 0) {
    SetErrorf("No extraction possible on empty list of circles");
    return(NULL);
  }

  extractInfo.xmax = c->n-1;
  
  extractInfo.flags = EIIntIndex;
  if (!(*options & FSIOptCirclesNoLimit)) extractInfo.flags |= EIErrorBound;
  
  return(&extractInfo);
}


static void *GetExtractCirclesV(CIRCLES c, void **arg)
{
  FSIList *fsiList;
  CIRCLES c1;
  FSI_DECL;

  
  /* doc */
  if (c == NULL) return(doc);
  
  fsiList = (FSIList *) ARG_G_GetFsiList(arg);

  c1 = NewCircles();
  TempValue(c1);
  ARG_G_SetResValue(arg,c1);
  
  if (fsiList->nx1 == 0) return(circlesType);
  
  c1->array = Malloc(sizeof(Circle)*fsiList->nx1);
  c1->n = fsiList->nx1;
  
  FSI_FOR_START(fsiList); 
  if (fsiList->options & FSIOptCirclesNoLimit && (_i<0 || _i >= c->n)) continue;
  memcpy(&(c1->array[_k]),&(c->array[_i]),sizeof(Circle));
  FSI_FOR_END;

  return(circlesType);
}


static void *SetExtractCirclesV(CIRCLES c,void **arg)
{
  
  FSIList *fsiList = (FSIList *)  ARG_S_GetFsiList(arg);
  char *type = ARG_S_GetRightType(arg);    
  VALUE val = ARG_S_GetRightValue(arg);
  char *equal = ARG_S_GetEqual(arg);
  VALUE *pValueRes = ARG_S_GetResPValue(arg);

  LISTV lv;
  FSI_DECL;
  float f;
  SIGNAL sig;
  int _iold, i;

  /* doc */
  if (c == NULL) return(doc);
  
  if (type != listvType) {
    SetErrorf("Right hand side of assignation should be a &listv");
    return(NULL);
  }
  lv = CastValue(val,LISTV);
  
  /* Case := {} */
  if (*equal == ':') {
    if (lv->length != 0) {
      SetErrorf("With := syntax, right handside should be an empty listv");
      return(NULL);
    }
    
    /* Testing the indices are strictly increasing */
    _iold = -1;
    FSI_FOR_START(fsiList);
    if (_i <= _iold) {
      SetErrorf("Indices should be strictly increasing");
      return(NULL);
    }
    _iold = _i;
    FSI_FOR_END;
    
    /* Let's remove the circles ! */
    _iold = -1;
    i = 0;
    FSI_FOR_START(fsiList);
    if (_i == 0) {
      _iold = 0;
      continue;
    }
    else if (_iold == -1) {
      i = _i;
      _iold = _i;
      continue;
    }
    if (_i == _iold+1) {
      _iold = _i;
      continue;
    }
    memmove(&(c->array[i]),&(c->array[_iold+1]),(_i-_iold-1)*sizeof(Circle));
    i+=_i-_iold-1;
    _iold = _i;
    FSI_FOR_END;
    if (_i != c->n-1) {
      memmove(&(c->array[i]),&(c->array[_iold+1]),(c->n-1-_iold)*sizeof(Circle));
    }
    
    c->n -= fsiList->nx;
    
    *pValueRes = (VALUE) c;
    return(circlesType);    
  }
  
  
  /* case +=, -= ... */
  if (*equal != '=') {
    SetErrorf("%s syntaxnot valid",equal);
    return(NULL);
  }
  
  
  /* case = {sig1 ...} */
  if (lv->length != fsiList->nx) {
    SetErrorf("Right and left handside should have the same size");
    return(NULL);
  }
  
  FSI_FOR_START(fsiList);
  GetListvNth(lv,_k,&val,&f);
  if (val == NULL || GetTypeValue(val) != signaliType) {
    SetErrorf("Expect a listv of non empty signals on right handside");
    return(NULL);
  }
  sig = CastValue(val,SIGNAL);
  if (sig->size != 3) {
    SetErrorf("Expect a listv of signals of length 3 on right handside");
    return(NULL);
  }
  if (sig->Y[2] < 0) {
    SetErrorf("Radius must be positive");
    return(NULL);
  }
  c->array[_i].x = sig->Y[0]; 
  c->array[_i].y = sig->Y[1]; 
  c->array[_i].r = sig->Y[2]; 
  FSI_FOR_END;
  
  *pValueRes = (VALUE) c;
  return(circlesType);
}

/*
 * 'r' 'x' and 'y' fields
 */

static void *GetExtractOptionsRXYCirclesV(CIRCLES c, void **arg)
{ 
  static char *gextractOptionsCircles[] = {NULL};  
  return(gextractOptionsCircles);
}

static void *GetExtractInfoRXYCirclesV(CIRCLES c, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  
  unsigned long *options = ARG_EI_GetPOptions(arg);

  if (flagInit == YES) {
    extractInfo.nSignals = 1;
    extractInfo.xmin = 0;
    extractInfo.dx = 1;
    extractInfo.flags = EIIntIndex | EIErrorBound;
  }
  if (c->n == 0) {
    SetErrorf("No extraction possible on empty list of circles");
    return(NULL);
  }  

  extractInfo.xmax = c->n-1;

  return(&extractInfo);
}

static void * GetRXYCirclesV(CIRCLES c, void **arg, char flag)
{
  
  SIGNAL sig;
  int i;
  void *res;
    
  sig = TNewSignal();
  if (c->n != 0) SizeSignal(sig,c->n,YSIG);
  
  switch(flag) {
  case 'r' : for (i=0;i<c->n;i++) sig->Y[i] = c->array[i].r; break;
  case 'x' : for (i=0;i<c->n;i++) sig->Y[i] = c->array[i].x; break;
  case 'y' : for (i=0;i<c->n;i++) sig->Y[i] = c->array[i].y; break;
  }

  ARG_G_SetField(arg,NULL);
  res = GetSignalExtractField(sig,arg);
  if (res == NULL) return;
  if (res == signaliType) {
    sig = *((SIGNAL *) ARG_G_GetResPValue(arg));
    sig->dx = 1;
    sig->x0 = 0;
  }
}

static void * SetRXYCirclesV(CIRCLES c, void **arg, char flag)
{
  SIGNAL sig;
  int i;
  void *res;
  
  sig = TNewSignal();
  if (c->n != 0) SizeSignal(sig,c->n,YSIG);
  
  switch(flag) {
  case 'r' : for (i=0;i<c->n;i++) sig->Y[i] = c->array[i].r; break;
  case 'x' : for (i=0;i<c->n;i++) sig->Y[i] = c->array[i].x; break;
  case 'y' : for (i=0;i<c->n;i++) sig->Y[i] = c->array[i].y; break;
  }

  if ((res = SetSignalField(sig,arg)) == NULL) return(NULL);
  
  if (sig->size != c->n) {
    SetErrorf("Sorry, right handside should have the same size as left handside");
    return(NULL);
  }
 
  if (flag == 'r') {
    for (i=0;i<c->n;i++) {
      if (sig->Y[i]<0) { 
        SetErrorf("Sorry, radii should be positive");
        return(NULL);
      }
    }
  }

  switch(flag) {
  case 'r' : for (i=0;i<c->n;i++) c->array[i].r = sig->Y[i]; break;
  case 'x' : for (i=0;i<c->n;i++) c->array[i].x = sig->Y[i]; break;
  case 'y' : for (i=0;i<c->n;i++) c->array[i].y = sig->Y[i]; break;
  }
  
  return(res);
}

static char *rDoc = "{[[+-*/:]= (<signal> | <range>])} {Sets/Gets the radii}";

static void * GetRCirclesV(CIRCLES c, void **arg)
{
  if (c == NULL) return(rDoc);
  return(GetRXYCirclesV(c,arg,'r'));
}

static void * SetRCirclesV(CIRCLES c, void **arg)
{
  if (c == NULL) return(rDoc);
  return(SetRXYCirclesV(c,arg,'r'));
}


static char *xDoc = "{[[+-*/:]= (<signal> | <range>])} {Sets/Gets the abscissa}";

static void * GetXCirclesV(CIRCLES c, void **arg)
{
  if (c == NULL) return(xDoc);
  return(GetRXYCirclesV(c,arg,'x'));
}

static void * SetXCirclesV(CIRCLES c, void **arg)
{
  if (c == NULL) return(xDoc);
  return(SetRXYCirclesV(c,arg,'x'));
}

static char *yDoc = "{[[+-*/:]= (<signal> | <range>])} {Sets/Gets the ordinates}";

static void * GetYCirclesV(CIRCLES c, void **arg)
{
  if (c == NULL) return(yDoc);
  return(GetRXYCirclesV(c,arg,'y'));
}

static void * SetYCirclesV(CIRCLES c, void **arg)
{
  if (c == NULL) return(yDoc);
  return(SetRXYCirclesV(c,arg,'y'));
}



/************************************************************************
 *
 *
 *
 * GRAPHICS
 *
 *
 *
 ************************************************************************/


/* The GOBJECT structure for displaying a circles */
typedef struct graphCircles {

  GObjectFields; 
  
  /* The CIRCLES to be displayed */
  CIRCLES circles;
  
  /* the fill color */
  unsigned long fillColor;
      
} GraphCircles, *GRAPHCIRCLES;

/* The corresponding class */      
GCLASS theGraphCirclesClass = NULL;


/* Initialization of the GraphCircles structure */
static void _InitGraphCircles(GOBJECT o)
{
  GRAPHCIRCLES graph;

  graph = (GRAPHCIRCLES) o;

  graph->circles = NULL;
      
  graph->fillColor = graph->bgColor = invisibleColor;
    graph->rectType.left = graph->rectType.right = graph->rectType.bottom = graph->rectType.top = 1;

  
}


/* Deleting the content of a GraphCircles */
extern int tCIRCLES, tCIRCLES_;
static void _DeleteContentGraphCircles(GOBJECT o)
{
  GRAPHCIRCLES graph;

  graph = (GRAPHCIRCLES) o;

  if (graph->circles != NULL) DeleteCircles(graph->circles);
}

void _GetCirclesBound(CIRCLES c,float *x,float *y,float *w,float *h)
{
  int i;
  float f,x1,x2,y1,y2;
  
  if (c == NULL || c->n == 0) {
    *x = *y = *w = *h = 0;
    return;
  }
  
  x1 = FLT_MAX;
  x2 = FLT_MIN;
  y1 = FLT_MAX;
  y2 = FLT_MIN;
  for (i=0;i<c->n;i++) {
    f = c->array[i].x-c->array[i].r;
    x1 = MIN(x1,f);
    f = c->array[i].x+c->array[i].r;
    x2 = MAX(x2,f);

    f = c->array[i].y-c->array[i].r;
    y1 = MIN(y1,f);
    f = c->array[i].y+c->array[i].r;
    y2 = MAX(y2,f);
  }
  
  *x = x1;
  *y = y1;
  *w = x2-x1;
  *h = y2-y1;
}



/* The setg method */
static int _SetGraphCircles (GOBJECT o, char *field, char**argv)
{
  GRAPHCIRCLES graph;
  CIRCLES c;
  char *str;
  int i;
  LISTV lv;

   /* The help command */
  if (o == NULL) {
    SetResultStr("{{{graph [<circles>]} {Gets/Sets the circles object to be displayed by the GraphCircles. (The '-cgraph' field \
is equivalent to that field).}} \
{{fill [<color>]} {Sets/Gets the color that will be used to fill up the circles.}}}");
    return(YES);
  }
  
  graph = (GRAPHCIRCLES) o;
  c = graph->circles;
    
  /* the 'graph' and 'cgraph' fields */
  if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) {
    if (*argv == NULL) {
      SetResultValue(c);
      return(YES);
    }
    argv = ParseArgv(argv,tCIRCLES,&c,0);
    if (c->n == 0) Errorf("_SetGraphCircles() : You cannot display an empty 'circles'");
    if (graph->circles != NULL) DeleteCircles(graph->circles);
    graph->circles = c;
    AddRefValue(c);
    _GetCirclesBound(c,&(o->rx),&(o->ry),&(o->rw),&(o->rh));
    UpdateGlobalRectGObject(o);    
    return(YES);
  }

  /* The fill field */
  if (!strcmp(field,"fill")) {
    if (*argv == NULL) {
      SetResultStr(GetColorName(graph->fillColor));
      return(YES);  
    }    
    argv = ParseArgv(argv,tCOLOR,&(graph->fillColor),0);
    return(YES);
  }
  
  /* The 'rect' field */
  if (!strcmp(field,"rect")) {
    NoMoreArgs(argv);
    _GetCirclesBound(c,&(o->rx),&(o->ry),&(o->rw),&(o->rh));
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
static void _DrawGraphCircles (WINDOW win, GOBJECT obj, int x, int y,int w,int h)
{
  GRAPHCIRCLES graph;
  GOBJECT o1;
  CIRCLES c;
  float x0,y0,x1,y1;
  unsigned long fg,fill;
  int i;
  
  /* Some inits */
  graph = (GRAPHCIRCLES) obj;
  c = graph->circles;
  if (c == NULL) return;
  if (c->n == 0) return;

  /* Getting the minimum and maximum indexes the drawing should be performed on */
/*  Global2LocalRect(o,x,y,w,h,&x0,&y0,&x1,&y1,LargeRect); */

  fg = graph->fgColor;
  fill = graph->fillColor;
  o1 = (GOBJECT) obj->father;
  
  for (i=0;i<c->n;i++) {  
    if (fill != invisibleColor) {
      WSetColor(win,fill);
      WFillEllipse(obj,c->array[i].x-c->array[i].r,c->array[i].y-c->array[i].r,2*c->array[i].r,2*c->array[i].r,NO,LargeRect);  
    }
    WSetColor(win,fg);
    WDrawEllipse(obj,c->array[i].x-c->array[i].r,c->array[i].y-c->array[i].r,2*c->array[i].r,2*c->array[i].r,NO,LargeRect); 
  }
}

/* The isin procedure */
static float _IsInGraphCircles(GOBJECT o, GOBJECT *o1, int x, int y)
{
  float rx, ry;
  GRAPHCIRCLES g;
  int i;
  CIRCLES c;
  
  g = (GRAPHCIRCLES) o;
  c = g->circles;
  *o1 = NULL;
  
  /* Get the local coordinate */
  Global2Local(o,x,y,&rx,&ry);
  
  /* is this point in a circle ? */
  for (i=0;i<c->n;i++) {
    if ((c->array[i].x-rx)*(c->array[i].x-rx)+(c->array[i].y-ry)*(c->array[i].y-ry) < c->array[i].r*c->array[i].r) {
      *o1 = o;
      return(0);
    }
  }
  return(-1);
}


/* Defining the GraphCircles gclass */  
void DefineGraphCircles(void)
{
  theGraphCirclesClass = NewGClass("GraphCircles",theGObjectClass,"circles"); 
  theGraphCirclesClass->nbBytes = sizeof(GraphCircles);
  theGraphCirclesClass->init = _InitGraphCircles;
  theGraphCirclesClass->deleteContent = _DeleteContentGraphCircles;
  theGraphCirclesClass->set = _SetGraphCircles;
  theGraphCirclesClass->draw = _DrawGraphCircles;   
  theGraphCirclesClass->isIn = _IsInGraphCircles;   
  theGraphCirclesClass->varType = circlesType;
  theGraphCirclesClass->flags &= ~(GClassMoveResize+GClassLocalCoor);
  theGraphCirclesClass->info = "Graphic Class that allows to display circles";
  
}


/************************************************************************
 * 
 * 
 * Useful commands
 *
 * 
 ************************************************************************/

void C_Circles(char **argv)
{
  CIRCLES c;
  char *action;
  float x,y,dist,d;
  int i,i0;
  
  argv = ParseArgv(argv,tWORD,&action,tCIRCLES,&c,-1);
  
  
  if (!strcmp(action,"closest")) {
    argv = ParseArgv(argv,tFLOAT,&x, tFLOAT,&y,0);
    
    dist = FLT_MAX;
    i0 = -1;
    for (i=0;i<c->n;i++) {
      d = (c->array[i].x-x)*(c->array[i].x-x)+(c->array[i].y-y)*(c->array[i].y-y);
      if (d<dist) {
        i0 = i;
        dist = d;
      }
    }
    SetResultInt(i0);
    return;
  }
  
  Errorf("Unknown action '%s'",action);
}

static CProc circlesCommands[] = {
  
  "circles",C_Circles,"{{{closest <circles> <x> <y>} \
{Gets the index of the closes circle}}}",
  NULL,NULL,NULL
};

static CProcTable  circlesTable = {circlesCommands, "circles", "Commands related to circles"};

 
/************************************************************************
 * 
 * 
 * Loading/Adding the circles package
 *
 * 
 ************************************************************************/

int tCIRCLES, tCIRCLES_;

static void LoadCirclesPackage(void)
{
  tCIRCLES = AddVariableTypeValue(circlesType, &tsCircles, NULL);
  tCIRCLES_ = tCIRCLES+1;
  
  DefineGraphCircles();  
  
  AddCProcTable(&circlesTable);
}

void DeclareCirclesPackage(void)
{
  DeclarePackage("circles",LoadCirclesPackage,2003,"1.0","E.Bacry",
  "Demo package");
}


