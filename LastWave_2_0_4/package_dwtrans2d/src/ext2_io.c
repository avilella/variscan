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
extern void TestDebug(IMAGE image,int n,char * s);;
extern void TestDebug2(float val,int n);
/****************************************/
/********** write extrep in horizontal and vertical************/
/****************************************/

void W2_horvertpointpic2image(EXTLIS2 extlis, IMAGE hor_image,IMAGE  vert_image)
{
  EXT2 *values, ext;
  int nrow, ncol;
  int i;
  float *hor_values;
  float *vert_values;

 
  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = (EXT2 *)extlis->first;

  SizeImage(hor_image, nrow, ncol);
  SizeImage(vert_image, nrow, ncol);
  ZeroImage(hor_image);
  ZeroImage(vert_image);

  hor_values = (float *)hor_image->pixels;
  vert_values = (float *)vert_image->pixels;
 
  for(i = 0; i < nrow * ncol ; i++) {
    ext = values[i];
    if(ext != NULL) {  
      /* TestDebug2 (hor_values[i],0); */
      hor_values[i] = W2_horizontal( ext->mag,ext->arg);
      vert_values[i] = W2_vertical( ext->mag,ext->arg); 
      /* TestDebug2 (hor_values[i],1); */
    }
  }
  
  
}

/***********************************/
/* take extlis to wavelet level */
/***********************************/

static void W2_pointpic2image(EXTLIS2 extlis, IMAGE mag_image,IMAGE arg_image,char flagPoint)
{
  EXT2 *values, ext;
  int nrow, ncol;
  int i;
  float *mag_values;
  float *arg_values;


  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = (EXT2 *)extlis->first;

  SizeImage(mag_image, nrow, ncol);
  SizeImage(arg_image, nrow, ncol);
  ZeroImage(mag_image);
  ZeroImage(arg_image);

  mag_values = (float *)mag_image->pixels;
  arg_values = (float *)arg_image->pixels;

  for(i = 0; i < nrow * ncol ; i++) {
    ext = values[i];
    if(ext != NULL) { 
      if (!flagPoint) mag_values[i] = ext->mag;
      else mag_values[i] = 1;
      arg_values[i] = ext->arg;
    }
  }
}



void C_PointPic2Image(char **argv)
     
{
  EXTREP2 extrep;
  EXTLIS2 extlis;
  IMAGE mag_image, arg_image;
  int level;
  WTRANS2 wtrans2;
  char flagPoint;
  char opt;
  
  argv= ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,-1);
  if (wtrans2==NULL) wtrans2= GetWtrans2Cur();
 
  extrep = wtrans2->extrep;
  argv = ParseArgv(argv,tINT,&level,tIMAGE,&mag_image,tIMAGE,&arg_image,-1);
 
  if (!INRANGE(1, level, extrep->noct)) 
    Errorf("invalid level \n");

  flagPoint = NO;
  while (opt= ParseOption(&argv)) {
    switch (opt) {
    case 'p':
      flagPoint = YES;
      break;
    default:
      ErrorOption(opt);
    } 
 }
 NoMoreArgs(argv);

  extlis = extrep->array[level];

  W2_pointpic2image(extlis, mag_image, arg_image,flagPoint);
}


void C_Image2PointPic(char **argv)
     
{
  
  
  IMAGE mag_image, arg_image;
  int level,orientation;
  WTRANS2 wtrans2;

    argv = ParseArgv(argv,tIMAGE,&mag_image,tIMAGE,&arg_image,tINT,&level,tINT_,1, &orientation,tWTRANS2_,NULL,&wtrans2,0);
  if (wtrans2==NULL) wtrans2= GetWtrans2Cur();
 
   W2_compute_point( wtrans2->extrep->array[level],level,mag_image,arg_image,orientation);
  
 if (wtrans2->extrep->noct <level) wtrans2->extrep->noct=level;
   
}
