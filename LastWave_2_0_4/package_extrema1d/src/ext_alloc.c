/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'extrema1d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1999-2002 Emmanuel Bacry.                             */
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
/*  ext_alloc.c    Functions for (des)allocation of:                        */
/*                     extrema (EXT)                                        */
/*		       extrema list (EXTLIS)                              */
/*                     extrema representation (EXTREP)                    */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "int_fsilist.h"
#include "extrema1d.h"



/********************************************************
 * 
 *     Dealing with Extrema
 *
 ********************************************************/

 
 

/*
 *           (Des)Allocation of extrema EXT         
 */

extern int flagOn;

/* Delete an extremum  (WARNING : it's better to use RemoveDeleteExt) */
void DeleteExt(EXT ext)
{  
  if (ext == NULL) return;
  
  RemoveRefValue(ext);
  if (ext->nRef > 0) return;

  if (flagOn) Printf("** Delete Ext %p\n",ext); 
    
#ifdef DEBUGALLOC
DebugType = "Ext1d";
#endif
  Free(ext);
}



/* allocate an extrema and returns it */
EXT NewExt(void)
{
  EXT ext;

#ifdef DEBUGALLOC
DebugType = "Ext1d";
#endif

  ext = (EXT) (Malloc(sizeof(struct ext)));
 
  InitValue(ext,&tsExt);
   
  ext->type = 1;

  ext->previous = ext->next = NULL;
  ext->coarser = ext->finer = NULL;
  
  ext->extlis = NULL;
  
  return(ext);
}



/* Remove an extremum from its extrep and delete it */
void RemoveDeleteExt(EXT ext)
{
  if (ext->extlis != NULL) {
    ext->extlis->size--;
    if (ext->extlis->first == ext) ext->extlis->first = ext->next;
  }

  if (ext->previous != NULL) {
    if (ext->next != NULL) {
      ext->previous->next = ext->next;
      ext->next->previous = ext->previous;
    }
    else ext->previous->next = NULL;
  }
  else if (ext->next != NULL) ext->next->previous = NULL;
  
  if (ext->coarser != NULL) ext->coarser->finer = NULL;
  if (ext->finer != NULL) ext->finer->coarser = NULL;
  
  ext->next = ext->previous = ext->finer = ext->coarser = NULL;
  ext->extlis =  NULL;
    
  DeleteExt(ext);
}

/* Remove an extrema chain from its extrep */
void RemoveDeleteChain(EXT ext)
{
  EXT ext1;
  
  while (ext->coarser != NULL) ext = ext->coarser;
  
  while (ext != NULL) {
    ext1 = ext->finer;
    RemoveDeleteExt(ext);
    ext = ext1;
  }
}

/* Copy an extremum */
EXT CopyExt(EXT in,EXT out)
{
    if (out == NULL) out = NewExt();
    
	out->abscissa = in->abscissa;
	out->scale = in->scale;
	out->ordinate = in->ordinate;
	out->next = in->next;
	out->previous = in->previous;
	out->coarser = in->coarser;
	out->finer = in->finer;
	out->type = in->type;	
	out->index = in->index;
	out->extlis = in->extlis;
	
	return(out);
}



/*
 * Command to perform operations on extrema
 */ 

void C_Ext(char **argv)
{
  EXT ext;
  char *action;
  
  argv = ParseArgv(argv,tWORD,&action,tEXT,&ext,0);
  
  /* remove action */
  if (!strcmp(action,"remove")) RemoveDeleteExt(ext);  
  
  /* else remove chain action */
  else if (!strcmp(action,"removechain")) RemoveDeleteChain(ext);

    
  else Errorf("Unknown action '%s'",action);
}




/*
 * 'extrep' field
 */
static char *extrepDoc = "{} {Gets the extrema representation (type &extrep), the extremum is in.}";

static void * GetExtrepExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(extrepDoc);
  
  ext = (EXT) val;
  if(ext->extlis == NULL)  return(GetValueField(nullValue,arg));
  return(GetValueField(ext->extlis->extrep,arg));
}

/*
 * 'first' field
 */
static char *firstDoc = "{} {Gets the first extremum at the same scale.}";

static void * GetFirstExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(firstDoc);
  
  ext = (EXT) val;
  while (ext->previous != NULL) ext = ext->previous;

  return(GetValueField(ext,arg));
}


/*
 * 'last' field
 */
static char *lastDoc = "{} {Gets the last extremum at the smae scale.}";

static void * GetLastExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(lastDoc);
  
  ext = (EXT) val;
  while (ext->next != NULL) ext = ext->next;

  return(GetValueField(ext,arg));
}


/*
 * 'coarsest' field
 */
static char *coarsestDoc = "{} {Gets the coarsest extremum (or null if none) on the same chain.}";

static void * GetCoarsestExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(coarsestDoc);
  
  ext = (EXT) val;
  while (ext->coarser != NULL) ext = ext->coarser;

  return(GetValueField(ext,arg));
}


/*
 * 'finest' field
 */
static char *finestDoc = "{} {Gets the finest extremum (or null if none) on the same chain.}";

static void * GetFinestExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(finestDoc);
  
  ext = (EXT) val;
  while (ext->finer != NULL) ext = ext->finer;

  return(GetValueField(ext,arg));
}


/*
 * 'coarser' field
 */
static char *coarserDoc = "{} {Gets the coarser extremum (or null if none) at the next larger scale on the same chain.}";

static void * GetCoarserExtV(VALUE val, void **arg)
{
  EXT ext;
  ext = (EXT) val;

  /* Documentation */
  if (val == NULL) return(coarserDoc);
  
  if (ext->coarser != NULL) return(GetValueField(ext->coarser,arg));
  return(GetValueField(nullValue,arg));
}


/*
 * 'finer' field
 */
static char *finerDoc = "{} {Gets the finer extremum (or null if none) at the next smaller scale on the same chain.}";

static void * GetFinerExtV(VALUE val, void **arg)
{
  EXT ext;
  ext = (EXT) val;

  /* Documentation */
  if (val == NULL) return(finerDoc);
  
  if (ext->finer != NULL) return(GetValueField(ext->finer,arg));
  return(GetValueField(nullValue,arg));
}



/*
 * 'previous' field
 */
static char *previousDoc = "{} {Gets the previous extremum (or null if none) at the same scale.}";

static void * GetPreviousExtV(VALUE val, void **arg)
{
  EXT ext;
  ext = (EXT) val;

  /* Documentation */
  if (val == NULL) return(previousDoc);
  
  if (ext->previous != NULL) return(GetValueField(ext->previous,arg));
  return(GetValueField(nullValue,arg));
}

/*
 * 'next' field
 */
static char *nextDoc = "{} {Gets the next extremum (or null if none) at the same scale.}";

static void * GetNextExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(nextDoc);

  ext = (EXT) val;
  
  if (ext->next != NULL) return(GetValueField(ext->next,arg));
  return(GetValueField(nullValue,arg));
}


/*
 * 'nbExt' field
 */
static char *nbDoc = "{} {Gets the number of extrema at the same scale as this extremum.}";

static void * GetNbExtExtV(VALUE val, void **arg)
{
  EXT ext;
  
  /* Documentation */
  if (val == NULL) return(nbDoc);
  
  ext = (EXT) val;
  
  if (ext->extlis == NULL) return(GetIntField(0,arg));
  return(GetIntField(ext->extlis->size,arg));
}


/*
 * 'z' field
 */
static char *zDoc = "{[= <wtValue>]} {Sets/Gets the wavelet transform value at the extremum.}";

static void * GetZExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(zDoc);
  
  return(GetFloatField(((EXT) val)->ordinate,arg));
}

static void * SetZExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(zDoc);
  
  return(SetFloatField(&(((EXT) val)->ordinate),arg,0));
}


/*
 * 'y' field
 */
static char *yDoc = "{} {Gets the scale of an extremum. It is an integer (0 corresponds to the smallest scale).}";

static void * GetYExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(yDoc);
  
  return(GetIntField(((EXT) val)->scale,arg));
}


/*
 * 'index' field
 */
static char *indexDoc = "{[= <index>]} {Sets/Gets the position of an extremum using an integer index.}";

static void * GetIndexExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(indexDoc);
  
  return(GetIntField(((EXT) val)->index,arg));
}

static void * SetIndexExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(indexDoc);
  
  return(SetIntField(&(((EXT) val)->index),arg,0));
}



/*
 * 'x' field
 */
static char *xDoc = "{[= <x>]} {Sets/Gets the abscissa of an extremum.}";

static void * GetXExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(xDoc);
  
  return(GetFloatField(((EXT) val)->abscissa,arg));
}

static void * SetXExtV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(xDoc);
  
  return(SetFloatField(&(((EXT) val)->abscissa),arg,0));
}


/*
 * Answers to the different print messages
 */
 
void PrintExt(EXT ext)
{
  Printf("<&ext;%p>\n",ext);
}

char *ToStrExt(EXT ext, char flagShort)
{
  static char str[30];
  
  sprintf(str,"<&ext;%p>",ext);
  
  return(str);
}


void PrintInfoExt(EXT ext)
{
  Printf("  x  :  %g [%d]\n",ext->abscissa,ext->index);
  Printf("  y  :  %d\n",ext->scale);
  Printf("  z  :  %g\n",ext->ordinate);
}



/*
 * The field list
 */
 
struct field fieldsExt[] = {

  "x", GetXExtV, SetXExtV, NULL, NULL,
  "index", GetIndexExtV, SetIndexExtV, NULL, NULL,
  "y", GetYExtV, NULL, NULL, NULL,
  "z", GetZExtV, SetZExtV, NULL, NULL,
  "nbExt", GetNbExtExtV, NULL, NULL, NULL,
  "next", GetNextExtV, NULL, NULL, NULL,
  "last", GetLastExtV, NULL, NULL, NULL,
  "first", GetFirstExtV, NULL, NULL, NULL,
  "previous", GetPreviousExtV, NULL, NULL, NULL,
  "coarser", GetCoarserExtV, NULL, NULL, NULL,
  "coarsest", GetCoarsestExtV, NULL, NULL, NULL,
  "finer", GetFinerExtV, NULL, NULL, NULL,
  "finest", GetFinestExtV, NULL, NULL, NULL,
  "extrep", GetExtrepExtV, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for EXT
 */
 
char *extType = "&ext"; 

TypeStruct tsExt = {

  "{{{&ext} {This type is used to store a local extremum of a wavelet transform. It is itself in a structure of type &extrep (i.e., \
an extrema representation)}}}",  /* Documentation */

  &extType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteExt,     /* The Delete function */
  NewExt,     /* The Delete function */
  
  CopyExt,       /* The copy function */
  NULL,       /* The clear function */
  
  ToStrExt,       /* String conversion */
  PrintExt,   /* The Print function : print the object when 'print' is called */
  PrintInfoExt,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsExt,      /* The list of fields */
};
 



/********************************************************
 * 
 *     Dealing with Extlis
 *
 ********************************************************/

/* delete an extrema and all the extrema it points to */
static void DeleteExtAndPoint (EXT ext)
{
  if (ext != NULL)
    {
      DeleteExtAndPoint(ext->next);
      ext->next = ext->previous = ext->finer = ext->coarser = NULL;
      ext->extlis = NULL;
      DeleteExt(ext);
    }
}


/* Delete an extrema list */
void DeleteExtlis(EXTLIS extlis)
{
  DeleteExtAndPoint(extlis->first);
#ifdef DEBUGALLOC
DebugType = "Extlis1d";
#endif
  Free(extlis);
}

/* Initialization of an  extrema list */
void ClearExtlis(EXTLIS extlis)
{
  extlis->size = 0;
  DeleteExtAndPoint(extlis->first);
  extlis->first = NULL;
  extlis->end = NULL;
}

/* Allocate an extrema list and return it */
EXTLIS NewExtlis(void)
{
  EXTLIS extlis;

#ifdef DEBUGALLOC
DebugType = "Extlis1d";
#endif
  extlis = (EXTLIS) (Malloc(sizeof(struct extlis)));

  extlis->size = 0;
  extlis->first = NULL;
  extlis->end = NULL;
  extlis->extrep = NULL;

  return(extlis);
}


/********************************************************
 * 
 *     Dealing with Extrep
 *
 ********************************************************/

/***********************************************************************************
 *
 *  Misc functions on extreps
 *
 ***********************************************************************************/

/* Check an extrep is not empty */
void CheckExtrep(EXTREP extrep)
{
  if (extrep == NULL || extrep->nOct == 0) 
    Errorf("Run 'extrema' first");
}


/*
 * Command to get/set the fields of a extrep
 */ 

void C_SetExtrep(char **argv)
{
  EXTREP extrep;
  EXT ext;
  int ival,o,v;
  char *field;
  float fval,x,y;
  char *name1,*name;

  argv = ParseArgv(argv,tSTR,&field,tEXTREP,&extrep,-1);
  
  if (!strcmp(field,"noct")) {
    if (*argv == NULL) SetResultInt(extrep->nOct);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0 || ival > NOCT-1) Errorf("Bad 'noct' value '%d'",ival);
      extrep->nOct = ival;
    }
  }

  else if (!strcmp(field,"nvoice")) {
    if (*argv == NULL) SetResultInt(extrep->nVoice);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0 || ival > NVOICE-1) Errorf("Bad 'nvoice' value '%d'",ival);
      extrep->nVoice = ival;
    }
  }

  else if (!strcmp(field,"size")) {
    if (*argv == NULL) SetResultInt(extrep->size);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0) Errorf("Bad 'size' value '%d'",ival);
      extrep->size = ival;
    }
  }

  else if (!strcmp(field,"dx")) {
    if (*argv == NULL) SetResultFloat(extrep->dx);
    else {
      argv = ParseArgv(argv,tFLOAT,&fval,0);
      if (fval <= 0) Errorf("Bad 'dx' value '%.8g'",fval);
      extrep->dx = fval;
    }
  }

  else if (!strcmp(field,"x0")) {
    if (*argv == NULL) SetResultFloat(extrep->x0);
    else {
      argv = ParseArgv(argv,tFLOAT,&fval,0);
      extrep->x0 = fval;
    }
  }
  
  else if (!strcmp(field,"name")) {
    NoMoreArgs(argv);
    SetResultStr(extrep->name);
  }

  else if (!strcmp(field,"wname")) {
    if (*argv == NULL) SetResultStr(extrep->wName);
    else {
      argv = ParseArgv(argv,tSTR,&name,0);
      if (extrep->wName != NULL) Free(extrep->wName);
      extrep->wName = CopyStr(name);
    }
  }

  else if (!strcmp(field,"*wtrans")) {
    ParseArgv(argv,tVNAME_,NULL,&name1,0);
    if (name1 == NULL) SetResultInt((int) (extrep->wtrans != NULL));
    else if (extrep->wtrans == NULL) {
      DeleteVariableIfExist(name1);
      SetResultInt(0);
    }
    else {
      SetVariable(name1,(VALUE) extrep->wtrans);
      SetResultInt(1);
    }
  }
  
  else if (!strcmp(field,"*extFirst")) {
    ParseArgv(argv,tINT,&o,tINT,&v,tVNAME,&name1,0);
    if (o >extrep->nOct || o <=0 ) Errorf("Bad octave number '%d'",o);
    if (v >=extrep->nVoice || v <0 ) Errorf("Bad voice number '%d'",v);
    if (extrep->D[o][v]->first==NULL) {
      DeleteVariableIfExist(name1);
      SetResultInt(0);
    }
    else {
      SetVariable(name1,(VALUE) extrep->D[o][v]->first);
      SetResultInt(1);
    }
  }

  else if (!strcmp(field,"*extClosest")) {
    ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,tVNAME,&name1,0);
    /* Get the oct and voice */
    v = ((int) (y+.5))%extrep->nVoice;
    o = ((int) (y+.5))/extrep->nVoice+1;
    if (o >extrep->nOct || o <=0 ) {
      DeleteVariableIfExist(name1);
      SetResultInt(0);
    }
    if (v >=extrep->nVoice || v <0 ) {
      DeleteVariableIfExist(name1);
      SetResultInt(0);
    }   
    if (extrep->D[o][v]->first==NULL) {
      DeleteVariableIfExist(name1);
      SetResultInt(0);
    }
    else {
      for (ext = extrep->D[o][v]->first; ext->next !=NULL;ext = ext->next) {
        if (ext->abscissa > x) break;
      }
      if (ext->abscissa > x && ext->previous) {
        if (fabs(ext->previous->abscissa-x) < fabs(ext->abscissa-x)) ext = ext->previous;
      }
      SetVariable(name1,(VALUE) ext);
      SetResultInt(1);
    }
  }

    
  else Errorf("Unknow extrep field '%s'",field);  
}


/*
 * Various actions on extreps
 */ 

void C_Extrep(char **argv)
{
  EXTREP extrep,extrep1;
  char *name;
  int nVoice1,nOct1,nVoice2,nOct2;
  char *action;
  float x,y;
  int o,v;
  EXT ext;

  argv = ParseArgv(argv,tWORD,&action,tEXTREP_,NULL,&extrep,-1);
  if (extrep == NULL) extrep = (GetWtransCur())->extrep;
  
  if (!strcmp(action,"closest")) {
    ParseArgv(argv,tFLOAT,&x,tFLOAT,&y,0);
    /* Get the oct and voice */
    v = ((int) (y+.5))%extrep->nVoice;
    o = ((int) (y+.5))/extrep->nVoice+1;
    if (o >extrep->nOct || o <=0 || v >=extrep->nVoice || v <0 || extrep->D[o][v]->first==NULL) ext = NULL;
    else {
      for (ext = extrep->D[o][v]->first; ext->next !=NULL;ext = ext->next) {
        if (ext->abscissa > x) break;
      }
      if (ext->abscissa > x && ext->previous) {
        if (fabs(ext->previous->abscissa-x) < fabs(ext->abscissa-x)) ext = ext->previous;
      }
    }
    
    if (ext == NULL) SetResultValue(nullValue);
    else SetResultValue(ext);
    return;
  }

  
  else Errorf("Unknow action '%s'",action);  
}


/*
 * Get the current extrep 
 * (generate an error if there is none)
 */
 
EXTREP GetExtrepCur(void)
{
  EXTREP extrep;
  WTRANS wtrans;
 
 if (!ParseTypedValLevel_(levelCur, "objCur", NULL, (VALUE *) &wtrans, wtransType)) Errorf1("");

  if (wtrans->extrep == NULL) Errorf("Current wavelet transform does not have any extrema representation");
  extrep = wtrans->extrep;
  AddRefValue(extrep);
  TempValue(extrep);
   
  return(extrep);
}


   
/*************************
 * Copy an extrep 
 ************************/

void CopyExtrep(EXTREP extrep1,EXTREP extrep2,int flagCut,float xMin,float xMax)
{
  EXTLIS extlis1,extlis2;
  EXT ext1,ext2,ext2Old;
  int o,v;
    
  if (extrep1 == extrep2) return;
    
  ClearExtrep(extrep2);
	
  extrep2->x0 = extrep1->x0;
  extrep2->dx = extrep1->dx;
  extrep2->size = extrep1->size ;
  extrep2->nVoice = extrep1->nVoice;
  extrep2->nOct = extrep1->nOct;
  extrep2->aMin = extrep1->aMin;
  if (extrep1->wName != NULL) extrep2->wName = CopyStr(extrep1->wName);
 
  for(o=1;o<=extrep1->nOct;o++)
    for(v=0;v<extrep1->nVoice;v++) {
      extlis1 = extrep1->D[o][v];
      extlis2 = extrep2->D[o][v];
      for(ext1 = extlis1->first;ext1 != NULL; ext1 = ext1->next)  {
           if (flagCut == YES && (ext1->abscissa < xMin || ext1->abscissa > xMax)) 
                continue;
           ext2Old = ext2;
           ext2 = NewExt();
           CopyExt(ext1,ext2);
           ext2->coarser = ext2->finer = NULL;
           ext2->extlis = extlis2;
           if (extlis2->size == 0) {
              extlis2->first = ext2;
              ext2->previous = NULL;
           }
           else {
              ext2->previous = ext2Old;
              ext2->previous->next = ext2;
           }
           ext2->next = NULL; 
           extlis2->size++;
      }
      if (extlis2->size != 0) extlis2->end = ext2;
   }
}

EXTREP CopyExtrep1(EXTREP extrep1,EXTREP extrep2)
{
  if (extrep2 == NULL) extrep2 = NewExtrep();
  CopyExtrep(extrep1,extrep2,NO,0,0);
  return(extrep2);
}

void C_ECopy(char **argv)
{
  EXTREP extrep1,extrep2;
  WTRANS wtransCur = GetWtransCur();
  	
  argv = ParseArgv(argv,tEXTREP,&extrep1,tEXTREP,&extrep2,0);
	
  CheckExtrep(extrep1);
	
  CopyExtrep(extrep1,extrep2,NO,0.,0.);
}

/* Default name for an extrep */
static char defaultName[] = "";

/* Desallocate an extrep */
void DeleteExtrep(EXTREP extrep)
{
  int i,j;

  RemoveRefValue(extrep);
  if (extrep->nRef > 0) return;
  
  DeleteSignal(extrep->coarse);
  DeleteFilterGroup(extrep->fg);

  for(i=0;i<NOCT;i++) 
    for(j=0;j<NVOICE;j++)
      DeleteExtlis(extrep->D[i][j]);
        
  if (extrep->name != NULL && extrep->name !=  defaultName) Free(extrep->name);
  if (extrep->wName != NULL) Free(extrep->wName);

  if (extrep->wtrans != NULL) extrep->wtrans->extrep = NULL;

#ifdef DEBUGALLOC
DebugType = "Extrep1d";
#endif
      
  Free(extrep);       
}


/* Initialization of an  extrema representation */
void ClearExtrep (EXTREP extrep)
{
  int i,j;

  extrep->size = 0;
  extrep->dx = 0.;
  extrep->x0 = 0.;
  extrep->nOct = 0;
  extrep->nVoice = 0;
  if (extrep->wName != NULL) Free(extrep->wName);
  extrep->wName = NULL;	

  ClearSignal(extrep->coarse);
  for(i=0;i<NOCT;i++)   
    for(j=0;j<NVOICE;j++)
      ClearExtlis(extrep->D[i][j]); 
}


/* Allocate an extrema representation */
EXTREP NewExtrep(void)
{
  extern TypeStruct tsExtrep;
  EXTREP extrep;
  int i,j;

#ifdef DEBUGALLOC
DebugType = "Extrep1d";
#endif
  extrep = (EXTREP) (Malloc(sizeof(struct extrep)));

  InitValue(extrep,&tsExtrep);
  
  
  for(i=0;i<NOCT;i++) 
    for(j=0;j<NVOICE;j++) {
      extrep->D[i][j] = NewExtlis();
      extrep->D[i][j]->extrep = extrep;
    }
  
  extrep->name = defaultName;  
  extrep->coarse = NewSignal();
  extrep->size = 0;
  extrep->dx = 0.;
  extrep->x0 = 0.;
  extrep->nOct = 0;
  extrep->nVoice = 0;
  extrep->fg = NULL;
  extrep->wtrans = NULL;
  extrep->wName = NULL;
   
  return(extrep);
}


/*
 * Answers to the different print messages
 */
 
void PrintExtrep(EXTREP extrep)
{
  if (extrep->name == NULL)
    Printf("<&extrep[%d,%d];%p>\n",extrep->nOct,extrep->nVoice,extrep);
  else
    Printf("<'%s';&extrep[%d,%d];%p>\n",extrep->name,extrep->nOct,extrep->nVoice,extrep);
}

char *ToStrExtrep(EXTREP extrep, char flagShort)
{
  static char str[30];
  
  if (extrep->name == defaultName) {
    sprintf(str,"<&extrep;%p>",extrep);
  }
  else {
    sprintf(str,"<&extrep;%s>",extrep->name);
  }
  return(str);
}


void PrintInfoExtrep(EXTREP extrep)
{
  Printf("  number of octave  :  %2d\n",extrep->nOct);
  Printf("  number of voice   :  %2d\n\n",extrep->nVoice);
}


/*
 * Extract an ext from a extrep : wtrans->D[i1,i2]
 */
static EXT ExtractExt(EXTREP extrep, char *field, FSIList *fsiList)
{
  int i1,i2;
      
        
        /* There must be [] */
        if (fsiList == NULL) {
          SetErrorf("Field D needs extraction : D[]");
          return(NULL);
        }
      
        /* 1 or 2 integers in between the [] */
        if (fsiList->nx == 0 || fsiList->nx > 2) {
          SetErrorf("D[] expects 1 or 2 indexes");
          return(NULL);
        }
        
        /* Get the integers */
        i1 = (int) FSI_FIRST(fsiList);
        if (fsiList->nx == 1) i2 = 0;
        else i2 = (int) FSI_SECOND(fsiList);
        
        /* Test wether they are in the right range */
        if (i1 < 0 || i1 >= extrep->nOct) {
          SetErrorf("Octave index '%d' out of range : should be in [0,%d]",i1,extrep->nOct-1);
          return(NULL);
        }
        if (i2 < 0 || i2 >= extrep->nVoice) {
          SetErrorf("Voice index '%d' out of range : should be in [0,%d]",i2,extrep->nVoice-1);
          return(NULL);
        }
        
        return(extrep->D[i1][i2]->first);

}


/*
 * Get  of field D
 */
static char *Ddoc = "{[o,v]} {Get the first extrema at octave 'o' and voice 'v'}"; 
 
static void *GetExtractExtrepV(VALUE val, void **arg)
{
  EXTREP extrep;
  char *field;
  FSIList *fsiList;
  EXT ext;

   /* doc */
  if (val == NULL) {
    if (!strcmp(ARG_G_GetField(arg),"D")) return(Ddoc);
  }
  
  extrep = (EXTREP) val;
  field = ARG_G_GetField(arg);
  fsiList = ARG_G_GetFsiList(arg);


  ext = ExtractExt(extrep,field,fsiList);

  if (ext == NULL) return(NULL);   
  
  ARG_G_SetResValue(arg,ext);      
  return(GetTypeValue(ext));
}

 
/*
 * Get the options for extraction (called for field D only) : There is none !!
 */

static void *GetExtractOptionsExtrepV(VALUE val, void **arg)
{
  static char *extractOptionsExtrep[] = {NULL};

  return(extractOptionsExtrep);
}

/*
 * Function to get the ExtractInfo for field D
 */

static void *GetExtractInfoExtrepV(VALUE val, void **arg)
{
  EXTREP extrep = (EXTREP) val;
  char *field = ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.nSignals = 1;
    extractInfo.xmax = MAX(NOCT-1,NVOICE-1);
    extractInfo.dx = 1;
    extractInfo.xmin = 0;
    extractInfo.flags = EIIntIndex | EIErrorBound;
    flagInit = NO;
  }

  return(&extractInfo);
}



/*
 * 'name' field
 */
static char *nameDoc = "{[= <name>]} {Sets/Gets the name of an extrep}";

static void * GetNameExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(nameDoc);
  
  return(GetStrField(((EXTREP) val)->name,arg));
}

static void * SetNameExtrepV(VALUE val, void **arg)
{
  EXTREP extrep = (EXTREP) val;
  
       /* doc */
  if (val == NULL) return(nameDoc);

  if (extrep->name==defaultName) {
    extrep->name=CharAlloc(1);
    extrep->name[0] = '\0';
  }
  return(SetStrField(&(extrep->name),arg));
}

/*
 * 'dx' field
 */
static char *dxDoc = "{[= <dx>]} {Sets/Gets the abscissa step of the original signal of the extrema representation.}";

static void * GetDxExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(dxDoc);
  
  return(GetFloatField(((EXTREP) val)->dx,arg));
}

static void * SetDxExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(dxDoc);

 return(SetFloatField(&(((EXTREP) val)->dx),arg,FieldSPositive));
}


/*
 * 'x0' field
 */

static char *x0Doc = "{[= <x0>]} {Sets/Gets the first abscissa of the original signal of the extrep.}";
static void * GetX0ExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(x0Doc);
  
  return(GetFloatField(((EXTREP) val)->x0,arg));
}

static void * SetX0ExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(x0Doc);
  
 return(SetFloatField(&(((EXTREP) val)->x0),arg,0));
}

/*
 * 'size' field
 */
static char *sizeDoc = "{[= <size>]} {Sets/Gets the size of the original signal of the extrema representation.}";

static void * GetSizeExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(sizeDoc);
  
  return(GetIntField(((EXTREP) val)->size,arg));
}

static void * SetSizeExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(sizeDoc);
  
  return(SetIntField(&(((EXTREP) val)->size),arg,FieldPositive));
}

/*
 * 'wavelet' field
 */
static char *waveletDoc = "{[= <name>]} {Gets/Sets the name of the analyzing wavelet used for the extrema representation.}";

static void * GetWaveletExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(waveletDoc);
  
  if (((EXTREP) val)->wName == NULL) return(GetStrField("",arg));
  
  return(GetStrField(((EXTREP) val)->wName,arg));
}

static void * SetWaveletExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(waveletDoc);
  
  if (((EXTREP) val)->wName == NULL) {
    ((EXTREP) val)->wName = CharAlloc(1);
    ((EXTREP) val)->wName[0] = '\0';
  }
 
  return(SetStrField(&(((EXTREP) val)->wName),arg));
}

/*
 * 'wtrans' field
 */
static char *wtransDoc = "{} {Gets the wavelet transform associated to the extrema representation.}";

static void * GetWtransExtrepV(VALUE val, void **arg)
{

  /* Documentation */
  if (val == NULL) return(wtransDoc);

  if (((EXTREP) val)->wtrans == NULL)
    return(GetValueField(nullValue,arg));
    
  return(GetValueField(((EXTREP) val)->wtrans,arg));
}



/*
 * 'nvoice' field
 */
 
static char *nvoiceDoc = "{} {Gets the number of voices per octave of the extrema representation.}"; 

static void * GetNVoiceExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(nvoiceDoc);
  
  return(GetIntField(((EXTREP) val)->nVoice,arg));
}



/*
 * 'noct' field
 */
static char *noctDoc = "{} {Gets the number of octave of the extrema representation.}";

static void * GetNOctExtrepV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(noctDoc);
  
  return(GetIntField(((EXTREP) val)->nOct,arg));
}


/*
 * The field list
 */
 
struct field fieldsExtrep[] = {

  "D", GetExtractExtrepV, NULL, GetExtractOptionsExtrepV, GetExtractInfoExtrepV, 
  "name", GetNameExtrepV, SetNameExtrepV, NULL, NULL,
  "dx", GetDxExtrepV, SetDxExtrepV, NULL, NULL,
  "x0", GetX0ExtrepV, SetX0ExtrepV, NULL, NULL,
  "nvoice", GetNVoiceExtrepV, NULL, NULL, NULL,
  "noct", GetNOctExtrepV, NULL, NULL, NULL,
  "size", GetSizeExtrepV, SetSizeExtrepV, NULL, NULL,
  "wavelet", GetWaveletExtrepV, SetWaveletExtrepV, NULL, NULL,
  "wtrans", GetWtransExtrepV, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for EXTREP
 */
 
char *extrepType = "&extrep";


TypeStruct tsExtrep = {

  "{{{&extrep} {This type is used to store the local extrema of a 1d wavelet transform.}}}",  /* Documentation */

  &extrepType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteExtrep,     /* The Delete function */
  NewExtrep,     /* The Delete function */
  
  CopyExtrep1,       /* The copy function */
  ClearExtrep,       /* The clear function */
  
  ToStrExtrep,       /* String conversion */
  PrintExtrep,   /* The Print function : print the object when 'print' is called */
  PrintInfoExtrep,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsExtrep,      /* The list of fields */
};
 

