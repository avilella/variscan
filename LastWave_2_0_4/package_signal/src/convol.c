/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0.3                     */
/*                                                                          */
/*      Copyright (C) 1998-2002 Xavier Suraud, Emmanuel Bacry.              */
/*      email : lastwave@cmap.polytechnique.fr                              */
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
#include "signals.h"

#define FLOAT2INT(X) ( ((X) >=0) ? (int)((X) + .5) : (int)((X) - .5))



void extraction (SIGNAL signal, SIGNAL out, int indice, int size2)
  {
   int k, cut;
       
   cut = ((indice + 1)*size2 - 1) - (signal->size - 1);
   cut =  cut > 0 ? cut : 0;
 
   memcpy(out->Y, (signal->Y)+(indice*size2), (size2 - cut)*sizeof(float));

   for (k = size2 - cut; k< 2*size2; k++)
	out->Y[k] = 0;

  }

void multiplication (SIGNAL signal1Real, SIGNAL signal1Im, SIGNAL signal2Real, SIGNAL signal2Im)
  { 
     int     n;
     float   real1;

     if (signal1Real->size != signal2Real->size || signal1Im->size !=  signal2Im->size)
       Errorf("C_multiplication: les tailles doivent etres egales");
     
     for (n=0; n < signal1Real->size; n++)
        {
	real1             = signal1Real->Y[n];
        signal1Real->Y[n] = real1 * signal2Real->Y[n] - signal1Im->Y[n] * signal2Im->Y[n];
        signal1Im->Y[n]   = real1 * signal2Im->Y[n]   + signal1Im->Y[n] * signal2Real->Y[n];
        }
  }
      


void rapide (SIGNAL signal1, SIGNAL signal2, SIGNAL out)
  {
    int     size2;
    int     begin, end, u, v, indice, n;
    double  R;
    SIGNAL  b, FfilterIm, FfilterReal, Fa_rReal, Fa_rIm;

 
     n           = (signal2->size == 1) ? 1 : ceil(log(signal2->size)/log(2)); 
     size2       =  ((signal2->size < 1<<n) && (signal2->size > (1<<(n-1)))) ? (1<<n): (1<<n-1);          
     SizeSignal(out, signal1->size + signal2->size, YSIG);
     R           = ceil((double)signal1->size / (double)size2);
     b           = NewSignal();
     SizeSignal(b, 2*size2, YSIG);
     b->dx       = signal1->dx;
     extraction (signal2, b, 0, size2);
     FfilterReal = NewSignal();
     FfilterIm   = NewSignal();
     Fft(b, NULL, FfilterReal, FfilterIm, 1, NO);

     // WriteSigFile(FfilterReal,"totof1",NO, "y",YES);
     // WriteSigFile(FfilterIm,  "totof2",NO, "y",YES);
          
     Fa_rReal  = NewSignal();
     Fa_rIm    = NewSignal();
        
     for (n = 0; n < signal1->size + signal2->size ; n ++)
            out->Y[n] = 0;

     for (indice = 0; indice < R; indice ++)
	{
           extraction (signal1, b, indice, size2);
           Fft (b, NULL, Fa_rReal, Fa_rIm, 1, NO);
           multiplication (Fa_rReal, Fa_rIm, FfilterReal, FfilterIm);
           Fft (Fa_rReal, Fa_rIm, b, NULL, -1, NO);
           //WriteSigFile(b,"totos",NO, "y",NO);
           begin = indice * size2;
           u     = (2 + indice) * size2 - 1;
           v     = out->size - 1;
           end = MIN(u,v);
	   for (n=begin; n<=end; n++)  
                 out->Y[n] += b->Y[n-indice*size2];
	}
          
     DeleteSignal(Fa_rReal);
     DeleteSignal(Fa_rIm);
     DeleteSignal(b);
     DeleteSignal(FfilterReal);
     DeleteSignal(FfilterIm);     
  } 
 
  
void  FFTConvolution (SIGNAL signal1, SIGNAL signal2, SIGNAL out, int borderType, float x_begin_conv, float x_end_conv)
{
    int     size_conv, size;
    int     n, n1, n2, nmk ,i ,div;
    float   step, endfilter;
    float   firstpoint, minsupport, maxsupport;    
    SIGNAL  tempout, out1;


   tempout    = NewSignal();
   out1       = NewSignal();
   step       = signal1->dx;
   endfilter  = (float)(signal2->size - 1)*step;
   
   /* le support  total du signal qu'il faudra balayer ???*/
   minsupport = (x_begin_conv - signal1->x0 - (signal2->x0 + endfilter))/step;
   maxsupport = (x_end_conv   - signal1->x0 - signal2->x0)/step;
   n1         = FLOAT2INT(minsupport);
   n2         = FLOAT2INT(maxsupport);
   size       = FLOAT2INT((x_end_conv - x_begin_conv)/step + 1);
   //Printf("n1rapide=%d pour %g , n2rapide=%d\n", n1, step, n2);

   switch (borderType){

       case  Border0:
    	   //size        =  (x_end_conv - x_begin_conv)/step + 1;   
           SizeSignal(out, size, YSIG);
           out->dx     = step;
           size_conv   = n2 - n1 + 1;        
           SizeSignal(tempout, size_conv , YSIG);
           tempout->dx = step;  
           for (n=0, i=n1 ; n < size_conv, i < n1 + size_conv; n++, i++)
	       tempout->Y[n] = i<0 ? 0: i>=signal1->size ? 0 : signal1->Y[i];
           rapide (tempout, signal2, out1); 
           DeleteSignal(tempout);           
           memcpy(out->Y, out1->Y + signal2->size - 1, size*sizeof(float));
           DeleteSignal(out1);            
           out->x0     = x_begin_conv;
           out->firstp = MAX(0,-n1+signal1->firstp);            
           out->lastp  = out->size-1-MAX(0,n2-signal1->lastp); 
           break;

       case  BorderCon:
	   //size      = (x_end_conv - x_begin_conv)/step + 1;
           SizeSignal(out, size, YSIG);
           out->dx     = step;
           size_conv   = n2 - n1 + 1; 
           SizeSignal(tempout, size_conv, YSIG);
           //Printf ("size = %d, size_conv= %d\n\n", size, size_conv);
           tempout->dx = step;
           for (n=0, i=n1; n < size_conv, i < n1 + size_conv; n++, i++)
	       tempout->Y[n] =  i<0 ? signal1->Y[0]: i>=signal1->size ? signal1->Y[signal1->size-1] : signal1->Y[i];
           rapide (tempout, signal2, out1);
           DeleteSignal(tempout);           
           memcpy(out->Y, out1->Y + signal2->size - 1 , size*sizeof(float));
           DeleteSignal(out1);           
           out->x0     = x_begin_conv;
           out->firstp = MAX(0,-n1+signal1->firstp);
           out->lastp  = out->size-1-MAX(0,n2-signal1->lastp);
           //WriteSigFile(out,"totos",NO, "y",NO);
       	   break;
     
       case  BorderPer:      
	   //size       = (x_end_conv - x_begin_conv)/step + 1;
           SizeSignal(out, size, YSIG);
           out->dx     = step;
           size_conv   = n2 - n1 + 1;
           SizeSignal(tempout, size_conv, YSIG);
           tempout->dx = step; 
           for (n=0; n < size_conv; n++)
 	     {
                  i              = (n+n1) < 0 ? ((-(n+n1))/signal1->size+1)*signal1->size + (n+n1): n+n1;
                  tempout->Y[n]  = signal1->Y[i%signal1->size];
	     }
           rapide (tempout, signal2, out1);
           DeleteSignal(tempout); 
           //WriteSigFile(tempout,"totos",NO, "y",NO);           
           memcpy(out->Y, out1->Y + signal2->size - 1, size*sizeof(float));
           DeleteSignal(out1);           
           out->x0     = x_begin_conv;
           out->firstp = MAX(0,-n1+signal1->firstp);
           out->lastp  = out->size-1-MAX(0,n2-signal1->lastp);
           break;

       case  BorderMir: 
           // sans rajout de point
	 //size        = (x_end_conv - x_begin_conv)/step + 1;
           SizeSignal(out, size, YSIG);
           out->dx     = step;
           size_conv   = n2 - n1 + 1;
           SizeSignal(tempout, size_conv, YSIG);
           tempout->dx =  step;
           for (n=0; n < size_conv; n++)
	     {
                   i             = (n + n1) < 0 ? 2*((-(n + n1))/signal1->size+1)*(signal1->size) + (n + n1) : n+ n1;
                   div           = signal1->size == 1 ? 0 : i % (2*signal1->size - 2);
		   tempout->Y[n] = (0 <= div && div < signal1->size) ? signal1->Y[div] : signal1->Y[2*signal1->size - 2 - div];   
             }
           rapide (tempout, signal2, out1);
           //WriteSigFile(out1,"totos",NO, "y",NO);
           DeleteSignal(tempout);           
           memcpy(out->Y, out1->Y + signal2->size - 1, size*sizeof(float));
           DeleteSignal(out1);           
           out->x0     = x_begin_conv;
           out->firstp = MAX(0,-n1+signal1->firstp);
           out->lastp  = out->size-1-MAX(0,n2-signal1->lastp);
           
           break;

        case  BorderMir1:
           //  point en plus aux extremites
	   //size      = (x_end_conv - x_begin_conv)/step + 1;
           SizeSignal(out, size, YSIG);
           out->dx   = step;
           size_conv = n2 - n1 + 1;
	   //Printf ("size = %d, size_conv= %d\n\n", size, size_conv);
           SizeSignal(tempout, size_conv, YSIG);
           for (n=0; n < size_conv; n++)
	       {
                   i             = (n+n1)  < 0 ? 2*((-(n+n1))/signal1->size+1)*(signal1->size) + (n+n1): (n+n1); 
                   div           = i % (2*signal1->size - 1 );
		   tempout->Y[n] = (0 <= div && div < signal1->size) ? signal1->Y[div] : signal1->Y[2*signal1->size - 1 - div];   
	       }
	   rapide (tempout, signal2, out1);
           //WriteSigFile(out1,"totos",NO, "y",NO);
           DeleteSignal(tempout);           
           memcpy(out->Y, out1->Y + signal2->size - 1, size*sizeof(float));
           DeleteSignal(out1);           
           out->x0     = x_begin_conv;
           out->firstp = MAX(0,-n1+signal1->firstp);
           out->lastp  = out->size-1-MAX(0,n2-signal1->lastp);
           
           break;           
	   }
}
  

void  DirectConvolution (SIGNAL signal1,SIGNAL signal2, SIGNAL signalOut, int borderType, float x_begin_conv, float x_end_conv)
{ 
    int     size_conv, begin, end, a, n1, n2;
    int     k, n, nmk, i, div;
    float   sum, step, minsupport, maxsupport;
   
 
   step          = signal1->dx;
   signalOut->dx =  step;  
   minsupport    = (x_begin_conv - (signal2->x0 + (signal2->size - 1)*step) - signal1->x0)/step;
   n1            = FLOAT2INT(minsupport);  
   maxsupport    =  (x_end_conv   - (signal2->x0 + (signal2->size - 1)*step)  -signal1->x0)/step;
   n2            = FLOAT2INT(maxsupport);  
   //Printf("n1lente=%d, n2lente=%d \n", n1, n2);

   switch (borderType) {

       case  Border0:
           size_conv         = n2 - n1 + 1 ;
           SizeSignal(signalOut, size_conv, YSIG)    ;
           signalOut->firstp = MAX(0,-n1+signal1->firstp);
           signalOut->lastp  = signalOut->size-1-MAX(0,n2+signal2->size-1-signal1->lastp);
           signalOut->x0     = x_begin_conv;            
           for (n=0; n < size_conv; n++)
	     {
	       sum   = 0;
               a     = n + signal2->size - 1 + n1;  
               begin =   a - signal1->size + 1;
               begin = MAX(begin,0);
               end   = MIN (signal2->size - 1, a);
               for (nmk = a-begin, k = begin; nmk >= a-end, k<=end ; nmk--, k++)
	       	       sum += signal2->Y[k]*signal1->Y[nmk];

               signalOut->Y[n]=sum;
	     }
	   break;

       case  BorderCon:
	   //size_conv         = (x_end_conv - x_begin_conv)/step + 1;
           size_conv         = n2 - n1 + 1 ;
           SizeSignal(signalOut, size_conv, YSIG) ;
           signalOut->firstp = MAX(0,-n1+signal1->firstp);
           signalOut->lastp  = signalOut->size-1-MAX(0,n2+signal2->size-1-signal1->lastp);
           signalOut->x0     =  x_begin_conv;
           for (n=0; n < size_conv; n++)
	     {
	       sum=0;
               a = n + signal2->size - 1 + n1;
               for (nmk = a, k=0 ; nmk >= a - signal2->size + 1, k < signal2->size; nmk--, k++)
		   sum +=  (nmk<0) ? (signal2->Y[k] * signal1->Y[0]) : ((nmk>=signal1->size) ? (signal2->Y[k] * signal1->Y[signal1->size-1]) : (signal2->Y[k]*signal1->Y[nmk])); 
               signalOut->Y[n] = sum;
	     }
	   break;
   
        case  BorderPer:      
	   //size_conv  = (x_end_conv - x_begin_conv)/step + 1;
           size_conv         = n2 - n1 + 1 ;
           SizeSignal(signalOut, size_conv, YSIG);
           signalOut->firstp = MAX(0,-n1+signal1->firstp);
           signalOut->lastp  = signalOut->size-1-MAX(0,n2+signal2->size-1-signal1->lastp);
           signalOut->x0     = x_begin_conv;
           for (n=0; n < size_conv; n++)
	     {
	       sum=0;
               a = n + signal2->size - 1 + n1;
	       for (k=0; k < signal2->size; k++)
		 {
                 
                   nmk  = (a-k) < 0 ? ((-(a-k))/signal1->size+1)*signal1->size + a-k : a-k;
                   sum += signal2->Y[k] * signal1->Y[nmk%signal1->size]; 
		 }
               signalOut->Y[n] = sum;
	     }
 	   break;

        case  BorderMir:
	  // sans rajout de point
	   //size_conv         = (x_end_conv - x_begin_conv)/step + 1;
           size_conv         = n2 - n1 + 1 ;
           SizeSignal(signalOut, size_conv, YSIG);
           signalOut->x0     =  x_begin_conv;
           signalOut->firstp = MAX(0,-n1+signal1->firstp);
           signalOut->lastp  = signalOut->size-1-MAX(0,n2+signal2->size-1-signal1->lastp);
           for (n=0; n < size_conv; n++)
	     {
	       sum=0;
               a  =  n + signal2->size - 1 + n1;
               for (k=0 ; k< signal2->size; k++)
		 {
                   nmk = (a-k) < 0 ? 2*((-(a-k))/signal1->size+1)*(signal1->size) + (a-k): a-k;
                   div  = (signal1->size == 1) ? 0 : nmk%(2*signal1->size - 2);
		   sum += (0 <= div && div < signal1->size) ?  signal2->Y[k] * signal1->Y[div] : signal2->Y[k] * signal1->Y[2*signal1->size-2-div];   
		 }
             signalOut->Y[n] = sum;
	     }
	break;

         case  BorderMir1:
           // point en plus aux extremites
           //size_conv         = (x_end_conv - x_begin_conv)/step + 1;
           size_conv         = n2 - n1 + 1 ;
           SizeSignal(signalOut, size_conv, YSIG);
           signalOut->x0     = x_begin_conv;
           signalOut->firstp = MAX(0,-n1+signal1->firstp);
           signalOut->lastp  = signalOut->size-1-MAX(0,n2+signal2->size-1-signal1->lastp);
           for (n=0; n < size_conv; n++)
	     {
	       sum=0;
               a = n + signal2->size - 1 + n1;
               for (k=0 ;  k<signal2->size;  k++)
		 {
                   nmk  = (a-k) < 0 ? 2*((-(a-k))/signal1->size+1)*signal1->size + (a-k): a-k;
                   div  = nmk % (2*signal1->size - 1);
		   sum += (0 <= div && div < signal1->size) ? signal2->Y[k] * signal1->Y[div] : signal2->Y[k] * signal1->Y[2*signal1->size - 1 -div];   
		 }
             signalOut->Y[n] = sum;
	     }
	break;
	   }
 
/* end switch */
}

  
void C_Conv (char ** argv)
{
  int     size_conv;
  int     borderType, fastconvolution;
  float   begin_conv, end_conv, step;
  SIGNAL  signal, filter, out; 
  char    * borderName, opt;
  char **argv1;
  float t;
   
   
  argv = ParseArgv(argv, tSIGNALI, &signal, tSIGNALI, &filter, tSIGNAL_, NULL, &out, tSTR, &borderName,-1);
  
  if (!strcmp(borderName,"bconst")) borderType = BorderCon;
  else if(!strcmp(borderName,"b0")) borderType = Border0;
  else if(!strcmp(borderName,"bmirror")) borderType = BorderMir;
  else if(!strcmp(borderName,"bperiodic"))  borderType = BorderPer;
  else if(!strcmp(borderName,"bmirror1")) borderType = BorderMir1;
  else Errorf("Undefined border effect: %s", borderName);

  step  = signal->dx;
  if (step != filter->dx) Errorf("Both signal and filter must have the same dx");

  if (signal==out || filter==out) Errorf("Output signal must be different from input and filter");

  
  if (borderType==BorderCon || borderType==Border0) {  
    begin_conv = signal->x0 + filter->x0;
    size_conv  = signal->size + filter->size - 1;
    end_conv   = begin_conv + (size_conv - 1) * step;
  }
  else {
    begin_conv = signal->x0;
    if (borderType==BorderMir) size_conv  = 2*signal->size - 1;
    else if (borderType==BorderMir1) size_conv  = (signal->size == 1) ? 1 : 2*signal->size;
    else size_conv  = signal->size;
    end_conv   = begin_conv + (size_conv - 1) * step;
  }
 
  fastconvolution = 0;
  while (opt = ParseOption(&argv)) {        
    switch(opt) {
    
    case 'x':
	  argv1 = ParseArgv(argv, tFLOAT_, 0., &begin_conv, tFLOAT_, 0., &end_conv,-1);
      if (argv1 != argv+2) {
	    if (borderType==BorderCon || borderType==Border0 || borderType==BorderPer) {  
          begin_conv = signal->x0;
		  size_conv  = signal->size;
		  end_conv   = begin_conv + (size_conv - 1) * step;
	    }
        if (borderType==BorderMir || borderType==BorderMir1) {  
	      begin_conv = signal->x0;
		  size_conv  = signal->size;
		  end_conv   = begin_conv + (size_conv - 1) * step;
	    }      
	  }
	  else {
	    argv = argv1;
	    if (begin_conv>=end_conv) Errorf("xMin should be greater than xMax");
	  }
      break;

	case 'f':
	  fastconvolution = 1;
	  break;
  
    default:
      ErrorOption(opt);
    } 
  }
  NoMoreArgs(argv); 
  
  t = MyTime();
  if (fastconvolution == 1) FFTConvolution (signal, filter, out, borderType, begin_conv, end_conv);
  else DirectConvolution (signal, filter, out, borderType, begin_conv, end_conv);
  t = MyTime()-t;
  
  SetResultFloat(t); 
}

    













