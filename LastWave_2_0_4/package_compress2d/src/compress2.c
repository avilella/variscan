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
#include "owtrans2d.h"
#include "compress2d.h"



/*******************************************************
 * 
 * Image Quantization
 *
 *******************************************************/
 
void  QuantizeOWtrans2 (OWTRANS2 wtrans, float StepSize)
{ 
  float RateTot=0;
  float DistTot = 0;
  int i;
 
  /* Checkings */
  CheckOWtrans2(wtrans);

  /* Desallocation */
  if (wtrans->coeff) DeleteCoeffOWtrans2(wtrans);
  
  /* Allocation */
  wtrans->coeff = (COEFFSET *) Malloc(sizeof(COEFFSET)*wtrans->noct*3+1);
 
  /* Let's quantize */
  for (i=0; i<wtrans->noct*3+1 ; i++) {   
    wtrans->coeff[i] = NewCoeffSet(wtrans->subimage[i]->pixels,wtrans->subimage[i]->nrow*wtrans->subimage[i]->ncol, StepSize,i!=0);
    GetRateDistCoeffSet(wtrans->coeff[i]);
    RateTot = RateTot + wtrans->coeff[i]->rate;
    DistTot = DistTot + wtrans->coeff[i]->dist;
  }
}

void C_Quant2(char **argv)
{
  OWTRANS2 wtrans;
  float StepSize;


  argv = ParseArgv(argv,tOWTRANS2_,NULL,&wtrans,tFLOAT,&StepSize,0);
  
  if (wtrans ==NULL) wtrans= GetOWtrans2Cur(); 
 
  if (StepSize<0) Errorf("Bad value of StepSize ");
  
  QuantizeOWtrans2(wtrans,StepSize);
}



/*******************************************************
 * 
 * Image Coding
 *
 *******************************************************/

void  CodeOWtrans2 (OWTRANS2 wtrans, char *filename)
{
  int paramPrecision =4;
  int nSets = 3*wtrans->noct+1;
  int i;
  FILE *stream;
  ENCODER encoder;
  int tint;
      
  /* Checkings */
  CheckOWtrans2(wtrans);
  if (wtrans->coeff==NULL) Errorf("CodeOWtrans2() : You must run 'quant2' first !");

  /* Open the file */
  stream = FOpen(filename,"w");
  if (stream == NULL) Errorf("CodeOWtrans2() : Cannot open file '%s'", filename);
  
  /* Print the wavelet name */
  fprintf(stream,"%s\n",wtrans->wavelet->name);   

  /*
   * Start coding 
   */
  encoder = NewEncoder (stream,stderr);
  WritePositiveEncoder(encoder,wtrans->hsize);
  WritePositiveEncoder(encoder,wtrans->vsize);

  tint =  (int) (wtrans->coeff[0]->quant->stepSize* exp (paramPrecision* log(2.0)) + 0.5);
  WriteNonNegEncoder(encoder,tint);
  WriteNonNegEncoder(encoder,wtrans->noct);

  for (i=0; i<nSets; i++) WriteHeaderCoeffSet(wtrans->coeff[i],encoder);

  for (i=0; i<nSets; i++) EncodeCoeffSet(wtrans->coeff[i],encoder);

  FlushEncoder(encoder);

  DeleteEncoder(encoder);

  FClose(stream);    
}


void C_Code2(char **argv)
{
  OWTRANS2 wtrans;
  char * outfilename;
  float stepSize;
  
  argv = ParseArgv(argv,tOWTRANS2_,NULL,&wtrans,tSTR,&outfilename,tFLOAT,&stepSize,0);
  
  if (wtrans ==NULL) wtrans= GetOWtrans2Cur(); 

  if (stepSize<0) Errorf("Bad value of StepSize ");
 
  QuantizeOWtrans2(wtrans,stepSize);

  CodeOWtrans2(wtrans,outfilename);
}


/*******************************************************
 * 
 * Image Decoding
 *
 *******************************************************/

void  DecodeOWtrans2 (OWTRANS2 wtrans,char *infilename)
{
 
  int i;
  int paramPrecision = 4; 
  FILE *stream;
  char str[15];
  DECODER decoder;
  int tint;
  float StepSize;
  int nSets,noct,hsize,vsize;
    
  stream = FOpen(infilename,"r");
  if (stream == NULL) Errorf("DecodeOWtrans2() : Cannot open file '%s'", infilename);
  
  fscanf(stream,"%s\n",str);

  SetWaveletOWtrans2 (wtrans,str);
   
  decoder = NewDecoder(stream,stderr);
 
  hsize= ReadPositiveDecoder(decoder);
  vsize= ReadPositiveDecoder(decoder);

  tint = ReadNonNegDecoder(decoder);
  StepSize =  (float) tint / exp (paramPrecision * log(2.0));
  SizeImage(wtrans->original,wtrans->vsize,wtrans->hsize);

  noct = ReadNonNegDecoder(decoder);
  
  SetNOctOWtrans2(wtrans,noct,hsize,vsize);
  
  nSets = 3*wtrans->noct+1;
   
  wtrans->coeff = (COEFFSET *) Malloc(nSets*sizeof(COEFFSET));
    
  for (i=0; i<(nSets) ; i++) 
    wtrans->coeff[i] = NewCoeffSet (wtrans->subimage[i]->pixels, wtrans->subimage[i]->nrow*wtrans->subimage[i]->ncol, StepSize,i!=0);
  
  for (i=0; i<nSets; i++) ReadHeaderCoeffSet(wtrans->coeff[i],decoder);

  for (i=0; i<nSets; i++) DecodeCoeffSet(wtrans->coeff[i],decoder);
 
  DeleteDecoder(decoder);
  FClose(stream);
}



void C_Decode2(char **argv)
{
  OWTRANS2 wtrans;
  char * infilename;
  IMAGE image;

  argv = ParseArgv(argv,tOWTRANS2_,NULL,&wtrans,tSTR,&infilename,tIMAGE_,NULL,&image,0);
  
  if (wtrans ==NULL) wtrans= GetOWtrans2Cur(); 
  if (image == NULL) image = wtrans->original;
    
  DecodeOWtrans2(wtrans,infilename);
  
  OWt2r (wtrans,image);
}
