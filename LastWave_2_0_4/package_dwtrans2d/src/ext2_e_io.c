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

/********************************************************************/
/* The following writes POINT_EXPR to a file with BINARY code.       */
/********************************************************************/

static void W2_point_pic_write_BINARY(EXTLIS2 extlis,FILE * stream)
{
  int nrow = extlis->nrow;
  int ncol = extlis->ncol;
  int size = extlis->size;
  EXT2 *values;
  EXT2 ext;
  
  float * TAB, *HeadTab;
  int i,I,j;
  int res;

  values = (EXT2 *)extlis->first;

  fprintf(stream,"\n%d ",size);
  /* write the slots of the extlis */
  fprintf(stream,"%d ", nrow);
  fprintf(stream,"%d\n", ncol);
 
  TAB = FloatAlloc(size*4);
  HeadTab= TAB;
  res=0;
  for(i = 0,I = 0; i < nrow; i++,I += ncol)
    for(j = 0; j < ncol; j++) {
      if(ext = values[I+j]) {
	TAB[res++] = (float) ext->x; 
	TAB[res++] = (float) ext->y;
	TAB[res++] = ext->mag;
	TAB[res++] = ext->arg;	
      }
    }

  res= fwrite(HeadTab,size*4*sizeof(float),1,stream);
  
  Free(TAB);

}



static void PointReprWriteStream(EXTREP2 extrep,STREAM s)
{
  FILE *stream;
  int l;

  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("PointReprWriteStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("WriteWtrans2() : You cannot write a wtrans2 to standard input"); 
  stream = s->stream; 


  fprintf(stream,"NOCT (Point_Repr) : %d ", extrep->noct);
  fprintf(stream,"Normalized %d ", extrep->normalized);
  fprintf(stream,"Lipschitz %d ", extrep->lipflag);

  for(l = 1; l <= extrep->noct; l++) 
    W2_point_pic_write_BINARY(extrep->array[l],stream);
 
 fprintf(stream,"\n\n");
    WriteImageStream(extrep->coarse,s,NO,0,0,YES);
 
}


/****************************************************/
/* write ext directly without going through image */
/****************************************************/

void C_PointReprWrite(char **argv)
{
  WTRANS2 wtrans;
  char *fname;
  STREAM s;
  EXTREP2 extrep;
  int binary_flag = YES;

 argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);
  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
  argv= ParseArgv(argv,tSTREAM_,NULL,&s,-1);
 
  
  if (s == NULL) {
       argv = ParseArgv(argv,tSTR,&fname,0);
  
       s =  OpenFileStream(fname,"w");
  
       if (s == NULL) Errorf("Error while opening the file %s",fname);

        if (!s->stream) 
          Errorf("unable to open %s\n",fname);
       CheckWtrans2(wtrans); 
       extrep = wtrans->extrep;
       W2_check_point_repr(extrep);
       PointReprWriteStream(extrep,s);
  }
    else {  NoMoreArgs(argv);
            CheckWtrans2(wtrans);
            extrep = wtrans->extrep;
            W2_check_point_repr(extrep);
            PointReprWriteStream(extrep,s);
          
     }
  CloseStream(s);
}
 


/********************************************************************/
/* The following read a BINARY to point_rep.                        */
/********************************************************************/

static void W2_point_pic_e_read(EXTLIS2 extlis,FILE* stream)
{
  int nrow, ncol, size;
  int i;
  EXT2 *values;
  EXT2 ext;
  int res;

  float * TAB, *HeadTab;
  size=0;
  ncol=0;
  nrow=0;


   fscanf(stream,"\n%d ",&size); /* the order must agree with in write */
   fscanf(stream,"%d ",&nrow);
   fscanf(stream,"%d\n",&ncol);
 
  W2_change_point_pic(extlis, nrow, ncol);
  extlis->size = size;
  values = (EXT2 *)extlis->first;
 

    ext = (EXT2)Malloc(sizeof(struct ext2));
    TAB = FloatAlloc(size*4);
    HeadTab= TAB;

   
   res=fread (HeadTab, size*4*sizeof(float),1,stream  );
  

   
  for (i = 0; i <size; i++) { /* order agrees with write */
    ext =  (EXT2)Malloc(sizeof(struct ext2)); 
   
    ext->x = (int) TAB[4*i];
    ext->y = (int) TAB[4*i+1];
    if (ext->y > 512)  Errorf("ext->x = %d ",ext->x); 
     if (ext->y > 512) Errorf("ext->y = %d res=%d",ext->y,res); 
    ext->mag = TAB[4*i+2];
    ext->arg = TAB[4*i+3];
    ext->previous = ext->next = NULL;
    ext->coarser = ext->finer = NULL;
    ext->chain = NULL;
    values[ext->y * ncol + ext->x] = ext;
   
  }
  
  Free(TAB);
  
}

/********************************************************************/    
/* read ext repr from files directly without going through images */
/********************************************************************/ 
   
static void ReadPointReprStream(EXTREP2 extrep,STREAM s)
     
{
 
  FILE *stream;
  int l,noct,normalized,lipflag;
  

/* Some basic tests on the stream */
  if (s->mode != StreamRead) Errorf("ReadPointReprStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadPointReprStream() : You cannot read an image to standard input"); 

   stream = s->stream; 

  
    fscanf(stream,"NOCT (Point_Repr) : %d ",&noct);
    fscanf(stream,"Normalized %d ",&normalized);
    fscanf(stream,"Lipschitz %d ",&lipflag);
    extrep->noct= noct; 
    extrep->normalized=normalized;
    extrep->lipflag=lipflag;

  for (l = 1; l <= extrep->noct; l++)
    W2_point_pic_e_read(extrep->array[l],stream);

  fscanf(stream,"\n\n");
  
ReadImageStream(extrep->coarse ,s,YES,0,0,NO);
  extrep->coarse->border_hor = 1;
  extrep->coarse->border_ver = 1;

 
} 
  
void C_PointReprRead(char **argv)
{
   char  *filename;
  WTRANS2 wtrans=NULL;
  STREAM s;


  
  
   argv= ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);
    if (wtrans ==NULL) wtrans= GetWtrans2Cur(); 

    DeleteExtrep2(wtrans->extrep);
    DeleteChainrep2(wtrans->chainrep);
    wtrans->extrep = NewExtrep2();
    wtrans->chainrep = NewChainrep2(); 

    argv = ParseArgv(argv,tSTREAM_,NULL,&s,-1);
    
   if (s == NULL) {
       argv = ParseArgv(argv,tSTR,&filename,-1);
          
       s =  OpenFileStream(filename,"r");
  
       if (s == NULL) Errorf("Error while opening the file %s",filename);

 
     if (!s->stream) 
          Errorf("unable to open %s\n",filename);
         
        ReadPointReprStream(wtrans->extrep, s); 
  }
    else {
        ReadPointReprStream(wtrans->extrep, s); 
     }


  CloseStream(s);	
  wtrans->noct = wtrans->extrep->noct;
  wtrans->norient = 4;
}
