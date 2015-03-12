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

/**************************/
/* Print POINT_PIC fields */
/**************************/

/* 'pinfo' command */
void C_PInfo2(char ** argv)

{ 
  WTRANS2 wtrans;
  EXTREP2 extrep;
  EXTLIS2 extlis;
  int level;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,-1);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();

  extrep = wtrans->extrep;
  if (INRANGE(1,level,extrep->noct)) {
    extlis = extrep->array[level];
    Printf("\n");
    Printf(" nrow  :  %d  \n",extlis->nrow);
    Printf(" ncol  :  %d  \n",extlis->ncol);
    Printf(" size  :  %d  \n",extlis->size);
    Printf(" \n");
  }
 else
  Errorf("Invalid level \n");
}


 
/**************  ALLOC OF POINT  ***************/

/* alloc an edge ext */
EXT2 NewExt2(void)
{
  EXT2 ext ; 

  ext  = (EXT2) (Malloc(sizeof(struct ext2)));
  ext->scale = 0;
  ext->x = 0;
  ext->y = 0;
  ext->hor = 0.0;
  ext->ver = 0.0;
  ext->mag = 0.0;
  ext->arg = 0.0;
  ext->previous = ext->next = NULL;
  ext->coarser = ext->finer = NULL;
  ext->chain = NULL;
  ext->chain_index = 0;
  return(ext);
}

/* alloc a new ext picture */
EXTLIS2 NewExtLis2(void)
{
  EXTLIS2 extlis;

  extlis = (EXTLIS2) (Malloc(sizeof(struct extlis2)));
  extlis->size = 0;
  extlis->nrow = 0;
  extlis->ncol = 0;
  extlis->first = NULL;
  return (extlis);
}



/* allocate a ext representatin */
EXTREP2 NewExtrep2(void)
{
  EXTREP2 extrep;
  int i;

#ifdef DEBUGALLOC
DebugType = "Extrep2";
#endif
  
  extrep = (EXTREP2) (Malloc(sizeof(struct extrep2)));


  for (i = 0; i < W2_MAX_LEVEL; i ++)
    extrep->array[i] = NewExtLis2();

  extrep->noct = 0;
  extrep->coarse = NewImage();

  extrep->normalized = YES;
  extrep->lipflag = NO;

  return (extrep);
}

void DeleteExtrep2( EXTREP2 extrep)
{
  int i;
 if (extrep) { 
 for (i = 0; i < W2_MAX_LEVEL; i ++)
     if (extrep->array) {W2_delete_point_pic(extrep->array[i]);
     }
 DeleteImage(extrep->coarse);

#ifdef DEBUGALLOC
DebugType = "Extrep2";
#endif
 
  Free(extrep);
  extrep=NULL;
 }
}
 

void W2_point_pic_alloc(EXTLIS2 extlis,int nrow,int ncol)
{
  int i, j, I;
  EXT2 *values;
  if (extlis) {

  if (!(values = (EXT2 *)Malloc(nrow * ncol * sizeof(EXT2)))) 
    Errorf("not enough memory for POINT_PIC");
  extlis->first = values;
  extlis->nrow = nrow;
  extlis->ncol = ncol;
  extlis->size = 0;
   for (i = 0, I = 0; i < nrow ; i++, I += ncol)
    for (j = 0; j < ncol; j++)
      values[I + j] = NULL;
  }
}


void W2_change_point_pic(EXTLIS2 extlis,int nrow,int ncol)
{
  int s;

  if (extlis == NULL) Errorf("extlis is NULL \n");

  if (extlis->first) {
    s = extlis->nrow * extlis->ncol;
    if (nrow * ncol > s || nrow * ncol < s - (1 << 10)) {
      W2_delete_point_pic(extlis);
      W2_point_pic_alloc(extlis, nrow, ncol);
    }
    else { 
      W2_clear_point_pic(extlis);
      extlis->nrow = nrow;
      extlis->ncol = ncol;
    }
  }
  else {
      
       W2_point_pic_alloc(extlis, nrow, ncol);
   }
}


void W2_check_point_repr(EXTREP2 extrep)
{
  if (extrep->noct == 0)
    Errorf ("Run extrema2 first!\n");
}




