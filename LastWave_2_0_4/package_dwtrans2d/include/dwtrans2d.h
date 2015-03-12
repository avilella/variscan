

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




#ifndef DWTRANS2D_H

#define DWTRANS2D_H



/**********************/
/* Some constants ... */
/**********************/

#define W2_MAX_LEVEL 12 /* max number of octaves */
#define W2_MAX_ORIENTATION 10  /* max number of orientations */
#define W2_NUM_WTRANS2 4      /* max number of wavelet transform */
#define W2_NFACT W2_MAX_LEVEL     /* should be the same as MAX_LEVEL */


/**********************/
/* Some includes ... */
/**********************/

#include "images.h"
#include "filter2d.h"





/**********************/
/* Some constants ... */
/**********************/



#define HORIZONTAL      1     /* d/dx */
#define VERTICAL        2     /* d/dy */
#define MAGNITUDE       3     /* magnitude */
#define ARGUMENT        4     /* gradient */
#define W2_SIG_SIZE 66000


 


/************************************/
/*       wtransform structure       */
/************************************/


typedef struct wtrans2 {
 
  ValueFields;
  
  int num; /* index of the transform */
  FILTER2 filterh1; /* ptr to the H1 filter for decomp */
  FILTER2 filterg1; /* ptr to the G1 filter for decomp */
  FILTER2 filterk1; /* ptr to the K1 filter for decomp */
  FILTER2 filterh2; /* ptr to the H2 filter for recons */
  FILTER2 filterg2; /* ptr to the G2 filter for recons */
  FILTER2 filterk2; /* ptr to the K2 filter for recons */
  IMAGE images[W2_MAX_LEVEL][W2_MAX_ORIENTATION];
  int noct; /* total number of octaves  of decomposition */
  int norient; /* total number of orientations */
  struct extrep2 *extrep; /* ptr to the extrep */
  struct chainrep2 *chainrep; /* ptr to the chainrep */

  int periodic; /* periodized flag */
  char  * name;
  char * wName;
} Wtrans2, *WTRANS2;
 


extern char *dwtrans2Type;
extern int tWTRANS2, tWTRANS2_;

extern int W2_plot_var;
#define W2_BW 0
#define W2_CP 1






/***********************************************************/
/*******************    wtrans2      ***********************/
/***********************************************************/

/* wtrans2_alloc.c */
extern void DeleteWtrans2(WTRANS2 wtrans2);
extern WTRANS2 NewWtrans2(void);
extern WTRANS2 GetWtrans2VariableLevel(LEVEL level,char *name);
extern WTRANS2 GetWtrans2Variable(char *name);
extern void SetWtrans2VariableLevel(LEVEL level,char *name,WTRANS2 wtrans2);
extern void SetWtrans2Variable(char *name,WTRANS2 wtrans2);
extern char ParseWtrans2Level_(LEVEL level, char *arg, WTRANS2 defVal, WTRANS2 *wtrans2);
extern char ParseWtrans2_(char *arg, WTRANS2 defVal, WTRANS2 *wtrans2);
extern void ParseWtrans2Level(LEVEL level, char *arg, WTRANS2 *wtrans2);
extern void ParseWtrans2(char *arg, WTRANS2 *wtrans2);
extern WTRANS2 GetWtrans2Cur(void);
extern void SetDefaultFilter2(WTRANS2 wtrans2); 


/* wtrans2_IO.c */
extern void  ReadWtrans2(WTRANS2 wtrans, STREAM s);
extern void  WriteWtrans2(WTRANS2 wtrans, STREAM s);

/*fast_convol.c */
extern double W2_my_log2(double x);
extern void W2_conv_hv_per_H1(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale);
extern void W2_conv_hv_per(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale, int norm_L2);
extern void W2_conv_hv(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale, int norm_L2);
extern void W2_conv_hv_H1(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale);
extern void W2_conv_hv_H2(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale);
extern void W2_conv_hv_recons(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale, int norm_L2);
extern void W2_conv_hv_per_recons(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale, int norm_L2);
extern void W2_conv_hv_per_H2(IMAGE input,IMAGE output, FILTER2 h_filt, int h_scale,FILTER2 v_filt, int v_scale);

/*fast_ddecomp.c */
extern void W2_periodyadic_decomp(WTRANS2 wtrans, int bgn_level, int num_level);
extern void W2_dyadic_decomposition( WTRANS2 wtrans, int bgn_level,int num_level);
extern void W2_dyadic_reconstruction( WTRANS2 wtrans, int end_level,int num_level);
extern void W2_check_filterr(WTRANS2 wtrans);
extern void CheckWtrans2(WTRANS2 wtrans);
extern void W2_border(WTRANS2 wtrans);
extern void W2_periodyadic_recons(WTRANS2 wtrans,int end_level,int num_level);




#endif





