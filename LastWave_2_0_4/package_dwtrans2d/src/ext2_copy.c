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

/* copy a ext */
void W2_copy_point(EXT2 input,EXT2 output)
{
  output->scale = input->scale;
  output->x = input->x;
  output->y = input->y;
  output->mag = input->mag;
  output->arg = input->arg;
  output->hor = input->hor;
  output->ver = input->ver;
  output->chain_index = input->chain_index;
  output->chain = input->chain; 
}


static void W2_copy_point_pic_link(EXTLIS2 extlis, EXTLIS2 new_extlis)
{
  EXT2 *values, *out_values, newpoint, ext;
  int pk, nk, i, j, k;
  int nrow, ncol;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  
  values = extlis->first;
  out_values = new_extlis->first;

  for ( i = 0; i < nrow; i++)  /* copy link  */
    for (j = 0; j < ncol; j++) 
      if (ext = values[i * ncol + j]) {
	k = ext->y * ncol + ext->x;
	newpoint = out_values[k];
	if (ext->previous) {
	  pk = ext->previous->y * ncol + ext->previous->x;
	  newpoint->previous = out_values[pk];
	}
	if (ext->next) {
	  nk = ext->next->y * ncol + ext->next->x;
	  newpoint->next = out_values[nk];
	}
      }
}

/********************************/
/* remove the extlis linking */
/********************************/
  
void W2_remove_point_pic_link(EXTLIS2 extlis)
{
  int i, j, nrow, ncol;
  EXT2 ext;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++) 
      if((extlis->first[i * ncol + j]) != NULL) {
	ext = extlis->first[i * ncol + j];
	ext->next = NULL;
	ext->previous = NULL;
	ext->chain = NULL;
      }
}

void W2_point_pic_copy(EXTLIS2 input, EXTLIS2 output)
{
  int nrow, ncol;
  EXT2 *values, *out_values;
  EXT2 newpoint, ext;
  int i, j, k;


  nrow = input->nrow;
  ncol = input->ncol;
  values = input->first;

  W2_change_point_pic(output, nrow, ncol); 
  out_values = output->first;

  for ( i = 0; i < nrow; i++)  /* copy values no link */
    for (j = 0; j < ncol; j++)
      if (ext = values[i * ncol + j]) {
	k = ext->y * ncol + ext->x;
	newpoint = NewExt2();
	W2_copy_point(ext, newpoint);
	/* copy_coarser(ext, newpoint);
	copy_finer(ext, newpoint); */

	out_values[k] = newpoint;
      }

  W2_copy_point_pic_link(input, output);
  output->size = input->size;
}



/***************************************************************/
/* copy all the pointers and higher level data structure build */
/*   on points                                                 */
/***************************************************************/

static void W2_point_plane_copy(WTRANS2 wtrans, EXTLIS2 newextlis,CHAINLIS2 newchainlis, int from)
{
  EXTLIS2 extlis;
  CHAINLIS2 chainlis;
  CHAIN2 chain;

  extlis = wtrans->extrep->array[from];
  chainlis = wtrans->chainrep->array[from];

  for(chain = chainlis->first; chain; chain = chain->next) {
    /* chain->head = NULL; */
    chain->last = NULL;
  }

  W2_point_pic_copy(extlis, newextlis);

  W2_chain_pic_first_point_copy(chainlis,newchainlis,newextlis);

}
			       
  
/************************************************************/
/* copy extlis form one to the other with chain included */
/************************************************************/

void C_PointPicCopy(char **argv)
{
  EXTLIS2 newextlis;
  CHAINLIS2 newchainlis;
  int n;
  WTRANS2 w2from=NULL;
  WTRANS2 w2to;

  argv= ParseArgv(argv,tWTRANS2,&w2from,tWTRANS2,&w2to,0);
   
  DeleteExtrep2(w2to->extrep);
  DeleteChainrep2(w2to->chainrep);
  w2to->extrep = NewExtrep2();
  w2to->chainrep = NewChainrep2(); 
 
  for (n= 1;n<=w2from->extrep->noct;n++) {
      newextlis = NewExtLis2();
      newchainlis = NewChainLis2();
      W2_point_plane_copy(w2from, newextlis, newchainlis, n);
      w2to->chainrep->array[n] = newchainlis;
      w2to->extrep->array[n] = newextlis;
  }
   w2to->extrep->noct = w2from->extrep->noct;
   CopyImage(w2from->extrep->coarse,w2to->extrep->coarse);
   w2to->chainrep->noct = w2from->chainrep->noct;
   
  w2to->noct = w2to->extrep->noct;
  w2to->norient = 4;
}

/************************************************************/
/* clear  extlis form one  with chain included */
/************************************************************/

void C_P2Clear(char **argv)
{
  
  WTRANS2 wtrans2;

  argv= ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,0);
  if (wtrans2==NULL) wtrans2= GetWtrans2Cur();
 
  DeleteExtrep2(wtrans2->extrep);
  DeleteChainrep2(wtrans2->chainrep);
  wtrans2->extrep = NewExtrep2();
  wtrans2->chainrep = NewChainrep2(); 
}


