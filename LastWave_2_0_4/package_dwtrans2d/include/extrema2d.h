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


#ifndef EXTREMA2D_H
#define EXTREMA2D_H

#include "dwtrans2d.h"

/**************************************/
/* structure of edge ext            */
/**************************************/

typedef struct ext2 {
  int scale;
  int x, y; /* abscissa of ext */
  float mag,arg; /* polar representation of gradient vector  */
  float hor,ver; /* cartesian representation of the gradient vector */
  struct ext2 *previous, *next; 
  struct ext2 *coarser, *finer; 
  struct chain2 *chain; /* the chain where the ext resides */
  int chain_index;
  } Ext2, *EXT2;

typedef struct extlis2 {
  int size; /* number of ext in extlis */
  int nrow, ncol;
  EXT2 *first;
} Extlist2, *EXTLIS2;


typedef struct extrep2 {
  int lipflag;
  int normalized;
  int noct;
  EXTLIS2 array[W2_MAX_LEVEL];
  IMAGE coarse; /* the coarser image */
} Extrep2, *EXTREP2; 



/**************************************/
/* structure of chain                 */
/**************************************/
typedef struct chain2 {
  int size;                /* number of ext in the chain */
  float mag; /* the statistics average of ext magnitude in chain */
  float var; /* the variance of ext magnitude in chain */
  EXT2 first; /* the first ext of chain */
  EXT2 last; /* the last ext of chain */
  struct chain2 *previous,*next;
} Chain2, *CHAIN2;

typedef struct ChainLis2 {
  int size;                /* number of chain in the chainlis */
  int nrow, ncol;
  struct chain2 *first;
} ChainLis2, *CHAINLIS2;

typedef struct chainrep2 {
  int noct;
  CHAINLIS2 array[W2_MAX_LEVEL];
} ChainRep2,*CHAINREP2; 


/***********************************************************/
/*******************    denoise      ***********************/
/***********************************************************/

/* denoise_regular.c */
extern void    W2_denoise_regular(WTRANS2 wtrans,int fileflag,FILE *fp);
extern EXT2 W2_walk_to_finest(EXT2 ext);
extern void    W2_rotate_to_same_sign(EXT2 point1,EXT2 point2, int *rotateflag, float * rotate_theta);


/***********************************************************/
/*******************     chain       ***********************/
/***********************************************************/
/*chain_alloc.c*/
extern CHAIN2 NewChain2(void); /* alloc a new_chain */
extern CHAINREP2 NewChainrep2(void); /* alloc a new_chain_repr */
extern void DeleteChainLis2(CHAINLIS2 chainlis);
extern CHAINLIS2 NewChainLis2(void); /* alloc a new_chain_pic */
extern void DeleteChainrep2(CHAINREP2 chainrep);
extern void ClearChainrep2(WTRANS2 wtrans);

/*chain_args.c*/
extern void W2_chain_pic_arg(WTRANS2 wtrans,int level, int endmode);
extern void W2_chain_pic_predict_arg(char **argv);
extern void W2_chainpicsmootharg(WTRANS2 wtrans2,int level);

/*chain_compute.c*/
extern void ComputeChainrep2(WTRANS2 wtrans, int uniform);

/*chain_delete.c*/
extern void RemoveChain2FromChainLis2(CHAIN2 chain,CHAINLIS2 chainlis);
extern void DeleteChain2Prop(WTRANS2 wtrans,CHAIN2 chain,int level);
extern void DeleteChain2( EXTLIS2 extlis, CHAINLIS2 chainlis, CHAIN2 chain);

/*chain_direction.c*/
extern void W2_chain_order_reverse(EXT2 point1,EXT2  point2, int *nextflag);
extern void W2_chainning_direction(CHAINLIS2 chainlis);

/*chain_focus.c*/
extern void W2_chain_focus (WTRANS2 wtrans, int level, int buffersize,  int newpicflag, int ratioflag, int positionflag);

/*chain_functions.c*/
extern EXT2 W2_last_finer_coarser_is_point(EXT2 ext);
extern EXT2 W2_first_coarser(EXT2 ext);
extern EXT2 W2_first_finer_coarser_is_point(EXT2 ext);
extern void W2_chain_append( CHAIN2 chain1,CHAIN2 chain2);
extern EXT2 W2_last_point(CHAIN2 chain);
extern void W2_chain_garbage(CHAIN2 chain);

/*chain_garbage.c*/
extern void W2_remove_point_in_chain_pic( EXTLIS2 extlis,CHAINLIS2 chainlis);
extern void W2_collect_point_in_point_pic(EXTLIS2 extlis, CHAINLIS2 chainlis);
extern void W2_chain_pic_first_point_copy(CHAINLIS2 input, CHAINLIS2 output, EXTLIS2 poutput);

/*chain_insert.c*/
extern void W2_chain_pic_interpolation(EXTLIS2 extlis,CHAINLIS2 chainlis);
extern void W2_chain_pic_interpolation(EXTLIS2 extlis,CHAINLIS2 chainlis);
extern void W2_insertafter(EXT2 previousext, EXT2 iext);
extern void W2_insert_point_between(CHAIN2 chain,EXT2 ext,EXT2 nextp, EXTLIS2 extlis,int insert_after);

/*chain_piece.c*/
extern void W2_partition_cross_scale_chain(EXTLIS2 extlis,CHAINLIS2 chainlis);
extern void W2_merge_cross_scale_chain(CHAINLIS2 chainlis);
extern void W2_zigzag_off(EXTLIS2 finer_point_pic,CHAINLIS2 finer_chain_pic,EXTLIS2 extlis, CHAINLIS2 chainlis);

/*chain_smooth.c*/
extern void W2_chainpicbluramp(WTRANS2 wtrans2,int level, float sigma);
extern void W2_chainpicblurabs(char **argv);

/*chain_split.c*/
extern void W2_chain_split(CHAIN2 chain, EXT2 ext, CHAIN2 newchain);
extern void W2_chain_split_2( CHAIN2 chain, EXT2 ext, CHAIN2 newchain);

/*chain_statistics.c*/
extern void W2_update_chain_pic(CHAINLIS2 chainlis);
extern void W2_update_chain( CHAIN2 chain,int chain_index);
extern void W2_chain_repr_thresh(WTRANS2 wtrans,int thresh_size,float thresh_mag, int level,int orflag,int propflag, int hascoarser);

/* ext_compute.c */
void W2_direction(float argument,int * dir,int *dir1,int *dir2);

/***********************************************************/
/*******************    ext        ***********************/
/***********************************************************/

/*point_alloc.c*/
extern EXT2 NewExt2(void);
extern EXTLIS2 NewExtLis2(void);
extern EXTREP2 NewExtrep2(void);
extern void DeleteExtrep2( EXTREP2 extrep);
extern void W2_point_pic_alloc(EXTLIS2 extlis, int nrow, int ncol);
extern void W2_change_point_pic(EXTLIS2 extlis, int nrow,  int ncol);
extern void W2_check_point_repr(EXTREP2 extrep);

/*point_compute.c*/
extern void  W2_compute_point(EXTLIS2 W2_point_pic, int level,IMAGE image_magnitude,IMAGE image_argument, int orientation);

/*point_copy.c*/

extern void W2_copy_point(EXT2 input,EXT2 output);
extern void W2_remove_point_pic_link(EXTLIS2 extlis);
extern void W2_point_pic_copy(EXTLIS2 input, EXTLIS2 output);

/* point_delete.c */
extern void W2_delete_a_point(EXT2 ext,EXTLIS2 extlis);
extern void W2_free_point(EXT2 ext);
extern void W2_remove_point_from_point_pic(EXT2 ext,EXTLIS2 extlis);
extern void W2_remove_point_from_chain(EXT2 ext,CHAIN2 chain);
extern void W2_delete_point_pic( EXTLIS2 extlis);
extern void W2_clear_point_pic(EXTLIS2 extlis);
extern void W2_clear_point_repr (EXTREP2 extrep);

/* point_io.c */
extern void W2_horvertpointpic2image(EXTLIS2 extlis,IMAGE hor_image,IMAGE vert_image);

/* point_normalize.c */
extern void W2_point_pic_normalize(WTRANS2 wtrans, int k);
extern void W2_point_pic_denormalize(WTRANS2 wtrans, int k);

/* point_proj.c */
extern W2_test_maxima(int k0,float * image_m,int k,int nrow, int ncol);
extern void W2_neighborbox(int boxsize, int row, int col,int ncol, int  nrow, int* xbox0, int* xbox1, int* ybox0, int* ybox1);     
extern void W2_point_repr_projection(WTRANS2 wtrans,int clipping);

/* point_relocation.c */
extern void W2_point_pic_relocate(EXTLIS2 extlis, CHAINLIS2 chainlis);

/* point_thresh.c */
extern void W2_point_pic_thresh(EXTLIS2 extlis, float thresh);

/*point_view.c */
extern void W2_viewimage(IMAGE image, char * str1);
extern void W2_view_image(IMAGE original_image,int position,int flag_zoom,int flag_mouse,int flag_resc,float Min,float Max);

/* point_print.c */
void W2_point_print(EXT2 ext);  

/* error_recons.c */
extern void W2_copy_HV_to(WTRANS2 wtrans,WTRANS2  to_wtrans);
extern void W2_initialize_variances( WTRANS2 wtrans);
extern void W2_write_SNR_error(char *filename,int iteration,int noct,int initial);
extern void W2_SNR_variance( WTRANS2 wtrans_original, WTRANS2 wtrans_reconstruct, int iteration);


/***********************************************************/
/*******************  miscellaneous   **********************/
/***********************************************************/
/* coordinate.c */
extern float W2_argument(float x,float y);
extern float W2_horizontal(float mag,float arg);
extern float W2_vertical(float mag,float arg);
extern void W2_polar_coordinate(WTRANS2 wtrans, int level);
extern void W2_polar_repr(WTRANS2 wtrans, int begin_level,int num_level);
extern void W2_cartesian_coordinate(WTRANS2 wtrans, int level);
extern void W2_point_repr_cartesian(EXTREP2 extrep);


#endif
