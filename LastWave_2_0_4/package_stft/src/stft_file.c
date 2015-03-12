/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval                                 */
/*      email  : remi.gribonval@inria.fr                                    */
/*               lastwave@cmapx.polytechnique.fr                            */
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
#include "stft.h"


/***************************/
/* Write a stft in a file. */
/***************************/

/*
 * Function to write a stft using only ascii format
 *   stft        : the stft to write
 *   stream      : the file stream
 *   flagHeader  : Do we want a header to be written ?
 */

void WriteStftStream(STFT stft,STREAM s,char flagHeader)
{
  int i,j;
  FILE *stream;
  
  /* Some basic tests on the stream */
  if (s->mode != StreamWrite) 
    Errorf("WriteStftStream() : The stream should be an output stream and not an input stream"); 
  if (s->stream == NULL) 
    Errorf("WriteStftStream() : You cannot write a stft to standard output"); 
  stream = s->stream; 
  /* Checking the stft */
  CheckStft(stft);

  /* Write header if asked */
  if (flagHeader == YES) {
    fprintf(stream,"LastWave Header\n");
    fprintf(stream,"size    %d\n",stft->signalSize);
    fprintf(stream,"firstp  %d\n",stft->firstp);
    fprintf(stream,"lastp   %d\n",stft->lastp);
    fprintf(stream,"dx      %.8g\n",stft->dx);
    fprintf(stream,"x0      %.8g\n",stft->x0);
    fprintf(stream,"type    %s\n",StftType2Name(stft->type));
    fprintf(stream,"border  %s\n",BorderType2Name(stft->borderType));
    fprintf(stream,"GABOR_MAX_FREQID %d\n",GABOR_MAX_FREQID);
    fprintf(stream,"windowSize %d\n",stft->windowSize);
    fprintf(stream,"window  %s\n",WindowShape2Name(stft->windowShape));
    fprintf(stream,"tRate   %d\n",stft->tRate);
    fprintf(stream,"nFrames %d\n",stft->nFrames);
    fprintf(stream,"fRate   %d\n",stft->fRate);
    fprintf(stream,"nSubBands %d\n",stft->nSubBands);
    fprintf(stream,"End of Header\n");
  }

  /* We then write in the file */
  /* according to the type     */
  switch(stft->type) {
  case ComplexStft :
    for(i = 0; i < stft->nFrames; i++) {
      for(j = 0; j < stft->nSubBands; j++) {
	fprintf(stream,"%.8g   ",stft->real->Y[(i*stft->nSubBands)+j]);
      }
      fprintf(stream,"\n");
    }
    for(i = 0; i < stft->nFrames; i++) {
      for(j = 0; j < stft->nSubBands; j++) {
	fprintf(stream,"%.8g   ",stft->imag->Y[i*stft->nSubBands+j]);
      }
      fprintf(stream,"\n");
    }
    break;
  case RealStft :
  case HighResStft :
    for(i = 0; i < stft->nFrames; i++) {
      for(j = 0; j < stft->nSubBands; j++) {
	fprintf(stream,"%.8g   ",stft->real->Y[i*stft->nSubBands+j]);
      }
      fprintf(stream,"\n");
    }
    break;
  default :
    Errorf("WriteStftStream : unknown type %d",stft->type);
  }
}

/* 
 * Same as above but with a filename instead of a stream 
 */
void WriteStftFile(STFT stft,char *filename,int flagHeader)
{
  STREAM stream;
  /* Open file */
  stream = OpenFileStream(filename,"w");
  if (stream == NULL) 
    Errorf("WriteStftFile() : Error while opening the file %s",filename);
  WriteStftStream(stft,stream,flagHeader);
  CloseStream(stream);
}
	
/* EOF */	

