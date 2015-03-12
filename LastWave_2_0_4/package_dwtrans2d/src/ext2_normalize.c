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


void C_PointReprNormalize2(char **argv)
{ 
  WTRANS2 wtrans;
  int k;
  int periodic=YES;
 
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);

 if (wtrans ==NULL) wtrans= GetWtrans2Cur();

 NoMoreArgs(argv);


  if(wtrans->extrep->normalized) Errorf("The ext is already normalized \n");
  for (k = 1; k <= wtrans->extrep->noct; k++)
    W2_point_pic_normalize(wtrans, k);
  wtrans->extrep->normalized = YES;
}

void W2_point_pic_normalize(WTRANS2 wtrans,int k)
{
  int ncol, nrow, i, j;
  EXT2 *values, ext;
  EXTLIS2 extlis;
  double fact,inv;
  
  extlis = wtrans->extrep->array[k];

  ncol = extlis->ncol;
  nrow = extlis->nrow;
  values = extlis->first;
  for(i = 0; i < nrow; i++)
    for (j = 0; j < ncol; j++)
      if (ext = values[i * ncol + j]) 
	ext->mag = ext->mag * wtrans->filterg1->factors[k-1];

  /* 2nde normalisation en compensation de perdecomp */
  if(k==1)
    fact = pow(2.,1./2.) / 1.8 ;
  else
    fact = pow(2.,(double)(k)/ 2.); 
  inv=1./fact;
  if(wtrans->periodic)
    for(i = 0; i < nrow; i++)
      for (j = 0; j < ncol; j++)
	if (ext = values[i * ncol + j]) 
	  ext->mag = ext->mag * inv;
}

/* point_denormalize - divide the normalize constant for */
/* each ext */

void C_PointReprDenormalize2(char ** argv)
    
{
  WTRANS2 wtrans;
  int k;
  int periodic=YES;
 
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);

  NoMoreArgs(argv);

 if (wtrans ==NULL) wtrans= GetWtrans2Cur();

  if(wtrans->extrep->normalized== NO) Errorf("ext is not normalized \n");
  for(k = 1; k <= wtrans->extrep->noct; k++)
   { W2_point_pic_denormalize(wtrans,k);
   }
  wtrans->extrep->normalized = NO;

}
  
void W2_point_pic_denormalize(WTRANS2 wtrans,int k)
{
  int nrow, ncol, i, j;
  EXT2 *values, ext;
  EXTLIS2 extlis;
  double fact;
  
  extlis = wtrans->extrep->array[k];
  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;

  for(i = 0; i< nrow; i++)
    for(j = 0; j < ncol; j++)
      if (ext = values[i * ncol + j])
	  ext->mag = ext->mag /wtrans->filterg1->factors[k-1];

 /* 2nde denormalisation correspondant a perdecomp */
  if(k==1)
    fact = pow(2.,1./2.) / 1.8 ;
  else 
    fact =(double)(pow(2.,(double)k/ 2.));
  if(wtrans->periodic)
    for(i = 0; i < nrow; i++)
      for (j = 0; j < ncol; j++)
	if (ext = values[i * ncol + j]) 
	  ext->mag = ext->mag * fact;
}


    
    
    




