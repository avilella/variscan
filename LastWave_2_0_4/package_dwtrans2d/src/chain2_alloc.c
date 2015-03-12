/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'dwtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  E.Bacry, J.Fraleu, J.Kalifa, E. Le Pennec, */
/*                         W.L. Hwang , S.Mallat, S.Zhong                   */
/*      emails : lastwave@cmap.polytechnique.fr                             */
/*               fraleu@cmap.polytechnique.fr                               */
/*               kalifa@cmap.polytechnique.fr                               */
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
  
/**************  ALLOC OF CHAIN  ***************/

/* alloc a chain */
CHAIN2 NewChain2(void)
{
   CHAIN2 chain ; 

  chain  = (CHAIN2) (Malloc(sizeof(struct chain2)));

  chain->size = 0;
  chain->mag = 0;
  chain->var = 0;
  chain->first = NULL;
  chain->last = NULL;
  chain->previous = NULL;
  chain->next = NULL;

  return(chain);
}

/* alloc a new chain picture */
CHAINLIS2 NewChainLis2(void)
{
   CHAINLIS2 chainlis;

   chainlis = (CHAINLIS2) (Malloc(sizeof(struct ChainLis2)));

  chainlis->size = 0;
  chainlis->nrow = 0;
  chainlis->ncol = 0;
  chainlis->first =NULL;
  return (chainlis);
}


void DeleteChainLis2(CHAINLIS2 chainlis)
{  
  CHAIN2 chain;
   if (chainlis == NULL) { 
     Warningf("DeleteChainLis2() : Free a NULL pointer!");}
   else { chain =chainlis->first;    
    while (chain)
        { RemoveChain2FromChainLis2(chain, chainlis);
          if (chain)
           { Free(chain);
             chain=NULL;
           }   
           chain = chainlis->first;  
        }
  
   Free(chainlis);
   chainlis=NULL;
   
   }
}
/* allocate a chain representatin */
CHAINREP2 NewChainrep2(void)
{
  CHAINREP2 chainrep;
  int i;
  
  if (!(chainrep = (CHAINREP2) (Malloc(sizeof(struct chainrep2)))))
    Errorf ("Mem. alloc for CHAIN_REPR failed\n");

  for (i = 0; i < W2_MAX_LEVEL; i++)
    chainrep->array[i] = NewChainLis2();
  chainrep->noct = 0;

  return (chainrep);
}

void DeleteChainrep2(CHAINREP2 chainrep)
{ int i;
 if (chainrep )  { 
      for (i = 0; i < W2_MAX_LEVEL; i++)
       if (chainrep->array!=NULL)
        DeleteChainLis2(chainrep->array[i]);
           
 Free(chainrep);
 chainrep=NULL;
 }
}

/* clean the chainrep */

void ClearChainrep2(WTRANS2 wtrans)  
{
  int i;
  CHAINREP2 chainrep = wtrans->chainrep;
  EXTREP2 extrep = wtrans->extrep;
  CHAIN2 chain;

  /* W2_check_chain_repr(chainrep);*/
  if (chainrep != NULL)
  {
   for(i = 1; i <= chainrep->noct; i++) {
    if (extrep!=NULL) W2_remove_point_pic_link(extrep->array[i]);
    chain = chainrep->array[i]->first;
    while( chain != NULL) {
      RemoveChain2FromChainLis2(chain, 
                                  chainrep->array[i]);
      chain = chainrep->array[i]->first;
    }
  }
}

}

