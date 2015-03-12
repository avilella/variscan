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
  
/* #include "W2_motion.h" */

extern int W2_mot_proc;

#define W2_mod(a,d) (((a) % (d) + (d)) % (d))

/*float renorm_L2[]={180.312229,47.143756,20.226928,9.761818,4.839074,2.414366,1.206533,0.603151,0.193646,0.000562};*/

float W2_renorm_L2[]={0.707107,0.184977,0.0793213,0.0382816,0.0189768,0.00946813,0.00473154,0.00151927,0.00000882701};

float W2_renorm_L2p[]={0.395285,0.5,0.3125,0.41833,0.330719,0.475985,0.376299,0.460977,0.52451,0.272431,0.309978,0.331402,0.377077,0.413706,0.470725,0.707107,0.560164};








float W2_convbf[W2_SIG_SIZE];

/*---------------------------------------------------------------------*/
double W2_my_log2(double x)
{
  double y;
  y= log(x)/ log(2.0);
  return(y);
} 



/*---------------------------------------------------------------------*/
static void W2_symevn(int n,float * v,float * u)
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-1; vp >= v; vp--, up++)
    *up = *vp;
}
/*---------------------------------------------------------------------*/
static void W2_asyevn(int n,float *v,float *u)
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-1; vp >= v; vp--, up++)
    *up = -(*vp);
}
/*---------------------------------------------------------------------*/
static void  W2_symodd(int n,float *v,float *u)
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-2; vp > v; vp--, up++)
    *up = *vp;
}
/*---------------------------------------------------------------------*/
static void  W2_asyodd(int n,float *v,float *u)
{
  float *vp, *up, *vn = v+n;
  for(vp = v, up = u; vp < vn; vp++, up++)
    *up = *vp;
  for(vp = vn-2; vp > v; vp--, up++)
    *up = -(*vp);
}

/*---------------------------------------------------------------------*/

static void W2_ref(int border,int n,float *v,float *u)
{
  switch(border) {
    case W2_SYMEVN:
n-= n%2;
      W2_symevn(n,v,u);
      break;
    case W2_SYMODD:
n+= (1-n%2);
      W2_symodd(n,v,u);
      break;
    case W2_ASYEVN:
n-= n%2;
      W2_asyevn(n,v,u);
      break;
    case W2_ASYODD:
n+= (1-n%2);
      W2_asyodd(n,v,u);
      break;
  }
}
/*---------------------------------------------------------------------*/
static void W2_convper(IMAGE input,IMAGE output, FILTER2 filt,int scale)
{
    int ncol = input->ncol, 
	nrow = input->nrow;
    float *image_input;
    float *image_output;
    int filtsize = filt->size;
    int filtshift = filt->shift;
    float filtsym = filt->symmetry;
    float *filter = filt->values;
    int i, I, ind, j, k, left, right, l1, r1;
    float sum,v=sqrt(2);
    int ncol2= 2*(ncol-ncol%2);
    int octave;
    
    octave = (int) W2_my_log2((double)scale);


    if(scale == 1) {
	switch(filtshift) {
	case 1:
	    l1= 1;
	    r1= 0;
	    break;
	case -1:
	    l1= 0;
	    r1= 1;
	    break;
	case 0:
	    l1= r1= 1;
	    break;
	}
    } else
	if(filtshift) {
	    l1= r1= scale/2;
	} else {
	    l1= r1= scale;
	}

     
    SizeImage(output,nrow, ncol);
    image_input = (float *)input->pixels;
    image_output = (float *)output->pixels;

      
    for(i= 0, I= 0; i < nrow; i++, I+= ncol) {
	for(j= 0; j < ncol; j++) {
	    ind=I+j;
	    sum= filter[0] * image_input[ind]; 
	    for(k= 1, left= j-l1, right= j+r1;
		k < filtsize ;
		k++, right+= scale, left-= scale) {
		sum+= filter[k] * (filtsym*image_input[I+W2_mod(left,ncol)]
                                   + image_input[I+W2_mod(right,ncol)]); 
	    }
	    image_output[ind]= sum;
	}
    }
}




/*---------------------------------------------------------------------*/
void W2_conv_hv_per_H1(IMAGE input,IMAGE output,FILTER2 h_filt,int  h_scale,FILTER2 v_filt,int v_scale)
  
{ IMAGE wrk_image;
  wrk_image = NewImage();
  W2_convper(input,wrk_image,h_filt,h_scale);
  TranspImage(wrk_image, output);
  W2_convper(output,wrk_image,v_filt,v_scale);
  TranspImage(wrk_image, output);
  DeleteImage(wrk_image);
}

/*---------------------------------------------------------------------*/
static void W2_convper_norm(IMAGE input,IMAGE output, FILTER2 filt,int  scale)
{
    int ncol = input->ncol, 
	nrow = input->nrow;
    float *image_input;
    float *image_output;
    int filtsize = filt->size;
    int filtshift = filt->shift;
    float filtsym = filt->symmetry;
    float *filter = filt->values;
    int i, I, ind, j, k, left, right, l1, r1;
    float sum,v=sqrt(2);
    int ncol2= 2*(ncol-ncol%2);
    int octave;

    octave = (int) W2_my_log2((double)scale);


    if(scale == 1) {
	switch(filtshift) {
	case 1:
	    l1= 1;
	    r1= 0;
	    break;
	case -1:
	    l1= 0;
	    r1= 1;
	    break;
	case 0:
	    l1= r1= 1;
	    break;
	}
    } else
	if(filtshift) {
	    l1= r1= scale/2;
	} else {
	    l1= r1= scale;
	}


    SizeImage(output,nrow, ncol);
    image_input = (float *)input->pixels;
    image_output = (float *)output->pixels;


    for(i= 0, I= 0; i < nrow; i++, I+= ncol) {
	for(j= 0; j < ncol; j++) {
	    ind=I+j;
	    sum= filter[0] * image_input[ind];
	    for(k= 1, left= j-l1, right= j+r1;
		k < filtsize ;
		k++, right+= scale, left-= scale) {
		sum+= filter[k] * (filtsym*image_input[I+W2_mod(left,ncol)]
                                   + image_input[I+W2_mod(right,ncol)]);
	    }
	    image_output[ind]= sum/sqrt(W2_renorm_L2[octave]);
	}
    }
}

/*---------------------------------------------------------------------*/
void W2_conv_hv_per(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale,int norm_L2)
{   IMAGE wrk_image;
wrk_image = NewImage();
    if(norm_L2)
	W2_convper_norm(input,wrk_image,h_filt,h_scale);
    else
	W2_convper(input,wrk_image,h_filt,h_scale);

    TranspImage(wrk_image, output);

    if(norm_L2)
	W2_convper_norm(output,wrk_image,v_filt,v_scale);
    else
	W2_convper(output,wrk_image,v_filt,v_scale);

    TranspImage(wrk_image, output);
    DeleteImage(wrk_image);
}

/*---------------------------------------------------------------------*/
static void W2_conv_norm(IMAGE input,IMAGE output,FILTER2 filt,int  scale)
{
    int ncol = input->ncol, 
	nrow = input->nrow;
    int border= input->border_hor;
    float *image_input;
    float *image_output;
    int filtsize = filt->size;
    int filtshift = filt->shift;
    float filtsym = filt->symmetry;
    float *filter = filt->values;
    int i, I, Iout, j, k, left, right, l1, r1;
    float sum,v=sqrt(2);
    int ncol2= 2*(ncol-ncol%2);
    int ncol_out= ncol;
    int border_output= border;
    int octave;

    octave = (int) W2_my_log2((double)scale);

    if(scale == 1) {
      if (W2_mot_proc)  /* special border case for motion, 10/20/92 */ 
	    ncol_out+= (ncol_out%2 ? -1 : 1);
	switch(filtshift) {
	case 1:
	    l1= 1;
	    r1= 0;
	    if(filtsym < 0.0)
		border_output= W2_ASYODD;
	    else
		border_output= W2_SYMODD;
	    break;
	case -1:
	    l1= 0;
	    r1= 1;
	    if(filtsym < 0.0) {
/* need REWRITE Zhong */
		border_output= border + (border>1 ? -1-2*(border%2) : 3-2*(border%2));
	    } else {
		border_output= W2_SYMEVN;
	    }
	    break;
	case 0:
	    l1= r1= 1;
	    border_output= border;
	    break;
	}
    } else {
	if(filtshift) {
	    l1= r1= scale/2;
	} else {
	    l1= r1= scale;
	}
	if(filtsym < 0.0) {
	    border_output= border + (border>1 ? -2 : 2);
	} else {
	    border_output= border;
	}
    }

    SizeImage(output,nrow, ncol_out);
    output->border_hor= border_output;
    output->border_ver= input->border_ver;
    image_input = (float *)input->pixels;
    image_output = (float *)output->pixels;

    for(i= 0, I= 0, Iout= 0; i < nrow; i++, I+= ncol, Iout+= ncol_out) {
	W2_ref(border,ncol,image_input+I,W2_convbf);
	for(j= 0; j < ncol_out; j++) {
	    sum= filter[0] * W2_convbf[j];
	    for(k= 1, left= j-l1, right= j+r1;
		k < filtsize ;
		k++, right+= scale, left-= scale) {
		sum+= filter[k] * (filtsym*W2_convbf[W2_mod(left,ncol2)]
                                   + W2_convbf[W2_mod(right,ncol2)]);
	    }
	    image_output[Iout+j]= sum/sqrt(W2_renorm_L2[octave]);
	}
    }
}



/*---------------------------------------------------------------------*/
static void W2_conv(IMAGE input,IMAGE output,FILTER2 filt,int  scale)
{
  int ncol = input->ncol,
      nrow = input->nrow;
  int border= input->border_hor;
  float *image_input;
  float *image_output;
  int filtsize = filt->size;
  int filtshift = filt->shift;
  float filtsym = filt->symmetry;
  float *filter = filt->values;
  int i, I, Iout, j, k, left, right, l1, r1;
  float sum;
  int ncol2= 2*(ncol-ncol%2);
  int ncol_out= ncol;
  int border_output= border;

  if(scale == 1) {
  if (W2_mot_proc)  /* special border case for motion, 10/20/92 */
      ncol_out+= (ncol_out%2 ? -1 : 1);
    switch(filtshift) {
     case 1:
      l1= 1;
      r1= 0;
      if(filtsym < 0.0)
        border_output= W2_ASYODD;
      else
        border_output= W2_SYMODD;
      break;
     case -1:
      l1= 0;
      r1= 1;
      if(filtsym < 0.0) {
/* need REWRITE Zhong */
        border_output= border + (border>1 ? -1-2*(border%2) : 3-2*(border%2));
      } else {
        border_output= W2_SYMEVN;
      }
      break;
     case 0:
      l1= r1= 1;
      border_output= border;
      break;
    }
  } else {
    if(filtshift) {
      l1= r1= scale/2;
    } else {
      l1= r1= scale;
    }
    if(filtsym < 0.0) {
      border_output= border + (border>1 ? -2 : 2);
    } else {
      border_output= border;
    }
  }

  SizeImage(output,nrow, ncol_out);
  output->border_hor= border_output;
  output->border_ver= input->border_ver;
  image_input = (float *)input->pixels;
  image_output = (float *)output->pixels;

    for(i= 0, I= 0, Iout= 0; i < nrow; i++, I+= ncol, Iout+= ncol_out) {
      W2_ref(border,ncol,image_input+I,W2_convbf);
      for(j= 0; j < ncol_out; j++) {
        sum= filter[0] * W2_convbf[j];
        for(k= 1, left= j-l1, right= j+r1;
            k < filtsize ;
            k++, right+= scale, left-= scale) {
          sum+= filter[k] * (filtsym*W2_convbf[W2_mod(left,ncol2)]
                                   + W2_convbf[W2_mod(right,ncol2)]);
        }
        image_output[Iout+j]= sum;
      }
    }
}
/*---------------------------------------------------------------------*/
void W2_conv_hv(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale,int norm_L2)
{   IMAGE wrk_image;
wrk_image = NewImage();
    if(norm_L2)
	W2_conv_norm(input,wrk_image,h_filt,h_scale);
    else
	W2_conv(input,wrk_image,h_filt,h_scale);

    TranspImage(wrk_image, output);

    if(norm_L2)
	W2_conv_norm(output,wrk_image,v_filt,v_scale);
    else
	W2_conv(output,wrk_image,v_filt,v_scale);

    TranspImage(wrk_image, output);
    DeleteImage(wrk_image);
}





/*---------------------------------------------------------------------*/

void W2_conv_hv_H1(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale)
{ IMAGE wrk_image;
  wrk_image = NewImage();
  W2_conv(input,wrk_image,h_filt,h_scale);
  TranspImage(wrk_image, output);
  W2_conv(output,wrk_image,v_filt,v_scale);
  TranspImage(wrk_image, output);
  DeleteImage(wrk_image);
}

/*---------------------------------------------------------------------*/
void W2_conv_hv_H2(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale)
{ IMAGE wrk_image;
  
  wrk_image = NewImage();
  W2_conv(input,wrk_image,h_filt,h_scale);
  TranspImage(wrk_image, output);
  W2_conv(output,wrk_image,v_filt,v_scale);
  TranspImage(wrk_image, output);
  DeleteImage(wrk_image);
}

/*---------------------------------------------------------------------*/

static void W2_conv_normd(IMAGE input,IMAGE output, FILTER2 filt,int  scale)
{
    int ncol = input->ncol, 
	nrow = input->nrow;
    int border= input->border_hor;
    float *image_input;
    float *image_output;
    int filtsize = filt->size;
    int filtshift = filt->shift;
    float filtsym = filt->symmetry;
    float *filter = filt->values;
    int i, I, Iout, j, k, left, right, l1, r1;
    float sum;
    int ncol2= 2*(ncol-ncol%2);
    int ncol_out= ncol;
    int border_output= border;
    int octave;

    octave = (int) W2_my_log2((double)scale);

    if(scale == 1) {
	if (!W2_mot_proc) /* special border case for motion, 10/20/92 */
	    ncol_out+= (ncol_out%2 ? -1 : 1);
	switch(filtshift) {
	case 1:
	    l1= 1;
	    r1= 0;
	    if(filtsym < 0.0)
		border_output= W2_ASYODD;
	    else
		border_output= W2_SYMODD;
	    break;
	case -1:
	    l1= 0;
	    r1= 1;
	    if(filtsym < 0.0) {
/* need REWRITE Zhong */
		border_output= border + (border>1 ? -1-2*(border%2) : 3-2*(border%2));
	    } else {
		border_output= W2_SYMEVN;
	    }
	    break;
	case 0:
	    l1= r1= 1;
	    border_output= border;
	    break;
	}
    } else {
	if(filtshift) {
	    l1= r1= scale/2;
	} else {
	    l1= r1= scale;
	}
	if(filtsym < 0.0) {
	    border_output= border + (border>1 ? -2 : 2);
	} else {
	    border_output= border;
	}
    }

    SizeImage(output,nrow, ncol_out);
    output->border_hor= border_output;
    output->border_ver= input->border_ver;
    image_input = (float *)input->pixels;
    image_output = (float *)output->pixels;

    for(i= 0, I= 0, Iout= 0; i < nrow; i++, I+= ncol, Iout+= ncol_out) {
	W2_ref(border,ncol,image_input+I,W2_convbf);
	for(j= 0; j < ncol_out; j++) {
	    sum= filter[0] * W2_convbf[j];
	    for(k= 1, left= j-l1, right= j+r1;
		k < filtsize ;
		k++, right+= scale, left-= scale) {
		sum+= filter[k] * (filtsym*W2_convbf[W2_mod(left,ncol2)]
                                   + W2_convbf[W2_mod(right,ncol2)]);
	    }
	    image_output[Iout+j]= sum*sqrt(W2_renorm_L2[octave]);
	}
    }
}
/*---------------------------------------------------------------------*/

void W2_conv_hv_recons(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale,int norm_L2)
{   IMAGE wrk_image;
    wrk_image = NewImage();
    if(norm_L2)
	W2_conv_normd(input,wrk_image,h_filt,h_scale);
    else
	W2_conv(input,wrk_image,h_filt,h_scale);

    TranspImage(wrk_image, output);

    if(norm_L2)
	W2_conv_normd(output,wrk_image,v_filt,v_scale);
    else
	W2_conv(output,wrk_image,v_filt,v_scale);

    TranspImage(wrk_image, output);
    DeleteImage(wrk_image);
}

/*---------------------------------------------------------------------*/
void W2_convper_normd(IMAGE input,IMAGE output, FILTER2 filt,int  scale)
{
    int ncol = input->ncol, 
	nrow = input->nrow;
    float *image_input;
    float *image_output;
    int filtsize = filt->size;
    int filtshift = filt->shift;
    float filtsym = filt->symmetry;
    float *filter = filt->values;
    int i, I, ind, j, k, left, right, l1, r1;
    float sum,v=sqrt(2);
    int ncol2= 2*(ncol-ncol%2);
    int octave;

    octave = (int)W2_my_log2((double)scale);


    if(scale == 1) {
	switch(filtshift) {
	case 1:
	    l1= 1;
	    r1= 0;
	    break;
	case -1:
	    l1= 0;
	    r1= 1;
	    break;
	case 0:
	    l1= r1= 1;
	    break;
	}
    } else
	if(filtshift) {
	    l1= r1= scale/2;
	} else {
	    l1= r1= scale;
	}


    SizeImage(output,nrow, ncol);
    image_input = (float *)input->pixels;
    image_output = (float *)output->pixels;


    for(i= 0, I= 0; i < nrow; i++, I+= ncol) {
	for(j= 0; j < ncol; j++) {
	    ind=I+j;
	    sum= filter[0] * image_input[ind];
	    for(k= 1, left= j-l1, right= j+r1;
		k < filtsize ;
		k++, right+= scale, left-= scale) {
		sum+= filter[k] * (filtsym*image_input[I+W2_mod(left,ncol)]
                                   + image_input[I+W2_mod(right,ncol)]);
	    }
	    image_output[ind]= sum*sqrt(W2_renorm_L2[octave]);
	}
    }
}
/*---------------------------------------------------------------------*/

void W2_conv_hv_per_recons(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale,int norm_L2)
{   IMAGE wrk_image;
    wrk_image = NewImage();
    if(norm_L2)
	W2_convper_normd(input,wrk_image,h_filt,h_scale);
    else
	W2_convper(input,wrk_image,h_filt,h_scale);

    TranspImage(wrk_image, output);

    if(norm_L2)
	W2_convper_normd(output,wrk_image,v_filt,v_scale);
    else
	W2_convper(output,wrk_image,v_filt,v_scale);

    TranspImage(wrk_image, output);
     DeleteImage(wrk_image);
}
/*---------------------------------------------------------------------*/

void W2_conv_hv_per_H2(IMAGE input,IMAGE output,FILTER2 h_filt,int h_scale,FILTER2 v_filt,int v_scale)
{ IMAGE wrk_image;
  
  wrk_image = NewImage();
 
  W2_convper(input,wrk_image,h_filt,h_scale);
  
  TranspImage(wrk_image, output);

  W2_convper(output,wrk_image,v_filt,v_scale);
 
  TranspImage(wrk_image, output);

   DeleteImage(wrk_image);

}


/*---------------------------------------------------------------------*/
