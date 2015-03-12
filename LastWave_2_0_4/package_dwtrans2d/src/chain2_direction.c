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
/**************************************************************************/
/* decide the order the point1 and point2 in a chain. Default is previous */
/**************************************************************************/

static void W2_chain_order(EXT2 point1, EXT2 point2,int *nextflag)
{
  EXT2 finger;
  
  finger = point1;
  while(finger) {
    if (finger != point2)  finger = finger->next;
    else break;
  }

  if(finger == point2) *nextflag = YES;
  else *nextflag = NO;
}


void W2_chain_order_reverse(EXT2 point1,EXT2 point2,int *nextflag)
{
  EXT2 finger;
  
  finger = point1;
  while(finger) {
    if (finger != point2)  finger = finger->next;
    else break;
  }

  if(finger) {
    *nextflag = YES;
    return;
  }

  finger = point1;
  while(finger) {
    if(finger != point2) finger = finger->previous;
    else break;
  }

  if(finger) {
    *nextflag = NO;
    return;
  }

  *nextflag = -1;
  return;
}

/****************************************************************************/
/* adjust the previous and next pointers of all the chains in the chainlis */
/* such that their relation defined by next or previous is the same as      */
/* the chain in the coarser scale that they are associated with             */
/****************************************************************************/

void W2_chainning_direction(CHAINLIS2 chainlis)
{
  CHAIN2 chain;
  EXT2 point1, point2, finger1, finger2;
  extern void W2_chain_reverse(CHAIN2 chain);

  int nextflag;
  int reverse;

  reverse = 0;

  for(chain =chainlis->first; chain; chain = chain->next) {
    point2 = chain->first;

    while(point2) {
      point2 = W2_first_coarser(point2);

      if(point2) finger2 = point2->coarser; /* 2 1 */
      else finger2 = NULL;
      if(point2) {
	point1 = point2->next;
	point1 = W2_first_coarser(point1);
      }
      else point1 = NULL;

      if(point1) finger1 = point1->coarser;
      else finger1 = NULL;
    
      nextflag = YES;
      if(finger1 && finger2) 
	W2_chain_order(finger2, finger1, &nextflag);
      if(nextflag == NO) reverse++;
      else reverse--;

      point2 = point1;
    }

    if (reverse > 0) W2_chain_reverse(chain); 
  }
}


    
    
    
    
