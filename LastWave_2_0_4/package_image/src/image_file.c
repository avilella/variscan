/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'image' 2.0                        */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry, Jerome Fraleu.              */
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





#include "lastwave.h"
#include "images.h"



/*
 * Read an image from a stream 
 *
 * flagHeader : YES if there is a header in the stream
 * nrow,ncol  : The size of the image to read
 * flagChar   : YES if each pixel value is coded on a single char (otherwise it is coded on a float)
 */

void ReadImageStream(IMAGE image ,STREAM s, char flagHeader,int nrow, int ncol,char flagChar)
{
  FILE *stream;
  float *values;
  int i, pix_number;
  unsigned char tmp;
  char header[200],car[200];
  char *h;
  int countRC;
  int comment=0;

 
  if (s->mode != StreamRead) Errorf("ReadImageStream() : The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadImageStream() : You cannot read an image to standard input"); 
  stream = s->stream;
  
  /* Read the header if necessary */
  if (flagHeader) {
    countRC =0;
    while (countRC!=3){
      h = header;
      while(1) {
	*h = fgetc(stream);
	if (*h == '\n' || *h == '\r') break;
	h++;
	if (h-header == 190) break;
      }
      *(h+1) = '\0';
      countRC += (*h=='\n' || *h == '\r');
      if (strchr(header,'#')) countRC--;
      else { 
        if (countRC==2) { 
          nrow=atoi(header);
          sprintf(car,"%d",nrow);
          for (i=0;i<(strlen(car)+1);i++) header[i]=' ';
          ncol=atoi(header);  
        }
      }  
    }
  }
  
  /* Resize the image */
  SizeImage(image,ncol,nrow);

  values = image->pixels;
  pix_number = nrow * ncol;

  /* Ascii read */
  if(flagChar) {
    for(i = 0; i < pix_number; i++) {
      fscanf(stream,"%c",&tmp);
      values[i] = (float )tmp;
    } 
  }
  
  /* Binary float read */
  else  fread(values, pix_number * sizeof(float), 1, stream);
}


/* The corresponding command */
C_ReadImage(char ** argv)
{
  char  *fname;
  IMAGE image;
  STREAM s;
  char flagHeader,flagChar;
  char car;
  int nrow,ncol;

  argv =   ParseArgv(argv,tIMAGE,&image,tSTREAM_,NULL,&s,-1);

  fname = NULL;
  if (s == NULL) {    
    argv = ParseArgv(argv,tSTR,&fname,-1);
    s =  OpenFileStream(fname,"r");
    if (s == NULL) Errorf("Cannot open the file '%s'",fname);
  }

  ClearImage(image);
  if (s->mode != StreamRead) Errorf("The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("You cannot read an image to standard input"); 

  flagHeader = YES; 
  flagChar = NO;
  while (car=ParseOption(&argv)) {
    switch (car) {
    case 'h':
	  argv = ParseArgv(argv,tINT,&nrow,tINT,&ncol,-1);
      flagHeader = NO;
	  break;
    case 'c':
	  flagChar = YES;
	  break;
    default:
	  ErrorOption(car);
    }
  }

  NoMoreArgs(argv);
  
  ReadImageStream(image,s,flagHeader,nrow,ncol,flagChar);    
  
  if (fname) CloseStream(s);	
}



/*
 * Write an image to a stream 
 *
 * flagHeader : YES if you want a header at the beginning of the stream
 * flagChar   : YES if each pixel value is coded on a single char (otherwise it is coded on a float)
 *              In case it is YES and if min<max then rescaling will be performed between min and max in 
 *              order to match 0 and 255.
 */

void WriteImageStream(IMAGE image,STREAM s,char flagChar,float min, float max, char flagHeader)
{
 
  float *values, *tmpvalues;
  float  val;
  int nrow, ncol;
  int i, pix_number;
  IMAGE tmp_image;
  FILE * stream;
  
  if (s->mode != StreamWrite) Errorf("WriteImageStream() : The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf("WriteImageStream() : You cannot write a signal to standard output"); 
  stream = s->stream;
     

  nrow = image->nrow;
  ncol = image->ncol;
  values = image->pixels;
  pix_number = nrow * ncol;

  /* Header */
  if (flagHeader) {
    fprintf(stream,"P5\n%d %d\n",nrow,ncol);
    if (flagChar) fprintf(stream,"255\n");
    else fprintf(stream,"255.0\n");
  }
  /* end of the header */

  tmp_image = NewImage();
  SizeImage(tmp_image,ncol,nrow);
  tmpvalues = tmp_image->pixels;

  for (i = 0; i < pix_number ; i++) {
    if(flagChar && min<max)  val = (values[i]-min)*255./(max-min);
    else val = values[i];
    if (flagChar) {
      val = MAX(0,val);
      val = MIN(255,val);
    }
    tmpvalues[i] = val;
  }
  
  if(flagChar) {
    for(i = 0; i < pix_number; i++)
      fprintf(stream,"%c",(unsigned char)((int)(tmpvalues[i])));
  }
  else {
    fwrite(tmpvalues,pix_number*sizeof(float),1,stream);
  }

  DeleteImage(tmp_image);
}


/* The corresponding command */
C_WriteImage(char**argv)
{
  char *fname;
  STREAM s;
  IMAGE input;
  char flagHeader,flagRescale,flagChar;
  float min,max,f;
  char car;
  
  argv= ParseArgv(argv,tIMAGEI,&input,tSTREAM_,NULL,&s,-1);
 
  fname = NULL;
  if (s == NULL) {
    argv = ParseArgv(argv,tSTR,&fname,-1);  
    s =  OpenFileStream(fname,"w");
    if (s == NULL) Errorf("Cannot open the file '%s'",fname);
  } 

  if (s->mode != StreamWrite) Errorf("The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf(" You cannot write a signal to standard output"); 
     
  flagHeader = YES;
  flagRescale = NO; 
  flagChar = NO;
  min = max = 0;
  while (car=ParseOption(&argv)) {
    switch (car) {
    case 'c':
      flagChar = YES;
      break;
     case 'r':
      flagRescale = YES;
      argv = ParseArgv(argv,tFLOAT_,0.0,&min,tFLOAT_,0.0,&max,-1);
      break;
    case 'h':
      flagHeader = NO;
      break;
    default:
      ErrorOption(car);
    } 
  }
  
  
  NoMoreArgs(argv);
 
  if (flagRescale) {
    if (min == max) MinMaxImage(input,NULL,NULL,&min,NULL,NULL,&max);
    else if (min > max) {
      f = min; min = max; max = f;
    }
  }
  
  WriteImageStream(input,s,flagChar,min,max,flagHeader); 

  if (fname) CloseStream(s);
}

