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

/************************************************************************/
/* The function removes texture(weak edges), i.e., keep edge points     */
/* that have a coarser at the next level                                */
/************************************************************************/

int W2_point_pic_remove_texture(EXTLIS2 extlis)
{
  int nrow = extlis->nrow;
  int ncol = extlis->ncol;
  EXT2 *values; 
  EXT2 ext;
  int i, j, k, I, nb;
  
  values  = extlis->first;
  nb = 0;
  for (i = 0, I = 0; i < nrow; i++, I += ncol)
    for (j = 0; j < ncol; j++) {
      k = I + j;
      if (ext = values[k]) {
          if (ext->coarser == NULL) {
            W2_delete_a_point(ext, extlis);
          
	  ext = NULL;
	  nb ++;
	}  
      }
      }
  return nb;

}


/***************************************************************/
/* This function removes from extlis all points whose       */
/* magnitudes are no greater than thresh                       */
/***************************************************************/
void W2_point_pic_thresh(EXTLIS2 extlis,float  thresh)
     
{
  EXT2 ext;
  EXT2 *values;
  int i, j, k, ncol, nrow;

  if (!extlis) {
    Printf("NULL extlis\n");
    return;
  }
  ncol = extlis->ncol;
  nrow = extlis->nrow;
  values = (EXT2 *)extlis->first;

  for (i = 0; i < nrow; i++)
    for (j = 0; j < ncol; j++) {
      k = i * ncol + j;
      if (ext = values[k])
        if (ext->mag <= thresh) {
          W2_delete_a_point(ext, extlis);
          ext = NULL;
        }
    }
}
    

/***************************************************************/
/* This function removes from point_expr all points whose      */
/* magnitudes are less than thresh                             */
/***************************************************************/
 


void C_Point2Thresh(char **argv)
{
  WTRANS2 wtrans;
  EXTREP2 extrep;
  float thresh_mag, new_thresh;
  int level, i;
  char car;

   argv =ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,tFLOAT,&thresh_mag,0);
   if (wtrans==NULL) wtrans= GetWtrans2Cur();
   
  
  CheckWtrans2(wtrans);
  extrep = wtrans->extrep;
  W2_check_point_repr(extrep);

  if(level == 0) 
      for (i = 1; i <= extrep->noct; i++) {
	  new_thresh = thresh_mag*pow(2.,(double)i/2.)/3.;
	  W2_point_pic_thresh(extrep->array[i],new_thresh);
      }
  if(level != 0) {
    if (INRANGE(1,level,extrep->noct) == NO)
      Errorf("C_Point2Thresh() : the level is out of bound of noct \n");
    else {
	new_thresh = thresh_mag*pow(2.,(double)level/2.)/3.;
	W2_point_pic_thresh(extrep->array[level],new_thresh);
    }	
  }	
}

/******************************************************************/
/* The function counts the number of points in the extrep     */
/******************************************************************/
static int W2_point_pic_count(int level,EXTLIS2 extlis)
{
  int nrow = extlis->nrow;
  int ncol = extlis->ncol;
  EXT2 *values = extlis->first;
  EXT2 ext;
  int i,j, I;
  int count = 0;
  for (i = 0, I = 0; i < nrow; i++, I += ncol)
    for (j = 0; j < ncol; j++)
      if (ext = values[I + j]) {
/*      if (ext->mag > 0.0000001) */
        count++;
      }
  Printf("number of points at level %2d are %d\n",level,count);
  return(count);
}

static void W2_point_repr_point_count(EXTREP2 extrep)
{
  int total = 0;
  int l;

  for (l = 1; l <= extrep->noct; l++)
    total += W2_point_pic_count( l, extrep->array[l]);
  Printf("total number of points are %d\n",total);
}
  
void C_Point2Count(char **argv)
   
{  
  WTRANS2 wtrans;
   argv =ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);
   if (wtrans==NULL) wtrans= GetWtrans2Cur();
   
  CheckWtrans2(wtrans);
  W2_check_point_repr(wtrans->extrep);

  W2_point_repr_point_count(wtrans->extrep);
}
