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
#define W2_MAX_ITERATION 1000

/*****************************************************************/
/* Clear the images of a wtrans and set the values in HORIZONTAL */
/* and VERTICAL  to be zoro.                                     */
/*****************************************************************/

static void W2_wtrans_images_init(WTRANS2 wtrans)
{
  int nrow, ncol;
  EXTREP2 extrep;
  int l, orient;
  /*  IMAGE SizeImage();*/

  extrep = wtrans->extrep;
  nrow = extrep->coarse->nrow;
  ncol = extrep->coarse->ncol;

  SizeImage(wtrans->images[0][0], nrow, ncol);
  ZeroImage(wtrans->images[0][0]); 
  wtrans->noct = extrep->noct;

  for (l = 1; l <= wtrans->noct; l++) 
    for (orient = 0; orient <= ARGUMENT;orient++) {
      SizeImage(wtrans->images[l][orient], nrow, ncol);
      if (orient ==  HORIZONTAL || orient == VERTICAL)
	ZeroImage(wtrans->images[l][orient]);
    }
  SizeImage(wtrans->images[wtrans->noct][0], nrow, ncol);
  ZeroImage(wtrans->images[wtrans->noct][0]); 
/* ZHONG */
  W2_border(wtrans);
}
    



/******************************************************************/
/* error flag computes the distance between gamma space and the   */
/* original image at each level after each iteration              */
/******************************************************************/

static void W2_point_recons(WTRANS2 wtrans,int iteration,int initial,int error_flag,
	     char *error_file, int clipping)
{
  EXTREP2 extrep;
  int i;
  WTRANS2 W2_T2;
  extrep = wtrans->extrep;
  
    W2_T2=NewWtrans2();

  if (error_flag && initial) { /* copy signals(images) */
    CheckWtrans2(wtrans); /* do ddecomp first */
    CopyImage(wtrans->images[0][0],W2_T2->images[0][0]);
    W2_copy_HV_to(wtrans, W2_T2);
    W2_T2->noct = wtrans->noct;
  }
 
  if (error_flag) 
    W2_initialize_variances(W2_T2);

  W2_point_repr_cartesian(extrep);  /*compute cartesian of each ext */

  if(initial) {
    W2_wtrans_images_init(wtrans);
    W2_point_repr_projection(wtrans,clipping); 

    if (wtrans->periodic)
      W2_periodyadic_recons(wtrans, 0, wtrans->noct);
    else
      W2_dyadic_reconstruction(wtrans, 0, wtrans->noct);
  }

/*   if (error1_flag) copy_HV_to(wtrans, T2[2]); */

  Printf("iteration =% d\n",iteration);
  for (i = 0; i < iteration; i++) {
    if (wtrans->periodic)
      W2_periodyadic_decomp(wtrans, 0, wtrans->noct);
    else
      W2_dyadic_decomposition(wtrans, 0, wtrans->noct);
    

/*     if (error1_flag) compute_cost(wtrans, T2[2], i); */

    W2_point_repr_projection(wtrans,clipping);

/*     if (error1_flag) copy_HV_to(wtrans, T2[2]); */

     if (wtrans->periodic)
      W2_periodyadic_recons(wtrans, 0, wtrans->noct);
    else
      W2_dyadic_reconstruction(wtrans, 0, wtrans->noct);
    
    if (error_flag) W2_SNR_variance(W2_T2,wtrans, i); /* compute SNR */

    Printf("iteration = %d\n",iteration- i -1);
  }

  if (error_flag) 
    W2_write_SNR_error(error_file, iteration,wtrans->noct,initial);

/* write as subroutin in the future *//*  if (error1_flag) {  
    fp1 = fopen("wave2.err","w");
    for (i = 0; i < iteration; i++){
      for ( level = 1; level <= wtrans->noct ; level++)
	fprintf(fp1," %12g",square_error[i][level]);
	fprintf(fp1,"\n");
    }
    fclose(fp1);
    } */

     DeleteWtrans2(W2_T2);  
}

void C_Point2Recons(char **argv)
{
  WTRANS2 wtrans = NULL;
  int iteration, initialization = YES, clipping= YES;
  int error_flag = NO /* , error1_flag= NO */;
  char error_file[50];
  int wantdenormalize= YES;
  char car;
  int k;
  
  sprintf(error_file, "SNR");
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&iteration,-1);
  
if (wtrans ==NULL)   wtrans= GetWtrans2Cur();
 
  W2_check_point_repr(wtrans->extrep);
  if (wtrans->noct == 0) {
    wtrans->noct = wtrans->extrep->noct;
    wtrans->norient = 4; 
  }

  while (car=ParseOption(&argv)) {
    switch (car) {
    case 'i' :
      initialization = NO;
      break;
    case 'c' :
      clipping = NO;
      break;
    case 'e' :
      argv = ParseArgv(argv,tSTR,&error_file,0);
      error_flag = YES;
      break;
    case 'n':
      wantdenormalize = NO;
      break;
      /*    case 'd' :
      error1_flag = YES;
      T2[2]->noct = wtrans->noct;
      break; */
    default :
      ErrorOption(car);
    }
  }

  NoMoreArgs(argv);

  CheckWtrans2(wtrans);
  W2_check_filterr(wtrans);
  if(wantdenormalize && wtrans->extrep->normalized) {
    for(k = 1; k <= wtrans->extrep->noct; k++)
   { W2_point_pic_denormalize(wtrans,k);
   }
    wtrans->extrep->normalized = NO;
  } 

  W2_point_recons(wtrans,iteration,initialization,error_flag,
	       error_file,clipping);
 

}


