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

#ifndef _QUANTIZER_
#define _QUANTIZER_


typedef struct uniformQuant  {
 
  int imin, imax, imean;
  float qmin, qmax, qmean;
  MonoLayerCoder *entropy;
  int paramPrecision, zeroCenter;  
  float p;

  float *data;  /* input datas */
  int nData;

  int * symbol; /* symbol = quantif(datas). */
  int nSymbol;
  
  float stepSize;
 


  float max, min, mean, var, sum, sumSq;
  float initialDist;

} UniformQuant, *UNIFORMQUANT;


extern UNIFORMQUANT NewUniformQuant (MONOLAYERCODER entropy, int paramPrecision,float StepSize,int zeroCenter); 
extern void DeleteUniformQuant (UNIFORMQUANT u);
extern void GetStatsUniformQuant (UNIFORMQUANT u);
extern void SetDataEncodeUniformQuant (UNIFORMQUANT u, float *newData, int newNData);
extern void SetDataDecodeUniformQuant (UNIFORMQUANT u, float *newData, int newNData, int imax, int imin, int imean);
extern void GetRateDistUniformQuant( UNIFORMQUANT u, float minStepSize,float *rate, float *dist); 
extern void EncodeUniformQuant (UNIFORMQUANT u, ENCODER encoder);
extern void DequantizeUniformQuant (UNIFORMQUANT u, DECODER decoder);
extern void WriteHeaderUniformQuant (UNIFORMQUANT u, ENCODER encoder);
extern void ReadHeaderUniformQuant(UNIFORMQUANT u, DECODER decoder);


#endif

