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

#define CHAIN_LENGTH 1024

/***********************/
/* interpolation chain */
/***********************/

static void W2_chain_interpolation(CHAIN2 chain, EXTLIS2 extlis)
{
  EXT2 ext, npoint;

  if (chain->first) {
    ext = chain->first;
    if (ext->next) npoint = ext->next;
    else npoint = NULL;
  }
  else npoint = NULL;

  while (npoint) {
    W2_insert_point_between(chain,ext,npoint,extlis,YES);
    if (npoint->next) {
      ext = npoint;
      npoint = npoint->next;
    }
    else npoint = NULL;
  }
}



void W2_chain_pic_interpolation(EXTLIS2 extlis, CHAINLIS2 chainlis)
{
  CHAIN2 chain;

 for(chain = chainlis->first; chain; chain = chain->next)
    W2_chain_interpolation(chain, extlis);

  W2_update_chain_pic(chainlis);
  W2_collect_point_in_point_pic(extlis, chainlis); 
  W2_remove_point_in_chain_pic(extlis, chainlis);  
}




/* insert iext after previousext */

void W2_insertafter(EXT2 previousext,EXT2 iext)
{
  EXT2 last;
  last = iext;

  if(iext == NULL) return;

  while(last && (last->next)) {
    last = last->next;
  }
    
  iext->previous = previousext;

/*   iext->next = previousext->next; */
  last->next = previousext->next;

  if (previousext->next) {
/*    previousext->next->previous = iext; */
    previousext->next->previous = last;
  }
  previousext->next = iext;
}


/* iext inserts before previousext and become the chain->first */

void W2_insert_chain_first(EXT2 previousext,EXT2  iext,CHAIN2 chain)
{
  iext->next = previousext;
  iext->previous = NULL;
  previousext->previous = iext;
  if (chain->first) chain->first = iext; 
}

/*****************************************************************/
/* insert points between ext and nextp and form a longer chain */
/*****************************************************************/

void W2_insert_point_between(CHAIN2 chain,EXT2 ext,EXT2 nextp,EXTLIS2  extlis,int insert_after)
{

  EXT2 iext, previousext;
  EXT2 *values;
  float darg, dmag;
  int nb, dx, dy, i, k, ncol;
  int rotateflag;
  float rotate_theta;
  float tmparg1, tmparg2;
  

  values = extlis->first;
  ncol = extlis->ncol;


  dx = (int)fabs((double)(nextp->x - ext->x));
  dy = (int)fabs((double)(nextp->y - ext->y));

  nb = MAX(dx, dy);
  nb = MAX(0,nb - 1);

  rotateflag = NO;

  if(ext->arg * nextp->arg < 0.0) {
    tmparg1 = ext->arg;
    tmparg2 = nextp->arg;
    W2_rotate_to_same_sign(ext, nextp,&rotateflag,&rotate_theta);
  }


  darg = nextp->arg - ext->arg;
  dmag = nextp->mag - ext->mag;

  previousext = ext;

  for(i = 1; i <= nb; i++) {
    iext = NewExt2();
    W2_copy_point(ext,iext);
    if(dy > dx) {
      if(dx == 0)  
	iext->x = ext->x;
      else 
	iext->x = ext->x + 
	  (int)(i/((float)(nb+1))) * (nextp->x - ext->x);

      iext->y = ext->y + SIGN(nextp->y-ext->y);
    }
    else {
      if(dy == 0) 
	iext->y = ext->y;
      else 
	iext->y = ext->y + 
	  (int)(i/((float)(nb+1))) * (nextp->y - ext->y);

      iext->x = ext->x + SIGN(nextp->x-ext->x);
    }
    iext->scale = ext->scale;

    iext->arg = ext->arg + darg * i/(nb + 1.0);
    if(rotateflag) {
      iext->arg = iext->arg + rotate_theta;
      if(iext->arg > M_PI) iext->arg = iext->arg - 2.0 * M_PI;
    }

    iext->mag = ext->mag + dmag * i/(nb + 1.0);
    k = iext->y * ncol + iext->x;

    if (values[k] == NULL) {
      values[k] = iext;
      extlis->size++;
    }

    if(insert_after) W2_insertafter(previousext, iext);
    else W2_insert_chain_first(ext, iext, chain);

    chain->size++;
    previousext = iext; /* the new previous ext */
  }
  if(rotateflag) {
    ext->arg = tmparg1;
    nextp->arg = tmparg2;
  }
}



