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


/*****************************************************************/
/* partition the chains such that all the points in a chain will */
/* have the same corresponding chain in coarser scale            */
/*****************************************************************/

void W2_partition_cross_scale_chain(EXTLIS2 extlis,CHAINLIS2 chainlis)
{
  EXT2 ext, *values;
  CHAIN2 chain, newchain;
  int i, j, nrow, ncol;
  int idx;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;

  for(i = 0; i < nrow; i++)
    for( j = 0; j < ncol; j++)
      if(ext = values[i * ncol + j]) {
	if (ext->coarser) { 
	  ext->chain_index = ext->coarser->chain_index;
	  if(ext->coarser->finer != ext) Errorf("HaHa");
	}
      }

  for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++)
      if(ext = values[i * ncol +j])  {
	if(ext->coarser == NULL) {
	  ext->chain_index = -1; 
	}
      }


  for(chain = chainlis->first; chain; chain = chain->next) {
    idx = -1;
    for(ext = chain->first; ext; ext = ext->next) {
      if(ext->coarser) {
	idx = ext->chain_index;
	if(ext->next) {
	  if ((ext->next->chain_index != idx) && 
	      (ext->next->chain_index != -1)) {
	    newchain = NewChain2();
	    W2_chain_split_2(chain, ext->next, newchain);
	    W2_chain_append(chain, newchain);
	    break;
	  }
	}
      }
    }
  }
  W2_update_chain_pic(chainlis);
  W2_chainning_direction(chainlis);
  W2_collect_point_in_point_pic(extlis, chainlis);
  W2_remove_point_in_chain_pic(extlis, chainlis);  

}

/****************************************************************/      
/* return a chain haveing the same coarser chain as the chain1  */
/****************************************************************/

CHAIN2 W2_cross_scale_chain(CHAIN2 chain1,CHAIN2 chain2)
{
  int found = NO;
  EXT2 point1, point2;
 

  point1 = W2_first_coarser(chain1->first);
  if(point1) {
    for(chain2; chain2; chain2 = chain2->next) {
      point2 = W2_first_coarser(chain2->first);
      if(point2) {
        if(point1->coarser->chain ==  point2->coarser->chain) {
          found = YES;
          break;
        }
      }
    }
  }
    
  if(found) return(chain2); 
  else return(NULL);
}


/********************************************************************/
/* merge chains having the same coarser chain into a chain         */
/********************************************************************/
 
void W2_merge_cross_scale_chain(CHAINLIS2 chainlis)
{
  CHAIN2 chain, finger, mark,  coarse_chain, merge_chain;
  EXT2 lastpoint, mark2, ext;
  int i, NUM = 1024, first;
  CHAIN2 *score;

  score = (CHAIN2 *)Malloc(sizeof(CHAIN2) * NUM);

  for(i = 0; i < NUM; i++)
    score[i] = NULL;

  chain = chainlis->first;
  while(chain) {
    if(chain->first) {
      finger = W2_cross_scale_chain(chain, chain->next);
      while(finger) {
	mark = finger->next;
	mark2 = W2_first_coarser(finger->first);
	coarse_chain = mark2->coarser->chain;
	ext = coarse_chain->first;
	i = 0;
	while(ext != mark2->coarser)  {
	  i = i + 1;
	  if(i >= NUM) Errorf("MERGE_CROSS_SCALE_CHAIN: i > NUM");
	  ext = ext->next;
	}
	score[i] = finger;
	RemoveChain2FromChainLis2(finger, chainlis);
	finger = mark;
	finger = W2_cross_scale_chain(chain, finger);
      }
      
      mark2 = W2_first_coarser(chain->first);
      if(mark2) {
	coarse_chain = mark2->coarser->chain;
	ext = coarse_chain->first;
	i = 0;
	while(ext != mark2->coarser) {
	  i = i + 1;
	  if(i >= NUM) Errorf("MERGE_CROSS_SCALE_CHAIN: i > NUM");
	  ext = ext->next;
	}
	score[i] = chain;

	i = 0;
	first = YES;
	while(i < NUM) {
	  if(score[i] != NULL) {
	    if(first) {
	      merge_chain = score[i];
	      first= NO;
	    }
	    else {
	      lastpoint = W2_last_point(merge_chain);
	      W2_insertafter(lastpoint, score[i]->first);
	    }
	  }
	  i = i + 1;
	}
	chain->first = merge_chain->first;
      }
    }
    chain = chain->next;
    for(i = 0; i < NUM; i++)
      score[i] = NULL;
  }
  W2_update_chain_pic(chainlis);

  Free((char *)score);
}

/************************************************************/
/* to remove the zigzag in the fine scale chains            */
/************************************************************/

void W2_zigzag_off(EXTLIS2 finer_point_pic,CHAINLIS2 finer_chain_pic,EXTLIS2 extlis,CHAINLIS2 chainlis)
{
  EXT2 point1, point2, mark1, mark2;
  EXT2 finger1, finger2, lastpoint,tmp;
  EXT2 ext, finger;
  CHAIN2 chain;
  int nextflag, nb;
   
  EXT2 finer_first, finer_last;

  for(chain = chainlis->first; chain; chain = chain->next) {
    finer_first = NULL;
    finer_last = NULL;
    if(chain->first) {
      point2 = W2_first_finer_coarser_is_point(chain->first); /* 2 1 */
      if(point2) {
	finer_first = point2->finer;
	tmp = W2_last_finer_coarser_is_point(chain->first);
	finer_last = tmp->finer;

	nb = 0;
	finger = finer_first->chain->first;
	while(finger) {
	  if (finger->coarser && (finger->coarser->finer == finger))
	    nb = nb + 1;
	  finger = finger->next;
	}

	if(finer_first && finer_last && (nb > 2)) {

	  /* circle */
	  tmp = W2_last_point(finer_first->chain);
	  tmp->next = finer_first->chain->first;
	  finer_first->chain->first->previous = tmp;
	  finer_first->chain->first = finer_first;

	  /* hole */
	  if(finer_first->previous) {
	    finer_first->previous->next = NULL;
	    finer_first->previous = NULL;
	  }
	  W2_update_chain(finer_first->chain, finer_first->chain_index); 
	}
      
	point1 = W2_first_finer_coarser_is_point(point2->next);
      }
      else point1 = NULL;

      while(point2 && point1) {
	ext = point2;
	
	finger1 = point1->finer;
        finger2 = point2->finer;
	if(finger1 && finger2) {

	  W2_chain_order_reverse(finger2, finger1, &nextflag);

	  if((nextflag ==YES)&&(finger2->next != finger1)) {
	    mark2 = finger2->next;
	    mark1 = finger1->previous;
 	    lastpoint = W2_last_point(finger1->chain); 
	    if(mark2) mark2->previous = NULL;
	    if(mark1) mark1->next = NULL;
	    finger2->next = finger1;
	    finger1->previous = finger2;
	    W2_insertafter(lastpoint, mark2); 
	  }
	  if(nextflag == NO) { 
	    point2->finer->coarser = point1;
	    point1->finer->coarser = point2;
	    tmp = point2->finer;
	    point2->finer = point1->finer;
	    point1->finer = tmp;
	    point1 = point2; 
	  }
	  if(nextflag == -1) {
	  } 
	}
	point2 = point1; /* ok */
	point1 = W2_first_finer_coarser_is_point(point2->next);
      }

      if(point2 && finer_last && (nb > 2) ){
	finger = finer_last->next; 
	while(finger) {
	  mark1 = finger->next;
	  W2_delete_a_point(finger, finer_point_pic);
	  finger = mark1;
	}
      }
    }
  }
  
  W2_collect_point_in_point_pic(finer_point_pic, finer_chain_pic);
  W2_remove_point_in_chain_pic(finer_point_pic, finer_chain_pic);  
}
  
    



