/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
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


/****************************************************************************/
/*                                                                          */
/*  misc.c    miscellaneous Functions                                       */
/*                                                                          */
/****************************************************************************/




#include "lastwave.h"
#include <time.h>




/*
 * Quick Sort 
 *
 *    Sorts the array of double 'x' which is of size 'n'
 *
 * WARNING : 'x' starts at index 1 (up to 'n' included)
 */

void QuickSort(double *x,int n)
{
  int l,j,ir,i;
  double xx;

  l = ( n>>1)+1;
  ir = n;
  for (;;) {
    if(l>1) xx = x[--l];
    else {
      xx = x[ir];
      x[ir] = x[1];
      if( --ir ==1) { x[1]=xx; return; }
    }
    i=l;
    j=l<<1;
    while( j<= ir) {
      if( j<ir && x[j]< x[j+1]) ++j;
      if(xx < x[j]) {
        x[i] = x[j]; j+=(i=j);
      }
      else j=ir+1;
    }
    x[i] = xx;
  }
}


/*
 * Command to call a unix command
 */
 
void C_SystemUnix(char **argv)
{
  char *cmd;

  cmd = List2Str(argv," ");
  TempStr(cmd);
  
  system("stty iexten"); 
  system("stty ixon"); 
  system("stty ixoff"); 
  system("stty echonl");
  system("stty -noflsh"); 
  system("stty echo");
  system("stty icanon");
  
  system(cmd);

  system("stty -icanon min 1");
  system("stty -echo");
  system("stty -iexten"); 
  system("stty -ixon"); 
  system("stty -ixoff");   
  system("stty -echonl");
  system("stty noflsh"); 
}



/* Random Uniform number */ 

# define MBIG 1000000000
# define MSEED 161803398
# define MZ 0
# define FAC (1.0/MBIG)

static int iff=0;
static long int idum = -1;

void RandInit(long int idum1)
{
  iff = 0;
  idum = idum1;
}

void C_RandInit(char **argv)
{
  int i;
  
  ParseArgv(argv,tINT_,-1,&i,0);
  RandInit(i);
}

float Urand(void)
{
  static int inext,inextp;
  static long ma[56];
  register long mj,mk;
  register i,ii,k;

  if ( idum <0 || iff == 0){
     iff = 1;
     if (idum < 0) idum = -time(NULL);
     mj = MSEED - ( idum < 0 ? -idum : idum); 
     mj %= MBIG;                         
     ma[55] = mj;
     mk = 1;
     for ( i=1; i<=54; i++){
       ii = ( 21*i) % 55;
       ma[ii] = mk;
       mk = mj - mk;
       if ( mk < MZ ) mk += MBIG;
       mj = ma[ii];
     }
     for ( k=1; k<=4; k++)
       for (i=1; i<=55; i++){
	 ma[i] -= ma[1 + (i+30) % 55];
	 if ( ma[i] < MZ ) ma[i] += MBIG;
       }
     inext = 0;
     inextp = 31;
     idum = 1;
   }
   if( ++inext == 56 )inext=1;
   if( ++inextp == 56 )inextp=1;
   mj = ma[inext] - ma[inextp];
   if( mj < MZ) mj += MBIG;
   ma[inext] = mj;
   return mj*FAC;
}


/* Random Gaussian number */ 

float Grand(float sigma)
{
   static int iset=0;
   static float gset;
   float fac,rsq,v1,v2;

   if (iset == 0) {
    do {
         v1 = 2.0*Urand()-1.0;
         v2 = 2.0*Urand()-1.0;
         rsq=v1*v1+v2*v2;
        }
    while (rsq >= 1.0 || rsq == 0.0);
  fac=sqrt(-2.0*log(rsq)/rsq);
  gset = v1*fac;
  iset=1;
  return(v2*fac*sigma);
 }
 else {
  iset = 0;
  return(sigma*gset);
 }
}
  
  

/* The GetTime function (in seconds) */

float MyTime(void) 
{
  return(((float) clock())/CLOCKS_PER_SEC);
}

/* And the corresponding command */

void C_Time(char **argv)
{
  NoMoreArgs(argv);
  
  SetResultFloat(MyTime());
}      


/* Test whether we are running on big endian or little endian and set the corresponding variable */
char cpuBinaryMode;
void  InitCPUBinaryMode(void) 
{ 
  short unsigned int i;
  char *c;
  
  i = 1;
  c = (char *) (&i);
  if (*c == 0) cpuBinaryMode = BinaryBigEndian;
  else cpuBinaryMode = BinaryLittleEndian;
} 
  

/* 
 * Function to convert a Big (resp. Little) Endian array of values to a Little (resp. Big) array of values 
 *
 *    array   : is the array of values
 *    n       : the number of values
 *    sizeval : the number of bytes of each value
 */
void BigLittleValues(void *array, int n, size_t sizeval)
{
  size_t i;
  int j;
  unsigned char c;
  unsigned char *pvar;
  
  for (j=0;j<n*sizeval;j+=sizeval) {
    
    pvar = ((unsigned char *) array) + j;
  
    for(i=0;i<sizeval/2;i++) {
      c = *(pvar+i);
      *(pvar+i) = *(pvar+sizeval-1-i);
      *(pvar+sizeval-1-i) = c;
    }
  }
}


