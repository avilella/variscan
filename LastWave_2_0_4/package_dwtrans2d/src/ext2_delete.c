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

/* delete a ext and all structure poinging to it */

void W2_delete_a_point(EXT2 ext, EXTLIS2 extlis)
{
  if(ext == NULL) return;
  W2_remove_point_from_point_pic(ext, extlis);
 
  W2_remove_point_from_chain(ext,ext->chain);
  ext->chain = NULL;
  W2_free_point(ext);
  ext = NULL;  
}  
  


/* free a ext sturcture -- in the future rename free ext  */

void W2_free_point(EXT2 ext)
{
  if(ext == NULL) return;

  Free(ext);
  ext=NULL;
}



/* remove a ext form extlis */

void W2_remove_point_from_point_pic(EXT2 ext, EXTLIS2 extlis)
{
  int k;

  if(ext == NULL) return;
  if (ext->coarser) {
    if (ext->coarser->finer == ext)
      ext->coarser->finer = NULL;
    ext->coarser = NULL;
  }
  if (ext->finer) {
    if (ext->finer->coarser == ext)
      ext->finer->coarser = NULL;
    ext->finer =  NULL;
  }

  k = ext->y * extlis->ncol + ext->x;
  if (extlis->first[k] == ext) {
    extlis->first[k] = NULL;
    extlis->size--;
  }
}


/*** remove a ext from the chain containning it */

void W2_remove_point_from_chain(EXT2 ext,CHAIN2 chain)
{
  if(ext == NULL) return;

  if(chain) {

    if(ext->previous) {
      ext->previous->next = ext->next;
      if(ext->next) ext->next->previous = ext->previous;
    }
    else {
      chain->first = ext->next;
      if(chain->first) chain->first->previous = NULL;
    }
    ext->chain->size --;
  }
  
  ext->previous = NULL;
  ext->next = NULL;
  ext->chain = NULL;
  ext->chain_index = 0;
}




/* delete a extlis  */
void W2_delete_point_pic(EXTLIS2 extlis)
{
  if (extlis)
      {  
          W2_clear_point_pic(extlis);
          if (extlis->first)
             Free (extlis->first);
          extlis->first = NULL;

  extlis->nrow = 0;
  extlis->ncol = 0;
  extlis->size = 0;
  }
  Free(extlis);
  extlis=NULL;


}

      
/* clear a extlis representation */

void W2_clear_point_pic(EXTLIS2 extlis)
{
  int i, j, nrow, ncol;
  EXT2 ext, *values;
  if (extlis!=NULL){ 
  nrow = extlis->nrow;
  ncol = extlis->ncol;

   values = extlis->first;
 
  
  for (i = 0; i < nrow; i++)
    for (j= 0; j < ncol; j++) {
      if (ext = values[i *ncol + j]) {
	W2_delete_a_point(ext, extlis);
       
      }
    }
  extlis->size = 0;
  }
}


/* Reinitialization  of an ext representation */
void W2_clear_point_repr (EXTREP2 extrep)
{
  int i;

  ClearImage(extrep->coarse);
  for (i = 1; i <= extrep->noct; i++)
    W2_clear_point_pic(extrep->array[i]);
}
