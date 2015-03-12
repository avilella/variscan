/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0.4                       */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry                        */
/*      email  : remi.gribonval@inria.fr                                    */
/*               lastwave@cmapx.polytechnique.fr                            */
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

//#define STFT_ADVANCED

/****************************************************************************/
/*                                                                          */
/*  stft_package.c   Functions which deal with the                          */
/*                  Short Time Fourier Transform package                    */
/*                                                                          */
/****************************************************************************/

#include "lastwave.h"

#include "stft.h"


/*****************************/
/*
 *	STFT commands 
 */
/*****************************/

// Basic 'stft' commands
extern void C_Stft(char **);
extern void C_Stftd(char **);
extern void C_GetStftMax(char **);
extern void C_GetWindowShape(char **);

#ifdef STFT_ADVANCED
extern void C_FilterMultiplyFft(char **);
extern void C_FilterConvol(char **);
extern void C_TestQRL(char **);
#endif

static CProc stftCommands[] = { 
#ifdef STFT_ADVANCED
    // Getting the window corresponding to a name and a size
 {"windowshape",C_GetWindowShape,
"{{{<signal> <size> [<shift>=0] [<windowShape>=gauss]} {Sets a signal with a given window of size <size> \
and whose window is <windowShape>. It is shifted by the given amount, which should remain within [0,1[. "
WindowShapeHelpString
" }}}"},
#endif

  // Stft variable manipulation
    {"stft",C_Stft,
"{{window <windowShape> <windowSize>} {Returns a signal which contains a copy of a window tabulated in the package 'stft'.}}" 
#ifdef STFT_ADVANCED
" {{exp} {Returns a listv {real imag} which is a copy of the complex exponentials tabulated in the package 'stft'.}} \
 {{gg     <windowShape> <windowSize>} {Returns a listv {real imag} which is a copy of the 'GG' tabulated in the  package 'stft'.}}"
#endif 
" {{+ <stftIn1> <stftIn2> [<stftOut>=<stftIn1>]} {Adds two stfts and puts the result in a new one.}} \
 {{- <stftIn1> <stftIn2> [<stftOut>=<stftIn1>]} {Substract two stfts and puts the result in a new one.}} \
 {{* <stftIn1> <stftIn2> [<stftOut>=<stftIn1>]} {Multiply two stfts and puts the result in a new one.}} \
 {{/ <stftIn1> <stftIn2> [<stftOut>=<stftIn1>]} {Divides two stfts and puts the result in a new one.}} \
 {{conjugate <stftIn> [<stftOut>=<stftIn>]} {Conjugates a (complex) stft and puts the result in a new one.}} \
 {{ln   <stftIn> [<stftOut>=<stftIn>]} {Takes the natural logarithm of a stft and puts the result in a new one. \
If the stft is complex, the result is complex with its imaginary part equal to the phase of the input.}} \
 {{log  <stftIn> [<stftOut>=<stftIn>]} {Takes the logarithm in base 10 of a stft and puts the result in a new one. \
If the stft is complex, the result is complex with its imaginary part equal to the phase of the input.}} \
 {{log2 <stftIn> [<stftOut>=<stftIn>]} {Takes the logarithm in base 2 of a stft and puts the result in a new one. \
If the stft is complex, the result is complex with its imaginary part equal to the phase of the input.}} \
 {{write <stft> (<stream>|<filename>) [-h]} {Writes a stft to a <file> in ascii format. With option '-h' no header is written.}}"},

    // Computation of Complex/Real/HighRes stfts from a signal
    {"stftd",C_Stftd,
"{{{[<stft>=objCur] <signal> [<windowShape>='gauss'] <windowSize> [(complex | real | phase)] [-b <borderType>] [-T <time redundancy>=4] [-F <freq redundancy>=2]} \
{Computes a Short Time Fourier Transform of the <signal>.\n\
 - The window size is <windowSize>, its shape is given by <windowShape>. "
WindowShapeHelpString
"\n"
"- You can specify the type of stft that should be computed : \n\
 * 'complex' corresponds to a regular short time fourier transform. \n\
 * 'real', 'phase' correspond respectively to the energy/phase of the best\
 matched real Gabor atoms (which is quite close to, but slightly different\
 from, the magnitude or phase of the complex spectrogram ...).\n"
" - The treatment of border effects is determined by the argument \
of option '-b'. "
BorderTypeHelpString
"\n- Options '-T' and '-F' determine the time and frequency redundancy factors, that is to say the *time-frequency grid* associated with the spectrogram. \
This means that :\n\
    *** a FFT is computed at each 1/<time redundancy>th of the window size.\n\
    *** the number of bins of each FFT is <freq redundancy>*<windowSize>/4+1 (the +1 is for the Nyquist frequency).}}}"},

      // Getting the location of the maximum of a stft
    {"stftmax",C_GetStftMax, 
     "{{{[<stft>=objCur] [-(t,T) <tMin> <tMax>] [-(f,F) <fMin> <fMax>] [-c]} {Gets the point where the maximum of energy of a stft is reached. It \
returns the list '<maxEnergy> <timeId> <freqId>', or 0 if <maxEnergy> is zero, -1 if the domain is empty. \
The options '-t' and '-T' allow to restrict the search \
to points for which the time is in between a given range <tMin> <tMax> which is specified using real time scale ('-t') or timeIds ('-T'). \
The options  '-f' and '-F' allow to restrict the search in the same way in the frequency domain. The option '-c' restricts \
the search to points not affected by border effects.}}}"},
 
    // Some functions for debug and advanced users
#ifdef STFT_ADVANCED
      {"qrl",C_TestQRL,"{{{<min> <max> <rate>} {}}}"},
 {"fmfft",C_FilterMultiplyFft,"{{{<signal> <window> <timeId> <fRate> <resultR> <resultI> [-b <border>]} {}}}"},
 {"fcv",C_FilterConvol,"{{{<signal> <octave> <freqId> <tRate> <resultR> <resultI> [-b <border>='pad0'] [-w <windowShape>='gauss']} {}}}"},
#endif

  {NULL,NULL,NULL}
};

static CProcTable stftTable = {stftCommands,"stft","Commands which deal with Short Time Fourier Transforms"};

/***********************************
* 
* Loading/Adding the STFT package
*
***********************************/
extern TypeStruct tsStft;

int tSTFT,   tSTFT_;

extern void InitStftTabulations(void);

static void LoadStftPackage(void)
{
    /* Adding stft VARIABLES */
    tSTFT = AddVariableTypeValue(stftType, &tsStft, NULL);
    tSTFT_ = tSTFT+1;

    /* Initializing global tabulations for the Stft package */
    InitStftTabulations();
    AddCProcTable(&stftTable);

    DefineGraphStft(); 
}

void DeclareStftPackage(void)
{
  DeclarePackage("stft",LoadStftPackage,2001,"2.0.4","R.Gribonval, E. Bacry and J.Abadia",
  "Package allowing the computation of Short Time Fourier Transforms and some derived time-frequency representations.");
}


/* EOF */

