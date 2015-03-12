/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0.4                     */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry.                             */
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
/*  signal.c         Functions which deal with the signal package           */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "signals.h"



/***********************************
 * 
 *          CProcs related to signals
 *
 ***********************************/


/* In signal_functions.c */
extern void C_Thresh(), C_Sort();
extern void C_Padd(), C_Stats(),C_Histo(), C_OldConv(), C_Conv(), C_CorrSig();

  
/* In signal_file.c */
extern void C_Write(),C_Read(),C_ReadInfo();

/* In signal_create.c */
extern void C_Cantor(),C_UCantor();

/* In fft_commands.c */
extern void C_Fft();


static CProc signalCommands[] = {
  
  /*
   * CProcs in signal_functions.c 
   */  
  "thresh",C_Thresh,"{{{<signalIn> <signalOut> -(x | y)  (<min> | *) (<max> | *))} \
{Thresholds the <signalIn> and sets the result in <signalOut>. All the values between <min> and <max> are set to 0. \n\
  - If <min> is '*' then <min> is chosen as the minimum of <signalIn> \n\
  - If <max> is '*' then <max> is chosen as the maximum of <signalIn> \n\
  - If '-y' (resp '-x') then the thresholding affects the value of the Y (resp. X) array of <signalIn>.}}}",
  "sort",C_Sort,"{{<signalIn> {Sorts the values of <signalIn> according to X (if 'xysig') or Y (if 'ysig')}}}",
  "padd",C_Padd,"{{{<signalIn> [<signalOut>=<signalIn>] [*border*= (*bconst | *b0 | *bmirror | *bperiodic)] [-s <newSize>]} \
{Padds a signal so that it ends having <newSize> points (if '-s' specified) or the closest size which is greater than the actual size and which is a power \
2 (if '-s' not specified). The padding values are specified by the argument <border>. The resulting signal is stored in <signalOut> (by default it is stored in <signalIn>).}}}",
  "stats",C_Stats,"{{{mean <signalIn> [-c]} {Computes the mean of a signal ('-c' is the \"causal\" flag : if set, it means that the \
mean is computed only from the indexes 'firstp' to 'lastp'of <signalIn>).}} \
{{var <signalIn> [-c]} {Computes the variance of a signal ('-c' : same as in 'mean' command).}} \
{{skew <signalIn> [-c]} {Computes the skewness of a signal ('-c' : same as in 'mean' command).}} \
{{kurt <signalIn> [-c]} {Computes the kurtosis of a signal ('-c' : same as in 'mean' command).}} \
{{nth <signalIn> <n> [-cCa]} {Computes the (NON centered) <n>th moment of a signal ('-c' : same as in 'mean' command). If '-C' then the moment is centered. If '-a' the absolute moment is computed (<n> can be a float).}} \
{{minmax <signalIn> [-c]} {Computes the minimum and the maximum values of a signal and sends back the corresponding indexes in a listv {<imin> <imax>} ('-c' : same as in 'mean' command).}} \
{{lp <signalIn> <p> [-c]} {Computes the Lp norm of a signal ('-c' : same as in 'mean' command).}} \
{{corr <signalIn> [<signalIn1>] [-c]} {Computes the correlation function between the x array and the y array of <signalIn> or \
(if <signalIn1> is specified) between the y arrays of <signalIn> and <signalIn1>  ('-c' : same as in 'mean' command).}} \
{{print <signalIn> [-c]} {Prints some statistical information about a signal ('-c' : same as in 'mean' command).}} \
{{fit <signalIn> [-x <xMin> <xMax>]} {Computes a linear fit y = ax+b of <signalIn> between abscissa <xMin> and <xMax>. Returns the listv {a sigma_a b sigma_b iMin iMax}.}}}",
  "histo",C_Histo,"{{{<signalIn> <signalOut> <n> [-x <xMin> <xMax>] "
  "[-y <yMin> <yMax>] [-w <signalWeight>] [-c]} {Computes in <signalOut> the <n> "
  "branch histogram of the values of <signalIn>. \n"
  " -x : Only the values between the abscissa <xMin> and <xMax> are taken "
  "into account.\n"
  " -y : Only the values between the ordinate <yMin> and <yMax> are taken "
  "into account. \n"
  "      So the histogram will be made of <n> bars between <ymin> and <ymax>.\n"
  " -c : Only points corresponding to indexes between firstp and lastp are "
  "taken into account. \n"
  " -w : Each value of <signalIn> is associated (for computing the histogram) "
  "to a given weight.}}}",
/*  "oldconv",C_OldConv,"{{{<signalIn> <filter> <signalOut> [<border>=pad] [-d]} \
{Computes the convolution of a signal with a filter and puts it in <signalOut>. You can specify the border effects used \
for the <signalIn> by specifying the <border> ('per'=periodic, 'mir'=mirror, 'pad0'=padding with 0 values, \
'pad'=padding with constant extremity values). \
If option '-d' is set then it forces the convolution to be computed explicitly in the direct space (no fft involved). If \
it is not set, then it uses the fastest method (between fast convolution algorithm using fft or direct method). The firstp \
and lastp fields of the <signalOut> are updated so that it points to the first and last points not affected by the border effects. \
It returns the elapsed time in seconds.}}}",
*/
"conv", C_Conv ,"{{{<signal> <filter> <signalout> <border_effect> [-f] [-x [<xmin> <xmax>]]} \
{Computes the convolution of <signal> by the compact support <filter>. \
Border effect of <signal> can be chosen among 'b0', 'bconst', 'bperiodic', 'bmirror' or 'bmirror1' (same effect as for *option extractions of signals). \
The fields firstp and lastp are updated (they allow you to keep track of the points affected by border effects). \
If the flag -f is not set, the convolution is computed directly (no FFT) otherwise it is computed using an FFT. \
If the flag -x is not set, the abscissa interval on which the result is computed is (i) the abscissa interval of <signal> (for 'bperiodic') \
(ii) double the abscissa interval of <signal> (for 'bmirror' and 'bmirror1') (iii) the abscissa interval + the filter border effect \
(for 'b0' and 'bconst'). \
The flag -x allows to fix the abscissa interval <xmin> <xmax> of the result. Default values for <xmin> <xmax> \
are the abscissa interval of <signal>. The command retursn the time ellapsed (in seconds).}}}",

  "corr", C_CorrSig,"{{{<signalIn1> <signalIn2> <signalCorrelation> <dxmin> <dxmax> [<nSignal>] [-c] [-n]} {Computes the correlation function between the \
realization <signalIn1> of a first stochastic process and the ralization <signalIn2> of a second stochastic process. The output \
is set in <signalCorrelation>. The correlation function is computed with a lag going from <dxmin> to \
<dxmax> (the dx fields of the realizations are taken into account). These values represent the origin of the second signal compared to the origin of the first signal \
(which is fixed). \
By default, the border effects are not used for the computations unless '-c' is specified. The result is divided by the product of the square \
mean roots of the variances of <signalIn1> and <signalIn2> unless '-n' is specified. if <nSignal> is specified, it is used as an output signal to store, for each lag \
the number of terms that was used to compute the average}}}",


    
  /*
   * CProcs in signal_file.c 
   */
  "write",C_Write,"{{{<signalIn> (<filename> | <stream>) [('xy' | 'yx' | 'x' | 'y']) [-h] [-b] [-r [('little' | 'big')]]} {Writes a signal into a <file> or a <stream>. \
It writes the file either in a raw format (using '-r' option) or (by default) in the LastWave format with ascii coding (unless '-b' is specified in which case binary codes are used). \n\
In the LastWave format, there are different modes :\n\
  - 'xy' : First column is X and second is Y (in ascii mode), or the X-values are stored first then the Y-values (in binary mode).\n\
  - 'yx' : First column is Y and second is X (in ascii mode), or the Y-values are stored first then the X-values (in binary mode). \n\
  - 'y'  : A single column made of Y \n\
  - 'x'  : A single column made of X \n\
By default it uses 'xy' mode for xy-signals and 'y' mode for y-signals. The options are : \n\
  -h : No Header (ONLY in ascii mode).\n\
  -b : Binary writing (instead of ascii). \n\
  -r : The data (just the y\\'s) are written in raw format (binary floats) with no header. In that case you can specify whether you want to write them as little endian \
data or big endian data. If nothing is specified the endianness of the computer is used.}}}",
  "read",C_Read,"{{{<signalOut> (<filename> | <stream>) [[<xCol>] <yCol>] [-f <firstPoint>] [-s <sizeToRead>] [-r [('little' | 'big')]] [-b]}  {Reads a signal from a \
<file> or a <stream>. The file must be in the LastWave format (i.e., created with 'write') or in the raw format (option '-r'). If option '-r' is not set then you can specify the column number for the x-values \
and the column number for the y-values (first column is 1). If none are specified then this command tries to read the first two columns \
as x and y or, if there is only one colum, directly the y values. If a stream is specicified and if option '-r' is not set then, at the end of the command the stream is positionned at the end of the signal (even if just a few points are read). The options are \n\
  -f : It reads the signal starting from index <firstPoint> (first point is at index 0) \n\
  -s : It reads only <sizeToRead> values\n\
  -r : The data (just the y\\'s) are in raw format (binary floats) with no header. In that case you can specify whether you want to read them as little endian \
data or big endian data. If nothing is specified the endianness of the computer is used.\n\
  -b : Just here for compatibility with Signal Package 1.1.2. (and earlier) versions.}}}",
  "readinfo",C_ReadInfo,"{{{(<filename> | <stream>) [-p]}  {This command is used in order to get information about a signal file or stream that will be read (later) by the 'read' comand. \
If '-p' is set then information on the file or stream is printed in a in fully explained english. If it is not set then either it does not return anything \
(which means that the file is not readable) or it returns a listv which gives some information. The first element of the listv is 0 \
if there is no header or 1 there is one. If the file has no header then the remaining is of the form \
'nColumns' 'size'. If it has one the next element describes the way the values are coded : it is either 'ascii', \
'binary little' or 'binary big'. The next element is the type of the signal (either 'x' or 'xy'). If the signal is of type 'y' then the next two elements \
are the signal fields 'x0' and 'dx'. Finally the last two elements are the sigal fields 'firstp and lastp'.}}}",

  /*
   * CProcs in signal_create.c 
   */
  "cantor",C_Cantor,"{{{<signalOut> <depth> <s1> <s2> <s3> <p1> <p3>} {Generates a 1-->3 cantor (with a hole in the middle) given a number of \
iterations <depth> the relative sizes \
<s1>, <s2> and <s3> (must be integers!) of each 3 interval and the weights <p1> and <p3> of the first interval and the last one. This algorithm \
stops after <depth> iteration. It is not adapted to the local resolution (as for the command 'ucantor')}}}",
  "ucantor",C_UCantor,"{{{<signalOut> <size> <r1> <p1> <r2> <p2> [... <rN> <pN>]} {Generates a 1->N cantor of size <size> down to the smallest resolution \
(the smaller the interval the greater the number of iterations). If you want not adapt the number of iterations to the local resolution use \
the 'cantor' command instead. The <ri>'s correspond to the relative size of each interval (the sum must be equal to 1) \
and the <pi>'s correspond to their respective weights (the sum must be equal to 1).}}}", 

  /* 
   * CProcs in fft_commands.c
   */
  "fft",C_Fft,"{{{<signalInReal> [<signalInImag>] <signalOutReal> <signalOutImag> [-is]} {Computes the Fourier transform \
or the inverse Fourier transform (if '-i' is set) of a signal (complex or real) which must has a size which is a power of 2. \
By default, the fourier transform of a complex signal \
is supported  by [-Fs/2,Fs/2[ where Fs = 1/signal->dx is the sample frequency (in Hertz) and has the same number of points \
as the original signal. If '-s' is set then no shift is performed \
after the fft algorithm has been performed, thus the resulting fourier transform is represented between [0,Fs[. \
Consequently, for the inverse \
fourier transform if '-s' is not set then the input fourier transform is supposed to be between [-Fs/2,Fs/2[ and otherwise \
it should be between [0,Fs[. In the case you  want to do the fourier transform of real signal, you should ommit \
<signalInImag> in the command line. The computed fourier transform \
is supported by [0,Fs]. It has one more point than the original signal. If you want to invert the so obtained fourier \
transform, you should ommit <signalOutImag> in the command line and use the '-i' option. The so-obtained real signal will \
be placed in <signalOutReal>. Let us note that in the case of a real input signal, the option '-s' is useless. \
In any case, 'fft' returns the elapsed time in seconds.}}}",

	
  NULL,NULL,NULL
};

static CProcTable  signalTable = {signalCommands, "signal", "Commands related to signals"};



/***********************************
 * 
 * Loading/Adding the signal package
 *
 ***********************************/

int tSIGNAL, tSIGNAL_;
int tSIGNALI, tSIGNALI_;
extern TypeStruct tsSignal;

static void LoadSignalPackage(void)
{
  /*
   * The &signali type MUST be defined before the &signal type, so that when looking for
   *  matching a type to a variable, we first try to match &signali
   */ 
  tSIGNALI = AddVariableTypeValue(signaliType, &tsSignal, (char (*) (LEVEL, char *, void *, void **)) ParseSignalILevel_);
  tSIGNALI_ = tSIGNALI+1;

  tSIGNAL = AddVariableTypeValue(signalType, &tsSignal, (char (*) (LEVEL, char *, void *, void **)) ParseSignalLevel_);
  tSIGNAL_ = tSIGNAL+1;

  AddCProcTable(&signalTable);
  
  DefineGraphSignal();
}


void DeclareSignalPackage(void)
{
  DeclarePackage("signal",LoadSignalPackage,1998,"2.0.4","E.Bacry, N.Decoster and X.Suraud",
             "Package allowing to deal with signals.");
}
