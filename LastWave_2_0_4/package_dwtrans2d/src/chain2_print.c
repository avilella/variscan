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

/****************************************************************************/
/* The following prints out the information in a chain                      */
/****************************************************************************/

static void W2_chain_print(CHAIN2 chain,int pointflag ,int nb)
{
  EXT2 ext;
  int i;

  Printf("size(length) =%5d  mag(amplitude) =%f var(magnitude) = %f\n",
	 chain->size,chain->mag,chain->var);
  if(pointflag) {
    i = 0;
    for(ext = chain->first; ext; ext = ext->next) {
      if(i > nb) break;
      W2_point_print(ext);
      i++;
    }
  }
  Printf("\n");
}

/*************************************************************************/
/* print out the context of the chain with sufficient large size and amp */
/*************************************************************************/

static void W2_chain_pic_print(CHAINLIS2 chainlis,int  pointflag,int  length,float  amp,int nb)
{
  CHAIN2 chain;

  for(chain = chainlis->first; chain; chain = chain->next) {
    if((chain->mag > amp) && (chain->size > length)) {
      if(pointflag && (nb == 0)) nb = chain->size;
      W2_chain_print(chain,pointflag,nb);
    }
  }
}

/****************************************/
/* print out the chain in the chainlis */
/****************************************/

void C_ChainPicPrint2(char **argv)
{
  int level, length;
  float amp;
  int pointflag = NO;
  int nb;
  WTRANS2 wtrans=NULL;
  CHAINREP2 chainrep;
  char car;

  length = 0;
  amp = 0.000001;

  
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,-1);
  if (wtrans ==NULL)   wtrans= GetWtrans2Cur();

  chainrep = wtrans->chainrep;

  while (car=ParseOption(&argv)) {
    switch (car) {
    case 'p':
      pointflag = YES;
      argv = ParseArgv(argv,tINT_,0,&nb,-1);
      break;
    case 'l' :
      argv = ParseArgv(argv,tFLOAT,&length,-1);
      break;
    case 'a' :
      argv = ParseArgv(argv,tFLOAT,&amp,-1);
      break;
    default :
      ErrorOption(car);
    }
  }
  NoMoreArgs(argv);
  if(INRANGE(1,level,chainrep->noct))
    W2_chain_pic_print(chainrep->array[level],pointflag,length,amp,nb);
  else Printf("level is out of bound \n");
}

/**************************/
/* Print CHAIN_PIC fields */
/**************************/

/* 'cinfo' command */
void C_C2Info(char **argv)
{
  WTRANS2 wtrans=NULL;
  CHAINREP2 chainrep;
  CHAINLIS2  chainlis;
  int level;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,-1);
  if (wtrans ==NULL)   wtrans= GetWtrans2Cur();


  chainrep = wtrans->chainrep;
  if (INRANGE(1,level,chainrep->noct)) {
    chainlis = chainrep->array[level];
    Printf("\n");
    Printf(" nrow  :  %d  \n",chainlis->nrow);
    Printf(" ncol  :  %d  \n",chainlis->ncol);
    Printf(" size  :  %d  \n",chainlis->size);
    Printf(" \n");
  }
 else Printf("level is out of bound \n");
}
