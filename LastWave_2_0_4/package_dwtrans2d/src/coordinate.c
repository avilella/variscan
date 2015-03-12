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
  
static float W2_magnitude( float x, float y)
  
{
  return((float)sqrt((double)(x*x + y*y)));
}

float W2_argument( float x, float y)
  
{
  if(x == 0.0 && y == 0.0)
    return(0.0);
  else
    return((float)atan2((double)y,(double)x));
}

float W2_horizontal(float mag,float arg)
  
{ 
  return((float)(mag * cos((double)arg)));
}

float W2_vertical(float mag,float arg)
  
{
  return((float)(mag * sin((double)arg)));
}


/*******************************************************************/
/* Compute magnitude and argument from horizontal and vertical in  */
/* each image level of a wtrans.                                   */
/*******************************************************************/


void W2_polar_coordinate(WTRANS2 wtrans,int level)
{
  IMAGE image_horizantal = wtrans->images[level][HORIZONTAL],
        image_vertical = wtrans->images[level][VERTICAL];
  IMAGE image_magnitude = wtrans->images[level][MAGNITUDE],
        image_angle = wtrans->images[level][ARGUMENT];

  float *values_h = (float *)image_horizantal->pixels,
        *values_v = (float *)image_vertical->pixels;
  float *values_m, *values_a ;

  int i,j,k, I;
  int nrow, ncol;

   nrow = image_horizantal->nrow;
   ncol = image_vertical->ncol;

   SizeImage(image_magnitude, nrow, ncol);
   values_m = (float *) image_magnitude->pixels;  
   SizeImage(image_angle, nrow, ncol);
   values_a =  (float *)image_angle->pixels ;

   for(i = 0, I = 0; i < nrow; i++, I += ncol)
    for(j = 0; j < ncol; j++) {
      k = I + j;
      values_m[k] = W2_magnitude(values_h[k],values_v[k]);
      values_a[k] = W2_argument(values_h[k],values_v[k]);
    }
}

/*********************************************************************/
/* Compute the polar reprezantation of wtrans                        */
/*********************************************************************/

void W2_polar_repr(WTRANS2 wtrans,int begin_level,int num_level)
{
  int l;
  for(l = begin_level; l <=  num_level; l++)    
    W2_polar_coordinate(wtrans, l);
}


/*******************************************************************/
/* Compute horizontal and vertical from magnitude and argument in  */
/* each image level of a wtrans.                                   */
/*******************************************************************/


void W2_cartesian_coordinate(WTRANS2 wtrans,int level)
{
  IMAGE image_horizontal = wtrans->images[level][HORIZONTAL],
        image_vertical = wtrans->images[level][VERTICAL];
  IMAGE image_magnitude = wtrans->images[level][MAGNITUDE],
        image_angle = wtrans->images[level][ARGUMENT];

  float *values_m = (float *)image_magnitude->pixels,
        *values_a = (float *)image_angle->pixels;
  float *values_h ,
        *values_v ;

  int i,j,k;
  int nrow, ncol;

   nrow = image_magnitude->nrow;
   ncol = image_magnitude->ncol;

   SizeImage(image_horizontal, nrow, ncol);
   values_h = (float *) image_horizontal->pixels;  
   SizeImage(image_vertical, nrow, ncol);
   values_v =  (float *)image_vertical->pixels;

   for(i = 0; i < nrow; i++)
    for(j = 0; j < ncol; j++) {
      k = i * ncol + j;
      values_h[k] = W2_horizontal(values_m[k],values_a[k]);
      values_v[k] = W2_vertical(values_m[k],values_a[k]);
    }
}


/*********************************************************************/
/* Compute the cartisian reprezantation of wtrans                    */
/*********************************************************************/

static void W2_cartesian_repr(WTRANS2 wtrans,int begin_level,int num_level)
{
  int l;
  for(l = begin_level; l <=  num_level; l++)    
    W2_cartesian_coordinate(wtrans, l);
}


/***********************************************************************/
/* The following deals with coordinate transformation in POINT_REPR    */
/***********************************************************************/

/* Compute horizontal and vertical of a ext from its magnitude and   */
/* argument.                                                           */

static void W2_point_cartesian(EXT2 ext)
{
  ext->hor = W2_horizontal(ext->mag,ext->arg);
  ext->ver = W2_vertical(ext->mag,ext->arg);
}

/* Compute horizantals and verticals of a extlis from known         */
/* magnitude and argument.                                             */


static void W2_point_pic_cartesian(EXTLIS2 extlis)
{
  int nrow = extlis->nrow,
      ncol = extlis->ncol;
  EXT2 *values = extlis->first;
  EXT2 ext;
  int i,I,j;
  for(i = 0,I = 0; i < nrow; i++,I += ncol)
    for(j = 0; j < ncol; j++)
      if(ext = values[I+j])
        W2_point_cartesian(ext);
}

/********* Compute cartisian in a extrep *******************/

void W2_point_repr_cartesian(EXTREP2 extrep)
{
  int l;
  int noct;
  noct = extrep->noct;
  
  for(l = 1; l <= noct; l++)
    W2_point_pic_cartesian(extrep->array[l]);
}


