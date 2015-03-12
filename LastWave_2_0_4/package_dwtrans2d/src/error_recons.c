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
#include "dwtrans2d.h"
  
#define W2_MAX_ITERATION 1000
float VARIANCE0;  /* signal variance of the input */
float VARIANCE[W2_MAX_LEVEL][W2_MAX_ORIENTATION]; 
float SNR[W2_MAX_ITERATION][W2_MAX_LEVEL]; /* num of iteration and level */

/* Compute the noise(the difference of the two images) variance     */
static float W2_noise_variance(IMAGE original, IMAGE reconstruct)
{
  int nrow, ncol;
  float *values_original;
  float *values_reconstruct;
  int i, j;
  float diff, sum;

  nrow = original->nrow;
  ncol = original->ncol;
  values_original = (float *)original->pixels;
  values_reconstruct = (float *)reconstruct->pixels;
  
  sum = 0.0;
  for (i = 0 ; i < nrow ; i++)
    for (j = 0; j < ncol ; j++) {
      diff = values_original[i*ncol+j] - values_reconstruct[i*ncol+j];
      sum += diff * diff;
    }
  return (sum/(nrow * ncol));
}


void W2_copy_HV_to(WTRANS2 wtrans, WTRANS2 to_wtrans)
{
  int level, orient;
  for (level = 1 ; level <=  wtrans->noct ; level++)
    for (orient = 0; orient < W2_MAX_ORIENTATION ; orient++) 
      if ((orient == HORIZONTAL) || (orient == VERTICAL))  
	CopyImage(wtrans->images[level][orient],
		   to_wtrans->images[level][orient]);
}

void W2_initialize_variances(WTRANS2 wtrans)
{

  
  int level, orient;

  if (wtrans->noct == 0) Errorf("Error during the initialization of SNR");

  GetNthMomentImage(wtrans->images[0][0], 2, &VARIANCE0,NO);

  for (level = 1; level <= wtrans->noct; level++)
    for ( orient = 0; orient < W2_MAX_ORIENTATION; orient++)
      if ((orient == HORIZONTAL) || (orient == VERTICAL)) {
        GetNthMomentImage(wtrans->images[level][orient], 2, &(VARIANCE[level][orient]),NO);
      }
}


void W2_write_SNR_error(char *filename,int iteration,int noct,int initial)
{
  int level, k, k0 , i;
  char file_string[50];
  FILE *fp;

  for(k=0; filename[k] != '\0'; ++k);
  k0 = k;
  for(k=0; k < k0 ; ++k)
    file_string[k] = filename[k];
  file_string[k0] = '.'; 

  for ( level = 0; level <= noct ; level++) {
    file_string[k0+1] = '0' + level/10;
    file_string[k0+2] = '0' + level % 10;
    file_string[k0+3] = '\0' ; 
  
    if(initial == YES)
      fp = FOpen(file_string,"w");
    else
      fp = FOpen(file_string,"a"); 

    for (i = 0; i < iteration; i++){
	fprintf(fp," %f",SNR[i][level]);
	fprintf(fp,"\n");
      } 
    FClose(fp);
  }
}


/* Compute the SNR of the variance between original and reconstruct */
void W2_SNR_variance(WTRANS2 wtrans_original, WTRANS2 wtrans_reconstruct,int iteration)
{
  int level;
  float noise_var, signal_var;
  

  for (level = 1; level <= wtrans_reconstruct->noct; level++) {
    noise_var=
      W2_noise_variance(wtrans_original->images[level][HORIZONTAL],
                     wtrans_reconstruct->images[level][HORIZONTAL])
    + W2_noise_variance(wtrans_original->images[level][VERTICAL],
                     wtrans_reconstruct->images[level][VERTICAL]);
    signal_var=
      VARIANCE[level][HORIZONTAL] + VARIANCE[level][VERTICAL];
    SNR[iteration][level]= 0.5 * log10((double)(noise_var/signal_var));
  }
  /* the SNR of the input */
  noise_var = W2_noise_variance(wtrans_original->images[0][0],
			     wtrans_reconstruct->images[0][0]);
  signal_var = VARIANCE0;
  SNR[iteration][0]= 0.5 * log10((double)(noise_var/signal_var));
}


