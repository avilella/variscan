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
/* walk along maxline to the finest level from ext */

EXT2 W2_walk_to_finest(EXT2 ext)
{
  while(ext && ext->finer)
    ext = ext->finer;
  return(ext);
}

/* rotate to the same signs because PI and -PI are the same */

void W2_rotate_to_same_sign(EXT2 point1, EXT2  point2,int *  rotateflag, float *rotate_theta)
{

  if ((point1->arg <= -1. * M_PI/2.0) && (point2->arg >= M_PI/2.0)) {
    *rotateflag = 1; /* rotate point1 */
    *rotate_theta = M_PI + point1->arg; /* > 0. */ 
    point2->arg = point2->arg - *rotate_theta; /* both are positive */

    point1->arg = M_PI;
  }
  if ((point2->arg <= -1. * M_PI/2.0) && (point1->arg >= M_PI/2.0)) {
    *rotateflag = 2; /* rotate point2 */
    *rotate_theta = M_PI + point2->arg; 
    point1->arg = point1->arg - *rotate_theta; /* both are positive */
    point2->arg = M_PI;
  }
}

