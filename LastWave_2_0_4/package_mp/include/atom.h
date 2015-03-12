/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval                                 */
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



#ifndef MP_ATOM_H

#define MP_ATOM_H

#include "stft.h"

#define ATOM_ADVANCED

/***************************************************************************************************************/
/*
 *  The main parameters of an ATOM : 
 *  -a normalized window g(t)                 (windowShape,windowSize)
 *  -a time-frequency localization            (timeId,freqId)
 *  -a chirp parameter                        (chirpId)
 *
 *  The 'coefficients'
 *  -a complex coefficient                    (coeffR,coeffI)
 *   corresponds to the inner product between a real valued signal 
 *   and the normalized complex valued atom
 *    
 *   g(t-timeId) * exp(i*2*PI*(freqId*(t-timeId)/GABOR_MAX_FREQID+(chirpId/2)*(t-timeId)^2/GABOR_MAX_FREQID))
 *
 *  -the inner product product                (realGG,imagGG)
 *   between the normalized complex valued atom and its complex conjugate.
 *   It depends on g(t), freqId and chirpId, and we indicate that it is properly set
 *   with flagGGIsSet.
 *
 *  -a real (squared) coefficient and a phase (coeff2,cosPhase,sinPhase)
 *   are generally obtained from a complex coefficient
 *
 *  K*g(t-timeId) * cos(2*PI*(phi+freqId*(t-timeId)/GABOR_MAX_FREQID+(chirpId/2)*(t-timeId)^2/GABOR_MAX_FREQID))
 *    
 */
/**********************************************/
typedef struct atom {
  /* The fields of the ATFCONTENT structure */
  ATFContentFields;

  // The window : shape and windowSize
  char windowShape;
  unsigned long windowSize;	// scale  s = (windowSize-1)*dx
  // Time-frequency localization
  float timeId;	// 0 <= timeId <  signalSize
  float freqId;	// 0 <= freqId <= GABOR_NYQUIST_FREQID
  float chirpId;//   |chirpId| <= GABOR_MAX_CHIRPID

  // Real and imaginary part of the complex coefficient (COMPLEX atom)
  float coeffR;
  float coeffI;

  // Inner-product between the unitary complex atom and its conjugate
  float realGG;	// The real part of <g,_g>
  float imagGG;	// The imaginary part of <g,_g>
  char flagGGIsSet;

  // Squared amplitude and phase (REAL atom)
  float coeff2;
  float cosPhase,sinPhase; // exp 2*pi*phase

} Atom,*ATOM;

// 
// The basic variables and functions for &maximadict variable management 
//
extern char 	 *atomType;
extern TypeStruct tsAtom;
extern int 	tATOM_;
extern int 	tATOM;

extern ATOM 	NewAtom(void);
extern ATOM 	TNewAtom(void);
extern ATOM 	DeleteAtom(ATOM atom);
extern void 	ClearAtom(ATOM atom);
extern ATOM 	CopyAtom(const ATOM in,ATOM out);
extern void 	CheckAtom(const ATOM atom);
extern void 	CheckAtomReal(const ATOM atom);


/* ATOM I/O */
extern void 	PrintAtom(const ATOM atom,char flagShort);

extern char     AtomsIntersect(const ATOM atom1,const ATOM atom2,long *pTimeIdMin,long *pTimeIdMax);

extern void 	BuildAtom(const ATOM atom,SIGNAL atomR,SIGNAL atomI,char borderType,char flagAtomSizeOnly);

/*************************************************************************************/
/*
 * The inner product between a signal and a (complex) MONOCHANNEL atom
 * (if not monochannel, an error is generated).
 * The values of coeffR/coeffI/coeff2/realGG/imagGG/phase/cosPhase/sinPhase
 * of the atom do not influence the result.
 */
// If the frequency is either 0 or Nyquist the atom takes indeed real
// values and we garantee that the imaginary part of the inner product is zero.
/****************************************************/
extern void AutoAtomInnerProduct(const ATOM atom,char flagForceNumeric,float *pReal,float *pImag);
extern void SCAtomInnerProduct(const SIGNAL signal,const ATOM atom,char borderType,float *coeffR,float *coeffI);

// Conversion between complex and real type
extern void CastAtomReal(ATOM atom);

#endif

/* EOF */

