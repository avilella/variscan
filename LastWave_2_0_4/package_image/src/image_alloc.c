/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0.3                      */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry, Jerome Fraleu.              */
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
/*  image_alloc.c   Functions which deal with the dynamical                 */
/*                   allocation of memory for IMAGE 's                      */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "images.h"
#include "int_fsilist.h"



/**********************************************
 *
 * Send a message to an image
 *
 **********************************************/

/* Default name for an image */
static char defaultName[] = "";

char *imageType = "&image";
char *imageiType = "&imagei"; 



/********************************************/
extern int flagOn;

IMAGE NewImage()
{
  extern TypeStruct tsImage;

  IMAGE image;
  
#ifdef DEBUGALLOC
DebugType = "Image";
#endif
  
  image = (IMAGE) (Malloc(sizeof(struct image)));
  
  InitValue(image,&tsImage);
  
  image->pixels = NULL;
  image->nrow = image->ncol = 0;
  image->sizeMalloc = 0;
  image->border_ver =0;
  image->border_hor =0;
  image->name = defaultName;


  if (flagOn) Printf("** New Image %p\n",image); 

  return (image);
}


IMAGE TNewImage(void)
{
  IMAGE image;
  
  image = NewImage();
  TempValue(image);
  return(image);
}


void DeleteImage(image)
     IMAGE image;
{
  if (image) {
    if (image->nRef==0) {
      Warningf("DeleteImage() : Trying to delete a temporary image\n");
      return;
    }
    RemoveRefValue(image);
    if (image->nRef > 0) return;
    if (flagOn) Printf("** Delete Image %p\n",image); 
   
    if (image->name != defaultName) Free(image->name);
    image->name=NULL;
    if (image->pixels) Free(image->pixels);
    image->pixels = NULL;

#ifdef DEBUGALLOC
DebugType = "Image";
#endif

    Free(image);
    image=NULL;
  }
}


/* Reinitialization of structure and desallocate the array */
void ClearImage(IMAGE image)
{
  if (image->pixels) Free(image->pixels);
  image->pixels = NULL;
  image->nrow = image->ncol = 0;
  image->sizeMalloc = 0;
}     


void SizeImage(IMAGE image, int ncol, int nrow)
{
  extern float *FloatCAlloc(int size);

  int s;
  int nrow1= nrow+ (1-nrow%2);
  int ncol1= ncol+ (1-ncol%2);
  
  if (image == NULL) Errorf("SizeImage() : Try to change the size of an NULL image. \n");

  s = nrow1*ncol1;
  if (s > image->sizeMalloc) {
    if (image->pixels) Free(image->pixels);
    image->pixels = FloatCAlloc(s);
    image->sizeMalloc = s;
  }
  image->nrow = nrow;
  image->ncol = ncol;
}

IMAGE CopyImage(IMAGE in,IMAGE out)
{
  int nrow1= in->nrow+ (1-in->nrow%2);
  int ncol1= in->ncol+ (1-in->ncol%2);

 if (in == out) return(out);
 
 if (out == NULL) out = NewImage();
    
 SizeImage(out,in->ncol,in->nrow);
  
 CopyFieldsImage(in,out);
  
  memcpy(out->pixels,in->pixels,nrow1*ncol1*sizeof(float));
  
  return(out);
}

void CopyFieldsImage(IMAGE in,IMAGE  out)
{
  out->border_hor =  in->border_hor; 
  out->border_ver =  in->border_ver;
}

/***********************************************************************************
 *
 *  Image and variables
 *
 ***********************************************************************************/
/* 
 * Getting an image variable at a given level
 *   (Set the error message and returns NULL if there is an error)
 */ 

IMAGE GetImageVariableLevel(LEVEL level,char *name)
{
  IMAGE image;
  char *type;
  VALUE value;
  value = GetVariableContentLevel(level,name,NULL);
   type = GetTypeValue(value);
  if (type != imageType && type != imageiType) {
     SetErrorf("GetImageVariableLevel() : Variable '%s' has not the expected type '&image' or '&imagei'",name);
    return(NULL);
  }
 
  image = (IMAGE) value;

  return(image);
} 

IMAGE GetImageVariable(char *name)
{
  return(GetImageVariableLevel(levelCur,name));
}


/* 
 * Setting an image variable at a given level (creates it if it does not exist)
 */ 
 
void SetImageVariableLevel(LEVEL level,char *name,IMAGE image)
{
  SetVariableLevel(level,name,(VALUE) image);
}

void SetImageVariable(char *name,IMAGE image)
{
  SetVariable(name,(VALUE) image);
}




/***********************************************************************************
 *
 *  Parsing an image 
 *
 ***********************************************************************************/
 
 
/*
 * Parse an output Image
 */

char ParseImageLevel_(LEVEL level, char *arg, IMAGE defVal, IMAGE *im)
{
  char *type;
  float f;
  
  type = TTEvalExpressionLevel_(level,arg,&f,(VALUE *) im,ImageType,NO,NO,AnyType,YES);

  if (type == NULL) {
    *im = defVal;
    if (defVal != NULL) (*im)->nRef++;
    return(NO);
  }
  
  return(YES);
}

char ParseImage_(char *arg, IMAGE defVal, IMAGE *im)
{
  return(ParseImageLevel_(levelCur,arg,defVal,im));
}

void ParseImageLevel(LEVEL level,char *arg,IMAGE *im)
{
  if (ParseImageLevel_(level,arg,NULL,im) == NO) Errorf1("");
}

void ParseImage(char *arg, IMAGE *im)
{
  ParseImageLevel(levelCur,arg,im);
}

/*
 * Parse an input Image
 */

char ParseImageILevel_(LEVEL level, char *arg, IMAGE defVal, IMAGE *im)
{  
  char *type;
  float f;
  
  type = TTEvalExpressionLevel_(level,arg,&f,(VALUE *) im,ImageType,NO,NO,AnyType,NO);

  if (type == NULL || (*im)->nrow == 0 || (*im)->ncol == 0) {
    *im = defVal;
    if (defVal != NULL) {
      if (defVal->nrow == 0 || defVal->ncol == 0) Errorf("ParseImageILevel_() : default signal is empty");
      (*im)->nRef++;
    }
    return(NO);
  }    
  
  return(YES);
}


char ParseImageI_(char *arg, IMAGE defVal,  IMAGE *im)
{
  return(ParseImageILevel_(levelCur,arg,defVal,im));
}

void ParseImageILevel(LEVEL level, char *arg,  IMAGE *im)
{
  if (ParseImageILevel_(level,arg,NULL,im) == NO) Errorf1("");
}

void ParseImageI(char *arg,  IMAGE *im)
{
  ParseImageILevel(levelCur,arg,im);
}



void C_InfoImage (char **argv)
{  

  IMAGE image;
  
  argv = ParseArgv(argv,tIMAGEI,&image,0);

  if (image == NULL) 
    Errorf(" NULL image");
  
  Printf(" Image '%s'    [%d]\n",image->name,image->nRef);
  
  

  Printf(" ncol x nrow  :  %d x %d  [%d]\n",image->ncol,image->nrow,image->sizeMalloc);
    
  Printf("\n");
  
}


void C_CopyImage (char **argv)
{  

  
  IMAGE input,output;
  
  argv = ParseArgv(argv,tIMAGEI,&input,tIMAGE,&output,0);
  
  
  CopyImage(input,output);

}


/*
 * Function to get the type of an image
 */
static char * GetTypeImage(VALUE value)
{
  IMAGE i = (IMAGE) value;
  
  if (i->nrow == 0 || i->ncol == 0) return(imageType);
  return(imageiType);
}


/*
 * Print an image when it is a result
 */

#define ImPrintLengthRow 6
#define ImPrintLengthCol 10
 
void PrintImage(IMAGE im)
{
  int r,c;
  
  if (im->ncol==0 || im->nrow==0) {
    Printf("<nrow=%d;ncol=%d>",im->nrow,im->ncol);
    return;
  }
  for (r = 0; r < im->nrow; r++) {
    for (c = 0; c < im->ncol; c++) {
      Printf("%g ", im->pixels[im->ncol*r+c]);
    }
    Printf("\n");
  }
}

/*
 * String conversion
 */

char *ToStrImage(IMAGE im, char flagShort)
{
  static char strShort[50];
  int nAlloc;
  char *str;
  int r,c;

  if (flagShort || im->ncol==0 || im->nrow==0 || im->ncol > ImPrintLengthCol || im->nrow > ImPrintLengthRow) {
    sprintf(strShort,"<nrow=%d;ncol=%d>",im->nrow,im->ncol);
    return(strShort);
  }

  nAlloc = 300;
  str = CharAlloc(nAlloc);
  TempPtr(str);
  sprintf(str,"<nrow=%d;ncol=%d>\n",im->nrow,im->ncol);
  
  for (r = 0; r < im->nrow; r++) {
    for (c = 0; c < im->ncol; c++) {
      if (strlen(str) > nAlloc-40) {
        nAlloc+=300;
        str = CharAlloc(nAlloc);
        TempPtr(str);
      }
      sprintf(strShort,"%g ",im->pixels[im->ncol*r+c]);
      strcat(str,strShort);
    }
    if (r!= im->nrow-1) strcat(str,"\n");
  }
  return(str);
}

/*
 * Print the info of a signal
 */
void PrintInfoImage(IMAGE im)
{
  Printf("   nrow x ncol  :  %d x %d  [%d]\n",im->nrow,im->ncol,im->sizeMalloc);
}


/*
 * The main routine that returns a pixel value given (col,row) indexes and a border type.
 */
float CR2PixIm(IMAGE im,int row, int col, BorderType btc, BorderType btr)
{
  switch (btc) {
    case BorderNone :
    break;
    case BorderPer : 
      col = fmod(col,im->ncol);
      if (col < 0) col+=im->ncol;
      break;
    case BorderMir1 : 
      col = fmod(col,2*im->ncol);
      if (col < 0) col+=2*im->ncol;
      if (col >= im->ncol) col=2*im->ncol-1-col;
      break;
    case BorderMir : 
      col = fmod(col,2*im->ncol-2);
      if (col < 0) col+=2*im->ncol-2;
      if (col >= im->ncol) col=2*im->ncol-2-col;
      break;
    case Border0 :
      if (col < 0 || col>=im->ncol) return(0);
      break;
  }
  switch (btr) {
    case BorderNone :
    break;
    case BorderPer : 
      row = fmod(row,im->nrow);
      if (row < 0) row+=im->nrow;
      break;
    case BorderMir1 : 
      row = fmod(row,2*im->nrow);
      if (row < 0) row+=2*im->nrow;
      if (row >= im->nrow) row=2*im->nrow-1-row;
      break;
    case BorderMir : 
      row = fmod(row,2*im->nrow-2);
      if (row < 0) row+=2*im->nrow-2;
      if (row >= im->nrow) row=2*im->nrow-2-row;
      break;
    case Border0 :
      if (row < 0 || row>=im->nrow) return(0);
      break;
  }
  
  return(im->pixels[im->ncol*row+col]);
}
      

/*
 * The different extract options 
 */
static char *extractOptionsIm[] = {"*nolimit","*bperiodic","*bmirror","*bmirror1","*b0",NULL};
enum {
  FSIOptImNoLimit = FSIOption1,
  FSIOptImBPer = FSIOption2,
  FSIOptImBMir = FSIOption3,
  FSIOptImBMir1 = FSIOption4,
  FSIOptImB0 = FSIOption5
};

/*
 * Doc for extraction []
 */
static char *doc = "{[*opt,rowIndexes;colIndexes] [:]= (<float> | <range> | <signal> | <image>)} {Get/Set the images values}"; 



/* 
 * Set an image [] using explicit parameters
 */
char *SetImageField_(IMAGE imLeft,char *field, FSIList *fsiList, float fltRight, VALUE value,char *equal, char *fieldName)
{
  FSI_DECL_ROW;
  FSI_DECL_COL;
  int i,j,k,size,_j,_iold,n;
  char *type;
  SIGNAL sigRight,sig;
  IMAGE imRight,im;
  LISTV lv;
  float *arrayLeft;
  char flagY;
  RANGE rg;
  
  /* Get the type of the right value  */
  if (value == NULL) type = numType;
  else {
    value = ValueOf(value);
    type = GetTypeValue(value);
  }
  
  /* Conversion of range to an image */
  if (type == rangeType) {
    rg = CastValue(value,RANGE);
    im = TNewImage();
    SizeImage(im,rg->size,1);
    im->pixels[0] = rg->first;
    for (i=1;i<rg->size;i++) im->pixels[i] = im->pixels[i-1]+rg->step;
    value = (VALUE) im;
    type = imageiType;
  }
  /* Conversion of signal to an image */
  else if (type == signaliType) {
    sig = CastValue(value,SIGNAL);
    im = TNewImage();
    SizeImage(im,sig->size,1);
    for (i=0;i<sig->size;i++) im->pixels[i] = sig->Y[i];
    value = (VALUE) im;
    type = imageiType;
  }
  /* Conversion of an empty signal to an image */
  else if (type == signalType) {
    im = TNewImage();
    value = (VALUE) im;
    type = imageType;
  }
    
  if (type != numType && type != imageiType && type != imageType) {
    SetErrorf("Expect a float/range/signal/image (and not a '%s') on right handside",type);
    return(NULL);
  }

  if (type == numType) {
    if (value != NULL) fltRight = CastValue(value,NUMVALUE)->f;
    imRight = NULL;
  }
  else if (type == imageiType || type == imageType) {
    imRight = CastValue(value,IMAGE);
  }

  
  /*************************
   *
   * Case value operator is *= += -= /= ^= %=
   *
   *************************/
   
  if (*equal != '=' && *equal != ':') {

    if (type == listvType) {
      SetErrorf("Expect a float/signal/image (and not a '%s') on right handside",type);
      return(NULL);
    }

    if (imLeft->nrow  == 0 || imLeft->ncol  == 0) {
      SetErrorf("Expect a non empty image on the left handside");
      return(NULL);
    }
    
    if (imRight && (fsiList && (imRight->nrow != fsiList->nx || imRight->ncol != fsiList->ny) 
                    || !fsiList && (imRight->nrow != imLeft->nrow || imRight->ncol != imLeft->ncol))) {
      SetErrorf("Images should have the same size");
      return(NULL);
    }
    
    arrayLeft = imLeft->pixels;
    
    switch (*equal) {
    case '+' :  
      if (fsiList == NULL || fsiList->nx == 0 || fsiList->ny == 0) {
        if (imRight == NULL) for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]+=fltRight;
        else for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]+=imRight->pixels[i];
      }
      else {
        FSI_FOR_INIT(fsiList);
        if (!fsiList->flagImage) {
          if (imRight == NULL) {FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]+=fltRight;FSI_FOR_END_COL;FSI_FOR_END_ROW;}
          else {n = 0;FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]+=imRight->pixels[n++];FSI_FOR_END_COL;FSI_FOR_END_ROW;}
        }
        else {
          if (imRight == NULL) {FSI_FOR_START_IMAGE(fsiList);arrayLeft[_ir*imLeft->ncol+_ic]+=fltRight;FSI_FOR_END_IMAGE;}
          else {n = 0;FSI_FOR_START_IMAGE(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]+=imRight->pixels[n++];FSI_FOR_END_IMAGE;}
        }
      }      
      break; 
    case '-' :  
      if (fsiList == NULL || fsiList->nx == 0 || fsiList->ny == 0) {
        if (imRight == NULL) for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]-=fltRight;
        else for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]-=imRight->pixels[i];
      }
      else {
        FSI_FOR_INIT(fsiList);
        if (!fsiList->flagImage) {
          if (imRight == NULL) {FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]-=fltRight;FSI_FOR_END_COL;FSI_FOR_END_ROW;}
          else {n = 0;FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]-=imRight->pixels[n++];FSI_FOR_END_COL;FSI_FOR_END_ROW;}
        }
        else {
          if (imRight == NULL) {FSI_FOR_START_IMAGE(fsiList);arrayLeft[_ir*imLeft->ncol+_ic]-=fltRight;FSI_FOR_END_IMAGE;}
          else {n = 0;FSI_FOR_START_IMAGE(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]-=imRight->pixels[n++];FSI_FOR_END_IMAGE;}
        }
      }      
      break; 
    case '*' :  
      if (fsiList == NULL || fsiList->nx == 0 || fsiList->ny == 0) {
        if (imRight == NULL) for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]*=fltRight;
        else for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]*=imRight->pixels[i];
      }
      else {
        FSI_FOR_INIT(fsiList);
        if (!fsiList->flagImage) {
          if (imRight == NULL) {FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]*=fltRight;FSI_FOR_END_COL;FSI_FOR_END_ROW;}
          else {n = 0;FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]*=imRight->pixels[n++];FSI_FOR_END_COL;FSI_FOR_END_ROW;}
        }
        else {
          if (imRight == NULL) {FSI_FOR_START_IMAGE(fsiList);arrayLeft[_ir*imLeft->ncol+_ic]*=fltRight;FSI_FOR_END_IMAGE;}
          else {n = 0;FSI_FOR_START_IMAGE(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]*=imRight->pixels[n++];FSI_FOR_END_IMAGE;}
        }
      }      
      break; 
    case '^' :  
      if (fsiList == NULL || fsiList->nx == 0 || fsiList->ny == 0) {
        if (imRight == NULL) for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]=pow(arrayLeft[i],fltRight);
        else for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]=pow(arrayLeft[i],imRight->pixels[i]);
      }
      else {
        FSI_FOR_INIT(fsiList);
        if (!fsiList->flagImage) {
          if (imRight == NULL) {FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]=pow(arrayLeft[_ir*imLeft->ncol+_ic],fltRight);FSI_FOR_END_COL;FSI_FOR_END_ROW;}
          else {n = 0;FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]=pow(arrayLeft[_ir*imLeft->ncol+_ic],imRight->pixels[n++]);FSI_FOR_END_COL;FSI_FOR_END_ROW;}
        }
        else {
          if (imRight == NULL) {FSI_FOR_START_IMAGE(fsiList);arrayLeft[_ir*imLeft->ncol+_ic]=pow(arrayLeft[_ir*imLeft->ncol+_ic],fltRight);FSI_FOR_END_IMAGE;}
          else {n = 0;FSI_FOR_START_IMAGE(fsiList);arrayLeft[_ir*imLeft->ncol+_ic]=pow(arrayLeft[_ir*imLeft->ncol+_ic],imRight->pixels[n++]);FSI_FOR_END_IMAGE;}
        }
      }      
      break; 
    case '%' :  
      if (fsiList == NULL || fsiList->nx == 0 || fsiList->ny == 0) {
        if (imRight == NULL) for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]= ((int) arrayLeft[i]) % ((int) fltRight);
        else for (i=0;i<imLeft->nrow*imLeft->ncol;i++) arrayLeft[i]=((int) arrayLeft[i]) % ((int) imRight->pixels[i]);
      }
      else {
        FSI_FOR_INIT(fsiList);
        if (!fsiList->flagImage) {
          if (imRight == NULL) {FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]=((int) arrayLeft[_ir*imLeft->ncol+_ic]) % ((int) fltRight);FSI_FOR_END_COL;FSI_FOR_END_ROW;}
          else {n = 0;FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]=((int) arrayLeft[_ir*imLeft->ncol+_ic]) % ((int) imRight->pixels[n++]);FSI_FOR_END_COL;FSI_FOR_END_ROW;}
        }
        else {
          if (imRight == NULL) {FSI_FOR_START_IMAGE(fsiList);arrayLeft[_ir*imLeft->ncol+_ic]=((int) arrayLeft[_ir*imLeft->ncol+_ic]) % ((int) fltRight);FSI_FOR_END_IMAGE;}
          else {n = 0;FSI_FOR_START_IMAGE(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]=((int) arrayLeft[_ir*imLeft->ncol+_ic]) % ((int) imRight->pixels[n++]);FSI_FOR_END_IMAGE;}
        }

      }      
      break; 
    case '/' :  
      if (imRight == NULL && fltRight == 0) {      
        SetErrorf("Division by 0");
        return(NULL);
      }
      if (fsiList == NULL || fsiList->nx == 0 || fsiList->ny == 0) {
        if (imRight == NULL) for (i=0;i<imLeft->nrow*imLeft->ncol;i++)  arrayLeft[i]/=fltRight;
        else for (i=0;i<imLeft->nrow*imLeft->ncol;i++) {
          if (imRight->pixels[i] == 0) {     
            SetErrorf("Division by 0");
            return(NULL);
          }
          arrayLeft[i]/=imRight->pixels[i];
        }
      }
      else {
        FSI_FOR_INIT(fsiList);
        if (!fsiList->flagImage) {
          if (imRight == NULL) {FSI_FOR_START_ROW(fsiList);FSI_FOR_START_COL(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]/=fltRight;FSI_FOR_END_COL;FSI_FOR_END_ROW;}
          else {
            n = 0;
            FSI_FOR_START_ROW(fsiList);
            FSI_FOR_START_COL(fsiList); 
            if (imRight->pixels[n] == 0) {     
              SetErrorf("Division by 0");
              return(NULL);
            }
            arrayLeft[_ir*imLeft->ncol+_ic]/=imRight->pixels[n++];
            FSI_FOR_END_COL;
            FSI_FOR_END_ROW;
          }
        }
        else {
          if (imRight == NULL) {FSI_FOR_START_IMAGE(fsiList); arrayLeft[_ir*imLeft->ncol+_ic]/=fltRight;FSI_FOR_END_IMAGE;}
          else {
            n = 0;
            FSI_FOR_START_IMAGE(fsiList);
            if (imRight->pixels[n] == 0) {     
              SetErrorf("Division by 0");
              return(NULL);
            }
            arrayLeft[_ir*imLeft->ncol+_ic]/=imRight->pixels[n++];
            FSI_FOR_END_IMAGE;
          }
        }
      }      
      break; 
    }
  	return(imageiType);
}

  /*************************
   *
   * Case i[rowIndices;colIndices] = image/float
   *
   *************************/

  if (*equal == '=' && type != listvType) {
    /* Case of i=imRight */
    if (fsiList == NULL) {
      if (imRight) CopyImage(imRight,imLeft);
      else {
        SizeImage(imLeft,1,1);
        imLeft->pixels[0] = fltRight;
      }
      return(imageiType);
    }      
      
    /* Case of im[]=imRight */
    if (fsiList->nx==0 || fsiList->ny==0) {
      if (imRight) CopyImage(imRight,imLeft);
      else {
        SizeImage(imLeft,1,1);
        imLeft->pixels[0] = fltRight;
      }
      return(imageiType);
    }      
    
    if (imLeft->ncol  == 0 || imLeft->nrow  == 0) {
      SetErrorf("Expect a non empty image on the left handside");
      return(NULL);
    }
   
    arrayLeft = imLeft->pixels;
  
    /* Case im[1;1]=2 */
    if (type == numType) {
      if (fsiList->nx != 1 || fsiList->ny != 1) {
        SetErrorf("Size of both handsides should match (left is %dx%d, right is 1x1)",fsiList->nx,fsiList->ny);
        return(NULL);
      }
      FSI_FOR_INIT(fsiList);
      _ir =  (int) FSIArray((&(fsiList->fsi[0])),0);
      _ic =  (int) FSIArray((&(fsiList->fsi[_ns+1])),0);
      arrayLeft[_ir*imLeft->ncol+_ic]=fltRight;
      return(imageiType);
    }
    

    /* Case of im[...]=imRight */
    if (!(fsiList->flagImage) && (imRight->nrow != fsiList->nx || imRight->ncol != fsiList->ny)) {
      SetErrorf("Size of both handsides should match (left is %dx%d, right is %dx%d)",fsiList->nx,fsiList->ny,imRight->nrow,imRight->ncol);
      return(NULL);
    }
    if (fsiList->flagImage && imRight->nrow != 1) {
      SetErrorf("Right handside should not be an image");
      return(NULL);
    }
    if (!(fsiList->flagImage)) {
      FSI_FOR_INIT(fsiList);
      n = 0;
      FSI_FOR_START_ROW(fsiList); 
      FSI_FOR_START_COL(fsiList); 
      arrayLeft[_ir*imLeft->ncol+_ic] = imRight->pixels[n++];
      FSI_FOR_END_COL;
      FSI_FOR_END_ROW;
      return(imageiType);
    }
    else {
      n = 0;
      FSI_FOR_INIT(fsiList);
      FSI_FOR_START_IMAGE(fsiList);
      arrayLeft[_ir*imLeft->ncol+_ic] = imRight->pixels[n++];
      FSI_FOR_END_IMAGE;
      return(imageiType);
   }
  }
  
  /* 
   * Case of im[range1,...,rangeN] := float
   */   

  if (*equal == ':') {
    if (fsiList == NULL || type != numType) {
      SetErrorf("Inadequate ':=' syntax");
      return(NULL);
    }
    arrayLeft = imLeft->pixels;
    if (!(fsiList->flagImage)) {
      FSI_FOR_INIT(fsiList);
      FSI_FOR_START_ROW(fsiList); 
      FSI_FOR_START_COL(fsiList); 
      arrayLeft[_ir*imLeft->ncol+_ic] = fltRight;
      FSI_FOR_END_COL;
      FSI_FOR_END_ROW;
      return(imageiType);
    }
    else {
      FSI_FOR_INIT(fsiList);
      FSI_FOR_START_IMAGE(fsiList);
      arrayLeft[_ir*imLeft->ncol+_ic] = fltRight;
      FSI_FOR_END_IMAGE;
      return(imageiType);
   }
  }


 SetErrorf("Weird error");
  return(NULL);
}


/*
 * Basic routine to deal with setting [] an image
 */
void *SetImageField(IMAGE im,void **arg)
{
  char *field = ARG_S_GetField(arg);
  FSIList *fsiList = ARG_S_GetFsiList(arg);
  char *type = ARG_S_GetRightType(arg);    
  float flt = ARG_S_GetRightFloat(arg);   
  VALUE value = ARG_S_GetRightValue(arg);
  char *equal = ARG_S_GetEqual(arg);

  if (SetImageField_(im,field,fsiList,flt,value,equal,NULL) == NULL) return(NULL);
  ARG_S_SetResValue(arg,(VALUE) im);
  return(imageiType);
}  


/* 
 * Routine to deal with setting [] images
 */

static void *SetExtractImageV(VALUE value,void **arg)
{
   /* doc */
  if (value == NULL) return(doc);

  return(SetImageField((IMAGE) value,arg));
}

/* 
 * Routines to deal with extracting [] images
 */

static void *GetImageField_(IMAGE im, void **arg)
{
  char *field;
  FSIList *fsiList;
  float *pFlt;
  char **pStr;
  
  int max;
  FSI_DECL_ROW;
  FSI_DECL_COL;
  BorderType bt;
  SIGNAL sigResult;
  IMAGE imResult;
  float xmax,xmin,ymax,ymin;
  ExtractInfo *ei;
  char flagIndex;
  float firstFlt;
  int n,first,last;
  char flagX,flagY;
  RANGE rg;
  int i,nx1,ny1;

  field = ARG_G_GetField(arg);
  fsiList = ARG_G_GetFsiList(arg);
  pFlt = ARG_G_GetResPFloat(arg);
  pStr = ARG_G_GetResPStr(arg);

  if (fsiList == NULL) Errorf("GetExtractImageV() : Weird error");

  /* Get extractInfo */
  ei = fsiList->ei;
  xmax = ei->xmax;
  xmin = ei->xmin;
  ymax = ei->ymax;
  ymin = ei->ymin;

  if (fsiList->options & FSIOptImBPer) bt = BorderPer;
  else if (fsiList->options & FSIOptImBMir) bt = BorderMir;
  else if (fsiList->options & FSIOptImBMir1) bt = BorderMir1;
  else if (fsiList->options & FSIOptImB0) bt = Border0;
  else bt = BorderNone;
 
  /* Case of flagImage im[<;>] */
  if (fsiList->flagImage) {
    sigResult = TNewSignal();
    SizeSignal(sigResult,fsiList->nx,YSIG);
    sigResult->size = 0;
    FSI_FOR_INIT(fsiList);
    fsic = &(fsiList->fsi[_ns+1]);
    FSI_FOR_START_ROW(fsiList); 
    _fc = FSIArray(fsic,_kr1);
    if (fsiList->options & FSIOptImNoLimit) {
      if (_fr<0 || _fr >= im->nrow) continue;
      if (_fc<0 || _fc >= im->ncol) continue;
    }
    sigResult->Y[sigResult->size++] = CR2PixIm(im,(int) _fr,(int) _fc,bt,bt);
    FSI_FOR_END_ROW;
    ARG_G_SetResValue(arg,sigResult);
    return(signaliType);
  }

  if (bt == BorderNone) {nx1 = fsiList->nx1; ny1 = fsiList->ny1;}
  else {nx1 = fsiList->nx; ny1 = fsiList->ny;}
  
  /* case of an empty extraction */
  if (nx1 == 0 || ny1 == 0) {
    ARG_G_SetResValue(arg,TNewImage());
    return(imageType);
  }
    
  /*
   * Case the result will be a simple float
   */
  if (nx1 == 1 && ny1 == 1) {

    FSI_FOR_INIT(fsiList);
    
    if (fsiList->options & FSIOptImNoLimit) {
      FSI_FOR_START_ROW(fsiList); 
      if (_fr<xmin || _fr > xmax) continue;
      else break;
      FSI_FOR_END_ROW;
      FSI_FOR_START_COL(fsiList); 
      if (_fc<ymin || _fc > ymax) continue;
      else break;
      FSI_FOR_END_COL;
    }
    else  {
      _fr =  FSIArray((&(fsiList->fsi[0])),0);
      _fc =  FSIArray((&(fsiList->fsi[_ns+1])),0);
    }

    *pFlt = CR2PixIm(im,(int) _fr,(int) _fc,bt,bt);

    return(numType);
  }
  
  /*
   * Case the result will be a signal
   */
  if (nx1 == 1) {
    FSI_FOR_INIT(fsiList);
    
    if (fsiList->options & FSIOptImNoLimit) {
      FSI_FOR_START_ROW(fsiList); 
      if (_fr<xmin || _fr > xmax) continue;
      else break;
      FSI_FOR_END_ROW;
      sigResult = TNewSignal();
      SizeSignal(sigResult,ny1,YSIG);
      n = 0;
      FSI_FOR_START_COL(fsiList); 
      if (_fc<ymin || _fc > ymax) continue;
      else sigResult->Y[n] = CR2PixIm(im,(int) _fr,(int) _fc,bt,bt);
      n++;
      FSI_FOR_END_COL;
    }
    else  {
      _fr =  FSIArray((&(fsiList->fsi[0])),0);
      sigResult = TNewSignal();
      SizeSignal(sigResult,ny1,YSIG);
      n = 0;
      FSI_FOR_START_COL(fsiList); 
      sigResult->Y[n] = CR2PixIm(im,(int) _fr,(int) _fc,bt,bt);
      n++;
      FSI_FOR_END_COL;
    }

    ARG_G_SetResValue(arg,sigResult);

    return(signaliType);
  }


  /*
   * Case the result will be an image
   */
  FSI_FOR_INIT(fsiList);
    
  if (fsiList->options & FSIOptImNoLimit) {
    imResult = TNewImage();
    SizeImage(imResult,ny1,nx1);
    n = 0;
    FSI_FOR_START_ROW(fsiList); 
    if (_fr<xmin || _fr > xmax) continue;
    FSI_FOR_START_COL(fsiList); 
    if (_fc<ymin || _fc > ymax) continue;
    imResult->pixels[n] = CR2PixIm(im,(int) _fr,(int) _fc,bt,bt);
    n++;
    FSI_FOR_END_COL;
    FSI_FOR_END_ROW;
  }
  else  {
    imResult = TNewImage();
    SizeImage(imResult,ny1,nx1);
    n = 0;
    FSI_FOR_START_ROW(fsiList); 
    FSI_FOR_START_COL(fsiList); 
    imResult->pixels[n] = CR2PixIm(im,(int) _fr,(int) _fc,bt,bt);
    n++;
    FSI_FOR_END_COL;
    FSI_FOR_END_ROW;
  }

  ARG_G_SetResValue(arg,imResult);

  return(imageiType);
  
}


void *GetImageExtractField(IMAGE im, void **arg)
{
  char *field = ARG_G_GetField(arg);
  void *res;
  
  ARG_G_SetField(arg,NULL);
  
  res = GetImageField_(im, arg);
  
  ARG_G_SetField(arg,field);

  return(res);
}
  
static void *GetExtractImageV(VALUE value, void **arg)
{
  char *field = ARG_G_GetField(arg);

  /* doc */
  if (value == NULL) return(doc);
  
  return(GetImageField_((IMAGE) value, arg));
}


/*
 * Function to get the ExtractInfo for field NULL
 */

static void *GetExtractInfoImageV(VALUE value, void **arg)
{
  static ExtractInfo extractInfo;
  static char flagInit = YES;
  
  IMAGE im = (IMAGE) value;
  char *field = (char *) ARG_EI_GetField(arg);
  unsigned long *options = ARG_EI_GetPOptions(arg);


  if (im->nrow == 0 || im->ncol == 0) {
    SetErrorf("No extraction on empty image");
    return(NULL);
  }

  /* If *bperiodic,... then *nolimit must be off */
  if (*options & (FSIOptImBPer | FSIOptImBMir | FSIOptImBMir1 | FSIOptImB0)) *options &= ~FSIOptImNoLimit;
      
  /* Init of the extraction info */
  if (flagInit) {
    extractInfo.nSignals = 2;
    flagInit = NO;
  }
  
  extractInfo.xmax = im->nrow-1;
  extractInfo.dx = 1;
  extractInfo.xmin = 0;

  extractInfo.ymax = im->ncol-1;
  extractInfo.dy = 1;
  extractInfo.ymin = 0;
      
  /* '*nolimit' option : set some flags */
  extractInfo.flags = EIIntIndex;
  if (!(*options & (FSIOptImNoLimit | FSIOptImBPer | FSIOptImBMir | FSIOptImBMir1 | FSIOptImB0))) extractInfo.flags |= EIErrorBound;
      
  return(&extractInfo);
}

/*
 * Get the options for extraction (called for field NULL only)
 */

static char *optionDoc = "{{*nolimit,*b0,*bconst,*bmirror,*bmirror1,*bperiodic} \
{*nolimit : indexes can be out of range} \
{*b0 : border effect with 0 value} \
{*bperiodic : periodic border effect)} \
{*bmirror1 : mirror+periodic border effect (first and last points are repeated)} \
{*bmirror : mirror+periodic border effect (first and last points are NOT repeated)}\
}";

static void *GetExtractOptionsImageV(VALUE value, void **arg)
{
  IMAGE im;
  char *field;
  
   /* doc */
  if (value == NULL) {return(optionDoc);}

  im = (IMAGE) value;

  field = (char *) ARG_EO_GetField(arg);
  if (field != NULL) return(NULL);
  return(extractOptionsIm);
}


/*
 * 'ncol' field
 */

static char *ncolDoc = "{[= <ncol>]} {Sets/Gets the number of columns of an image. If allocation is changed then all the values are initialized to 0.}";


static void * GetNcolImageV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(ncolDoc);
  
  return(GetIntField(((IMAGE) value)->ncol,arg));
}

static void * SetNcolImageV(VALUE value, void **arg)
{
 IMAGE im = (IMAGE) value;
 int size;
  
 /* doc */
 if (value == NULL) return(ncolDoc);

 size = im->ncol;
 if (SetIntField(&size,arg,FieldPositive)==NULL) return(NULL);
 SizeImage(im,size,im->nrow);
 return(numType);

}

/*
 * 'nrow' field
 */

static char *nrowDoc = "{[= <nrow>]} {Sets/Gets the number of rows of an image. If allocation is changed then all the values are initialized to 0.}";


static void * GetNrowImageV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(nrowDoc);
  
  return(GetIntField(((IMAGE) value)->nrow,arg));
}

static void * SetNrowImageV(VALUE value, void **arg)
{
 IMAGE im = (IMAGE) value;
 int size;
  
 /* doc */
 if (value == NULL) return(nrowDoc);

 size = im->nrow;
 if (SetIntField(&size,arg,FieldPositive)==NULL) return(NULL);
 SizeImage(im,im->ncol,size);
 return(numType);

}

/*
 * 'name' field
 */

static char *nameDoc = "{[= <name>]} {Sets/Gets the name of an image}";

static void * GetNameImageV(VALUE value, void **arg)
{
  /* Documentation */
  if (value == NULL) return(nameDoc);
  
  return(GetStrField(((IMAGE) value)->name,arg));
}

/* Set the name of an image */
extern int SetNameImage(IMAGE im, char *name)
{
  if (im->name != defaultName && im->name != NULL) {
    Free(im->name);
    im->name = NULL;
  }
  if (name == NULL) im->name = defaultName;
  else im->name = CopyStr(name);
  return(1);
}


static void * SetNameImageV(VALUE value, void **arg)
{
  IMAGE im = (IMAGE) value;
  
     /* doc */
  if (value == NULL) return(nameDoc);

  if (im->name==defaultName) {
    im->name=CharAlloc(1);
    im->name[0] = '\0';
  }
  return(SetStrField(&(im->name),arg));
}

/*
 * 'tosignal' field
 */

static char *tosignalDoc = "{} {Converts the image to a signal (it just puts the rows one after the other)}";

static void * GetSignalImageV(IMAGE im, void **arg)
{
  SIGNAL sig;
  
  if (im == NULL) return(tosignalDoc);

  sig = TNewSignal();
  SizeSignal(sig,im->nrow*im->ncol,YSIG);
  memcpy(sig->Y,im->pixels,sizeof(float)*sig->size);
  
  return(GetValueField(sig,arg));
}


/*
 * The field list
 */
struct field fieldsImage[] = {
  "", GetExtractImageV, SetExtractImageV, GetExtractOptionsImageV, GetExtractInfoImageV,
  "ncol", GetNcolImageV, SetNcolImageV, NULL, NULL,
  "nrow", GetNrowImageV, SetNrowImageV, NULL, NULL,
  "name", GetNameImageV, SetNameImageV, NULL, NULL,
  "tosignal", GetSignalImageV, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL, NULL
};


/*
 * The type structure for IMAGE
 */

TypeStruct tsImage = {

  "{{{&image} {This type is the basic type for images/matrices. Images \
can be built using the <value11,...,value1M;value21,...,value2M;...;valueN1,...,valueNM> syntax. \
The values can be either a float, a signal, a range, or an image, \
signals, ranges and images. The different operators are \n \
- +,-,*,/ (and +=,-=,*=,/=) : regular operators \n \
- //,% : integer division and remainder \n \
- ==,!=,<=,>=,<,> : regular tests \n \
- x^f (and ^=) : each value of |x| is taken to the popwer f \n \
- x*^n : each value of x to the power n where n is a positive integer \n\
- ~ : transposition operator \n\
- ** : matrix multiplication \n\
- x^^n : take the square matrix m to the power n where n is an integer \n\
- is,isnot : test if 2 signals correspond or not to the same C object \n \
- sinh,sin,cosh,cos,tanh,tan,acos,asin,atan : trigonometric operators \n \
- min,max : if 1 argument, returns the min or max value of an image, if 2 arguments returns \
the image made of the min/max of each value. \n\
- log2,log,ln,sqrt,abs,exp,ceil,floor,round,frac,int : other math functions \n \
- sum : computes the sum of all image values \n \
- mean : same as sum but divides by the total number of points\n \
- any : returns 1 if at least one of the values is different from 0\n \
- all : returns 1 if all of the values are different from 0\n \
- find : returns a 2xN image made of index couples corresponding to non 0 values \n\
- Image Constructors : <...;...;...>,Id,Zero,One,I,J,Grand,Urand and diag (for building diagonal matrices from a range/signal).}} \
{{&imagei} {This type corresponds to non empty images.}}}",  /* Documentation */

  &imageType,       /* The basic (unique) type name */
  GetTypeImage,     /* The GetType function */                       
  
  DeleteImage,     /* The Delete function */
  NewImage,     /* The New function */
  
  CopyImage,       /* The copy function */
  ClearImage,       /* The clear function */
  
  ToStrImage,       /* String conversion */
  PrintImage,   /* The Print function : print the object when 'print' is called */
  PrintInfoImage,   /* The PrintInfo function : called by 'info' */

  NULL,              /* The NumExtract function : used to deal with syntax like 10a */
   
  fieldsImage,      /* The list of fields */
};
