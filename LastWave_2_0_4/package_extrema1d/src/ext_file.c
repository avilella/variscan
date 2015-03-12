/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'extrema1d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1999-2002                 Emmanuel Bacry              */
/*      email : lastwave@cmap.polytechnique.fr                              */
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
/*    ext_file.c       write/read an extrep                                 */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "extrema1d.h"



/****************************************************/
/*         Write an extrep                          */
/****************************************************/

void WriteExtrepStream (EXTREP extrep,STREAM s,int flagHeader)
{
  int o,v;
  EXT ext;
  FILE *stream;
   
  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("WriteStreamExtrep() :The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf("WriteStreamExtrep() : You cannot write a signal to standard output"); 
  stream = s->stream; 
 
   
  /* Write header if asked */
  if (flagHeader == YES) {
    fprintf(stream,"LastWave Header\n");
    fprintf(stream,"Number octaves %d\n",extrep->nOct);
    fprintf(stream,"Number voices %d\n",extrep->nVoice);
	fprintf(stream,"Smallest scale %.8g\n",extrep->aMin);
	if (extrep->wName != NULL) fprintf(stream,"Wavelet name %s\n",extrep->wName);
    else fprintf(stream,"Wavelet name Unknown\n");
	fprintf(stream,"Size %d\n",extrep->size);
	fprintf(stream,"x0 %.8g\n",extrep->x0);
	fprintf(stream,"dx %.8g\n",extrep->dx);
	fprintf(stream,"End of Header\n");
  }
	
   for(o=1;o<=extrep->nOct;o++)
	for(v=0;v<extrep->nVoice;v++) 
	  for(ext = extrep->D[o][v]->first;ext != NULL;ext = ext->next) 
        fprintf(stream,"%.8g %d %.8g\n",ext->abscissa,ext->scale,ext->ordinate);         
}

void WriteExtrep (EXTREP extrep,char filename[],int flagHeader)
{
  STREAM stream;

  CheckExtrep(extrep);
  
  /* Open file */
  stream = OpenFileStream(filename,"w");
  if (stream == NULL) Errorf("WriteExtrep() : Error while opening the file %s",filename);

  WriteExtrepStream (extrep,stream,flagHeader);

  CloseStream(stream);
}


/*
 * The corresponding command 
 */
 
void C_EWrite(char **argv)
{
  EXTREP extrep;
  STREAM stream;
  char *filename;
  int flagHeader;
  char opt;
    	
  argv = ParseArgv(argv,tEXTREP_,NULL,&extrep,-1);
  if (extrep == NULL) extrep = GetExtrepCur();
  argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
  if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
  else filename = NULL;

  /* options */
  flagHeader = YES;
  while(opt = ParseOption(&argv)) { 
    switch(opt) {
    case 'h': flagHeader = NO; break;
    default: ErrorOption(opt);
    }
  }    
  NoMoreArgs(argv);

  if (stream == NULL) WriteExtrep(extrep,filename,flagHeader);
  else WriteExtrepStream(extrep,stream,flagHeader);
}
   
   

/****************************************************/
/*         Read an extrep                           */
/****************************************************/

/*  Insert an extremum in an extrep */
int InsertExt(EXTREP extrep,EXT ext)
{
  int nVoice;
  int nOct;
  EXT ext1;
  EXTLIS extlis;
  
  nOct = ext->scale / extrep->nVoice + 1;
  nVoice = ext->scale % extrep->nVoice;
  
  if (nOct > extrep->nOct) return(NO);
  
  extlis = extrep->D[nOct][nVoice];
  if (extlis->first == NULL) {
    extlis->first = extlis->end = ext;
    extlis->size = 1;
    ext->next = ext->previous = NULL;
    ext->extlis = extlis;
    return(YES);
  }
  
  ext1 = extlis->first;
  while (ext1->abscissa < ext->abscissa && ext1->next != NULL) ext1 = ext1->next;
  if (ext1->abscissa < ext->abscissa) {
    ext1->next = ext;
    ext->previous = ext1;
    ext->next = NULL;
    extlis->end = ext;
  }
  else {
    ext->previous = ext1->previous;
    ext->next = ext1;
    ext1->previous = ext;
    if (ext->previous != NULL) ext->previous->next = ext;
    else extlis->first = ext;
  }
  
  ext->extlis = extlis;
  
  extlis->size++;
  return(YES);
} 


/* Read from a stream */
void ReadExtrepStream (EXTREP extrep,STREAM s)
{
  float x1,x3;
  int i2;
  int flagHeader;
  int size,nOct,nVoice;
  float dx,x0,aMin;
  char wName[255],str[255];
  int n,nErr = 0;
  EXT ext;
  FILE *stream;
  
  /* Some basic tests on the stream */
  if (s->mode != StreamRead) Errorf("ReadExtrepStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadExtrepStream() : You cannot write a signal to standard input"); 
  stream = s->stream; 
	
  ClearExtrep(extrep);
	
  flagHeader = NO;

  /* Let's try to read the header */
  fscanf(stream,"%[^\n] ",str);
  n = sscanf(str,"%f %d %f",&x1,&i2,&x3);
  if (n == -1) Errorf("ReadExtrepStream() : Error at the beginning of the file");
  if (n == 0) { /* there is a header */
    flagHeader = YES;
	if (strcmp(str,"LastWave Header")) Errorf("ReadExtrepStream() : Error in the header of the file");
    n += fscanf(stream,"Number octaves %d ",&nOct);
    n += fscanf(stream,"Number voices %d ",&nVoice);
    n += fscanf(stream,"Smallest scale %f ",&aMin);
    n += fscanf(stream,"Wavelet name %s ",wName);
    n += fscanf(stream,"Size %d ",&size);
    n += fscanf(stream,"x0 %f ",&x0);
    n += fscanf(stream,"dx %f\n",&dx);
		
	fscanf(stream,"%[^\n] ",str);
	if (n != 7 || strcmp(str,"End of Header")) Errorf("ReadExtrepStream() : Error in the header of the file");
		
    fscanf(stream,"%[^\n] ",str);
    n = sscanf(str,"%f %d %f",&x1,&i2,&x3);
    if (n == -1) Errorf("ReadExtrepStream() : Error at the end of the header of the file");

    if (n != 3) Errorf("ReadExtrepStream() : Error during reading the file");
  }   
  
  if (flagHeader == NO) Errorf("ReadExtrepStream() : I am lost without a header");
    
  extrep->nOct = nOct;
  extrep->nVoice = nVoice;
  extrep->aMin = aMin;
  extrep->size = size;
  extrep->x0 = x0;
  extrep->dx = dx;
  extrep->wName = CopyStr(wName);
    
  while (YES) {
    ext = NewExt();
    ext->abscissa = x1;
    ext->scale = i2;
    ext->ordinate = x3;
    if (InsertExt(extrep,ext) == NO) Errorf("ReadExtrepStream() : Error during extrema insertion");
    n = fscanf(stream,"%[^\n] ",str);
    if (n == EOF) break;
    n = sscanf(str,"%f %d %f",&x1,&i2,&x3);
	if (n != 3) Errorf("ReadExtrepStream() : Error while reading the file");
  }
   
  Chain(extrep,0.);
  ChainDelete(extrep);
}
      

/* Same as above but with a filename insetad of a stream */
void ReadExtrep(EXTREP extrep,char *filename)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"r");
  if (stream == NULL) Errorf("ReadExtrep() : Error while opening the file %s",filename);

  ReadExtrepStream(extrep,stream);
	
  CloseStream(stream);	
}


/* The corresponding command */                
void C_ERead (char **argv)
{
  EXTREP extrep;
  char *filename;
  STREAM stream;
    
  argv = ParseArgv(argv,tEXTREP_,NULL,&extrep,-1);
  if (extrep == NULL) extrep= GetExtrepCur();
  
  argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
  if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
  else filename = NULL;
      
  NoMoreArgs(argv);

  if (filename != NULL) ReadExtrep(extrep,filename);
  else  ReadExtrepStream(extrep,stream);
}
                
                

                
                
