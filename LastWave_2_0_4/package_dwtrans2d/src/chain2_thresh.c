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
/***********************************************************/
/* delete chains based on its length, average magnitude or */
/* other features such as whether its coarser exists       */
/***********************************************************/

static void W2_chain_pic_thresh( WTRANS2 wtrans,int level,int thresh_size,float thresh_mag,int orflag,int propflag,int hascoarser)
    
{
  CHAINLIS2 chainlis;
  EXTLIS2 extlis;
  CHAIN2 chain;

  extlis = wtrans->extrep->array[level];
  chainlis = wtrans->chainrep->array[level];

  W2_update_chain_pic(chainlis);

  for(chain = chainlis->first; chain;chain = chain->next){
    if(orflag) {
      if(chain->size <= thresh_size || chain->mag <= thresh_mag) {
        if(propflag) DeleteChain2Prop(wtrans, chain, level);
        else  DeleteChain2(extlis, chainlis, chain);
      }
    }
    else {
      if(chain->size <= thresh_size && chain->mag <= thresh_mag)
        if(propflag) DeleteChain2Prop(wtrans, chain, level);
        else DeleteChain2(extlis, chainlis, chain);
    }
  }
}

/****************************************************************/
/** threshing chainrep according to chain size and magnitude **/
/****************************************************************/

void W2_chain_repr_thresh(WTRANS2 wtrans,int thresh_size,float thresh_mag,int level,int orflag,int propflag,int hascoarser)
   
{
  int l;

  if (INRANGE(1,level,wtrans->noct)) 
    W2_chain_pic_thresh(wtrans,level,thresh_size,thresh_mag,
                     orflag,propflag,hascoarser);
  else {
    for(l = 1; l <= wtrans->chainrep->noct; l++)
      W2_chain_pic_thresh(wtrans,l,thresh_size,thresh_mag,
                       orflag,propflag,hascoarser);
  }

}



/****************************/
/** threshing a chainrep **/
/****************************/

void C_ChainReprThresh2(char **argv)
{
  int thresh_size,level,orflag = YES,propflag = NO, hascoarser = NO;
  float thresh_mag;
  WTRANS2 wtrans=NULL;
  char car;

  thresh_size = 0;
  thresh_mag = 0.0;
  
 
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,-1);

   if (wtrans ==NULL)   wtrans= GetWtrans2Cur();

   while (car=ParseOption(&argv)) {
       switch (car) {
      case 'l':
	argv = ParseArgv(argv,tINT,&thresh_size,-1);
        break;
      case 'a':
	argv = ParseArgv(argv,tFLOAT,&thresh_mag,-1);
         break;
      case 'p':
	propflag = YES;
	break;
      default :
	 ErrorOption(car);
    }
   }
  NoMoreArgs(argv);
  W2_chain_repr_thresh(wtrans,thresh_size,thresh_mag,level,
		    YES,propflag,NO);
}


