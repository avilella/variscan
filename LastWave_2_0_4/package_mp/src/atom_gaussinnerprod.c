/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 2000 Remi Gribonval, Emmanuel Bacry and Javier Abadia */
/*      email  : remi.gribonval@inria.fr                                    */
/*               lastwave@cmapx.polytechnique.fr                            */
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
#include "atom.h"


extern void GetTabWindowFactor(char windowShape,unsigned long windowSize,float *windowFactorPtr);

/************************************************************/
/*
 *   The (approximate) analytic formula to compute the
 *   inner product between two (complex and normalized monochannel) 
 *   GAUSSIAN CHIRPED atoms.
 *   The value of coeffR/coeffI/coeff2/realGG/imagGG/phase/cosPhase/sinPhase 
 *   do not influence the result.
 *   WARNING : never call this function directly, it should only be called through
 *             CCAtomInnerProduct.
 */
/************************************************************/
void CCAtomAnalyticGauss(const ATOM atom1,const ATOM atom2,float *pReal,float *pImag)
{
    static float TWO_PI 	= 2*M_PI;
    static float precisionTerm	= 0.0;

    float TWO_PI_OVER_GABOR_MAX_FREQID;

    /* The normalization factors */
    float K1;
    float K2;
    int octave1,octave2;

    /* The difference in time, frequency and chirprate */
    float du;
    float dXi;
    float dc;

    /* The squared scales 	 */
    float s1,s2,sSum;

    /* The quotient of those entities 		*/
    float sDiffQuotient;
    float sProdQuotient;
    float sProdQuotientTimeDc;

    /* The center of the frequential gaussian	*/
    float XiMeanR;
    float XiMeanI;

    /* The difference in frequency "relative"
     * to the frequency gaussian */
    float dXiR;
    float dXiI;

    /* The squared denominator that is common to many terms */
    float halfDen2;
    float den2;

    /* The factors that are used through the loop */
    float loopFactorAmp;
    float loopFactorFreq;
    float loopFactorFreq2;

    /* The \pi/\alpha in polar coordinates 	*/
    float rho;
    float theta;

    /* The common factor in polar coordinates	*/
    float factorLogAmp;
    float factorAmp;
    float factorPhase;
    
    /* The same in rectangular coordinates 	*/
    float factorR;
    float factorI;

    /*
     * The limits of the frequency periodization
     */
    float q0;
    float width;

    int qMin;
    int	qMax;
    int q;

    /* The factors that are computed with a loop on the periodized gaussian */
    float real,imag,amp;
    float expR = 0.0,expI = 0.0;

    /* Initializing */
    TWO_PI_OVER_GABOR_MAX_FREQID	= TWO_PI/GABOR_MAX_FREQID;

    octave1 = (int) (log(atom1->windowSize)/log(2.0)+0.5);
    if(1<<octave1 != atom1->windowSize)
      Errorf("CCAtomAnalyticGauss : windowSize is not a power of two!");
    octave2 = (int) (log(atom2->windowSize)/log(2.0)+0.5);
    if(1<<octave2 != atom2->windowSize)
      Errorf("CCAtomAnalyticGauss : windowSize is not a power of two!");

    /* The normalization factors */
    GetTabWindowFactor(GaussWindowShape,atom1->windowSize,&K1);
    GetTabWindowFactor(GaussWindowShape,atom2->windowSize,&K2);
    
    /* The difference in time, frequency and chirprate */
    du	= atom1->timeId-atom2->timeId;
    dXi	= TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->freqId -atom2->freqId);
    dc	= TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->chirpId-atom2->chirpId);

    /* Preparing just the needeed variables */

    /* The scale-related variables */
    if(atom1->windowSize == atom2->windowSize) {
      /* windowSize^2 */
      s1 = s2 	= atom1->windowSize*atom1->windowSize;
      sSum	= s1+s2;
      /* sigma^2 * (windowSize)^4/(2*(windowSize)^2) */
      sProdQuotient 	= theGaussianSigma2*0.5*s1;
      sDiffQuotient	= 0.0;
    }
    else {
      s1		= atom1->windowSize*atom1->windowSize;
      s2		= atom2->windowSize*atom2->windowSize;
      sSum		= s1+s2;
      sProdQuotient 	= theGaussianSigma2*(s1*s2)/sSum;
      sDiffQuotient	= (s2-s1)/sSum;
    }
/*    Printf("o1 %d -> %d, o2 %d -> %d\n",octave1,s1,octave2,s2);
      Printf("sum %d, prod : %g, diff %g\n",sSum,sProdQuotient,sDiffQuotient);
*/
    /* The mean frequencies */
    if(du == 0.0)
    {
	XiMeanR 	= XiMeanI = 0.0;
	dXiR		= dXiI	  = dXi;
    }
    else 
    {
	if(dc == 0.0)
	{
	    XiMeanR = XiMeanI	= TWO_PI_OVER_GABOR_MAX_FREQID*atom1->chirpId*du;
	}
	else 
	{
	    XiMeanI = TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->chirpId+atom2->chirpId)*du/2;
	    if(s1==s2)
	    {
		XiMeanR	= XiMeanI;
	    }
	    else
	    {
		XiMeanR	= TWO_PI_OVER_GABOR_MAX_FREQID*(s1*atom1->chirpId+s2*atom2->chirpId)*du/sSum;
	    }
	}
	dXiR		= dXi-XiMeanR;
	dXiI		= dXi-XiMeanI;
    }

    /* The factors den2,rho,theta and loopFreq2 */
    if(dc == 0.0)
    {
	den2	       	= 2;

	rho		= TWO_PI*sProdQuotient;
	theta		= 0.0;

	loopFactorFreq2 = 0.0;
    }
    else
    {
	/* Auxiliary variables */
	sProdQuotientTimeDc	= sProdQuotient*dc;
	halfDen2		= 1+sProdQuotientTimeDc*sProdQuotientTimeDc;
	
	/* The useful ones */
	den2		= 2*halfDen2;

	rho		= TWO_PI*sProdQuotient/sqrt(halfDen2);
	theta		= atan(sProdQuotientTimeDc);

	loopFactorFreq2 = sProdQuotientTimeDc*sProdQuotient/den2;

    }

    /* The loopFactorAmp */
    loopFactorAmp	= sProdQuotient/den2;


    /* The factorLogAmp, factorPhase, loopFactorFreq */
    if(du == 0.0)
    {
	factorLogAmp	= log(rho)/2;
	factorPhase	= theta/2;

	loopFactorFreq	= 0.0;

    }
    else
    {
	factorLogAmp	= log(rho)/2	/* Contribution of \sqrt{\pi/\alpha} */
	    -du*du/(2*theGaussianSigma2*sSum);	/* Contribution of the time gaussian */	

	factorPhase	= theta/2
	    -TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->freqId+atom2->freqId)*du/2;

	if(s1 == s2 && dc != 0.0)
	{
	    factorPhase	+= dc*du*du/8; 
	}
	else if(dc != 0.0)
	{
	    factorPhase	+= dc*du*du*(1+2*sDiffQuotient*sDiffQuotient/den2)/8; 
	}


	if(s1 == s2)
	{
	    loopFactorFreq = 0.0;
	}
	else
	{
	    loopFactorFreq  =  du*sDiffQuotient/den2;
	}

    }
    

    /* The common factor in polar coordinates */
    factorAmp	= K1*K2*exp(factorLogAmp);
    factorR 	= factorAmp*cos(factorPhase);
    factorI 	= factorAmp*sin(factorPhase);

    /* The loop bounds */

    /* Width  = scale of freq gaussian */
    if(precisionTerm == 0.0)
    {
	precisionTerm = -log(CCATOM_PRECISION*M_PI*sqrt(2.0));
    }
    width	= sqrt(MAX(0,(log(factorAmp/sqrt(loopFactorAmp))+precisionTerm)/loopFactorAmp));
    /* Center = max of freq gaussian   */
    q0		= -dXiR/TWO_PI;		
    qMin 	= floor(q0-width);
    qMax 	= ceil (q0+width);


/*    Printf("XiMeanR %g XiMeanI %g\n",XiMeanR,XiMeanI);
      Printf("width %g\n",width);
      Printf("min max %d %d\n",qMin,qMax);*/


    dXi  += qMin*TWO_PI;
    dXiR += qMin*TWO_PI;
    dXiI += qMin*TWO_PI;

    /* Now the loop */
    for(q  = qMin;
	q <= qMax;
	q++,dXi += TWO_PI,dXiR  += TWO_PI,dXiI  += TWO_PI)
    {
	/* The amplitude */
	real = -loopFactorAmp*dXiR*dXiR;
	amp  = exp(real);

	/* The phase (a-b*dXiI)*dXiI */
	imag = loopFactorFreq;

	/* We avoid one multiplication if possible */
	if(dc != 0.0)
	{
	    imag -= loopFactorFreq2*dXiI;
	}
	imag *= dXiI;

	/* Adding the term */
	expR	+= amp*cos(imag);
	expI 	+= amp*sin(imag);
    }

    /* Complex multiplication with the common exponential factor */
    *pReal	= expR*factorR-expI*factorI;
    *pImag	= expR*factorI+expI*factorR;

    // If the atoms are indeed 'real' (i.e. unmodulated) 
    if(atom1->freqId == 0.0 && atom1->chirpId == 0.0 && atom2->freqId == 0.0 && atom2->chirpId == 0.0)
      *pImag = 0.0;
}




/* EOF */

