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

/************************************************************/
/*
 *   The (approximate) analytic formulas to compute the
 *   inner product between two (complex and normalized monochannel) 
 *   FOF and EXPONENTIAL (asymmetric) atoms. No formula is
 *   available for chirped atoms with these shapes.
 *   The value of coeffR/coeffI/coeff2/realGG/imagGG/phase/cosPhase/sinPhase 
 *   do not influence the result.
 *   WARNING : never call these functions directly, they should only be called through
 *             CCAtomInnerProduct.
 */
/************************************************************/
/* A useful complex structure */
typedef struct mycomplex {
    float real;
    float imag;
} MYCOMPLEX;


MYCOMPLEX CalculIntegralI1(float u1,float u2,int s1,int s2,float Xi1,float Xi2,float alfa,int q,float t)
{
    
    MYCOMPLEX result; 
    float real;
    float imag;
    float den;
    float cosinus;
    float sinus;
    float expon;
    float a,b,g,h;

    a = -alfa*(s1+s2)/(s1*s2);
    b = (Xi1-Xi2)-2*M_PI*q;
    g = alfa*((u1/s1)+(u2/s2));
    h = Xi2*u2-Xi1*u1;

/*    Printf("\natom1(u1: %f,Xi1: %f,s1: %d)\n",u1,Xi1,s1);
    Printf("atom2(u2: %f,Xi2: %f,s2: %d)\n",u2,Xi2,s2);
    Printf("alfa:%f,a:%f,b:%f,g:%f,h:%f,t:%f\n",alfa,a,b,g,h,t);   */

    cosinus	= cos(b*t+h);
    sinus	= sin(b*t+h);
    expon	= exp(a*t+g);
    den 	= a*a + b*b;

/*    Printf("cos:%f,sin:%f,expon:%f,den:%f\n",cosinus,sinus,expon,den);  */

    real = expon*(a*cosinus + b*sinus)/den;
    imag = expon*(a*sinus - b*cosinus)/den;

/*    Printf("(%f,%f)\n",real,imag);   */

    result.real = real;
    result.imag = imag;
   
/*   Printf("I11:(%f,%f)\n",result.real,result.imag);   */

    return(result);
}

MYCOMPLEX CalculIntegralI234(float u1,float u2,int s1,int s2,float Xi1,float Xi2,float alfa,int q,float d,float e,float t)
{
    
    MYCOMPLEX result; 
    float real1,real2;
    float imag1,imag2;
    float a,b,x,y,g,h;
    float den;
    float cosinus1,cosinus2;
    float sinus1,sinus2;
    float expon;

    a = -alfa*(s1+s2)/(s1*s2);
    b = (Xi1-Xi2) - 2*M_PI*q;
    g = alfa*((u1/s1)+(u2/s2));
    h = Xi2*u2 - Xi1*u1;

/*    Printf("\natom1(u1: %f,Xi1: %f,s1: %d)\n",u1,Xi1,s1);
    Printf("atom2(u2: %f,Xi2: %f,s2: %d)\n",u2,Xi2,s2);  
    Printf("a:%f,b:%f,g:%f,h:%f,d:%f,e:%f,t:%f,q:%d\n",a,b,g,h,d,e,t,q);    */
    
    x = a*a - b*b + d*d;
    y = 2*a*b;
    den = x*x + y*y;

    cosinus1	= cos(d*t-e);
    sinus1	= sin(d*t-e);

    cosinus2	= cos(b*t+h);
    sinus2	= sin(b*t+h);
    expon	= exp(a*t+g);

/*   Printf("cos1:%f,sin1:%f,cos2:%f,sin2:%f,expon:%f,den:%f\n",cosinus1,sinus1,cosinus2,sinus2,expon,den); */ 

    real1 = (a*x + b*y)*cosinus1 + d*x*sinus1;
    imag1 = (b*x - a*y)*cosinus1 - d*y*sinus1;

/*    Printf("real1:%f,imag1:%f\n",real1,imag1); */

    real2 = expon*(real1*cosinus2 - imag1*sinus2)/den;
    imag2 = expon*(real1*sinus2 + imag1*cosinus2)/den;

    result.real = real2;
    result.imag = imag2;

/*    Printf("I23:(%f,%f)\n",result.real,result.imag);   */

    return(result);
}

    
float CalculNormFoF(float alfa,float beta,int s)
{
    float norm,norm2;
    float expon;
    float beta2;
    float num;
    float den;
    float alfa2;

    alfa2 = alfa*alfa;
    beta2 = beta*beta;
    expon = exp((-2*alfa*M_PI)/beta);

    num   = alfa*(4*alfa2 + beta2)*(alfa2 + beta2);
    den   = s*beta2*(8*expon*alfa2 + 5*beta2*expon + 3*beta2);

    norm2 = num/den;

    norm  = 4*sqrt(norm2);

/*    Printf("alfa2:%f,beta2:%f,num:%f,den:%f,norm2:%f,norm:%f\n",alfa2,beta2,num,den,norm2,norm); */
    
    return(norm);
}

/*
 * The formula for FOF ASYMMETRIC WINDOW
 */

void CCAtomAnalyticFoF(const ATOM atom1,const ATOM atom2,float *pReal,float *pImag)		
{
    static float TWO_PI 	= 2*M_PI;
    //    static float precisionTerm	= 0.0;

    float TWO_PI_OVER_GABOR_MAX_FREQID;

   
    MYCOMPLEX I11,I12,I2341,I2342,I2343,I2344,I23451,I23452,I23461,I23462,innProd,innProdUpdate;

    float alfa;
    float d1,d2,e1,e2;
    float c1,c2;
    float u,u1,u2;
    float u1Shift,u2Shift;
    float MAX1,MIN1,MAX2,MAX3;
    int flagCase1;
    float Xi1,Xi2;
    float factor;

    int atom1Size = atom1->windowSize;
    int atom2Size = atom2->windowSize;
      
    /* The limits of the frequency periodization */
  
    float q0;
    float width;

    int qMin;
    int	qMax;
    int q; 


   /* The difference in time, frequency */
    float dXi;
 

    /* The squared scales 	 */
    
    int s,s1,s2; 
    
    /* Initializing */
    TWO_PI_OVER_GABOR_MAX_FREQID	= TWO_PI/GABOR_MAX_FREQID;

    
    /* The difference in time, frequency */
    
    u1  = atom1->timeId;
    u2  = atom2->timeId;
    s1	= atom1Size;
    s2	= atom2Size;
    Xi1 = TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->freqId);
    Xi2 = TWO_PI_OVER_GABOR_MAX_FREQID*(atom2->freqId);

/*    Printf("\n--------------------------------------------------------------------------------\n");
    Printf("\n\t\tatom1(u1: %f,Xi1: %f,s1: %d)\n",u1,Xi1,s1);
    Printf("\t\tatom2(u2: %f,Xi2: %f,s2: %d)\n\n",u2,Xi2,s2); */

    dXi	= Xi1 - Xi2;

    /* Preparing just the needeed variables */

    innProd.real = 0.0;
    innProd.imag = 0.0;

    u1Shift = u1 + M_PI*s1/betaFoF;
    u2Shift = u2 + M_PI*s2/betaFoF;

    /* Damping factor */
  
    alfa = log(decayFoF);
    
    /* L2 Norms */

    c1 = CalculNormFoF(alfa,betaFoF,s1);	
    c2 = CalculNormFoF(alfa,betaFoF,s2);

    /* Integral I4 exists? */

    if ((u2>=u1Shift)||(u1>=u2Shift))
    {
	flagCase1 = NO;
/*	Printf("**********CASE2************"); */
    }
    else
    {
	flagCase1 = YES;
/*	Printf("**********CASE1************"); */
    }	
    if (u2 >= u1)
    {
	MAX3 = u2;
    }
    else 
    {
	MAX3 = u1;
    }

    if (u1Shift>=u2Shift)
    {
	MAX1 	= u1Shift;
	MIN1 	= u2Shift;
	u	= u1;
	s	= s1;
/*	Printf("*****Calcul. with I2*****\n"); */
	if (u1>= u2Shift)
	{
	    MAX2 = u1;
	}
	else
	{
	    MAX2 = u2Shift;
	}
    }
    else
    {
	MAX1 = u2Shift;
	MIN1 = u1Shift;
	u	= u2;
	s	= s2;
/*	Printf("*****Calcul. with I3*****\n"); */
	
	if (u2>= u1Shift)
	{
	    MAX2 = u2;
	}
	else
	{
	    MAX2 = u1Shift;
	}
    }
   
    width = 1;

    q0		= -dXi/TWO_PI;		
    qMin 	= floor(q0-width);
    qMax 	= ceil (q0+width);
    
    factor = 0.5*c1*c2;


/*    Printf("u:%f,s:%d,u1Shift:%f,u2Shift:%f,MAX1:%f,MAX2:%f\n\n",u,s,u1Shift,u2Shift,MAX1,MAX2); */
    for(q  = -width; q <= width; q++)
    {
	I11 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MAX1);
	I12 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MAX2);
	I2341	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s,betaFoF*u/s,MAX1);	
	I2342	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s,betaFoF*u/s,MAX2);

	innProdUpdate.real = factor*(I2342.real - I2341.real - I12.real - I11.real);
	innProdUpdate.imag = factor*(I2342.imag - I2341.imag - I12.imag - I11.imag);

	if ( flagCase1 == YES )
	{	
	    /* Integral I4 */

	    d1 		= betaFoF*(s1-s2)/(s1*s2);
	    d2		= betaFoF*(s1+s2)/(s1*s2);
	    e1		= betaFoF*((u1/s1)-(u2/s2));
	    e2 		= betaFoF*((u1/s1)+(u2/s2));

	    I11 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MIN1);
	    I12 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MAX3);
	    I2341	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s1,betaFoF*u1/s1,MIN1);		      I2342	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s1,betaFoF*u1/s1,MAX3);
	    I2343	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s2,betaFoF*u2/s2,MIN1);
	    I2344	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s2,betaFoF*u2/s2,MAX3);
	    I23451	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d1,e1,MIN1);	
	    I23452	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d2,e2,MIN1);
	    I23461	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d1,e1,MAX3);	
	    I23462	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d2,e2,MAX3);
	    
	    innProdUpdate.real += 0.5*factor*(I11.real-I12.real-I2341.real+I2342.real-I2343.real+I2344.real);
	    innProdUpdate.imag += 0.5*factor*(I11.imag-I12.imag-I2341.imag+I2342.imag-I2343.imag+I2344.imag);
	   
	    innProdUpdate.real += 0.25*factor*(I23451.real+I23452.real-I23461.real-I23462.real);
	    innProdUpdate.imag += 0.25*factor*(I23451.imag+I23452.imag-I23461.imag-I23462.imag);
	    
	}
	
	innProd.real += innProdUpdate.real;
	innProd.imag += innProdUpdate.imag;
   
/*	Printf("q=%d\t",q);
	Printf("I11:(%f,%f)\n",I11.real,I11.imag);
	Printf("I12:(%f,%f)\n",I12.real,I12.imag);
	Printf("I231:(%f,%f)\n",I231.real,I231.imag);
	Printf("I232:(%f,%f)\n",I232.real,I232.imag); 
	Printf("Update:(%.14f,%.14f) \tInnProd:(%.14f,%.14f)\n",innProdUpdate.real,innProdUpdate.imag,innProd.real,innProd.imag);   */

    }
    
	
    *pReal = innProd.real;
    *pImag = innProd.imag;

    // If the atoms are indeed 'real' (i.e. unmodulated) 
    if(atom1->freqId == 0.0 && atom1->chirpId == 0.0 && atom2->freqId == 0.0 && atom2->chirpId == 0.0)
      *pImag = 0.0;
}


/*
 * The formula for EXPONENTIAL ASYMMETRIC WINDOW
 */

void CCAtomAnalyticExponential(const ATOM atom1,const ATOM atom2,float *pReal,float *pImag)		
{
    static float TWO_PI 	= 2*M_PI;
    //    static float precisionTerm	= 0.0;
    MYCOMPLEX I1,innProd;
    
    float TWO_PI_OVER_GABOR_MAX_FREQID;

    float decay=1e4;
    float a,a2;
    float Norm;
    float u,u1,u2;
    float Xi,Xi1,Xi2,MAX1;
    int atom1Size = atom1->windowSize;
    int atom2Size = atom2->windowSize;
  
    double den;
    double phase;
    double a2S1S2Root;

    
    /* The limits of the frequency periodization */
  
    float q0;
    float width;

    int qMin;
    int	qMax;
    int q; 


   /* The difference in time, frequency */
    float du;
    float dXi;
    double dXiLoop;
    float dc;

    /* The squared scales 	 */
    
    float s,s1,s2,sSum,sSum2,sProd,sProd2,sProdRoot;


    /* The same in rectangular coordinates 	*/
    
    double loopFactorR;
    double loopFactorI;
    double factor;


    /* The factors that are computed with a loop on the periodized gaussian */

    double expR = 0.0;
    double expI = 0.0;
    //    double real = 0.0;
    //    double imag = 0.0;
    double Real = 0.0;
    double Imag = 0.0;

    // TODO : actually implement this function !
    Errorf("CCAtomAnalyticExponential not implemented");

    /* Initializing */
    TWO_PI_OVER_GABOR_MAX_FREQID	= TWO_PI/GABOR_MAX_FREQID;

    
    /* The difference in time, frequency */
    
    u1  = atom1->timeId;
    u2  = atom2->timeId;
    Xi1 = TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->freqId);
    Xi2 = TWO_PI_OVER_GABOR_MAX_FREQID*(atom2->freqId);

    Printf("\natom1(u1: %f,Xi1: %f,s1: %d)\n",u1,Xi1,atom1Size);
    Printf("atom2(u2: %f,Xi2: %f,s2: %d)\n",u2,Xi2,atom2Size);

    dXi	= Xi1 - Xi2;
    dc	= TWO_PI_OVER_GABOR_MAX_FREQID*(atom1->chirpId-atom2->chirpId);

    /* Preparing just the needeed variables */

    /* Damping factor */
  
    a = log(decay);
    a2= a*a;

    /* The scale-related variables */
    
    if(atom1->windowSize == atom2->windowSize) {
      s1 = s2 	= atom1Size;
      sSum	= s1+s2;
      sSum2	= sSum*sSum;
      sProd	= s1*s2;
      sProd2	= sProd*sProd;
      sProdRoot	= sqrt(sProd);
    }
    else {
      s1	= atom1Size;
      s2	= atom2Size;
      sSum	= s1+s2;
      sSum2	= sSum*sSum;
      sProd	= s1*s2;
      sProd2	= sProd*sProd;
      sProdRoot	= sqrt(sProd);
    }

    if(u1 >= u2)	/*    MAX(u1,u2)= u1  */ 
    {
      MAX1 = u1;
      u  = u1;
      du = u2 - u1;
      Xi = -Xi2;
      s  = s2;
    }
    else {
      MAX1 = u2;
      u  = u2;
      du = u1 - u2;
      Xi = Xi1;
      s  = s1;
    }

/*    Printf("(du: %f ,dXi: %f ,s: %f ,u: %f ,Xi: %f)\n",du,dXi,s,u,Xi);
    Printf("(a: %f ,sSum: %f ,sProd: %f ,sProdRoot: %f)\n\n",a,sSum,sProd,sProdRoot); */
    

    a2S1S2Root = 2*a*sProdRoot;


    /* The loop bounds */

    /* Width  = scale of freq gaussian */
/*    if(precisionTerm == 0.0)
    {
	precisionTerm = -log(CCATOM_PRECISION*M_PI*sqrt(2.0));
    }
    width	= sqrt(MAX(0,(log(factorAmp/sqrt(loopFactorAmp))+precisionTerm)/loopFactorAmp)); */

    /* Center = max of freq gaussian   */

    width = 50000;

    q0		= -dXi/TWO_PI;		
    qMin 	= floor(q0-width);
    qMax 	= ceil (q0+width);

/*    factorR = cos(Xi*du);
    factorI = sin(Xi*du);  */

    factor = exp(a*du/s);

    phase = qMin*TWO_PI*u;


/*    dXi  += qMin*TWO_PI;  */

    /* Now the loop */

    
    innProd.real = 0.0;
    innProd.imag = 0.0;
    


    for(q  = -width;
	q <= width;
	q++)
    {
	dXiLoop = dXi - (TWO_PI * q);
	phase = Xi*du + (TWO_PI * q * u);
	
	den = (a2*sSum2) + (sProd2*dXiLoop*dXiLoop);
	loopFactorR = (a*a2S1S2Root*sSum)/den;
	loopFactorI = (dXiLoop*sProd*a2S1S2Root)/den;
        expR = cos(phase);
        expI = sin(phase);

/*	real = expR*loopFactorR+expI*loopFactorI;
        imag = expR*loopFactorI-expI*loopFactorR;  */
/*        Printf("(q: %d,dXiLoop: %f,phase: %f,factor: %f)\n",q,dXiLoop,phase,factor);
        Printf("(loopFactorR: %f,loopFactorI: %f,expR: %f,expI: %f)\n",loopFactorR,loopFactorI,expR,expI); */
       

	I1 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,a,q,MAX1);
	innProd.real -= I1.real;
	innProd.imag -= I1.imag;
   /* Complex multiplication with the common exponential factor */
/*	Real += real*factorR+imag*factorI;
	Imag += imag*factorR-real*factorI; */

	Real += factor*(loopFactorR*expR+loopFactorI*expI);
	Imag += factor*(loopFactorI*expR-loopFactorR*expI);
	

/*	Printf("(Real: %f ,Imag: %f)\n\n",Real,Imag);  */
   
    }
    
    Norm = 2*a/sqrt(s1*s2);

    *pReal = Norm*innProd.real;
    *pImag = Norm*innProd.imag;

    // If the atoms are indeed 'real' (i.e. unmodulated) 
    if(atom1->freqId == 0.0 && atom1->chirpId == 0.0 && atom2->freqId == 0.0 && atom2->chirpId == 0.0)
      *pImag = 0.0;
}



void C_AIP(char **argv)    /* javi: Analytic Inner Product Command for FoF atoms */
{
    float u1,u2;
    float f1,f2;
    int s1,s2;
    static float TWO_PI 	= 2*M_PI;
    //    static float precisionTerm	= 0.0;
   
    MYCOMPLEX I11,I12,I2341,I2342,I2343,I2344,I23451,I23452,I23461,I23462,innProd,innProdUpdate;

    float alfa;
    float d1,d2,e1,e2;
    float c1,c2;
    float u;
    float u1Shift,u2Shift;
    float MAX1,MIN1,MAX2,MAX3;
    int flagCase1;
    float Xi1,Xi2;
    float factor;


    
    /* The limits of the frequency periodization */
  
    float q0;
    float width;

    int qMin;
    int	qMax;
    int q; 


   /* The difference in time, frequency and chirprate */
    float dXi;
  

    /* The squared scales 	 */
    
    int s;
    
    /* The difference in time, frequency and chirprate */
    argv = ParseArgv(argv,tFLOAT,&u1,tFLOAT,&u2,tFLOAT,&f1,tFLOAT,&f2,tINT,&s1,tINT,&s2,0);
    

    Xi1 = TWO_PI*f1;
    Xi2 = TWO_PI*f2;

    Printf("beta:%f, decay:%f\n",betaFoF,decayFoF);
    Printf("atom1:(u1: %f,f1: %f,s1: %d)\n",u1,f1,s1);
    Printf("atom2:(u2: %f,f2: %f,s2: %d)\n\n",u2,f2,s2);

    dXi	= Xi1 - Xi2;

    /* Preparing just the needeed variables */

    innProd.real = 0.0;
    innProd.imag = 0.0;

    u1Shift = u1 + M_PI*s1/betaFoF;
    u2Shift = u2 + M_PI*s2/betaFoF;

    /* Damping factor */
  
    alfa = log(decayFoF);
    
    /* L2 Norms */

    c1 = CalculNormFoF(alfa,betaFoF,s1);	
    c2 = CalculNormFoF(alfa,betaFoF,s2);

    /* Integral I4 exists? */

    if ((u2>=u1Shift)||(u1>=u2Shift))
    {
	flagCase1 = NO;
	Printf("***CASE2***");
    }
    else
    {
	flagCase1 = YES;
	Printf("***CASE1***");
    }	
    if (u2 >= u1)
    {
	MAX3 = u2;
    }
    else 
    {
	MAX3 = u1;
    }

    if (u1Shift>=u2Shift)
    {
	MAX1 	= u1Shift;
	MIN1 	= u2Shift;
	u	= u1;
	s	= s1;
	Printf("*****Calcul. with I2*****\n");

	if (u1>= u2Shift)
	{
	    MAX2 = u1;
	}
	else
	{
	    MAX2 = u2Shift;
	}
    }
    else
    {
	MAX1 = u2Shift;
	MIN1 = u1Shift;
	u	= u2;
	s	= s2;
	Printf("*****Calcul. with I3*****\n"); 
	
	if (u2>= u1Shift)
	{
	    MAX2 = u2;
	}
	else
	{
	    MAX2 = u1Shift;
	}
    }
   
    width = 10;

    q0		= -dXi/TWO_PI;		
    qMin 	= floor(q0-width);
    qMax 	= ceil (q0+width);
    
    factor = 0.5*c1*c2;


/*    Printf("u:%f,s:%d,u1Shift:%f,u2Shift:%f,MAX1:%f,MAX2:%f\n\n",u,s,u1Shift,u2Shift,MAX1,MAX2); */
    for(q  = -width; q <= width; q++)
    {
	I11 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MAX1);
	I12 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MAX2);
	I2341	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s,betaFoF*u/s,MAX1);	
	I2342	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s,betaFoF*u/s,MAX2);

	innProdUpdate.real = factor*(I2342.real - I2341.real - I12.real - I11.real);
	innProdUpdate.imag = factor*(I2342.imag - I2341.imag - I12.imag - I11.imag);

	if ( flagCase1 == YES )
	{	
	    /* Integral I4 */

	    d1 		= betaFoF*(s1-s2)/(s1*s2);
	    d2		= betaFoF*(s1+s2)/(s1*s2);
	    e1		= betaFoF*((u1/s1)-(u2/s2));
	    e2 		= betaFoF*((u1/s1)+(u2/s2));

	    I11 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MIN1);
	    I12 	= CalculIntegralI1(u1,u2,s1,s2,Xi1,Xi2,alfa,q,MAX3);
	    I2341	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s1,betaFoF*u1/s1,MIN1);
	    I2342	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s1,betaFoF*u1/s1,MAX3);
	    I2343	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s2,betaFoF*u2/s2,MIN1);
	    I2344	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,betaFoF/s2,betaFoF*u2/s2,MAX3);
	    I23451	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d1,e1,MIN1);	
	    I23452	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d2,e2,MIN1);
	    I23461	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d1,e1,MAX3);	
	    I23462	= CalculIntegralI234(u1,u2,s1,s2,Xi1,Xi2,alfa,q,d2,e2,MAX3);
	    
	    innProdUpdate.real += 0.5*factor*(I11.real-I12.real-I2341.real+I2342.real-I2343.real+I2344.real);
	    innProdUpdate.imag += 0.5*factor*(I11.imag-I12.imag-I2341.imag+I2342.imag-I2343.imag+I2344.imag);
	   
	    innProdUpdate.real += 0.25*factor*(I23451.real+I23452.real-I23461.real-I23462.real);
	    innProdUpdate.imag += 0.25*factor*(I23451.imag+I23452.imag-I23461.imag-I23462.imag);
	    
	}
	
	innProd.real += innProdUpdate.real;
	innProd.imag += innProdUpdate.imag;
   
/*	Printf("q=%d\t",q);
	Printf("I11:(%f,%f)\n",I11.real,I11.imag);
	Printf("I12:(%f,%f)\n",I12.real,I12.imag);
	Printf("I231:(%f,%f)\n",I231.real,I231.imag);
	Printf("I232:(%f,%f)\n",I232.real,I232.imag); 
	Printf("Update:(%.14f,%.14f) \tInnProd:(%.14f,%.14f)\n",innProdUpdate.real,innProdUpdate.imag,innProd.real,innProd.imag);  */

    }
    
	
    Printf(" pREAL:%.12f, pIMAG:%.12f\n",innProd.real,innProd.imag);
}

/* EOF */

