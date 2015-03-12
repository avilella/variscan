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
#include "ihisto.h"
#include "arithcode.h"

/*
 * Coding Values
 */
 
#define CVCodeValueBits 16
long CVMaxFreq =  ((long)1 << (CVCodeValueBits - 2)) - 1;
long CVOne = ((long)1 << CVCodeValueBits) - 1;
long CVQtr = (((long)1 << CVCodeValueBits) - 1) / 4 + 1;
long CVHalf = 2 * ((((long)1 << CVCodeValueBits) - 1) / 4 + 1);
long CVThreeQtr = 3 * ((((long)1 << CVCodeValueBits) - 1) / 4 + 1);



/*
 * Arithmetic Encoder
 */

ARITHENCODER NewArithEncoder(BITOUT b) 
{
  ARITHENCODER a;
  
  a = (ARITHENCODER) Malloc(sizeof(ArithEncoder));

  a->output = b;
  a->low = 0;
  a->high = CVOne;
  a->bitsToFollow = 0;
  
  return(a);
}

void DeleteArithEncoder(ARITHENCODER a) 
{
  Free(a);
}

static void BpfArithEncoder(ARITHENCODER a, int bit)
{
  int i;
  
  PushBitOut(a->output,bit);
  for (i = 0; i < a->bitsToFollow; i++) PushBitOut(a->output,1-bit);
  a->bitsToFollow = 0;
}

void EncodeArithEncoder(ARITHENCODER a, int count, int countLeft, int countTot)
{
  long range = a->high - a->low + 1;
    
  a->high = a->low + (range * (countLeft + count)) / countTot - 1;
  a->low = a->low + (range * countLeft) / countTot;

  while (1) {
    if (a->high < CVHalf) BpfArithEncoder(a,0);
	else if (a->low >= CVHalf) {
	  BpfArithEncoder(a,1);
	  a->low -= CVHalf;
	  a->high -= CVHalf;
	} else if (a->low >= CVQtr && a->high < CVThreeQtr) {
	  a->bitsToFollow++;
	  a->low -= CVQtr;
	  a->high -= CVQtr;
	} else break;
	a->low = 2 * a->low;
	a->high = 2 * a->high + 1;
  }
}

void FlushArithEncoder(ARITHENCODER a)
{
  int i;
  
  for (i = 0; i < CVCodeValueBits; i++) {
	if (a->low >= CVHalf) {
	  BpfArithEncoder(a,1);
	  a->low -= CVHalf;
	} else BpfArithEncoder(a,0);
	a->low = 2 * a->low;
  }
  FlushBitOut(a->output);
}


/*
 * Arithmetic Decoder
 */

ARITHDECODER NewArithDecoder(BITIN b) 
{
  ARITHDECODER a;
  int i;
  
  a = (ARITHDECODER) Malloc(sizeof(ArithDecoder));

  a->input = b;
  a->low = 0;
  a->high = CVOne;
  a->value = 0;
  for (i = 0; i < CVCodeValueBits; i++) 
    a->value = (a->value << 1) + PullBitIn(a->input);
    
  return(a);  
}

void DeleteArithDecoder(ARITHDECODER a) 
{
  Free(a);
}

int DecodeArithDecoder(ARITHDECODER a,IHISTOGRAM ih)
{
    long range; 
    int cum;
    int answer;
    int ct, ctLeft, ctTotal;

    ctTotal = TotalCountIHistogram(ih);
    range = a->high - a->low + 1;
    cum = (((long)(a->value - a->low) + 1) * ctTotal - 1) / range;
    answer = SymbolIHistogram(ih,cum);
    
    ct =  CountIHistogram(ih,answer); 
    ctLeft = LeftCountIHistogram(ih,answer);

    a->high = a->low + (range * (ctLeft + ct)) / ctTotal - 1;
    a->low = a->low + (range * ctLeft) / ctTotal;
    while (1) {                 
        if (a->high < CVHalf) {
	    ;
        } else if (a->low >= CVHalf) { 
            a->value -= CVHalf;
            a->low -= CVHalf; 
            a->high -= CVHalf;
        }
        else if (a->low >= CVQtr && a->high < CVThreeQtr) {
            a->value -= CVQtr;
            a->low -= CVQtr;    
            a->high -= CVQtr;
        }
        else break;       
        a->low = 2 * a->low;
        a->high = 2 * a->high + 1;  
        a->value = 2 * a->value + PullBitIn(a->input); 
    }
    return answer;
}


/*
 * INt coding
 */
 
CDELTAENCODE NewCDeltaEncode(BITOUT bo)
{
  CDELTAENCODE cd;
  
  cd = (CDELTAENCODE) Malloc(sizeof(CDeltaEncode));
  
  cd->output = bo;
  return(cd);
} 

void DeleteCDeltaEncode(CDELTAENCODE cd)
{
  Free(cd);
}


void EncodePositiveCDeltaEncode(CDELTAENCODE cd, int i)
{
  int digits;
    
  for (digits = 30; digits > 0; digits--) {
    if (i >= (1 << digits)) {
      PushBitOut(cd->output,0);
    }
  }
  PushBitOut(cd->output,1);
  for (digits = 29; digits >= 0; digits--) {
    if (i >= (2 << digits)) {
      if (i & (1 << digits)) {
    PushBitOut(cd->output,1);
      } else {
    PushBitOut(cd->output,0);
      }
    }
  }
}

void EncodeNonNegativeCDeltaEncode(CDELTAENCODE cd, int i)
{
  EncodePositiveCDeltaEncode(cd,i + 1);
}

void EncodeCDeltaEncode(CDELTAENCODE cd, int i)
{
  if (i < 0) {
    PushBitOut(cd->output,1);
    EncodePositiveCDeltaEncode(cd,-i);
  } else {
    PushBitOut(cd->output,0);
    EncodeNonNegativeCDeltaEncode(cd,i);
  }
}



/*---------------------------------------------------------------------------*/

CDELTADECODE NewCDeltaDecode(BITIN bi)
{
  CDELTADECODE cd;
  
  cd = (CDELTADECODE) Malloc(sizeof(CDeltaDecode));
  
  cd->input = bi;
  
  return(cd);
} 

void DeleteCDeltaDecode(CDELTADECODE cd)
{
  Free(cd);
}

int DecodePositiveCDeltaDecode(CDELTADECODE cd)
{
  int digits = 0;
  int value = 1;
  int i;
    
  while(PullBitIn(cd->input) == 0) {
    digits++;
  }
  for (i = 0; i < digits; i++) {
    value = 2 * value + PullBitIn(cd->input);
  }
  return value;
}

/*---------------------------------------------------------------------------*/
int DecodeNonNegativeCDeltaDecode(CDELTADECODE cd)
{
  int value = DecodePositiveCDeltaDecode(cd) - 1;
  return value;
}

/*---------------------------------------------------------------------------*/

int DecodeCDeltaDecode(CDELTADECODE cd)
{
  int value;

  if (PullBitIn(cd->input)) {
    value =  -DecodePositiveCDeltaDecode(cd);
  } else {
    value =  DecodeNonNegativeCDeltaDecode(cd);
  }
  return value;
}
