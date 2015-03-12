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
int W2_test_maxima(int k0,float *image_m,int k,int nrow,int ncol)
{
  if (image_m[k] >= image_m[k+k0])
    return(YES);
  else
    return(NO);
}


/******* compute the neighor box  ***********/

void W2_neighborbox(int boxsize,int row,int col,int  ncol,int nrow,int*xbox0,int *xbox1,int *ybox0,int *ybox1)
{
  *ybox0 = MAX(0, row - boxsize); /* neighbor box in finer_pic */ 
  *ybox1 = MIN(nrow - 1, row + boxsize);
  *xbox0 = MAX(0, col - boxsize);
  *xbox1 = MIN(ncol - 1, col + boxsize);
}  
	
/**********************************************/
#define W2_POINT_DISTANCE 512
/* 
 * M_PI is defined in <math.h>
#define M_PI 3.14159265
 *
*/
static void W2_interp(float u0, float un, int n, float *u, float r1)
{
  double r_1, r_2, r2, rn_1, r2n_2, r2n, r2n_2i, rn_i, ri, r2i, a0, an;
  int i;

  rn_1 = r1;
  for(i=1; i < n-1 ; ++i)
    rn_1 *= r1;
  r2n_2 = rn_1 * rn_1;
  r_1 = 1. / r1;
  r_2 = r_1 * r_1;
            r2 = r1 * r1;
  r2n = r2n_2 * r2;

  a0 = u0  / (1. - r2n);
  an = un  / (1. - r2n);

  u[0] = u0;
  r2n_2i = r2n_2;
  r2i = r2;
  ri = r1;
  rn_i = rn_1;
  for (i = 1; i < n; i++) {
    u[i] = a0 * ri * (1 - r2n_2i) + an * rn_i * (1 - r2i) ;

    r2n_2i *= r_2;
    r2i *= r2;
    ri *= r1;
    rn_i *= r_1;
  }
/*   u[n] = un; */

}

static void W2_pt_level_proj_1st(WTRANS2 wtrans,int level)
{
  EXTLIS2 extlis = wtrans->extrep->array[level];
  EXT2 *point_image;
  EXT2 ext;
  int nrow = extlis->nrow,
      ncol = extlis->ncol;
  IMAGE pic_h = wtrans->images[level][HORIZONTAL];
  IMAGE pic_v = wtrans->images[level][VERTICAL];
  float *image_h = (float *)pic_h->pixels,
        *image_v = (float *)pic_v->pixels;
  int i,I,j;
  int t0,t1;
  int scale = 1<<level;
  float e0,e1, a, exponent;
  int k,n;
  float u[W2_POINT_DISTANCE];

  exponent = 2. / (double)(scale);
  a = 1. / pow(5.8 , (double)exponent);
  /* a = pow(2.7182817 ,2/ (double)(scale));  */
  /* a=exp(1./(double)(scale)); */
  
  point_image = (EXT2 *) extlis->first;
  for(i = 0,I = 0; i < nrow; i++,I += ncol) {
      t0= I;
      e0= 0.0-image_h[t0];
      while(t0 < I+ncol-1) {
        for(t1 = t0+1; t1 < I+ncol-1; t1++)
          if(ext= point_image[t1]) break;
        if(ext) {
          e1= ext->hor-image_h[t1];
        } else {
          t1= I+ncol-1;
          e1= 0.0-image_h[t1];
	}

        n= t1-t0;
        W2_interp(e0, e1, n, u, a);
        for(k= 0; k < n; k++)
	  image_h[t0+k]+= u[k];
        t0 = t1;
        e0 = e1;
      }
      image_h[I+ncol-1]= 0.0;
  }

  for(j = 0; j < ncol; j++) {
      t0= j;
      e0= 0.0-image_v[t0];
      while(t0 < j+(nrow-1)*ncol) {
        for(t1 = t0+ncol; t1 < j+(nrow-1)*ncol; t1+=ncol)
          if(ext= point_image[t1]) break;
        if(ext) {
          e1= ext->ver-image_v[t1];
        } else {
          t1= j+(nrow-1)*ncol;
          e1= 0.0-image_v[t1];
        }
	n= (t1-t0)/ncol;
        W2_interp(e0, e1, n, u, a);
        for(k = 0; k < n; k++)
          image_v[t0+k*ncol]+= u[k];
        t0 = t1;
        e0 = e1;
      }
      image_v[j+(nrow-1)*ncol]= 0.0;
  }

}

static void W2_pt_level_proj_2nd(WTRANS2 wtrans,int level)
{
  EXTLIS2 extlis = wtrans->extrep->array[level];
  EXT2 *point_image = extlis->first;
  EXT2 ext;
  IMAGE pic_m = wtrans->images[level][MAGNITUDE];
  IMAGE pic_a = wtrans->images[level][ARGUMENT];

  int nrow = pic_m->nrow,
      ncol = pic_m->ncol;
  float *image_m = (float *)pic_m->pixels,
        *image_a = (float *)pic_a->pixels;

  int i,I,j;
  int t0,t1;
  int t_min;
  float m0,m1;
  float m_min;

  for(i = 1,I = ncol; i < nrow-1; i++,I += ncol) {
    t0 = t1 = I;
    m0 = image_m[t0];
    while(t0 < I+ncol-1) {
      while(++t1 < I+ncol-1)
        if(ext = point_image[t1]) {
          m1 = ext->mag;
          break;
        }
      if(t1 == I+ncol-1) {
        if(t0 == I)
          break;
        else
          m1 = image_m[t1];
      }

      m_min = image_m[t_min=t0];
      for(j = t0+1; j <= t1; j++)
        if(image_m[j] < m_min)
          m_min = image_m[t_min=j];
      if(m0 > m_min)
        for(j = t0+1; j < t_min; j++) {
          if(fabs(fabs(image_a[j])/M_PI-0.5) >= 0.375) {
            if(image_m[j] > image_m[j-1])
              image_m[j] = image_m[j-1];
          } else {
            break;
          }
        }
      if(m1 > m_min)
        for(j = t1-1; j > t_min; j--) {
          if(fabs(fabs(image_a[j])/M_PI-0.5) >= 0.375) {
            if(image_m[j] > image_m[j+1])
              image_m[j] = image_m[j+1];
          } else {
            break;
          }
        }

      t0 = t1;
      m0 = m1;
    }
  }

  for(j = 1; j < ncol-1; j++) {
    t0 = t1 = j;
    m0 = image_m[t0];
    while(t0 < (nrow-1)*ncol+j) {
      while((t1 += ncol) < (nrow-1)*ncol+j)
        if(ext = point_image[t1]) {
          m1 = ext->mag;
          break;
        }
      if(t1 == (nrow-1)*ncol+j) {
        if(t0 == j)
          break;
        else
          m1 = image_m[t1];
      }

      m_min = image_m[t_min=t0];
      for(i = t0+ncol; i <= t1; i += ncol)
        if(image_m[i] < m_min)
          m_min = image_m[t_min=i];
      if(m0 > m_min)
        for(i = t0+ncol; i < t_min; i += ncol) {
          if(fabs(fabs(image_a[i])/M_PI-0.5) <= 0.125) {
            if(image_m[i] > image_m[i-ncol])
              image_m[i] = image_m[i-ncol];
          } else {
            break;
          }
        }
      if(m1 > m_min)
        for(i = t1-ncol; i > t_min; i -= ncol) {
          if(fabs(fabs(image_a[i])/M_PI-0.5) <= 0.125) {
            if(image_m[i] > image_m[i+ncol])
              image_m[i] = image_m[i+ncol];
          } else {
            break;
          }
        }

      t0 = t1;
      m0 = m1;
    }
  }

}
 
static void W2_point_level_projection(WTRANS2 wtrans,int level,int clipping)
{
   W2_pt_level_proj_1st(wtrans,level);
  if(clipping == YES) {
    W2_polar_coordinate(wtrans,level); 
    W2_pt_level_proj_2nd(wtrans,level);
    W2_cartesian_coordinate(wtrans, level); 
  }
}

void W2_point_repr_projection(WTRANS2 wtrans,int clipping)
{
  int l;

  for(l = 1; l <= wtrans->extrep->noct; l++) {
    W2_point_level_projection(wtrans, l, clipping);
  }
  CopyImage(wtrans->extrep->coarse, wtrans->images[wtrans->noct][0]);

} 
