/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0.1                      */
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



#include "lastwave.h"
#include "images.h"
#include "signals.h"



/* 
 * Initializes an image to 0
 */
void ZeroImage(IMAGE input)
{
  int j;
  float *values;
  int nrow1= input->nrow+ (1-input->nrow%2);
  int ncol1= input->ncol+ (1-input->ncol%2);

  values = input->pixels;

  for (j = 0; j < nrow1*ncol1; j++)
    values[j] = 0.0;
}


/*
 * Adds two images 
 */
void AddImage(IMAGE image1, IMAGE image2,IMAGE output)
{
  int i;
  int nrow = image1->nrow;
  int ncol = image1->ncol;
  float *values_output, *values_image1, *values_image2;
  

  if ((nrow != image2->nrow) || (ncol != image2->ncol)) {
     Errorf ( "Size of images are not equal \n");
  }

  values_image1 = image1->pixels;
  values_image2 = image2->pixels;


  if (output == NULL) Errorf("AddImage() : Weird error");
  
  if((output != image1) && (output != image2)) SizeImage(output, ncol, nrow);

  values_output = output->pixels;

  for (i = 0; i < nrow * ncol; i++) 
    values_output[i] = values_image1[i] + values_image2[i];

  output->border_hor= image1->border_hor;
  output->border_ver= image1->border_ver;
}


/*
 * Substracts two images 
 */

void SubImage(IMAGE image1, IMAGE image2,IMAGE output)
{
  int i;
  int nrow = image1->nrow;
  int ncol = image1->ncol;
  float *values_output, *values_image1, *values_image2;
  

  if ((nrow != image2->nrow) || (ncol != image2->ncol)) {
     Errorf ( "Size of images are not equal \n");
  }

  values_image1 = image1->pixels;
  values_image2 = image2->pixels;


  if (output == NULL) Errorf("SubImage() : Weird error");
  
  if((output != image1) && (output != image2)) SizeImage(output, ncol, nrow);

  values_output = output->pixels;

  for (i = 0; i < nrow * ncol; i++) 
    values_output[i] = values_image1[i] - values_image2[i];

  output->border_hor= image1->border_hor;
  output->border_ver= image1->border_ver;
}

/*
 * Multiplies two images 
 */
void MulImage(IMAGE image1, IMAGE image2,IMAGE output)
{
  int i;
  int nrow = image1->nrow;
  int ncol = image1->ncol;
  float *values_output, *values_image1, *values_image2;
  

  if ((nrow != image2->nrow) || (ncol != image2->ncol)) {
     Errorf ( "Size of images are not equal \n");
  }

  values_image1 = image1->pixels;
  values_image2 = image2->pixels;


  if (output == NULL) Errorf("MulImage() : Weird error");
  
  if((output != image1) && (output != image2)) SizeImage(output, ncol, nrow);

  values_output = output->pixels;

  for (i = 0; i < nrow * ncol; i++) 
    values_output[i] = values_image1[i] * values_image2[i];

  output->border_hor= image1->border_hor;
  output->border_ver= image1->border_ver;
}

/*
 * Divides two images 
 */
void DivImage(IMAGE image1, IMAGE image2,IMAGE output)
{
  int i;
  int nrow = image1->nrow;
  int ncol = image1->ncol;
  float *values_output, *values_image1, *values_image2;
  

  if ((nrow != image2->nrow) || (ncol != image2->ncol)) {
     Errorf ( "Size of images are not equal \n");
  }

  values_image1 = image1->pixels;
  values_image2 = image2->pixels;


  if (output == NULL) Errorf("DivImage() : Weird error");
  
  if((output != image1) && (output != image2)) SizeImage(output, ncol, nrow);

  values_output = output->pixels;

  for (i = 0; i < nrow * ncol; i++) {
    if (values_image2[i] == 0) {
      Warningf("DivImage() : Division by zero");
      values_output[i] = FLT_MAX/2;
    }
    else values_output[i] = values_image1[i] / values_image2[i];
  }

  output->border_hor= image1->border_hor;
  output->border_ver= image1->border_ver;
}

/*
 * Adds an image with a number 
 */
void AddNumImage(IMAGE output,float  num)
{
  int j;
  int nrow, ncol;
  float *values;

  nrow = output->nrow;
  ncol = output->ncol;
  values = output->pixels;
  for (j = 0; j < nrow * ncol; j++)
    values[j] += num;
}


/*
 * Substracts an image with a number 
 */
void SubNumImage(IMAGE output, double num)
{
  int j;
  int nrow, ncol;
  float *values;

  nrow = output->nrow;
  ncol = output->ncol;
  values = output->pixels;
  for (j = 0; j < nrow * ncol; j++)
    values[j] -= num;
}


/*
 * Divides an image with a number 
 */
void DivNumImage(IMAGE output, double num)
{
  int j;
  int nrow, ncol;
  float *values;

  if (num == 0) Errorf("DivNumImage() : Division by 0");
  
  nrow = output->nrow;
  ncol = output->ncol;
  values = output->pixels;
  for (j = 0; j < nrow * ncol; j++)
    values[j] /= num;
}


/*
 * Multiplies an image with a number 
 */
void MulNumImage(IMAGE output, double num)
{
  int j;
  int nrow, ncol;
  float *values;

  nrow = output->nrow;
  ncol = output->ncol;
  values = output->pixels;
  for (j = 0; j < nrow * ncol; j++)
    values[j] = values[j] * (float) num ;
}


/*
 * Gets the minimum and maximum values of an image
 */
void MinMaxImage(IMAGE image,int *xmin,int *ymin, float *pmin,int *xmax,int *ymax,float *pmax)
{
  int x,y;

  if (xmin != NULL) *xmin = *ymin = *xmax = *ymax = 0;
  (*pmin) = (*pmax) = image->pixels[0];
  for (y = 0; y < image->nrow; y++) {
    for (x = 0; x < image->ncol; x++) {
      if (image->pixels[y*image->ncol+x] > (*pmax)) {
        (*pmax) = image->pixels[y*image->ncol+x];
        if (xmin != NULL) {
          *ymax = y;
          *xmax = x;
        }
      }
      if (image->pixels[y*image->ncol+x] < (*pmin)) {
        (*pmin) = image->pixels[y*image->ncol+x];
        if (xmin != NULL) {
          *ymin = y;
          *xmin = x;
        }
      }
    }
  }
}

/* Get moment of order 'n' (returns the mean) */
float GetNthMomentImage(IMAGE image, int n, float *pNthMoment, int flagCentered)
{
  int i,k;
  float m,mn,s,f;

  if (n< 0) Errorf("GetNthMomentImage() : 'n' should be positive");

  if (n==0) {
    *pNthMoment = 1;
    return(1);
  }
      
  /* Compute the mean */
  m = 0;
  if (flagCentered || n ==1) {
    for (i = 0;i<image->nrow*image->ncol;i++) m += image->pixels[i];
    m /= image->nrow*image->ncol;
  }

  /* If n == 1 we are done ! */
  if (n == 1) {
    *pNthMoment = m;
    return(m);
  }
  
  /* Compute the Nth moment */
  mn = 0;
  for (i = 0;i<image->nrow*image->ncol;i++) {
    f = image->pixels[i]-m;
    s = 1;
    for (k=0;k<n;k++) s *= f;
    mn += s;
  }
  mn /= image->nrow*image->ncol;
  *pNthMoment = mn;
  
  return(m);
}

/* Get absolute moment of order 'f1' (returns the mean) */
float GetAbsMomentImage(IMAGE image, float f1, float *pNthMoment, int flagCentered)
{
  int i;
  float m,mn;

  if (f1==0) {
    *pNthMoment = 1;
    return(1);
  }
      
  /* Compute the mean */
  m = 0;
  if (flagCentered) {
    for (i = 0;i<image->nrow*image->ncol;i++) m += image->pixels[i];
    m /= image->nrow*image->ncol;
  }

  /* If n == 1 we are done ! */
  if (f1 == 1) {
    *pNthMoment = m;
    return(m);
  }
  
  /* Compute the Nth moment */
  mn = 0;
  for (i = 0;i<image->nrow*image->ncol;i++) {
    mn += pow(fabs(image->pixels[i]-m),f1);
  }
  mn /= image->nrow*image->ncol;
  *pNthMoment = mn;
  
  return(m);
}

/* Get Lp Norm */
float GetLpNormImage(IMAGE im, float p)
{
  int i;
  float lp;
       
  /* Compute the Lp Norm */
  lp = 0;
  for (i = 0;i<=im->nrow*im->ncol;i++) {
    lp += pow(fabs(im->pixels[i]),p);
  }
  lp = pow(lp,1.0/p);

  return(lp);
}
/*
 * Command for computing some statistical values of an image
 */
 
void C_StatsImage(char **argv)
{
  char *action;
  IMAGE image;
  float f,m,min,max,f1,p;
  int i,iMin,iMax,jMin,jMax;
  LISTV lv;
  int flagAbs,flagCentered;
  char opt;


  argv = ParseArgv(argv,tWORD,&action,-1);

  /* 'mean' action */
  if (!strcmp(action,"mean")) {
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    GetNthMomentImage(image,1,&f,NO);
    SetResultFloat(f);
  }

  /* 'var' action */
  else if (!strcmp(action,"var")) {
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    GetNthMomentImage(image,2,&f,YES);
    SetResultFloat(f);
  }

  /* 'skew' action */
  else if (!strcmp(action,"skew")) {
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    GetNthMomentImage(image,3,&f,YES);
    SetResultFloat(f);
  }

  /* 'kurt' action */
  else if (!strcmp(action,"kurt")) {
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    GetNthMomentImage(image,4,&f,YES);
    SetResultFloat(f);
  }

  /* 'nth' action */
  else if (!strcmp(action,"nth")) {
    argv = ParseArgv(argv,tIMAGEI,&image,tFLOAT,&f1,-1);
    flagAbs = NO;
    flagCentered = NO;
    while(opt = ParseOption(&argv)) { 
      switch(opt) {
        case 'a': flagAbs = YES; break;
        case 'C': flagCentered = YES; break;
        default: ErrorOption(opt);
      }
    }    
    NoMoreArgs(argv);
   if (!flagAbs) {
     if (f1 != (int) f1 || f1 < 0) Errorf("<n> should be a positive integer unless you set option '-a'");
     m = GetNthMomentImage(image,(int) f1,&f,flagCentered);
   }
   else m = GetAbsMomentImage(image,f1,&f,flagCentered);
   SetResultFloat(f);
  }
    
  /* 'minmax' action */
  else if (!strcmp(action,"minmax")) {
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    MinMaxImage(image,&jMin,&iMin,&min,&jMax,&iMax,&max);
    lv = TNewListv();
    AppendInt2Listv(lv,iMin);
    AppendInt2Listv(lv,jMin);
    AppendInt2Listv(lv,iMax);
    AppendInt2Listv(lv,jMax);
    SetResultValue(lv);
  }

  /* 'lp' action */
  else if (!strcmp(action,"lp")) {
    argv = ParseArgv(argv,tIMAGEI,&image,tFLOAT,&p,0);
    f = GetLpNormImage(image,p);
    SetResultFloat(f);
  }
  
  /* 'print' action */
  else if (!strcmp(action,"print")) {
    argv = ParseArgv(argv,tIMAGEI,&image,0);
    m = GetLpNormImage(image,2);
    Printf("L2 norm   : %.8g\n",m);
    m = GetNthMomentImage(image,2,&f,YES);
    Printf("Mean      : %.8g\n",m);
    Printf("Variance  : %.8g\n",f);
    m = GetNthMomentImage(image,3,&f,YES);
    Printf("Skewness  : %.8g\n",f);
    m = GetNthMomentImage(image,4,&f,YES);
    Printf("Kurtosis  : %.8g\n",f);
    MinMaxImage(image,&jMin,&iMin,&min,&jMax,&iMax,&max);
    Printf("Minimum at %dx%d is %.8g\n",iMin,jMin,min); 
    Printf("Maximum at %dx%d is %.8g\n",iMax,jMax,max); 
  }

  else Errorf("Unknow action '%s'",action);    	
}




/*
 * Gets a horizontal or vertical cut of an image 
 */
void ImageGetCut ( IMAGE image,SIGNAL signal, char direction, int index)
{ 
  int i;
 
  if (direction=='H')
  {
    SizeSignal(signal,image->ncol,YSIG);
    for (i=0;i<image->ncol;i++)
     signal->Y[i] = image->pixels[image->ncol * index +i];    
    return;
  }    
    
  if (direction=='V')
  {
     SizeSignal(signal,image->nrow,YSIG);
     for (i=0;i<image->nrow;i++)
       signal->Y[i] = image->pixels[image->ncol * i +index];
    return;
  }    
  
  Errorf("ImageGetCut() : Bad direction '%c'",direction);
}


/*
 * Sets a horizontal or vertical cut of an image 
 */
void ImageSetCut (IMAGE image,SIGNAL signal, char direction, int index)
{ 
  int i;
 
  if (direction=='H')
  { 
    if (signal->size < image->ncol) Errorf("ImageSetCut() : Signal is too short");
    for (i=0;i<image->ncol;i++)
    image->pixels[image->ncol * index +i] =  signal->Y[i] ;
    return;
  }    

 
    
  if (direction=='V')
  {
    if (signal->size < image->nrow) Errorf("ImageSetCut() : Signal is too short");
    for (i=0;i<image->nrow;i++)
     image->pixels[image->ncol * i +index] = signal->Y[i]  ;

    return;
  }    
  
  Errorf("ImageSetCut() : Bad direction '%c'",direction);
}

/*
 * Compute the scalar product of 2 images 
 */
double ImageScalarProduct(IMAGE image1,IMAGE image2)
{
  int i;
  int nrow = image1->nrow;
  int ncol = image1->ncol;
  float *values_image1, *values_image2;
  double sum;

  if ((nrow != image2->nrow) || (ncol != image2->ncol)) 
     Errorf ( "ImageScalarProduct() : Size of images are not equal \n");

  values_image1 = image1->pixels;
  values_image2 = image2->pixels;
  sum = 0;
  for (i = 0; i < nrow * ncol; i++) 
    sum += values_image1[i] * values_image2[i];

  return(sum);
}


/*
 * Threshold an image 
 */
void ThreshImage(IMAGE input,IMAGE output,int flagint,int flagMin,float min,int flagMax,float max)
{ 
    int nrow,ncol;
    float minim=255.0;
     float maxim=0.0;
    float coef;
    int i,size;
    float *invalues;
    float *outvalues;
 
    nrow = input->nrow;
     ncol = input->ncol;
     size=nrow*ncol;
      invalues = input->pixels;
    
     if ((nrow != output->nrow) | (ncol != output->ncol)) SizeImage(output, ncol, nrow);
     outvalues = output->pixels;
  
     for(i=0;i<size;i++) {   
       coef=invalues[i];
       if(coef>maxim) maxim=coef;
 	   if(coef<minim) minim=coef;
     }
    

     
     if (flagint) {
      if (flagMin) minim = (float)((int) min);
      if (flagMax) maxim = (float)((int) max);    
       for(i=0;i<size;i++) { 
         coef=invalues[i];
         if(coef>maxim) coef=maxim;
	     if(coef<minim) coef= minim;
	     outvalues[i]=(float)((int)coef);
       }
     }
     else {
      if (flagMin) minim =  min;
      if (flagMax) maxim =  max;    
       for(i=0;i<size;i++) { 
         coef=invalues[i];
         if(coef>maxim) coef = maxim;
         if(coef<minim) coef = minim;
	     outvalues[i] = coef;
       }
     }
}


/* The correspnding command */
void C_ThreshImage(char ** argv)

{
    IMAGE image;
    float min;
    float max;
    char *arg1,*arg2;
     char opt;
    int flagint,flagMin,flagMax;

    argv= ParseArgv (argv,tIMAGEI,&image,-1);

   argv = ParseArgv(argv,tSTR,&arg1,tSTR,&arg2,-1);
   if (!strcmp(arg1,"*")) flagMin = NO;
     else {
       flagMin = YES;
       ParseFloat(arg1,&min);
     }
     if (!strcmp(arg2,"*")) flagMax = NO;
     else {
       flagMax = YES;
       ParseFloat(arg2,&max);
     }
 
   
   flagint = NO;
    while(opt = ParseOption(&argv)) { 
   switch(opt) {
   case 'i': 
     flagint = YES;
    break;
   default:
     ErrorOption(opt);
     }
    }
 NoMoreArgs(argv);

   if (flagMin == NO && flagMax == NO) Errorf("You have to specify either the Min or the Max");
  
  ThreshImage(image,image,flagint,flagMin,min,flagMax,max);
}



/* The image command */
void C_Image (char **argv)
{
  char * name;
  IMAGE image,newimage;
  int i,j;
  float min,max;
  SIGNAL signal;
  char direction;
  int index;
  
  char * fieldName;

   argv = ParseArgv(argv,tSTR,&fieldName,-1);

  if (!strcmp(fieldName,"invert")) {
     argv = ParseArgv(argv,tIMAGE,&image);
     MinMaxImage(image,NULL,NULL,&min,NULL,NULL,&max);
     for(i=0; i < image->nrow; i++)
       for (j=0; j < image->ncol; j++)
      	  image->pixels[i * image->ncol + j] =max - image->pixels[i * image->ncol + j];
  } 
  
  else { 
    Errorf("Bad action '%s'\n",fieldName); 
  }
}

