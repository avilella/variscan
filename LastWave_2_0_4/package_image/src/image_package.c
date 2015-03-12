/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0.4                      */
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

/*
 * The file that contains CProcs for Image 
 *
 */
/* In Image Alloc */
extern void  C_Image(),  C_InfoImage(), C_StatsImage(), C_CopyImage(), C_SetImage(); 
extern void  C_ReadImage(), C_WriteImage(),C_Matrix();
extern void C_ThreshImage();

static CProc imageCommands[] = {

    "image",C_Image,"{{{invert <image>} {Inverts the values of all the pixels of an image (i.e., the min becomes the max and vice versa).}}}",
   "istats",C_StatsImage,"{{{mean <imageIn>} {Computes the mean of an image.}} \
{{var <imageIn>} {Computes the variance of an image.}} \
{{skew <imageIn>} {Computes the skewness of an image.}} \
{{kurt <imageIn>} {Computes the kurtosis of an image.}} \
{{nth <imageIn> <n> [-Ca]}  {Computes the (NON centered) <n>th moment of an image. If '-C' then the moment is centered. If '-a' the absolute moment is computed (<n> can be a float).}} \
{{minmax <imageIn> [-c]} {Computes the minimum and the maximum values of an image and returns, in a listv, the corresponding indexes <iMin> <jMin> and <iMax> <jMax>.}} \
{{lp <imageIn> <p> [-c]} {Computes the Lp norm of an image.}} \
{{print <imageIn>} {Prints some statistical information about an image.}}}",
    "ithresh", C_ThreshImage, "{{{Not to be used} {Old LastWave Command}}}",
    
    /* I/O image */
    "iread",C_ReadImage,"{{{<image> (<filename> | <stream>) [-h <nrow> <ncol>] [-c]} \
{Reads an image from disk. By default, the file should have a header and should be binary floats. If '-h' then\
there is no header and the image is supposed to have the size <nrow> <ncol>. If '-c' then the image is coded using \
characters (each value is in between 0 and 255).}}}",
    "iwrite",C_WriteImage,"{{{<image> (<filename> | <stream>) [-hc] [-r [<min> <max>]} \
{Write an image on the disk. By default, it will write a header and binary floats. If '-c' then characters are \
written (each value is in between 0 and 255). If '-h' then no header is written. If '-r' then the image is \
rescaled between 0 (<min>) and 255 (<max>). if <min> and <max> are not specified then the minimum and \
maximum of the image are used.}}}",

    /* matrix */
    "matrix",C_Matrix,"{{{trace <imageIn>} {Computes the trace of a matrix.}} \
{{det <imageIn>} {Computes the determinat of a matrix.}} \
{{diagsym <imageIn>} {Diagonalizes a symetric matrix. Returns a listv made of a matrix (corresponding to the eigen vectors) \
and a signal (corresponding to the eigen values).}}}",

   NULL,NULL,NULL
};

static CProcTable  imageTable = {imageCommands, "image", "Commands related to images"};

/***********************************
 * 
 * Loading/Adding the imagel package
 *
 ***********************************/

int tIMAGE, tIMAGE_;
int tIMAGEI, tIMAGEI_;
extern TypeStruct tsImage;


static void LoadImagePackage(void) 
{ 
  /*
   * The &imagei type MUST be defined before the &image type, so that when looking for
   *  matching a type to a variable, we first try to match &imagei
   */ 
  tIMAGEI = AddVariableTypeValue(imageiType, &tsImage, (char (*) (LEVEL, char *, void *, void **)) ParseImageILevel_);
  tIMAGEI_ = tIMAGEI+1;

  tIMAGE = AddVariableTypeValue(imageType, &tsImage, (char (*) (LEVEL, char *, void *, void **)) ParseImageLevel_);
  tIMAGE_ = tIMAGE+1;  
   
  AddCProcTable(&imageTable);
 
  DefineGraphImage();
} 

void DeclareImagePackage(void)
{
  DeclarePackage("image",LoadImagePackage,1998,"2.0.4","E.Bacry and J.Fraleu",
"Package allowing to deal with images/matrices.");
  
}

 
