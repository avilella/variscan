/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0.3                        */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
/*      emails : fraleu@cmap.polytechnique.fr                               */
/*               lastwave@cmap.polytechnique.fr                             */
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



/****************************************************************************/
/*                                                                          */
/*  image_matrix.c   Functions which deal with matrices                     */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "signals.h"
#include "images.h"

/*
 * Useful structures
 */
 
/* Floating matrix structure */
typedef struct fmatrix {
 float **Y;
 int size;
 int sizeMalloc;
} *FMATRIX;

/* Floating Vector structure */
typedef struct fvector {
  float *Y;
  int   size;
} *FVECTOR;


/* Integer Vector structure */
typedef struct ivector {
 int *Y;
 int size;
} *IVECTOR;

/*
 * (Des)Allocation
 */
 
/* Creation (and Allocation) of a New square FMATRIX of size size */
FMATRIX NewFMatrix(int size)
{
  FMATRIX cm;
  int i;

  cm = (FMATRIX) Malloc(sizeof(struct fmatrix));
  cm->size = size;
  cm->sizeMalloc = size;

  cm->Y = (float **) Malloc(sizeof(float *)*size) -1;  
  for(i=1;i<=size;i++) cm->Y[i] = FloatAlloc(size)-1;
  
  return(cm);
}

/* Delete a FMATRIX Structure */
void DeleteFMatrix(FMATRIX cm)
{
  int i;
  
  if (cm) {
    for(i=1;i<=cm->sizeMalloc;i++) Free(cm->Y[i]+1);
	Free(cm->Y+1);
  }

  Free(cm);
}

FMATRIX Image2FMatrix(IMAGE im)
{
  FMATRIX m;
  int i,k;

  if (im->nrow != im->ncol) Errorf("Image2FMatrix() : Not square image");
  
  m = NewFMatrix(im->nrow);
  
  for (k=0,i=1;i<=im->nrow;i++) {
    memcpy(m->Y[i]+1,im->pixels+k,im->ncol*sizeof(float));
    k+=im->ncol;
  }
  
  return(m);
}

void FMatrix2Image(FMATRIX m, IMAGE im)
{
  int i,k;
  
  SizeImage(im,m->size,m->size);
  
  for (k=0,i=1;i<=im->nrow;i++) {
    memcpy(im->pixels+k,m->Y[i]+1,im->ncol*sizeof(float));
    k+=im->ncol;
  }
}

/* Creation (and Allocation) of a New IVECTOR of size size */
IVECTOR NewIvector(int size)
{
  IVECTOR ov;
  
  ov = (IVECTOR) Malloc(sizeof(struct ivector));
   
  ov->size = size;
  ov->Y = IntAlloc(size)-1;

  return(ov);
}

/* Delete a IVECTOR Structure */
void DeleteIVector(IVECTOR ov)
{
  if (ov)
    {
      Free(ov->Y+1);
      Free(ov);
    }
}

/* Creation (and Allocation) of a New FVECTOR of size size */
FVECTOR NewFVector(int size)
{
  FVECTOR ov;
  
  ov = (FVECTOR) Malloc(sizeof(struct fvector));
   
  ov->size = size;
  ov->Y = FloatAlloc(size)-1;

  return(ov);
}

void FVector2Signal(FVECTOR v, SIGNAL s)
{
  SizeSignal(s,v->size,YSIG);
  memcpy(s->Y,v->Y+1,v->size*sizeof(float));
}

/* Delete a FVECTOR Structure */
void DeleteFVector(FVECTOR ov)
{
  if (ov)
    {
      Free(ov->Y+1);
      Free(ov);
    }
}


/*
 * Math routines 
 */

/* Matrix Inversion using LU decomposition */
#define TINY 1.0e-20;
static void ludcmp(FMATRIX a,IVECTOR indx,float *d)
{
  
  int n = a->size;
  int i,imax,j,k;
  float big,dum,sum,temp;
  FVECTOR vv;

  vv=NewFVector(n);
  *d=1.0;
  for (i=1;i<=n;i++) {
    big=0.0;
    for (j=1;j<=n;j++)
      if ((temp=fabs(a->Y[i][j])) > big) big=temp;
    if (big == 0.0) Errorf("ludcmp(): Singular matrix");
    vv->Y[i]=1.0/big;
  }
  for (j=1;j<=n;j++) {
    for (i=1;i<j;i++) {
      sum=a->Y[i][j];
      for (k=1;k<i;k++) sum -= a->Y[i][k]*a->Y[k][j];
      a->Y[i][j]=sum;
    }
    big=0.0;
    for (i=j;i<=n;i++) {
      sum=a->Y[i][j];
      for (k=1;k<j;k++)
	sum -= a->Y[i][k]*a->Y[k][j];
      a->Y[i][j]=sum;
      if ( (dum=vv->Y[i]*fabs(sum)) >= big) {
	big=dum;
	imax=i;
      }
    }
    if (j != imax) {
      for (k=1;k<=n;k++) {
	dum=a->Y[imax][k];
	a->Y[imax][k]=a->Y[j][k];
	a->Y[j][k]=dum;
      }
      *d = -(*d);
      vv->Y[imax]=vv->Y[j];
    }
    indx->Y[j]=imax;
    if (a->Y[j][j] == 0.0) a->Y[j][j]=TINY;
    if (j != n) {
      dum=1.0/(a->Y[j][j]);
      for (i=j+1;i<=n;i++) a->Y[i][j] *= dum;
    }
  }
  DeleteFVector(vv);
}
#undef TINY

static void lubksb(FMATRIX a,IVECTOR indx,FVECTOR b)
{
  int n = a->size;
  int i,ii=0,ip,j;
  float sum;
  
  for (i=1;i<=n;i++) {
    ip=indx->Y[i];
    sum=b->Y[ip];
    b->Y[ip]=b->Y[i];
    if (ii)
      for (j=ii;j<=i-1;j++) sum -= a->Y[i][j]*b->Y[j];
    else if (sum) ii=i;
    b->Y[i]=sum;
  }
  for (i=n;i>=1;i--) {
    sum=b->Y[i];
    for (j=i+1;j<=n;j++) sum -= a->Y[i][j]*b->Y[j];
    b->Y[i]=sum/a->Y[i][i];
  }
}
 
void InverseFMatrix(FMATRIX m) 
{
  int n = m->size;
  float d;
  IVECTOR indx;
  FVECTOR col;
  FMATRIX res;
  int i,j;

  indx = NewIvector(n);
  col = NewFVector(n);
  res = NewFMatrix(n);
  
  ludcmp(m,indx,&d);
  for (j=1;j<=n;j++) {
    for (i=1;i<=n;i++) col->Y[i] = 0.0;
    col->Y[j] = 1.0;
    lubksb(m,indx,col);
    for(i=1;i<=n;i++) res->Y[i][j] = col->Y[i];
  }
  
  for (j=1;j<=n;j++) 
    for (i=1;i<=n;i++) m->Y[i][j] = res->Y[i][j];
  
  DeleteIVector(indx);
  DeleteFVector(col);
  DeleteFMatrix(res);
}

void InverseImage(IMAGE i,IMAGE j)
{
  FMATRIX m;
  
  m = Image2FMatrix(i);
  InverseFMatrix(m);
  FMatrix2Image(m,j);
  DeleteFMatrix(m);
}


/* compute the determinant of a FMATRIX */
static float DetFMatrix(FMATRIX m)
{
  
  int n = m->size;
  float d;
  IVECTOR index= NewIvector(n);
  int i;

  ludcmp(m,index,&d);
  for(i=1;i<=n;i++) d *= m->Y[i][i];
  DeleteIVector(index);
  return(d);
}


/* Matrix inversion using Cholesky Decomposition */
/* The applies only for positive definite symmetric matrices */


static void choldc(FMATRIX a, FVECTOR p)
{
  int n = a->size;
  int i,j,k;
  float sum;
  
  for(i=1;i<=n;i++){
    for(j=i;j<=n;j++){
      for(sum = a->Y[i][j],k=i-1;k>=1;k--) 
	sum -= a->Y[i][k]*a->Y[j][k];
      if (i==j){
	if (sum <= 0.0)
	  Errorf("choldc(): Non positive definite matrix");
	p->Y[i] = sqrt(sum);
      }
      else a->Y[i][j] = sum/p->Y[i];
    }
  }
}
	

static void cholsl(FMATRIX a, FVECTOR p, FVECTOR b, FVECTOR x)
{
  int n = a->size;
  int i,k;
  float sum;

  for(i=1;i<=n;i++){
    for(sum=b->Y[i], k=i-1;k>=1;k--) sum -= a->Y[i][k]*x->Y[k];
    x->Y[i]=sum/p->Y[i];
  }
  for(i=n;i>=1;i--){
    for(sum=x->Y[i],k=i+1;k<=n;k++) sum -= a->Y[k][i]*x->Y[k];
    x->Y[i] = sum/p->Y[i];
  }
}



static void InversePositiveFMatrix(FMATRIX m) 
{
  int n = m->size;
  FVECTOR col, out, diag;
  FMATRIX res;
  int i,j;

  diag = NewFVector(n);
  col = NewFVector(n);
  out = NewFVector(n);
  res = NewFMatrix(n);
  
  choldc(m,diag);
  for (j=1;j<=n;j++) {
    for (i=1;i<=n;i++) col->Y[i] = 0.0;
    col->Y[j] = 1.0;
    cholsl(m,diag,col,out);
    for(i=1;i<=n;i++) res->Y[i][j] = out->Y[i];
  }
  
  for (j=1;j<=n;j++) 
    for (i=1;i<=n;i++) m->Y[i][j] = res->Y[i][j];
  
  DeleteFVector(diag);
  DeleteFVector(col);
  DeleteFVector(out);
  DeleteFMatrix(res);
}

/*
 * Transpose an image
 */
 
void TranspImage(IMAGE input, IMAGE output)
{
  int i, j, I, J;
  int nrow, ncol;
  float *input_image;
  float *output_image;
  IMAGE wrk_image;
  
  nrow = input->nrow;
  ncol = input->ncol;

  
  if(input == output) {
    wrk_image = NewImage();
    CopyImage(input, wrk_image);
    input = wrk_image;
  }
  else wrk_image = NULL;
  
  SizeImage(output,nrow,ncol);
  input_image = input->pixels;
  output_image = output->pixels;

  for (i= 0, I= 0; i < nrow ; i++)
    for (j= 0 , J= 0; j < ncol; j++ , J+= nrow, I++)
      output_image[J+i] = input_image[I];

  output->border_hor= input->border_ver;
  output->border_ver= input->border_hor;
  if (wrk_image) DeleteImage(wrk_image);
}


/*
 * Take a matrix the power n
 */
void PowerImage(IMAGE input, IMAGE output, int n)
{
  IMAGE tempIm,in,out;
  int i,j,k,l;
  char flagTempIn,flagTempOut;
  
  if (input->ncol != input->nrow) Errorf("PowerImage() : Matrix should be square");
  
  if (n == 1) {
    if (input == output) return;
    CopyImage(input,output);
    return;
  }
  if (n == -1) {
    InverseImage(input,output);
    return;
  }

  SizeImage(output,input->ncol,input->nrow);
  
  if (n == 0) {
    for (k=0,i=0;i<input->nrow;i++) {
      for(j=0;j<input->ncol;j++) {
        output->pixels[k++] = i==j;
      }
    }
    return;
  }
  
  flagTempIn = flagTempOut = NO;
  in = input;
  out = output;
  if (n < 0) {
    in = NewImage();
    flagTempIn = YES;
    SizeImage(in,input->ncol,input->nrow);
    InverseImage(input,in);
    n = -n;
  }
  
  if (in==out) {
    in = input;
    out = NewImage();
    flagTempOut = YES;
    SizeImage(out,input->ncol,input->nrow);
  }
        
  tempIm = NewImage();
  SizeImage(tempIm,input->ncol,input->nrow);
  CopyImage(in,out);
  n--;
  
  while(n>0) {

    for (k=0,i=0;i<in->nrow;i++) {
      for(j=0;j<in->ncol;j++) {
        tempIm->pixels[k] = 0;
        for (l=0;l<in->nrow;l++) tempIm->pixels[k] += in->pixels[i*in->ncol+l]*out->pixels[l*in->ncol+j];
        k++;
      }
    }
    
    CopyImage(tempIm,out);
    n--;
  }
  
  if (out != output) CopyImage(out,output);
  if (flagTempOut) DeleteImage(out);
  if (flagTempIn) DeleteImage(in);
  DeleteImage(tempIm);
}




static void tred2(FMATRIX a, FVECTOR d, FVECTOR e)
{
  int l,k,j,i;
  float scale,hh,h,g,f;
  int n = a->size;
  
  for (i=n;i>=2;i--) {
    l=i-1;
    h=scale=0.0;
    if (l > 1) {
      for (k=1;k<=l;k++) scale += fabs(a->Y[i][k]);
      if (scale == 0.0) e->Y[i]=a->Y[i][l];
      else {
        for (k=1;k<=l;k++) {
          a->Y[i][k] /= scale; 
          h += a->Y[i][k]*a->Y[i][k];
        }
        f=a->Y[i][l];
        g=(f >= 0.0 ? -sqrt(h) : sqrt(h));
        e->Y[i]=scale*g;
        h -= f*g; 
        a->Y[i][l]=f-g;
        f=0.0;
        for (j=1;j<=l;j++) {
          /* Next statement can be omitted if eigenvectors not wanted */
          a->Y[j][i]=a->Y[i][j]/h; 
          g=0.0; 
          for (k=1;k<=j;k++) g += a->Y[j][k]*a->Y[i][k];
          for (k=j+1;k<=l;k++) g += a->Y[k][j]*a->Y[i][k];
          e->Y[j]=g/h; 
          f += e->Y[j]*a->Y[i][j];
        }
        hh=f/(h+h); 
        for (j=1;j<=l;j++) {
          f=a->Y[i][j];
          e->Y[j]=g=e->Y[j]-hh*f;
          for (k=1;k<=j;k++) a->Y[j][k] -= (f*e->Y[k]+g*a->Y[i][k]);
        } 
      }
    } 
    else e->Y[i]=a->Y[i][l];
    d->Y[i]=h;
  }
  
  /* Next statement can be omitted if eigenvectors not wanted */
  d->Y[1]=0.0;
  e->Y[1]=0.0;
  
  /* Contents of this loop can be omitted if eigenvectors not
     wanted except for statement d[i]=a[i][i]; */
  for (i=1;i<=n;i++) {
    l=i-1;
    if (d->Y[i]) { 
      for (j=1;j<=l;j++) {
        g=0.0;
        for (k=1;k<=l;k++) g += a->Y[i][k]*a->Y[k][j];
        for (k=1;k<=l;k++) a->Y[k][j] -= g*a->Y[k][i];
      }
    }
    d->Y[i]=a->Y[i][i]; 
    a->Y[i][i]=1.0; 
    for (j=1;j<=l;j++) a->Y[j][i]=a->Y[i][j]=0.0;
  }
}


#define SQR(x) ((x) * (x)) 
static float pythag(float a, float b)
{
  float absa,absb;
  
  absa=fabs(a);
  absb=fabs(b);

  if (absa > absb) return absa*sqrt(1.0+SQR(absb/absa));
  else return (absb == 0.0 ? 0.0 : absb*sqrt(1.0+SQR(absa/absb)));
}
#undef SQR

#define SIGN1(a,b) ((b) > 0.0 ? fabs(a) : -fabs(a))
static void tqli(FVECTOR d, FVECTOR e, FMATRIX z)
{
  float pythag(float a, float b);
  int m,l,iter,i,k;
  float s,r,p,g,f,dd,c,b;
  int n = d->size;
  
  for (i=2;i<=n;i++) e->Y[i-1]=e->Y[i]; 
  e->Y[n]=0.0;
  
  for (l=1;l<=n;l++) {
    iter=0;
    do {
    for (m=l;m<=n-1;m++) {
      dd=fabs(d->Y[m])+fabs(d->Y[m+1]);
      if ((float)(fabs(e->Y[m])+dd) == dd) break;
    }
    if (m != l) {
      if (iter++ == 30) Errorf("Too many iterations in tqli");
      g=(d->Y[l+1]-d->Y[l])/(2.0*e->Y[l]); 
      r=pythag(g,1.0);
      g=d->Y[m]-d->Y[l]+e->Y[l]/(g+SIGN1(r,g)); 
      s=c=1.0;
      p=0.0;
      for (i=m-1;i>=l;i--) { 
        f=s*e->Y[i];
        b=c*e->Y[i];
        e->Y[i+1]=(r=pythag(f,g));
        if (r == 0.0) { 
          d->Y[i+1] -= p;
          e->Y[m]=0.0;
          break;
        }
        s=f/r;
        c=g/r;
        g=d->Y[i+1]-p;
        r=(d->Y[i]-g)*s+2.0*c*b;
        d->Y[i+1]=g+(p=s*r);
        g=c*r-b;
        /* Next loop can be omitted if eigenvectors not wanted*/
        for (k=1;k<=n;k++) {
          f=z->Y[k][i+1];
          z->Y[k][i+1]=s*z->Y[k][i]+c*f;
          z->Y[k][i]=c*z->Y[k][i]-s*f;
        }
      }
      if (r == 0.0 && i >= l) continue;
      d->Y[l] -= p;
      e->Y[l]=g;
      e->Y[m]=0.0;
    }
    } while (m != l);
  }
}
#undef SIGN1

void DiagonalizeImage(IMAGE i,IMAGE vectors, SIGNAL val)
{
  FMATRIX a;
  FVECTOR v,v1;

  a = Image2FMatrix(i);
  v = NewFVector(i->nrow);
  v1 = NewFVector(i->nrow);

  tred2(a,v,v1);
  tqli(v,v1,a);
  
  FMatrix2Image(a,vectors);
  FVector2Signal(v,val);

  DeleteFMatrix(a);
  DeleteFVector(v);
  DeleteFVector(v1);
}

void C_Matrix(char **argv)
{
  char *action;
  IMAGE im,im1;
  SIGNAL v;
  float f;
  int i;
  FMATRIX m;
  LISTV lv;
  
  argv = ParseArgv(argv,tWORD,&action,tIMAGE,&im,-1);

  if (!strcmp(action,"trace")) {
    if (im->nrow != im->ncol) Errorf("Image should be square");
    f = 0;
    for (i=0;i<im->nrow;i++) {
      f += im->pixels[i*im->ncol+i];
    }
    SetResultFloat(f);
    return;
  }

  if (!strcmp(action,"det")) {
    if (im->nrow != im->ncol) Errorf("Image should be square");
    m = Image2FMatrix(im);
    f = DetFMatrix(m);
    DeleteFMatrix(m);
    SetResultFloat(f);
    return;
  }

  if (!strcmp(action,"diagsym")) {
    if (im->nrow != im->ncol) Errorf("Image should be square");
    im1 = TNewImage();
    v = TNewSignal();
    DiagonalizeImage(im,im1,v);
    lv = TNewListv();
    AppendValue2Listv(lv,(VALUE) im1);
    AppendValue2Listv(lv,(VALUE) v);
    SetResultValue(lv);
    return;
  }
}
/* */

#define FREERETURN {DeleteFVector(h);DeleteFVector(g);return;}

void toeplz(FVECTOR r, SIGNAL filter, SIGNAL corr)
{ 
    int n = corr->size;
    int j,k,m,m1,m2;
    float pp,pt1,pt2,qq,qt1,qt2,sd,sgd,sgn,shn,sxn;
    FVECTOR g,h;

    SizeSignal(filter,n,YSIG);
    if (r->Y[n] == 0.0) Errorf("toeplz(): toeplz-1 singular ppl minor");
    g=NewFVector(n);
    h=NewFVector(n);
    filter->Y[0]=corr->Y[0]/r->Y[n];
    if (n == 1) FREERETURN
    g->Y[1]=r->Y[n-1]/r->Y[n];
    h->Y[1]=r->Y[n+1]/r->Y[n];
    for (m=1;m<=n;m++) {
	m1=m+1;
	sxn = -corr->Y[m1-1];
	sd =  -r->Y[n];
	for (j=1;j<=m;j++) {
	    sxn += r->Y[n+m1-j]*filter->Y[j-1];
	    sd += r->Y[n+m1-j]*g->Y[m-j+1];
	}
	if (sd == 0.0) Errorf("toeplz(): toeplz-2 singular ppl minor");
	filter->Y[m1-1]=sxn/sd;
	for (j=1;j<=m;j++)
	    filter->Y[j-1] -= filter->Y[m1-1]*g->Y[m-j+1];
	if (m1 == n) FREERETURN
	sgn = -r->Y[n-m1];
	shn = -r->Y[n+m1];
	sgd = -r->Y[n];
	for (j=1;j<=m;j++) {
	    sgn += r->Y[n+j-m1]*g->Y[j];
	    shn += r->Y[n+m1-j]*h->Y[j];
	    sgd += r->Y[n+j-m1]*h->Y[m-j+1];
	}
	if ((sd == 0.0) || (sgd == 0.0))
	    Errorf("toeplz(): toeplz-3 singular ppl minor");
	g->Y[m1]=sgn/sgd;
	h->Y[m1]=shn/sd;
	k=m;
	m2=(m+1) >> 1;
	pp=g->Y[m1];
	qq=h->Y[m1];
	for (j=1;j<=m2;j++) {
	    pt1=g->Y[j];
	    pt2=g->Y[k];
	    qt1=h->Y[j];
	    qt2=h->Y[k];
	    g->Y[j]=pt1-pp*qt2;
	    g->Y[k]=pt2-pp*qt1;
	    h->Y[j]=qt1-qq*pt2;
	    h->Y[k--]=qt2-qq*pt1;
	}
    }
    Errorf("Toeplz(): Should not arrive here!");
}/* */

