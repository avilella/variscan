/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'dwtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  E.Bacry, J.Fraleu, J.Kalifa, E. Le Pennec, */
/*                         W.L. Hwang , S.Mallat, S.Zhong                   */
/*      emails : lastwave@cmap.polytechnique.fr                             */
/*               fraleu@cmap.polytechnique.fr                               */
/*               kalifa@cmap.polytechnique.fr                               */
/*               lepennec@cmap.polytechnique.fr                             */
/*               mallat@cmap.polytechnique.fr                               */
/*               whwang@iis.sinica.edu.tw                                   */
/*               szhong@chelsea.princeton.edu                               */
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

#include "extrema2d.h"
  
/*********************************************************************/
/* To test whether angle information is robust or not, we remove the */
/* ARGUMENT in all the level and compute ther derivative of chain    */
/* and take its perpedicular direction as gradient and put it into   */
/* ARGUMENT in each corresponding level.  Since the chain is only    */
/* integer positios, we first smooth the chain, and take derivative  */
/* later.  The is the filter to smooth the chain.                    */
/* ----------------------------------------------------------------- */

#define W2_CHAIN_LENGTH 2048



static void W2_smooth(float * signal,int size)
{
  int l;
  float*f_array;

 

  if(size < 3)
    return;

  f_array =FloatAlloc(size+1);

  f_array[1] = 0.25*(signal[0] + 2.0*signal[1] + signal[2]);
  f_array[size-1] = 0.25*(signal[size-3] + 2.0*signal[size-2] +signal[size-1]);

  for(l = 2; l < size-2; l++)
      {
	  f_array[l] = ((signal[l-2] + 4.* signal[l-1] + 6.0*signal[l] + 4. * signal[l+1] + signal[l+2]) / 16.);
	   f_array[l] =(signal[l-2] + 4.* signal[l-1] + 6.0*signal[l] + 4. * signal[l+1] + signal[l+2]) / 16.; 
      }
  for(l = 1; l < size-1; l++)
   { 
   signal[l] = f_array[l]; 
  }
 Free(f_array);
}

static void W2_chain_arg(CHAIN2 chain, int endmode)
{
  EXT2 ext;
  int k,n;
  float * xxf,*yyf,*xxd,*yyd;
  

  if (! chain->first) return;
 
  for(ext = chain->first, k = 0;
      ext;
      ext = ext->next, k++) {}
 if (k >= W2_CHAIN_LENGTH) 
     Errorf("chain_arg:too many ext regarding the static memory allocation");

  xxf = FloatAlloc(k);
  yyf = FloatAlloc(k);
  xxd = FloatAlloc(k);
  yyd = FloatAlloc(k);

 for(ext = chain->first, k = 0;
      ext;
      ext = ext->next, k++) {

      xxf[k] = (float)ext->x;
      yyf[k] = (float)ext->y;
     
  }
  n = k;

  for(k = 1; k < n-1; k++) {
     if (isnan(xxf[k]) ) Errorf("xxf "); 
      if (isnan(yyf[k]) ) Errorf("yyf ");  }
  W2_smooth(xxf,n);
 for(k = 1; k < n-1; k++) {
     if (isnan((double)xxf[k]) ) Errorf("xxf 2 k=%d  avec n=%d et xx[k]=%f",k,n,xxf[k]); 
      if (isnan(yyf[k]) ) Errorf("yyf 2");  }
  W2_smooth(xxf,n);
 for(k = 1; k < n-1; k++) {
     if (isnan(xxf[k]) ) Errorf("xxf 3"); 
      if (isnan(yyf[k]) ) Errorf("yyf 3");  }
  W2_smooth(yyf,n);
 for(k = 1; k < n-1; k++) {
     if (isnan(xxf[k]) ) Errorf("xxf 4"); 
      if (isnan(yyf[k]) ) Errorf("yyf 4");  }
  W2_smooth(yyf,n);

  xxd[0] = xxf[1]-xxf[0];
  yyd[0] = yyf[1]-yyf[0];
  for(k = 1; k < n-1; k++) {
    
    xxd[k] = 0.5*(xxf[k+1]-xxf[k-1]);
    yyd[k] = 0.5*(yyf[k+1]-yyf[k-1]);
  }
  xxd[k] = xxf[k]-xxf[k-1];
  yyd[k] = yyf[k]-yyf[k-1];

  if(endmode) {
    ext = chain->first;
    ext->arg = W2_argument(-yyd[0], xxd[0]);
    ext = W2_last_point(chain);
    ext->arg = W2_argument(-yyd[n], xxd[n]);
  }
  else {
    for(ext = chain->first, k = 0;
	ext;
	ext = ext->next, k++) {
    
      ext->arg = W2_argument(-yyd[k], xxd[k]);
    }
  }
 Free(xxf);
 Free(yyf);
 Free(xxd);
 Free(yyd);

}


void W2_chain_pic_arg(WTRANS2 wtrans,int level, int endmode)
{
  CHAINLIS2 chainlis ;
  CHAIN2 chain;

  chainlis = wtrans->chainrep->array[level];
  if ((chainlis->size > 0) && (chainlis->first))
    for(chain = chainlis->first; chain ; chain = chain->next) 
      W2_chain_arg(chain, endmode);
}


/** Compute the angle from the tangent of the chain **/
void C_ChainPicPredictArg(char **argv)
{
  WTRANS2 wtrans2=NULL;
  int level, endmode = NO;
  char car;
 

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,tINT,&level,-1);
  
 if (wtrans2 ==NULL)  wtrans2 =  GetWtrans2Cur();

  while (car=ParseOption(&argv))
    switch (car) {
      case 'e':
	endmode = YES;
	break;
      default :
	  ErrorOption(car);
    }

  
  W2_chain_pic_arg(wtrans2, level, endmode);
}

/* arg smoothing by averaging on weighted consecutive points (smooth point2)*/

static void W2_arg_smooth_2_points(EXT2 point1,EXT2 point2)
{
  int rotateflag;
  float tmparg1,  rotate_theta;

  if(point1 && point2) {

    rotateflag = NO; /* because PI and -PI are the same .. */
    if(point1->arg * point2->arg < 0.0) {
      tmparg1 = point1->arg;

      W2_rotate_to_same_sign(point1, point2, &rotateflag, &rotate_theta);
    }

    point2->arg = (point2->arg + point1->arg)/2.0;

    if(rotateflag) {
      /* rotate_back */
      point1->arg = tmparg1;

      point2->arg = point2->arg + rotate_theta;
      if(point2->arg > M_PI) point2->arg = point2->arg - 2.0 * M_PI;
    }
  }
}

/* smooth the middle by averaging three points by weight 1/4, 1/2, 1/4 */

static void W2_arg_smooth_three_points(EXT2 point1, EXT2 point2, EXT2 point3)
{
  float temp1, temp2, temp3;

  if(point1 && point2 && point3) {
    /* printf("%f %f %f   newpoint2 ",point1->arg, point2->arg, point3->arg);*/

    temp2 = point2->arg; /* store in temp2 */
    W2_arg_smooth_2_points(point1, point2);
    temp1 = point2->arg; /* (a + b)/2 */
    point2->arg = temp2;
    W2_arg_smooth_2_points(point3, point2);/* (b + c)/2 */
    
    temp3 = point3->arg; 
    point3->arg = temp1; 
    W2_arg_smooth_2_points(point3, point2); 
    point3->arg = temp3;

    /* printf("%f \n", point2->arg); */
  }
}

/* smooth chain */

static void W2_chain_arg_smooth(CHAIN2 chain)
{
  EXT2 ext;

  for(ext = chain->first; ext ; ext = ext->next) {
    W2_arg_smooth_three_points(ext->previous, ext,
			    ext->next);
  }
}

static void W2_chain_pic_arg_smooth(CHAINLIS2 chainlis)
{
  CHAIN2 chain;

  for(chain = chainlis->first; chain; chain = chain->next)
    W2_chain_arg_smooth(chain);
}


void W2_chainpicsmootharg(WTRANS2 wtrans2,int level)
{
 if(INRANGE(1, level, wtrans2->chainrep->noct)) {
    W2_update_chain_pic(wtrans2->chainrep->array[level]);
    W2_chain_pic_arg_smooth(wtrans2->chainrep->array[level]);
  }
  else { Errorf("W2_chainpicsmootharg() : level does not right \n"); }

}

void C_ChainPicSmoothArg(char ** argv)
{
  int level;
  WTRANS2 wtrans2=NULL;

   argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,tINT,&level,0);
  
 if (wtrans2 ==NULL) 
     wtrans2 =  GetWtrans2Cur();
 W2_chainpicsmootharg(wtrans2,level);
}

