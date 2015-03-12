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
  


extern int flagnormalize;

int W2_mot_proc = NO;            /* to indicate the motion process */


/*--------------------------------------------------------------------------*/

static void W2_check_ddecomp(WTRANS2 wtrans)
{
  if (wtrans->noct == 0)
    Errorf("Run the wavelet transform first!\n");
}

/*--------------------------------------------------------------------------*/

static void W2_check_image(WTRANS2 wtrans)
{  
  if (wtrans->images[0][0]->sizeMalloc ==0)
    Errorf("W2_check_image() : There's no image to analyze\n");
   
 
}

static void W2_check_filterd(WTRANS2 wtrans)
{  
    if (wtrans->filterg1->size==0 ||wtrans->filterh1->size==0 || wtrans->filterk1->size==0)
       Errorf("There's no filter for the decomposition, use dwt2f <name> first"); 
 
}

void W2_check_filterr(WTRANS2 wtrans)
{  
    if (wtrans->filterg2->size==0 ||wtrans->filterh2->size==0 || wtrans->filterk2->size==0)
       Errorf("There's no filter for the reconstruction, use dwt2f <name> first"); 
 
}

/*--------------------------------------------------------------------------*/
/* void C_PerDecomp2(char ** argv) */
    
/* { */
/*   int noct; */
/*   WTRANS2 wtrans; */
/*   extern void W2_polar_repr(WTRANS2 wtrans,int begin_level,int num_level); */

/*   argv=  ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&noct,0); */

/*   if (wtrans ==NULL) wtrans= GetWtrans2Cur(); */
 
/*   W2_check_image(wtrans); */
/*   W2_check_filterd(wtrans); */
/*   W2_periodyadic_decomp(wtrans,0,noct); */
/*   W2_polar_repr(wtrans,1,noct);  */
/*   wtrans->noct = noct; */
/*   wtrans->norient = 4; */

/* } */

/*------------------------------------------------------------------------*/
/* void C_DDecomp2(char ** argv) */
/* { */
/*   int noct; */
/*   char car; */
/*   WTRANS2 wtrans; */
/*   extern void W2_polar_repr(WTRANS2 wtrans,int begin_level,int num_level); */


/*   argv=  ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&noct,-1); */

/*   if (wtrans ==NULL) wtrans= GetWtrans2Cur(); */

 /* Modified to add motion flag on 10/20/92 */


/*   while (car = ParseOption(&argv)) { */
/*     switch (car) {   */
/*     case 'm': */
/*       W2_mot_proc=YES; */
/*       break; */
/*     default : */
/*       ErrorOption(car); */
/*       break; */
/*     } */
/*    } */
/*    NoMoreArgs(argv); */
  
/*   W2_check_image(wtrans); */
/*   W2_check_filterd(wtrans); */
/*   W2_dyadic_decomposition(wtrans,0,noct); */
/*   W2_polar_repr(wtrans,1,noct);   */
/*   wtrans->noct = noct; */
/*   wtrans->norient = 4; */

/* } */

/* New R1*/

void C_DDecomp2(char ** argv)
{
  int periodic=YES;
  int noct;
  char car;
  WTRANS2 wtrans;
  extern void W2_polar_repr(WTRANS2 wtrans,int begin_level,int num_level);

  argv=  ParseArgv(argv,tWTRANS2_,NULL,&wtrans,tINT,&noct,-1);
  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
    
 /*  if (noct==-1)  */
/*     Errorf("Missing <noct>"); */

    /* Modified to add motion flag on 10/20/92 */
 
  while (car = ParseOption(&argv)) {
    switch (car) {
    case 'N':
      periodic=NO;
      break;
    case 'm':
      W2_mot_proc=YES;
      break;
    default :
      ErrorOption(car);
      break;
    }
   }
   NoMoreArgs(argv);
   
   W2_check_image(wtrans);

   SetDefaultFilter2(wtrans);  
  W2_check_filterd(wtrans);
  if (periodic)
    {
      W2_periodyadic_decomp(wtrans,0,noct);
      wtrans->periodic=YES;
    }
  else
    {
      W2_dyadic_decomposition(wtrans,0,noct);
      wtrans->periodic=NO;
    }
  W2_polar_repr(wtrans,1,noct);  
  wtrans->noct = noct;
  wtrans->norient = 4;

}


void C_DRecons2(char ** argv)
{
  int end_level;
  WTRANS2 wtrans;
  
  argv=  ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0);

  if (wtrans ==NULL) wtrans= GetWtrans2Cur();

/* Modified to add motion flag on 10/20/92 */

  NoMoreArgs(argv);
  W2_check_ddecomp (wtrans);
  W2_check_filterr(wtrans);
  end_level = 0;
  if (wtrans->periodic)
    W2_periodyadic_recons(wtrans, end_level, wtrans->noct);
  else
    {
      W2_border(wtrans);
      W2_dyadic_reconstruction(wtrans, end_level, wtrans->noct);
    }

}



/*--------------------------------------------------------------------------*/
/* void C_DRecons2(char ** argv) */
/* { */
/*   int end_level; */
/*   WTRANS2 wtrans; */

/*   argv=  ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0); */

/*   if (wtrans ==NULL) wtrans= GetWtrans2Cur(); */

/*   end_level = 0; */
  
/*   W2_check_ddecomp (wtrans); */
/*  W2_check_filterr(wtrans); */
/*   W2_border(wtrans); */

/*   W2_dyadic_reconstruction(wtrans, end_level, wtrans->noct); */
/* }   */

/*------------------------------------------------------------------------*/
/* void C_PerRecons2(char ** argv) */
/* { */
/*   int end_level; */
/*   WTRANS2 wtrans; */

/*   argv=  ParseArgv(argv,tWTRANS2_,NULL,&wtrans,0); */
  
/*   if (wtrans ==NULL) wtrans= GetWtrans2Cur(); */

/*   W2_check_ddecomp (wtrans); */
/*   W2_check_filterr(wtrans); */
/*   end_level = 0; */
/*   W2_periodyadic_recons(wtrans, end_level, wtrans->noct); */
/* } */

/*------------------------------------------------------------------------*/
void W2_periodyadic_decomp(WTRANS2 wtrans,int bgn_level,int num_level)
{
    IMAGE image_bsc, image_result;
    int l;
    int scale;
    double fact;
    int Normalisation =0;   /* Si 1 la norme L2 de l'ondelette =1 */
    
    /* Modified to include special decomposition for motion; 10/20/92 */

    for(l= 0,scale= 1<<bgn_level; l < num_level; l++,scale+= scale) {
	image_bsc = wtrans->images[l][0];
	image_result = wtrans->images[l+1][0];
       	W2_conv_hv_per_H1(image_bsc,image_result,
		wtrans->filterh1,scale,wtrans->filterh1,scale);

	image_result = wtrans->images[l+1][HORIZONTAL];
	W2_conv_hv_per(image_bsc,image_result,
		    wtrans->filterg1,scale,wtrans->filterk1,scale,Normalisation);

	image_result = wtrans->images[l+1][VERTICAL];
	W2_conv_hv_per(image_bsc,image_result,
		    wtrans->filterk1,scale,wtrans->filterg1,scale,Normalisation);
    }
       for(l= 1;l <= num_level; l++) {
	 if(l==1)
 	   fact = pow(2.,1./2.) / 1.8 ;
	 else 
	   fact = pow(2.,(double)(l)/ 2.);
	 /* fact=wtrans->filterg1->factors[l]; */
	 MulNumImage(wtrans->images[l][HORIZONTAL],(double) fact);
	 MulNumImage(wtrans->images[l][VERTICAL],(double) fact);
	 MulNumImage(wtrans->images[l][0],(double) fact);
       }
}

/*------------------------------------------------------------------------*/
void W2_dyadic_decomposition(WTRANS2 wtrans,int bgn_level,int num_level)
{
    IMAGE image_bsc, image_result;
    int l;
    int scale;
    int Normalisation =0;   /* Si 1 la norme L2 de l'ondelette =1 */
    
    /* Modified to include special decomposition for motion; 10/20/92 */

    for(l= 0,scale= 1<<bgn_level; l < num_level; l++,scale+= scale) {
	image_bsc = wtrans->images[l][0];

	image_result = wtrans->images[l+1][0];
	W2_conv_hv_H1(image_bsc,image_result,
		wtrans->filterh1,scale,wtrans->filterh1,scale);
        
	if (W2_mot_proc) {
	    image_result = wtrans->images[l+1][HORIZONTAL];

	    W2_conv_hv(image_bsc,image_result,
		    wtrans->filterg1,1,wtrans->filterk1,1,Normalisation);

	    image_result = wtrans->images[l+1][VERTICAL];
	    W2_conv_hv(image_bsc,image_result,      
		    wtrans->filterk1,1,wtrans->filterg1,1,Normalisation); 

	}
	else {
	    image_result = wtrans->images[l+1][HORIZONTAL];
	    W2_conv_hv(image_bsc,image_result,
		    wtrans->filterg1,scale,wtrans->filterk1,scale,Normalisation);

	    image_result = wtrans->images[l+1][VERTICAL];
	    W2_conv_hv(image_bsc,image_result,
		    wtrans->filterk1,scale,wtrans->filterg1,scale,Normalisation);
	}
    }
}
/*--------------------------------------------------------------------------*/
void W2_border(WTRANS2 wtrans)
{
  int l;

  wtrans->images[0][0]->border_hor=
  wtrans->images[0][0]->border_ver= W2_SYMEVN;

  wtrans->images[1][1]->border_hor=
  wtrans->images[1][2]->border_ver= W2_ASYODD;

  wtrans->images[1][2]->border_hor=
  wtrans->images[1][1]->border_ver= W2_SYMEVN;

  if(wtrans->filterh1->shift) {
    for(l= 1; l<wtrans->noct; l++) {

      wtrans->images[l][0]->border_hor=
      wtrans->images[l][0]->border_ver= W2_SYMODD;

      wtrans->images[l+1][1]->border_hor=
      wtrans->images[l+1][2]->border_ver= W2_ASYODD;

      wtrans->images[l+1][1]->border_ver=
      wtrans->images[l+1][2]->border_hor= W2_SYMODD;
    }
    wtrans->images[wtrans->noct][0]->border_hor=
    wtrans->images[wtrans->noct][0]->border_ver= W2_SYMODD;
  } else {
    for(l= 1; l<wtrans->noct; l++) {

      wtrans->images[l][0]->border_hor=
      wtrans->images[l][0]->border_ver= W2_SYMEVN;

      wtrans->images[l+1][1]->border_hor=
      wtrans->images[l+1][2]->border_ver= W2_ASYEVN;

      wtrans->images[l+1][1]->border_ver=
      wtrans->images[l+1][2]->border_hor= W2_SYMEVN;
    }
    wtrans->images[wtrans->noct][1]->border_hor=
    wtrans->images[wtrans->noct][2]->border_ver= W2_SYMEVN;
  }
}
/*--------------------------------------------------------------------------*/
void W2_dyadic_reconstruction(WTRANS2 wtrans,int  end_level, int num_level)
{

  IMAGE image_bsc, image_result,tmp_image;
  int l;
  int scale;
  int Normalisation=0;
 
  tmp_image = NewImage();
  for(l = 0, scale = 1; l < end_level + num_level - 1; l++, scale += scale);

  for(; l >= end_level; l--, scale /= 2) {
    
    image_result = wtrans->images[l][0];

    image_bsc = wtrans->images[l+1][0];
    W2_conv_hv_H2(image_bsc, image_result, 
	      wtrans->filterh2, scale, wtrans->filterh2, scale);

    image_bsc = wtrans->images[l+1][HORIZONTAL]; 
    W2_conv_hv_recons(image_bsc, tmp_image, 
	      wtrans->filterg2, scale, wtrans->filterk2, scale,Normalisation);
    AddImage(image_result, tmp_image, image_result);

    image_bsc = wtrans->images[l+1][VERTICAL]; 
    W2_conv_hv_recons(image_bsc, tmp_image, 
	      wtrans->filterk2, scale, wtrans->filterg2, scale,Normalisation);
    AddImage(image_result, tmp_image, image_result);
    
  }
  DeleteImage(tmp_image);
  
}


/*--------------------------------------------------------------------------*/
void CheckWtrans2(WTRANS2 wtrans)
{
  if (wtrans->noct == 0)
   Errorf("Run the wavelet transform first!\n");
}
/*--------------------------------------------------------------------------*/
void W2_periodyadic_recons(WTRANS2 wtrans,int  end_level, int num_level)
{

  IMAGE image_bsc, image_result,tmp_image;
  int l;
  int scale;
  double inv,fact;
  int Normalisation=0;
  int n=0;
  inv = 1. / pow(2.,(double)(num_level)/2.);
   
  tmp_image = NewImage();
  MulNumImage(wtrans->images[num_level][0],inv);
  
  
  for(l=num_level;l>0;l--) {
    if(l==1) 
      fact = pow(2.,1./2.) / 1.8 ;
    else 
      fact =(double)(pow(2.,(double)l/ 2.));
    /* fact= (wtrans->filterg1->factors[l-1])*(wtrans->filterg2->factors[l-1])/fact; */
    fact=1/fact;
    MulNumImage(wtrans->images[l][HORIZONTAL],(double)fact);
    MulNumImage(wtrans->images[l][VERTICAL],(double) fact);
    /* MulNumImage(wtrans->images[l][0],(double) fact); */
  }
  
  for(l = 0, scale = 1; l < end_level + num_level - 1; l++, scale += scale,n++);

  for(; l >= end_level; l--, scale /= 2) { 
    image_result = wtrans->images[l][0];
    image_bsc = wtrans->images[l+1][0];
    
    W2_conv_hv_per_H2(image_bsc, image_result, 
	      wtrans->filterh2, scale, wtrans->filterh2, scale);
  
    image_bsc = wtrans->images[l+1][HORIZONTAL]; 
    W2_conv_hv_per_recons(image_bsc, tmp_image, 
	      wtrans->filterg2, scale, wtrans->filterk2, scale,Normalisation);
  
   AddImage(image_result, tmp_image, image_result);
    
    image_bsc = wtrans->images[l+1][VERTICAL]; 
    W2_conv_hv_per_recons(image_bsc, tmp_image, 
	      wtrans->filterk2, scale, wtrans->filterg2, scale,Normalisation);
  
   AddImage(image_result, tmp_image, image_result);
 
  }  
  
  
       

  inv =  pow(2.,(double)(num_level)/2.);
  MulNumImage(wtrans->images[num_level][0],inv);
  
  for(l=num_level;l>0;l--) {
    if(l==1) 
      fact = pow(2.,1./2.) / 1.8 ;
    else 
      fact =(double)(pow(2.,(double)l/ 2.));
    /* fact= fact/((wtrans->filterg1->factors[l-1])*(wtrans->filterg2->factors[l-1])); */
    MulNumImage(wtrans->images[l][HORIZONTAL],(double)fact);
    MulNumImage(wtrans->images[l][VERTICAL],(double) fact);
    /* MulNumImage(wtrans->images[l][0],(double) fact); */
  }
  




  DeleteImage(tmp_image);
}
/*--------------------------------------------------------------------------*/

