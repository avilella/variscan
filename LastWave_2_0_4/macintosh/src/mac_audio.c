/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 . 4                           */
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


/********************************************************************/
/*                                                                  */
/*   mac_audio.c :                                                    */
/*                                                                  */
/********************************************************************/

#include "computer.h"

#ifdef LASTWAVE_MAC_AUDIO

#include "lastwave.h"
#include "xx_system.h"
#include "xx_audio.h"
#include "signals.h"


/*
 * Dealing With sounds
 */

void XXSoundRecord(float **samplesLeft,float **samplesRight, 
			  unsigned long *nbSamples,float *sampleFreq,
			  unsigned char soundQuality,
			  unsigned long customSampleFreq,unsigned char customBitsPerSample,
			  unsigned long maxNbSamples)
{
  Point pt = {30,30};
  Handle sndHandle;
  OSErr err;
  SoundHeader *soundHeader;
  ExtSoundHeader *extSoundHeader;
  short *s;
  unsigned char *c;
  int i,n;
  long offset;

  if (samplesRight) Errorf("Sorry, stereo audio not yet available on macintosh computers");

  /* Start recording */
  sndHandle = NULL;
  switch (soundQuality) {
    case cdSoundQuality : 
      err = SndRecord(NULL,pt,'cd  ',(SndListResource ***) &sndHandle);
      break;
    case voiceSoundQuality : 
      err = SndRecord(NULL,pt,'best',(SndListResource ***) &sndHandle);
      break;
    case phoneSoundQuality : 
      Errorf("'phoneSoundQuality' not available yet on macintosh computers"); 
    case customSoundQuality : 
      Errorf("'customSoundQuality' not available yet on macintosh computers"); 
    default : 
      Errorf("XXSoundRecord() : Bad sound quality");
  }
   
  /* If one changes his mind */ 
  if (err == userCanceledErr) {
    *nbSamples = 0;
    return;
  }  
  
  /* Did it work ? */  
  if (err != noErr) Errorf("XXSoundRecord() : Sorry, I have a problem with your sound input device");

 
  /* Get the sound Header */
  GetSoundHeaderOffset((SndListResource **) sndHandle,&offset);
  HLock(sndHandle);
  soundHeader = (SoundHeader *) (*sndHandle +offset);
  
  
  /* Case of extended Sound Header */
  if (soundHeader->encode == extSH) {
    extSoundHeader = (ExtSoundHeader *) (*sndHandle+offset);
    if (extSoundHeader->sampleSize == 8) {
      HUnlock(sndHandle);
      DisposeHandle(sndHandle);
      Errorf("XXSoundRecord() : Sorry, you should install Sound Manager 3.0");
    }
    *nbSamples = extSoundHeader->numFrames;
    *samplesLeft = FloatAlloc(*nbSamples);
    if (extSoundHeader->samplePtr) s = (short *) extSoundHeader->samplePtr;
    else s = (short *) extSoundHeader->sampleArea;
    n = extSoundHeader->numChannels;
    for (i=0;i<*nbSamples;i++) {
      (*samplesLeft)[i] = s[n*i]/32768.0;
    }
    *sampleFreq = (extSoundHeader->sampleRate & 0xFFFF0000)>>16;
    *sampleFreq += (extSoundHeader->sampleRate & 0xFFFF)/65536.0;
  }

  /* Case of regular Sound Header */
  else if (soundHeader->encode == stdSH) {
    *nbSamples = soundHeader->length;
    *samplesLeft = FloatAlloc(*nbSamples);
    if (soundHeader->samplePtr) c = (unsigned char *) soundHeader->samplePtr;
    else c = (unsigned char *) soundHeader->sampleArea;
    for (i=0;i<*nbSamples;i++) {
      (*samplesLeft)[i] = (c[i]-128)/255.0;
    }
    *sampleFreq = (soundHeader->sampleRate & 0xFFFF0000)>>16;
    *sampleFreq += (soundHeader->sampleRate & 0xFFFF)/65536.0;
  }
  
  /* Other cases */
  else {
    HUnlock(sndHandle);
    DisposeHandle(sndHandle);
    Errorf("XXSoundRecord() : Sorry, This type of sound recording is not supported by LastWave");
  }
  
  HUnlock(sndHandle);
  DisposeHandle(sndHandle);
}


/* Function to play a sound. */

#define NMaxQueuedSounds 10

void XXSoundPlay1(short *samples,int nbSamples,float sampleFreq)
{
  static SndChannelPtr chan = NULL;  
  static int n = 0;
  ExtSoundHeader theSndHeaders[NMaxQueuedSounds];
  ExtSoundHeader *sndHeader;
  int i ,j;
  SndCommand theSndCmds[NMaxQueuedSounds];
  SndCommand *sndCmd;
  short *theSamples[NMaxQueuedSounds];
  short **s;
  
  if (sampleFreq < 8192) {
    Warningf("XXSoundPlay() : Cannot play sample frequency smaller than 8192 (I changed it)");
    sampleFreq = 8192;
  }
  
  if (chan == NULL) {
    if (SndNewChannel(&chan,sampledSynth,initMono,NULL) != 0) Errorf("XXSoundPlay() : Error while channel allocation");
  }
  
  sndHeader = theSndHeaders+n;
  sndCmd = theSndCmds+n;
  s = theSamples+n;
  n = (n+1)%NMaxQueuedSounds;
  *s = (short *) CharAlloc(nbSamples*sizeof(short));
  memcpy(*s,samples,nbSamples*sizeof(short));
  
  sndHeader->samplePtr = (char *) *s;
  sndHeader->numChannels = 1;
  i = (int) sampleFreq;
  j = (int) ((i-sampleFreq)*65536);
  sndHeader->sampleRate = i<<16 + j; 
  sndHeader->loopStart = 0;
  sndHeader->loopEnd = 0;
  sndHeader->encode = extSH;
  sndHeader->numFrames = nbSamples;
  sndHeader->sampleSize = 16;
  
  sndCmd->cmd = bufferCmd;
  sndCmd->param1 = 0; 
  sndCmd->param2 = (long) sndHeader;
/*  SndDoCommand(chan,sndCmd,true); */
  SndDoImmediate(chan,sndCmd); 
  Free(*s);
}

// Play a sound on the default output device, at the given sampling frequency.
// The sound may be mono, in which case sampleRight must be NULL.
void XXSoundPlay(short *samplesLeft,short *samplesRight,unsigned long nSamples,float sampleFreq)
{
  SndListResource sndResource;
  ExtSoundHeader sndHeader;
  SndListResource *ptrSndResource;
  Handle sndHandle;
  int i ,j;

  if (samplesRight) Errorf("Sorry, stereo audio not yet available on macintosh computers");
    
  if (sampleFreq < 8192) {
    Warningf("XXSoundPlay() : Cannot play sample frequency smaller than 8192 (I changed it)");
    sampleFreq = 8192;
  }
  
  ptrSndResource = &sndResource;
  sndHandle = (Handle) &ptrSndResource;
    
  sndResource.format = 1;     
  sndResource.numModifiers = 1;
  sndResource.modifierPart[0].modNumber = sampledSynth;
  sndResource.modifierPart[0].modInit = 0;
  sndResource.numCommands = 1;
  sndResource.commandPart[0].cmd = bufferCmd;  
  sndResource.commandPart[0].param1 = 0;  
  sndResource.commandPart[0].param2 = (long) &sndHeader;  
  
  sndHeader.samplePtr = (char *) samplesLeft;
  sndHeader.numChannels = 1;
  i = (int) sampleFreq;
  j = (int) ((i-sampleFreq)*65536);
  sndHeader.sampleRate = i<<16 + j; 
  sndHeader.loopStart = 0;
  sndHeader.loopEnd = 0;
  sndHeader.encode = extSH;
  sndHeader.numFrames = nSamples;
  sndHeader.sampleSize = 16;


  SndPlay(NULL,(SndListResource **) sndHandle,false);
}

// Stop the currently playing sound
void XXSoundStopPlaying(void)
{
  Errorf("Sorry, stopping audio while playing not yet available on macintosh computers");
}


#endif