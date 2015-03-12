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

/******************************************************************/
/* after smooth a chain, the ext location in the chain will be  */
/* different with the ext location in extlis. The function   */
/* removes all the points in the extlis and assigned the ext */
/* from chainlis                                                 */
/******************************************************************/


void W2_point_pic_relocate(EXTLIS2 extlis,CHAINLIS2 chainlis)
{
  int nrow, ncol;
  EXT2 *values, ext;
  CHAIN2 chain, newchain;
  int i, j, k, size;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;
  size = 0;

  for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++) {
      values[i * ncol + j] = NULL;
    }
  
  for(chain = chainlis->first; chain; chain = chain->next) {
    ext = chain->first;
    while(ext) {
      if((INRANGE(0, ext->y, nrow-1)) &&
	  (INRANGE(0, ext->x, ncol-1)))
	 k = ext->y * ncol + ext->x;
	 else { 
	   Printf("abs_error \n");
	 }
      if(values[k] == NULL)  {
	values[k] = ext;
	if(ext->coarser && (ext->coarser->finer != ext))
	  Errorf("Ha ");
	size ++;
      }
      else {
	  newchain = NewChain2();
	  W2_chain_split(chain, ext, newchain);
	  W2_delete_a_point(ext, extlis);
	  W2_chain_append(chain, newchain);
	  break;
	}
      ext = ext->next;
    }
  }
   
  extlis->size = size;

  W2_update_chain_pic(chainlis);
}



