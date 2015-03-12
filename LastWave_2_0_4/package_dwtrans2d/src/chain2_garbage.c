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
/*********************************************************/
/* remove the points in chainlis while not in extlis */
/*********************************************************/

static void W2_remove_point_in_chain(CHAIN2 chain, EXTLIS2 extlis,CHAINLIS2 chainlis)
{
  EXT2 ext, finger;
  extern int W2_in_point_pic(EXT2 ext, EXTLIS2 extlis);

  
  ext = chain->first;
  while(ext) {
    finger = ext->next;
    if (!(W2_in_point_pic(ext,extlis))) {
      ext->chain = chain;
      W2_remove_point_from_chain(ext, chain);
      if(chain->first == NULL) 
        RemoveChain2FromChainLis2(chain, chainlis);
      W2_free_point(ext);
    }
    ext = finger;
  }
}
/*******************************/
/* is the ext in extlis ? */
/*******************************/

int W2_in_point_pic(EXT2 ext, EXTLIS2 extlis)
{
  EXT2 *values;
  int ncol;

  ncol = extlis->ncol;
  values = extlis->first;

 if ((values[ext->y * ncol + ext->x] != NULL) && 
      (values[ext->y * ncol + ext->x] == ext))
   return(YES); 

  else return(NO);
}

/***********************************************************/
/* remove all the points in chainlis but not in extlis */
/***********************************************************/

void W2_remove_point_in_chain_pic(EXTLIS2 extlis,CHAINLIS2 chainlis)
{
  CHAIN2 chain;

  for(chain = chainlis->first; chain; chain = chain->next)
    W2_remove_point_in_chain(chain, extlis, chainlis);
}


/*********************************************************/
/* collect the points in extlis but not in chainlis  */
/* by creating new chains                                */
/*********************************************************/

void W2_collect_point_in_point_pic(EXTLIS2 extlis,CHAINLIS2  chainlis)
{
  int i, j, nrow, ncol;
  EXT2 ext, *values;
  CHAIN2 finger, newchain;
  
  if(chainlis->first == NULL) Errorf("Chain representation has not computed yet \n");
  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;

  for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++) 
      if (ext = values[i * ncol + j]) {
	ext->chain = NULL;
	ext->chain_index = -1;
    }

  W2_update_chain_pic(chainlis);


  finger = chainlis->first;
  while(finger && finger->next) finger = finger->next;

  for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++) {
      if (ext = values[i * ncol + j]) 
	if(ext->chain == NULL) {
	  /* collecting points in extlis but not in chainlis */
	  while(ext->previous && (ext->previous->chain == NULL))
	    ext = ext->previous;

	  newchain = NewChain2();
	  newchain->first = ext;
	  ext->chain = newchain;
	  while(ext->next && (ext->next->chain == NULL)) {
	    ext = ext->next;
	    ext->chain = newchain;
	  }

	  chainlis->size ++;
	  W2_update_chain(newchain, chainlis->size);
	  W2_chain_append(finger, newchain);

	  finger = newchain;
	}
    }
}




void W2_chain_pic_first_point_copy(CHAINLIS2 input,CHAINLIS2 output , EXTLIS2 poutput)
{
  CHAIN2 chain, nchain, lastchain;
  EXT2 *poutput_values;
  int k, ncol;

  output->size = input->size;
  output->nrow = input->nrow;
  output->ncol = input->ncol;
  poutput_values = poutput->first;
  ncol = poutput->ncol;

  for (chain = input->first; chain ; chain = chain->next) {
    nchain = NewChain2();

    if (chain->first) {
      k = chain->first->y * ncol+ chain->first->x;
      nchain->first = poutput_values[k];
    }

    nchain->size = chain->size; /* only copy size */
      /* mag, .... */
    nchain->next = NULL;

    if (chain == input->first) {
      nchain->previous = NULL;
      output->first = nchain;
    }
    else {
      nchain->previous = lastchain;
      lastchain->next = nchain;
    }
    lastchain = nchain; 
  }
}



