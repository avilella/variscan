/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'owtrans2d' 2.0                    */
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



/* Misc functions for copying arrays */
static void Copy3(float *p1,float *p2,int length)
{
  memcpy(p2,p1,length*sizeof(float));
}
static void Copy4(float *p1,int stride1, float *p2,int length)
{
 int temp = length; while(temp--) {*p2++ = *p1; p1 += stride1;}
}
static void Copy41 (float *p1, float *p2,  int stride2,int length)
{
  int temp = length; while(temp--) {*p2 = *p1++; p2 += stride2;}
}

/*---------------------------------------------------------------------------
* Do symmetric extension of data using prescribed symmetries
*   Original values are in output[npad] through output[npad+size-1]
*   New values will be placed in output[0] through output[npad] and in
*      output[npad+size] through output[2*npad+size-1] (note: end values may
*      not be filled in)
*   left_ext = 1 -> extension at left bdry is   ... 3 2 1 | 0 1 2 3 ...
*   left_ext = 2 -> extension at left bdry is ... 3 2 1 0 | 0 1 2 3 ...
*   right_ext = 1 or 2 has similar effects at the right boundary
*
*   symmetry = 1  -> extend symmetrically
*   symmetry = -1 -> extend antisymmetrically
*/

static void SymmetricExtension (OWAVELET2 w,float *output, int size, int left_ext, 
				    int right_ext, int symmetry)
{
  int i;
  int npad = w->npad;
  int first = npad, last = npad + size-1;
int nextend;
  int originalFirst;
  int originalLast ;
  int originalSize ;
int period;

  if (symmetry == -1) {
    if (left_ext == 1)
      output[--first] = 0;
    if (right_ext == 1)
      output[++last] = 0;
  }
  originalFirst = first;
  originalLast = last;
  originalSize = originalLast-originalFirst+1;

  period = 2 * (last - first + 1) - (left_ext == 1) - (right_ext == 1);

  if (left_ext == 2)
    output[--first] = symmetry*output[originalFirst];
  if (right_ext == 2)
    output[++last] = symmetry*output[originalLast];

  /* extend left end */
  nextend = MIN (originalSize-2, first);
  for (i = 0; i < nextend; i++) {
    output[--first] = symmetry*output[originalFirst+1+i];
  }

  /* should have full period now -- extend periodically */
  while (first > 0) {
    first--;
    output[first] = output[first+period];
  }

  /* extend right end */
  nextend = MIN (originalSize-2, 2*npad+size-1 - last);
  for (i = 0; i < nextend; i++) {
    output[++last] = symmetry*output[originalLast-1-i];
  }

  /* should have full period now -- extend periodically */
  while (last < 2*npad+size-1) {
    last++;
    output[last] = output[last-period];
  }
}

/*---------------------------------------------------------------------------
* Do periodic extension of data using prescribed symmetries
*   Original values are in output[npad] through output[npad+size-1]
*   New values will be placed in output[0] through output[npad] and in
*      output[npad+size] through output[2*npad+size-1] (note: end values may
*      not be filled in)
*/

static void PeriodicExtension (OWAVELET2 w, float *output, int size)
{
  int npad = w->npad;
  int first = npad, last = npad + size-1;

  /* extend left periodically */
  while (first > 0) {
    first--;
    output[first] = output[first+size];
  }

  /* extend right periodically */
  while (last < 2*npad+size-1) {
    last++;
    output[last] = output[last-size];
  }
}

/*---------------------------------------------------------------------------*/

static void MallatToLinear (OWTRANS2 wtrans, float *mallat)
{
  int i, j, k;
  
  int *lowHsize = IntAlloc(wtrans->noct);
  int *lowVsize = IntAlloc(wtrans->noct);
  
  lowHsize[wtrans->noct-1] = (wtrans->hsize+1)/2;
  lowVsize[wtrans->noct-1] = (wtrans->vsize+1)/2;
  

  for (i = wtrans->noct-2; i >= 0; i--) {
    lowHsize[i] = (lowHsize[i+1]+1)/2;
    lowVsize[i] = (lowVsize[i+1]+1)/2;
  }
  
  /* move transformed image (in Mallat order) into linear array structure 
   special case for LL subband */
  for (j = 0; j < wtrans->subimage[0]->nrow; j++)
    for (i = 0; i < wtrans->subimage[0]->ncol; i++)
      wtrans->subimage[0]->pixels[j*(wtrans->subimage[0]->ncol)+i] = mallat[j*wtrans->hsize+i];
 
  for (k = 0; k < wtrans->noct; k++) {
    for (j = 0; j < wtrans->subimage[k*3+1]->nrow; j++)
      for (i = 0; i < wtrans->subimage[k*3+1]->ncol; i++)
	wtrans->subimage[k*3+1]->pixels[j*wtrans->subimage[k*3+1]->ncol+i] = 
	  mallat[j*wtrans->hsize+(lowHsize[k]+i)];

    for (j = 0; j < wtrans->subimage[k*3+2]->nrow; j++)
      for (i = 0; i < wtrans->subimage[k*3+2]->ncol; i++) 
	wtrans->subimage[k*3+2]->pixels[j*wtrans->subimage[k*3+2]->ncol+i] = 
	  mallat[(lowVsize[k]+j)*wtrans->hsize+i];

    for (j = 0; j < wtrans->subimage[k*3+3]->nrow; j++)
      for (i = 0; i < wtrans->subimage[k*3+3]->ncol; i++)
	wtrans->subimage[k*3+3]->pixels[j*wtrans->subimage[k*3+3]->ncol+i] = mallat[(lowVsize[k]+j)*wtrans->hsize+(lowHsize[k]+i)];
  }

  Free(lowHsize);
  Free(lowVsize);
}

/*****************************************************************************/

static void LinearToMallat (OWTRANS2 wtrans, float *mallat)
{
  int i, j, k;

  int *lowHsize = IntAlloc(wtrans->noct);
  int *lowVsize = IntAlloc(wtrans->noct);
  
  lowHsize[wtrans->noct-1] = (wtrans->hsize+1)/2;
  lowVsize[wtrans->noct-1] = (wtrans->vsize+1)/2;
  
  for (i = wtrans->noct-2; i >= 0; i--) {
    lowHsize[i] = (lowHsize[i+1]+1)/2;
    lowVsize[i] = (lowVsize[i+1]+1)/2;
  }
  
  /* put linearized image in Mallat format
   special case for LL subband */
  for (j = 0; j < wtrans->subimage[0]->nrow; j++)
    for (i = 0; i < wtrans->subimage[0]->ncol; i++)
      mallat[j*wtrans->hsize+i] = wtrans->subimage[0]->pixels[j*wtrans->subimage[0]->ncol+i];

  for (k = 0; k < wtrans->noct; k++) {
    for (j = 0; j < wtrans->subimage[k*3+1]->nrow; j++)
      for (i = 0; i < wtrans->subimage[k*3+1]->ncol; i++)
	mallat[j*wtrans->hsize+(lowHsize[k]+i)] = 
	  wtrans->subimage[k*3+1]->pixels[j*wtrans->subimage[k*3+1]->ncol+i];

    for (j = 0; j < wtrans->subimage[k*3+2]->nrow; j++)
      for (i = 0; i < wtrans->subimage[k*3+2]->ncol; i++)
	mallat[(lowVsize[k]+j)*wtrans->hsize+i] = wtrans->subimage[k*3+2]->pixels[j*wtrans->subimage[k*3+2]->ncol+i];

    for (j = 0; j < wtrans->subimage[k*3+3]->nrow; j++)
      for (i = 0; i < wtrans->subimage[k*3+3]->ncol; i++)
	mallat[(lowVsize[k]+j)*wtrans->hsize+(lowHsize[k]+i)] = 
	  wtrans->subimage[k*3+3]->pixels[j*wtrans->subimage[k*3+3]->ncol+i];
  }
  
  Free(lowHsize);
  Free(lowVsize);
}



/***********************************************
 * 
 * Decomposition
 *
 ***********************************************/ 

static void TransformStep (OWAVELET2 w, float *input, float *output, int size, int sym_ext)
{
  int i, j;
  int npad = w->npad;
  OFILTER2 analysisLow = w->analysisLow;
  OFILTER2 analysisHigh = w->analysisHigh;

  int lowSize = (size+1)/2;
  int left_ext, right_ext;

  if (analysisLow->size %2) {
    /* odd filter length */
    left_ext = right_ext = 1;
  } else {
    left_ext = right_ext = 2;
  }

  if (sym_ext)
    SymmetricExtension (w,input, size, left_ext, right_ext, 1);
  else
    PeriodicExtension (w,input, size);
    
  /*                      coarse  detail
   xxxxxxxxxxxxxxxx --> HHHHHHHHGGGGGGGG */
  for (i = 0; i < lowSize; i++)  {
    output[npad+i] = 0.0;
    for (j = 0; j < analysisLow->size; j++)  {
      output [npad+i] += 
	input[npad + 2*i + analysisLow->firstIndex + j] *
	analysisLow->coeff[j];
    }
  }
  
  for (i = lowSize; i < size; i++)  {
    output[npad+i] = 0.0;
    for (j = 0; j < analysisHigh->size; j++)  {
      output [npad+i] += 
	input[npad + 2*(i-lowSize) + analysisHigh->firstIndex + j] * 
	analysisHigh->coeff[j];
    }
  }
}

/* 1d decomp */
static void OWtrans1dDecomp(OWAVELET2 w, float *input, float *output, int size, int noct, int sym_ext)
{
   int i;
   int currentIndex = 0;
   float *data[2];
   int lowSize = size, highSize;
   int npad = w->npad;
   int symmetric = w->symmetric;
   
   /* If form of extension unspecified, default to symmetric
    extensions for symmetrical filters and periodic extensions for
    asymmetrical filters */
   if (sym_ext == -1) sym_ext = symmetric;

   /* data[0] and data[1] are padded with npad entries on each end */
   data [0] = FloatAlloc(2*npad+size);
   data [1] = FloatAlloc(2*npad+size);

   for (i = 0; i < size; i++)
     data[currentIndex][npad+i] = input[i];

   while (noct--)  {
     if (lowSize <= 2 && symmetric == 1) 
       Errorf("OWtrans1dDecomp() : Warning ! Reduce # of transform steps or increase signal size or switch to periodic extension Low pass subband is too small");

     /* Transform */
     Printf ("transforming, size = %d\n", lowSize);
     TransformStep (w, data[currentIndex], data[1-currentIndex],lowSize, sym_ext);
     
     highSize = lowSize/2;
     lowSize = (lowSize+1)/2;
     
     /* Copy high-pass data to output signal */ 
     Copy3 (data[1-currentIndex] + npad + lowSize, output + lowSize, highSize);

     for (i = 0; i < lowSize+highSize; i++) Printf ("%5.2f ", data[1-currentIndex][npad+i]);
     Printf ("\n\n");
     
     /* Now pass low-pass data (first 1/2 of signal) back to transform routine  */
     currentIndex = 1 - currentIndex;
   }
   
   /* Copy low-pass data to output signal */ 
   Copy3 (data[currentIndex] + npad, output, lowSize);
   
   Free(data[1]);
   Free(data[0]); /* ???*/
}

/* 2d decomp */
static void OWtrans2dDecomp (OWAVELET2 w, float *input, float *output, int hsize, int vsize,int noct, int sym_ext)
{
   int j;
   int hLowSize = hsize, hHighSize;
   int vLowSize = vsize, vHighSize;
   int npad = w->npad;
   float *temp_in;
   float *temp_out;
   int symmetric = w->symmetric;
   
   /* If form of extension unspecified, default to symmetric
    extensions for symmetrical filters and periodic extensions for
    asymmetrical filters */
   if (sym_ext == -1)
     sym_ext = symmetric;

   temp_in = FloatAlloc(2*npad+MAX(hsize,vsize));
   temp_out = FloatAlloc(2*npad+MAX(hsize,vsize));

   Copy3 (input, output, hsize*vsize);

   while (noct--)  {
      if ((hLowSize <= 2 || vLowSize <= 2) && sym_ext == 1) 
	    Errorf ("Reduce # of transform steps or increase signal size or switch to periodic extension Low pass subband is too small");

      /* Do a convolution on the low pass portion of each row */
      for (j = 0; j < vLowSize; j++)  {
	   /* Copy row j to data array */ 
	   Copy3 (output+(j*hsize), temp_in+npad, hLowSize);
	 
	   /* Convolve with low and high pass filters */
	   TransformStep (w,temp_in, temp_out, hLowSize, sym_ext);

	   /* Copy back to image */ 
	   Copy3 (temp_out+npad, output+(j*hsize), hLowSize);
      }

      /* Now do a convolution on the low pass portion of  each column */
      for (j = 0; j < hLowSize; j++)  {
	   /* Copy column j to data array */ 
	   Copy4 (output+j, hsize, temp_in+npad, vLowSize);
	 
	  /* Convolve with low and high pass filters */
	  TransformStep (w,temp_in, temp_out, vLowSize, sym_ext);

	 /* Copy back to image */ 
	 Copy41 (temp_out+npad, output+j, hsize, vLowSize);
      }

      /* Now convolve low-pass portion again */
      hHighSize = hLowSize/2;
      hLowSize = (hLowSize+1)/2;
      vHighSize = vLowSize/2;
      vLowSize = (vLowSize+1)/2;
   }

  Free(temp_out);
  Free(temp_in);
}


/* C-function to perform decomposition */
void OWt2d (OWTRANS2 wtrans, int noct)
{  
  float *temp;
  
  SetWaveletOWtrans2(wtrans,NULL);

  if (wtrans->wavelet == NULL) Errorf("OWt2d() : Weird error : Wavelet is not initialized");
  if (wtrans->original->ncol == 0 || wtrans->original->nrow == 0) Errorf("OWt2d() : No image to analyze");
   
  SetNOctOWtrans2(wtrans,noct,wtrans->original->ncol,wtrans->original->nrow);
         
  temp = FloatAlloc(wtrans->hsize*wtrans->vsize);
  TempPtr(temp);
  
   
  OWtrans2dDecomp (wtrans->wavelet,wtrans->original->pixels, temp, wtrans->hsize, wtrans->vsize, wtrans->noct,-1);

  /* linearize data */
  MallatToLinear (wtrans, temp);
}


/* The corresponding command */
 void C_OWt2d(char **argv)
{
  OWTRANS2 wtrans;
  int noct;
  
  argv = ParseArgv(argv,tOWTRANS2_,NULL,&wtrans,tINT,&noct,0);
  
  if (wtrans ==NULL) wtrans= GetOWtrans2Cur(); 
  
  OWt2d(wtrans,noct);
}



/***********************************************
 * 
 * Reconstruction
 *
 ***********************************************/ 


static void InvertStep (OWAVELET2 w, float *input, float *output, int size, int sym_ext)
{
   int i, j;
   int left_ext, right_ext, symmetry;
   /* amount of low and high pass -- if odd # of values, extra will be
      low pass */
   int lowSize = (size+1)/2, highSize = size/2;
  int npad = w->npad;
  OFILTER2 analysisLow = w->analysisLow;
  OFILTER2 analysisHigh = w->analysisHigh;
  OFILTER2 synthesisLow = w->synthesisLow;
  OFILTER2 synthesisHigh = w->synthesisHigh;  
   int firstIndex;
   int lastIndex;
   float *temp;

   symmetry = 1;
   if (analysisLow->size % 2 == 0) {
    /* even length filter -- do (2, X) extension */
     left_ext = 2;
   } else {
     /* odd length filter -- do (1, X) extension */
     left_ext = 1;
   }

   if (size % 2 == 0) {
     /* even length signal -- do (X, 2) extension */
     right_ext = 2;
   } else {
     /* odd length signal -- do (X, 1) extension */
     right_ext = 1;
   }

   temp = FloatAlloc(2*npad+lowSize);
   for (i = 0; i < lowSize; i++) {
     temp[npad+i] = input[npad+i];
   }

   if (sym_ext)
     SymmetricExtension (w,temp, lowSize, left_ext, right_ext, symmetry);
   else
     PeriodicExtension (w,temp, lowSize);

   /* coarse  detail
    HHHHHHHHGGGGGGGG --> xxxxxxxxxxxxxxxx */
   for (i = 0; i < 2*npad+size; i++)
     output[i] = 0.0;

   firstIndex = synthesisLow->firstIndex;
   lastIndex = synthesisLow->size - 1 + firstIndex;

   for (i = -lastIndex/2; i <= (size-1-firstIndex)/2; i++)  {
      for (j = 0; j < synthesisLow->size; j++)  {
	output[npad + 2*i + firstIndex + j] +=
	  temp[npad+i] * synthesisLow->coeff[j];
      }
   }

   left_ext = 2;

   if (analysisLow->size % 2 == 0) {
    /* even length filters */
     right_ext = (size % 2 == 0) ? 2 : 1;
     symmetry = -1;
   } else {
     /* odd length filters */
     right_ext = (size % 2 == 0) ? 1 : 2;
     symmetry = 1;
   }

   for (i = 0; i < highSize; i++) {
     temp[npad+i] = input[npad+lowSize+i];
   }
   if (sym_ext)
     SymmetricExtension (w,temp, highSize, left_ext, right_ext,
			  symmetry);
   else
     PeriodicExtension (w,temp, highSize);


   firstIndex = synthesisHigh->firstIndex;
   lastIndex = synthesisHigh->size - 1 + firstIndex;

   for (i = -lastIndex/2; i <= (size-1-firstIndex)/2; i++)  {
      for (j = 0; j < synthesisHigh->size; j++)  {
	output[npad + 2*i + firstIndex + j] +=
	  temp[npad+i] * synthesisHigh->coeff[j];
      }
   }

   Free(temp);
}

static void OWtrans1dRecons (OWAVELET2 w, float *input, float *output, int size, int noct, int sym_ext)
{
   int i;
   int currentIndex = 0;
   float *data[2];
   int npad = w->npad;
   int *lowSize;
   int *highSize;
   int symmetric = w->symmetric;
   
   /* If form of extension unspecified, default to symmetric
    extensions for symmetrical filters and periodic extensions for
    asymmetrical filters */
   if (sym_ext == -1) sym_ext = symmetric;

   lowSize = (int *)  Malloc(sizeof(int)*noct);
   highSize = (int *)  Malloc(sizeof(int)*noct);

   lowSize[0] = (size+1)/2;
   highSize[0] = size/2;

   for (i = 1; i < noct; i++) {
     lowSize[i] = (lowSize[i-1]+1)/2;
     highSize[i] = lowSize[i-1]/2;
   }

   data [0] = FloatAlloc(2*npad+size);
   data [1] = FloatAlloc(2*npad+size);

   Copy3 (input, data[currentIndex]+npad, lowSize[noct-1]); 

   while (noct--)  {
     
     /* grab the next high-pass component */ 
     Copy3 (input + lowSize[noct], 
	 data[currentIndex]+npad+lowSize[noct], highSize[noct]);
     
     /* Combine low-pass data (first 1/2^n of signal) with high-pass
      data (next 1/2^n of signal) to get higher resolution low-pass data */
     InvertStep (w,data[currentIndex], data[1-currentIndex], 
		  lowSize[noct]+highSize[noct], sym_ext);
     
     /* Now pass low-pass data (first 1/2 of signal) back to transform routine */
     currentIndex = 1 - currentIndex;
   }

   /* Copy inverted signal to output signal */ 
   Copy3 (data[currentIndex]+npad, output, size);

   Free(highSize);
   Free(lowSize); /* ???*/

   Free(data[1]);
   Free(data[0]); /* ???*/
}

static void OWtrans2dRecons (OWAVELET2 w, float *input, float *output, int hsize, int vsize, int noct, int sym_ext)
{
   int i, j;
   int npad = w->npad;
   int *hLowSize,*hHighSize;
   int *vLowSize ,*vHighSize;
    float *temp_in ,*temp_out;
   int symmetric = w->symmetric;

   /* If form of extension unspecified, default to symmetric
    extensions for symmetrical filters and periodic extensions for
    asymmetrical filters */
   if (sym_ext == -1)
     sym_ext = symmetric;

   hLowSize = (int *) Malloc(sizeof(int)*noct);
   hHighSize = (int *) Malloc(sizeof(int)*noct);
   vLowSize =(int *) Malloc(sizeof(int)*noct),
   vHighSize = (int *) Malloc(sizeof(int)*noct);

   hLowSize[0] = (hsize+1)/2;
   hHighSize[0] = hsize/2;
   vLowSize[0] = (vsize+1)/2;
   vHighSize[0] = vsize/2;

   for (i = 1; i < noct; i++) {
     hLowSize[i] = (hLowSize[i-1]+1)/2;
     hHighSize[i] = hLowSize[i-1]/2;
     vLowSize[i] = (vLowSize[i-1]+1)/2;
     vHighSize[i] = vLowSize[i-1]/2;
   }

   temp_in = FloatAlloc(2*npad+MAX(hsize,vsize));
   temp_out = FloatAlloc(2*npad+MAX(hsize,vsize));

   Copy3 (input, output, hsize*vsize); 

   while (noct--)  {
      /* Do a reconstruction for each of the columns */
      for (j = 0; j < hLowSize[noct]+hHighSize[noct]; j++)  {
	 /* Copy column j to data array */ 
	 Copy4 (output+j, hsize, temp_in+npad, 
	       vLowSize[noct]+vHighSize[noct]);
	 
	 /* Combine low-pass data (first 1/2^n of signal) with high-pass
	  data (next 1/2^n of signal) to get higher resolution low-pass data */
	 InvertStep (w,temp_in, temp_out,vLowSize[noct]+vHighSize[noct], sym_ext);

	 /* Copy back to image */ 
	 Copy41 (temp_out+npad, output+j, hsize,
	       vLowSize[noct]+vHighSize[noct]);
      }

      /* Now do a reconstruction pass for each row */
      for (j = 0; j < vLowSize[noct]+vHighSize[noct]; j++)  {
	 /* Copy row j to data array */ 
	 Copy3 (output + (j*hsize), temp_in+npad, 
	       hLowSize[noct]+hHighSize[noct]);

	 /* Combine low-pass data (first 1/2^n of signal) with high-pass
	  data (next 1/2^n of signal) to get higher resolution low-pass data */
	 InvertStep (w,temp_in, temp_out,
		      hLowSize[noct]+hHighSize[noct], sym_ext);
	 
	 /* Copy back to image */ 
	 Copy3 (temp_out+npad, output + (j*hsize), 
	       hLowSize[noct]+hHighSize[noct]);
      }
   }

  Free(hLowSize);
  Free(hHighSize);
  Free(vLowSize);
  Free(vHighSize);
  Free(temp_in);
  Free(temp_out);
}

/* The C function for reconstruction */
void OWt2r (OWTRANS2 wtrans, IMAGE image)
{
  float *temp;
    
  /* We check the fact that the owtrans2 should not be empty */
  CheckOWtrans2(wtrans);

  /* Some Allocation */
  temp = FloatAlloc(wtrans->hsize*wtrans->vsize);
  TempPtr(temp);

  /* put data in Mallat format */
  LinearToMallat (wtrans,temp);

  /* Allocation of reconstructed image */
  SizeImage(image,wtrans->hsize,wtrans->vsize);
  
  /* Let's reconstruct */
  OWtrans2dRecons(wtrans->wavelet,temp, image->pixels, wtrans->hsize, wtrans->vsize,wtrans->noct,-1);
}

/* The corresponding command */
void C_OWt2r(char **argv)
{
  OWTRANS2 wtrans;
  IMAGE image;
  
  argv = ParseArgv(argv,tOWTRANS2_,NULL,&wtrans,-1);
  if (wtrans==NULL) wtrans= GetOWtrans2Cur(); 
  
  argv = ParseArgv(argv,tIMAGE_,wtrans->original,&image,0);

  OWt2r(wtrans,image);
}

