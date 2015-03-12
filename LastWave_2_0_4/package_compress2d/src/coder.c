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
#include "coder.h"
#include "quantize.h"



ENCODER NewEncoder (FILE *out, FILE *log)
{
   ENCODER e;
   
   e = (ENCODER) Malloc(sizeof(Encoder));
   
   e->bitout   = NewBitOut (out, log);
   e->arith    = NewArithEncoder(e->bitout);
   e->intcoder = NewCDeltaEncode (e->bitout);
   
   return(e);
}

void DeleteEncoder (ENCODER e)
{
  DeleteCDeltaEncode(e->intcoder);
  DeleteArithEncoder(e->arith);
  DeleteBitOut(e->bitout);
  
  Free(e);
}


void FlushEncoder(ENCODER e)
{
  FlushArithEncoder(e->arith);
}

void WritePositiveEncoder (ENCODER e, int i)     { EncodePositiveCDeltaEncode(e->intcoder,i); }
void WriteNonNegEncoder (ENCODER e, int i)       { EncodeNonNegativeCDeltaEncode(e->intcoder,i); }
void WriteIntEncoder (ENCODER e, int i)          { EncodeCDeltaEncode(e->intcoder,i); }
static void writeBit (ENCODER e, int bit)        { PushBitOut (e->bitout,bit); }
static void writeNBits (ENCODER e, int n, int i) { while(n--){writeBit(e,(i&(1 << n))!=0);} }
static void writeSymbol (ENCODER e,int symbol, IHISTOGRAM h)
{ 
  EncodeArithEncoder(e->arith,CountIHistogram(h,symbol), LeftCountIHistogram(h,symbol),
			         TotalCountIHistogram(h)); 
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

DECODER NewDecoder (FILE * in, FILE  *log)
{
  DECODER d;
  
  d = (DECODER) Malloc(sizeof(Decoder));
  
  d->bitin    = NewBitIn (in, log);
  d->intcoder = NewCDeltaDecode (d->bitin);
  d->arith    = NULL;
  
  return(d);
}


void DeleteDecoder (DECODER d)
{
   DeleteCDeltaDecode(d->intcoder);
   if (d->arith != NULL) DeleteArithDecoder(d->arith);
   DeleteBitIn(d->bitin);
}


static int readSymbol (DECODER d, IHISTOGRAM h) 
                    { if (d->arith==NULL) d->arith = NewArithDecoder(d->bitin); 
                          return DecodeArithDecoder(d->arith,h); }
int ReadPositiveDecoder (DECODER d)   { return DecodePositiveCDeltaDecode(d->intcoder); }
int ReadNonNegDecoder(DECODER d)     { return DecodeNonNegativeCDeltaDecode(d->intcoder); }
int ReadIntDecoder (DECODER d)        { return DecodeCDeltaDecode(d->intcoder); }
static  int readBit (DECODER d)        { return PullBitIn(d->bitin); }
static  int readNBits (DECODER d,int n) { int i = 0; while (n--) 
                           {i |= readBit(d)*(1<<n);} return i; }



/*---------------------------------------------------------------------------*/

ENTROPYCODER NewEntropyCoder(int histoCapacity)
{
  ENTROPYCODER e;
  
  e = (ENTROPYCODER) Malloc(sizeof(EntropyCoder));
  
  e->histoCapacity = histoCapacity;
  e->type = EntropyCoderType;
  
  return(e);
}

void DeleteEntropyCoder(ENTROPYCODER ec)
{
   if (ec->type == EntropyCoderType) Free(ec); 
   else if (ec->type == MonoLayerCoderType) DeleteMonoLayerCoder((MONOLAYERCODER) ec); 
   else if (ec->type == EscapeCoderType) DeleteEscapeCoder((ESCAPECODER) ec); 
   else Errorf("DeleteEntropyCoder() : Weird Error");
}

float WriteEntropyCoder(ENTROPYCODER ec, ENCODER encoder, int symbol, char update,int context1, int context2)
{
   if (ec->type == EscapeCoderType) return(WriteEscapeCoder((ESCAPECODER) ec,encoder, symbol,update,context1,context2)); 
   else Errorf("WriteEntropyCoder() : Weird Error");
}

int ReadEntropyCoder(ENTROPYCODER ec, DECODER decoder, char update,int context1, int context2)
{
   if (ec->type == EscapeCoderType) return(ReadEscapeCoder((ESCAPECODER) ec, decoder,update,context1,context2)); 
   else Errorf("ReadEntropyCoder() : Weird Error");
}

float CostEntropyCoder (ENTROPYCODER ec, int symbol, char update, int context1, int context2)
{ 
  if (ec->type == EscapeCoderType) return CostEscapeCoder((ESCAPECODER) ec, symbol, update, context1, context2); 
  
  Errorf("UpdateCostEntropyCoder() : Weird error");
}

float UpdateCostEntropyCoder (ENTROPYCODER ec, int symbol, int context1 , int context2)
{ 
  return(CostEntropyCoder(ec, symbol, YES, context1, context2)); 
}

void UpdateEntropyCoder  (ENTROPYCODER ec,int symbol, int context1 , int context2)
{ 
  CostEntropyCoder(ec, symbol, YES, context1, context2); 
}

float EntropyEntropyCoder(ENTROPYCODER ec, float p) 
{ 
  return -log(p)/log(2.0); 
}

void ResetEntropyCoder(ENTROPYCODER ec) 
{ 
  if (ec->type == EscapeCoderType) ResetEscapeCoder((ESCAPECODER) ec); 
  
  else Errorf("ResetEntropyCoder() : Weird error");
}




/*---------------------------------------------------------------------------*/

MONOLAYERCODER NewMonoLayerCoder (int histoCapacity) 
{
  MONOLAYERCODER e;
  
  e = (MONOLAYERCODER) Malloc(sizeof(MonoLayerCoder));
  
  e->histoCapacity = histoCapacity;
  e->type = MonoLayerCoderType;
  
  return(e);
}

void DeleteMonoLayerCoder (MONOLAYERCODER e) 
{
  Free(e);
}

void SetNSymMonoLayerCoder(MONOLAYERCODER m, int nSym)
{
  if (m->type == EscapeCoderType) SetNSymEscapeCoder((ESCAPECODER) m,nSym); 
  
  else Errorf("SetNSymMonoLayerCoder() : Weird error");
}




/*---------------------------------------------------------------------------*/

ESCAPECODER NewEscapeCoder1(int histoCapacity) 
{
  ESCAPECODER e;
  
  e = (ESCAPECODER) Malloc(sizeof(EscapeCoder));
  
  e->histoCapacity = histoCapacity;

  e->freq = e->uniform = NULL;
  e->seen = NULL;
  e->type = EscapeCoderType;
  
  return(e);
}

ESCAPECODER NewEscapeCoder3(int histoCapacity, int nSym)
{
  ESCAPECODER e;
  
  e = (ESCAPECODER) Malloc(sizeof(EscapeCoder));
  
  e->histoCapacity = histoCapacity;

  e->seen = NULL;
  e->type = EscapeCoderType;
  SetNSymEscapeCoder (e,nSym);
  
  return(e);
}

void DeleteEscapeCoder (ESCAPECODER e)
{ 
  if (e->seen != NULL) {
    Free(e->seen);
    DeleteIHistogram(e->freq);
    DeleteIHistogram(e->uniform);
    
  }
  Free(e);
}


void ResetEscapeCoder (ESCAPECODER e)
{
  int *temp = IntAlloc(e->nSym+1);
  int i;

  for (i = 0; i < e->nSym; i++) /* no symbols observed -- set all counts to 0 */
    temp[i] = 0;
  temp[e->nSym] = 1;                /* except for escape symbol */
  InitCounts1IHistogram(e->freq,temp);

  for (i = 0; i < e->nSym; i++) /* uniform histogram -- set all counts to 1 */
    temp[i] = 1;
  InitCounts1IHistogram(e->uniform,temp);

  Free(temp);
}

void SetNSymEscapeCoder (ESCAPECODER e, int newNSym)
{
  int i;
  
  if (e->seen != NULL) {
    Free(e->seen);
    DeleteIHistogram(e->freq);
    DeleteIHistogram(e->uniform);
   }

  e->nSym = newNSym;
 
  e->freq = NewIHistogram (e->nSym+1, e->histoCapacity);
  e->uniform = NewIHistogram (e->nSym, e->histoCapacity);
 
  e->seen = CharAlloc(e->nSym);
  for (i = 0; i < e->nSym; i++)
    e->seen[i] = NO;

  ResetEscapeCoder (e);
}


float WriteEscapeCoder(ESCAPECODER ec, ENCODER encoder, int symbol, char update,int context1, int context2)
{
  const int Escape = ec->nSym;
  float bits;

  context1 = context2 = -1;  /* prevents warning from compiler */

  if (ec->seen[symbol]) {
    bits = EntropyIHistogram(ec->freq,symbol);
    if (encoder != NULL)
      writeSymbol (encoder,symbol, ec->freq);
  } else {
    if (encoder != NULL) {
      writeSymbol (encoder,Escape, ec->freq);
      writeSymbol (encoder,symbol, ec->uniform);
    }
    bits = EntropyIHistogram(ec->freq,Escape) + EntropyIHistogram(ec->uniform,symbol);

    if (update)
      ec->seen[symbol] = YES;
  }
  if (update) IncCountIHistogram(ec->freq,symbol);

  return bits;
}

int ReadEscapeCoder (ESCAPECODER ec, DECODER decoder, char update, 
		       int context1, int context2)
{
  const int Escape = ec->nSym;
  int symbol;

  context1 = context2 = -1;  /* prevents warning from compiler */

  symbol = readSymbol(decoder,ec->freq);
  if (symbol == Escape) {
    symbol = readSymbol(decoder,ec->uniform);
  }

  if (update) IncCountIHistogram(ec->freq,symbol);

  return symbol;
}

float CostEscapeCoder (ESCAPECODER ec, int symbol, char update, int context1, int context2)
{
  const int Escape = ec->nSym;
  float bits;

  context1 = context2 = -1;  /* prevents warning from compiler */

  if (ec->seen[symbol]) {
    bits = EntropyIHistogram(ec->freq,symbol);

  } else {
    bits = EntropyIHistogram(ec->freq,Escape) + EntropyIHistogram(ec->uniform,symbol);

    if (update)
      ec->seen[symbol] = YES;
  }
  if (update)
    IncCountIHistogram(ec->freq,symbol);

  return bits;
}



/*---------------------------------------------------------------------------*/

COEFFSET NewCoeffSet (float *data, int nData,float stepSize, int ZeroCenter)
{  
  int paramPrecision = 4;
  COEFFSET c;
  
  
   c = (COEFFSET) Malloc(sizeof(CoeffSet));
  
   c->data = data;
   c->nData = nData;

   c->capacity = 8192;

  c->entropy = NewEscapeCoder1(c->capacity);

  c->quant =  NewUniformQuant((MonoLayerCoder *) c->entropy,paramPrecision, stepSize,ZeroCenter); 
  
  return(c); 
}

void DeleteCoeffSet(COEFFSET c)
{  
  DeleteEscapeCoder(c->entropy);
  DeleteUniformQuant(c->quant);
 
  Free(c);
}


void WriteHeaderCoeffSet (COEFFSET c, ENCODER encoder)
{ 
  WriteHeaderUniformQuant(c->quant,encoder); 
}

void ReadHeaderCoeffSet  (COEFFSET c, DECODER decoder)
{ 
  ReadHeaderUniformQuant(c->quant,decoder); 
}

void EncodeCoeffSet (COEFFSET c, ENCODER encoder)
{
  EncodeUniformQuant(c->quant,encoder);
}
 
void DecodeCoeffSet (COEFFSET c,  DECODER decoder)
{ 
  SetDataDecodeUniformQuant(c->quant,c->data, c->nData,-1,1,-1);
  DequantizeUniformQuant(c->quant,decoder);
}
 

void GetRateDistCoeffSet (COEFFSET c)
{ 
  float minStepSize = 0.05;
  
  /* calcul qmin, qmax */
  SetDataEncodeUniformQuant(c->quant,c->data, c->nData);
    
 
     /* calcul de la quantification sauvegarde des valeurs dans quant->symbol[] */
   GetRateDistUniformQuant(c->quant,minStepSize, &(c->rate), &(c->dist)); 
 
}

/*---------------------------------------------------------------------------*/

