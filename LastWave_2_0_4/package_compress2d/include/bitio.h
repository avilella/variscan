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

#ifndef BITIO_H
#define BITIO_H


/*
 * Structure to send bits to a stream
 */
 		
typedef struct bitOut {

    unsigned char bitBuf;
    int bitsInBuf;
    int nBytes;
    FILE  *output;
    FILE *bitLog;
    
} BitOut, *BITOUT;

/*
 * Structure to get bits from a stream
 */

typedef struct bitIn {

    unsigned char bitBuf;
    int bitsInBuf;
    int bitCount;
    FILE *input;    
    FILE *bitLog;

} BitIn, *BITIN;



extern BITOUT NewBitOut(FILE *out, FILE *log); 
extern void DeleteBitOut(BITOUT b);
extern int  BitCountBitOut(BITOUT b); 
extern void PushBitOut(BITOUT b, int bit);
extern void FlushBitOut(BITOUT b);
extern BITIN NewBitIn(FILE *is, FILE *log); 
extern void DeleteBitIn(BITIN b);
extern int BitCountBitIn(BITIN b) ;
extern int PullBitIn(BITIN b);

#endif
