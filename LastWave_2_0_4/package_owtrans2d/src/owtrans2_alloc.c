/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'owtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002 Geoff Davis, Emmanuel Bacry, Jerome Fraleu. */
/*                                                                          */
/*      The original program was written in C++ by Geoff Davis.             */
/*      Then it has been translated in C and adapted to LastWave by         */
/*      J. Fraleu and E. Bacry.                                             */
/*                                                                          */
/*      If you are interested in the C++ code please go to                  */
/*          http://www.cs.dartmouth.edu/~gdavis                             */
/*                                                                          */
/*      emails : geoffd@microsoft.com                                       */
/*               fraleu@cmap.polytechnique.fr                               */
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


#include "lastwave.h"
#include "int_fsilist.h"
#include "owtrans2d.h"
#include "compress2d.h"

extern OWAVELET2  NewOWavelet2(char * nameFilterset);
extern void DeleteOWavelet2(OWAVELET2 w);

static char defaultName[] = "";

char *owtrans2Type = "&owtrans2";



/*
 * Get the current wtransform 
 * (generate an error if there is none)
 */
 
OWTRANS2 GetOWtrans2Cur(void)
{
  OWTRANS2 wtrans;
 
 if (!ParseTypedValLevel_(levelCur, "objCur", NULL, (VALUE *) &wtrans, owtrans2Type)) Errorf1("");

  AddRefValue( wtrans);
  TempValue( wtrans);
   
  return(wtrans);
}

/*
 * Answers to the different print messages
 */
 
void PrintOWtrans2(OWTRANS2 wtrans)
{
  if (wtrans->name == NULL)
    Printf("<&owtrans2[%d];%p>\n",wtrans->noct,wtrans);
  else
    Printf("<'%s';&wtrans[%d];%p>\n",wtrans->name,wtrans->noct,wtrans);
}

char *ToStrOWtrans2(OWTRANS2 wtrans, char flagShort)
{
  static char str[30];
  
  if (wtrans->name == defaultName) {
    sprintf(str,"<&owtrans2;%p>",wtrans);
  }
  else {
    sprintf(str,"<&owtrans2;%s>",wtrans->name);
  }
  
  return(str);
}


void PrintInfoOWtrans2(OWTRANS2 wtrans)
{
  Printf("  number of octave  :  %2d\n",wtrans->noct);

  if (wtrans->wavelet != NULL) 
    Printf("  wavelet    :  %s\n",wtrans->wavelet->name);  
}

/*
 * NumExtraction
 *
 * (10a, 20, 31,...)
 */

static IMAGE Extract(OWTRANS2 wtrans,int o, int v)
{
  int numSubband;

  if (o < 0 || o > wtrans->noct) {
    if (wtrans->noct != 0) SetErrorf("Octave index '%d' out of range : should be in [0,%d]",o,wtrans->noct);
    else SetErrorf("Octave index '%d' out of range : should be 0",o);
    return(NULL);
  }
  if (o> 0 && (v < 0 || v >= 4)) {
    SetErrorf("Orientation number '%d' out of range : should be in [0,%d]",v,3);
    return(NULL);
  }
  if (o == 0 && (v < 0 || v >= 10)) {
    SetErrorf("Orientation number '%d' out of range : should be in [0,%d]",v,9);
    return(NULL);
  }
 
  if (o == 0 && v == 0) return(wtrans->original);
  else if (o == 0) return(wtrans->workimage[v-1]);
  else {
    numSubband = wtrans->noct*3 - 3*o+v;
    return(wtrans->subimage[numSubband]);
  }
}

static char *numdoc = "The syntax <i><j> refers to the image which corresponds to octave <i> and orientation <j>. The octave number <i> ranges between [1,noct]. \
The orientation number <j> is 1,2 or 3 (0 corresponds to the projection on V_i). The image 0 corresponds to the analyzed image. \
The images 1,2,...9 are working images that are not used by the wavelet transform algorithm.";

static void *NumExtractOWtrans2(OWTRANS2 wtrans,void **arg)
{
  int n;
  char flagDot;
  int v;
  int o;
  VALUE val;
  
     /* doc */
  if (wtrans == NULL) return(numdoc);

  n = ARG_NE_GetN(arg);
  flagDot = ARG_NE_GetFlagDot(arg);
  v = n%10;
  o = n/10;

  if (flagDot == YES) {
    SetErrorf("No '.' allowed in this syantax");
    return(NULL);
  }

  val = (VALUE) Extract(wtrans,o,v);
  if (val == NULL) return(NULL);
  
  ARG_NE_SetResValue(arg,val);
  return(imageiType);
}



/*
 * Basic function to call to get the image at octave 'oct' and orientation 'orient'
 */

IMAGE GetOWtrans2Image(OWTRANS2 wtrans, int oct, int orient)
{
  IMAGE i;
  
  i = Extract(wtrans,oct,orient);  
  if (i == NULL) Errorf1("");
  
  return(i);  
}


/*
 * Get the options for extraction : There is none !!
 */

static void *GetExtractOptionsOWtrans2V(OWTRANS2 wtrans, void **arg)
{
  static char *extractOptionsOWtrans2[] = {NULL};

  return(extractOptionsOWtrans2);
}

/*
 * Function to get the ExtractInfo 
 */

static void *GetExtractInfoOWtrans2V(OWTRANS2 wtrans, void **arg)
{
  char *field =  ARG_EI_GetField(arg);
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
static char *doc = "{[oct,orient]} {it returns the image which corresponds to octave <oct> and orientation <orient>. The octave number <oct> ranges between [1,noct]. \
The orientation number <orient> is 1,2 or 3 (and 0 corresponds to the projection on V_oct). The image [0,0] corresponds to the analyzed image. \
The images [0,1],[0,2],...,[0,9] are working images that are not used by the wavelet transform algorithm.}"; 
 
static void *GetExtractOWtrans2V(OWTRANS2 wtrans, void **arg)
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
          SetErrorf("owtrans2[] expects 2 indices");
          return(NULL);
        }
        
        /* Get the integers */
        o = (int) FSI_FIRST(fsiList);
        v = (int) FSI_SECOND(fsiList);        
 
  i = Extract(wtrans,o,v);

  if (i == NULL) return(NULL);   
        
  ARG_G_SetResValue(arg,i);
  AddRefValue(i);
  TempValue(i);
  return(GetTypeValue(i));
}


static void *SetExtractOWtrans2V(OWTRANS2 wtrans, void **arg)
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
          SetErrorf("owtrans2[] expects 2 indices");
          return(NULL);
        }
        
        /* Get the integers */
        o = (int) FSI_FIRST(fsiList);
        v = (int) FSI_SECOND(fsiList);        
 
  i = Extract(wtrans,o,v);

  if (i == NULL) return(NULL);   
        
  return(SetImageField(i,arg));
}


/***************************************************************************
*
*  Allocation and Desallocation
*
****************************************************************************/

/*
 * Allocation of a OWtrans2
 */
 
OWTRANS2 NewOWtrans2(void)
{
  OWTRANS2 trans;
  int i;

#ifdef DEBUGALLOC
DebugType = "OWtrans2";
#endif
  
  trans = (OWTRANS2) Malloc(sizeof(struct owtrans2));
  InitValue(trans,&tsOWtrans2);
  
  trans->name=defaultName;
 
  trans->wavelet = NULL;
  
  trans->workimage = (IMAGE *) Malloc(sizeof(IMAGE)*9);
  for (i=0;i<9;i++) trans->workimage[i] = NewImage();
  
  trans->hsize = trans->vsize = trans->noct = 0;
    
  trans->original = NewImage();
  
  trans->subimage = NULL;
  trans->coeff = NULL;
 
  return(trans);
}


/*
 * Desallocation of the subimages
 */
static void  DeleteSubImagesOWtrans2 (OWTRANS2 owtrans2)
{ 
  int nSubbands = 3 * owtrans2->noct + 1;
  int i;
  
  if (owtrans2->subimage) {
    for (i=0;i<nSubbands;i++) DeleteImage(owtrans2->subimage[i]);
    Free(owtrans2->subimage);
 
    owtrans2->subimage=NULL;
  }
  
  owtrans2->noct = 0;
}


/*
 * Sets the number of octaves and allocates in subimages 3*noct+1 images.
 * If ncol != 0 these images are resized to the right size for decomposition of an
 * original image of size ncol*nrow
 */
void SetNOctOWtrans2(OWTRANS2 wtrans,int noct,int ncol,int nrow)
{
  int i;
  int n;
  
  if (noct < 0) Errorf("SetNOctOWtrans2() : Bad 'noct' value '%d'",noct);
  
  DeleteSubImagesOWtrans2(wtrans);
  DeleteCoeffOWtrans2(wtrans);

  wtrans->vsize = nrow;
  wtrans->hsize = ncol;
  wtrans->noct = noct;
    

  if (wtrans->noct == 0) return;

  n = 3 * wtrans->noct + 1;  
  wtrans->subimage  = (IMAGE *) Malloc(sizeof(IMAGE)*n);
 
  for (i = wtrans->noct-1; i >=0; i--) {
    wtrans->subimage[3*i+1] = NewImage();
    wtrans->subimage[3*i+2] = NewImage();
    wtrans->subimage[3*i+3] = NewImage();
    if (ncol != 0) {
      nrow = nrow/2;
      ncol = ncol/2; 
      SizeImage(wtrans->subimage[3*i+1],ncol, nrow);
      SizeImage(wtrans->subimage[3*i+2],ncol, nrow);
      SizeImage(wtrans->subimage[3*i+3],ncol, nrow);
    }
  } 
 
  wtrans->subimage[0] = NewImage();
  
  if (ncol != 0) SizeImage(wtrans->subimage[0],ncol, nrow);     
}

  
/*
 * Desallocation of the coeffs
 */
void  DeleteCoeffOWtrans2 (OWTRANS2 owtrans2)
{ 
  int nSubbands = 3 * owtrans2->noct + 1;
  int i;
  
  if (owtrans2->coeff) {
    for (i=0;i<nSubbands;i++) DeleteCoeffSet(owtrans2->coeff[i]); 
    Free(owtrans2->coeff);
    owtrans2->coeff=NULL;
  }
}

/*
 * Desallocation of a OWtrans2
 */
void DeleteOWtrans2(OWTRANS2 wtrans2)
{
  int i;
 
  RemoveRefValue(wtrans2);
  if (wtrans2->nRef > 0) return;

  DeleteSubImagesOWtrans2(wtrans2);
  DeleteCoeffOWtrans2(wtrans2);

  DeleteImage(wtrans2->original);
   
  wtrans2->hsize = wtrans2->vsize =0;
  
  if (wtrans2->wavelet) DeleteOWavelet2(wtrans2->wavelet);
  wtrans2->wavelet = NULL;
 
  if (wtrans2->name != defaultName)  Free(wtrans2->name); 
  wtrans2->name=NULL;

  for (i=0;i<9;i++) DeleteImage(wtrans2->workimage[i]);

  Free(wtrans2->workimage);

#ifdef DEBUGALLOC
DebugType = "OWtrans2";
#endif
  
  Free(wtrans2);
}


/*
 * Clear a owtrans2
 */
 void ClearOWtrans2(OWTRANS2 owtrans2)
{ 
  int i;

  DeleteSubImagesOWtrans2(owtrans2);
  DeleteCoeffOWtrans2(owtrans2);

  ClearImage(owtrans2->original);
   
  owtrans2->hsize =owtrans2->vsize =0;
   
  for (i=0;i<9;i++) ClearImage(owtrans2->workimage[i]);
}
 
 
/*
 * Copy a WTRANS2 into another 
 */
OWTRANS2 CopyOWtrans2(OWTRANS2 in,OWTRANS2 out)
{
  int i;
  int n;

  /* Tests*/
  if (in == NULL) return(NULL);
  if (out == NULL) out = NewOWtrans2();
  if (in == out) return(in);

  ClearOWtrans2(out);
  
  SetNOctOWtrans2(out,in->noct,in->hsize,in->vsize);

  SetWaveletOWtrans2(out,in->wavelet->name);
  
  if (in->noct != 0) {
    n = 3*in->noct+1;
    for (i=0;i<n;i++) CopyImage(in->subimage[i],out->subimage[i]);
  }
  
  CopyImage(in->original,out->original);  
 
  for (i=0;i<9;i++) CopyImage(in->workimage[i],out->workimage[i]);  
  
  return(out);
}


/*
 * Set the wavelet of a OWtrans2
 */
void SetWaveletOWtrans2 (OWTRANS2 wtrans,char * nameWavelet)
{
  OWAVELET2 wave;
    
  if(!nameWavelet) wave = NewOWavelet2(defaultO2WaveletName);
  else wave = NewOWavelet2(nameWavelet);

  if (wtrans->wavelet != NULL) DeleteOWavelet2(wtrans->wavelet);
  wtrans->wavelet = wave;
}


/*
 * Check the fact that a owtrans2 is not empty
 */
void CheckOWtrans2(OWTRANS2 wtrans)
{ 
  int nrow,ncol,i,j,n;
  IMAGE image1;
  
  /*
   * We check the size of each image
   */
  if (wtrans->subimage==NULL) Errorf("CheckOWtrans2() : The owtrans2 is empty !");  
  nrow = wtrans->vsize;
  ncol = wtrans->hsize;
  n = 3 * wtrans->noct + 1;  
  for (i = wtrans->noct-1; i >=0; i--) {
    nrow = nrow/2;
    ncol = ncol/2; 
    for (j=1;j<=3;j++) {
      image1 = wtrans->subimage[3*i+j];
      if (image1->ncol != ncol || image1->nrow != nrow) Errorf("CheckOWtrans2() : Bad size of owtrans2 subimage");
    }
  } 
  image1 = wtrans->subimage[0];
  if (image1->ncol != ncol || image1->nrow != nrow) Errorf("CheckOWtrans2() : Bad size of owtrans2 subimage");
}  

/*
 * 'name' field
 */
static char *nameDoc = "{[= <name>]} {Sets/Gets the name of a owtrans2}";

static void * GetNameOWtrans2V(OWTRANS2 wtrans, void **arg)
{
  /* Documentation */
  if (wtrans == NULL) return(nameDoc);
  
  return(GetStrField(wtrans->name,arg));
}

static void * SetNameOWtrans2V(OWTRANS2 wtrans, void **arg)
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
static char *noctDoc = "{[= <noct>]} {Gets the number of octave of a 2d orthogonal wavelet transform.}";

static void * GetNOctOWtrans2V(OWTRANS2 wtrans, void **arg)
{
  /* Documentation */
  if (wtrans == NULL) return(noctDoc);
  
  return(GetIntField(wtrans->noct,arg));
}

/*
 * 'wavelet' field
 */
static char *waveletDoc = "{[= <name>]} {Gets/Sets the analyzing wavelet used for the wavelet transform.}";

static void * GetWaveletOWtrans2V(OWTRANS2 wtrans, void **arg)
{
  /* Documentation */
  if (wtrans == NULL) return(waveletDoc);
  
  if (wtrans->wavelet == NULL) return(GetStrField("",arg));
  
  return(GetStrField(wtrans->wavelet->name,arg));
}

/*
 * The field list
 */
struct field fieldsOWtrans2[] = {

  "", GetExtractOWtrans2V, SetExtractOWtrans2V, GetExtractOptionsOWtrans2V, GetExtractInfoOWtrans2V,
  "name", GetNameOWtrans2V, SetNameOWtrans2V, NULL, NULL,
  "noct", GetNOctOWtrans2V, NULL, NULL, NULL,
  "wavelet", GetWaveletOWtrans2V, NULL, NULL, NULL,

  NULL, NULL, NULL, NULL, NULL
};


/*
 * The type structure for OWTRANS2
 */

TypeStruct tsOWtrans2 = {

  "{{{&owtrans2} {This type is the basic type for 2d orthogonal wavelet transforms.}}}",  /* Documentation */

  &owtrans2Type,       /* The basic (unique) type name */
  NULL,     /* The GetType function */                       
  
  DeleteOWtrans2,     /* The Delete function */
  NewOWtrans2,     /* The Delete function */
  
  CopyOWtrans2,       /* The copy function */
  ClearOWtrans2,       /* The clear function */
  
  ToStrOWtrans2,       /* String conversion */
  PrintOWtrans2,   /* The Print function : print the object when 'print' is called */
  PrintInfoOWtrans2,   /* The PrintInfo function : called by 'info' */

  NumExtractOWtrans2,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsOWtrans2,      /* The list of fields */
};
 

