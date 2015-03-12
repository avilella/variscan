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
/* static void W2_corresponding_coarser_finer(EXTLIS2 fextlis,EXTLIS2 extlis) */
void W2_corresponding_coarser_finer(EXTLIS2 fextlis,EXTLIS2 extlis)
{
  int i, j, nrow, ncol;
  EXT2 *fvalues, *values, fpoint, ext;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  fvalues = fextlis->first;
  values = extlis->first;

  for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++)
      if((fpoint = fvalues[i * ncol + j]) && 
	 (ext = values[i * ncol + j])) {
	fpoint->coarser = ext;
	ext->finer = fpoint;
      }
}

  

/************************************************************/
/* describe the chain structure in fine from that  in coarse */
/*************************************************************/


static W2_chain_buffer_statistics(EXTLIS2 extlis, CHAINLIS2 chainlis,int buffersize)
{
  int nrow, ncol, nb, row_l, col_l, row_r, col_r, m, n;

  EXT2 *values, ext, finger;
  CHAIN2 chain;
  float sum, magfactor, averagemag;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;

 for(chain = chainlis->first; chain; chain = chain->next) {
      sum = 0.0;
      nb = 0;
      for (ext = chain->first; ext; ext = ext->next) {
        W2_neighborbox(buffersize, ext->y, ext->x, ncol, nrow,
                    &col_l,&col_r,&row_l,&row_r);
        for(m = row_l; m <= row_r; m++)
          for(n = col_l; n <= col_r; n++)
            if(finger = values[m * ncol + n]) {
              if(finger->scale % 2 == 0) { /* not been visited */
                sum += finger->mag; /* only count once */
                nb ++;
                finger->scale = finger->scale -1;
              }
          }
      }
      for (ext = chain->first; ext; ext = ext->next) {
        W2_neighborbox(buffersize, ext->y, ext->x, ncol, nrow, 
                    &col_l, &col_r, &row_l, &row_r);
        for(m = row_l; m <= row_r; m++)
          for(n = col_l; n <= col_r; n++)
            if(finger = values[m * ncol + n]) {
              if(finger->scale % 2 == 1) finger->scale = finger->scale + 1;
            }
      }
      if((nb == 0) || (chain->mag < 0.5) || 
         (sum < 0.5)) magfactor = 1.0;
      else {
        averagemag = sum/nb;
        magfactor = averagemag / chain->mag ;
      }
      chain->var = magfactor; /* tmp_storage */
    }
}

/* modify position according to the coordinate at scale 4 and 8 */


static void W2_modify_position(EXTLIS2 extlis)
{
  int nrow, ncol, i, j;
  EXT2 *values, ext, cpoint;
  float x1, y1, x2, y2, x3, y3;

  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;

  for( i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++)
      if(ext = values[i * ncol + j]) {
	if(ext->coarser) {
	  cpoint = ext->coarser;
	  x2 = (float)cpoint->x;
	  y2 = (float)cpoint->y;
	  if(cpoint->coarser) {
	    x3 = cpoint->coarser->x;
	    y3 = cpoint->coarser->y;
	  }

	  if(cpoint->coarser) {
	    x1 = (3.0 * x2 - x3)/2.;
	    y1 = (3.0 * y2 - y3)/2.;
	  }
	  else {
	    x1 = x2;
	    y1 = y2;
	  }
	}
	ext->x = (int) (x1+0.5);
	ext->y = (int) (y1 + 0.5);
      }
}



void C_chain_focus(char ** argv)
 {
  WTRANS2 wtrans;
 char car;
 
  int newpicflag = YES, ratioflag = NO, positionflag = NO;
  int buffersize, level;

  Printf("\n*argv = %s\n",*argv);
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,tINT_,2, &buffersize,-1);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
  
  if (level < 2) return;


  while  (car=ParseOption(&argv))
    switch (car) {
    case 'r':
      ratioflag = YES;
      break;
    case 'n':
      newpicflag = NO;
      break;
    case 'p':
      positionflag = YES;
      break;
    default :
      	ErrorOption(car);
    }
    
 W2_chain_focus ( wtrans, level, buffersize,  newpicflag, ratioflag, positionflag);  
}


void W2_chain_focus (WTRANS2 wtrans, int level, int buffersize,  int newpicflag, int ratioflag, int positionflag)
    {
  EXTLIS2 fextlis, extlis;
  CHAINLIS2 chainlis, newchainlis;
  CHAIN2 chain;
  EXT2 ext, *fvalues, *values;
  int  i, j, nrow, ncol, k;


  extlis = wtrans->extrep->array[level];
  chainlis = wtrans->chainrep->array[level];
  fextlis = wtrans->extrep->array[level-1];

  if(newpicflag) {
    W2_clear_point_pic(fextlis);

    nrow = extlis->nrow;
    ncol = extlis->ncol;

    W2_point_pic_copy(extlis, fextlis);
    newchainlis = NewChainLis2();
    W2_chain_pic_first_point_copy(chainlis,newchainlis,fextlis);
    wtrans->chainrep->array[level-1] = newchainlis;

    if(positionflag) {
      W2_corresponding_coarser_finer(fextlis, extlis);
      W2_modify_position(fextlis);
      W2_point_pic_relocate(fextlis,wtrans->chainrep->array[level-1]);
    }

    /* adjust ext->mag  */
    if(ratioflag) {
      W2_update_chain_pic(chainlis);
      W2_chain_buffer_statistics(fextlis, chainlis, buffersize);
      values = extlis->first;
      fvalues = fextlis->first;

      for(i = 0; i < nrow; i++)
	for(j = 0; j < ncol; j++) {
	  if (ext = values[i * ncol + j]) {
	    k = ext->y * ncol + ext->x;
	    fvalues[k]->mag = ext->mag * ext->chain->var;
	  }
	}
    }
  }
  else {
    /* copy to finer */
    for(chain = chainlis->first; chain; chain = chain->next)
      for(ext = chain->first; ext; ext = ext->next) {
	if(ext->finer) {
	  if(ratioflag) {
	    ext->finer->mag = chain->var * ext->mag;
	    Printf("magfactor %f \n", chain->var);
	  }
	  else ext->finer->mag = ext->mag;
          ext->finer->arg = ext->arg;
	}
      }
  }
}


	  
	  

