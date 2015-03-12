/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'wtrans1d' 2.0                     */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
/*      email : lastwave@polytechnique.fr                                   */
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
/*    wtrans_file.c       write/read a wtrans                               */
/*                                                                          */
/****************************************************************************/

#include "lastwave.h"
#include "wtrans1d.h"


/****************************************************
 *
 *         Write a wtrans                           
 *
 ****************************************************/

void WriteBinWtransStream (WTRANS wtrans,STREAM s,char flagBinary)
{
   int o,v;
   FILE *stream;
   
  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("WriteBinWtransStream() :The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf("WriteBinWtransStream() : You cannot write a signal to standard output"); 
  stream = s->stream; 
   
   
  /* Write header */
  fprintf(stream,"Number octaves %d\n",wtrans->nOct);
  fprintf(stream,"Number voices %d\n",wtrans->nVoice);
  fprintf(stream,"size %d\n",wtrans->size);
  fprintf(stream,"x0 %g\n",wtrans->x0);
  fprintf(stream,"dx %g\n",wtrans->dx);
  fprintf(stream,"Type %d\n",wtrans->type);
  fprintf(stream,"Smallest scale %.8g\n",wtrans->aMin);
  fprintf(stream,"Wavelet name : %s\n",wtrans->wName);

  for(o=1;o<=wtrans->nOct;o++)
	for(v=0;v<wtrans->nVoice;v++) 
/* ./package_signal/src/signal_file.c:111:void WriteSigStream(SIGNAL signal,STREAM s, char flagBinary, char *mode,int flagHeader) */
                WriteSigStream(wtrans->D[o][v],s,flagBinary,"y",YES);          
 
      
  if (wtrans->type == W_ORTH) WriteSigStream(wtrans->A[o-1][0],s,flagBinary,"y",YES);    
}



void WriteBinWtransFile (WTRANS wtrans,char *filename,char flagBinary)
{
  STREAM stream;

  CheckWtrans(wtrans);
  
  /* Open file */
  stream = OpenFileStream(filename,"w");
  if (stream == NULL) Errorf("WriteBinWtrans() : Error while opening the file %s",filename);

  /* read */
  WriteBinWtransStream(wtrans,stream,flagBinary);
    
  /* Close */
  CloseStream(stream);
}      



/* The command */   
void C_WWrite (char **argv)
{
  WTRANS wtrans;
  char *filename;
  STREAM stream;
  int flagBinary;
  char opt;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,-1);
  if (wtrans == NULL) wtrans = GetWtransCur();
  
  argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
  if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
  else filename = NULL;

  /* options */
  flagBinary = YES;
  while(opt = ParseOption(&argv)) { 
    switch(opt) {
    case 'a': flagBinary = NO; break;
    default: ErrorOption(opt);
    }
  }    
  NoMoreArgs(argv);

  CheckWtrans(wtrans);

  if (filename == NULL) WriteBinWtransStream(wtrans,stream,flagBinary);
  else WriteBinWtransFile(wtrans,filename,flagBinary);
}


/****************************************************
 *
 *         Read a wtrans                           
 *
 ****************************************************/

void ReadBinWtransStream (WTRANS wtrans,STREAM s)
{
  int n,o,v,size;
  float aMin,x0,dx;
  int nOct,nVoice,type;
  char wName[20];
  FILE *stream;
  
  /* Some basic tests on the stream */
  if (s->mode != StreamRead) Errorf("ReadBinWtransStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadBinWtransStream() : You cannot write a signal to standard input"); 
  stream = s->stream; 
   
	
  /* Let's read the header */
  n = 0;
  x0 = 0;
  dx = 1;
  size = -1;
  n += fscanf(stream,"Number octaves %d ",&nOct);
  n += fscanf(stream,"Number voices %d ",&nVoice);
  n += fscanf(stream,"size %d ",&size);
  n += fscanf(stream,"x0 %f\n",&x0);
  n += fscanf(stream,"dx %f\n",&dx);
  n += fscanf(stream,"Type %d ",&type);
  n += fscanf(stream,"Smallest scale %f ",&aMin);
  n += fscanf(stream,"Wavelet name : %s ",wName);
		
  if (n < 5) Errorf("Error in the header of the file");

  wtrans->nOct = nOct;
  wtrans->nVoice = nVoice;
  wtrans->size = size;
  wtrans->type = type;
  wtrans->aMin = aMin;
  wtrans->dx = dx;
  wtrans->x0 = x0;
  if (wtrans->wName != NULL) Free(wtrans->wName);
  wtrans->wName = CopyStr(wName);
  
  for(o=1;o<=wtrans->nOct;o++)
	for(v=0;v<wtrans->nVoice;v++) 
      ReadSigStream(wtrans->D[o][v],s,0,-1,0,0); 
  
  if (wtrans->type == W_ORTH) ReadSigStream(wtrans->A[o-1][0],s,0,-1,0,0);    
    
  if (wtrans->size <= 0) wtrans->size = wtrans->D[1][0]->size;     
}


void ReadBinWtransFile (WTRANS wtrans,char *filename)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"r");
  if (stream == NULL) Errorf("ReadBinWtrans() : Error while opening the file %s",filename);

  /* read */
  ReadBinWtransStream(wtrans,stream);
    
  /* Close */
  CloseStream(stream);
}      


/* The command */   
void C_WRead (char **argv)
{
  WTRANS wtrans;
  char *filename;
  STREAM stream;

  argv = ParseArgv(argv,tWTRANS_,NULL,&wtrans,-1);
  if (wtrans == NULL) wtrans = GetWtransCur();
  
  argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
  if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
  else filename = NULL;

  NoMoreArgs(argv);
 			
  if (filename == NULL) ReadBinWtransStream(wtrans,stream);
  else ReadBinWtransFile(wtrans,filename);
}


         
                
