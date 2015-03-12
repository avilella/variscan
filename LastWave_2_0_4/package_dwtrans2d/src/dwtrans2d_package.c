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



#include "lastwave.h"
#include "dwtrans2d.h"
  
/*
 * The file that contains CProcs for DWtrans2
 *
 */



/* In filter2.c */
extern void  C_DWt2f();

/*In wtrans2_alloc.c */
extern void C_PrintFilter2(), C_DW2Info(),C_DW2Read(),C_DW2Write();

/* In fast_ddecomp.c */
extern void  C_DDecomp2(), C_DRecons2();

static CProc Wtrans2Commands[] = {
 
     "dw2read", C_DW2Read,"{{{[<dwtrans2>=objCur] (<filename> | <stream>)} \
{Reads a wavelet transform from a file named <filename> or a <stream>.}}}",

    "dw2write", C_DW2Write,"{{{[<dwtrans2>=objCur] (<filename> | <stream>)} \
{Writes a wavelet transform in a file named <filename> or a <stream>.}}}",
   
    
    "dwt2f",C_DWt2f,"{{{<filterFileName>} \
{Specifies the filename of the filters that must be used in case a new dyadic \
wavelet decomposition is performed (using the 'dwt2d' command). \
The filter directory (defined in the file 'scripts/dwtrans2d/dwtrans2d.pkg') contains all the filter available files. \
The filenames ending by the '.1' suffixes correspond to decomposition filters and the ones ending with the '.2' suffixes \
correspond to reconstruction filters. For this command, you must specify the name without the suffix (it will load both \
the reconstruction and decomposition filters at the same time). \
For now, only one pair of decomposition/reconstruction filters are available? Its name is 'p3'. \
More details about filters can be found in the Appendix A \
of the paper : \n\
Characterization of signals from multiscale edges \n\
by Stephane Mallat and Sifen Zhong, \n\
IEEE Transactions on Pattern Analysis and  Machine  Intelligence,\n\
Vol. 14, No. 7, p. 710-732, July 1992.\n\
As described in the Appendix of the paper, filter  p3 corresponds to the H(w) defined in (97) with n = 1.}}}",

  "dwt2d",C_DDecomp2,"{{{[<dwtrans2>=objCur] <noct> [-N] [-m]} \
{Performs a dyadic wavelet decomposition on the current <dwtrans2> on <noct> octaves. \
The image to be analyzed is supposed to be in the image 0 of the <dwtrans2>. If \
the number of columns and  the number of rows are N = 2^p, then <noct> must range \
from 1 to p+1. When <oct> is equal to p+1, the coarsest is a constant. Options are :\n\
-N : Does not use the default periodization of the image.}}}",

  "dwt2r",C_DRecons2,"{{{[<dwtrans2>=objCur] [-N]} \
{Performs a dyadic wavelet reconstruction from a dyadic wavelet decomposition (made with the 'dwt2d' command). \
It reconstructs the image which corresponds to the dyadic wavelet transform stored in <dwtrans2>. \
It uses the same number of octaves as the decomposition has been  made with. The reconstructed image is put in image 0 of <dwtrans2>.}}}",

   NULL,NULL,NULL
};

CProcTable  wtrans2Table = {Wtrans2Commands,  "dwtrans2d","Commands related to wavelet 2D transform "};

extern void C_PInfo2();
extern void C_Point2Compute(),C_PointReprNormalize2(),C_PointReprDenormalize2();
extern void C_Point2Thresh(),C_Point2Print(),C_Point2ProPrint(),C_Point2Recons(), C_Point2Count();
extern void C_PointPicCopy(), C_P2Clear(), C_PointPic2Image(),  C_Image2PointPic();
extern void C_PointReprRead(), C_PointReprWrite(),C_ComputeGrad();

CProc Point2Commands[] = {

    "extrema2",C_Point2Compute,"{{{[<dwtrans2>=objCur] [-o] [-n] [-N]} \
 {Computes the extrema representation of the current wavelet transform. \
The  extrema are defined as the modulus local maxima along some gradient direction. The command returns the number of extrema found. \
Options are :\n\
-n : Due to the discretization of the filter values, the wavelet modulus maxima of a step edge do not have the same amplitude at all scales. In the extrema computation, we have correlated the extremas modulus such that the extremas of a step edge in each scale will have the same values. We called this process the 'normalization process'. With -n, we eliminate the normalization process in the computation of the extrema. \n\
-o : The extrema are defined as the modulus local maxima in any gradient direction. It is possible that, for example, the modulus at location (x,y) is bigger than the modulus at (x-1,y) but less than that at (x+1,y), however the gradient at location (x+1,y) is opposite to the gradient at (x,y). We would expect in this case that the intensity at the location (x,y) is a local maxima and include the (x,y) as a extrema location. With -o, we will eliminate this inclusion.Properties of the extrema are described in the paper : \n\
Remark : The periodization of the image is used if necessary.}}}",

    "e2image",C_PointPic2Image,"{{{[<dwtrans2>=objCur] <octaveNumber> <modImage> <phaseImage> [-p]} \
{Copies the modulus and phase of extrema at octave number <octaveNumber> to the images <modImage> for the modulus and <phaseImage> for the phase. If \
'-p' is set then the <modImage> is just made of 1 (to indicate an extrema) or 0.}}}",

    "e2thresh",C_Point2Thresh,"{{{[<dwtrans2>=objCur] <octaveNumber> <threshold>} {Thresholds the extrema at scale <octaveNumber> and whose modulus are smaller than \
<threshold>2^(<octaveNumber>/2)/3. If <octaveNumber> is 0 then all the scales are thresholded.}}}",

     "e2recons",C_Point2Recons,"{{{[<dwtrans2>=objCur] <iteration> [-N][-i][-n]} \
{Reconstructs the image from the extrema of the wavelet transform (computing using the 'extrema2' command). \
The algorithm reconstructs an approximation of the original image, by alternatively projecting between two affine spaces. \
The reconstructed image is in image 0 of <dwtrans2>. The parameter <iteration> gives the number of alternative projection \
desired for the reconstruction. 20 iterations are sufficient to  reconstruct an  approximation with an SNR larger than 25 db. \
Options are : \n\
 -i : Does not initialize the reconstruction algorithm. It begins the iterations from an image (in 0) obtained after <n> iterations \
previously performed and from the associated extrema. \
The resultant image is stored in 0 and corresponds to an approximation of the original image after <n>+<iteration> iterations.\n\
 -n :  Reconstruction without denormalization of the extrema modulus.}}}",
 /*-e <error file> : After each iteration, we compute at each scale the SNR of the errors between the original wavelet transform \
 and the current sequence of functions, stored them in  the  file <error file>. The original wavelet transform are copied \
 to wtrans2 d when the reconstruction begins.\n\ */

    "e2reconsgrad",C_ComputeGrad,"{{{[<dwtrans2>=objCur] <iterations> [<threshold>=0]} \
{Reconstruction from the wavelet maxima with the conjugate gradient algorithm. Before the reconstruction is performed, the extrema are thresholded using <threshold>.}}}",

    "e2write",C_PointReprWrite,"{{{[<dwtrans2>=objCur] (<file> | <stream>)} \
{Writes an extrema representation in a <file> or a <stream>}}}",
    "e2read",C_PointReprRead,"{{{[<dwtrans2>=objCur] (<file> | <stream>)} \
{Reads an extrema representation in a <file> or a <stream>}}}",

/*
    "imagee2",C_Image2PointPic,"{{{<mag_image> <arg_image> <level> [<orientation> =1] [<wtrans2> = objCur]} {Convert magnitude and argument representation to a extlis }}}",

    "e2clear",C_P2Clear,"{{{[<wtrans2> = objCur]} \
{e2clear clear all extremas with all pointers and chain included }}}",
    "e2count", C_Point2Count,"{{{[<wtrans2> = objCur]} \
{e2count counts the number of extremas at each level}}}",
    "e2copy",C_PointPicCopy,"{{{<wtrans2IN>  <wtrans2OUT>} \
{e2copy copy extremas with all pointers and chain included ???}}}",

    "e2info",C_PInfo2,"{{{[<wtrans2> = objCur] <level>} \
{Print the informatiom of a extlis}}}",
    "e2print",C_Point2Print,"{{{[<wtrans2> = objCur] <level> [-c <colmin> <colmax>] [-r <rowmin> <rowmax>]} \
{e2print prints the context of the extrema defined in the rectangle  [<colmin> <colmax>] * [<rowmin> <rowmax>] at the level <level>.}}}",
    "e2proprint",C_Point2ProPrint,"{{{[<wtrans2> = objCur] <level> [-c <colmin> <colmax>] [-r <rowmin> <rowmax>]} \
{Proprint prints all the extremas in a maxline with the coordinates of the extrema  in the finest scale of these maxlines within the region specified by -c <colmin> <colmax> and -r <rowmin> <rowmax>}}}",

     "e2norm",C_PointReprNormalize2,"{{{[<wtrans2> = objCur] [-N] } \
{e2norm normalizes the extrema modulus in the multiscale edge representation.\n\
Due to the discretization of the filter values, the  modulus maxima of a step edge is not the same at all scales. The command is therefore introduced to correct the modulus values of the extrema by multiplying coefficients, obtained from step edge and the filter, in such a way that after the normalization, all the modulus of the extrema for a step edge in each level are the same. It is especially useful in the Lipschitz exponent calculation, since we know that the Lipschitz exponent of a step edge is 0 in our system. In the command extrema2, this normalization process is automatically performed.}}}",
    "e2denorm",C_PointReprDenormalize2,"{{{[<wtrans2> = objCur] [-N]} \
{The command is an inversion to the command e2norm.\n\
Before reconstruction from the extrema representation, we suggest the user make sure that the denormalization of the extrema modulus is done.\n\
In the command e2recons, which reconstructs an approximation of the original image from its wavelet transform edge representation, the denormalization is performed automatically.}}}",

 /*------------------------*/   
   
   
   NULL,NULL,NULL
};

CProcTable  point2Table = {Point2Commands,  "dwtrans2d", "Commands related to ext used in wavelet 2D transform "};


extern void C_ChainCompute2(),C_ChainReprCount2();
extern void C_ChainReprThresh2(), C_ChainPicPrint2(), C_ChainPicPredictArg(),C_ChainPicBlurAmp(),C_ChainPicBlurAbs(), C_ChainPicSmoothArg(),C_C2Info();

CProc Chain2Commands[] = {

    "chain2",C_ChainCompute2,"{{{[<dwtrans2>=objCur]} \
{Links the maxima into curves referred to as 'chains'. \
It applies some smoothness constraints on both the gradient and modulus values along a chain. \
(i.e., close maxima are linked only if they have similar gradient and modulus.}}}",


/*
    "c2thresh",C_ChainReprThresh2,"{{{[<wtrans2> = objCur] <level> [-l <thresh_size>] [-a <thresh_mag>] [-p]} \
{c2thresh removes the chains at a level <level> according to its statistical properties.\n\
It removes the chain with average modulus no more than <thresh_mag> or with extrema no more than <thresh_size>. Options are \n\
 -a  <thresh_mag> :\n\
   remove the chains with average modulus no more than <thresh_mag>.\n\
 -l  <thresh_size> :\n\
   remove the chains with size no more than <thresh_size>.\n\
 -p :\n\
   remove all the extrema propagating to the chain been removed.}}}",
   
    "c2print",C_ChainPicPrint2,"{{{[<wtrans2> = objCur] <level> [-p <nb>] [-l <length>] [-a <am>]} \
{c2print prints the context of chain in the level <level>. Options are :\n\
 -a <am> :\n\
   c2print prints the chains with average modulus no less than amplitude <am>.\n\
 -l <length> :\n\
   c2print prints the chains with length no less than length <length>.\n\
 -p <nb> :\n\
   Without specifying this option, cprints prints out only the chain pic information at the level <level>. When specified, it prints the first <nb> of extrema in each chain of the chain_pic.}}}",
    "c2info",C_C2Info,"{{{[<wtrans2> = objCur] <level>} \
{Print the information of a chainlis at level <level>}}}"
,
    "c2count",C_ChainReprCount2,"{{{[<wtrans2> = objCur]} \
{c2count report the number of chains and the total extrema in the chains at each level}}}",

    "c2args", C_ChainPicPredictArg,"{{{[<wtrans2> = objCur] <level> [-e]} \
{c2args computes the arguments of the extrema in a chain from their tangents along the chain.\n\
   To test whether angle information is robust or not, we remove the argument in all the level and compute their derivative of chain and take its perpedicular direction as gradient and put it into argument in each corresponding level. Since the chain is only defined at integer positions, we first smooth the chain, and take derivative later. \n\
Option is :\n\
 -e : If -e is set only the first and the last points arguments are modified. }}}",
    "c2absblur",C_ChainPicBlurAbs,"{{{[<wtrans2> = objCur] <level> <sigma>} \
{c2absblur smoothes the extrema coordinates in a chain at the level <level> by convolving with a Gaussian kernel with standard deviation <sigma>}}}",
    "c2ampblur",C_ChainPicBlurAmp,"{{{{{[<wtrans2> = objCur] <level> <sigma>} \
{c2ampblur smoothes the modulus of extrema in a chain at the level <level> bu convolving with a Gaussian with standard deviation <sigma>}}}",
    "c2argsmooth",C_ChainPicSmoothArg,"{{{[<wtrans2> = objCur] <level>} \
{c2argsmooth smooth extrema arguments in a chain at level <level> by convolving with a filter with a filter with weight (1,4,6,4,1)}}}", 
*/
   NULL,NULL,NULL
};

CProcTable chain2Table = {Chain2Commands,"",  "Commands related to chain used in wavelet 2D transform"};

extern void C_lipschitz_noise_remove();

CProc NoiseDenoiseCommands[] = {

    "denoise2",C_lipschitz_noise_remove,"{{{[<dwtrans2>=objCur]} \
{Removes the white noise in images by thresholding extrema chains corresponding to particular values of Lipschitz exponents. \
Lipschitz exponents are calculated from the decay of the wavelet transform modulus local extrema along chains. \
The denoising process proceeds as follows : \n\
1. thresholds all the extrema in a chain whose Lipschitz exponent is smaller than a given threshold. \n\
2. thresholds all the chains at any level according to their lengths or average amplitudes. \n\
3. removes all the extrema not propagating to a given scale. \n\
4. rechains the chains such that all the chains in a fine scale corresponding to the same coarser chain \
are grouped together. We know that a contour has chains at many scales. Moreover, the white noise's energy \
distributes mostly in fine scales. Thus, the chains in fine scales are distorted more severely than those in coarse scales. \
This is the reason why we need to rechain the chains (in fine scales) by making use of the chains in a coarse scales. \n\
5. smoothes the abscissa and amplitude along the chains at some scales by a Gaussian kernel with a given standard deviation. \n\
6. predicts a chain's phases in fine scales from the abscissa of the chain and smoothes the resultant phases. \n\
The denoising process is interactive. It means that when you type in the 'denoise2' command, it is up to you to decide \
whether any of the step above will be proceeded. \
You will be asked questions you need to answer (if you type 'X' it will exit the denoising process, if you are asked for an \
integer, it will be printed 'int' moreover a default answer will be always suggest in between brackets. Just hit the 'return' key \
to select it).}}}",
   
   NULL,NULL,NULL
};

CProcTable noise2Table = {NoiseDenoiseCommands,  "dwtrans2d", "Commands related to the noise in pictures "};


/***********************************
 * 
 * Loading/Adding the dwtrans2d package
 *
 ***********************************/

int tWTRANS2, tWTRANS2_;

extern TypeStruct tsDWtrans2;


void LoadDWtrans2(void) 
{ 

  tWTRANS2 = AddVariableTypeValue(dwtrans2Type,&tsDWtrans2, NULL);
  tWTRANS2_ = tWTRANS2+1;
  
  AddCProcTable(&wtrans2Table);
  AddCProcTable(&point2Table);
  AddCProcTable(&chain2Table);
  AddCProcTable(&noise2Table);
  
  InitDFilter2();

} 

void DeclareDWtrans2dPackage(void)
{
  DeclarePackage("dwtrans2d",LoadDWtrans2,1998,"2.0","E.Bacry, J.Fraleu, J.Kalifa, E.LePennec, S.Mallat, W.L.Hwang, S.Zhong",
  "Package allowing to perform 2d dyadic wavelet transform decomposition, reconstruction and extrema reconstruction");
}

 
