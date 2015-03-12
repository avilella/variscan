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
/*
          7  0  1

          6  *  2

          5  4  3
*/

typedef struct W2_neighbor {
  struct ext2 *neighbor[8];
} *W2_NEIGHBOR;


static W2_NEIGHBOR W2_new_point_neighbor(void)
{
  W2_NEIGHBOR neighbor;
  int i;
  if(!(neighbor = (W2_NEIGHBOR)(Malloc(sizeof(struct W2_neighbor)))))
    Errorf ("Mem. alloc for NEIGHBOR failed\n");
  for (i = 0; i < 8 ; i++)
    neighbor->neighbor[i] = NULL;
  return (neighbor);
}


static W2_delete_point_neighbor(W2_NEIGHBOR neighbor)
{
  int i;
  for (i = 0 ; i < 8; i++) 
    neighbor->neighbor[i] = NULL;

  Free(neighbor);
  neighbor = NULL;
}

/******** fill up the neighbors of points in a extlis ************/

static W2_compute_point_neighbor(EXTLIS2 extlis, W2_NEIGHBOR neighbor,int k)
{
  int ncol = extlis->ncol;
  EXT2 *values = extlis->first;
  neighbor->neighbor[0] = values[k - ncol];
  neighbor->neighbor[4] = values[k + ncol];
  neighbor->neighbor[1] = values[k - ncol + 1];
  neighbor->neighbor[5] = values[k + ncol - 1];
  neighbor->neighbor[2] = values[k + 1];
  neighbor->neighbor[6] = values[k - 1];
  neighbor->neighbor[3] = values[k + ncol + 1];
  neighbor->neighbor[7] = values[k - ncol - 1];

}

  
/* link the points in the same level through previous and next */

static void W2_point_pic_link(WTRANS2 wtrans,int level,int uniform)
{
  EXTLIS2 extlis = wtrans->extrep->array[level];
  int nrow = extlis->nrow,
      ncol = extlis->ncol;
  EXT2 *values = extlis->first;
  EXT2 ext,neighbor,coarse, adjacent, adjacent1, adjacent2, adjacent3;
  W2_NEIGHBOR pneighbor;
  int dir,dir1,dir2;
  int i,j,I;
  float measure, measure_tmp;

  /* obtain from coarser */
  for(i = 1,I = ncol; i < nrow - 1; i++,I += ncol)
    for(j = 1; j < ncol - 1; j++)
      if(ext = values[I+j]) {
        if(ext->coarser && ext->coarser->finer && 
	   (ext->coarser->finer == ext)) {
	  coarse = ext->coarser;
          if(coarse->next) /* if coarser->next and its finer is close */
            if(coarse->next->finer) { 
	      neighbor = coarse->next->finer;
              if(abs(neighbor->x - ext->x) < 2 &&
                 abs(neighbor->y - ext->y) < 2)
                ext->next = neighbor;
	    }
          if(coarse->previous)  /* coarser->previous and its finer is close */
            if(coarse->previous->finer) {
	      neighbor = coarse->previous->finer;
              if(abs(neighbor->x - ext->x) < 2 &&
                 abs(neighbor->y - ext->y) < 2) 
                ext->previous = neighbor;
	    }
	}
      }

  /* by gradiant */
  for(i = 1,I = ncol; i < nrow-1; i++,I += ncol)
    for(j = 1; j < ncol - 1; j++)
      if(ext = values[I+j]) { 
	pneighbor = W2_new_point_neighbor();
	W2_compute_point_neighbor(extlis, pneighbor, I+j);

        W2_direction(ext->arg,&dir,&dir1,&dir2);

	/* among the three closed directions, find next */
	if (ext->next) {
	  adjacent = ext->next;
	  if (uniform)  measure = (float)
	    fabs((double)(ext->next->mag - ext->mag));
	  else measure = ext->next->mag;
	}
	else  adjacent = NULL;

	if (dir == dir1) {
	  dir1 = (dir +1)%8;
	  dir2 = (dir +7)%8;
	}

	adjacent1 = pneighbor->neighbor[dir];
	adjacent2 = pneighbor->neighbor[dir1];
	adjacent3 = pneighbor->neighbor[dir2];

	if (adjacent1) {
	  if (uniform) 
	    if (adjacent) {
	      measure_tmp = (float)
		fabs((double)(adjacent1->mag - ext->mag));
	      if (measure_tmp < measure) {
		adjacent = adjacent1;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent1;
	      measure = (float) fabs((double)(adjacent->mag - ext->mag));
	    }
	  else
	    if (adjacent) {
	      measure_tmp = adjacent1->mag;
	      if (measure_tmp > measure) {
		adjacent = adjacent1;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent1;
	      measure = adjacent->mag;
	    }
	}


	if (adjacent2) {
	  if (uniform) 
	    if (adjacent) {
	      measure_tmp = (float)
		fabs((double)(adjacent2->mag - ext->mag));
	      if (measure_tmp < measure) {
		adjacent = adjacent2;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent2;
	      measure = (float)fabs((double)(adjacent2->mag - ext->mag));
	    }
	  else
	    if (adjacent) {
	      measure_tmp = adjacent2->mag;
	      if (measure_tmp > measure) {
		adjacent = adjacent2;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent2;
	      measure = adjacent->mag;
	    }
	}


	if (adjacent3) {
	  if (uniform) 
	    if (adjacent) {
	      measure_tmp = (float)fabs((double)(adjacent3->mag - ext->mag));
	      if (measure_tmp < measure) {
		adjacent = adjacent3;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent3;
	      measure = (float)fabs((double)(adjacent3->mag - ext->mag));
	    }
	  else
	    if (adjacent) {
	      measure_tmp = adjacent3->mag;
	      if (measure_tmp > measure) {
		adjacent = adjacent3;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent3;
	      measure = adjacent3->mag;
	    }
	}
	if (adjacent) ext->next = adjacent;
	else ext->next = NULL;
	      

	/* among the three opposite directions, find previous */
	if (ext->previous) {
	  adjacent = ext->previous;
	  if (uniform)  measure = (float)
	    fabs((double)(ext->previous->mag - ext->mag));
	  else measure = ext->previous->mag;
	}
	else adjacent = NULL;

	if (dir == dir1) {
	  dir1 = (dir +1)%8;
	  dir2 = (dir +7)%8;
	}

	adjacent1 = pneighbor->neighbor[(dir+4)%8];
	adjacent2 = pneighbor->neighbor[(dir1+4)%8];
	adjacent3 = pneighbor->neighbor[(dir2+4)%8];

	if (adjacent1) {
	  if (uniform) 
	    if (adjacent) {
	      measure_tmp = (float)fabs((double)(adjacent1->mag - ext->mag));
	      if (measure_tmp < measure) {
		adjacent = adjacent1;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent1;
	      measure = (float)fabs((double)(adjacent1->mag - ext->mag));
	    }
	  else
	    if (adjacent) {
	      measure_tmp = adjacent1->mag;
	      if (measure_tmp > measure) {
		adjacent = adjacent1;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent1;
	      measure = adjacent1->mag;
	    }
	}


	if (adjacent2) {
	  if (uniform) 
	    if (adjacent) {
	      measure_tmp = (float)fabs((double)(adjacent2->mag - ext->mag));
	      if (measure_tmp < measure) {
		adjacent = adjacent2;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent2;
	      measure = (float)fabs((double)(adjacent2->mag - ext->mag));
	    }
	  else
	    if (adjacent) {
	      measure_tmp = adjacent2->mag;
	      if (measure_tmp > measure) {
		adjacent = adjacent2;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent2;
	      measure = adjacent2->mag;
	    }
	}


	if (adjacent3) {
	  if (uniform) 
	    if (adjacent) {
	      measure_tmp = (float)fabs((double)(adjacent3->mag - ext->mag));
	      if (measure_tmp < measure) {
		adjacent = adjacent3;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent3;
	      measure = (float)fabs((double)(adjacent3->mag - ext->mag));
	    }
	  else
	    if (adjacent) {
	      measure_tmp = adjacent3->mag;
	      if (measure_tmp > measure) {
		adjacent = adjacent3;
		measure = measure_tmp;
	      }
	    }
	    else {
	      adjacent = adjacent3;
	      measure = adjacent3->mag;
	    }
	}
	if (adjacent) ext->previous = adjacent;
	else ext->previous = NULL;

	if(pneighbor) {
	  W2_delete_point_neighbor(pneighbor);
	  pneighbor = NULL;
	}
	
      }

  /** ext->next->previous == ext **/
  /** ext->previous->next == ext **/

    for(i = 0,I = 0; i < nrow; i++,I += ncol)
      for(j = 0; j < ncol; j++)
	if(ext = values[I+j]) {
	  if(neighbor = ext->next) {
	    if(!neighbor->previous)
	      neighbor->previous = ext;
	    else if(neighbor->previous != ext)
	      ext->next = NULL;
	  }
	  if(neighbor = ext->previous) {
	    if(!neighbor->next)
	      neighbor->next = ext;
	    else if(neighbor->next != ext)
	      ext->previous = NULL;
	  }
	}

}

/** compute chainlis from extlis at level **/

static CHAINLIS2 W2_compute_chain(WTRANS2 wtrans, int level)
{
  EXTLIS2 extlis;
  EXT2 *values;
  EXT2 ext;
  CHAINLIS2 chainlis;
  int nrow, ncol;
  CHAIN2 chain;
  int i, j, I;

  extlis = wtrans->extrep->array[level];
  chainlis = wtrans->chainrep->array[level];
  values = (EXT2 *)extlis->first;
  nrow= chainlis->nrow = extlis->nrow;
  ncol= chainlis->ncol = extlis->ncol;

  chain = NewChain2();
  chainlis->first = chain;

  for(i = 1,I = ncol; i < nrow - 1; i++,I += ncol)
    for(j = 1; j < ncol - 1; j++) {
      if(ext = values[I+j])
        if(ext->chain == NULL) {
	  chain->next = NewChain2();
	  chain->next->previous = chain;
	  chain = chain->next;
          chain->first = ext;
          while(ext->previous && (ext->previous->next == ext)) { 
	    /* walk through previous to beginning */
            if(ext->previous == chain->first ||
               ext->previous->chain) {/*no previous is in a chain,no loop*/
	      /* if (ext->previous->chain) {
		chain->head = ext->previous;
	      } */
	      if (ext->previous == chain->first) ext->previous = NULL;
              break;
	    }
            else
              ext = ext->previous;
          }

	  /* if (!(chain->head)) chain->head = ext->previous; */
	  ext->previous = NULL;
	  ext->chain = chain; 

          ext->chain_index = chainlis->size; 
          chain->first = ext;
          chain->size = 1; 
          chain->mag = ext->mag;

          while(ext->next && (ext->next->previous == ext)) { 
	    /* walk through next to ending */
            if(ext->next == chain->first || /* no loop to the header */
               ext->next->chain) { /* no next in a chain */
	      if (ext->next->chain) chain->last = ext->next;
              ext->next = NULL;
              break;
            }else 
	    ext = ext->next;
            ext->chain = chain;
            ext->chain_index = chainlis->size;  
            chain->size++;
            chain->mag += ext->mag;
          }

	  if (!(chain->last)) chain->last = ext->next;
	  ext->next = NULL;

	  /* a chain has completed */
          chain->mag /= chain->size;  
          chainlis->size++;
	}
    }


  /* chainlis is established */
  chain->next = NULL;
  if (chainlis->first->next)  chainlis->first= chainlis->first->next;
                              
  else { chain->first= NULL;}

  W2_collect_point_in_point_pic(extlis, chainlis);

  return(chainlis);
}


/* compute the chainrep from wtrans from coarse to fine */
void ComputeChainrep2(WTRANS2 wtrans,int uniform)
{
  EXTREP2 extrep;
  CHAINREP2 chainrep;

  CHAINLIS2  chainpic;
  int l;

  extrep = wtrans->extrep;
  chainrep = wtrans->chainrep;
 
 
  for(l = extrep->noct; l >= 1; l--) {
    chainpic= wtrans->chainrep->array[l];
    W2_remove_point_pic_link(extrep->array[l]);
    W2_point_pic_link(wtrans,l, uniform);
    
    wtrans->chainrep->array[l] = W2_compute_chain(wtrans,l); 
   
  }
  chainrep->noct = wtrans->noct;
}


void C_ChainCompute2(char ** argv)
  
{
  WTRANS2 wtrans;
  int uniform = YES;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();

 
  W2_check_point_repr(wtrans->extrep);

  if(wtrans->chainrep->noct == 0) {

    wtrans->chainrep->noct = wtrans->extrep->noct;
  }
  else ClearChainrep2(wtrans);
  
  ComputeChainrep2(wtrans,uniform);
 
}










