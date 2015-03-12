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

extern int W2_mot_proc;


static int W2_iexp2(int j)
{
  int k;
  int s;
  s = 1;
  for(k = 0; k < j; k++)
    s += s;
  return(s);
}
/* copied from $wave2/vchain_src/vchain_quant.c */
static float W2_modulo2PI(float value)
{
  if (value > M_PI)
    return(value - 2 * M_PI);
  else if (value < - M_PI)
    return(value + 2 * M_PI);
  else
    return(value);

}
/**** quantumize the argument into three closet directions  *****/
/**** useful in the chain computation                       *****/

void W2_direction(float argument,int * dir,int *dir1,int *dir2)
{
  float fdir, rdir;
  
  fdir = argument * 4.0/M_PI + 8.0;
  rdir = (int) (fdir<0 ? fdir-0.5 : fdir+0.5);

  *dir = ((int)rdir) % 8 ;  /* the 1st */
  if (fdir > rdir) {
    *dir1 = (*dir + 1) % 8; /* the 2nd */
    *dir2 = (*dir + 7) % 8; /* the 3rd */
  } 
  else if (fdir < rdir) {
    *dir1 = (*dir + 7) % 8;
    *dir2 = (*dir + 1) % 8;
  } else {
    *dir2 = *dir1 = *dir;  /* degenerate case */
  }
}


static void W2_double_dir(float argument,int * dir)
{
  float fdir, rdir;
  
  fdir = argument * 2.0/M_PI + 4.0;
  rdir = (int) (fdir<0 ? fdir-0.5 : fdir+0.5);

  *dir = 2 * (((int)rdir) % 4) ;  
}

static int W2_opposit_dir(float arg1,float arg2)
{
  int a,b , dir;
  float  diff;

  diff = W2_modulo2PI(arg1 - arg2);
  W2_direction(diff,&dir,&a,&b);
  if( (dir == 3) || (dir == 4) || (dir == 5) )
    return(YES);
  else
    return(NO);
}


void  W2_compute_point(EXTLIS2 extlis,int level,IMAGE image_magnitude, IMAGE image_argument,int  orientation)
    
     
{
  int nrow, ncol;
  int i,j, I, k, k0;
  int is_max_point, dir,dir0,dir1,dir2,bol1,bol2,bol3,bol4;
  int horizontal_flag, vertical_flag, point_potential;
  float *values_magnitude, *values_argument;
  float mag, argument;
  EXT2 ext;
 ;

  nrow = image_magnitude->nrow;
  ncol = image_magnitude->ncol;
  values_magnitude = (float *)image_magnitude->pixels;
  values_argument = (float *)image_argument->pixels;

 
  W2_point_pic_alloc(extlis, nrow, ncol);

  for (i = 0, I = 0; i < nrow; i++, I += ncol)
    for (j = 0; j < ncol; j++) {
      k = I + j;
      mag = values_magnitude[k];
      argument = values_argument[k];
      W2_double_dir(argument,&dir);

      switch (dir) {
      case 0: 
	k0 = -1;
	break;
      case 4: 
	k0 = 1;
	break;
      case 2: 
	k0 = -ncol;
	break;
      case 6: 
	k0 = ncol;
	break;
      }

      if ((((i == 0) || (i == nrow - 1)) && ((dir == 2) || (dir == 6))) ||
	  (((j == 0) || (j == ncol -1)) && ((dir == 0) || (dir == 4))) )
	point_potential = NO;
      else point_potential = YES;

      if (point_potential) {
	if(orientation == NO) {
	  bol1 =  (values_magnitude[k] > values_magnitude[k+k0]) ;

	  bol2 = bol1 &&  W2_test_maxima(-k0,values_magnitude,k,nrow,ncol); 

	  bol3 = (values_magnitude[k] > values_magnitude[k-k0]) ;

	  bol4 = bol3 &&  W2_test_maxima(k0,values_magnitude,k,nrow,ncol); 
	}
	else {
	  bol1 = ( (values_magnitude[k] > values_magnitude[k+k0]) 
		  || W2_opposit_dir(values_argument[k],values_argument[k+k0]));

	  bol2 = (bol1 && (W2_test_maxima(-k0,values_magnitude,k,nrow,ncol)
	       || W2_opposit_dir(values_argument[k],values_argument[k-k0])) );

	  bol3 = ( (values_magnitude[k] > values_magnitude[k-k0]) 
	       || W2_opposit_dir(values_argument[k],values_argument[k-k0]) );

	  bol4 = (bol3 && (W2_test_maxima(k0,values_magnitude,k,nrow,ncol)
	        || W2_opposit_dir(values_argument[k],values_argument[k+k0]) ) );
	}

	is_max_point = bol2 || bol4 ;

	if (is_max_point) {
	  ext = NewExt2();
	  ext->scale =W2_iexp2(level);
	  ext->x = j;
	  ext->y = i;
	  ext->mag = mag;
	  ext->arg = argument;
	  extlis->first[k] = ext;
	  extlis->size++;
	}
      }
    }

  /* At the finner level we fill the wholes of maxima whose directions
     is diagonales, when needed */
  if(level == 1) {
    for (i = 1, I = ncol; i < nrow - 1; i++, I += ncol)
      for (j = 1; j < ncol - 1; j++) {
	k = I + j;
	mag = values_magnitude[k];
	argument = values_argument[k];
	W2_direction(argument,&dir0,&dir1,&dir2);
	horizontal_flag = NO;
	vertical_flag = NO;

	/* If the direction is diagonal, we check wether their can be
	   a potential whole */
	switch (dir0) {
	case 1: 
	case 5: 
	  bol1 = (extlis->first[k-1+ncol] &&
		  (extlis->first[k-ncol] || 
		   extlis->first[k-ncol+1] || 
		   extlis->first[k+1]));
	  bol2 = (extlis->first[k+1-ncol] &&
		  (extlis->first[k+ncol] || 
		   extlis->first[k+ncol-1] || 
		   extlis->first[k-1]));
		  break;
	case 3: 
	case 7: 
	  bol1 = (extlis->first[k+1+ncol] &&
		  (extlis->first[k-ncol] || 
		   extlis->first[k-ncol-1] || 
		   extlis->first[k-1]));
	  bol2 = (extlis->first[k-1-ncol]  &&
		  (extlis->first[k+ncol]  || 
		   extlis->first[k+ncol+1]  || 
		   extlis->first[k+1]));
		  break;
	default:
	  bol1 = 0;
	  bol2 = 0;
		  break;
	}


	if((bol1 || bol2) && !(extlis->first[k])) {
	  /* Then we check maxima in both directions */
	  /* Horizontal direction */
	  k0 = 1;
	  bol1 =  (values_magnitude[k] > values_magnitude[k+k0]) ;
	  bol2 = bol1 && W2_test_maxima(-k0,values_magnitude,k,nrow,ncol);
	  bol3 = (values_magnitude[k] > values_magnitude[k-k0]) ;
	  bol4 = bol3 && W2_test_maxima(k0,values_magnitude,k,nrow,ncol);
	  is_max_point = bol2 || bol4 ;
	  if (is_max_point)  horizontal_flag = YES;
	  else horizontal_flag = NO;

	  /* Vertical direction */
	  k0 = ncol;
	  bol1 =  (values_magnitude[k] > values_magnitude[k+k0]) ;
	  bol2 = bol1 && W2_test_maxima(-k0,values_magnitude,k,nrow,ncol);
	  bol3 = (values_magnitude[k] > values_magnitude[k-k0]) ;
	  bol4 = bol3 && W2_test_maxima(k0,values_magnitude,k,nrow,ncol);
	  is_max_point = bol2 || bol4 ;
	  if (is_max_point) vertical_flag = YES;
	  else vertical_flag = NO;

	  if (horizontal_flag || vertical_flag) {
	    ext = NewExt2();
	    ext->scale = W2_iexp2(level);
	    ext->x = j;
	    ext->y = i;
	    ext->mag = mag;
	    ext->arg = argument;
	    extlis->first[k] = ext;
	    extlis->size++;
	  }
	}
      }
  }

 
}


extern void W2_corresponding_coarser_finer(EXTLIS2 fextlis,EXTLIS2 extlis);



static void W2_compute_point_repr(WTRANS2 wtrans,int orientation)
{
  int l;
  EXTREP2 extrep;
  
  extrep = wtrans->extrep;
  if (extrep->noct != 0) {
    W2_clear_point_repr(extrep);
}

  extrep->noct = wtrans->noct;
  CopyImage(wtrans->images[wtrans->noct][0], extrep->coarse);

  for(l = 1; l <= extrep->noct; l++)
     W2_compute_point(extrep->array[l],l,wtrans->images[l][MAGNITUDE],
					wtrans->images[l][ARGUMENT],
					orientation);
  /* Pb coarser */
  for (l =1; l< extrep-> noct;l++)
    W2_corresponding_coarser_finer(extrep->array[l], extrep->array[l+1]);

}


void C_Point2Compute(char ** argv)
     
{
  WTRANS2 wtrans=NULL;
  char car;
  int nb= 0;
  int l, orientation = YES;
  /* int num_level; */
/*   double fact; */
  int periodic=YES;

 
 argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);

 if (wtrans ==NULL) wtrans= GetWtrans2Cur();

 CheckWtrans2(wtrans);
  car =' ';
 
  wtrans->extrep->normalized= YES;
 
  while (car== ParseOption(&argv)) {
    switch (car) {
    case 'o':
      orientation = NO;
      break;
    case 'n':
      wtrans->extrep->normalized = NO;
      break;
    case 'N':
      periodic=NO;
      break;
    default:
      ErrorOption(car);
    } 
 }

  NoMoreArgs(argv);
 
 if (W2_mot_proc) {
    wtrans->extrep->normalized = NO; /* kludge to keep factor=1 for motion 10/20/92 */
    orientation = NO;
  }

  W2_compute_point_repr(wtrans,orientation);

  for (l = 1; l <= wtrans->extrep->noct; ++l)
    nb += wtrans->extrep->array[l]->size;
  if(wtrans->extrep->normalized) {
    for(l = 1; l <= wtrans->extrep->noct; l++)
	    W2_point_pic_normalize(wtrans,l);            
  } 
  
  SetResultInt(nb);

  wtrans->extrep->lipflag = NO;


}



