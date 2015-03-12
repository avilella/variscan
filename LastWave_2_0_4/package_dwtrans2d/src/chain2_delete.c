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
/* remove chain from a chainlis2, after the removal, chain still exists */
/************************************************************************/

void RemoveChain2FromChainLis2(CHAIN2 chain,CHAINLIS2 chainlis)
{
  if(chain == chainlis->first) {
    if(chain->next) {
      chainlis->first = chain->next;
      chainlis->first->previous = NULL;
    }
    else chainlis->first = NULL;
  } 
  else {
    chain->previous->next = chain->next;
    if (chain->next) chain->next->previous = chain->previous;
  }
  chainlis->size --;

}

/* Remove all points in chain */

static void RemoveAllExtInChain2(CHAIN2 chain, EXTLIS2 extlis)
{
  EXT2 ext, finger;

  ext = chain->first;
  finger = NULL;
  while(ext && ext->next) { 
    finger = ext;
    ext = ext->next;
  }

  while(ext) {
    W2_remove_point_from_point_pic(ext, extlis);
    if(finger)  {
      ext = finger;
      finger = finger->previous;
    }
    else ext = NULL;
  }
}


/* delete points in the propagation of chain */
void DeleteChain2Prop(WTRANS2 wtrans, CHAIN2 chain, int level)
{
  EXTREP2 extrep;
  CHAINREP2 chainrep;
  EXT2 ext, point1, finger, finger1;
  int l;

  extrep = wtrans->extrep;
  chainrep = wtrans->chainrep;

  RemoveChain2FromChainLis2(chain, chainrep->array[level]);

  /* update the ext */
  ext = chain->first;
  while(ext) {
    point1 = ext->next;
    finger = ext;
    l = level;
    while (finger) {
      if(finger->finer && (finger->finer->coarser == finger)) {
	l--;
	finger = finger->finer;
      }
      else break;
    }
    while(finger) {
      if(finger->coarser && (finger->coarser->finer == finger)) 
	finger1 = finger->coarser;
      else finger1 = NULL;

      W2_delete_a_point(finger, extrep->array[l]);
      finger = NULL;

      l++;
      finger = finger1;
    }
    ext = point1;
  }
}  

/** delete a chain in chainlis, and its points in extlis */

void DeleteChain2(EXTLIS2 extlis, CHAINLIS2 chainlis, CHAIN2 chain)
{
  RemoveChain2FromChainLis2(chain, chainlis);

  RemoveAllExtInChain2(chain, extlis);
}



