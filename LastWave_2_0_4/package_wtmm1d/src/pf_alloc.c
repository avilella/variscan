/*..........................................................................*/
/*                                                                          */
/*  L a s t W a v e    P a c k a g e 'wtmm1d' 2.0                           */
/*                                                                          */
/*      Copyright (C) 1998-2002 Benjamin Audit.                             */
/*      email : audit@ebi.ac.uk                                            */
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

#include "wtmm1d.h"

char *partitionFunctionType = "&PF";


/* A simple fonction that returns the FILE * associated to a STREAM */
static FILE *stream2File(STREAM stream)
{
  /* If there is a FILE * defined just return it */
  if(stream->stream != NULL)
    return stream->stream;

  /* We compare the stream->id to see if it correspond to standard STREAM */
  /*  if(stream->id == _StdinStream->id)
      return stdin;
      if(stream->id == _StdoutStream->id)
      return stdout;
      if(stream->id == _StderrStream->id)
      return stderr;
      */
  
  /* Otherwise an error occurs */
  Errorf("stream2File() : Sorry no FILE* associated to that STREAM.");

  return NULL;
}

/*
 * Answers to the different print messages
 */
 
void PrintPartitionFunction(LWPARTFUNC lwpf)
{
  char method[PFMETHODSIZE+1];

  PFGetMethod(lwpf->pf,method);

  Printf("<'%s';&PF[%d,%d];%p>\n",
	 method,
	 PFGetOctaveNumber(lwpf->pf),
	 PFGetVoiceNumber(lwpf->pf),
	 lwpf
	 );
}

char *ToStrPartitionFunction(LWPARTFUNC lwpf, char flagShort)
{
  static char str[PFMETHODSIZE+1+6];
  char method[PFMETHODSIZE+1];
  
  PFGetMethod(lwpf->pf,method);

  
  sprintf(str,"<&PF;%s>",method);
  
  return(str);
}


void PrintInfoPartitionFunction(LWPARTFUNC lwpf)
{
  char method[PFMETHODSIZE+1];
  int number,i;

  PFGetMethod(lwpf->pf,method);
    
  Printf("  method           : %10s\n",method);
  Printf("  minimum scale    : %10.2f\n",PFGetAMin(lwpf->pf));
  Printf("  number of octave : %10d\n",PFGetOctaveNumber(lwpf->pf));
  Printf("  number of voice  : %10d\n",PFGetVoiceNumber(lwpf->pf));
  Printf("  list of q values : ");
  
  number = PFGetQNumber(lwpf->pf);
  if(number <= 0)
    Printf("\n");
  else
    {
      Printf("%10.2f", PFGetQFloat(lwpf->pf,0) );
      for(i=1;i<number;i++)
	Printf(", %.2f", PFGetQFloat(lwpf->pf,i) );
    }

  Printf("\n");
}

/****************************************************************************
 *
 *  Some useful internal functions to deal with partition functions
 *
 ****************************************************************************/
#define PFACCESSMODE_Q 'q'
#define PFACCESSMODE_INDEX 'i'

/* Return a temporary pointer to double with all the numerical values in the
   LISTV (supposed to contain only numerical values).
   On errors set the error string and returns NULL.
*/
static double *PFListv2QArray(LISTV lv)
{
  double *qArray;
  int qNumber = lv->length;
  int iq;

  if (qNumber < 1) {
      SetErrorf("PFListv2QArray : listv is empty");
      return(NULL);
  }
  /* TMalloc() : generates an error if allocation fails
     -> can't return NULL is that case
  */
  qArray = (double *) TMalloc(qNumber*sizeof(double));

  for(iq=0;iq<qNumber;iq++) {
    if (lv->values[iq] == NULL) {
      qArray[iq] = lv->f[iq];
      continue;
    }
    
    if (GetTypeValue(lv->values[iq]) != numType) {
      SetErrorf("PFListv2QArray() : Bad element of type '%s'",
		GetTypeValue(lv->values[iq]));
      return(NULL);
    }
    qArray[iq] = CastValue(lv->values[iq],NUMVALUE)->f;
  }

  return(qArray);
}

/* Return a temporary pointer to double with all the numerical values in the
   signal (supposed to contain only numerical values).
   On errors set the error string and returns NULL.
*/
static double *PFSignal2QArray(SIGNAL sig)
{
  double *qArray;
  int qNumber = sig->size;
  int iq;

  if (qNumber < 1) {
      SetErrorf("PFSignal2QArray : signal is empty");
      return(NULL);
  }
  /* TMalloc() : generates an error if allocation fails
     -> can't return NULL is that case
  */
  qArray = (double *) TMalloc(qNumber*sizeof(double));

  for(iq=0;iq<qNumber;iq++) {
    qArray[iq] = sig->Y[iq];
  }
    
  return(qArray);
}

/* Return a temporary pointer to int with all the numerical values in the
   LISTV (supposed to contain only numerical values) rounded to integer values.
   On errors set the error string and returns NULL.
*/
static int *PFListv2IndArray(LISTV lv)
{
  int *indArray;
  int indNumber = lv->length;
  int i;

  if (indNumber < 1) {
      SetErrorf("PFListv2IndArray : listv is empty");
      return(NULL);
  }
  /* TMalloc() : generates an error if allocation fails
     -> can't return NULL is that case
  */
  indArray = (int *) TMalloc(indNumber*sizeof(int));

  for(i=0;i<indNumber;i++) {
    if (lv->values[i] == NULL) {
      indArray[i] = (int) (lv->f[i]+0.5); /* values should be >=0 */
      continue;
    }
    
    if (GetTypeValue(lv->values[i]) != numType) {
      SetErrorf("PFListv2IndArray() : Bad element of type '%s'",
		GetTypeValue(lv->values[i]));
      return(NULL);
    }
    indArray[i] = (int) (CastValue(lv->values[i],NUMVALUE)->f+0.5 );
  }

  return(indArray);
}

/* Return a temporary pointer to signal with the corresponding partition 
   function (determined by type and indQ).
   On errors set the error string and returns NULL.
*/
static SIGNAL PFIndQ2Signal(LWPARTFUNC lwpf,char *type,int indQ,int mode)
{
  SIGNAL signal = TNewSignal();
  int size,res,i;
  float sln2;

  size = PFAccessSize(lwpf->pf);
  if(size == -1) {
    SetErrorf("PFIndQ2Signal() : pf is not valid");
    return(NULL);
  } else if (size == 0) {
    SetErrorf("PFIndQ2Signal() : pf's qList is empty");
    return(NULL);
  }

  SizeSignal(signal,size,YSIG);
  signal->x0 = (float) (log(PFGetAMin(lwpf->pf))/log(2.));
  signal->dx = 1./((float) PFGetVoiceNumber(lwpf->pf));
  signal->lastp = PFGetIndexMax(lwpf->pf);
      
  if (!strcmp(type,"t"))
    res = PFAccessTQFloat(lwpf->pf,indQ,mode,signal->Y);  
  else if (!strcmp(type,"h"))
    res = PFAccessHQFloat(lwpf->pf,indQ,mode,signal->Y);  
  else if (!strcmp(type,"d"))
    res = PFAccessDQFloat(lwpf->pf,indQ,mode,signal->Y);  
  else if (!strcmp(type,"st"))
    res = PFAccessVarTQFloat(lwpf->pf,indQ,signal->Y);  
  else if (!strcmp(type,"sh"))
    res = PFAccessVarHQFloat(lwpf->pf,indQ,signal->Y);  
  else if (!strcmp(type,"sd"))
    res = PFAccessVarDQFloat(lwpf->pf,indQ,signal->Y);  
  else {
    SetErrorf("PFIndQ2Signal() : type must be t, h, d, st, sh or sd");
    return(NULL);
  }
  
  /* covert variance to stddev if necessary */
  if (type[0] == 's') {
    for(i=0;i < size; i++)
      signal->Y[i] = sqrt(signal->Y[i]);
  }
  /* pf_lib uses the natural log so we divide by ln2 */
  sln2 = 1/log(2.0);
  for(i=0;i < size; i++)
    signal->Y[i] *= sln2;
      
  switch(res)
    {
    case PFYes:
      return(signal);
      break;
    case PFNotValid:
      SetErrorf("PFIndQ2Signal() : "
		"one of the arguments passed to PFAccess%sQFloat was not valid",
		type);
      return(NULL);
      break;
    default:
      SetErrorf("PFIndQ2Signal() : "
		"serious error. (maybe the pointer lwpf->pf was NULL)");
      return(NULL);
    }
  
  /* We should never reach this point */
  return(signal);
}

/* Return a temporary pointer to a signal with the corresponding partition 
   function (determined by type and q).
   On errors set the error string and returns NULL.
*/
static SIGNAL PFQ2Signal(LWPARTFUNC lwpf,char *type,double q,int mode)
{
  int indQ;

  /* Is this q in the qList */
  indQ = PFAccessIndQ(lwpf->pf, q);
  if(indQ == -1) {
    SetErrorf("PFQ2Signal() : %g is not in the qList",q);
    return(NULL);
  }

  return( PFIndQ2Signal(lwpf,type,indQ,mode) );
}

/* Return a temporary pointer to a LISTV with the corresponding partition 
   function (determined by type and numerical values in LISTV).
   On errors set the error string and returns NULL.
*/
static LISTV PFListvQ2ListvSig(LWPARTFUNC lwpf,LISTV lvq,char *type,int mode)
{
  int i;
  double *qArray;
  LISTV lvsig = TNewListv();
  SIGNAL signal;

  if( (qArray = PFListv2QArray(lvq)) == NULL ) {
    return(NULL);
  }

  SetLengthListv(lvsig, lvq->length);

  for(i=0;i<lvq->length;i++) {
    if( (signal = PFQ2Signal(lwpf,type,qArray[i],mode)) == NULL )
      return(NULL);
    SetListvNthValue(lvsig,i,(VALUE) signal);
  }

  return(lvsig);
}

/* Return a temporary pointer to a LISTV with the corresponding partition 
   function (determined by type and q index values in LISTV).
   On errors set the error string and returns NULL.
*/
static LISTV PFListvInd2ListvSig(LWPARTFUNC lwpf,LISTV lvind,char *type,int mode)
{
  int i;
  int *indArray;
  LISTV lvsig = TNewListv();
  SIGNAL signal;

  if( (indArray = PFListv2IndArray(lvind)) == NULL ) {
    return(NULL);
  }

  SetLengthListv(lvsig, lvind->length);

  for(i=0;i<lvind->length;i++) {
    if( (signal = PFIndQ2Signal(lwpf,type,indArray[i],mode)) == NULL )
      return(NULL);
    SetListvNthValue(lvsig,i,(VALUE) signal);
  }

  return(lvsig);
}

/* Return a temporary pointer to a LISTV with the corresponding partition 
   function (determined by type and numerical values in signal).
   On errors set the error string and returns NULL.
*/
static LISTV PFSigQ2ListvSig(LWPARTFUNC lwpf,SIGNAL sigq,char *type,int mode)
{
  int i;
  LISTV lvsig = TNewListv();
  SIGNAL signal;

  if (sigq->size < 1) {
      SetErrorf("PFSigQ2ListvSig : q value signal is empty.");
      return(NULL);
  }

  SetLengthListv(lvsig, sigq->size);

  for(i=0;i<sigq->size;i++) {
    if( (signal = PFQ2Signal(lwpf,type,sigq->Y[i],mode)) == NULL )
      return(NULL);
    SetListvNthValue(lvsig,i,(VALUE) signal);
  }

  return(lvsig);
}

/* Return a temporary pointer to a LISTV with the corresponding partition 
   function (determined by type and numerical values in signal).
   On errors set the error string and returns NULL.
*/
static LISTV PFSigInd2ListvSig(LWPARTFUNC lwpf,SIGNAL sigind,char *type,int mode)
{
  int i;
  LISTV lvsig = TNewListv();
  SIGNAL signal;

  if (sigind->size < 1) {
      SetErrorf("PFSigInd2ListvSig : q index value signal is empty.");
      return(NULL);
  }

  SetLengthListv(lvsig, sigind->size);

  for(i=0;i<sigind->size;i++) {
    if( (signal = PFIndQ2Signal(lwpf,type,(int) (sigind->Y[i]+0.5),mode)) == NULL )
      return(NULL);
    SetListvNthValue(lvsig,i,(VALUE) signal);
  }

  return(lvsig);
}

/* Return a temporary pointer to a LISTV with the list of q's.
   On errors set the error string and returns NULL.
*/
static LISTV PF2ListvQ(LWPARTFUNC lwpf)
{
  LISTV lvq = TNewListv();
  int iq,qNumber;

  qNumber = PFGetQNumber(lwpf->pf);
  if( qNumber <= 0 ) {
    SetResultf("PF2ListvQ() : Empty qlist !!");
    return(NULL);
  }

  SetLengthListv(lvq, qNumber);

  for(iq=0;iq<qNumber;iq++) {
    SetListvNthFloat(lvq,iq,PFGetQFloat(lwpf->pf,iq));
  }

  return(lvq);
}

/* Return a temporary pointer to a LISTV with the corresponding partition 
   function for all q's (determined by type).
   On errors set the error string and returns NULL.
*/
static LISTV PF2ListvSig(LWPARTFUNC lwpf,char *type,int mode)
{
  LISTV lvsig = TNewListv();
  SIGNAL signal;
  int iq,qNumber;

  qNumber = PFGetQNumber(lwpf->pf);
  if( qNumber <= 0 ) {
    SetResultf("PF2ListvSig() : Empty qlist !!");
    return(NULL);
  }

  SetLengthListv(lvsig, qNumber);

  for(iq=0;iq<qNumber;iq++) {
    if( (signal = PFIndQ2Signal(lwpf,type,iq,mode)) == NULL )
      return(NULL);
    SetListvNthValue(lvsig,iq,(VALUE) signal);
  }

  return(lvsig);
}

/*****************************************************************************
 *
 *  Allocation and Desallocation
 *
 *****************************************************************************/

/* 
 * Allocation of a partition function
 */

LWPARTFUNC NewPartitionFunction(void)
{
  extern TypeStruct tsPartitionFunction;
  LWPARTFUNC lwpf;

#ifdef DEBUGALLOC
  DebugType = "Pf";
#endif

  lwpf = (LWPARTFUNC) Malloc(sizeof(struct LastWavePartitionFunction));
  InitValue(lwpf,&tsPartitionFunction);

  lwpf->pf = PFNew();

  return(lwpf);
}

/*
 * Desallocation of a partition function
 */

void DeletePartitionFunction(LWPARTFUNC lwpf)
{
  RemoveRefValue(lwpf);
  if(lwpf->nRef > 0) return;

  PFDelete(lwpf->pf);

#ifdef DEBUGALLOC
  DebugType = "Pf";
#endif
  Free(lwpf);
}

/*
 * Make some desallocation on a partition function.
 *
 * Doesn't do anything yet: the corresponding 
 * modification as to be done in pf_lib.c
 */
 
void ClearPartitionFunction(LWPARTFUNC lwpf)
{
  
}	    

/*****************************************************************************
 *
 *  Misc functions on LWPARTFUNC
 *
 *****************************************************************************/

/*
 * Copy a LWPARTFUNC into another 
 */

LWPARTFUNC CopyPartitionFunction(LWPARTFUNC in,LWPARTFUNC out)
{
  /* Tests*/
  if (in == NULL) return(NULL);
  if (out == NULL) out = NewPartitionFunction();
  if (in == out) return(out);

  PFDelete(out->pf);
  out->pf = PFCopy(in->pf);

  if(out->pf == NULL) {
    SetErrorf("CopyPartitionFunction(): "
	      "You must have tried to copy a non initialized PF.");
    return(NULL);
  }
  
  return(out);
}


/*
 * 'method' field
 */
static char *methodDoc =
"{[= <method>]} "
"{Sets/Gets the method of a partition function i.e. the type of  wavelet "
"transform that was originally performed.\n"
"(This field is used to avoid adding two pfs computed using different "
"wavelets.)}";

/* Manu : Comment dois je gerer l'allocation memoire de methode pour les
   get/set suivant ?
*/
static void *GetMethodPFV(VALUE val, void **arg)
{
  static char method[PFMETHODSIZE+1];

  /* Documentation */
  if (val == NULL) 
    return(methodDoc);
  
    
  PFGetMethod( ((LWPARTFUNC) val)->pf, method );
  
  return( GetStrField(method,arg) );
}

static void * SetMethodPFV(VALUE val, void **arg)
{
  LWPARTFUNC lwpf = (LWPARTFUNC) val;
  char *method;
  void *setRes;
  int res;

  /* doc */
  if (val == NULL)
    return(methodDoc);

  method = CharAlloc(PFMETHODSIZE+1);
  PFGetMethod( ((LWPARTFUNC) val)->pf, method );
  if( (setRes = SetStrField(&(method),arg)) == NULL ) {
    Free(method);
    return(NULL);
  }
  
  res = PFSetMethod(lwpf->pf,method);
  Free(method);

  if(res != PFYes) {
    SetErrorf("Problem while writing (%s), maybe string is to long",method);
    return(NULL);
  }

  return(setRes);
}

/*
 * 'amin' field
 */

static char *aminDoc = 
"{[= <amin>]} "
"{Sets/Gets the smallest scale of the partition function.}";

static void * GetAminPFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(aminDoc);
  
  return(GetFloatField(
		  (float) PFGetAMin( ((LWPARTFUNC) val)->pf )
		  ,arg)
	 );
}

static void * SetAminPFV(VALUE val, void **arg)
{
  float amin;
  int res;
  void *setRes;

  /* Documentation */
  if (val == NULL)
    return(aminDoc);

  amin = (float) PFGetAMin( ((LWPARTFUNC) val)->pf );

  if( (setRes = SetFloatField(&amin,arg,FieldSPositive)) == NULL )
    return(NULL);

  res = PFSetAMin( ((LWPARTFUNC) val)->pf,amin);
  if(res != PFYes) {
    SetErrorf("weird: Problem while writing aMin, maybe it was <= 0");
    return(NULL);
  }

  return(setRes);
}

/*
 * 'noct' field
 */

static char *noctDoc =
"{} {Gets the number of octaves of a partition function.\n"
"(Your are not allowed to modify the octave number.)}";


static void * GetNOctPFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(noctDoc);
  
  return(GetIntField( 
		PFGetOctaveNumber( ((LWPARTFUNC) val)->pf )
		,arg
		)
	 );
}

/*
 * 'nvoice' field
 */

static char *nvoiceDoc =
"{} {Gets the number of voices per octave of a partition function.\n"
"(Your are not allowed to modify the voice number.)}";


static void * GetNVoicePFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(nvoiceDoc);
  
  return(GetIntField( 
		PFGetVoiceNumber( ((LWPARTFUNC) val)->pf )
		,arg
		)
	 );
}

/*
 * 'nscale' field
 */

static char *nscaleDoc =
"{} {Gets the number of computed scales of the partition function.\n"
"(Your are not allowed to modify this field.)}";


static void * GetNScalePFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(nscaleDoc);
  
  return(GetIntField( 
		PFGetIndexMax( ((LWPARTFUNC) val)->pf ) + 1
		,arg
		)
	 );
}

/*
 * 'sigsize' field
 */

static char *sigsizeDoc =
"{[= <signal size>]} "
"{Sets/Gets the size of the original signals"
" on which were computed the partition functions.\n"
"(In the case you used the non standard addition, it is the sum of these "
"fields. So, if you use the non standard addition on pfs that were the "
"result of standard addition this field is not correct).} ";

static void * GetSigSizePFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(sigsizeDoc);
  
  return(GetIntField( 
		PFGetSignalSize( ((LWPARTFUNC) val)->pf )
		,arg
		)
	 );
}

static void * SetSigSizePFV(VALUE val, void **arg)
{
  int sigsize;
  int res;
  void *setRes;

  /* Documentation */
  if (val == NULL)
    return(sigsizeDoc);

  sigsize = PFGetSignalSize( ((LWPARTFUNC) val)->pf );

  if( (setRes = SetIntField(&sigsize,arg,FieldSPositive)) == NULL )
    return(NULL);

  res = PFSetSignalSize( ((LWPARTFUNC) val)->pf,sigsize);
  if(res != PFYes) {
    SetErrorf("weird: Problem while writing sigzize, maybe it was <= 0");
    return(NULL);
  }

  return(setRes);
}



/*
 * 'signum' field
 */

static char *signumDoc =
"{} "
"{Gets the number of signals that were used to compute the partition "
"functions.\n"
"(Your are not allowed to modify this field.)}";

static void * GetSigNumPFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(signumDoc);
  
  return(GetIntField( 
		PFGetSignalNumber( ((LWPARTFUNC) val)->pf )
		,arg
		)
	 );
}

/*
 * 'qnumber' field
 */

static char *qnumberDoc =
"{} {Gets the number of q on which were computed the partition functions.\n"
"(Your are not allowed to modify this field.)}";

static void * GetQNumberPFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(qnumberDoc);
  
  return(GetIntField( 
		PFGetQNumber( ((LWPARTFUNC) val)->pf )
		,arg
		)
	 );
}

/*
 * 'qlist' field
 */

static char *qlistDoc =
"{[= <listv of q values>]} {Sets/Gets the qList for which the partition "
"functions have been computed.\n"
"WARNING: when you set a new qList it erases all the previous results.}";

static void * GetQListPFV(VALUE val, void **arg)
{
  LISTV lvq;
  
  /* Documentation */
  if (val == NULL)
    return(qlistDoc);
  
  if( ( lvq = PF2ListvQ((LWPARTFUNC) val) ) == NULL )
    return(NULL);

  return(GetValueField(lvq,arg));
}

static void * SetQListPFV(VALUE val, void **arg)
{
  LISTV lvq;
  int res;
  void *setRes;
  double *qArray;

  /* Documentation */
  if (val == NULL)
    return(qlistDoc);

  if( ( lvq = PF2ListvQ((LWPARTFUNC) val) ) == NULL ) {
    return(NULL);
  }

  if( (setRes = SetListvField(&lvq,arg)) == NULL ) {
    return(NULL);
  }

  if( (qArray = PFListv2QArray(lvq)) == NULL ) {
    return(NULL);
  }

  res = PFSetQList( ((LWPARTFUNC) val)->pf,
		    lvq->length,qArray);

  if(res != PFYes) {
    SetErrorf("Problem while writing new qlist, "
	      "it is very likely that either two q values where equal "
	      "that the PF has not been initialized.");
    return(NULL);
  }

  return(setRes);
}

/*
 * 'dim' field
 */

static char *dimDoc =
"{[= <dimension>]} "
"{Sets/Gets the dimension of the original data (1 for signal 2 for images) "
"on which were computed the partition functions.}";

static void * GetDimPFV(VALUE val, void **arg)
{
  /* Documentation */
  if (val == NULL)
    return(dimDoc);
  
  return(GetIntField( 
		PFGetDimension( ((LWPARTFUNC) val)->pf )
		,arg
		)
	 );
}

static void * SetDimPFV(VALUE val, void **arg)
{
  int dim;
  int res;
  void *setRes;

  /* Documentation */
  if (val == NULL)
    return(dimDoc);

  dim = PFGetDimension( ((LWPARTFUNC) val)->pf );

  if( (setRes = SetIntField(&dim,arg,FieldSPositive)) == NULL )
    return(NULL);

  res = PFSetDimension( ((LWPARTFUNC) val)->pf,dim);
  if(res != PFYes) {
    SetErrorf("Problem while writing dimension, maybe it was <= 0 "
	      "or lwpf->pf was NULL (that would be weird!).");
    return(NULL);
  }

  return(setRes);
}

/****************************************************************************
 *
 *  partition function and variables
 *
 ****************************************************************************/

/*
 * Various actions on partition_function
 */ 

void C_Pf(char **argv)
{
  char *action;

  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if (!strcmp(action,"init"))
    {
      LWPARTFUNC lwpf;
      LISTV lv;
      char *method;
      double *qArray;
      float aMin;
      int octaveNumber,voiceNumber,size,res;
      
      argv = ParseArgv(argv,tPF,&lwpf,tSTR,&method,
		       tFLOAT,&aMin,tINT,&octaveNumber,tINT,&voiceNumber,
		       tINT,&size,tLISTV,&lv,0);
      
      if( (qArray = PFListv2QArray(lv)) == NULL )
	Errorf1("");
      
      res = PFInit(lwpf->pf,method,
		   aMin,octaveNumber,voiceNumber,
		   size,1,lv->length,qArray);
      
      switch(res)
	{
	case PFYes:
	  SetResultInt(res);
	  break;
	case PFNotValid:
	  Errorf("One of the parameter isn\'t valid");
	  break;
	default:
	  Errorf("serious error. (maybe the pointer lwpf->pf was NULL)");
	  }
    }
  /* 'do1Scale' action */
  else if (!strcmp(action,"do1Scale"))
    {
      LWPARTFUNC lwpf;
      SIGNAL signal;
      float *Y;
      int octaveNumber,voiceNumber,scale,res,size;
      char opt;

      argv = ParseArgv(argv,tPF,&lwpf,tSIGNALI,&signal,
		       tINT_,-999,&octaveNumber,-1);

      if(octaveNumber != -999)
	{
	  argv = ParseArgv(argv,tINT,&voiceNumber,-1);

	  if( !INRANGE(1,octaveNumber,PFGetOctaveNumber(lwpf->pf)) )
	    Errorf("Bad range for oct");

	  if(!INRANGE(0,voiceNumber,PFGetVoiceNumber(lwpf->pf)-1) )
	    Errorf("Bad range for voice");
	  
	  scale = (octaveNumber -1)*PFGetVoiceNumber(lwpf->pf) + voiceNumber;
	}
      else
	scale = PFGetIndexMax(lwpf->pf) + 1;
      
      Y = signal->Y;
      size = signal->size;
      while( (opt = ParseOption(&argv)) ) 
	{ 
	  switch(opt) 
	    {
	    case 'c':
	      Y = signal->Y + signal->firstp;
	      size = signal->lastp - signal->firstp + 1;
	      break;
	    default:
	      ErrorOption(opt);
	    }
	}
      NoMoreArgs(argv);

	res = PFComputeOneScaleF(lwpf->pf,scale,Y,size);
	switch(res)
	  {
	  case PFYes:
	    SetResultInt(res);
	    break;
	  case PFNotValid:
	    Errorf("One of the parameter isn\'t valid");
	    break;
	  default:
	    Errorf("serious error. (maybe the lwpf->pf pointer was NULL)");
	  }
    }
  /* 'write' action */
  else if (!strcmp(action,"write"))
    {
      LWPARTFUNC lwpf;
      STREAM stream;
      FILE *fp;
      int flagToBeClosed = NO;
      int res;
      char *fileName;

      argv = ParseArgv(argv,tPF,&lwpf,tSTREAM_,NULL,&stream,-1);
      
      if(stream != NULL)
	{
	  fp = stream2File(stream);
	  NoMoreArgs(argv);
	}
      /* if the last argument is not a stream it may be a filename */
      else 
	{
	  ParseArgv(argv,tSTR_,NULL,&fileName,0);
	  if(fileName != NULL)
	    {
	      fp = FOpen(fileName,"w");
	      flagToBeClosed = YES;
	      if (fp == NULL) 
		Errorf("Can't open file: %s",fileName);
	    }
	  /* if there is no last argument we use stdoutStream */
	  else fp = stream2File(stdoutStream);
	}
      
      res = PFWriteAscii(fp,lwpf->pf);
      if(flagToBeClosed == YES)
	FClose(fp);
      switch(res)
	{
	case PFYes:
	  SetResultInt(res);
	  break;
	case PFWriErr:
	  Errorf("probleme while writing");
	  break;
	default:
	  Errorf("serious writing error. (maybe the file descriptor was NULL)");
	}
    }
  /* 'writebin' action */
  else if (!strcmp(action,"writebin"))
    {
      LWPARTFUNC lwpf;
      FILE *fp;
      char *fileName;
      int res;

      argv = ParseArgv(argv,tPF,&lwpf,tSTR,&fileName,0);
      
      fp = FOpen(fileName,"w");
      if (fp == NULL) 
	Errorf("Can't open file: %s",fileName);
      
      res = PFWriteBin(fp,lwpf->pf);
      FClose(fp);
      
      switch(res)
	{
	case PFYes:
	  SetResultInt(res);
	  break;
	case PFWriErr:
	  Errorf("probleme while writing");
	  break;
	default:
	  Errorf("serious writing error. (maybe the file descriptor was NULL)");
	}
    }

  /* 'read' action */
  else if (!strcmp(action,"read"))
    {
      LWPARTFUNC lwpf;
      STREAM stream;
      FILE *fp;
      char *fileName;
      int flagToBeClosed = NO;
      int res;
      argv = ParseArgv(argv,tPF_,NULL,&lwpf,tSTREAM_,NULL,&stream,-1);
      
      if(stream != NULL)
	{
	  fp = stream2File(stream);
	  NoMoreArgs(argv);
	}
      /* if the last argument is not a stream it may be a filename */
      else 
	{
	  argv = ParseArgv(argv,tSTR_,NULL,&fileName,0);
	  if(fileName != NULL)
	    {
	      fp = FOpen(fileName,"r");
	      flagToBeClosed = YES;
	      if (fp == NULL) 
		Errorf("Can't open file: %s",fileName);
	    }
	  /* if there is no last argument we use stdinStream */
	  else fp = stream2File(stdinStream);
	}

      if(lwpf == NULL) {
	lwpf = NewPartitionFunction();
	TempValue(lwpf);
      }
	
      res = PFRead(fp,lwpf->pf);
      if(flagToBeClosed == YES)
	FClose(fp);

      switch(res)
	{
	case PFYes:
	  SetResultValue(lwpf);
	  break;
	case PFErrFormat:
	  Errorf("the input has a bad format");
	  break;
	case PFNotValid:
	  Errorf("one of the value read is not meaningful");
	  break;
	default:
	  Errorf("serious reading error. (maybe the file descriptor was NULL)");
	}
    }  

  /* 'copy' action */
  else if (!strcmp(action,"copy"))
    {
      LWPARTFUNC lwpf,lwpf1;
      PartitionFunction pfTemp;

      argv = ParseArgv(argv,tPF,&lwpf,tPF,&lwpf1,0);
      
      
      if( PFIsValid(lwpf->pf) != PFYes )
	Errorf("The input pf is not valid.");

      pfTemp = PFCopy(lwpf->pf);
      if(pfTemp == NULL)
	Errorf("An error occured while copying.");
      PFDelete(lwpf1->pf);
      lwpf1->pf = pfTemp;
    }

  /* 'wtmm' action */
  else if (!strcmp(action,"wtmm")) 
    {
      LWPARTFUNC lwpf;
      EXTREP extrep;
      LISTV lv;
      SIGNAL sig;
      double *qArray;
      int qNumber;

      argv = ParseArgv(argv,tPF_,NULL,&lwpf,tEXTREP,&extrep,
		       tLISTV_,NULL,&lv,-1);
      if( lv == NULL) {
	argv = ParseArgv(argv,tSIGNALI,&sig,0);
	if( (qArray = PFSignal2QArray(sig)) == NULL )
	  Errorf1("");
	qNumber = sig->size;
      } else { 
	NoMoreArgs(argv);
	if( (qArray = PFListv2QArray(lv)) == NULL )
	  Errorf1("");
	qNumber = lv->length;
      }
      
      if(lwpf == NULL) {
	lwpf = NewPartitionFunction();
	TempValue(lwpf);
      }
      
      ComputePartFuncOnExtrep(lwpf,extrep,qNumber,qArray);
      
      SetResultValue(lwpf);
    }
  /* 'cont' action */
  else if (!strcmp(action,"cont")) 
    {
      LWPARTFUNC lwpf;
      WTRANS wtrans;
      LISTV lv;
      SIGNAL sig;
      double *qArray;
      int qNumber;
      int flagCausal;
      char opt;

      argv = ParseArgv(argv,tPF_,NULL,&lwpf,tWTRANS,&wtrans,
		       tLISTV_,NULL,&lv,-1);
      if( lv == NULL) {
	argv = ParseArgv(argv,tSIGNALI,&sig,-1);
	if( (qArray = PFSignal2QArray(sig)) == NULL )
	  Errorf1("");
	qNumber = sig->size;
      } else { 
	if( (qArray = PFListv2QArray(lv)) == NULL )
	  Errorf1("");
	qNumber = lv->length;
      }
  

      flagCausal = NO;
      while( (opt = ParseOption(&argv)) ) { 
	switch(opt) {
	case 'c':
	  flagCausal = YES;
	  break;
	default:
	  ErrorOption(opt);
	}
      }
      
      if(lwpf == NULL) {
	lwpf = NewPartitionFunction();
	TempValue(lwpf);
      }

      ComputePartFuncOnWtrans(lwpf,wtrans,qNumber,qArray,flagCausal);

      SetResultValue(lwpf);
    }
  /* 'get', 'getq', 'geti' action */
  else if ( !strcmp(action,"get")
	    ||  !strcmp(action,"getq")
	    ||  !strcmp(action,"geti")
	    ) 
    {
      LWPARTFUNC lwpf;
      LISTV lv,lvsig;
      SIGNAL signal,sig;
      float q;
      char *type;
      char opt;
      int mode;
      char access_mode = 'q';
      
      if( !strcmp(action,"geti") ) 
	access_mode = 'i';


      argv = ParseArgv(argv,tWORD,&type,tPF,&lwpf,
		       tLISTV_,NULL,&lv,-1);
      if( lv == NULL)
	argv = ParseArgv(argv,tFLOAT_,FLT_MAX,&q,-1);
      
      if( q == FLT_MAX ) {
	argv = ParseArgv(argv,tSIGNALI_,NULL,&sig,-1);
      }

      mode = PFEXTENSIVE;
      while( (opt = ParseOption(&argv)) ) 
	{ 
	  switch(opt) 
	    {
	    case 'i':
	      mode = PFINTENSIVE;
	      break;
	    default:
	      ErrorOption(opt);
	    }
	}
      NoMoreArgs(argv);

      if(lv != NULL) {
	if( access_mode == 'q' ) {
	  if( (lvsig = PFListvQ2ListvSig(lwpf,lv,type,mode)) == NULL)
	    Errorf1("");
	} else {
	  if( (lvsig = PFListvInd2ListvSig(lwpf,lv,type,mode)) == NULL)
	    Errorf1("");
	}
	SetResultValue(lvsig);
      } else if (q != FLT_MAX) {
	if( access_mode == 'q' ) {
	  if( (signal = PFQ2Signal(lwpf,type,(double) q,mode)) == NULL)
	    Errorf1("");
	} else {
	  if( (signal = PFIndQ2Signal(lwpf,type,(int) (q+0.5),mode)) == NULL)
	    Errorf1("");
	}
	SetResultValue(signal);
      } else if (sig != NULL) {
	if( access_mode == 'q' ) {
	  if( (lvsig = PFSigQ2ListvSig(lwpf,sig,type,mode)) == NULL)
	    Errorf1("");
	} else {
	  if( (lvsig = PFSigInd2ListvSig(lwpf,sig,type,mode)) == NULL)
	    Errorf1("");
	}
	SetResultValue(lvsig);
      } else {
	if( (lvsig = PF2ListvSig(lwpf,type,mode)) == NULL)
	  Errorf1("");
	SetResultValue(lvsig);
      }

    }
  /* 'add' action */
  else if (!strcmp(action,"add"))
    {
      LWPARTFUNC lwpf,lwpf1,lwpf2;
      char opt;
      int flagNonstandard,res;

      argv = ParseArgv(argv,tPF,&lwpf1,tPF,&lwpf2,-1);

      flagNonstandard = NO;
      while( (opt = ParseOption(&argv)) ) 
	{ 
	  switch(opt) 
	    {
	    case 'n':
	      flagNonstandard = YES;
	      break;
	    default:
	      ErrorOption(opt);
	    }
	}
      NoMoreArgs(argv);

      if( PFGetOctaveNumber(lwpf1->pf) < PFGetOctaveNumber(lwpf2->pf) ) {
	lwpf = lwpf1;
	lwpf1 = lwpf2;
	lwpf2 = lwpf;
      }

      if( (lwpf = CopyPartitionFunction(lwpf1,NULL)) == NULL)
	Errorf1("");
      TempValue(lwpf);

      if(flagNonstandard == NO) {
	res = PFStandardAddition(lwpf->pf,lwpf2->pf);
      } else {
	res = PFNonStandardAddition(lwpf->pf,lwpf2->pf);
      }

      switch(res)
	{
	case PFYes:
	  SetResultValue(lwpf);
	  break;
	case PFNotValid:
	  Errorf("one of the pf's is not valid");
	  break;
	case PFNotCompatible:
	  Errorf("The pf's are not compatible for addition");
	  break;
	default:
	  Errorf("serious error. (shouldn't occur)");
	}
    }

  /* 'reset' action */
  else if (!strcmp(action,"reset"))
    {
      LWPARTFUNC lwpf;
      int res;

      argv = ParseArgv(argv,tPF,&lwpf,0);
      res = PFReset(lwpf->pf);

      switch(res)
	{
	case PFYes:
	  SetResultValue(lwpf);
	  break;
	case PFNotValid:
	  Errorf("pf is not valid");
	  break;
	case PFNo:
	  Errorf("serious error: pf was NULL !!");
	  break;
	default:
	  Errorf("serious error. (shouldn't occur)");
	}
    }

  else Errorf("Unknow action '%s'",action);  
  
}


/* Command to set/get all the fields */
void C_SetPf(char **argv)
{
  LWPARTFUNC lwpf;
  int number,nq;
  int res;
  float aMin;
  float q;
  float *qArray;
  double *qArrayD;
  char *fieldName;
  char *strValue;
  char **list;
  char **listTemp;
  char method[PFMETHODSIZE+1];

  argv = ParseArgv(argv,tSTR,&fieldName,tPF,&lwpf,-1);

  /* Case of the 'method' field */
  if (!strcmp(fieldName,"method")) 
    {
      argv = ParseArgv(argv,tSTR_,NULL,&strValue,0);
      /* Get */
      if (strValue == NULL) 
	{
	  PFGetMethod(lwpf->pf,method);
	  SetResultf(method);
	  return;
	}
      /* Set */
      res = PFSetMethod(lwpf->pf,strValue);
      if(res != PFYes)
	Errorf("Problem while writing (%s), maybe string is to long",strValue);
      return;  
    }
  
  /* Case of the 'amin' field */
  else if (!strcmp(fieldName,"amin"))
    {
      argv = ParseArgv(argv,tFLOAT_,-999.0,&aMin,0);
      /* Get */
      if( aMin == -999.0 ) 
	{
	  aMin = (float) PFGetAMin(lwpf->pf);
	  SetResultFloat(aMin);
	  return;
	}
      /* Set */
      res = PFSetAMin(lwpf->pf,aMin);
      if(res != PFYes)
	Errorf("Problem while writing aMin, maybe it was <= 0");
      return;  
    }

  /* Case of the 'noct' field */
  else if (!strcmp(fieldName,"noct") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetOctaveNumber(lwpf->pf);
	  SetResultInt(number);
	  return;
	}
      /* Set */
      Errorf("You are not allowed to modify the octave number");
      return;  
    }
  
  /* Case of the 'nvoice' field */
  else if (!strcmp(fieldName,"nvoice") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetVoiceNumber(lwpf->pf);
	  SetResultInt(number);
	  return;
	}
      /* Set */
      Errorf("You are not allowed to modify the voice number");
      return;  
    }
  
  /* Case of the 'nscale' field */
  else if (!strcmp(fieldName,"nscale") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetIndexMax(lwpf->pf);
	  SetResultInt(number + 1);
	  return;
	}
      /* Set */
      Errorf("You are not allowed to modify the number of scale computed");
      return;  
    }

  /* Case of the 'sigsize' field */
  else if (!strcmp(fieldName,"sigsize") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetSignalSize(lwpf->pf);
	  SetResultInt(number);
	  return;
	}
      /* Set */
      res = PFSetSignalSize(lwpf->pf,number);
      if(res != PFYes)
	Errorf("Problem while writing signal size, maybe it was <= 0");
      return;  
    }


  /* Case of the 'signumber' field */
  else if (!strcmp(fieldName,"signumber") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetSignalNumber(lwpf->pf);
	  SetResultInt(number);
	  return;
	}
      /* Set */
      Errorf("You are not allowed to modify the signal number");
      return;  
    }

  /* Case of the 'dim' field */
  else if (!strcmp(fieldName,"dim") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetDimension(lwpf->pf);
	  SetResultInt(number);
	  return;
	}
      /* Set */
      res = PFSetDimension(lwpf->pf,number);
      if(res != PFYes)
	Errorf("Problem while writing dimension, maybe it was <= 0");
      return;  
    }

  /* Case of the 'qnumber' field */
  else if (!strcmp(fieldName,"qnumber") )
    {
      argv = ParseArgv(argv,tINT_,-999,&number,0);
      /* Get */
      if( number == -999 ) 
	{
	  number = PFGetQNumber(lwpf->pf);
	  SetResultInt(number);
	  return;
	}
      /* Set */
      Errorf("You are not allowed to modify the q number (use setpf qlist)");
      return;  
    }

  /* Case of the 'qlist' field */
  else if (!strcmp(fieldName,"qlist") )
    {
      argv = ParseArgv(argv,tLIST_,NULL,&list,0);
      /* Get */
      if( list == NULL ) 
	{
	  number = PFGetQNumber(lwpf->pf);
	  if(number <= 0)
	    SetResultf(" ");
	  else
	    {
	      qArray = FloatAlloc(number);
	      TempPtr(qArray);
	      PFGetQListFloat(lwpf->pf,qArray);
	      /* AppendResultf("{");*/
	      for(nq=0;nq < number;nq++)
		AppendResultf(" %f ",qArray[nq]);
	      /* AppendResultf("}");*/
	    }
	  return;
	}
      /* Set */
      
      /* What is the list length */
      listTemp = list;
      number = 0;
      while (*listTemp != NULL) 
	{
	  number++;
	  listTemp++;
	}
      if( number <= 0 )
	Errorf("The qList is empty");
      /* We allocate space for the q's */
      qArrayD = (double *) Malloc(number*sizeof(double));
      /* We read the q's */
      for(nq=0;nq < number;nq++)
	{
	  if (ParseFloat_(list[nq],0,&q) == NO) 
	    {
	      Free(qArrayD);
	      Errorf1("");
	    }
	  qArrayD[nq] = (double) q;
	}     
      
      res = PFSetQList(lwpf->pf,number,qArrayD);
      Free(qArrayD);
      if(res != PFYes)
	Errorf("Problem while writing qList, may be pf has not been initialized");
      return;  
    }

  else Errorf("Unknow action '%s'",fieldName);  
}

/*
 * The field list
 */
struct field fieldsPartitionFunction[] = {

  "method", GetMethodPFV, SetMethodPFV, NULL, NULL,
  "amin", GetAminPFV, SetAminPFV, NULL, NULL,
  "noct", GetNOctPFV, NULL, NULL, NULL,
  "nvoice", GetNVoicePFV, NULL, NULL, NULL,
  "nscale", GetNScalePFV, NULL, NULL, NULL,
  "sigsize", GetSigSizePFV, SetSigSizePFV, NULL, NULL,
  "signum", GetSigNumPFV, NULL, NULL, NULL,
  "qnumber", GetQNumberPFV, NULL, NULL, NULL,
  "qlist", GetQListPFV, SetQListPFV, NULL, NULL,
  "dim", GetDimPFV, SetDimPFV, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for LWPARTFUNC
 */

TypeStruct tsPartitionFunction = {

  /* Documentation */
  "{{{&PF} "
  "{This type is the basic type for partition function handling "
  "for the multifractal formalism and in particular the WTMM method.}}} ",

  &partitionFunctionType,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeletePartitionFunction,     /* The Delete function */
  NewPartitionFunction,     /* The Delete function */
  
  CopyPartitionFunction,       /* The copy function */
  ClearPartitionFunction,       /* The clear function */
  
  ToStrPartitionFunction,       /* String conversion */
  PrintPartitionFunction,   /* The Print function : print the object when 'print' is called */
  PrintInfoPartitionFunction,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsPartitionFunction      /* The list of fields */
};

