/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0                       */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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



/* Canto generation down to the smallest resolution */
void Cantor(int numPoint,int depth,int nbPoint,float proba,
            int r1,int r2,int r3,float p1,float p3,int sum,SIGNAL signal)
{
    int	i;

    if (depth!=0)
	{
	Cantor(numPoint,depth-1,nbPoint*r1,proba*p1,r1,r2,r3,p1,p3,sum,signal);
	Cantor(numPoint+(int) (nbPoint*(r1+r2)*pow((double) sum,depth-1.)),
	       depth-1,nbPoint*r3,proba*p3,
	       r1,r2,r3,p1,p3,sum,signal);
	}
    else 
      for(i=0;i<nbPoint;i++) signal->Y[numPoint+i]=proba/nbPoint;
}

/* Corresponding command */
void C_Cantor(char **argv)
{
  SIGNAL signal;
  int depth,r1,r2,r3;
  float p1,p3;
  int sum;

  argv = ParseArgv(argv,tSIGNAL,&signal,tINT,&depth,tINT,&r1,tINT,&r2,tINT,&r3,tFLOAT,&p1,tFLOAT,&p3,0);

  sum = r1+r2+r3;
  SizeSignal(signal,(int) pow((double) sum,(double) depth),YSIG);

  ZeroSig(signal);

  signal->dx = pow((double) sum,(double) -depth);
  
  Cantor(0,depth,1,1.,r1,r2,r3,p1,p3,sum,signal);
}


/* Cantor which is constructed down to the smallest resolution */
void UCantor(SIGNAL signal,float left,float right,float proba,float *r,float *p,int nFlip)
{
  int i;
  float sum,l;
  
  if (fabs(right-left) <= signal->dx) {
    signal->Y[ISig(signal,left)] += proba;
    return;
  }
  
  for (sum = 0,l = left,i=0;r[i]!=-1;i++) {
    if (i == nFlip) UCantor(signal,l+(right-left)*r[i],l,proba*p[i],r,p,nFlip);
    else UCantor(signal,l,l+(right-left)*r[i],proba*p[i],r,p,nFlip);
    l += (right-left)*r[i];
  }
}

/* Corresponding command */  
void C_UCantor(char **argv)
{
  SIGNAL signal;
  int size,i;
  float r[10],p[10],sum;
  int nFlip;
  char opt;

  argv = ParseArgv(argv,tSIGNAL,&signal,tINT,&size,tFLOAT,&r[0],tFLOAT,&p[0],tFLOAT,&r[1],tFLOAT,&p[1],-1);
  
  for (i=2;i<9;i++) {
    argv = ParseArgv(argv,tFLOAT_,-1.0,&r[i],tFLOAT_,0.0,&p[i],-1);
    if (r[i] == -1 || p[i] == 0) break;
  }  
  r[9] = -1;
  
  for (sum = 0,i=0;i<10;i++) {
    if (r[i] == -1) break;
    if (r[i] <= 0) Errorf("The 'rN' ratios should be strictly positive");
    sum += r[i];
  }
  if (i == 10 || sum != 1.) Errorf("The sum of the ratios should be one");

  nFlip = 0;
  while(opt = ParseOption(&argv)) { 
    switch(opt) {
       case 'f': 
	      argv = ParseArgv(argv,tINT,&nFlip,-1);
	      break;
    default: ErrorOption(opt);
    }
   }    
   NoMoreArgs(argv);
  
  nFlip--;
     
  SizeSignal(signal,size,YSIG);

  ZeroSig(signal);

  signal->dx = 1.0/size;
  signal->x0 = 0;
  
  UCantor(signal,0.,1.,1.,r,p,nFlip);  
}









