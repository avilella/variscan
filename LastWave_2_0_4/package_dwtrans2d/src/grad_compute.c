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

void TestDebug (IMAGE image,int n,char * s)
{ int i,j;
  float max=0.0;
   if (image!=NULL)
    if (image!=NULL)
     if (image->pixels !=NULL)
  for (i =0;i<image->nrow;i++)
      for (j =0;j<image->ncol;j++) {
  if ((image->pixels[i*image->ncol+j]>max))
      max = image->pixels[i*image->ncol+j];
      }
  Printf(" max = %f, %s",max,s);
  if (max >100000000000000.0) Printf("ouil \n");
}
void TestDebug2 (float val,int n)
{ 
  float max=0.0;
  
  if (val >100000000000000.0) 
    Printf("ouil :%d\n",n);
}
static void W2_pointgrid2wtrans(EXTLIS2 extlis,IMAGE hor_image,IMAGE ver_image)
{
  EXT2 *values, ext;
  int nrow, ncol;
  int i;
  float *hor_values;
  float *ver_values;


  nrow = extlis->nrow;
  ncol = extlis->ncol;
  values = (EXT2 *)extlis->first;

  hor_values = (float *)hor_image->pixels;
  ver_values = (float *)ver_image->pixels;

  for(i = 0; i < nrow * ncol ; i++) {
    ext = values[i];
    if(!ext) {
      hor_values[i] = 0;

      ver_values[i] =0;
    }
  }
}

static void W2_LP(WTRANS2 wtrans2)
{
  int j,noct;
  EXTREP2 extrep;
  
  CheckWtrans2(wtrans2);
  extrep = wtrans2->extrep;
  noct = extrep->noct;

  
  noct=extrep->noct;
  
  /* calcul de Lp */
  if (wtrans2->periodic)
    W2_periodyadic_decomp(wtrans2, 0, noct);
  else
    W2_dyadic_decomposition(wtrans2, 0, noct);
  wtrans2->noct = noct;
  wtrans2->norient = 4;

  for(j=1;j<=noct;j++)
    W2_pointgrid2wtrans(extrep->array[j],wtrans2->images[j][HORIZONTAL],
                     wtrans2->images[j][VERTICAL]);
  if (wtrans2->periodic)
    W2_periodyadic_recons(wtrans2, 0,wtrans2->noct);
  else
    W2_dyadic_reconstruction(wtrans2, 0,wtrans2->noct);
}


static void AddImage_mult(double lambda1,IMAGE  image1, double lambda2,IMAGE image2,IMAGE output)
{
   int i,j;
  float *output_values;
  float *image1_values = (float *)image1->pixels;
  float *image2_values = (float *)image2->pixels;
  int nrow = image1->nrow,ncol = image1->ncol;

 if ((nrow != image2->nrow) || (ncol != image2->ncol)) {
     Errorf ( "Size of images are not equal \n");
  }

  if (output == NULL) output = NewImage();
  if((output != image1) && (output != image2)) 
    SizeImage(output, nrow, ncol);

  output_values = (float *)output->pixels; 

  for(i=0; i < nrow; i++)
      for (j=0; j < ncol; j++)
          output_values[i * ncol + j] = (float)lambda1 * image1_values[i * ncol + j]
                                       + (float)lambda2 * image2_values[i * ncol + j];
}

void C_ComputeGrad(char ** argv)
{
  IMAGE r=NewImage(),p=NewImage(),pnew=NewImage();
  IMAGE pav=NewImage();
  IMAGE f=NewImage(),g=NewImage(),Lf=NewImage();
  IMAGE buf=NewImage(), buf2=NewImage();
  IMAGE Lp=NewImage(),Lpav=NewImage();
  IMAGE hor_image,vert_image;
  int i,j,noct,nrow,ncol,iter;
  double lambda, lam2, lam3,prod;
  float thresh_mag;
  EXTREP2 extrep;
  WTRANS2 wtrans2=NULL;
  int wantdenormalize=YES;
  char car;

  argv =ParseArgv(argv,tWTRANS2_,NULL,&wtrans2,tINT,&iter,tFLOAT_,0.,&thresh_mag,-1);
   if (wtrans2==NULL) wtrans2= GetWtrans2Cur();

   while (car = ParseOption(&argv)) {
    switch (car) {
    case 'n':
      wantdenormalize=NO;
      break;
    default :
      ErrorOption(car);
      break;
    }
   }
   NoMoreArgs(argv);

  if(wantdenormalize && wtrans2->extrep->normalized) {
    for(i = 1; i <= wtrans2->extrep->noct; i++)
   { W2_point_pic_denormalize(wtrans2,i);
   }
    wtrans2->extrep->normalized = NO;
  } 

  CheckWtrans2(wtrans2);
  extrep = wtrans2->extrep;
   
  noct=extrep->noct;
  nrow=extrep->array[1]->nrow;
  ncol=extrep->array[1]->ncol;


/* Initialisation de la boucle */
  SizeImage(pav,ncol,nrow);
  SizeImage(Lpav,ncol,nrow);
  SizeImage(f,ncol,nrow);
  ZeroImage(f);
  ZeroImage(pav);
  ZeroImage(Lpav);


  /* Seuillage des maximas residuels */
  for (j = 1; j <= noct; j++)
   { 
   W2_point_pic_thresh(extrep->array[j],thresh_mag); 
   }

  for(j=1;j<=noct;j++)
  { 
    hor_image=wtrans2->images[j][HORIZONTAL];
    vert_image=wtrans2->images[j][VERTICAL]; 
     
    W2_horvertpointpic2image(extrep->array[j],hor_image,vert_image);  
     
  }
   


  CopyImage(extrep->coarse,wtrans2->images[noct][0]); 
  if (wtrans2->periodic)
    W2_periodyadic_recons(wtrans2, 0, wtrans2->noct); 
  else
    W2_dyadic_reconstruction(wtrans2, 0, wtrans2->noct); 
  
  CopyImage(wtrans2->images[0][0],r);
  CopyImage(wtrans2->images[0][0],p);
/*  image_copy(wtrans2->images[0][0],g);*/
  for(i=0;i<iter;i++)
    {
      Printf("iteration %d\n",i);

      /* Calcul de la nouvelle transformee en ondelettes */
 
      CopyImage(p,wtrans2->images[0][0]); 
       /* calcul de Lp */
      W2_LP(wtrans2);    
      CopyImage(wtrans2->images[0][0],Lp);

      /* Calcul de lambda_n */
      prod=ImageScalarProduct(p,Lp);
      if (prod) {
	lambda= ImageScalarProduct(r,p)/prod;
	lam2=ImageScalarProduct(Lp,Lp)/prod;
      }
      else { break;}
       /* f_n+1=f_n+ lambda_n * p_n */
      AddImage_mult(1., f, lambda, p,f);
      /*      image_copy(f,wtrans2->images[0][0]); 
	      
	      W2_LP();    
	      image_copy(wtrans2->images[0][0],Lf);
	      image_sub(Lf,g,Lf);
	      Printf("Error in Lf=%g\n",sqrt((double)image_scalprod(Lf,Lf)));*/

     /* r_n+1=r_n - lambda_n * Lp_n */      
      AddImage_mult(1.,r, -lambda,Lp,r);

      /* p_n+1= Lp_n - (...) p_n - (...) p_n-1 */
      prod=ImageScalarProduct(pav,Lpav);
      if (!i) prod=1;
      if(prod)
	  lam3=ImageScalarProduct(Lp,Lpav)/prod;
      else
	 { CopyImage(f,wtrans2->images[0][0]);
	 break;
	 }

      CopyImage(p,buf);
      AddImage_mult(lam2,p,lam3,pav,buf2); 
      SubImage(Lp,buf2,pnew); 

     /* On initialise la marche suivante */
     CopyImage(p,pav);
     CopyImage(Lp,Lpav);
     CopyImage(pnew,p);
    } 
  if (iter)CopyImage(f,wtrans2->images[0][0]);
 
/*  if(iter) image_copy(Lf,wtrans2->images[0][1]);*/
}


