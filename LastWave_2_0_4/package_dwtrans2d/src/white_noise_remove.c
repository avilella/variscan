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

/*nclude "W2_view.h"*/
/*nclude "W2_signals.h"*/

int t_pos = -1;


static void W2_zero_point(EXT2 ext)
{
  EXT2 finger;

  finger = ext;
  while(finger) {
    finger->mag = 0.0;
    finger = finger->coarser;
  }
}

static void W2_alpha_K(EXT2 point1,EXT2 point2,float * slope,float * intercept)
{
  float slope1, intercept1, scale;
  double ord1, ord2;
  
  ord1 = fabs((double)point1->mag);
  ord2 = fabs((double)point2->mag);
/*  scale = (float) (my_log2((double)point2->scale) - my_log2((double)point1->scale));
  slope1 = (float) (my_log2(ord2) - my_log2(ord1))/ scale;
  intercept1 = my_log2(ord1) - slope1 * my_log2((double)point1->scale); */

  scale = (float) (W2_my_log2((double)point2->scale) - W2_my_log2((double)point1->scale));
  slope1 = (float) (W2_my_log2(ord2) - W2_my_log2(ord1))/ scale;
  intercept1 = W2_my_log2(ord1) - slope1 * W2_my_log2((double)point1->scale);

  *slope = slope1;
  *intercept = intercept1;
}


static void W2_lipschitz_white_noise_remove(WTRANS2 wtrans,float ampthresh,float alphathresh)
{
  EXTLIS2 extlis;
  EXTREP2 extrep;
  EXT2 ext, finger, *values;
 
  int i, j, k, nrow, ncol;
  float alpha, intercept;
  float ord[3];
  int first_big, sign_change;

  extrep = wtrans->extrep;

  extlis = extrep->array[1];
  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = extlis->first;
  for(i = 0; i< nrow; i++)
    for(j = 0; j < ncol; j++) {
      ext = values[i * ncol + j];
      if(ext && (ext->coarser == NULL))
	ext->mag = 0.0;
      
      if(ext && ext->coarser) {
	for(k = 0; k < 3; k++) 
	  ord[k] = 0.0;
	finger = ext;
	k = 0;
	while(finger&&( k < 3)) {
	  ord[k] = finger->mag;
	  finger = finger->coarser;
	  k = k + 1;
	}

	sign_change = NO;
	first_big = NO;

	if((k == 3) && (((ord[1]-ord[0]) * (ord[1]-ord[2])) < 0.0))
	  sign_change = YES;
	if(ord[0] > ord[1]) first_big = YES;
	
	if(sign_change && first_big) { /* alpha from scale 4 and 8 */
	  W2_alpha_K(ext->coarser, ext->coarser->coarser, &alpha, &intercept);
	  if(alpha < alphathresh) {
 	    ext->mag = 0.0; 
	    /*     zero_point(ext); */
	  }
	}
	
	if(sign_change && (first_big == NO)) { /* alpha from scale 2 and 4 */
	  W2_alpha_K(ext, ext->coarser, &alpha, &intercept);
	  if(alpha < alphathresh) {
 	    W2_zero_point(ext); 
	  }
	}
	 
	if((sign_change == NO) && (k==3)) { /* alpha from scale 4 and 8 */
	  W2_alpha_K(ext->coarser, ext->coarser->coarser, &alpha, &intercept);
	  if(alpha < alphathresh) {
	    ext->mag = 0.0;
	  }
	}

	if((sign_change == NO) && (k==2)) { /* alpha from scale 2 and 4 */
	  W2_alpha_K(ext, ext->coarser, &alpha, &intercept);
	  if(alpha < alphathresh) {
	    W2_zero_point(ext);
	  }
	}
      }
    }

  /* remove all the points with amplitude less than threshold and all the */
  /* points propagating to them  */

  extlis = extrep->array[3];
  values = extlis->first;

  for(i = 0 ; i < nrow; i++)
    for(j = 0; j < ncol; j++) 
      if(ext = values[i * ncol +j]) {
	if(ext->mag < ampthresh) {
	  finger = W2_walk_to_finest(ext);
	  while(finger) {
	    finger->mag = 0.0;
	    finger = finger->coarser;
	  }
	}
      }
}


/********************************************/
/* view the threshed chain image at a level */
/********************************************/

static void W2_chain_threshold_view(WTRANS2 wtrans,int level,FILE *fp,int fileflag)
{
  char car, str1[2000];
  int lng;
  float amp;
  int scale,nrow,ncol;
  int pos1, pos2, repeat;
  CHAINREP2 chainrep;
   int res;

  chainrep = wtrans->chainrep;

  scale = (int)(pow((double)2, (double)level));
  nrow = chainrep->array[level]->nrow;
  ncol = chainrep->array[level]->ncol;
  
  Printf("\nAt scale  %d :\n",scale);
  Printf("--------------\n");
  pos1 = -1;
  pos2 = -1;
  repeat = NO;

   while(1) {
       /*  if(repeat == NO) {
      if(IMAGE_window){
	if(pos1 >= 0) t_pos = pos1;
      else {
	IMAGE_sequential_display(nrow, ncol, &t_pos);
	pos1 = t_pos;
      }
	W2_chain_pic_view(chainrep->array[level],0,0,0.0,pos1,0,0,0.0,0.0); 
      }
      }*/
    Printf("\nWhat is the minimal length (in pixels) of the chains \n");
    Printf("that you want to keep? (int/X(=exit))(0) \n");

    if(fileflag) fscanf(fp,"%s",str1);
    else   GetLine(str1); 
     car=str1[0];
     res =sscanf(str1,"%d",&lng);
       if ((!res) || (car=='\0'))
         {  
            lng = 0;
            car=str1[0];
         }    
    if((car == 'X')) Errorf("Exit the denoise command \n");


    
    Printf("\nWhat is the minimal average amplitude of the chains \n");
    Printf("that you want to keep? (float/X(=exit))(0.0)\n");

    if(fileflag) fscanf(fp,"%s",str1);
    else    GetLine(str1); 
     car=str1[0];
     res =sscanf(str1,"%f",&amp);
       if ((!res) || (car=='\0'))
         {  
            amp = 0.0;
            car=str1[0];
         }    
    if((car == 'X')) Errorf("Exit the denoise command \n");
    
 
     /* if(IMAGE_window) {
      if(pos2 >= 0) t_pos = pos2;
      else {
	IMAGE_sequential_display(nrow, ncol, &t_pos);
	pos2 = t_pos;
      }
      W2_chain_pic_view(chainrep->array[level],0,lng,amp,pos2,0,0,0.0,0.0); 
      }*/

    Printf("\nIs the threshold O.K. to you? (y/n/X(=exit))(y) \n");
    if(fileflag) fscanf(fp,"%s",str1);
    else  GetLine(str1); 
        car=str1[0];
    if((car == 'X')) Errorf("Exit the denoise command \n");
    if((car == 'y') || (car == 'Y') || (car == '\0')) {
      W2_chain_repr_thresh(wtrans,lng,amp,level,YES,NO,NO);
      break;
    }
    repeat = YES;
  }
}

    

/**************************/
/* remove the white noise */
/**************************/

extern void W2_corresponding_coarser_finer(EXTLIS2 fextlis,EXTLIS2 extlis);

void C_lipschitz_noise_remove(char **argv)
{
  float alphathresh;
  int level,l;
  float ampthresh = 0.0001;
  char **str2, *filename;
  EXTREP2 extrep;
  int coarselevel, fileflag = NO;
  FILE *fp;
  char car;
  WTRANS2 wtrans;   
  char str1[2000];
  int res; 
  extern int W2_point_pic_remove_texture(EXTLIS2 extlis);


   argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
 
  t_pos = -1;

  while (car=ParseOption(&argv)) 
    switch (car) { 
    case 'f':
      argv = ParseArgv(argv,tSTR,&filename,-1);
      if (!(fp = FOpen(filename, "r"))) {
	Printf("Error: file %s does not exist!\n", filename);
	return;
      }
      fileflag = YES;
      break;
    default :
	ErrorOption(car);
      break;
    } 

  
  extrep = wtrans->extrep;
  if(wtrans->extrep->noct < 3) Errorf("Need at least three scales \n");

  Printf("Do you want to remove singularities based on ");
  Printf("their Lipschitz exponent? (y/n/X(=exit))(y)\n");
  if(fileflag) fscanf(fp,"%s/n",str1);
  else   GetLine(str1);
  car=str1[0];
  if((car == 'X')) Errorf("Exit the denoise command \n");

  if((car == 'y')||(car == 'Y')||(car =='\0')) {
    Printf("\nWhat is the smallest Lipschitz exponent of the singularities \n");
    Printf("do you want to keep? (float/X(=exit))(-0.3)\n");
    if(fileflag) fscanf(fp,"%s/n",str1);
    else  GetLine(str1); 
    car=str1[0];
     res =sscanf(str1,"%f",&alphathresh);
       if ((!res) || (car=='\0'))
         { alphathresh= -0.3; 
            car=str1[0];
         }
   
    Printf("alpha = %f",alphathresh);
    Flush();
   if((car == 'X')) Errorf("Exit the denoise command \n");
    
    if(wtrans->extrep->lipflag == NO )
      {
	W2_lipschitz_white_noise_remove(wtrans,ampthresh,alphathresh);
	for (l =1; l< extrep-> noct;l++)
	  W2_corresponding_coarser_finer(extrep->array[l], extrep->array[l+1]);
	ComputeChainrep2(wtrans,YES);
	wtrans->extrep->lipflag = YES;
      }
  }


  Printf("\nDo you want to thresh the maxima chains at any scale? (y/n/X(=exit)) (y) \n");
  
  if(fileflag) fscanf(fp,"%s",str1);
  else   GetLine(str1); 
    car=str1[0];
  if((car == 'X')) Errorf("Exit the denoise command \n");
  if((car == 'y')||(car == 'Y')||(car =='\0')) {
    for(level = extrep->noct; level >= 1; level--) 
      W2_chain_threshold_view(wtrans,level,fp,fileflag);
  }
  Printf("\nDo you want to remove edges that do not propagate ");
  Printf("along enough dyadic scale levels? (y/n/X(=exit))(y) \n");
  if(fileflag) fscanf(fp,"%s",str1);
  else  GetLine(str1); 
  car=str1[0];
  if((car == 'X')) Errorf("Exit the denoise command \n");

  if((car == 'y')||(car == 'Y')||(car =='\0'))  {
    Printf("\nOn how many scale levels should the edges propagate? (int/X(=exit))(3) \n");
    if(fileflag) fscanf(fp,"%s/n",str1);
    else  GetLine(str1); 
     car=str1[0];
     res =sscanf(str1,"%d",&coarselevel);
       if ((!res) || (car=='\0'))
         {  coarselevel = 3;
            car=str1[0];
         }    
    Printf("CoarseLevel = %d \n",coarselevel);
     for(level = coarselevel-1; level >= 1; level--)
       {
	 W2_point_pic_remove_texture(extrep->array[level]);
	 W2_corresponding_coarser_finer(extrep->array[level], extrep->array[level+1]);
	 ComputeChainrep2(wtrans,YES);
       }

  }
  Printf("\nDo you want to regularize the chain structure? (y/n/X(=exit))(y)\n");
  if(fileflag) fscanf(fp,"%s",str1);
  else   GetLine(str1); 
    car=str1[0];
  if((car == 'X')) Errorf("Exit the denoise command \n");
  if((car == 'y')||(car == 'Y')||(car =='\0')) 
    W2_denoise_regular(wtrans,fileflag,fp);
  

  Printf("\nDo you want to remove all the edges at some scale \n");
  Printf("and recover this edge information from a coarser scale? (y/n/X(=exit))(y) \n");

  if(fileflag) fscanf(fp,"%s",str1);
  else   GetLine(str1); 
    car=str1[0];
  if((car == 'X')) Errorf("Exit the denoise command \n");
  if((car == 'y')||(car == 'Y')||(car == '\0')) {

    while(1) {
      Printf("\nAt which level should the edges information be removed?(int/X(=exit))(1) \n");
     if(fileflag) fscanf(fp,"%s",str1);
      else     GetLine(str1); 
      car=str1[0];
      res =sscanf(str1,"%d",&level);
       if ((!res) || (car=='\0'))  
         {  level = 1;
	 }
     
      if((car == 'X')) Errorf("Exit the denoise command \n");
      
       
     
      if((level + 1) > extrep->noct) Errorf("Level too large \n");

      str2 = Malloc(sizeof(char **));
      *str2 = CharAlloc(8);
      strcpy(*str2," "); 
      sprintf(*str2,"%d \0",level+1);
      
      W2_chain_focus(wtrans,level+1,2,YES,NO,NO);
      Printf("Do you want to remove the edges at another level? (y/n/X(=exit))(n) \n");
     
      if(fileflag) fscanf(fp,"%s",str1);
      else     GetLine(str1); 
        car=str1[0];  
      if((car == 'X')) Errorf("Exit the denoise command \n");
      if((car != 'y') &&(car != 'Y')) break;
    }
  }
  Printf("\n");
  if(fileflag)   FClose(fp);
 /* for (l = 1 ; l <= wtrans->extrep->noct; l++)
   TestDebug2(wtrans->extrep->array[l],l,"sortie");*/
}


