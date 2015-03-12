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

void W2_point_print(EXT2 ext)
{
  Printf("x = %3d, y = %3d, mag = %6.2f, arg = %5.2f\n",
	 ext->x, ext->y, ext->mag, ext->arg/M_PI);
}
          

/*********************************************************/
/** The functions deal with points print out. ************/
/*********************************************************/
 
void C_Point2Print(char **argv)
{
  WTRANS2 wtrans=NULL;
  int level;
  int row_min, row_max, col_min, col_max;
  int i, j;
  EXT2 ext;
  EXTLIS2 extlis;
  int ncol, nrow;
  EXT2 *values;
  int zap, nextline_flag = NO;
  char car;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,-1);
  if (wtrans ==NULL)   wtrans= GetWtrans2Cur();
 
 
  if (!INRANGE(1, level, wtrans->extrep->noct))
   Errorf("Invalid level \n");

  extlis = wtrans->extrep->array[level];

  ncol = extlis->ncol;
  nrow = extlis->nrow;
  values = (EXT2 *)extlis->first;

  row_min = 0;
  row_max = nrow - 1;
  col_min = 0;
  col_max = ncol - 1;

  while (car=ParseOption(&argv)) {
    switch (car) {
    case 'r' :
      argv = ParseArgv(argv,tINT,&row_min,tINT,&row_max,-1);
      if (row_min > row_max) {zap= row_min; row_min= row_max; row_max= zap;}
      break;
    case 'c' :
      argv = ParseArgv(argv,tINT,&col_min,tINT,&col_max,-1); 
      if (col_min > col_max) {zap= col_min; col_min= col_max; col_max= zap;}
      break;
    default :
      ErrorOption(car);
    }
  }

  NoMoreArgs(argv);
  /* check whether valid row and col, if not put into boundary */
  if (INRANGE(0,row_min,nrow-1) == NO)
    row_min = 0;
  if (INRANGE(0,row_max,nrow-1) == NO)
    row_max = extlis->nrow - 1;
  if (INRANGE(0,col_min,ncol-1) == NO)
    col_min = 0;
  if (INRANGE(0,col_max,ncol-1) == NO)
    col_max = ncol - 1;

  for (i = row_min; i <= row_max; i++)
    for (j = col_min; j <= col_max; j++)
      if (ext = values[i * ncol + j]) {
     	W2_point_print(ext);
      }
  if (nextline_flag == NO) Printf("\n");  
}

/*********************************************/
/***    propagation chain print out        ***/
/*********************************************/

static void W2_prop_print(EXT2 ext)
{
  for (ext; ext; ext = ext->coarser)
     W2_point_print(ext);
  Printf("\n");
}

static void W2_propagation_print(EXTLIS2 extlis,int row_min,int row_max,int col_min,int col_max)
{
  int i, j, k, ncol;
  EXT2 *values;
  EXT2 ext;

  ncol = extlis->ncol;
  values = extlis->first;

  for (i = row_min; i <= row_max; i++)
    for (j = col_min; j <= col_max; j++) {
      k = i * ncol + j;
      if (ext = values[k]) W2_prop_print(ext);
    } 
}     




/*************************************************************/
/* print out the points information at the propagation curve */
/* defined within block -r .. and -c ...                     */
/*************************************************************/

void C_Point2ProPrint(char **argv)
{
  EXTLIS2 extlis;
  EXTREP2 extrep;
  WTRANS2 wtrans=NULL;
  int row_min, row_max, col_min, col_max;
  int zap;
  int nrow, ncol;
  int level;
  char car;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&level,-1);
  if (wtrans ==NULL)   wtrans= GetWtrans2Cur();
 
 
  if (!INRANGE(1, level, wtrans->extrep->noct))
   Errorf("Invalid level \n");


  extrep = wtrans->extrep;
  extlis = extrep->array[level];
  nrow = extlis->nrow;
  ncol = extlis->ncol;

  row_min = 0;
  row_max = nrow - 1;
  col_min = 0;
  col_max = ncol - 1;

  while (car=ParseOption(&argv)) {
    switch (car) {
    case 'r' :
      argv = ParseArgv(argv,tINT,&row_min,tINT,&row_max,-1);
      if (row_min > row_max) {zap= row_min; row_min= row_max; row_max= zap;}
      break;
    case 'c' :
      argv = ParseArgv(argv,tINT,&col_min,tINT,&col_max,-1); 
      if (col_min > col_max) {zap= col_min; col_min= col_max; col_max= zap;}
      break;
    default :
      ErrorOption(car);
    }
 }   
  /* check whether valid row and col, if not put into boundary */
  if (INRANGE(0,row_min,nrow-1) == NO)
    row_min = 0;
  if (INRANGE(0,row_max,nrow-1) == NO)
    row_max = extlis->nrow - 1;
  if (INRANGE(0,col_min,ncol-1) == NO)
    col_min = 0;
  if (INRANGE(0,col_max,ncol-1) == NO)
    col_max = ncol - 1;

  W2_propagation_print(extlis, row_min, row_max, col_min, col_max);

}


