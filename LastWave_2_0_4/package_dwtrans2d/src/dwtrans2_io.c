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




/****************************************************************************/
/*                                                                          */
/*    wtrans2_IO.c       write/read a wtrans2                               */
/*                                                                          */
/****************************************************************************/

#include "lastwave.h"
#include "dwtrans2d.h"
  
/****************************************************
 *
 *         Read a wtrans2                           
 *
 ****************************************************/

void  ReadWtrans2(WTRANS2 wtrans, STREAM s)
{
  int noct,norient,o,v,periodic;
  int n=0;
  char wName[20];
  FILE *stream;
 /* Some basic tests on the stream */
  if (s->mode != StreamRead) Errorf("ReadWtrans2() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("ReadWtrans2() : You cannot write a wtrans2 to standard input"); 
  stream = s->stream; 

 /* Let's read the header */
  n = 0;
  n += fscanf(stream,"Wavelet name : %s ",wName);
  n += fscanf(stream,"Number octaves %d ",&noct);
  n += fscanf(stream,"Number orientations %d ",&norient);
  n += fscanf(stream,"Periodic %d ",&periodic);
  if (n != 4) Errorf("Error in the header of the file");

  wtrans->noct = noct;
  wtrans->norient = norient;
  
  if (wtrans->wName != NULL) Free(wtrans->wName);
  wtrans->wName = CopyStr(wName);

  ReadImageStream(wtrans->images[0][0],s,YES,0,0,NO);
  for(o=1;o<=wtrans->noct;o++)
      for(v=0;v<wtrans->norient;v++) {
           ReadImageStream(wtrans->images[o][v],s,YES,0,0,NO);          
      }
}


  
void C_DW2Read(char ** argv)
{ 
  char *fname;
  WTRANS2 wtrans=NULL;
  STREAM s;

  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);
  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
  
 
  argv= ParseArgv(argv,tSTREAM_,NULL,&s,-1);
 
   if (s == NULL) {
       argv = ParseArgv(argv,tSTR,&fname,0);
       s =  OpenFileStream(fname,"r");
       if (s == NULL) Errorf("W2Read() : Error while opening the file %s",fname);

 
     if (!s->stream) 
          Errorf("unable to open %s\n",fname);
         
        ReadWtrans2(wtrans, s);
      
     
   }
 else {
           ReadWtrans2(wtrans, s);
     }
 
    CloseStream(s);	
} 


void  WriteWtrans2(WTRANS2 wtrans, STREAM s)
{
  int o,v;
  FILE *stream;
 /* Some basic tests on the stream */
  if (s->mode != StreamWrite) Errorf("WriteWtrans2() :The stream should be an input stream and not an output stream"); 
  if (s->stream == NULL) Errorf("WriteWtrans2() : You cannot write a wtrans2 to standard input"); 
  stream = s->stream; 

 /* Let's print the header */
   if (wtrans->wName) fprintf(stream,"Wavelet name : %s ",wtrans->wName);
   else fprintf(stream,"Wavelet name : NoName ");
   fprintf(stream,"Number octaves %d ",wtrans->noct);
   fprintf(stream,"Number orientations %d ",wtrans->norient);
   fprintf(stream,"Periodic %d ",wtrans->periodic);

  WriteImageStream(wtrans->images[0][0],s,NO,0,0,YES);

  for(o=1;o<=wtrans->noct;o++)
	for(v=0;v<wtrans->norient;v++) 
        WriteImageStream(wtrans->images[o][v],s,NO,0,0,YES);
}

   

/****************************/
/* write a wtrans2 to a file */
/****************************/

void C_DW2Write(char**argv)
     
{
  char *fname;
  STREAM s;
   WTRANS2 wtrans=NULL;
  
  argv = ParseArgv(argv,tWTRANS2_,NULL,&wtrans,-1);
  if (wtrans ==NULL) wtrans= GetWtrans2Cur();
  argv= ParseArgv(argv,tSTREAM_,NULL,&s,-1);
 
  
  if (s == NULL) {
       argv = ParseArgv(argv,tSTR,&fname,0);
          
       s =  OpenFileStream(fname,"w");
       if (s == NULL) Errorf("W2Write() : Error while opening the file %s",fname);

    if (!s->stream) 
          Errorf("unable to open %s\n",fname);
         CheckWtrans2(wtrans); 
        WriteWtrans2(wtrans, s);
      
      
  }
    else {  NoMoreArgs(argv);
             CheckWtrans2(wtrans);
           WriteWtrans2(wtrans, s);
     }

    CloseStream(s);	
    
} 
 
