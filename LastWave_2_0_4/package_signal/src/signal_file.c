/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'signal' 2.0                       */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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
/*  signal_file.c   Deals with the files                                    */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "signals.h"




/*******************************************************
 *
 * Write a signal in a file  
 *
 ******************************************************/

/*
 * Function to write the y-values of a signal using raw format
 *   signal      : the signal to write
 *   stream      : the file stream
 *   binaryCoding  : either BinaryLittleEndian or BinaryBigEndian or 0 (in that latter case the current cpu mode is used)
 */

void WriteSigRawStream(SIGNAL signal,STREAM s, char binaryCoding)
{
  FILE *stream;
  int flagBL;
  
  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("WriteSigRawStream() : The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf("WriteSigRawStream() : You cannot write a signal to standard output"); 
  stream = s->stream; 
  	
  if ((IsCPULittleEndian && (binaryCoding == BinaryBigEndian)) || (IsCPUBigEndian && (binaryCoding == BinaryLittleEndian))) {
    flagBL = YES;
    BigLittleValues(signal->Y,signal->size,sizeof(float));
  }
  else flagBL = NO;
  
 /* We then write the values */	
 fwrite(signal->Y,sizeof(float),signal->size,stream);

 if (flagBL) {
    BigLittleValues(signal->Y,signal->size,sizeof(float));
  }
}

/* 
 * Same as above but with a filename instead of a stream 
 */
 
void WriteSigRawFile(SIGNAL signal,char *filename, char binaryCoding)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"w");

  if (stream == NULL) Errorf("WriteSigRawFile() : Error while opening the file %s",filename);
	
  WriteSigRawStream(signal,stream,binaryCoding);
  
  CloseStream(stream);
}
	
/*
 * Function to write a signal using only ascii format
 *   signal      : the signal to write
 *   stream      : the file stream
 *   flagBinary  : If YES then the values are coded using binary codes (and not ascii)
 *   mode == "xy"    ==> write X Y (two columns in ascii mode and X-values followed by Y-values if binary mode)
 *		     "yx"    ==> write Y X (two columns in ascii mode and Y-values followed by X-values if binary mode)
 *			 "x"     ==> write X   (just X is stored)
 *			 "y"     ==> write Y   (just Y is stored)    
 *           ""      ==> the  function chooses the best mode.
 *   flagHeader  : Do we want a header to be written ? (if flagBinary==YES then one should have flagHeader==YES)
 */

void WriteSigStream(SIGNAL signal,STREAM s, char flagBinary, char *mode,int flagHeader)
{
  int i;
  FILE *stream;
  
  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("WriteSigStream() : The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf("WriteSigStream() : You cannot write a signal to standard output"); 
  stream = s->stream; 
  
  /* Test the mode */
  if (!strcmp(mode,"")) {
    if (signal->type == YSIG) mode = "y";
    else mode = "xy";
  }
  if (strcmp(mode,"xy") && strcmp(mode,"yx") && strcmp(mode,"x") &&strcmp(mode,"y"))
    Errorf("WriteSigStream() : Mode '%s' not allowed",mode);

  /* Some basic checkings */
  if (flagBinary==YES) {
    if (flagHeader ==NO) Errorf("WriteSigStream() : Cannot write binary values without header");
  }
	
  /* Write header if asked */
  if (flagHeader == YES) {
	fprintf(stream,"LastWave Header\n");
	fprintf(stream,"size %d\n",signal->size);
	if (signal->name == NULL || signal->name[0] == '\0') fprintf(stream,"name none\n");
	else fprintf(stream,"name %s\n",signal->name);
	fprintf(stream,"firstp %d\n",signal->firstp);
	fprintf(stream,"lastp %d\n",signal->lastp);
	
	if (flagBinary == NO) fprintf(stream,"binary no\n");
	else {
	  if (IsCPULittleEndian) fprintf(stream,"binary little\n");
      else fprintf(stream,"binary big\n");
    }
    		
	/* 2 types of signal XYSIG or YSIG */
	if (signal->type == YSIG && !strcmp(mode,"y")) {
#ifdef NUMDOUBLE	
	  fprintf(stream,"dx %.16g\n",signal->dx);
	  fprintf(stream,"x0 %.16g\n",signal->x0);
#else
	  fprintf(stream,"dx %.8g\n",signal->dx);
	  fprintf(stream,"x0 %.8g\n",signal->x0);
#endif
	}
	if (signal->type == XYSIG && strcmp(mode,"xy") && strcmp(mode,"yx")) {
#ifdef NUMDOUBLE	
	  fprintf(stream,"dx %.16g\n",1.0);
	  fprintf(stream,"x0 %.16g\n",0.0);
#else
	  fprintf(stream,"dx %.8g\n",1.0);
	  fprintf(stream,"x0 %.8g\n",0.0);
#endif
	}
	
	fprintf(stream,"End of Header\n");
  }

if (flagBinary == NO) {
  /* We then write in the file */
  /* according to the mode     */
	
  if (!strcmp(mode,"y")) 
 	for (i=0;i<signal->size;i++) 
#ifdef NUMDOUBLE	
      fprintf(stream,"%.16g\n",signal->Y[i]);
#else
      fprintf(stream,"%.8g\n",signal->Y[i]);
#endif	
  if (!strcmp(mode,"x"))
	for (i=0;i<signal->size;i++) 
#ifdef NUMDOUBLE	
  	  fprintf(stream,"%.16g\n",XSig(signal,i));
#else
  	  fprintf(stream,"%.8g\n",XSig(signal,i));
#endif
	
  if (!strcmp(mode,"xy"))
	for (i=0;i<signal->size;i++) {
#ifdef NUMDOUBLE	
      fprintf(stream,"%.16g    ",XSig(signal,i));
	  fprintf(stream,"%.16g\n",signal->Y[i]);
#else
      fprintf(stream,"%.8g    ",XSig(signal,i));
	  fprintf(stream,"%.8g\n",signal->Y[i]);
#endif
	}
		
  if (!strcmp(mode,"yx"))
	for (i=0;i<signal->size;i++) {
#ifdef NUMDOUBLE	
	  fprintf(stream,"%.16g    ",signal->Y[i]);
	  fprintf(stream,"%.16g\n",XSig(signal,i));
#else
	  fprintf(stream,"%.8g    ",signal->Y[i]);
	  fprintf(stream,"%.8g\n",XSig(signal,i));
#endif
	}
}
else {
 /* We then write the values */	
  if (!strcmp(mode,"xy")) { 
    fwrite(signal->X,sizeof(float),signal->size,stream);
    fwrite(signal->Y,sizeof(float),signal->size,stream);
  }
  else if (!strcmp(mode,"yx")) { 
    fwrite(signal->Y,sizeof(float),signal->size,stream);
    fwrite(signal->X,sizeof(float),signal->size,stream);
  }
  if (!strcmp(mode,"x")) { 
    fwrite(signal->X,sizeof(float),signal->size,stream);
  }
  if (!strcmp(mode,"y")) { 
    fwrite(signal->Y,sizeof(float),signal->size,stream);
  }
}
}


/* 
 * Same as above but with a filename instead of a stream 
 */
 
void WriteSigFile(SIGNAL signal,char *filename, char flagBinary, char *mode,int flagHeader)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"w");

  if (stream == NULL) Errorf("WriteSigFile() : Error while opening the file %s",filename);
	
  WriteSigStream(signal,stream,flagBinary,mode,flagHeader);
  
  CloseStream(stream);
}
	


/*
 * Function to write a signal using ascii (optional) header and binary values
 *   signal      : the signal to write
 *   stream      : the file stream
 *   flagHeader  : Do we want a header to be written ?
 *   mode == "xy"    ==> write X values then Y values
 *		     "yx"    ==> write Y values then X values
 *			 "x"     ==> write X values
 *			 "y"     ==> write Y values
 *           ""      ==> the function chooses the best mode.
 */

void WriteBinSigStream(SIGNAL signal,STREAM s,char flagHeader,char *mode)
{
  FILE *stream;
  
  Warningf("WriteBinSigStream() : You should no longer use this C function. It will be suppressed in the next package version. Please use the new 'WriteSigStream' or 'WriteSigFile'functions.");

  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("WriteBinSigStream() :The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) Errorf("WriteBinSigStream() : You cannot write a signal to standard output"); 
  stream = s->stream; 

  /* Test the mode */
  if (!strcmp(mode,"")) {
    if (signal->type == YSIG) mode = "y";
    else mode = "xy";
  }
  if (strcmp(mode,"xy") && strcmp(mode,"yx") && strcmp(mode,"x") &&strcmp(mode,"y"))
    Errorf("WriteSigStream() : Mode '%s' not allowed",mode);
  if ((!strcmp(mode,"xy") || !strcmp(mode,"yx") || !strcmp(mode,"x")) && signal->type == YSIG)
    Errorf("WriteBinSigStream() : Mode '%s is not allowed for YSIG signals",mode);
  if ((!strcmp(mode,"xy") || !strcmp(mode,"yx"))  && flagHeader == NO) 
    Errorf("WriteBinSigStream() : Cannot use mode '%s' without any header",mode);
      
  /* Write header if asked */
  if (flagHeader == YES) {
 	fprintf(stream,"LastWave Header\n");
 	fprintf(stream,"size %d\n",signal->size);
	if (signal->name == NULL || signal->name[0] == '\0') fprintf(stream,"name none\n");
	else fprintf(stream,"name %s\n",signal->name);
	fprintf(stream,"firstp %d\n",signal->firstp);
	fprintf(stream,"lastp %d\n",signal->lastp);
		
	/* 2 types of signal XYSIG or YSIG */
	if (signal->type == YSIG && !strcmp(mode,"y")) {
#ifdef NUMDOUBLE	
	  fprintf(stream,"dx %.16g\n",signal->dx);
	  fprintf(stream,"x0 %.16g\n",signal->x0);
#else
	  fprintf(stream,"dx %.8g\n",signal->dx);
	  fprintf(stream,"x0 %.8g\n",signal->x0);
#endif
	}
	if (signal->type == XYSIG && strcmp(mode,"xy") && strcmp(mode,"yx")) {
#ifdef NUMDOUBLE	
	  fprintf(stream,"dx %.16g\n",1.0);
	  fprintf(stream,"x0 %.16g\n",0.0);
#else
	  fprintf(stream,"dx %.8g\n",1.0);
	  fprintf(stream,"x0 %.8g\n",0.0);
#endif
	}
		
	fprintf(stream,"End of Header\n");
  }

  /* We then write the values */	
  if (!strcmp(mode,"xy")) { 
    fwrite(signal->X,sizeof(float),signal->size,stream);
    fwrite(signal->Y,sizeof(float),signal->size,stream);
  }
  else if (!strcmp(mode,"yx")) { 
    fwrite(signal->Y,sizeof(float),signal->size,stream);
    fwrite(signal->X,sizeof(float),signal->size,stream);
  }
  if (!strcmp(mode,"x")) { 
    fwrite(signal->X,sizeof(float),signal->size,stream);
  }
  if (!strcmp(mode,"y")) { 
    fwrite(signal->Y,sizeof(float),signal->size,stream);
  }
}

/* 
 * Same as above but with a filename instead of a stream 
 */
void WriteBinSigFile(SIGNAL signal,char *filename,char flagHeader,char  *mode)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"w");
  if (stream == NULL) Errorf("WriteSignalBin() : Error while opening the file %s",filename);

  /* Write */
  WriteBinSigStream(signal,stream,flagHeader,mode);
    
  /* Close */
  CloseStream(stream);
}


/*
 * The corresponding command 
 */
 
void C_Write(char **argv)
{
  SIGNAL signal;
  STREAM stream;
  char *filename,*mode,*bmode;
  int flagHeader,flagBinary,flagRaw;
  char opt,binaryCoding;
    	
  argv = ParseArgv(argv,tSIGNALI,&signal,-1);
  argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
  if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
  else filename = NULL;


  argv = ParseArgv(argv,tSTR_,"",&mode,-1);
				
  /* options */
  flagHeader = YES;
  flagBinary = NO;
  flagRaw = NO;
  while(opt = ParseOption(&argv)) { 
    switch(opt) {
    case 'h': flagHeader = NO; break;
    case 'b': flagBinary = YES; break;
    case 'r':
      argv = ParseArgv(argv,tSTR_,"",&bmode,-1);
      if (!strcmp(bmode,"little")) binaryCoding = BinaryLittleEndian;
      else if (!strcmp(bmode,"big")) binaryCoding = BinaryBigEndian;
      else if (!strcmp(bmode,"")) binaryCoding = 0;
      else Errorf("Bad binary mode '%s'",bmode);
      flagRaw = YES;
      break;
    default: ErrorOption(opt);
    }
  }    
  NoMoreArgs(argv);

  if (flagRaw == YES) {
    if (stream == NULL) WriteSigRawFile(signal,filename,binaryCoding);
    else WriteSigRawStream(signal,stream,binaryCoding);
  }
  else {
    if (stream == NULL) WriteSigFile(signal,filename,flagBinary,mode,flagHeader);
    else WriteSigStream(signal,stream,flagBinary,mode,flagHeader);
  }
}

		
		
/*******************************************************
 *
 * Read a signal in a file  
 *
 ******************************************************/


/*
 * Function to read from a stream a signal coded in raw format, i.e., no header and binary (float) values
 *
 * signal      : the signal to read
 * stream      : the file stream
 *
 * firstIndex  : first index in the file to be read (starting from 0)
 * sizeToRead  : the number of points to be read (if -1 everything is read)
 *
 * binaryCoding  : either BinaryLittleEndian or BinaryBigEndian or 0 (in that latter case the current cpu mode is used)
 */
 
void ReadSigRawStream(SIGNAL signal,STREAM s, int firstIndex,int sizeToRead, char binaryCoding)
{
  long int pos,pos1;
  int size;  
  FILE *stream;
  
  /* 
   * Some basic tests on the stream 
   */
  if (s->mode != StreamRead) Errorf("ReadSigRawStream() : The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadSigRawStream() : You cannot read a signal to standard input"); 
  stream = s->stream;   
  
  /* Compute the size */
  pos = ftell(stream);
  fseek(stream,0,SEEK_END);
  pos1 = ftell(stream);
  fseek(stream,pos-pos1,SEEK_CUR);
  size = (pos1-pos+1)/sizeof(float);

  /* Some checkings */
  if (firstIndex >= size) Errorf("ReadSigRawStream() : 'firstIndex' (%d) is too large",firstIndex);
  if (sizeToRead == -1) sizeToRead = size-firstIndex;
  if (sizeToRead+firstIndex > size) Errorf("ReadSigRawStream() : 'sizeToRead' (%d) is too large",sizeToRead);

  /* Let's read */
  SizeSignal(signal,sizeToRead,YSIG);
  if (fseek(stream,firstIndex*sizeof(float),SEEK_CUR) != 0)     
    Errorf("ReadSigRawStream() : firstIndex seems to be too large for this file"); 
  if (fread(signal->Y,sizeof(float),sizeToRead,stream) != sizeToRead)
    Errorf("ReadSigRawStream() : sizeToRead seems to be too large for this file");

  /* Big/Little conversions */
  if (binaryCoding == 0) {
    if (IsCPULittleEndian) binaryCoding = BinaryLittleEndian;
    else binaryCoding = BinaryBigEndian;
  }
  if (((binaryCoding == BinaryLittleEndian) && IsCPUBigEndian) || ((binaryCoding == BinaryBigEndian) && IsCPULittleEndian)) {
    BigLittleValues(signal->Y,signal->size,sizeof(float));
  }
 
 /* Some settings */              
 SetNameSignal(signal,NULL);
}


/* Same as above but with a filename instead of a stream */
void ReadSigRawFile(SIGNAL signal,char *filename,  int firstIndex,int sizeToRead, char binaryCoding)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"r");
  if (stream == NULL) Errorf("ReadSigRawFile() : Error while opening the file %s",filename);

  ReadSigRawStream(signal,stream,firstIndex,sizeToRead,binaryCoding);
	
  CloseStream(stream);	
  
  if (!strcmp(signal->name,"")) SetNameSignal(signal,filename);
}

/*
 * Function to get information about a signal stored in a stream without reading the signal itself.
 *
 * The parameters are :
 *
 * stream      : the file stream
 *
 * The following parameters will be filled by te function
 *
 * siginfo : a signal structure where all the fields are filled up except the Y or X
 *           (contains, size, firstp, lastp, XYSIG, YSIG, dx, x0 info)
 *           WARNING : NO ALLOCATION IN 'siginfo' IS MADE
 * header : YES if there is a header
 * flagBinary : YES or NO
 * binaryCoding : BinaryLittleEndian or BinaryBigEndian (if flagBinary == YES only)
 * nCols   : the number of columns (if flagBinary == NO only)
 * 
 * It returns YES if the format is a LastWave format otherwise it returns NO
 *
 * This function does not change the position of the stream.
 */
 
char ReadInfoSigStream(STREAM s, SIGNAL siginfo, char *header, char *flagBinary, char *binaryCoding, int *nColumns)
{
  int flagHeader;
  int flagXY;
  int firstp,lastp,size;
  float dx,x0,f,f1;
  char name[255],str[1000],*str1,c;
  int n,i;
  int nCol;
  FILE *stream;
  fpos_t begining;
  char flagBinary1;
  char flagLittleEndian;
  char flagNewFormat;
  
  /* 
   * Some basic tests on the stream 
   */
  if (s->mode != StreamRead) Errorf("ReadInfoSigStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadInfoSigStream() : You cannot read a signal to standard input"); 
  stream = s->stream;   
   
  /* 
   * Let's try to read the header 
   */
  flagHeader = NO;
  flagBinary1 = NO;
  flagXY = YES;
  flagNewFormat = NO;
  fgetpos(stream,&begining);
  str1 = "LastWave Header";
  while (*str1 != '\0') {
    fscanf(stream,"%c",&c);
    if (c != *str1) break;
    str1++;
  }
  
  if (*str1 == '\0') {
    fscanf(stream,"%c",&c);
    flagHeader = YES;
    n = 0;
	n += fscanf(stream,"size %d ",&size);
	n += fscanf(stream,"name %[^\n] ",name);
	n += fscanf(stream,"firstp %d ",&firstp);
	n += fscanf(stream,"lastp %d ",&lastp);
		
	if (n != 4) Errorf("ReadSigStream() : Error in the header of the file");

    /* Extended header (Signal Package, version 1.5) */
	n += fscanf(stream,"binary %s ",&str);
    if (n == 5) {
      n--;
      flagNewFormat = YES;
      if (!strcmp(str, "no")) flagBinary1 = NO;
      else {
        flagBinary1 = YES;
        if (!strcmp(str, "little")) flagLittleEndian = YES;
        else if (!strcmp(str, "big")) flagLittleEndian = NO;
        else Errorf("ReadSigStream() : Bad field value '%s' of field 'binary'",str);
      } 
    }
    else flagBinary1 = NO;
    /* End of extended header */
    	
	fscanf(stream,"%[^\n] ",str);
	if (strcmp(str,"End of Header")) { /* it is a YSIG */
	  flagXY = NO;
#ifdef NUMDOUBLE
	  n += sscanf(str,"dx %lf",&dx);
	  n += fscanf(stream,"x0 %lf ",&x0);
#else
	  n += sscanf(str,"dx %f",&dx);
	  n += fscanf(stream,"x0 %f ",&x0);
#endif
	  if (n != 6) Errorf("ReadSigStream() : Error in the header of the file");
	  fscanf(stream,"%[^\n] ",str);
	  if (strcmp(str,"End of Header")) Errorf("ReadSigStream() : Error in the header of the file");
	}
  }

  fsetpos(stream,&begining);

  /*
   * Case there is a header 
   */
  if (flagHeader) {
    *header = YES;
    siginfo->size = size;
    siginfo->lastp = lastp;
    siginfo->firstp = firstp;
    if (flagXY == NO) {
      siginfo->dx = dx;
	  siginfo->x0 = x0;
	  siginfo->type = YSIG;
	  *nColumns = 1;
    }
    else {
      siginfo->type = XYSIG;
      *nColumns = 2;
    }

    if (flagBinary1) {
      *flagBinary = YES;
      if (flagLittleEndian) *binaryCoding = BinaryLittleEndian;
      else  *binaryCoding = BinaryBigEndian;
    }
    
    return(YES);
  }
        
        
  /*
   * Case there is no header
   */

  /* How many columns on the first line ? */
#ifdef NUMDOUBLE
  if (fscanf(stream,"%lf",&f) != 1) return(NO);
#else
  if (fscanf(stream,"%f",&f) != 1) return(NO);
#endif
  fsetpos(stream,&begining);  
  fscanf(stream,"%[^\n] ",str);
  nCol = 0;
  str1 = str;
  while (*str1 != '\0' && (*str1 == ' ' || *str1 == '\t')) str1++;
  while (*str1 != '\0') {
    nCol++;
    while (*str1 != '\0' && *str1 != ' ' && *str1 != '\t') str1++;
    while (*str1 != '\0' && (*str1 == ' ' || *str1 == '\t')) str1++;
  }

  fsetpos(stream,&begining);

  if (nCol <= 0) return(NO);
	
  /* Let's read !! */
  size = 0;
  for(i= 0 ;;i++) { 
  
    if (nCol == 1) {
#ifdef NUMDOUBLE
      n = fscanf(stream,"%lf ",&f);
#else
      n = fscanf(stream,"%f ",&f);
#endif
      if (n == EOF) break;
      if (n != 1) return(NO);
      size++;
    } 
    else if (nCol == 2) {
#ifdef NUMDOUBLE
      n = fscanf(stream,"%lf %lf ",&f,&f1);
#else
      n = fscanf(stream,"%f %f ",&f,&f1);
#endif
      if (n == EOF) break;
      if (n != 2) return(NO);
      size++;
    } 
    else {
#ifdef NUMDOUBLE
      n = fscanf(stream,"%lf %lf%*[^\n] ",&f,&f1);
#else
      n = fscanf(stream,"%f %f%*[^\n] ",&f,&f1);
#endif
      if (n == EOF) break;
      if (n != 2) return(NO);
      size++;
    } 
  }
    
  fsetpos(stream,&begining);
	
  /* Fill up the results */
  *header = NO;
  siginfo->size = size;
  siginfo->lastp = size-1;
  siginfo->firstp = 0;
  siginfo->dx = 1;
  siginfo->x0 = 0;
  if (nCol == 2) siginfo->type = XYSIG;
  else siginfo->type = YSIG;
  *nColumns = nCol;
  *flagBinary = NO;
  
  return(YES);
}

/* Same as above but with a filename instead of a stream */
char ReadInfoSigFile(char *filename, SIGNAL siginfo, char *header, char *flagBinary, char *binaryCoding, int *nColumns)
{
  STREAM stream;
  char ans;
  
  /* Open file */
  stream = OpenFileStream(filename,"r");
  if (stream == NULL) Errorf("ReadInfoSigStream() : Error while opening the file %s",filename);

  ans = ReadInfoSigStream(stream,siginfo,header,flagBinary,binaryCoding,nColumns);
	
  CloseStream(stream);	  
  
  return(ans);
}

/*
 * Function to read from a stream  a signal.
 * It can be coded using 
 *     - (Optional LastWave Header), ascii coded floats
 *     - Header, binary coded floats
 *
 * The parameters are :
 *
 * signal      : the signal to read
 * stream      : the file stream
 *
 * firstIndex  : first index in the file to be read (starting from 0)
 * sizeToRead  : the number of points to be read (if -1 everything is read)
 *
 * The two next fields are used only for ascii streams (othewise they should be set to 0,0)
 *
 * xcol        : The column number corresponding to the x-values (starting from 1)
 * ycol        : The column number corresponding to the y-values (starting from 1)
 *    If xcol or ycol is 0 then the function make some natural assumptions.....
 *    If xcol == -1 then only the Y field should be read
 */
 
void ReadSigStream(SIGNAL signal,STREAM s, int firstIndex,int sizeToRead, int xcol,int ycol)
{
  int flagHeader;
  int flagXY;
  int firstp,lastp,size;
  float dx,x0,f;
  char name[255],str[1000],*str1,c;
  int n,i,j;
  int nCol;
  FILE *stream;
  fpos_t beginning;
  char flagBinary;
  char flagLittleEndian;
  char flagNewFormat;
  
  /* 
   * Some basic tests on the stream 
   */
  if (s->mode != StreamRead) Errorf("ReadSigStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadSigStream() : You cannot read a signal to standard input"); 
  stream = s->stream;   
   
  /* 
   * Let's try to read the header 
   */
  flagHeader = NO;
  flagBinary = NO;
  flagXY = YES;
  flagNewFormat = NO;
  fgetpos(stream,&beginning);
  str1 = "LastWave Header";
  while (*str1 != '\0') {
    fscanf(stream,"%c",&c);
    if (c != *str1) break;
    str1++;
  }
  
  if (*str1 == '\0') {
    fscanf(stream,"%c",&c);
	flagHeader = YES;
    n = 0;
	n += fscanf(stream,"size %d ",&size);
	n += fscanf(stream,"name %[^\n] ",name);
	n += fscanf(stream,"firstp %d ",&firstp);
	n += fscanf(stream,"lastp %d ",&lastp);
		
	if (n != 4) Errorf("ReadSigStream() : Error in the header of the file");

    /* Extended header (Signal Package, version 1.5) */
	n += fscanf(stream,"binary %s ",&str);
    if (n == 5) {
      n--;
      flagNewFormat = YES;
      if (!strcmp(str, "no")) flagBinary = NO;
      else {
        flagBinary = YES;
        if (!strcmp(str, "little")) flagLittleEndian = YES;
        else if (!strcmp(str, "big")) flagLittleEndian = NO;
        else Errorf("ReadSigStream() : Bad field value '%s' of field 'binary'",str);
      } 
    }
    else flagBinary = NO;
    /* End of extended header */
    	
	fscanf(stream,"%[^\n] ",str);
	if (strcmp(str,"End of Header")) { /* it is a YSIG */
	  flagXY = NO;
#ifdef NUMDOUBLE
	  n += sscanf(str,"dx %lf",&dx);
	  n += fscanf(stream,"x0 %lf ",&x0);
#else
	  n += sscanf(str,"dx %f",&dx);
	  n += fscanf(stream,"x0 %f ",&x0);
#endif
	  if (n != 6) Errorf("ReadSigStream() : Error in the header of the file");
	  fscanf(stream,"%[^\n] ",str);
	  if (strcmp(str,"End of Header")) Errorf("ReadSigStream() : Error in the header of the file");
	}
	fgetpos(stream,&beginning);
  }
  fsetpos(stream,&beginning);

   


/* CASE OF AN ASCII FILE */
if (flagBinary == NO) {

while (1) {
   
  /* 
   * How many columns on the first line ? 
   */
#ifdef NUMDOUBLE
  if (fscanf(stream,"%lf",&f) != 1) Errorf("ReadSigStream() : Bad format");
#else
  if (fscanf(stream,"%f",&f) != 1) Errorf("ReadSigStream() : Bad format");
#endif
  fsetpos(stream,&beginning);  
  
  fscanf(stream,"%[^\n] ",str);
  nCol = 0;
  str1 = str;
  while (*str1 != '\0' && (*str1 == ' ' || *str1 == '\t')) str1++;
  while (*str1 != '\0') {
    nCol++;
    while (*str1 != '\0' && *str1 != ' ' && *str1 != '\t') str1++;
    while (*str1 != '\0' && (*str1 == ' ' || *str1 == '\t')) str1++;
  }

  if (nCol == 0) {
    if (flagNewFormat) Errorf("ReadSigStream() : Cannot figure out the number of colums");
    break;
  }
       
  /* 
   * Possible errors and set xcol and ycol
   */  
  if (xcol > nCol) Errorf("ReadSigStream() : Only %d columns in file ==> x column number %d is incorrect",nCol,xcol);
  if (ycol > nCol) Errorf("ReadSigStream() : Only %d columns in file ==> y column number %d is incorrect",nCol,ycol);
  if (xcol > 0 && ycol <= 0) Errorf("ReadSigStream() : Sorry you cannot read just X");
  if (xcol <= 0) {
    if (xcol == -1) xcol = 0;
    else if (flagHeader) {
      if (flagXY) xcol = 1;
      else xcol = 0;
    }
    else if (nCol == 2) xcol = 1;
    else xcol = 0;
  }
  if (ycol <= 0) {
    if (flagHeader) {
      if (flagXY) ycol = 2;
      else ycol = 1;
    }
    else if (nCol == 2) ycol = 2;
    else ycol = 1;
  }
      		
  /* 
   * Compute the size of the signal if no header and size to read is not specified
   */
  if (flagHeader == NO && sizeToRead == -1) {
    fsetpos(stream,&beginning);
	size = 0;
	while(fscanf(stream,"%[^\n] ",str) != EOF) size++;
  }
        	
  /* 
   * Check firstIndex and sizeToRead 
   */
  if (firstIndex < 0) firstIndex = 0;
  if (sizeToRead == -1) size = size - firstIndex;
  else if (flagHeader == YES) {
 	if (firstIndex+sizeToRead > size) Errorf("ReadSigStream() : Either firstIndex or sizeToRead is too big");
	size = sizeToRead;
  }
  else size = sizeToRead;
	

  /* 
   * Allocation 
   */		
  if (xcol != 0) SizeSignal(signal,size,XYSIG);
  else SizeSignal(signal,size,YSIG);           	
		
  /* 
   * We skip everything up to firstIndex
   */
  fsetpos(stream,&beginning);
  for (i = 0;i<firstIndex;i++) {
	n = fscanf(stream,"%[^\n]\n",str);
	if (n!=1) {
	  if (flagNewFormat) Errorf("ReadSigStream() : Signal is too short while reading file");
	  break;
	}
  }
	
  /* 
   * Let's read !!
   */
  for(i= 0 ;i<size;i++) {
    
    /* Case both X and Y must be read */
    if (xcol != 0) {

      for (j = 1; j<MIN(xcol,ycol);j++) fscanf(stream,"%*s");

      n = 0;
#ifdef NUMDOUBLE
      if (xcol < ycol) n = fscanf(stream,"%lf",&(signal->X[i]));
      else if (xcol > ycol) n = fscanf(stream,"%lf",&(signal->Y[i]));
      else {
        n = fscanf(stream,"%lf%*[^\n] ",&(signal->Y[i]));
#else
      if (xcol < ycol) n = fscanf(stream,"%f",&(signal->X[i]));
      else if (xcol > ycol) n = fscanf(stream,"%f",&(signal->Y[i]));
      else {
        n = fscanf(stream,"%f%*[^\n] ",&(signal->Y[i]));
#endif
        signal->X[i] = signal->Y[i];
        n+=1;
      }
      if (xcol != ycol) {    
        j++;
        for (; j<MAX(xcol,ycol);j++) fscanf(stream,"%*s");
#ifdef NUMDOUBLE
        if (xcol < ycol) n += fscanf(stream,"%lf%*[^\n] ",&(signal->Y[i]));
        else n += fscanf(stream,"%lf%*[^\n] ",&(signal->X[i]));
#else
        if (xcol < ycol) n += fscanf(stream,"%f%*[^\n] ",&(signal->Y[i]));
        else n += fscanf(stream,"%f%*[^\n] ",&(signal->X[i]));
#endif
      }  
      if (n != 2) {
        if (flagNewFormat) Errorf("ReadSigStream() : Error while reading file"); 
        break;
      }
	}
	
	/* Case just Y must be read */
	else {
      for (j = 1; j<ycol;j++) fscanf(stream,"%*s");
#ifdef NUMDOUBLE
      n = fscanf(stream,"%lf%*[^\n] ",&(signal->Y[i]));
#else
      n = fscanf(stream,"%f%*[^\n] ",&(signal->Y[i]));
#endif
      if (n != 1) {
        if (flagNewFormat) Errorf("ReadSigStream() : Error while reading file"); 
        break;
      } 
    } 
  }
	
  if (i != size) break;	

  if (flagNewFormat == NO && flagHeader) 
     Warningf("ReadSigStream() : (old Signal Package 1.1.2 header) You should update the format (writing the file again using 'write')");
	
	
  /* Some initialization */
  signal->size = size;
  if (flagHeader) {
    if (sizeToRead == -1){
      signal->lastp = lastp;
      signal->firstp = firstp;
    }
    if (flagXY == NO) {
      signal->dx = dx;
	  signal->x0 = x0 + dx*firstIndex;
    }
    if (strcmp(name,"none")) SetNameSignal(signal,name);
    else SetNameSignal(signal,NULL);
  }
  else	{
	signal->x0 = firstIndex;
  }
	
	
  /* We might need to sort the signal */
  if (xcol != 0) SortSig(signal);
  
  return;
  }
}

  /* 
   * If there was an error assuming data is ascii coded  maybe it is because the header is a Signal Package 1.1.2 header
   * and we are currently dealing with binary coded values without knowing it !
   */
  if (flagBinary == NO) {
    if (IsCPULittleEndian) {
      flagLittleEndian = YES;
      Warningf("ReadSigStream() : (old Signal Package 1.1.2 header) You should update the format (writing the file again using 'write'). I am assuming the values are binary coded (little endian).");
    }
    else {
      flagLittleEndian = NO;
      Warningf("ReadSigStream() : (old Signal Package 1.1.2 header) You should update the format (writing the file again using 'write'). I am assuming the values are binary coded (big endian).");
    }
    flagBinary = YES;
    fsetpos(stream,&beginning);
  }

  
/* CASE OF A BINARY FILE  (there should be a header, otherwise you should use the ReadSigRawStream function) */

  if (flagHeader == NO) Errorf("ReadSigStream() : It looks like the stream contains binary coded values with no header. I can't read that");
   
  /* 
   * Check firstIndex and sizeToRead 
   */
  if (firstIndex < 0) firstIndex = 0;
  if (sizeToRead == -1) sizeToRead = size - firstIndex;
  if (firstIndex+sizeToRead > size) Errorf("ReadSigStream() : Either firstIndex or sizeToRead is too big");
   
  /* Case XY */  
  if (flagXY) {
    SizeSignal(signal,sizeToRead,XYSIG);
    if (fseek(stream,firstIndex*sizeof(float),SEEK_CUR) != 0)     
      Errorf("ReadSigStream() : firstIndex seems to be too large for this file");
    if (fread(signal->X,sizeof(float),sizeToRead,stream) != sizeToRead)
      Errorf("ReadSigStream() : sizeToRead seems to be too large for this file");
    fseek(stream,size-sizeToRead-firstIndex,SEEK_CUR);
    fseek(stream,firstIndex*sizeof(float),SEEK_CUR);
    if (fread(signal->Y,sizeof(float),sizeToRead,stream) != sizeToRead)
      Errorf("ReadSigStream() : sizeToRead seems to be too large for this file");
    /* Go to the end of the signal */
    fseek(stream,size-sizeToRead-firstIndex,SEEK_CUR);
  }
  else {
    SizeSignal(signal,sizeToRead,YSIG);
    if (fseek(stream,firstIndex*sizeof(float),SEEK_CUR) != 0)     
       Errorf("ReadSigStream() : firstIndex seems to be too large for this file"); 
    if (fread(signal->Y,sizeof(float),sizeToRead,stream) != sizeToRead)
       Errorf("ReadSigStream() : sizeToRead seems to be too large for this file");
    /* Go to the end of the signal */
    fseek(stream,size-sizeToRead-firstIndex,SEEK_CUR);
  }
 
  /* Big/Little conversions */
  if ((flagLittleEndian && IsCPUBigEndian) || (!flagLittleEndian && IsCPULittleEndian)) {
    BigLittleValues(signal->Y,signal->size,sizeof(float));
    if (signal->type == XYSIG) BigLittleValues(signal->X,signal->size,sizeof(float));
  }
    
  /* Some settings */              
  if (strcmp(name,"none")) SetNameSignal(signal,name);
  else SetNameSignal(signal,NULL);
  if (flagXY == NO) {
    signal->dx = dx;
    signal->x0 = x0;
  }
  signal->firstp = firstp;
  signal->lastp = lastp;
}


/* Same as above but with a filename instead of a stream */
void ReadSigFile(SIGNAL signal,char *filename,  int firstIndex,int sizeToRead, int xcol,int ycol)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"r");
  if (stream == NULL) Errorf("ReadSigFile() : Error while opening the file %s",filename);

  ReadSigStream(signal,stream,firstIndex,sizeToRead, xcol,ycol);
	
  CloseStream(stream);	
  
  if (!strcmp(signal->name,"")) SetNameSignal(signal,filename);
}


/*
 * Function to read from a stream  a signal which values are coded using binary values
 *
 * signal      : the signal to read
 * stream      : the file stream
 * firstIndex  : first index in the file to be read (starting from 0)
 * sizeToRead  : the number of points to be read
 */
 
void ReadBinSigStream(SIGNAL signal,STREAM s,int firstIndex,int sizeToRead)
{
  int n;
  int firstp,lastp,size;
  float dx,x0;
  long int pos,pos1;
  int flagHeader,flagXY;
  char name[50];
  FILE *stream;
  
  Warningf("ReadBinSigStream() : You should no longer use this C function. It will be suppressed in the next package version. Please use the new 'ReadSigStream' or 'ReadSigFile'functions.");

  /* Some basic tests on the stream */
  if (s->mode != StreamRead) Errorf("ReadBinSigStream() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadBinSigStream() : You cannot write a signal to standard input"); 
  stream = s->stream; 

  /* Some basic checkings */
  if (firstIndex < 0) 
    Errorf("ReadBinSigStream() : firstIndex should strictly positive !");
  if (sizeToRead != -1 && sizeToRead <= 0) 
    Errorf("ReadBinSigStream() : sizeToRead should be strictly positive !");
    
  /* If there is a header just read it ! */
  flagHeader = NO;
  flagXY = NO;
  n = 0;
  n += fscanf(stream,"LastWave Header\nsize %d ",&size);
  if (n == 1) {
    flagHeader = YES;
	n += fscanf(stream,"name %[^\n] ",name);
    n += fscanf(stream,"firstp %d ",&firstp);
    n += fscanf(stream,"lastp %d ",&lastp);
    if (n != 4) Errorf("ReadBinSigStream() : Error in the header of a binary file signal");
#ifdef NUMDOUBLE
    n += fscanf(stream,"dx %lf ",&dx);
#else
    n += fscanf(stream,"dx %f ",&dx);
#endif
    if (n != 5) flagXY = YES;
    else {
#ifdef NUMDOUBLE
      n += fscanf(stream,"x0 %lf ",&x0);
#else
      n += fscanf(stream,"x0 %f ",&x0);
#endif
      if (n != 6) Errorf("ReadBinSigStream() : Error in the header of a binary file signal");
    } 
  	fscanf(stream,"End of Header ");
    
    /* Some basic checkings */
    if (firstIndex >= size) Errorf("ReadBinSigStream() : firstIndex is too large (>=%d)",size);
    if (sizeToRead != -1 && firstIndex+sizeToRead > size) 
      Errorf("ReadBinSigStream() : sizeToRead is too large (>%d)",size-firstIndex);
    if (sizeToRead == -1) sizeToRead = size-firstIndex;
  }
  else fseek(stream,0,SEEK_SET);
  
  /* Case XY */  
  if (flagXY) {
    SizeSignal(signal,sizeToRead,XYSIG);
    if (fseek(stream,firstIndex*sizeof(float),SEEK_CUR) != 0)     
      Errorf("ReadBinSigStream() : firstIndex seems to be too large for this file");
    if (fread(signal->X,sizeof(float),sizeToRead,stream) != sizeToRead)
      Errorf("ReadBinSigStream() : sizeToRead seems to be too large for this file");
    fseek(stream,size-sizeToRead-firstIndex,SEEK_CUR);
    fseek(stream,firstIndex*sizeof(float),SEEK_CUR);
    if (fread(signal->Y,sizeof(float),sizeToRead,stream) != sizeToRead)
      Errorf("ReadBinSigStream() : sizeToRead seems to be too large for this file");
 }
 else {
   if (!flagHeader) {
     pos = ftell(stream);
     fseek(stream,0,SEEK_END);
     pos1 = ftell(stream);
     sizeToRead = (pos1-pos+1)/sizeof(float);
     fseek(stream,pos-pos1,SEEK_CUR);
   }
   SizeSignal(signal,sizeToRead,YSIG);
     if (fseek(stream,firstIndex*sizeof(float),SEEK_CUR) != 0)     
       Errorf("ReadBinSigStream() : firstIndex seems to be too large for this file"); 
     if (fread(signal->Y,sizeof(float),sizeToRead,stream) != sizeToRead)
       Errorf("ReadBinSigStream() : sizeToRead seems to be too large for this file");
 }
 
 /* Some settings */              
 if (flagHeader && strcmp(name,"none")) SetNameSignal(signal,name);
 else SetNameSignal(signal,NULL);
 if (flagHeader) {
   if (flagXY == NO) {
     signal->dx = dx;
     signal->x0 = x0;
   }
   signal->firstp = firstp;
   signal->lastp = lastp;
  }
}   

/* Same as above but with a filename instead of a stream */ 
void ReadBinSigFile(SIGNAL signal,char *filename,int firstIndex,int sizeToRead)
{
  STREAM stream;

  /* Open file */
  stream = OpenFileStream(filename,"r");
  if (stream == NULL) Errorf("ReadBinSig() : Error while opening the file %s",filename);

  /* read */
  ReadBinSigStream(signal,stream,firstIndex,sizeToRead);
    
  /* Close */
  CloseStream(stream);
  
  if (!strcmp(signal->name,"")) SetNameSignal(signal,filename);
}


/*
 * The corresponding commands 
 */
 
void C_Read(char **argv)
{
	SIGNAL signal;
	char *filename,*mode;
    int xcol,ycol;
	char flagBinary,flagHeader,flagRaw,binaryCoding;
	int firstIndex,sizeToRead;
    char opt;
    STREAM stream;
    
	argv = ParseArgv(argv,tSIGNAL,&signal,-1);
	
    argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
    if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
    else filename = NULL;
      
    argv = ParseArgv(argv,tINT_,-2,&xcol,tINT_,-2,&ycol,-1);
	
	if (xcol == -2 && ycol == -2) xcol = ycol = 0;
	else if (ycol == -2) {
	  ycol = xcol;
	  xcol = -1;
	}
	else if (xcol < 0 || ycol < 0) Errorf("Bad <xcol> or <ycol> value");
	
	firstIndex = 0;
	sizeToRead = -1;
	
    /* options */
    flagBinary = NO;
    flagHeader = YES;
    flagRaw = NO;
    while(opt = ParseOption(&argv)) { 
    switch(opt) {
    case 'b': flagBinary = YES; break;
    case 'f':
      argv = ParseArgv(argv,tINT,&firstIndex,-1);
      if (firstIndex < 0) ErrorUsage1(); 
      break;
    case 's': 
      argv = ParseArgv(argv,tINT,&sizeToRead,-1);
      if (sizeToRead <= 0) ErrorUsage1(); 
      break;
    case 'r':
      argv = ParseArgv(argv,tSTR_,"",&mode,-1);
      if (!strcmp(mode,"little")) binaryCoding = BinaryLittleEndian;
      else if (!strcmp(mode,"big")) binaryCoding = BinaryBigEndian;
      else if (!strcmp(mode,"")) binaryCoding = 0;
      else Errorf("Bad binary mode '%s'",mode);
      flagRaw = YES;
      break;
    default: ErrorOption(opt);
    }
   }    
   NoMoreArgs(argv);
  
  if (flagRaw) {
    if (stream == NULL) ReadSigRawFile(signal,filename,firstIndex,sizeToRead,binaryCoding);
    else ReadSigRawStream(signal,stream,firstIndex,sizeToRead,binaryCoding);
    return;
  }
    
  if (flagBinary == NO) {
    if (stream == NULL) ReadSigFile(signal,filename,firstIndex,sizeToRead,xcol,ycol);
    else ReadSigStream(signal,stream,firstIndex,sizeToRead,xcol,ycol);
  }
  else {
    if (stream == NULL) ReadBinSigFile(signal,filename,firstIndex,sizeToRead);
    else ReadBinSigStream(signal,stream,firstIndex,sizeToRead);
  }
}
	 
void C_ReadInfo(char **argv)
{
	struct signal siginfo;
	char *filename,header,flagBinary,binaryCoding;
	int nCols;
	char flagPrint;
    char opt;
    STREAM stream;
    LISTV lv,lv1;
    	
    argv = ParseArgv(argv,tSTREAM_,NULL,&stream,-1);
  
    if (stream == NULL) argv = ParseArgv(argv,tSTR,&filename,-1);
    else filename = NULL;
      
    
    /* options */
    flagPrint = NO;
    while(opt = ParseOption(&argv)) { 
    switch(opt) {
    case 'p':
      flagPrint = YES;
      break;
    default: ErrorOption(opt);
    }
   }    
   NoMoreArgs(argv);

   if (filename) {
     if (ReadInfoSigFile(filename, &siginfo, &header,&flagBinary, &binaryCoding, &nCols)==NO) {
       if (flagPrint) Printf("Filename '%s' cannot be read using the 'read' command\n",filename);
       return;
     }
   }
   else {
     if (ReadInfoSigStream(stream, &siginfo, &header,&flagBinary, &binaryCoding, &nCols)==NO) {
       if (flagPrint) Printf("Filename '%s' cannot be read using the 'read' command\n",filename);
       return;
     }
   }  
   
   if (flagPrint) {
     if (filename) {
       if (header) Printf("Filename '%s' has a LastWave header\n",filename);
       else Printf("Filename '%s' has no header\n",filename);
     }
     else {
       if (header) Printf("Stream has a LastWave header\n");
       else Printf("Stream has no header\n");
     }
     if (header) {
       if (siginfo.type == YSIG) {
         if (flagBinary == YES) {
           if (binaryCoding == BinaryLittleEndian) Printf("YSIG (Binary coded, little endian)\n");
           else Printf("YSIG (Binary coded, big endian)\n");
         }
         else  Printf("YSIG (ascii coded)\n");
         Printf("size   : %d\n",siginfo.size);
#ifdef NUMDOUBLE         
         Printf("x0     : %.16g\n",siginfo.x0);
         Printf("dx     : %.16g\n",siginfo.dx);
#else
         Printf("x0     : %.8g\n",siginfo.x0);
         Printf("dx     : %.8g\n",siginfo.dx);
#endif
         Printf("firstp : %d\n",siginfo.firstp);
         Printf("lastp  : %d\n",siginfo.lastp);
       }
       else {
         if (flagBinary == YES) {
           if (binaryCoding == BinaryLittleEndian) Printf("XYSIG (Binary coded, little endian)\n");
           else Printf("XYSIG (Binary coded, big endian)\n");
         }
         else  Printf("XYSIG (ascii coded)\n");
         Printf("size   : %d\n",siginfo.size);
         Printf("firstp : %d\n",siginfo.firstp);
         Printf("lastp  : %d\n",siginfo.lastp);
       }
     }
     else {
       Printf("%d Columns (ascii coded)\n",nCols);
       Printf("size   : %d\n",siginfo.size);
     }
   }
   
   else {
     lv = TNewListv();
     SetResultValue(lv);
     if (header) {
       if (siginfo.type == YSIG) {
         AppendInt2Listv(lv,1);
         if (flagBinary == YES) {
           if (binaryCoding==BinaryLittleEndian) AppendStr2Listv(lv,"binary little");
           else AppendStr2Listv(lv,"binary big");
         }
         else {
          AppendStr2Listv(lv,"ascii");
        }
        AppendInt2Listv(lv,siginfo.size);
        lv1 = TNewListv();
        AppendValue2Listv(lv,(VALUE) lv1);
        AppendStr2Listv(lv1,"y");
        AppendFloat2Listv(lv1,siginfo.x0);
        AppendFloat2Listv(lv1,siginfo.dx);
        lv1 = TNewListv();
        AppendValue2Listv(lv,(VALUE) lv1);
        AppendFloat2Listv(lv1,siginfo.firstp);
        AppendFloat2Listv(lv1,siginfo.lastp);
      }
      else {
        if (flagBinary == YES) {
          AppendInt2Listv(lv,1);
           if (binaryCoding==BinaryLittleEndian) AppendStr2Listv(lv,"binary little");
           else AppendStr2Listv(lv,"binary big");
         }
         else {
          AppendStr2Listv(lv,"ascii");
        }
        AppendInt2Listv(lv,siginfo.size);
        AppendStr2Listv(lv,"xy");
        lv1 = TNewListv();
        AppendValue2Listv(lv,(VALUE) lv1);
        AppendFloat2Listv(lv1,siginfo.firstp);
        AppendFloat2Listv(lv1,siginfo.lastp);
      }
    }
    else {      
      AppendInt2Listv(lv,0);
      AppendInt2Listv(lv,siginfo.size);
    }
  } 
}
	
		
		
	
	
	
	
		
