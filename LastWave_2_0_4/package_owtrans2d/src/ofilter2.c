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


/*************************************************
 *
 * The coefficients of the different filters
 *
 *************************************************/
 
/* Haar */ 
#define Sqrt2 1.414213562
static float HaarCoeffs [] = { 1.0/Sqrt2, 1.0/Sqrt2 };

/* A few Daubechies filters */
static float Daub4Coeffs [] = { 0.4829629131445341,  0.8365163037378077,
		          0.2241438680420134, -0.1294095225512603 };
static float Daub6Coeffs [] = { 0.3326705529500825,  0.8068915093110924,
		          0.4598775021184914, -0.1350110200102546,
		         -0.0854412738820267,  0.0352262918857095 };
static float Daub8Coeffs [] = { 0.2303778133088964,  0.7148465705529154,
			  0.6308807679398587, -0.0279837694168599,
			 -0.1870348117190931,  0.0308413818355607,
			  0.0328830116668852, -0.0105974017850690 };

/* Filter from Eero Simoncelli's PhD thesis -- used in Edward Adelson's EPIC wavelet coder
   These are probably the filter coefficients used in Shapiro's EZW paper */
static float AdelsonCoeffs[] = { 0.028220367, -0.060394127, -0.07388188, 
			 0.41394752,   0.7984298,   0.41394752, 
			 -0.07388188, -0.060394127, 0.028220367 };


/* 7/9 Filter from M. Antonini, M. Barlaud, P. Mathieu, and
 I. Daubechies, "Image coding using wavelet transform", IEEE
 Transactions on Image Processing", Vol. pp. 205-220, 1992. */
static float AntoniniSynthesis [] = { -6.453888262893856e-02,
			      -4.068941760955867e-02,
			       4.180922732222124e-01,
			       7.884856164056651e-01,
			       4.180922732222124e-01,
			      -4.068941760955867e-02,
			      -6.453888262893856e-02 };
static float AntoniniAnalysis[] =  {  3.782845550699535e-02,
			     -2.384946501937986e-02,
			     -1.106244044184226e-01,
			      3.774028556126536e-01,
			      8.526986790094022e-01,
			      3.774028556126537e-01,
			     -1.106244044184226e-01,
			     -2.384946501937986e-02,
			      3.782845550699535e-02 };


/* Unpublished 18/10 filter from Villasenor's group */
static float Villa1810Synthesis [] = { 9.544158682436510e-04,
			  -2.727196296995984e-06,
			  -9.452462998353147e-03,
			  -2.528037293949898e-03,
			   3.083373438534281e-02,
			  -1.376513483818621e-02,
			  -8.566118833165798e-02,
			   1.633685405569902e-01,
			   6.233596410344172e-01,
			   6.233596410344158e-01,
			   1.633685405569888e-01,
			  -8.566118833165885e-02,
			  -1.376513483818652e-02,
			   3.083373438534267e-02,
			  -2.528037293949898e-03,
			  -9.452462998353147e-03,
			  -2.727196296995984e-06,
			   9.544158682436510e-04};
static float Villa1810Analysis [] = {  2.885256501123136e-02,
			   8.244478227504624e-05,
			  -1.575264469076351e-01,
			   7.679048884691438e-02,
			   7.589077294537618e-01,
			   7.589077294537619e-01,
			   7.679048884691436e-02,
			  -1.575264469076351e-01,
			   8.244478227504624e-05,
			   2.885256501123136e-02};

/* Filters from Chris Brislawn's tutorial code */
static float BrislawnAnalysis [] = {  0.037828455506995,  -0.023849465019380, 
			       -0.110624404418423,   0.377402855612654,
				0.852698679009403, 
				0.377402855612654,  -0.110624404418423,
			       -0.023849465019380,   0.037828455506995 };
static float BrislawnSynthesis [] = { -0.064538882628938, -0.040689417609558,
				 0.418092273222212, 
				 0.788485616405664,
				 0.418092273222212, 
				-0.040689417609558, -0.064538882628938};
static float Brislawn2Analysis [] = {  0.026913419, -0.032303352,
			        -0.241109818,  0.054100420,
			         0.899506092,  0.899506092, 
			         0.054100420, -0.241109818, 
			        -0.032303352,  0.026913419};
static float Brislawn2Synthesis [] = { 0.019843545,  0.023817599, 
				-0.023257840,  0.145570740,  
				 0.541132748,  0.541132748, 
				 0.145570740, -0.023257840, 
				 0.023817599, 0.019843545 };


/* Filters from J. Villasenor, B. Belzer, J. Liao, "Wavelet Filter
 Evaluation for Image Compression." IEEE Transactions on Image
 Processing, Vol. 2, pp. 1053-1060, August 1995. */
static float Villa1Analysis [] = {
  3.782845550699535e-02,
  -2.384946501937986e-02,
  -1.106244044184226e-01,
  3.774028556126536e-01,
  8.526986790094022e-01,
  3.774028556126537e-01,
  -1.106244044184226e-01,
  -2.384946501937986e-02,
  3.782845550699535e-02
};
static float Villa1Synthesis [] = {
  -6.453888262893856e-02,
  -4.068941760955867e-02,
  4.180922732222124e-01,
  7.884856164056651e-01,
  4.180922732222124e-01,
  -4.068941760955867e-02,
  -6.453888262893856e-02
};
static float Villa2Analysis [] = {
  -8.472827741318157e-03,
  3.759210316686883e-03,
  4.728175282882753e-02,
  -3.347508104780150e-02,
  -6.887811419061032e-02,
  3.832692613243884e-01,
  7.672451593927493e-01,
  3.832692613243889e-01,
  -6.887811419061045e-02,
  -3.347508104780156e-02,
  4.728175282882753e-02,
  3.759210316686883e-03,
  -8.472827741318157e-03
};
static float Villa2Synthesis [] = {
  1.418215589126359e-02,
  6.292315666859828e-03,
  -1.087373652243805e-01,
  -6.916271012030040e-02,
  4.481085999263908e-01,
  8.328475700934288e-01,
  4.481085999263908e-01,
  -6.916271012030040e-02,
  -1.087373652243805e-01,
  6.292315666859828e-03,
  1.418215589126359e-02
};
static float Villa3Analysis [] = {
  -1.290777652578771e-01,
  4.769893003875977e-02,
  7.884856164056651e-01,
  7.884856164056651e-01,
  4.769893003875977e-02,
  -1.290777652578771e-01
};
static float Villa3Synthesis [] = {
  1.891422775349768e-02,
  6.989495243807747e-03,
  -6.723693471890128e-02,
  1.333892255971154e-01,
  6.150507673110278e-01,
  6.150507673110278e-01,
  1.333892255971154e-01,
  -6.723693471890128e-02,
  6.989495243807747e-03,
  1.891422775349768e-02
};
static float Villa4Analysis [] = {
  -1.767766952966369e-01,
  3.535533905932738e-01,
  1.060660171779821e+00,
  3.535533905932738e-01,
  -1.767766952966369e-01
};
static float Villa4Synthesis [] = {
  3.535533905932738e-01,
  7.071067811865476e-01,
  3.535533905932738e-01
};
static float Villa5Analysis [] = {
  7.071067811865476e-01,
  7.071067811865476e-01
};
static float Villa5Synthesis [] = {
  -8.838834764831845e-02,
  8.838834764831845e-02,
  7.071067811865476e-01,
  7.071067811865476e-01,
  8.838834764831845e-02,
  -8.838834764831845e-02
};
static float Villa6Analysis [] = {
  3.314563036811943e-02,
  -6.629126073623885e-02,
  -1.767766952966369e-01,
  4.198446513295127e-01,
  9.943689110435828e-01,
  4.198446513295127e-01,
  -1.767766952966369e-01,
  -6.629126073623885e-02,
  3.314563036811943e-02
};
static float Villa6Synthesis [] = {
  3.535533905932738e-01,
  7.071067811865476e-01,
  3.535533905932738e-01
};



/* Other Filter */
static float OdegardAnalysis[] = {
   5.2865768532960523e-02,
  -3.3418473279346828e-02,
  -9.3069263703582719e-02,
   3.8697186387262039e-01,
   7.8751377152779212e-01,
   3.8697186387262039e-01,
  -9.3069263703582719e-02,
  -3.3418473279346828e-02,
   5.2865768532960523e-02
};
static float OdegardSynthesis[] = {
  -8.6748316131711606e-02,
  -5.4836926902779436e-02,
   4.4030170672498536e-01,
   8.1678063499210640e-01,
   4.4030170672498536e-01,
  -5.4836926902779436e-02,
  -8.6748316131711606e-02
};




/*************************************************
 *
 * Functions that deals with OFilter2 structure
 *
 *************************************************/
 
static OFILTER2 NewOFilter2(void)
{
  OFILTER2 filter;
  
  filter = (OFILTER2) Malloc(sizeof(struct ofilter2));
  
  filter->coeff = NULL;
  filter->size = 0;
  filter->firstIndex = 0;
  
  return(filter);
}

static void InitOFilter2(OFILTER2 filter, int filterSize, int filterFirst, float *data)
{
  int i;
  filter->size = filterSize;
  filter->firstIndex = filterFirst;
  filter->center = -filter->firstIndex;

  filter->coeff = FloatAlloc(filter->size);
  if (data != NULL) {
    for (i = 0; i < filter->size; i++) filter->coeff[i] = data[i];
  } else {
    for (i = 0; i < filter->size; i++) filter->coeff[i] = 0;
  }
}

static void CopyOFilter2(OFILTER2 in,OFILTER2 out)
{
  if (out->coeff != NULL) Free(out->coeff);
  InitOFilter2(out,in->size,in->firstIndex,in->coeff);
}

static void DeleteOFilter2(OFILTER2 filter)
{
  if (filter->coeff != NULL) Free(filter->coeff);
  Free(filter);
}
  
  
  
/*************************************************
 *
 * Function that deals with OFilterSet structures
 *
 *************************************************/

static OFILTERSET NewOFilterSet(void)
{
  OFILTERSET filter;
  
  filter = (OFILTERSET) Malloc(sizeof(OFilterSet));
  
  filter->symmetric = NO;
  filter->analysisLow = filter->analysisHigh = filter->synthesisLow = filter->synthesisHigh = NULL;
  
  return(filter);
}

static void InitOFilterSet (OFILTERSET filterset, int symmetric,float *anLow, int anLowSize, int anLowFirst,  
	                       float *synLow, int synLowSize, int synLowFirst)
{
  int i, sign;

  filterset->symmetric = symmetric;

  filterset->analysisLow = NewOFilter2();
  InitOFilter2(filterset->analysisLow,anLowSize, anLowFirst, anLow);


  /* If no synthesis coeffs are given, assume wavelet is orthogonal */
  if (synLow == NULL)  {
    filterset->synthesisLow = NewOFilter2();
    CopyOFilter2(filterset->analysisLow,filterset->synthesisLow);

    /* For orthogonal wavelets, compute the high pass filter using
     the relation g_n = (-1)^n h_{1-n}^*
     (or equivalently g_{1-n} = (-1)^{1-n} h_n^*) */

    filterset->analysisHigh = NewOFilter2();
    InitOFilter2(filterset->analysisHigh,filterset->analysisLow->size, 2 - filterset->analysisLow->size -
				filterset->analysisLow->firstIndex,NULL);
      
    /* Compute (-1)^(1-n) for first n */
    if (filterset->analysisLow->firstIndex % 2) sign = 1;
    else sign = -1;
    
    for (i = 0; i < filterset->analysisLow->size; i++)  {
      filterset->analysisHigh->coeff[1 - i - filterset->analysisLow->firstIndex - 
		                             filterset->analysisHigh->firstIndex] = 
	       sign * filterset->analysisLow->coeff[i];
      sign *= -1;
    }

    /* Copy the high pass analysis filter to the synthesis filter */
    filterset->synthesisHigh = NewOFilter2();
    CopyOFilter2(filterset->analysisHigh,filterset->synthesisHigh);
    
  } else {
  
    /* If separate synthesis coeffs given, assume biorthogonal */
    
    filterset->synthesisLow = NewOFilter2();
    InitOFilter2(filterset->synthesisLow,synLowSize, synLowFirst, synLow);

    /* For orthogonal wavelets, compute the high frequency filter using
     the relation g_n = (-1)^n complement (h~_{1-n}) and
                  g~_n = (-1)^n complement (h_{1-n})
     (or equivalently g_{1-n} = (-1)^{1-n} complement (h~_n)) */
    
    filterset->analysisHigh = NewOFilter2();
    InitOFilter2(filterset->analysisHigh,filterset->synthesisLow->size, 2 - filterset->synthesisLow->size -
			      filterset->synthesisLow->firstIndex,NULL);
    
    /* Compute (-1)^(1-n) for first n */
    if (filterset->synthesisLow->firstIndex % 2) sign = 1;
    else sign = -1;
    
    for (i = 0; i < filterset->synthesisLow->size; i++)  {
      filterset->analysisHigh->coeff[1 - i - filterset->synthesisLow->firstIndex -
			                         filterset->analysisHigh->firstIndex] = 
	      sign * filterset->synthesisLow->coeff[i];
      sign *= -1;
    }

    filterset->synthesisHigh = NewOFilter2();
    InitOFilter2(filterset->synthesisHigh,filterset->analysisLow->size, 2 - filterset->analysisLow->size -
			    filterset->analysisLow->firstIndex,NULL); 

    /* Compute (-1)^(1-n) for first n */
    if (filterset->analysisLow->firstIndex % 2) sign = 1;
    else sign = -1;

    for (i = 0; i < filterset->analysisLow->size; i++)  {
      filterset->synthesisHigh->coeff[1 - i - filterset->analysisLow->firstIndex -
			                          filterset->synthesisHigh->firstIndex] = 
	     sign * filterset->analysisLow->coeff[i];
      sign *= -1;
    }
  }
}

static void CopyOFilterSet (OFILTERSET in, OFILTERSET out)
{
  out->symmetric = in->symmetric;
  CopyOFilter2(in->analysisLow,out->analysisLow);
  CopyOFilter2(in->analysisHigh,out->analysisHigh);
  CopyOFilter2(in->synthesisLow,out->synthesisLow);
  CopyOFilter2(in->synthesisHigh,out->synthesisHigh);
}
 
static void DeleteOFilterSet(OFILTERSET filter)  
{
  DeleteOFilter2(filter->analysisLow);
  DeleteOFilter2(filter->analysisHigh);
  DeleteOFilter2(filter->synthesisLow);
  DeleteOFilter2(filter->synthesisHigh);
}
    
  
    
/********************************************
 *
 * Initializing the different OFilterSets
 *
 ********************************************/


static OFILTERSET Haar = NULL;
static OFILTERSET Daub4, Daub6, Daub8, Antonini, Villa, Adelson,
                  Brislawn, Brislawn2, Villa1, Villa2, Villa3, Villa4, Villa5, Villa6, Odegard;

static void InitOFilters2d(void)
{
  Haar = NewOFilterSet();
  InitOFilterSet (Haar,NO,HaarCoeffs,2,0,NULL,0,0);

  Daub4 = NewOFilterSet();
  InitOFilterSet (Daub4,NO,Daub4Coeffs,4,0,NULL,0,0);
  Daub6 = NewOFilterSet();
  InitOFilterSet (Daub6,NO,Daub6Coeffs,6,0,NULL,0,0);
  Daub8 = NewOFilterSet();
  InitOFilterSet (Daub8,NO,Daub8Coeffs,8,0,NULL,0,0);

  Antonini = NewOFilterSet();
  InitOFilterSet (Antonini,YES,AntoniniAnalysis,9,-4,AntoniniSynthesis,7,-3);

  Villa = NewOFilterSet();
  InitOFilterSet (Villa,YES,Villa1810Analysis,10,-4,Villa1810Synthesis,18,-8);

  Adelson = NewOFilterSet();
  InitOFilterSet (Adelson,YES,AdelsonCoeffs,9,-4,NULL,0,0);

  Brislawn = NewOFilterSet();
  InitOFilterSet (Brislawn,YES,BrislawnAnalysis,9,-4,BrislawnSynthesis,9,-3);
  Brislawn2 = NewOFilterSet();
  InitOFilterSet (Brislawn2,YES,Brislawn2Analysis,10,-4,Brislawn2Synthesis,10,-4);
  
  Villa1 = NewOFilterSet();
  InitOFilterSet (Villa1,YES,Villa1Analysis,9,-4,Villa1Synthesis,7,-3);
  Villa2 = NewOFilterSet();
  InitOFilterSet (Villa2,YES,Villa2Analysis,13,-6,Villa2Synthesis,11,-5);
  Villa3 = NewOFilterSet();
  InitOFilterSet (Villa3,YES,Villa3Analysis,6,-2,Villa3Synthesis,10,-4);
  Villa4 = NewOFilterSet();
  InitOFilterSet (Villa4,YES,Villa4Analysis,5,-2,Villa4Synthesis,3,-1);
  Villa5 = NewOFilterSet();
  InitOFilterSet (Villa5,YES,Villa5Analysis,2,0,Villa5Synthesis,6,-2);
  Villa6 = NewOFilterSet();
  InitOFilterSet (Villa6,YES,Villa6Analysis,9,-4,Villa6Synthesis,3,-1);

  Odegard = NewOFilterSet();
  InitOFilterSet (Odegard,YES,OdegardAnalysis,9,-4,OdegardSynthesis,7,-3);  
}


/***************************************************
 *
 * Functions that deal with OWavelet2 structures
 *
 ***************************************************/

char defaultO2WaveletName[40];

/*
 * Just copy the fields of a filterset into a wavelet
 */
static void LazycopyOWavelet2 (OFILTERSET filterset,OWAVELET2 w)
{ 
  w->symmetric = filterset->symmetric;
  w->analysisLow =filterset->analysisLow;
  w->analysisHigh = filterset->analysisHigh;
  w->synthesisLow = filterset->synthesisLow;
  w->synthesisHigh = filterset->synthesisHigh;
}

/*
 * Create a new wavelet from a filter set name
 */
OWAVELET2  NewOWavelet2(char * nameFilterset)
{
  OWAVELET2 w;
   
  if (Haar == NULL)  InitOFilters2d();
    
  w = (OWAVELET2) Malloc(sizeof(struct owavelet2));
  

  if (!strcmp(nameFilterset,"Haar")) LazycopyOWavelet2(Haar,w);
  else if (!strcmp(nameFilterset,"Daub4")) LazycopyOWavelet2(Daub4,w);
  else if (!strcmp(nameFilterset,"Daub6")) LazycopyOWavelet2(Daub6,w);
  else if (!strcmp(nameFilterset,"Daub8")) LazycopyOWavelet2(Daub8,w);
  else if (!strcmp(nameFilterset,"Antonini")) LazycopyOWavelet2(Antonini,w);
  else if (!strcmp(nameFilterset,"Villa")) LazycopyOWavelet2(Villa,w);
  else if (!strcmp(nameFilterset,"Adelson")) LazycopyOWavelet2(Adelson,w);
  else if (!strcmp(nameFilterset,"Brislawn")) LazycopyOWavelet2(Brislawn,w);
  else if (!strcmp(nameFilterset,"Brislawn2")) LazycopyOWavelet2(Brislawn2,w);
  else if (!strcmp(nameFilterset,"Villa1")) LazycopyOWavelet2(Villa1,w);
  else if (!strcmp(nameFilterset,"Villa2")) LazycopyOWavelet2(Villa2,w);
  else if (!strcmp(nameFilterset,"Villa3")) LazycopyOWavelet2(Villa3,w);
  else if (!strcmp(nameFilterset,"Villa4")) LazycopyOWavelet2(Villa4,w);
  else if (!strcmp(nameFilterset,"Villa5")) LazycopyOWavelet2(Villa5,w);
  else if (!strcmp(nameFilterset,"Villa6")) LazycopyOWavelet2(Villa6,w);
  else if (!strcmp(nameFilterset,"Odegard")) LazycopyOWavelet2(Odegard,w);
  else Errorf("NewOWavelet2() : Unknown name of wavelet");
     
  /* amount of space to leave for padding vectors for symmetric extensions */
  w->npad = MAX(w->analysisLow->size, w->analysisHigh->size);
  
  strcpy(w->name,nameFilterset);
  
  return(w);
}

/*
 * Delete a wavelet 
 */
void DeleteOWavelet2(OWAVELET2 w)
{
  Free(w);
}

/* Setting the default wavelet */
void OWt2f(char *str)
{
  OWAVELET2 w;

  w = NewOWavelet2(str);
  DeleteOWavelet2(w);
  
  strncpy(defaultO2WaveletName,str,15);
  defaultO2WaveletName[14] = '\0';
}

void C_OWt2f (char **argv)
{
  char *str;
  
  if (*argv == NULL) {
    SetResultStr(defaultO2WaveletName);
    return;
  }
  
  argv = ParseArgv(argv,tSTR,&str,0);
 
  OWt2f(str);    
}

void InitOWavelet2(void)
{
  strcpy(defaultO2WaveletName,"Haar");
}
