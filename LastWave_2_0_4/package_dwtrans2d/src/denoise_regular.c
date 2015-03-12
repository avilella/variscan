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
  

/*******************************************/
/* regularize the denoised chain structure */
/*******************************************/

void W2_denoise_regular(WTRANS2 wtrans,int fileflag,FILE *fp)
{
  int  level, i;
  int sigma;
  char str1[2000],car;
  EXTREP2 extrep;
  CHAINREP2 chainrep;
  int res;
  extrep = wtrans->extrep;
  chainrep = wtrans->chainrep;
  


 Printf("\nOn how many pixels do you want to smooth the chain features? (int/X(=exit))(6) \n"); 
  if(fileflag) fscanf(fp,"%s",str1);
  else GetLine(str1);
  

     car=str1[0];
     res =sscanf(str1,"%d",&sigma);
       if ((!res) || (car=='\0'))
	   {  sigma = 6; }  

  if((car == 'X')) Errorf("Exit the denoise command \n");
  
    for(level = 2; level >= 1; level--) {
      /* extlis = extrep->array[level]; */
/*       chainlis = chainrep->array[level]; */
      
     /*  On supprime tout */

      /* W2_partition_cross_scale_chain(extlis, chainlis); */
/*       W2_merge_cross_scale_chain(chainrep->array[level]); */
/*       W2_zigzag_off(extlis, chainlis,extrep->array[level+1], */
/* 		    chainrep->array[level+1]); */
      
      /*     sprintf(args1,"%d %f",level,(float)sigma);
    chainpicblurabs(args1);*/
    
   
        W2_chain_pic_arg(wtrans, level, NO);
      
	
     for(i = 0 ; i < MIN(6,sigma); i++) {
       W2_chainpicsmootharg( wtrans,level);
      }

     
     for(i = 0; i < 2; i++)
     W2_chainpicbluramp(wtrans,level,(float)sigma);
     }
 
strcpy(str1," ");
    sprintf(str1,"%d %f",3,(float)(sigma/2.0));;  
    W2_chainpicbluramp(wtrans,3,(float)(sigma/2.0));

    
  
} 
