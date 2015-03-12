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
/***************************************************/
/* split chain into three parts: chain, ext, and */
/* newchain */
/* chain -> (chain) (ext) (newchain)             */
/***************************************************/

void W2_chain_split(CHAIN2 chain, EXT2 ext,CHAIN2 newchain)
{
  if (ext == chain->first) {
    newchain->first = ext->next;
    chain->first = NULL;

    if(ext->next) ext->next->previous = NULL;
    ext->next = NULL;
  }
  else {
    newchain->first = ext->next;
    if(ext->previous) ext->previous->next = NULL;
    ext->previous = NULL;
    if(ext->next)  ext->next->previous = NULL;
    ext->next = NULL;
  }
  ext->chain = NULL;
  ext->chain_index = -1;
}

/*****************************************************/    
/** chain-> chain newchain and ext in newchain     */
/*****************************************************/

void W2_chain_split_2(CHAIN2 chain, EXT2 ext,CHAIN2 newchain)
{
  W2_chain_split(chain, ext, newchain);
  if(ext) {
    ext->next = newchain->first;
    if(newchain->first) newchain->first->previous = ext;
    newchain->first = ext;
    ext->chain = newchain;
  }
}


  
    
