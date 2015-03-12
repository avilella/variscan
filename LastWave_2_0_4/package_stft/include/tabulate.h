/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry and J.Abadia           */
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



#ifndef STFT_TABULATE_H

#define STFT_TABULATE_H

//
// The various border types and some conversion utilities 
//

// In this file are defined the border types
#include "signals.h"

#define 	BorderTypeIsOK(type) (type == BorderPad0 || type == BorderPeriodic || type == BorderMirror)
extern char  	Name2BorderType(char *name);
extern char *	BorderType2Name(char borderType);
#define BorderTypeHelpString "('per'=periodic, 'mir'=mirror, 'pad0'=padding with 0 values)"

//
// The different possible atom windows and some conversion utilities.
//

enum {
  Spline0WindowShape,
  Spline1WindowShape,
  Spline2WindowShape,
  Spline3WindowShape,
  GaussWindowShape,
  HammingWindowShape,
  HanningWindowShape,
  BlackmanWindowShape,
  ExponentialWindowShape,
  FoFWindowShape,
  Asym3WindowShape,
  LastWindowShape
};

#define 	WindowShapeIsOK(windowShape)	(INRANGE(0,(windowShape),LastWindowShape-1)) 
extern char  Name2WindowShape(char *name);
extern char *WindowShape2Name(char windowShape);
extern void  GetWindowShapeFunc(char windowShape,float (**f)(SIGNAL,float));
#define WindowShapeHelpString "The available window shapes are : blackman, hanning, hamming, gauss, spline0 (rectangle), spline1, spline2, spline3, exponential or FoF."

// Global parameter of the Gaussian windows
extern double theGaussianSigma2;
// Global parameters for the FoF windows
extern float decayFoF;
extern float betaFoF;

// Getting some attributes of an atom window 
extern char     GetFlagAsymWindowShape(char windowShape);
extern int  GetMaxWindowShape(char windowShape,int size);


#endif

