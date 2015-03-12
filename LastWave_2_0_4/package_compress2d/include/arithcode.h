/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'compress2d' 2.0                   */
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


#ifndef ARITHCODE_H
#define ARITHCODE_H

/*
 * Coding Values
 */



#define CodingValuesFields long low, high, value
typedef struct codingValues {
    CodingValuesFields;
} CodingValues, *CODINGVALUES;		


/*
 * Arithmetic Encoder
 */

typedef struct arithEncoder {
    CodingValuesFields;
    BITOUT output;
    int bitsToFollow;
} ArithEncoder, *ARITHENCODER;


extern ARITHENCODER NewArithEncoder(BITOUT b); 
extern void DeleteArithEncoder(ARITHENCODER a); 
extern void EncodeArithEncoder(ARITHENCODER a, int count, int countLeft, int countTot);
extern void FlushArithEncoder(ARITHENCODER a);


/*
 * Arithmetic Decoder
 */

typedef struct arithDecoder {
  CodingValuesFields;
  BITIN input;
} ArithDecoder, *ARITHDECODER;

extern ARITHDECODER NewArithDecoder(BITIN b); 
extern void DeleteArithDecoder(ARITHDECODER a); 
extern int DecodeArithDecoder(ARITHDECODER a,IHISTOGRAM ih);



/*
 * Int code
 */

typedef struct cdeltaEncode {
    BITOUT output;
} CDeltaEncode, *CDELTAENCODE;

extern CDELTAENCODE NewCDeltaEncode(BITOUT bo);
extern void DeleteCDeltaEncode(CDELTAENCODE cd);
extern void EncodePositiveCDeltaEncode(CDELTAENCODE cd, int i);
extern void EncodeNonNegativeCDeltaEncode(CDELTAENCODE cd, int i);
extern void EncodeCDeltaEncode(CDELTAENCODE cd, int i);




typedef struct cdeltaDecode {
    BITIN input;
} CDeltaDecode, *CDELTADECODE;


extern CDELTADECODE NewCDeltaDecode(BITIN bi);
extern void DeleteCDeltaDecode(CDELTADECODE cd);
extern int DecodePositiveCDeltaDecode(CDELTADECODE cd);
extern int DecodeNonNegativeCDeltaDecode(CDELTADECODE cd);
extern int DecodeCDeltaDecode(CDELTADECODE cd);

#endif
