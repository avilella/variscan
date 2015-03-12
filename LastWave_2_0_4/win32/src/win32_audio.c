/*..........................................................................*/

/*                                                                          */

/*      L a s t W a v e   K e r n e l   1 . 6 . 1                           */

/*                                                                          */

/*      Copyright (C) 2001 Remi Gribonval, Emmanuel Bacry.                  */

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





/********************************************************************/

/*                                                                  */

/*   win32_audio.c :                                                */

/*                                                                  */

/********************************************************************/



#include "computer.h"



#ifdef LASTWAVE_WIN32_AUDIO



#include <windows.h>

#include <mmsystem.h>

#include "soundlw.h"





//#include "lastwave.h"

// We do not include "lastwave.h", in order to avoid a conflict between 

// Windows's definition of 'PROC' and 'Beep' and LastWave's own one

// However we need to redefine the 'NO' constant, the 'MAX' macro and include some header files

#ifndef NO

#define NO 0

#endif

/* Max of two numbers */

#ifndef MAX

#define MAX(x,y)  ((x) > (y) ? (x) : (y))

#endif

// to get ULONG_MAX

#ifndef ULONG_MAX

#include <limits.h>

#endif

extern void Errorf(char *format,...);

extern void Warningf(char *format,...);

extern void Printf(char *format,...);

extern void Flush(void);

extern char* CharAlloc(int size);

extern float *FloatAlloc(int size);

extern void Free(void * ptr);



#include "xx_audio.h"







/*

 * Dealing With sounds

 */



/* Function to play a sound. */

// Currently we play a sound by writing it to a wav file and 

// play it 'asynchronously'. The help of the PlaySound function says :

//   The sound is played asynchronously and PlaySound returns immediately 

//   after beginning the sound. To terminate an asynchronously played 

//   waveform sound, call PlaySound with pszSound set to NULL. 

void XXSoundPlay(short *samplesLeft,short *samplesRight,unsigned long nbSamples,float sampleFreq)

{

  char *tmpFile = "./LastWaveTmpSndFile.wav";

  char *format  = "wave16";

  SoundFormat soundFormat;



  // Initialize the sound format;

  if (Name2SoundFormat(format,&soundFormat) == NO)

    Errorf("XXSoundPlay : (Weired Internal Error) invalid format '%s'",format);



  SoundWriteSamples(tmpFile,samplesLeft,samplesRight,nbSamples,sampleFreq,&soundFormat);

  PlaySound(tmpFile, NULL, SND_ASYNC); 

}





/* Function to stop playing a sound. */

void XXSoundStopPlaying(void)

{

  PlaySound(NULL,NULL,SND_PURGE); 

}



// REMI : these limitations shall be removed if we implement

// bufferized interactive recording (which can be stopped by the user)

// Limit the recording to 128 Mbytes

#define RECORD_MEMORY_LIMIT    1l<<27

// Limit recording to 3 minutes

#define RECORD_DURATION_LIMIT  180



/* Function to record a sound */

void XXSoundRecord(float **samplesLeft,float **samplesRight,

		   unsigned long *nbSamples,float *sampleFreq,

		   unsigned char soundQuality,

		   unsigned long customSampleFreq,unsigned char customBitsPerSample,

		   unsigned long maxNbSamples)

{

  MMRESULT     mmresult;

  WAVEFORMATEX waveFormat;

  HWAVEIN      waveInDevice;

  WAVEINCAPS   waveInCaps;

  WAVEHDR      waveHeader;

  unsigned long waveHeaderBufferSize  = 0;

  unsigned long i,j;

  // For conversion purposes

  short *shortBuffer;

  float  maxShort = 32768.0;



  // for the waiting clock

  char niter = 0;



  // In case there is a problem later on !!!

  *nbSamples  = 0;

  *sampleFreq = 1.0;

  

  // Interactive recording is not implemented so far !!!

  if(maxNbSamples ==0) {

    Warningf("XXSoundRecord : maxNbSamples=0 not implemented\n HINT : you should call 'sound record' with <duration> > 0"); 

    Errorf("XXSoundRecord : interactive recording not implemented for Windows computers");

  }



  // DEBUG INFO :

  //  Printf("maxNbSamples = %ld\n",maxNbSamples);



  // Select the right recording format : mono/stereo, 44100/8000 Hz, etc.

  waveFormat.wFormatTag = WAVE_FORMAT_PCM;

  if(samplesRight==NULL)

    waveFormat.nChannels = 1;

  else

    waveFormat.nChannels = 2;

  switch(soundQuality) {

    case cdSoundQuality : 

      waveFormat.nSamplesPerSec  = 44100;

      waveFormat.wBitsPerSample  = 16;

      break;

    case voiceSoundQuality : 

      waveFormat.nSamplesPerSec  = 22050;

      // NOTA : we do more than the required 8 bits, because it is simpler to implement ...

      waveFormat.wBitsPerSample  = 16; 

      break;

    case phoneSoundQuality : 

      waveFormat.nSamplesPerSec  = 8000;

      // NOTA : we do more than the required 8 bits, because it is simpler to implement ...

      waveFormat.wBitsPerSample  = 16; 

      break;

    case customSoundQuality : 

      waveFormat.nSamplesPerSec  = customSampleFreq;

      // NOTA : we do more than the required 8 bits, because it is simpler to implement ...

      waveFormat.wBitsPerSample  = MAX(16,customBitsPerSample);

      break;

    default : 

      Errorf("XXSoundRecord() : sound quality %d not implemented on Windows computers",soundQuality);

  }



  // Set the output Sampling Frequency

  *sampleFreq = waveFormat.nSamplesPerSec;



  // See the help of WAVEFORMATEX from Microsoft for the meaning of these fields !!!!

  waveFormat.nBlockAlign     = (waveFormat.nChannels)*(waveFormat.wBitsPerSample)/8; 

  waveFormat.nAvgBytesPerSec = (waveFormat.nSamplesPerSec)*(waveFormat.nBlockAlign);

  waveFormat.cbSize          = 0;

  

  // DEBUG info

  //Printf("There is/are %d sound input device(s) available\n",waveInGetNumDevs());

  

  // Open the default sound input device that supports the required sound recording quality

  mmresult = waveInOpen(&waveInDevice,WAVE_MAPPER,&waveFormat,0,0,CALLBACK_NULL);

  if(mmresult!=MMSYSERR_NOERROR) {

    Warningf("XXSoundRecord :  May be the sound quality you asked for is not available\n It should only happen with the 'custom' sound quality, in particular with strange bits/sample settings ...");  

    Errorf("XXSoundRecord :  Sorry, I have a problem opening your sound input device.");

  }



  // Get and print information on device capabilities ....

  //  mmresult = waveInGetDevCaps(waveInDevice,&waveInCaps,sizeof(WAVEINCAPS));

  mmresult = waveInGetDevCaps((UINT)waveInDevice,&waveInCaps,sizeof(WAVEINCAPS));



  if(mmresult!=MMSYSERR_NOERROR) Errorf("XXSoundRecord :  Sorry, I have a problem getting your sound input device capabilities");  



  Printf("Recording from: '%s', driver version %d.%d (manufacturer id=%d, product id=%d)\n",

	 waveInCaps.szPname,

	 (char)((waveInCaps.vDriverVersion)>>8),

	 (char)((waveInCaps.vDriverVersion)),

	 (int)(waveInCaps.wMid),(int)(waveInCaps.wPid));



  // Prepare the waveHeader

  // Set size of the buffer and allocate it

  

  // Intrinsic limit to 'maxNbSamples' :

  // 'waveHeaderBufferSize' should always be smaller than the largest 'unsigned long'

  if(maxNbSamples >= ULONG_MAX/(waveFormat.nChannels*waveFormat.wBitsPerSample/8))

    Errorf("XXSoundRecord : due to the limited range of the 'unsigned long' type on this Windows machine, the number of samples that can be recorded at once ('maxNbSamples') is limited to %ld",ULONG_MAX/(waveFormat.nChannels*waveFormat.wBitsPerSample/8));



  waveHeaderBufferSize = maxNbSamples*waveFormat.nChannels*(waveFormat.wBitsPerSample/8);

  // REASONABLE limit in terms of MEMORY use

  if(waveHeaderBufferSize > RECORD_MEMORY_LIMIT)

    Errorf("XXSoundRecord : recording is limited to %d Mbytes on Windows computers",RECORD_MEMORY_LIMIT>>20);

  // REASONABLE limit in terms of DURATION of the recording

  if(maxNbSamples/waveFormat.nSamplesPerSec > RECORD_DURATION_LIMIT)

    Errorf("XXSoundRecord : recording is limited to %d seconds on Windows computers",RECORD_DURATION_LIMIT);



  // DEBUG INFO 

  //Printf("MAX NB SAMPLE (MONO 8bits): %uld     (STEREO 16 bits): %ld\n",ULONG_MAX,ULONG_MAX/(2*2));

  //Printf("MAX DURATION  (8khz)      : %ld sec (44100khz)      : %ld sec\n",ULONG_MAX/8000,ULONG_MAX/(2*2*44100));



  waveHeader.lpData          =  CharAlloc(waveHeaderBufferSize);

  if(waveHeader.lpData == NULL)

    Errorf("XXSoundRecord : Mem. Alloc. failed for buffer");

  waveHeader.dwBufferLength  =  waveHeaderBufferSize;

  waveHeader.dwBytesRecorded = 0;

  waveHeader.dwFlags         = 0;

  mmresult = waveInPrepareHeader(waveInDevice,&waveHeader,sizeof(WAVEHDR));

  if(mmresult!=MMSYSERR_NOERROR) Errorf("XXSoundRecord :  Sorry, I have a problem preparing the buffer");  



  // DEBUG 

  //  Printf("bufferSize %ld bufferLength %ld\n",waveHeaderBufferSize,waveHeader.dwBufferLength);



  // Specify in which buffer the input device must write the recorded data

  mmresult = waveInAddBuffer(waveInDevice,&waveHeader,sizeof(WAVEHDR));

  if(mmresult!=MMSYSERR_NOERROR) Errorf("XXSoundRecord :  Sorry, I have a problem sending the buffer to your sound input device");  



  // Fills the buffer

  mmresult = waveInStart(waveInDevice);

  if(mmresult!=MMSYSERR_NOERROR) Errorf("XXSoundRecord :  Sorry, I have a problem filling the buffer from your sound input device");  



  // Wait until the buffer is done, and print a clock to make the user wait

  Printf("\\");Flush();

  while(!(waveHeader.dwFlags & WHDR_DONE)) { 

    switch(niter%4) {

    case 0 : Printf("\b|");Flush();break;

    case 1 : Printf("\b/");Flush();break;

    case 2 : Printf("\b-");Flush();break;

    case 3 : Printf("\b\\");Flush();break;

    }

    niter++;

  }

  Printf("\b");Flush();



  // Convert the number of Bytes recorded in a number of samples

  *nbSamples   = (waveHeader.dwBytesRecorded)/((waveFormat.nChannels)*(waveFormat.wBitsPerSample/8));

  // Some info

  Printf("Recorded %d Samples (%d Bytes)\n", *nbSamples,waveHeader.dwBytesRecorded);



  // Allocate samplesLeft/Right 

  *samplesLeft    = FloatAlloc(*nbSamples);

  if(samplesRight) {

    *samplesRight = FloatAlloc(*nbSamples);

  }

  // Copy the data

  shortBuffer = (short*) waveHeader.lpData;

  for(i = 0,j = 0; i<*nbSamples; i++,j+=waveFormat.nChannels) {

    (*samplesLeft)[i] = ((float)shortBuffer[j])/maxShort;

  }

  if(samplesRight) {

    for(i = 0,j = 1; i<*nbSamples; i++,j+=waveFormat.nChannels) {

      (*samplesRight)[i] = ((float)shortBuffer[j])/maxShort;

    }

  }



  // Unprepare header

  waveInUnprepareHeader(waveInDevice,&waveHeader,sizeof(WAVEHDR));



  // Close everything

  mmresult = waveInReset(waveInDevice);

  if(mmresult!=MMSYSERR_NOERROR) Errorf("XXSoundRecord :  Sorry, I have a problem resetting your sound input device");  



  mmresult = waveInClose(waveInDevice);

  if(mmresult==WAVERR_STILLPLAYING) Errorf("XXSoundRecord :  There are still buffers in the queue");  

  if(mmresult!=MMSYSERR_NOERROR) Errorf("XXSoundRecord :  Sorry, I have a problem closing the sound input device");  



  // Free the buffer

  Free(waveHeader.lpData);

}



/* Function to stop recording a sound. */

void XXSoundStopRecording(void)

{

  Warningf("XXSoundStopRecording : not implemented for Windows computers");

}







#endif //LASTWAVE_WIN32_AUDIO





// EOF

