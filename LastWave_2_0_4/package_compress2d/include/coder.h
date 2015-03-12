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


#ifndef CODER_H
#define CODER_H


typedef struct encoder {

  BITOUT bitout;

  ARITHENCODER arith;
  CDELTAENCODE intcoder;

} Encoder, *ENCODER;

extern ENCODER NewEncoder (FILE *out, FILE *log);
extern void DeleteEncoder (ENCODER e);
extern void FlushEncoder(ENCODER e);
extern void WriteIntEncoder (ENCODER e, int i);
extern void WritePositiveEncoder (ENCODER e, int i);
extern void WriteNonNegEncoder (ENCODER e, int i);




typedef struct decoder {
  BITIN bitin;
  ARITHDECODER arith;
  CDELTADECODE intcoder;  
} Decoder, *DECODER;

extern DECODER NewDecoder (FILE * in, FILE  *log);
extern void DeleteDecoder (DECODER d);
extern int ReadIntDecoder (DECODER d);
extern int ReadPositiveDecoder (DECODER d);
extern int ReadNonNegDecoder(DECODER d);




#define EntropyCoderType 1
#define EntropyCoderFields char type; int histoCapacity

typedef struct  entropyCoder {

  EntropyCoderFields;

  
} EntropyCoder, *ENTROPYCODER;

extern ENTROPYCODER NewEntropyCoder(int histoCapacity);
extern void DeleteEntropyCoder(ENTROPYCODER e);
extern float WriteEntropyCoder(ENTROPYCODER ec, ENCODER encoder, int symbol, char update,int context1, int context2);
extern int ReadEntropyCoder(ENTROPYCODER ec, DECODER decoder, char update,int context1, int context2);
extern float CostEntropyCoder (ENTROPYCODER ec, int symbol, char update, int context1, int context2);
extern float UpdateCostEntropyCoder (ENTROPYCODER ec, int symbol, int context1 , int context2);
extern void UpdateEntropyCoder  (ENTROPYCODER ec,int symbol, int context1 , int context2);
extern float EntropyEntropyCoder(ENTROPYCODER ec, float p); 
extern void ResetEntropyCoder(ENTROPYCODER ec); 




#define MonoLayerCoderType 2
#define MonoLayerCoderFields EntropyCoderFields; int nSym

typedef struct monoLayerCoder 
{
  MonoLayerCoderFields;  
} MonoLayerCoder, *MONOLAYERCODER;

extern MONOLAYERCODER NewMonoLayerCoder (int histoCapacity) ;
extern void DeleteMonoLayerCoder (MONOLAYERCODER e); 
extern void SetNSymMonoLayerCoder(MONOLAYERCODER m, int nSym);



#define EscapeCoderType 3

typedef struct escapeCoder {
  MonoLayerCoderFields;

  IHISTOGRAM freq, uniform;
  char *seen;

} EscapeCoder, *ESCAPECODER;

extern ESCAPECODER NewEscapeCoder1(int histoCapacity); 
extern ESCAPECODER NewEscapeCoder3(int histoCapacity, int nSym);
extern void ResetEscapeCoder (ESCAPECODER e);
extern void DeleteEscapeCoder (ESCAPECODER e);
extern void SetNSymEscapeCoder (ESCAPECODER e, int newNSym);
extern float WriteEscapeCoder(ESCAPECODER ec, ENCODER encoder, int symbol, char update,int context1, int context2);
extern int ReadEscapeCoder (ESCAPECODER ec, DECODER decoder, char update, int context1, int context2);
extern float CostEscapeCoder (ESCAPECODER ec, int symbol, char update, int context1, int context2);



/*---------------------------------------------------------------------------*/


typedef struct coeffset {

  ESCAPECODER entropy;
  int  nData;
  float *data;

  struct uniformQuant *quant;
  
  float rate;     /* cost (in bits) to perform each quantization */
  float dist;     /* distortion incurred using each quantizer  */

  int capacity; /* of the histogramm for Arithmetic Coder  */

} CoeffSet, *COEFFSET;


extern COEFFSET NewCoeffSet (float *data, int nData,float stepSize, int ZeroCenter);
extern void DeleteCoeffSet(COEFFSET c);
extern void WriteHeaderCoeffSet (COEFFSET c, ENCODER encoder);
extern void ReadHeaderCoeffSet  (COEFFSET c, DECODER decoder);
extern void EncodeCoeffSet (COEFFSET c, ENCODER encoder);
extern void DecodeCoeffSet (COEFFSET c,  DECODER decoder);
extern void GetRateDistCoeffSet (COEFFSET c);


#endif
/*---------------------------------------------------------------------------*/
