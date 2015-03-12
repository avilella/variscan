/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'dwtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  E.Bacry, J.Fraleu, J.Kalifa, E. Le Pennec, */
/*                         W.L. Hwang , S.Mallat, S.Zhong                   */
/*      emails : lastwave@cmap.polytechnique.fr                             */
/*               fraleu@cmap.polytechnique.fr                               */
/*               kalifa@cmap.polytechnique.fr                               */
/*               lepennec@cmap.polytechnique.fr                             */
/*               mallat@cmap.polytechnique.fr                               */
/*               whwang@iis.sinica.edu.tw                                   */
/*               szhong@chelsea.princeton.edu                               */
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
/*                   of WTRANSFORM structure                                */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "int_fsilist.h"
#include "extrema2d.h"


char *dwtrans2Type = "&dwtrans2";
static char defaultName[] = "";
  

/*
 * Answers to the different print messages
 */
 
void PrintWtrans2(WTRANS2 wtrans)
{
  if (wtrans->name == NULL)
    Printf("<&dwtrans2[%d];%p>\n",wtrans->noct,wtrans);
  else
    Printf("<'%s';&dwtrans2[%d];%p>\n",wtrans->name,wtrans->noct,wtrans);
}

char *ToStrWtrans2(WTRANS2 wtrans, char flagShort)
{
  static char str[30];
  
  if (wtrans->name == defaultName) {
    sprintf(str,"<&dwtrans2;%p>",wtrans);
  }
  else {
    sprintf(str,"<&dwtrans2;%s>",wtrans->name);
  }
  
  return(str);
}


void PrintInfoWtrans2(WTRANS2 wtrans)
{
  Printf("  number of octave  :  %2d\n",wtrans->noct);

  if (wtrans->wName != NULL) 
  Printf("  Wavelet name      :  %s\n",wtrans->wName);  
}

/*
 * NumExtraction
 *
 * (10a, 20, 31,...)
 */

static IMAGE Extract(WTRANS2 wtrans,int o, int v)
{
  if (o < 0 || o >= W2_MAX_LEVEL) {
    SetErrorf("Octave index '%d' out of range : should be in [0,%d]",o,W2_MAX_LEVEL);
    return(NULL);
  }

  if (v < 0 || v >= W2_MAX_ORIENTATION) {
    SetErrorf("Octave index '%d' out of range : should be in [0,%d]",o,W2_MAX_ORIENTATION);
    return(NULL);
  }

  return(wtrans->images[o][v]);
}

static char *numdoc = "The syntax <i><j> refers to the image [i,j] of the dwtrans2 structure. The image [0,¯] corresponds to the analyzed image. \
The projection on the V_oct spaces are stored in [oct,0]. [oct,1] corresponds to the vertical details at octave oct \
and [oct,2] corresponds to the horizontal details at octave oct. [oct,3] corresponds to the norm of ([oct,1],[oct,2]) and \
[oct,4] to its phase. All the other images are not used by the wavelet transform and can be used as working images.}";


static void *NumExtractWtrans2(WTRANS2 wtrans,void **arg)
{
  VALUE value;
  int n;
  char flagDot;
  int v;
  int o;

     /* doc */
  if (wtrans == NULL) return(numdoc);

  n = ARG_NE_GetN(arg);
  flagDot = ARG_NE_GetFlagDot(arg);
  v = n%10;
  o = n/10;

  if (flagDot == YES) {
    SetErrorf("No '.' allowed in this syntax");
    return(NULL);
  }

  value = (VALUE) Extract(wtrans,o,v);
  if (value == NULL) return(NULL);
  ARG_NE_SetResValue(arg,value);  
  
  return(imageiType);
}



/*
 * Get the options for extraction : There is none !!
 */

static void *GetExtractOptionsWtrans2V(WTRANS2 wtrans, void **arg)
{
  static char *extractOptionsWtrans2[] = {NULL};

  return(extractOptionsWtrans2);
}

/*
 * Function to get the ExtractInfo 
 */

static void *GetExtractInfoWtrans2V(WTRANS2 wtrans, void **arg)
{
  char *field = ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.nSignals = 1;
    extractInfo.dx = 1;
    extractInfo.xmin = 0;
    extractInfo.flags = EIIntIndex | EIErrorBound;
    flagInit = NO;
  }

  extractInfo.xmax = MAX(wtrans->noct,9);
  
  return(&extractInfo);
}

/*
 * Extraction
 */
static char *doc = "{[oct,orient]} {it returns the image which corresponds to octave <oct> and orientation <orient>. The image [0,0] corresponds to the analyzed image. \
The projection on the V_oct spaces are stored in [oct,0]. [oct,1] corresponds to the vertical details at octave oct \
and [oct,2] corresponds to the horizontal details at octave oct. [oct,3] corresponds to the norm of ([oct,1],[oct,2]) and \
[oct,4] to its phase. All the other images are not used by the wavelet transform and can be used as working images.}";
 
static void *GetExtractWtrans2V(WTRANS2 wtrans, void **arg)
{
  char *field;
  FSIList *fsiList;
  IMAGE i;
  int o,v;

   /* doc */
  if (wtrans == NULL) return(doc);
  
  field = ARG_G_GetField(arg);
  fsiList = ARG_G_GetFsiList(arg);

     
        /* 2 integers in between the [] */
        if (fsiList->nx != 2) {
          SetErrorf("dwtrans2[] expects 2 indices");
          return(NULL);
        }
        
        /* Get the integers */
        o = (int) FSI_FIRST(fsiList);
        v = (int) FSI_SECOND(fsiList);        
 
  i = Extract(wtrans,o,v);

  if (i == NULL) return(NULL);   
  
  ARG_G_SetResValue(arg,i);
  return(GetTypeValue(i));
}

static void *SetExtractWtrans2V(WTRANS2 wtrans, void **arg)
{
  char *field;
  FSIList *fsiList;
  IMAGE i;
  int o,v;

   /* doc */
  if (wtrans == NULL) return(doc);
  
  field = ARG_S_GetField(arg);
  fsiList = ARG_S_GetFsiList(arg);

     
        /* 2 integers in between the [] */
        if (fsiList->nx != 2) {
          SetErrorf("dwtrans2[] expects 2 indices");
          return(NULL);
        }
        
        /* Get the integers */
        o = (int) FSI_FIRST(fsiList);
        v = (int) FSI_SECOND(fsiList);        
 
  i = Extract(wtrans,o,v);

  if (i == NULL) return(NULL);   
  
  return(SetImageField(i,arg));
}


/***********************************************************************************
 *
 *  Allocation and Desallocation
 *
 ***********************************************************************************/



/************  ALLOCATE A WTRANS2  ****************/

extern TypeStruct tsDWtrans2;

WTRANS2 NewWtrans2(void)
{
  WTRANS2 trans;
  int i,j;
  

#ifdef DEBUGALLOC
DebugType = "DWtrans2";
#endif

  trans = (WTRANS2) Malloc(sizeof(struct wtrans2));

  InitValue(trans,&tsDWtrans2);
  
  for (i=0;i<W2_MAX_LEVEL;i++)
    for (j=0;j<W2_MAX_ORIENTATION;j++) {
      trans->images[i][j] = NewImage();
      trans->images[i][j]->name = CharAlloc(8);
      if (i==0) sprintf(trans->images[i][j]->name,"%d",j);
      else sprintf(trans->images[i][j]->name,"%d%d",i,j);
    }
   
  trans->filterg1 = NewFilter2();  /* decomposition filter */
  trans->filterh1 = NewFilter2();
  trans->filterk1 = NewFilter2();

  trans->filterg2 = NewFilter2();  /* reconstruction filter */
  trans->filterh2 = NewFilter2();
  trans->filterk2 = NewFilter2();

  trans->noct = 0;
  trans->norient = 0;

  trans->extrep = NewExtrep2(); 


  trans->chainrep = NewChainrep2();
  trans->name=defaultName;
  trans->wName=NULL;
  trans->periodic=NO;
  
  return(trans);
}


void ClearWtrans2(WTRANS2 wtrans2)
{
  int i,j;

  for (i=0;i<W2_MAX_LEVEL;i++)

    for (j=0;j<W2_MAX_ORIENTATION;j++) {
	 ClearImage(wtrans2->images[i][j]);
    }
   
  
   wtrans2->noct =0;
   wtrans2->norient=0;
   wtrans2->periodic=NO;
   if (wtrans2->wName) Free(wtrans2->wName);
   wtrans2->wName=NULL;
   ClearFilter2(wtrans2->filterg1); 
   ClearFilter2(wtrans2->filterh1); 
   ClearFilter2(wtrans2->filterk1); 

   ClearFilter2(wtrans2->filterg2); 
   ClearFilter2(wtrans2->filterh2); 
   ClearFilter2(wtrans2->filterk2); 
   
   DeleteExtrep2(wtrans2->extrep);
   DeleteChainrep2(wtrans2->chainrep);
 
   
   wtrans2->extrep = NewExtrep2(); 
  wtrans2->chainrep = NewChainrep2();  
}

/*
 * Copy a WTRANS2 into another 
 */

WTRANS2 CopyWtrans2(WTRANS2 in,WTRANS2 out)
{
  int i,j;
  
  /* Tests*/
  if (in == NULL) return(NULL);
  if (out == NULL) out = NewWtrans2();
  if (in == out) return(out);

  out->noct = in->noct;
  out->norient = in ->norient;
  out->periodic = in->periodic;
   if (in->wName)  out->wName = CopyStr(in->wName);
   else if (out->wName) {
     Free(out->wName); 
     out->wName=NULL;
   }
   
/* Copy all the images */
  for (i=0;i<W2_MAX_LEVEL;i++)
    for (j=0;j<W2_MAX_ORIENTATION;j++) {
      if (in->images[i][j] != NULL) {
        if (out->images[i][j] == NULL) out->images[i][j] =  NewImage();
	    CopyImage(in->images[i][j],out->images[i][j]);
      }
     
    };
    
    return(out);
}

/*
 * Desallocation of a wtrans
 */

void DeleteWtrans2(WTRANS2 wtrans2)
{
  int i,j;

  RemoveRefValue(wtrans2);
  if (wtrans2->nRef > 0) return;
  
  for (i=0;i<W2_MAX_LEVEL;i++)

    for (j=0;j<W2_MAX_ORIENTATION;j++) {
      DeleteImage(wtrans2->images[i][j]);
    }
   
  
   if (wtrans2->wName) Free(wtrans2->wName);
   wtrans2->wName=NULL;

   
   DeleteFilter2(wtrans2->filterg1); 
   DeleteFilter2(wtrans2->filterh1); 
   DeleteFilter2(wtrans2->filterk1); 

   DeleteFilter2(wtrans2->filterg2); 
   DeleteFilter2(wtrans2->filterh2); 
   DeleteFilter2(wtrans2->filterk2); 



    DeleteExtrep2(wtrans2->extrep);

   DeleteChainrep2(wtrans2->chainrep);
 
   if (wtrans2->name != defaultName)  Free(wtrans2->name); 
   wtrans2->name=NULL;
  
#ifdef DEBUGALLOC
DebugType = "DWtrans2";
#endif
   
   Free(wtrans2); 
}


/*
 * Get the current wtransform 
 * (generate an error if there is none)
 */
 
WTRANS2 GetWtrans2Cur(void)
{
  WTRANS2 wtrans;
 
 if (!ParseTypedValLevel_(levelCur, "objCur", NULL, (VALUE *) &wtrans, dwtrans2Type)) Errorf1("");

  AddRefValue( wtrans);
  TempValue( wtrans);
   
  return(wtrans);
}


/************************************************************************
                       COMMANDS
************************************************************************/


void C_SetDWtrans2(char **argv)
{
  WTRANS2 wtrans=NULL;
  char * fieldName;
  int noct,norient;
  char * name1;
  

  argv = ParseArgv(argv,tSTR,&fieldName,tWTRANS2_,NULL,&wtrans,-1);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();

  if (!strcmp(fieldName,"noct")) {
    
    argv= ParseArgv(argv,tINT_,-1,&noct,0);
  
   if (noct<0) SetResultInt(wtrans->noct);
   else if (noct > W2_MAX_LEVEL-1)  Errorf(" Bad 'noct' value '%d'",noct);
   else wtrans->noct=noct;
   return;
  }

/* else if (!strcmp(fieldName,"norient")) {
    
    argv= ParseArgv(argv,tINT_,-1,&norient,0);
    
      if (norient <0)  
      SetResultInt(wtrans->norient);    
      else 
        if (norient > W2_MAX_ORIENTATION-1) 
          Errorf(" Bad 'norient' value '%d'",norient);
        else
          wtrans->norient=norient;
     return;
  }
 */
 else if (!strcmp(fieldName,"name")) {
    NoMoreArgs(argv);
    SetResultStr(wtrans->name);
    return;
  }

 else if (!strcmp(fieldName,"wname")) {
    if (*argv == NULL) SetResultStr(wtrans->wName);
    else {
      argv = ParseArgv(argv,tSTR,&name1,0);
      if (wtrans->wName != NULL) Free(wtrans->wName);
      wtrans->wName = CopyStr(name1);
    }
  }
 
  else if (!strcmp(fieldName,"*image")) {
      argv = ParseArgv(argv,tVNAME_,NULL,&name1,tINT,&noct,tINT,&norient,0);
      if (!((noct>wtrans->noct) || (norient>(wtrans->norient+1))))
        SetImageVariable(name1,wtrans->images[noct][norient]);
      else  Errorf("Bad octave number '%d' or orient number '%d'",noct,norient);
  }
 else Errorf("Unknown dwtrans2 field '%s'",fieldName);  
}




 
void C_PrintFilter2(char **argv)
{ 
  WTRANS2 wtrans=NULL;
  int i;
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);
  if (wtrans ==NULL) wtrans= GetWtrans2Cur();

  Printf("\n------------------------\nFilter for decomposition : H1 G1 K1\n");
  if (wtrans->filterh1->size==0 || wtrans->filterg1->size==0 || wtrans->filterk1->size==0)
   Printf("Not initialized\n");
  else{ 
  Printf("name :%s\n",wtrans->filterh1->name);
  Printf("Filter H1\n");
  Printf("Size : %d   Symetry : %f \n",wtrans->filterh1->size,wtrans->filterh1->symmetry);
  Printf("Values : \n");
  for (i=0;i<wtrans->filterh1->size;i++)
   Printf("%f \n",wtrans->filterh1->values[i]);
 
  Printf("\nFilter G1\n");
  Printf("Size : %d   Symmetry : %f \n",wtrans->filterg1->size,wtrans->filterg1->symmetry);
  Printf("Values : \n");
  for (i=0;i<wtrans->filterg1->size;i++)
   Printf("%f \n",wtrans->filterg1->values[i]);
 
  Printf("\nFilter K1\n");
  Printf("Size : %d   Symmetry : %f \n",wtrans->filterk1->size,wtrans->filterk1->symmetry);
  Printf("Values : \n");
  for (i=0;i<wtrans->filterk1->size;i++)
   Printf("%f \n",wtrans->filterk1->values[i]);
  }
  Printf("\n------------------------\nFilter for reconstruction : H2 G2 K2\n");

  if (wtrans->filterh2->size==0 || wtrans->filterg2->size==0 || wtrans->filterk2->size==0)
   Printf("Not initialized\n");
  else{
  Printf("name :%s\n",wtrans->filterh2->name);
  Printf("Filter H2\n");
  Printf("Size : %d   Symetry : %f \n",wtrans->filterh2->size,wtrans->filterh2->symmetry);
  Printf("Values : \n");
  for (i=0;i<wtrans->filterh2->size;i++)
   Printf("%f \n",wtrans->filterh2->values[i]);
 
  Printf("\nFilter G2\n");
  Printf("Size : %d   Symmetry : %f \n",wtrans->filterg2->size,wtrans->filterg2->symmetry);
  Printf("Values : \n");
  for (i=0;i<wtrans->filterg2->size;i++)
   Printf("%f \n",wtrans->filterg2->values[i]);
 
  Printf("\nFilter K2\n");
  Printf("Size : %d   Symmetry : %f \n",wtrans->filterk2->size,wtrans->filterk2->symmetry);
  Printf("Values : \n");
  for (i=0;i<wtrans->filterk2->size;i++)
      Printf("%f \n",wtrans->filterk2->values[i]);}

}


/**************************/
/* Print WTRANS2 fields   */
/**************************/

/* 'dw2info' command */
void C_DW2Info(char ** argv)
    
{
  WTRANS2 wtrans=NULL;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);
  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
  
  Printf("Wtransform %s    [%d]\n",wtrans->name,wtrans->nRef);
  Printf(" periodic flag : %d\n",wtrans->periodic);
  Printf(" noct          : %d \n",wtrans->noct);

  Printf("Filter for decomposition  : ");
  if (wtrans->filterh1->size==0 || wtrans->filterg1->size==0 || wtrans->filterk1->size==0)
   Printf("Not initialized\n");
  else{ 
  Printf("%s\n",wtrans->filterh1->name);

  }

  Printf("Filter for reconstruction : ");

  if (wtrans->filterh2->size==0 || wtrans->filterg2->size==0 || wtrans->filterk2->size==0)
   Printf("Not initialized\n");
  else{
  Printf("%s\n",wtrans->filterh2->name); 
}

 Printf("\n");
  
}

/*
 * 'name' field
 */
static char *nameDoc = "{[= <name>]} {Sets/Gets the name of a dwtrans2}";

static void * GetNameWtrans2V(WTRANS2 val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(nameDoc);
  
  return(GetStrField(val->name,arg));
}

static void * SetNameWtrans2V(WTRANS2 wtrans, void **arg)
{
       /* doc */
  if (wtrans == NULL) return(nameDoc);

  if (wtrans->name==defaultName) {
    wtrans->name=CharAlloc(1);
    wtrans->name[0] = '\0';
  }
  return(SetStrField(&(wtrans->name),arg));
}

/*
 * 'noct' field
 */
static char *noctDoc = "{[= <noct>]} {Gets the number of octave of a 2d dyadic wavelet transform.}";

static void * GetNOctWtrans2V(WTRANS2 val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(noctDoc);
  
  return(GetIntField(val->noct,arg));
}

/*
 * 'wavelet' field
 */
static char *waveletDoc = "{} {Gets the analyzing wavelet used for the dyadic wavelet transform.}";

static void * GetWaveletWtrans2V(WTRANS2 val, void **arg)
{
  /* Documentation */
  if (val == NULL) return(waveletDoc);
  
  if (val->wName == NULL) return(GetStrField("",arg));
  
  return(GetStrField(val->wName,arg));
}


/*
 * The field list
 */
struct field fieldsWtrans2[] = {

  "", GetExtractWtrans2V, SetExtractWtrans2V, GetExtractOptionsWtrans2V, GetExtractInfoWtrans2V,
  "name", GetNameWtrans2V, SetNameWtrans2V, NULL, NULL,
  "noct", GetNOctWtrans2V, NULL, NULL, NULL,
  "wavelet", GetWaveletWtrans2V, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};



/*
 * The type structure for DWTRANS2
 */

TypeStruct tsDWtrans2 = {

  "{{{&dwtrans2} {This type is the basic type for 2d dyadic wavelet transforms. It contains a 2d array of images. The first index 'oct' \
corresponds to the octave number and the second index is called 'orient'. When a wavelet transform is performed, the analyzed image must be in \
[0,0]. The projection on the V_oct spaces are stored in [oct,0]. [oct,1] corresponds to the vertical details at octave oct \
and [oct,2] corresponds to the horizontal details at octave oct. [oct,3] corresponds to the norm of ([oct,1],[oct,2]) and \
[oct,4] to its phase. All the other images are not used by the wavelet transform and can be used as working images.}}}",  /* Documentation */

  &dwtrans2Type,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteWtrans2,     /* The Delete function */
  NewWtrans2,     /* The Delete function */
  
  CopyWtrans2,       /* The copy function */
  ClearWtrans2,       /* The clear function */
  
  ToStrWtrans2,       /* String conversion */
  PrintWtrans2,   /* The Print function : print the object when 'print' is called */
  PrintInfoWtrans2,   /* The PrintInfo function : called by 'info' */

  NumExtractWtrans2,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsWtrans2,      /* The list of fields */
};
 
