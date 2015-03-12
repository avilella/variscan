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
/*  wtrans.c         Functions which deal with the wtrans package           */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "wtrans1d.h"




/***********************************
 * 
 *          CProcs related to wtrans
 *
 ***********************************/


/* In wtrans_alloc.c */
extern void C_Wtrans(),C_SetWtrans();

/* In wtrans_functions.c */
extern void  C_WThresh();

/* In wtrans_file.c */
extern void C_WWrite(), C_WRead();

static CProc wtransCommands[] = {
 
  /*
   * CProcs in wtrans_alloc.c 
   */
  "setwtrans", C_SetWtrans,"{{{Old version procedure} {Not to be used}}}",
  
  /*
   * CProcs in wtrans_functions.c
   */  
  "wthresh", C_WThresh,NULL,

  /*
   * CProcs in wtrans_file.c 
   */
/*   "wwrite",C_WWrite,"{{{[<wtrans>=objCur] (<file> | <stream>)} {Writes a wavelet transform in a <file> or in \ */
/* a <stream> (in binary mode).}}}", */
  "wwrite",C_WWrite,"{{{[<wtrans>=objCur] (<file> | <stream>) [-a]} {Writes a wavelet transform in a <file> or in \
a <stream> (in binary mode, or in ascii mode if -a).}}}",
  "wread",C_WRead,"{{{[<wtrans>=objCur] (<file> | <stream>)} {Reads a wavelet transform from a <file> or a <stream>.}}}",

  NULL,NULL,NULL
};

CProcTable  wtransTable = {wtransCommands, "wtrans1d", "Commands for dealing with Wavelet transform structures"};

/***********************************
 * 
 *       The wavelet decomposition Table
 *
 ***********************************/

/* In cwt1d.c */
extern void C_CWtd(),C_Wt1dNoctMax(char **argv);

/* In odecomp.c */
extern void  C_OWtd(),C_OWtr();

/* In ddecomp.c */
extern void C_DWtd(),C_DWtr();

/* In filter_bior.c */
extern void  C_OWtf();

/* In filter_dyad.c */
extern void C_DWtf();

/* In filter_alloc.c */
extern void C_PrintFG();
extern void C_QuickLib();

static CProc wavelet1dCommands[] = {


  /* 
   * CProcs in cwt1d.c
   */

  "cwtd",C_CWtd,"{{{[<wtrans>=objCur] <aMin> <nOct> <nVoice> "
  "[<wavelet>=g2] [-b <border> = mir] [-c] [-m] [-e <expo> = -1]} "
  "{Performs the continuous wavelet transform of the signal A[0][0] in "
  "<wtrans> from scale <aMin> using <nOct> octaves, <nVoice> voices per "
  "octave, using the nth derivative of the Gaussian function (<wavelet>=g<n> where n is in [0..4] are such that \
g0, -g1, g2, -g3, g4 are the gaussian function and its successive derivatives) "
  "as analyzing wavelet or the morlet wavelet (which is a complex wavelet). In the case of a complex analyzing wavelet, \
the modulus of the wavelet transform is stored in the D signals (e.g., .10 .20 .33) and the phase (between 0 and 2pi) in \
the A signals (e.g., 10 20 30). \
The command returns the elapsed time in seconds. The first scale <aMin> should not be smaller than 1. There are 4 ways to deal with border effects "
  "('per'=periodic, 'mir'=mirror, 'pad0'=padding with 0 values, 'pad'=padding with constant extremity values so that the signal is continuous). \n"
  " -c: causal mode, only used when -m is used. The extrema representation is"
  "computed using the causal mode.\n"
  " -m: memory optimization. Set this option if you directly want to compute the extrema representation "
  "without storing the wavelet transform (for memory optimization).}}}",

  "cwtdoctmax",C_Wt1dNoctMax,"{{{<signalSize> <aMin> <nVoice> <wavelet>} "
  "{It returns the largest octave number that can be used using the 'cwtd' command with the same arguments.}}}",

  /*
   * CProcs in odecomp.c 
   */
  "owtd", C_OWtd,"{{{[<wtrans>=objCur] <nOct>} {Performs (bi)orthogonal wavelet transform of the signal A[0][0] in <wtrans> using <nOct> octaves.}}}",
  "owtr",C_OWtr,"{{{[<wtrans>=objCur]  [<signalOut>=0]} {Performs (bi)orthogonal wavelet reconstruction of the wavelet transform <wtrans> in signal <signalOut>.}}}",
  
  /*
   * CProcs in ddecomp.c 
   */
/*  "dwtr",C_DWtd,"[<wtrans>=objCur] {<noct> | -r}","Dyadic decomposition",
  "dwtr",C_DWtr,"[<wtrans>=objCur] <sigOut>","Dyadic reconstruction",
*/

  /*
   * CProcs in filter_bior.c
   */  
  "owtf",C_OWtf,"{{{[<wtrans>=objCur] <filterFilename>} {Sets a filter for [bi]orthogonal decomposition. Filter files are in scripts/wtrans1d/filters directory. Extension '.o' \
stands for orthogonal filters and '.b' for biorthogonal filters.}}}",

  /*
   * CProcs in filter_dyad.c
   */  
/*  "dwtf",C_DWtf,"[<wtrans>=objCur] <filter_name>","Set a filter for dyadic decomposition",
*/

  /*
   * CProcs in filter_alloc.c
   */  
/*  "pfilter",C_PrintFG,"{{{[<wtrans>=objCur] [H1 | H2 | G1 | G2]} {Prints (bi)orthogonal H1,H2,G1,G2 filter values.}}}",
*/
  NULL,NULL,NULL  
};

static CProcTable  waveletTable = {wavelet1dCommands, "wtrans1d", "Wavelet transform commands"};



/***********************************
 * 
 * Loading/Adding the wtrans package
 *
 ***********************************/

int tWTRANS, tWTRANS_;
extern TypeStruct tsWtrans;

static void LoadWtrans1dPackage(void)
{
  tWTRANS = AddVariableTypeValue(wtransType, &tsWtrans, NULL);
  tWTRANS_ = tWTRANS+1;
  AddCProcTable(&wtransTable);
  AddCProcTable(&waveletTable);
  DefineGraphWtrans();  
}

void DeclareWtrans1dPackage(void)
{
  DeclarePackage("wtrans1d",LoadWtrans1dPackage,1998,"2.0","B.Audit, E.Bacry, N.Decoster, S.Mallat and J.F.Muzy",
  "Package allowing to perform 1d (continuous and (bi)orthogonal) wavelet transform.");
}
