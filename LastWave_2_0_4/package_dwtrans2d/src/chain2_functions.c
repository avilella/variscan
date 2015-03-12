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


EXT2 W2_last_finer_coarser_is_point(EXT2 ext)
{
  
  EXT2 tmp;

  if(ext == NULL) return(NULL);

  while(ext && ext->next) {
    ext = ext->next;
  }
  tmp = ext;  /* last ext */
  while(tmp) {
    if(!(tmp->finer && (tmp->finer->coarser == tmp)))
      tmp = tmp->previous;
    else  break;
  }
}

/* return the first ext s.t. no coarser or coarse->finer != ext */

EXT2 W2_first_coarser(EXT2 ext)
{
  int found = NO;
 for(ext; ext; ext = ext->next) 
    if(ext->finer) {
      found = YES;
      break;
    }

  if(found) return(ext);
  else return(NULL);
}


/* return the first ext s.t. ext->finer->coarser == ext */
/* with thresh length given */

EXT2 W2_first_finer_coarser_is_point(EXT2 ext)
{
  while(ext) {
    if(!(ext->finer && (ext->finer->coarser == ext)))
      ext = ext->next;
    else  break;
  }
  return(ext);
}

/* return the first ext with */
/* ext->finer->coarser is ext */

EXT2 W2_first_coarser_finer_is_point(EXT2 ext)
{
  while(ext) {
    if(!(ext->coarser && (ext->coarser->finer == ext)))
      ext = ext->next;
    else  break;
  }
  return(ext);
}


/******************************/
/* append chain2 after chain1 */
/******************************/

void W2_chain_append(CHAIN2 chain1,CHAIN2 chain2) 
{
  if(chain2->first) {
    chain2->previous = chain1;
    chain2->next = chain1->next;
    if(chain1->next) chain1->next->previous = chain2;
    chain1->next = chain2;
  }
}

/* return the last ext of the chain */

EXT2 W2_last_point(CHAIN2 chain)
{
  EXT2 ext;

  ext = chain->first;

  while(ext && ext->next) {
    ext = ext->next;
  }

  return (ext);
}

/* next -> previous and previous->next, first is the same */
void W2_chain_reverse(CHAIN2 chain)
{
  EXT2 ext, npoint;

  /* all ext->next = NULL, and ext->previous = ext->next */

  ext = chain->first;
  while(ext) {
    npoint = ext->next;

    ext->previous = ext->next;
    ext->next = NULL;

    ext = npoint;
  }

  /* ext--p-->ext--p->-ext ... */

  for(ext = chain->first; ext; ext = ext->previous)
    if (ext->previous) ext->previous->next = ext;

  ext = chain->first;
  while(ext && ext->previous)
    ext = ext->previous;

  chain->first = ext;

  /* ext--p-->ext--p-->ext ... */
  /* ext<-n--_point<-n---ext ... */
}
     

