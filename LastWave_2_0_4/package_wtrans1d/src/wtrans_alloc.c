/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
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
/*  wtrans_alloc.c   Functions which deal with the memory allocation        */
/*                   of WTRANS structure                                    */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "int_fsilist.h"
#include "wtrans1d.h"
#include "extrema1d.h"



char *wtransType = "&wtrans";
static char *defaultName="";

/*
 * Answers to the different print messages
 */
 
void PrintWtrans(WTRANS wtrans)
{  
  if (wtrans->name == NULL)
    Printf("<&wtrans[%d,%d];%p>\n",wtrans->nOct,wtrans->nVoice,wtrans);
  else
    Printf("<'%s';&wtrans[%d,%d];%p>\n",wtrans->name,wtrans->nOct,wtrans->nVoice,wtrans);
}

char *ToStrWtrans(WTRANS wtrans, char flagShort)
{
  static char str[30];
  
  if (wtrans->name == defaultName) {
    sprintf(str,"<&wtrans;%p>",wtrans);
  }
  else {
    sprintf(str,"<&wtrans;%s>",wtrans->name);
  }
  
  return(str);
}


void PrintInfoWtrans(WTRANS wtrans)
{
  Printf("  number of octave  :  %2d\n",wtrans->nOct);
  Printf("  number of voice   :  %2d\n\n",wtrans->nVoice);

  if (wtrans->fg != NULL) 
    Printf("  filter file    :  %s\n",wtrans->fg->filename);  
}



/*
 * NumExtraction
 *
 * (10a, 20, .30,...)
 */
static char *numdoc = "The syntax <ij> corresponds to A[i,j] and the syntax <.ij> corresponds to D[i,j]";
static void *NumExtractWtrans(WTRANS val,void **arg)
{
  int n;
  char flagDot;
  int v;
  int o;
  
     /* doc */
  if (val == NULL) return(numdoc);

  n = ARG_NE_GetN(arg);
  flagDot = ARG_NE_GetFlagDot(arg);
  v = n%10;
  o = n/10;

  if (o < 0 || o >= NOCT) {
    SetErrorf("Octave index '%d' out of range : should be in [0,%d]",o,NOCT-1);
    return(NULL);
  }
  if (v < 0 || v >= NVOICE) {
    SetErrorf("Voice index '%d' out of range : should be in [0,%d]",v,NVOICE-1);
    return(NULL);
  }
 
  if (flagDot) ARG_NE_SetResValue(arg,val->D[o][v]);
  else  ARG_NE_SetResValue(arg,val->A[o][v]);
  return(signalType);
}


/*
 * Extract a signal from a wtrans : wtrans->D[i1,i2] or wtrans->A[i1,i2]
 */
static SIGNAL ExtractSignal(WTRANS wtrans, char *field, FSIList *fsiList)
{
  int i1,i2;
      
        
        /* There must be [] */
        if (fsiList == NULL) {
          SetErrorf("Field A or D needs extraction : A[] or D[]");
          return(NULL);
        }
      
        /* 1 or 2 integers in between the [] */
        if (fsiList->nx == 0 || fsiList->nx > 2) {
          SetErrorf("A[] and D[] expects 1 or 2 indexes");
          return(NULL);
        }
        
        /* Get the integers */
        i1 = (int) FSI_FIRST(fsiList);
        if (fsiList->nx == 1) i2 = 0;
        else i2 = (int) FSI_SECOND(fsiList);
        
        /* Test wether they are in the right range */
        if (i1 < 0 || i1 >= NOCT) {
          SetErrorf("Octave index '%d' out of range : should be in [0,%d]",i1,NOCT-1);
          return(NULL);
        }
        if (i2 < 0 || i2 >= NVOICE) {
          SetErrorf("Voice index '%d' out of range : should be in [0,%d]",i2,NVOICE-1);
          return(NULL);
        }
        
        if (!strcmp(field,"A")) return(wtrans->A[i1][i2]);
        else return(wtrans->D[i1][i2]);

}

/*
 * Get  of fields A and D
 */
static char *Adoc = "{[o,v]} {Get the Approximation signal at octave 'o' and voice 'v'}"; 
static char *Ddoc = "{[o,v]} {Get the Detail signals at octave 'o' and voice 'v'}"; 
 
static void *GetExtractWtransV(VALUE val, void **arg)
{
  WTRANS wtrans;
  char *field = ARG_G_GetField(arg);
  FSIList *fsiList;
  SIGNAL sig;

   /* doc */
  if (val == NULL) {
    if (!strcmp(field,"A")) return(Adoc);
    if (!strcmp(field,"D")) return(Ddoc);
  }
  
  wtrans = (WTRANS) val;
  fsiList = ARG_G_GetFsiList(arg);

  sig = ExtractSignal(wtrans,field,fsiList);

  if (sig == NULL) return(NULL);   
        
  ARG_G_SetResValue(arg,sig);
  AddRefValue(sig);
  TempValue(sig);
  return(GetTypeValue(sig));
}

/*
 * Set  of fields A and D
 */

static  void *SetExtractWtransV(VALUE val, void **arg)
 {
  WTRANS wtrans;
  char *field = ARG_S_GetField(arg);
  FSIList *fsiList;
  SIGNAL sig;

   /* doc */
  if (val == NULL) {
    if (!strcmp(field,"A")) return(Adoc);
    if (!strcmp(field,"D")) return(Ddoc);
  }
  
  wtrans = (WTRANS) val;
  fsiList = ARG_S_GetFsiList(arg);

  sig = ExtractSignal(wtrans,field,fsiList);

  if (sig == NULL) return(NULL);   
        
  ARG_S_SetFsiList(arg,NULL);      
  return(SetSignalField(sig,arg));
 }

 
/*
 * Get the options for extraction (called for field A or D only) : There is none !!
 */

static void *GetExtractOptionsWtransV(VALUE val, void **arg)
{
  static char *extractOptionsWtrans[] = {NULL};

  return(extractOptionsWtrans);
}

/*
 * Function to get the ExtractInfo for fields A and D
 */

static void *GetExtractInfoWtransV(VALUE val, void **arg)
{
  WTRANS wtrans = (WTRANS) val;
  char *field =  ARG_EI_GetField(arg);
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
static char *nameDoc = "{[= <name>]} {Sets/Gets the name of a wtrans}";

static void * GetNameWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(nameDoc);
  
  return(GetStrField(((WTRANS) val)->name,arg));
}

static void * SetNameWtransV(VALUE val, void **arg)
{
  WTRANS wtrans = (WTRANS) val;
  
       /* doc */
  if (val == NULL) return(nameDoc);

  if (wtrans->name==defaultName) {
    wtrans->name=CharAlloc(1);
    wtrans->name[0] = '\0';
  }
  return(SetStrField(&(wtrans->name),arg));
}

/*
 * 'dx' field
 */
static char *dxDoc = "{[= <dx>]} {Sets/Gets the abscissa step of the original signal of the wavelet transform.}";

static void * GetDxWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(dxDoc);
  
  return(GetFloatField(((WTRANS) val)->dx,arg));
}

static void * SetDxWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(dxDoc);

 return(SetFloatField(&(((WTRANS) val)->dx),arg,FieldSPositive));
}


/*
 * 'x0' field
 */

static char *x0Doc = "{[= <x0>]} {Sets/Gets the first abscissa of the original signal of the wavelet transform.}";
static void * GetX0WtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(x0Doc);
  
  return(GetFloatField(((WTRANS) val)->x0,arg));
}

static void * SetX0WtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(x0Doc);
  
 return(SetFloatField(&(((WTRANS) val)->x0),arg,0));
}


/*
 * 'nvoice' field
 */

static char *nvoiceDoc = "{[= <nvoice>]} {Sets/Gets the number of voices per octave of a wavelet transform.}";


static void * GetNVoiceWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(nvoiceDoc);
  
  return(GetIntField(((WTRANS) val)->nVoice,arg));
}

int  SetNVoiceWtrans(WTRANS wtrans, int ival)
{
  if (ival < 0 || ival > NVOICE-1) {
    SetErrorf("Bad 'nvoice' value '%d'",ival);
    return(0);
  }
  wtrans->nVoice = ival;
  return(1);
}

static void * SetNVoiceWtransV(VALUE val, void **arg)
{
  int nvoice;
  WTRANS wtrans;
  
  /* Documentation */
  if (val == NULL) return(nvoiceDoc);
  
  wtrans = (WTRANS) val;
  nvoice = wtrans->nVoice;

  if (SetIntField(&nvoice,arg,FieldSPositive)==NULL) return(NULL);

  if (SetNVoiceWtrans(wtrans,nvoice) == 0) return(NULL);
  
  return(numType);
}


/*
 * 'noct' field
 */
static char *noctDoc = "{[= <noct>]} {Sets/Gets the number of octave of a wavelet transform.}";

static void * GetNOctWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(noctDoc);
  
  return(GetIntField(((WTRANS) val)->nOct,arg));
}

int  SetNOctWtrans(WTRANS wtrans, int ival)
{
  if (ival < 0 || ival > NOCT-1) {
    SetErrorf("Bad 'noct' value '%d'",ival);
    return(0);
  }
  wtrans->nOct = ival;
  return(1);
}

static void * SetNOctWtransV(VALUE val, void **arg)
{
  int ival;
  WTRANS wtrans;
  
  /* Documentation */
  if (val == NULL) return(noctDoc);
  
  wtrans = (WTRANS) val;
  ival = wtrans->nOct;

  if (SetIntField(&ival,arg,FieldSPositive)==NULL) return(NULL);

  if (SetNOctWtrans(wtrans,ival) == 0) return(NULL);
  
  return(numType);
}


/*
 * 'size' field
 */
static char *sizeDoc = "{[= <size>]} {Sets/Gets the size of the original signal of the wavelet transform.}";

static void * GetSizeWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(sizeDoc);
  
  return(GetIntField(((WTRANS) val)->size,arg));
}

static void * SetSizeWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(sizeDoc);
  
  return(SetIntField(&(((WTRANS) val)->size),arg,FieldPositive));
}


/*
 * 'wavelet' field
 */
static char *waveletDoc = "{[= <name>]} {Gets/Sets the name of the analyzing wavelet used for the wavelet transform.}";

static void * GetWaveletWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(waveletDoc);
  
  if (((WTRANS) val)->wName == NULL) return(GetStrField("",arg));
  
  return(GetStrField(((WTRANS) val)->wName,arg));
}

static void * SetWaveletWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(waveletDoc);
  
  if (((WTRANS) val)->wName == NULL) {
    ((WTRANS) val)->wName = CharAlloc(1);
    ((WTRANS) val)->wName[0] = '\0';
  }
 
  return(SetStrField(&(((WTRANS) val)->wName),arg));
}

/*
 * 'type' field
 */
static char *typeDoc = "{[= (1 | 3)]} {Gets/Sets the type of a wavelet transform (1 for orthogonal and 3 for continuous).}";

static void * GetTypeWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(typeDoc);
  
  return(GetIntField(((WTRANS) val)->type,arg));
}

static void * SetTypeWtransV(VALUE val, void **arg)
{
  int i;
  
  /* Documentation */
  if (val == NULL) return(typeDoc);
  
  i = ((WTRANS) val)->type;
  
  if (SetIntField(&i,arg,FieldPositive)==NULL) return(NULL);
  
  if (i != 1 || i != 3) {
    SetErrorf("Bad value %d or type field",i);
    return(NULL);
  }
  
  ((WTRANS) val)->type = i;
  
  return(numType);
}

/*
 * 'amin' field
 */
static char *aminDoc = "{[= <amin>]} {Sets/Gets the smallest scale of a wavelet transform.}";

static void * GetAminWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(aminDoc);
  
  return(GetFloatField(((WTRANS) val)->aMin,arg));
}

static void * SetAminWtransV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(aminDoc);
  
  return(SetFloatField(&(((WTRANS) val)->aMin),arg,FieldSPositive));
}

/*
 * 'extrep' field
 */
static char *extrepDoc = "{} {Gets the extrema representation associated to the wavelet transform.}";

static void * GetExtrepWtransV(VALUE val, void **arg)
{

  /* Documentation */
  if (val == NULL) return(extrepDoc);

  if (((WTRANS) val)->extrep == NULL)
    return(GetValueField(nullValue,arg));

  return(GetValueField(((WTRANS) val)->extrep,arg));
}


/***********************************************************************************
 *
 *  Allocation and Desallocation
 *
 ***********************************************************************************/

/* 
 * Allocation of a wtrans
 */

WTRANS NewWtrans(void)
{
  extern TypeStruct tsWtrans;
  WTRANS wtrans;
  int i,j;

#ifdef DEBUGALLOC
DebugType = "Wtrans1d";
#endif
   
  wtrans = (WTRANS) Malloc(sizeof(struct wtrans));
 InitValue(wtrans,&tsWtrans);
  
  for (i=0;i<NOCT;i++)
    for (j=0;j<NVOICE;j++) {
      wtrans->D[i][j] = NewSignal();
      wtrans->A[i][j] = NewSignal();
    };

  wtrans->fg = NULL;

  wtrans->border = B_PERIODIC;
  
  wtrans->name = defaultName;
  
  wtrans->wName = NULL;
  wtrans->nOct = 0;
  wtrans->nVoice = 0;
  wtrans->type = 0;
  
  wtrans->extrep = NewExtrep();
  wtrans->extrep->wtrans = wtrans;

  return(wtrans);
}

/*
 * Desallocation of a wtrans
 */

void DeleteWtrans(WTRANS wtrans)
{
  int i,j;

  RemoveRefValue( wtrans);
  if (wtrans->nRef > 0) return;
  
  for (i=0;i<NOCT;i++) {
    for (j=0;j<NVOICE;j++) {
      DeleteSignal(wtrans->D[i][j]);
      DeleteSignal(wtrans->A[i][j]);
    }
  }

   DeleteFilterGroup(wtrans->fg); 

   if (wtrans->extrep) {
     wtrans->extrep->wtrans = NULL;
     DeleteExtrep(wtrans->extrep);
   }
   
  if (wtrans->wName != NULL) Free(wtrans->wName);
  if (wtrans->name != NULL &&  wtrans->name != defaultName) Free(wtrans->name);


#ifdef DEBUGALLOC
DebugType = "Wtrans1d";
#endif
    
   Free(wtrans);   
}




/*
 * Make some desallocation of the wtrans
 */
 
void ClearWtrans(WTRANS wtrans)
{
  int i,j;

  if (wtrans->wName != NULL) Free(wtrans->wName);
  wtrans->wName = NULL;	
  for (i=0;i<NOCT;i++)
    for (j=0;j<NVOICE;j++) {
  	  ClearSignal(wtrans->D[i][j]);
  	  ClearSignal(wtrans->A[i][j]);
  	}
}	    

/***********************************************************************************
 *
 *  Misc functions on wtransforms
 *
 ***********************************************************************************/

/*
 * Copy the fields of a WTRANS into another 
 */

void CopyFieldsWtrans(WTRANS in , WTRANS out)
{
  /* Tests*/
  if (in == NULL) return;
  if (out == NULL) Errorf("CopyFieldsWtrans() : output wtrans is NULL");

  /* Copy the filter group */
  SetFGWtrans(out,in->fg);
  
  /* Copy the other fields */
  out->type = in->type;
  out->border = in->border;
  out->nOct =  in->nOct;
  out->nVoice =  in->nVoice;
  out->dx = in->dx;
  out->x0 = in->x0;
  out->aMin = in->aMin;
  out->exponent = in->exponent;
  out->size = in->size;

  if (out->wName != NULL) {
    Free(out->wName);
    out->wName = NULL;
  }
  
  if (in->wName != NULL) out->wName = CopyStr(in->wName);
}


/*
 * Copy a WTRANS into another 
 */

WTRANS CopyWtrans(WTRANS in,WTRANS out)
{
  int i,j;
  
  /* Tests*/
  if (in == NULL) return(NULL);
  if (out == NULL) out = NewWtrans();
  if (in == out) return(out);

  CopyFieldsWtrans(in,out);

  /* Copy all the signals */
  for (i=0;i<NOCT;i++)
    for (j=0;j<NVOICE;j++) {
      if (in->D[i][j] != NULL) {
        if (out->D[i][j] == NULL) out->D[i][j] = NewSignal();
        CopySig(in->D[i][j],out->D[i][j]);
      }
      else {
        DeleteSignal(out->D[i][j]);
        out->D[i][j] = NULL;
      }
      if (in->A[i][j] != NULL) {
        if (out->A[i][j] == NULL) out->A[i][j] = NewSignal();
        CopySig(in->A[i][j],out->A[i][j]);
      }
      else {
        DeleteSignal(out->A[i][j]);
        out->A[i][j] = NULL;
      }
    };
    
    return(out);
}



/*
 * Get the current wtransform 
 * (generate an error if there is none)
 */
 
WTRANS GetWtransCur(void)
{
  WTRANS wtrans;
 
 if (!ParseTypedValLevel_(levelCur, "objCur", NULL, (VALUE *) &wtrans, wtransType)) Errorf1("");

  AddRefValue( wtrans);
  TempValue( wtrans);
   
  return(wtrans);
}


/*
 * Check that there is a decomposition in the wtrans
 */
void CheckWtrans(WTRANS wtrans)
{
  if (wtrans->nOct == 0) Errorf("You must run a wavelet decomposition first");
}

/*
 * JUST FOR COMPATIBILITY 
 * Not to be used anymore.
 */ 

void C_SetWtrans(char **argv)
{
  WTRANS wtrans;
  int ival,o,v;
  float fval;
  char *field,*name1;

  argv = ParseArgv(argv,tSTR,&field,tWTRANS,&wtrans,-1);
  
  if (!strcmp(field,"noct")) {
    if (*argv == NULL) SetResultInt(wtrans->nOct);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0 || ival > NOCT-1) Errorf("Bad 'noct' value '%d'",ival);
      wtrans->nOct = ival;
    }
  }

  else if (!strcmp(field,"nvoice")) {
    if (*argv == NULL) SetResultInt(wtrans->nVoice);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0 || ival > NVOICE-1) Errorf("Bad 'nvoice' value '%d'",ival);
    wtrans->nVoice = ival;
    }
  }

  else if (!strcmp(field,"name")) {
    NoMoreArgs(argv);
    SetResultStr(wtrans->name);
  }

  else if (!strcmp(field,"type")) {
    if (*argv == NULL) SetResultInt(wtrans->type);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0) Errorf("Bad 'type' value '%d'",ival);
      wtrans->type = ival;
    }
  }

  else if (!strcmp(field,"size")) {
    if (*argv == NULL) SetResultInt(wtrans->size);
    else {
      argv = ParseArgv(argv,tINT,&ival,0);
      if (ival < 0) Errorf("Bad 'size' value '%d'",ival);
      wtrans->size = ival;
    }
  }

  else if (!strcmp(field,"amin")) {
    if (*argv == NULL) SetResultFloat(wtrans->aMin);
    else {
      argv = ParseArgv(argv,tFLOAT,&fval,0);
      if (fval <= 0) Errorf("Bad 'amin' value '%d'",fval);
      wtrans->aMin = fval;
    }
  }

  else if (!strcmp(field,"x0")) {
    if (*argv == NULL) SetResultFloat(wtrans->x0);
    else {
      argv = ParseArgv(argv,tFLOAT,&fval,0);
      wtrans->x0 = fval;
    }
  }
  
  else if (!strcmp(field,"dx")) {
    if (*argv == NULL) SetResultFloat(wtrans->dx);
    else {
      argv = ParseArgv(argv,tFLOAT,&fval,0);
      if (fval <= 0) Errorf("Bad 'dx' value '%d'",fval);
      wtrans->dx = fval;
    }
  }
  
  else if (!strcmp(field,"wname")) {
    if (*argv == NULL) SetResultStr(wtrans->wName);
    else {
      argv = ParseArgv(argv,tSTR,&name1,0);
      if (wtrans->wName != NULL) Free(wtrans->wName);
      wtrans->wName = CopyStr(name1);
    }
  }

  else if (!strcmp(field,"*extrep")) {
    ParseArgv(argv,tVNAME_,NULL,&name1,0);
    if (name1 == NULL) SetResultInt((int) (wtrans->extrep != NULL));
    else if (wtrans->extrep == NULL) {
      DeleteVariableIfExist(name1);
      SetResultInt(0);
    }
    else {
      SetVariable(name1,(VALUE) (wtrans->extrep));
      SetResultInt(1);
    }
  }
  
  else if (!strcmp(field,"*signalD")) {
    ParseArgv(argv,tVNAME,&name1,tINT,&o,tINT,&v,0);
    if (o >=NOCT || o <0 ) Errorf("Bad octave number '%d'",o);
    if (v >=NVOICE || v <0 ) Errorf("Bad voice number '%d'",v);
    SetVariable(name1,(VALUE) (wtrans->D[o][v]));
  }

  else if (!strcmp(field,"*signalA")) {
    ParseArgv(argv,tVNAME,&name1,tINT,&o,tINT,&v,0);
    if (o >=NOCT || o <0 ) Errorf("Bad octave number '%d'",o);
    if (v >=NVOICE || v <0 ) Errorf("Bad voice number '%d'",v);
    SetVariable(name1,(VALUE) (wtrans->A[o][v]));
  }
  
  else Errorf("Unknow wtrans field '%s'",field);  
}



/*
 * The field list
 */
struct field fieldsWtrans[] = {

  "A", GetExtractWtransV, SetExtractWtransV, GetExtractOptionsWtransV, GetExtractInfoWtransV,
  "D", GetExtractWtransV, SetExtractWtransV, GetExtractOptionsWtransV, GetExtractInfoWtransV,
  "name", GetNameWtransV, SetNameWtransV, NULL, NULL,
  "dx", GetDxWtransV, SetDxWtransV, NULL, NULL,
  "x0", GetX0WtransV, SetX0WtransV, NULL, NULL,
  "nvoice", GetNVoiceWtransV, SetNVoiceWtransV, NULL, NULL,
  "noct", GetNOctWtransV, SetNOctWtransV, NULL, NULL,
  "size", GetSizeWtransV, SetSizeWtransV, NULL, NULL,
  "type", GetTypeWtransV, SetTypeWtransV, NULL, NULL,
  "amin", GetAminWtransV, SetAminWtransV, NULL, NULL,
  "wavelet", GetWaveletWtransV, SetWaveletWtransV, NULL, NULL,
  "extrep", GetExtrepWtransV, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for WTRANS
 */

TypeStruct tsWtrans = {

  "{{{&wtrans} {This type is the basic type for 1d wavelet transforms (both continuous, dyadic or orthogonal). \
It is organized as two 2d arrays of signals referred to as A (the approximation signals) and D (the detail signals). \
Both of them are indexed first by the octave number and then by the voice number. The signal to be analyzed is in A[0,0].}}}",  /* Documentation */

  &wtransType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteWtrans,     /* The Delete function */
  NewWtrans,     /* The Delete function */
  
  CopyWtrans,       /* The copy function */
  ClearWtrans,       /* The clear function */
  
  ToStrWtrans,       /* String conversion */
  PrintWtrans,   /* The Print function : print the object when 'print' is called */
  PrintInfoWtrans,   /* The PrintInfo function : called by 'info' */

  NumExtractWtrans,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsWtrans,      /* The list of fields */
};
 























