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
/* move chainlis->previous to its right position */
/* and remove the chain with chain->first NULL */
  
static void W2_clear_chain_pic(CHAINLIS2 chainlis)
{
  CHAIN2 chain, finger;

  chain = chainlis->first;

  while(chain && chain->previous) {
    chain = chain->previous;
  }

  chainlis->first = chain;

  while(chain) {
    if(chain->first == NULL) {
      finger = chain->next;
      RemoveChain2FromChainLis2(chain, chainlis);
      chain = finger;
    }
    else chain = chain->next;
  }
}

  
/*****************************************************/
/* update the informaion of a chainlis              */
/*****************************************************/

void W2_update_chain_pic(CHAINLIS2 chainlis)
{
  CHAIN2 chain;

  W2_clear_chain_pic(chainlis);

  chainlis->size = 0;
  chain = chainlis->first;
  for(chain = chainlis->first; chain; chain = chain->next) {
    chainlis->size ++;
    W2_update_chain(chain, chainlis->size);
  }
}


/**************************************/
/* compute the information of a chain */
/**************************************/

void W2_update_chain(CHAIN2 chain,int chain_index)
{
  int size;
  float mag, var;
  EXT2 ext;

  size = 0;
  mag = 0.0;
  var = 0.0;
  ext = chain->first;
  while(ext) {
    mag += ext->mag;
    size ++;
    var += ext->mag * ext->mag;
    ext->chain = chain;
    ext->chain_index = chain_index;
    ext = ext->next;
  }

  chain->size = size;
  if (size > 0)   {
    chain->mag = mag/size;
    var = var/size;
    chain->var = (float)fabs((double)(var - chain->mag * chain->mag));
  }
  else {
    chain->mag = 0.0;
    chain->var = 0.0;
  }
}

/**************************************************/
/* assign chain_index idx to each points in chain */
/**************************************************/

static void W2_chain_index(CHAIN2 chain,int idx)
{
  EXT2 ext;

  ext = chain->first;
  while(ext) {
    ext->chain_index = idx;
    ext = ext->next;
  }
}


/************************************************************************/
/* The function counts the number of chains and points in the chainlis */
/************************************************************************/

static void W2_chain_pic_count(CHAINLIS2 chainlis,int *nbpoint)
{
  CHAIN2 chain;
  int nb_point;

  nb_point = 0;
  W2_update_chain_pic(chainlis);

  for (chain = chainlis->first; chain ;chain = chain->next)
    nb_point += (chain->size);

  *nbpoint = nb_point;
}

void C_ChainReprCount2(char ** argv)
{
  WTRANS2 wtrans;
  int l, nbchain, nbpoint;

  nbpoint = 0;
  nbchain = 0;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();

  for (l = 1; l <= wtrans->chainrep->noct; l++) {
    W2_chain_pic_count(wtrans->chainrep->array[l],&nbpoint);
    nbchain = wtrans->chainrep->array[l]->size;
    Printf("Num of chain in level: %3d is %5d.\n", l, nbchain);
    Printf("Num of ext in chainlis is %d\n",nbpoint);
  }
}


	 


