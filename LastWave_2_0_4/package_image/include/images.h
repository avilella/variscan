/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0.1                      */
/*                                                                          */
/*      Copyright (C) 1998-2003 Emmanuel Bacry, Jerome Fraleu.              */
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




#ifndef IMAGES_H

#define IMAGES_H



#define W2_MAX_LEVEL 12 /* max number of octaves */



extern char *imageType,*imageiType;


/* structure for picture */
typedef struct image {
    
    ValueFields;
    
    float	*pixels;     /* The image itself.                 */
    int	 nrow;        /* Number of rows. Typically 512.    */
    int	 ncol;        /* Number of columns. Typically 512. */

    int	 sizeMalloc;		/* Size of the image	*/
    int border_hor, border_ver;
    char *name;

} Image, *IMAGE;


extern int tIMAGE, tIMAGE_;
extern int tIMAGEI, tIMAGEI_;

/* types of border */
#define  W2_SYMEVN 0
#define  W2_SYMODD 1
#define  W2_ASYEVN 2
#define  W2_ASYODD 3

/* oonstants */
/* default size of the image */
#define	 W2_IMSIZE_D	512	

/* maximum value of pixel */
#define  W2_PIXMAX	1.7E38		

 /* rw- pour l'utilisateur, r-- pour les autres */
#define  W2_PROTECT_NORMAL 0644 



/***********************************************************/
/*******************     image       ***********************/
/***********************************************************/
/*image_alloc.c */
extern IMAGE GetImageVariableLevel(LEVEL level,char *name);
extern IMAGE GetImageVariable(char *name);
extern void SetImageVariableLevel(LEVEL level,char *name,IMAGE image);
extern void SetImageVariable(char *name,IMAGE image);
extern IMAGE  NewImage(void);
extern IMAGE TNewImage(void);
extern void  SizeImage(IMAGE image,int ncol,int nrow);
extern  void DeleteImage(IMAGE image);
extern  void ClearImage( IMAGE image);
extern IMAGE CopyImage( IMAGE input,IMAGE output);
extern void CopyFieldsImage(IMAGE input,IMAGE  output);

extern char ParseImageLevel_(LEVEL level, char *arg, IMAGE defVal, IMAGE *imag);
extern char ParseImage_(char *arg, IMAGE defVal, IMAGE *imag);
extern void ParseImageLevel(LEVEL level,char *arg, IMAGE *imag);
extern void ParseImage(char *arg, IMAGE *imag);
extern char ParseImageILevel_(LEVEL level, char *arg, IMAGE defVal, IMAGE *imag);
extern char ParseImageI_(char *arg, IMAGE defVal, IMAGE *imag);
extern void ParseImageILevel(LEVEL level,char *arg, IMAGE *imag);
extern void ParseImageI(char *arg, IMAGE *imag);

extern void *SetImageField(IMAGE im,void **arg);
extern void *GetImageExtractField(IMAGE im, void **arg);



/*image_create.c */

extern void RectangleImage(IMAGE image,int nrow,int ncol);
extern void CircleImage(IMAGE image,int nrow, int ncol);

/*image_functions10.c */

extern void ZeroImage(IMAGE input);
extern void AddImage(IMAGE image1,IMAGE image, IMAGE output);
extern void SubImage(IMAGE image1,IMAGE image, IMAGE output);
extern void MulImage(IMAGE image1,IMAGE image, IMAGE output);
extern void DivImage(IMAGE image1,IMAGE image, IMAGE output);
extern void AddNumImage(IMAGE output, float num);
extern void SubNumImage(IMAGE output, double num);
extern void DivNumImage(IMAGE output, double num);
extern void MulNumImage(IMAGE output, double num);
extern void TranspImage( IMAGE input,IMAGE output);
extern void PowerImage(IMAGE input, IMAGE output, int n);
extern double ImageScalarProduct(IMAGE image1,IMAGE image2);
extern void ThreshImage(IMAGE input,IMAGE output,int flagint,int flagMin,float min,int flagMax,float max);
extern float GetNthMomentImage(IMAGE image, int n, float *pNthMoment,int flagCentered);
extern float GetAbsMomentImage(IMAGE image, float f1, float *pNthMoment, int flagCentered);
extern float GetLpNormImage(IMAGE signal, float p);
extern void MinMaxImage(IMAGE ima,int *xmin,int *ymin, float *pmin,int *xmax,int *ymax,float *pmax);

/* file_io.c */
extern void ReadImageStream(IMAGE image ,STREAM stream, char flagHeader,int nrow, int ncol,char flagChar);
extern void WriteImageStream(IMAGE image,STREAM s,char flagChar,float min, float max,char flagHeader);


/* In int_expr.c */
extern int EvalIExpressionLevel(LEVEL level,char* arg, IMAGE *iresult);
extern int EvalIExpression(char* arg, IMAGE *iresult);

/* In image_graph.c */
extern void DefineGraphImage(void);


#endif
