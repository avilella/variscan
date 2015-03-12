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

#include "lastwave.h"
#include "bitio.h"


/*
 * BITOUT
 */
 
BITOUT NewBitOut(FILE *out, FILE *log) 
{
  BITOUT b;
  
  b = (BITOUT) Malloc(sizeof(BitOut));
  
  b->output = out;
  b->bitLog = log;

  b->bitsInBuf = 0;
  b->bitBuf = 0;

  b->nBytes = 0;
  
  return(b);
}

void DeleteBitOut(BITOUT b)
{
  Free(b);
}

int  BitCountBitOut(BITOUT b) 
{ 
  return(b->bitsInBuf + 8 * b->nBytes); 
}

void PushBitOut(BITOUT b, int bit)
{
  b->bitBuf = (b->bitBuf << 1) | bit;
  b->bitsInBuf++;

  if (b->bitsInBuf == 8) {
    fwrite(&(b->bitBuf),1,1,b->output);
	b->nBytes++;
	b->bitsInBuf = 0;
  }    
}

void FlushBitOut(BITOUT b)
{
    while (b->bitsInBuf) PushBitOut(b,0);
}


/*
 * BITIN
 */

BITIN NewBitIn(FILE *is, FILE *log) 
{
  BITIN b;
  
  b = (BITIN) Malloc(sizeof(BitIn));

  b->input = is;
  b->bitLog = log;
  b->bitsInBuf = 0;
  b->bitBuf = 0;
  b->bitCount = 0;
  
  return(b);
}

void DeleteBitIn(BITIN b)
{
  Free(b);
}

int BitCountBitIn(BITIN b) 
{ 
  return b->bitCount; 
}


int PullBitIn(BITIN b)
{
  int bit;
	
  if (!b->bitsInBuf) {
    b->bitsInBuf = 8;
	fread(&(b->bitBuf),1,1,b->input);
  }
  
  bit = (b->bitBuf & 128) >> 7;
  b->bitBuf = b->bitBuf << 1;
  b->bitsInBuf--;
  b->bitCount++;
	
  return bit;
}

