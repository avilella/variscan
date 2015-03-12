/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'sound' 2.0                        */
/*                                                                          */
/*      For the 'sndfile' library                                           */
/*      Copyright (C) 1999 Erik de Castro Lopo <erikd@zip.com.au>           */
/*                                                                          */
/*      For everything else                                                 */
/*      Copyright (C) 1999-2002 Emmanuel Bacry.                             */
/*      Copyright (C) 2002 Remi Gribonval.                                  */
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



#include "lastwave.h"
#include "xx_audio.h"
#include "sndfile.h"
#include "signals.h"
#include "soundlw.h"

/*
 * 
 * The sound formats are referred to by names such as 'aiff8' for coding AIFF using 8 bits.
 * This name is used as the default extension (if none is specified) for the sound filename.
 * Thus for instance a filename named 'sound.aiff8' will refer to a soundfile coded using AIFF 8 bits format.
 * There is a predefined list of 'BasicSoundFormat' corresponding to formats LastWave knows about.
 * The user does not access directly this structure but rather the 'soundFormat' structure.
 * Every 'BasicSoundFormat' can be seen as a 'SoundFormat'. However,
 * in order to be able to associate a given extension (such a '.wav') to a given basic format (such as 'waw16')
 * the user can define a new 'SoundFormat' that points to a 'BasicSoundFormat' and give it the name he wants. 
 * In the case of 'raw' data (for which there is no header), one must specify (when reading a file) the sampling
 * rate and the number of channels. This is done by using the corresponding 'raw' basic format name (such as raw16_le)
 * and add after it the string '_<samplingRate>_<nChannels>'. If not specified, the sampling rate is 1
 * and the number of channels is 1. Thus 'raw16_le_44100_1' is a valid sound format. 
 * The 'BasicSoundFormat' structure is used to store everything but the sampling rate and the number of channels.
 * These two parameters will be coded (for raw format only) in the 'SoundFormat' structure.
 *
 */


/*
 * The different basic sound formats available
 */
static BasicSoundFormat theBasicSoundFormats[] =
{
  "lw",SF_FORMAT_LW,0,"LastWave read/write file format : This is the format managed by the 'write' and 'read' LastWave commands (without the '-r' option). It uses float values.  When writing, it uses '-b' option (i.e., binary coded values).",
  
  "aiff8",SF_FORMAT_AIFF+SF_FORMAT_PCM,8,"AIFF format on 8 bits",
  "aiff16",SF_FORMAT_AIFF+SF_FORMAT_PCM,16,"AIFF format on 16 bits (little endian)",
  "aiff24",SF_FORMAT_AIFF+SF_FORMAT_PCM,24,"AIFF format on 24 bits (little endian)",
  "aiff32",SF_FORMAT_AIFF+SF_FORMAT_PCM,32,"AIFF format on 32 bits (little endian)",
  
  "au8",SF_FORMAT_AU+SF_FORMAT_PCM,8,"AU format on 8 bits",
  "au16",SF_FORMAT_AU+SF_FORMAT_PCM,16,"AU format on 16 bits (big endian)",
  "au24",SF_FORMAT_AU+SF_FORMAT_PCM,24,"AU format on 24 bits (big endian)",
  "au32",SF_FORMAT_AU+SF_FORMAT_PCM,32,"AU format on 32 bits (big endian)",
  "au16_ulaw",SF_FORMAT_AU+SF_FORMAT_ULAW,16,"AU format on 8 bits (obtained by ulaw coding 16 bits)",
  "au16_alaw",SF_FORMAT_AU+SF_FORMAT_ALAW,16,"AU format on 8 bits (obtained by alaw coding 16 bits)",
  
  "raw8_s",SF_FORMAT_RAW+SF_FORMAT_PCM_S8,8,"RAW format on 8 bits (as char)",
  "raw8_u",SF_FORMAT_RAW+SF_FORMAT_PCM_U8,8,"RAW format on 8 bits (as unsigned char)",
  "raw16_le",SF_FORMAT_RAW+SF_FORMAT_PCM_LE,16,"RAW format on 16 bits (little endian)",
  "raw24_le",SF_FORMAT_RAW+SF_FORMAT_PCM_LE,24,"RAW format on 24 bits (little endian)",
  "raw32_le",SF_FORMAT_RAW+SF_FORMAT_PCM_LE,32,"RAW format on 32 bits (little endian)",
  "raw16_be",SF_FORMAT_RAW+SF_FORMAT_PCM_BE,16,"RAW format on 16 bits (big endian)",
  "raw24_be",SF_FORMAT_RAW+SF_FORMAT_PCM_BE,24,"RAW format on 24 bits (big endian)",
  "raw32_be",SF_FORMAT_RAW+SF_FORMAT_PCM_BE,32,"RAW format on 32 bits (big endian)",
  
  "wave8",SF_FORMAT_WAV+SF_FORMAT_PCM,8,"WAVE format on 8 bits",
  "wave16",SF_FORMAT_WAV+SF_FORMAT_PCM,16,"WAVE format on 16 bits (big endian)",
  "wave24",SF_FORMAT_WAV+SF_FORMAT_PCM,24,"WAVE format on 24 bits (big endian)",
  "wave32",SF_FORMAT_WAV+SF_FORMAT_PCM,32,"WAVE format on 32 bits (big endian)",
  "wave16_ulaw",SF_FORMAT_WAV+SF_FORMAT_ULAW,16,"WAVE format on 8 bits (obtained by ulaw coding 16 bits)", 
  "wave16_alaw",SF_FORMAT_WAV+SF_FORMAT_ALAW,16,"WAVE format on 8 bits (obtained by alaw coding 16 bits)",
  /*  "wave_float",SF_FORMAT_WAV+SF_FORMAT_FLOAT,32,"",
      "wave_ms_adpcm",SF_FORMAT_WAV+SF_FORMAT_MS_ADPCM,8,"",
      "wave_ima_adpcm",SF_FORMAT_WAV+SF_FORMAT_IMA_ADPCM,8,"",
  */
  
  "paf16_le",SF_FORMAT_PAF+SF_FORMAT_PCM_LE,16,"ENSONIQ PARIS format on 16 bits (little endian)",
  "paf24_le",SF_FORMAT_PAF+SF_FORMAT_PCM_LE,24,"ENSONIQ PARIS format on 24 bits (little endian)",
  "paf16_be",SF_FORMAT_PAF+SF_FORMAT_PCM_BE,16,"ENSONIQ PARIS format on 16 bits (big endian)",
  "paf24_be",SF_FORMAT_PAF+SF_FORMAT_PCM_BE,24,"ENSONIQ PARIS format on 24 bits (big endian)",
  
  /*  "svx8",SF_FORMAT_SVX+SF_FORMAT_PCM,8,"SVX (amiga) format on 8 bits",
      "svx16",SF_FORMAT_SVX+SF_FORMAT_PCM,16,"SVX (amiga) format on 16 bits",
      does not work */
  
  
  NULL,0,0,NULL
};



#define NMAXSF 100 /* The maximum number of sound formats */

static SoundFormat theSoundFormats[NMAXSF]; /* The array which holds all the sound format */

static nSoundFormats = 0; /* the number of sound formats in the array 'theSoundFormats' */


/* 
 * This routine allows to get the sound format associated to a name 
 */
char Name2SoundFormat(char *str, SoundFormat *sf)
{
  int i,l;
  char *str1,*str2;
  char c;
  int samplingRate,nChannels;
  
  sf->name = NULL;
  sf->samplingRate = 0;
  sf->nChannels = 0;
  sf->basicSoundFormat = NULL;
  
  /* We look in the basic sound formats array first */
  for (i=0;theBasicSoundFormats[i].name !=NULL;i++) {
    if (!strcmp(str,theBasicSoundFormats[i].name)) {
      sf->basicSoundFormat = theBasicSoundFormats+i;
      return(YES);
    }
  }
  
  /* Then in the sound formats array */
  for (i=0;i<nSoundFormats;i++) {
    if (!strcmp(str,theSoundFormats[i].name)) {
      *sf = theSoundFormats[i];
      return(YES);
    }
  }
  
  /*
   * Then we check the raw format with a possible sampling rate and number of channels specified 
   */
  if (strncmp(str,"raw",3)) return(NO);
  
  /* We first look for te corresponding basic format */ 
  for (i=0;theBasicSoundFormats[i].name !=NULL;i++) {
    if (strncmp(theBasicSoundFormats[i].name,"raw",3)) continue;
    l = strlen(theBasicSoundFormats[i].name);
    if (strncmp(theBasicSoundFormats[i].name,str,l)) continue;
    
    /* Now that we found it, we look for the sampling rate */
    str1 = str+l;
    if (*str1 != '_') continue;  
    str1++;
    str2 = str1;
    while(isdigit(*str2)) str2++;
    c = *str2;
    *str2 = '\0';
    if (ParseInt_(str1,1,&samplingRate)==NO || samplingRate <= 0) {
      *str2 = c;
      continue;
    }
    *str2 = c;
    if (*str2 == '\0') {
      sf->basicSoundFormat = theBasicSoundFormats+i;
      sf->samplingRate = samplingRate;
      return(YES);
    }
    if (*str2 != '_') continue;
    
    /* Then an eventual number of channels */
    str2++;
    str1 = str2;
    while(isdigit(*str2)) str2++;
    c = *str2;
    *str2 = '\0';
    if (ParseInt_(str1,1,&nChannels)==NO || nChannels <= 0) {
      *str2 = c;
      continue;
    }
    *str2 = c;
    if (*str2 == '\0') {
      sf->basicSoundFormat = theBasicSoundFormats+i;
      sf->samplingRate = samplingRate;
      sf->nChannels = nChannels;
      return(YES);
    }
    continue;
  }
  
  return(NO);
}

/*
 * Get the Basic sound format associated to a format code of the sndfile library 
 */
BasicSoundFormat *Code2BasicSoundFormat(int formatCode, int bitWidth)
{
  int i;
  
  for (i=0;theBasicSoundFormats[i].name !=NULL;i++) {
    if (theBasicSoundFormats[i].format == formatCode && theBasicSoundFormats[i].bitWidth == bitWidth) return(theBasicSoundFormats+i);
  }
  
  return(NULL);
}

/* 
 * Get the sound format of a file and the size of the sound in it.
 * In the case of a raw file it uses the extension to understand the format.
 * returns YES if succeeded.
 */
char SoundInfo(char *file, SoundFormat *sf, int *size)
{
  char *f,*str;
  SF_INFO info;
  SNDFILE *sndfile;
  BasicSoundFormat *bsf;
  FILE * stream;
  struct signal siginfo;
  char header,binaryMode,binaryCoding;
  int nCols;
  
  /* Does the file exist ? */
  stream = FOpen(file,"r");
  if (stream == NULL) {
    SetErrorf("SoundInfo() : File '%s' does not exist",file);
    return(NO);
  }
  FClose(stream);

  /* Is it a LastWave file ? */
  if (ReadInfoSigFile(file, &siginfo, &header,&binaryMode, &binaryCoding, &nCols)==YES) {
    if (header == YES) {
      if (siginfo.type == XYSIG) return(NO);
      sf->basicSoundFormat = Code2BasicSoundFormat(SF_FORMAT_LW,0);
      sf->samplingRate = 1/siginfo.dx+.5;
      sf->nChannels = 1;
      *size = siginfo.size;
      return(YES);
    }
  }
  
  f = ConvertFilename(file);
  
  if ((sndfile = sf_open_read(f,&info)) == NULL) {

    /* We try to use the extension */
    str = file+strlen(file);
    while (str != file && *str != '.') str--;
    if (*str != '.') {
      SetErrorf("SoundInfo() : Cannot understand which format the sound file '%s' is in",file);
      return(NO);
    }
    str++;
    if (Name2SoundFormat(str,sf) == NO) {
      SetErrorf("SoundInfo() : Cannot understand which format the sound file '%s' is in",file);
      return(NO);
    }
    if (sf->samplingRate == 0) {
      SetErrorf("SoundInfo() : Cannot understand what the sampling rate of the sound file '%s' is",file);
      return(NO);
    }
    if (sf->nChannels == 0) sf->nChannels = 1;
    info.samplerate = sf->samplingRate;
    info.format = sf->basicSoundFormat->format;
    info.pcmbitwidth = sf->basicSoundFormat->bitWidth;
    info.channels = sf->nChannels;
    if ((sndfile = sf_open_read(f,&info)) == NULL) {
      SetErrorf("SoundInfo() : Cannot understand which format the sound file '%s' is in (even when using the extension)",file);
      return(NO);
    }
  }
  
  sf_close(sndfile);
 
  sf->samplingRate = info.samplerate;
  *size = info.samples;
  sf->nChannels = info.channels;
  
  bsf = Code2BasicSoundFormat(info.format,info.pcmbitwidth);
  if (bsf == NULL) Errorf("SoundInfo() : Sorry, sound format unknown by LastWave");
  sf->basicSoundFormat = bsf;
  sf->name = NULL;
  
  return(YES);
}


/*
 * Reading a sound file 
 *
 *  file          : the name of the sound file.
 *  sig,sig1      : the signals to read (sig1 could be NULL). 
 *                  Only a reading of the first 2 channels is  supported.
 *  flagSample    : if YES then both 'start' and 'duration' are expressed
 *                  in samples, otherwise they are expressed in seconds.
 *  start         : first sample to read.
 *  duration      : number of samples to read.
 *  flagNormalize : if YES then the maximum format value will be
 *                  renormalized to [-1,1] otherwise the orginal values 
 *                  (e.g., 16 bits integers) are used.
 *  flagRaw       : if YES then the file is a raw file and the following
 *                  parameters indicate hown to read it :
 *        flagLittleEndian : if YES then raw little endian
 *                           (used only if bitWidth >= 16).
 *        flagUnsigned     : if YES then raw unsigned char
 *                           (only used if bitwidth is 8).
 *        bitWidth         : number of bits per sample.
 *        sRate            : the sampling rate.
 *        nc               : the number of channels.
 */

void SoundRead(char *file,SIGNAL sig, SIGNAL sig1,
	       char flagSample, float start, float duration,
	       char flagNormalize,SoundFormat *sf1)
{
  char *f;
  SF_INFO info;
  int samplingRate;
  int size,first,sizeAsked;
  int nChannels;
  SNDFILE *sndfile;
  double *sound;
  int i ,j;
  SoundFormat sf;
  FILE * stream;
  float xMin,yMin,xMax,yMax;
  int iMin,iMax;
  
  if (sig == NULL && sig1 == NULL) 
    Errorf("SoundRead : You should specify at least 1 signal");
  if (start <= 0 && flagSample) start = 1;
  if (start <= 0 && !flagSample) start = 0;  
  
  /* Does the file exist ? */
  stream = FOpen(file,"r");
  if (stream == NULL) 
    Errorf("SoundRead : File '%s' does not exist",file);
  FClose(stream);
  
  f = ConvertFilename(file);
  
  /* This is the case the raw format has been forced
   and all the parameters specified */
  if (sf1 != NULL) {
    if (sf1->nChannels == 0) {
      if (sig1 == NULL) sf1->nChannels = 1;
      else sf1->nChannels = 2;
    }
    if (sf1->nChannels <= 0) Errorf("SoundRead() : The number of channels should be strictly positive");
    if (sf1->samplingRate == 0) sf1->samplingRate = 1;
    if (sf1->samplingRate <= 0) Errorf("SoundRead() : The samplingRate should be strictly positive");
    sf.samplingRate = info.samplerate = sf1->samplingRate;
    sf.nChannels = info.channels = sf1->nChannels;
    info.pcmbitwidth = sf1->basicSoundFormat->bitWidth;
    info.format = sf1->basicSoundFormat->format;
    sf.basicSoundFormat = sf1->basicSoundFormat;
    if ((sndfile = sf_open_read(f,&info)) == NULL) 
      Errorf("SoundRead() : Cannot read the file using the raw format you specified",file);
    sf_close(sndfile);
    size = info.samples;  
  }
  /* Otherwise we first read the information to know
     if the file is readable (using the extension if necessary) */
  else {
    if (SoundInfo(file, &sf, &size) == NO) Errorf1("");
    info.samplerate = sf.samplingRate;
    info.format = sf.basicSoundFormat->format;
    info.pcmbitwidth = sf.basicSoundFormat->bitWidth;
    info.channels = sf.nChannels;
  }


  /* Convert the start and duration variables into number of samples */
  if (flagSample == NO) {
    first = start*sf.samplingRate+1+.5;
    sizeAsked = duration*sf.samplingRate+.5;
  }
  else {
    first = start+.5;
    sizeAsked = duration+.5;
  }

  /* Compute the number of samples to read */
  if (sizeAsked <= 0) sizeAsked = size;
  sizeAsked = MIN((sizeAsked+first-1),(size));
  sizeAsked = sizeAsked-first+1;
    
  /* Some checkings */
  if (sf.nChannels == 1 && sig1 != NULL) {
    Errorf("SoundRead() : Sorry, the sound file '%s' has just one channel",file);
  }
  if (first > size) {
    if (flagSample) Errorf("SoundRead() : Sorry, the sound file '%s' is too short for <start>=%d samples",file,first);
    else  Errorf("SoundRead() : Sorry, the sound file '%s' is too short for <start>=%g seconds",file,start);
  }
  
  /* Set some variables */
  samplingRate = sf.samplingRate;
  nChannels = sf.nChannels;
  
  
  /* Special case of the LastWave format */
  if ((sf.basicSoundFormat->format & SF_FORMAT_TYPEMASK) == SF_FORMAT_LW) {
    ReadSigFile(sig,file,first-1,sizeAsked,0,0);
    sig->x0 = 0;
    sig->dx = 1./samplingRate;
    if (flagNormalize) {
      xMin = 1;
      xMax = -1;
      MinMaxSig(sig,&xMin,&xMax,&yMin,&yMax,&iMin,&iMax,NO);
      yMax = MAX(fabs(yMax),fabs(yMin));
      for (i=0;i<sig->size;i++) sig->Y[i] /= yMax;
    }     
    return;
  }
    
  /* Look for the first sample */
  if ((sndfile = sf_open_read(f,&info)) == NULL) Errorf("SoundRead() : Weird error");
  if (sf_seek(sndfile,(first-1)*nChannels,SEEK_SET)==-1) {
    sf_close(sndfile);
    Errorf("SoundRead() : Weird error : Problem for seeking sample number '%d' of sound file %f",first,file);
    return;
  }
  
  /* Allocate a double array and read the sound in it */
  sound = DoubleAlloc(sizeAsked*nChannels);  
  sizeAsked = sf_read_double(sndfile,sound,sizeAsked*nChannels,flagNormalize)/nChannels;
  
  /* Close the file */
  sf_close(sndfile);
  
  /* Error while reading ? */
  if (sizeAsked == -1) {
    Free(sound);
    Errorf("SoundRead() : problem while reading sound file '%f'",file);
  }    
  
  /* Fill up the signal(s) */
  if (sig != NULL) {
    SizeSignal(sig,sizeAsked,YSIG);
    sig->x0 = 0;
    sig->dx = 1./samplingRate;
  }
  if (sig1 != NULL && nChannels == 2) {
    SizeSignal(sig1,sizeAsked,YSIG);
    sig1->x0 = 0;
    sig1->dx = 1./samplingRate;
  }  
  /* Fill up the signal(s)
     Case of a mono sound file read in a signal */
  if (nChannels == 1) {
    for (i=0;i<sizeAsked;i++) sig->Y[i] = sound[i];
  }
  /* Case of a multichannel sound file read in a single signal :
    only the first channel is read */
  else if (sig != NULL && sig1 == NULL) { 
    for (i=0,j=0;i<sizeAsked;i++,j+=nChannels) sig->Y[i] = sound[j];
  }
  /* or only the second channel is read ???????	  */
  else if (sig == NULL && sig1 != NULL) { 
    for (i=0,j=0;i<sizeAsked;i++,j+=nChannels) sig1->Y[i] = sound[j+1];
  }
  /* Case of a multichannel sound file read in two signals	  */
  else {
    for (i=0,j=0;i<sizeAsked;i++,j+=nChannels) {
      sig->Y[i] = sound[j];
      sig1->Y[i] = sound[j+1];
    }
  }
  
  /* Free the double array */
  Free(sound);
}    


/*
 * Routine for writing a sound in a file
 *
 *  file : the soundfile it will create
 *  sig,sig1 : the signals to read (sig1 could be NULL). Only a sound file with 1 or 2 channels is supported.
 *  flagNormalize : if YES then the signals will be normalized to the maximum amplitude before being written
 *                  if NO then 'maxNorm' is used as the maximum amplitude value. 
 *  maxNorm : specifies the maximum amplitude value in the case flagNormalize==NO. 
 *            If it is negative then the format maximum amplitude is used (e.g., [-32768,32767] for 16 bits integers).
 *  sf : the sound format to be used (if it is NULL then the extension of the <file> should be used to determine it)
 */

void SoundWrite(char *file,SIGNAL sig, SIGNAL sig1, char flagNormalize, float maxNorm, SoundFormat *sf)
{
  char *f;
  SF_INFO info;
  int samplingRate;
  int size;
  int nChannels;
  SNDFILE *sndfile;
  double *sound,maxp,maxm,max;
  int i ,j;
  unsigned long l;
  SoundFormat *sf1;
  SoundFormat soundFormat;
  char *str;
    
  if (sig == NULL) Errorf("SoundWrite() : The first signal should be specified");
  if (sig1 != NULL && sig->size != sig1->size) Errorf("SoundWrite() : The two signals should have the same size");
  
  /* Set the sampling rate and the number of channels */
  samplingRate =  1/sig->dx+.5;
  if (sig1 == NULL) nChannels = 1;
  else nChannels = 2;
  
  /* Case we have to use the extension to understand the format */    
  sf1 = sf;
  if (sf1 == NULL) {
    sf1 = &soundFormat;
    str = strlen(file)+file;
    while (str != file && *str != '.') str--;
    if (*str != '.') Errorf("SoundWrite : Sorry, I cannot understand what the format you want me to use is"); 
    if (Name2SoundFormat(str+1,sf1) == NO) Errorf("SoundWrite : Sorry, I cannot understand what the format you want me to use is"); 
    
    /* Check the compatibility */
    if (sf1->samplingRate != 0 && sf1->samplingRate != samplingRate) 
      Errorf("SoundWrite : Sorry,incompatibility between the sampling rate the format requests (%d) and the sampling rate of the signal (%d)",sf1->samplingRate,samplingRate);
    if (sf1->nChannels != 0 && sf1->nChannels != nChannels) 
      Errorf("SoundWrite : Sorry,incompatibility between the number of channels the format requests (%d) and the number of signals (%d)",sf1->nChannels,nChannels);
  }

  /* Special case of LastWave format (binary) */
  if ((sf1->basicSoundFormat->format & SF_FORMAT_TYPEMASK) == SF_FORMAT_LW) {
    if (nChannels == 2) Errorf("SoundWrite() : Sorry, only a single channel can be used in the LastWave format");
    WriteSigFile(sig,file,YES,"y",YES);
    return;
  }

  /* Open the file */      
  f = ConvertFilename(file);
  info.samplerate = samplingRate;
  info.pcmbitwidth = sf1->basicSoundFormat->bitWidth;
  info.format = sf1->basicSoundFormat->format;
  info.channels = nChannels;
  if ((sndfile = sf_open_write(f,&info)) == NULL) Errorf("SoundWrite() : Cannot open sound file '%s'",file);
  
  /* Write in the case of a single signal */
  if (sig1 == NULL) {
    sound = DoubleAlloc(sig->size);
    max = -1;
    for (i = 0;i<sig->size;i++) {
      sound[i] = sig->Y[i];
      if (fabs(sound[i])>max) max = fabs(sound[i]);
    }
    if (flagNormalize || maxNorm > 0) {
      l = 1;
      if (flagNormalize) {
        maxm = max/(l<<(sf1->basicSoundFormat->bitWidth-1));
        maxp = max/((l<<(sf1->basicSoundFormat->bitWidth-1))-1);
      }
      else {
        maxm = maxNorm/(l<<(sf1->basicSoundFormat->bitWidth-1));
        maxp = maxNorm/((l<<(sf1->basicSoundFormat->bitWidth-1))-1);
      }
      for (i = 0;i<sig->size;i++) {
        if (sound[i]>=0) sound[i] /= maxp;
        else sound[i] /= maxm;
      }
    }   
    size = sf_write_double(sndfile,sound,sig->size,0);
    Free(sound);
    sf_close(sndfile);
    if (size == -1) Errorf("SoundWrite() : Problem while writing sound file '%s'",file);
    return;
  }
  
  /* Write in the case of 2 signals */
  sound = DoubleAlloc(sig->size*2);
  max = -1;
  for (i = 0,j=0;i<sig->size;i++,j+=2) {
    sound[j] = sig->Y[i];
    sound[j+1] = sig1->Y[i];
    if (fabs(sound[j])>max) max = fabs(sound[j]);
    if (fabs(sound[j+1])>max) max = fabs(sound[j+1]);
  }
  if (flagNormalize || maxNorm > 0) {
    l = 1;
    if (flagNormalize) {
      maxm = max/(l<<(sf1->basicSoundFormat->bitWidth-1));
      maxp = max/((l<<(sf1->basicSoundFormat->bitWidth-1))-1);
    }
    else  {
      maxm = maxNorm/(l<<(sf1->basicSoundFormat->bitWidth-1));
      maxp = maxNorm/((l<<(sf1->basicSoundFormat->bitWidth-1))-1);
    }
    for (i = 0;i<2*sig->size;i++) {
      if (sound[i]>=0) sound[i] /= maxp;
      else sound[i] /= maxm;
    }
  }   
  size = sf_write_double(sndfile,sound,sig->size*2,0);
  Free(sound);
  sf_close(sndfile);
  if (size == -1) Errorf("SoundWrite : Problem while writing sound file '%s'",file);
}    

/* REMI : ANSI interface to the SoundWrite function		  */
void SoundWriteSamples(char *file,short *samplesLeft, short *samplesRight,unsigned long nbSamples, float sampleFreq,SoundFormat *sf)
{
  static SIGNAL sigLeft  = NULL;
  static SIGNAL sigRight = NULL;
  unsigned long i;
  /* Allocates the signals, only once */
  if(sigLeft == NULL) {
    sigLeft  = NewSignal();
    sigRight = NewSignal();
  }
  /* Sets the size and copies the samplesLeft */
  SizeSignal(sigLeft,nbSamples,YSIG);
  for(i = 0; i < nbSamples; i++) {
    sigLeft->Y[i] = samplesLeft[i];
  }
  if(sampleFreq < 2000) {
    Warningf("SoundWriteSamples: Sampling rate %f Hz is below 2000 Hz, I changed it to 2000 Hz\n",sampleFreq);
    sigLeft->dx = 1/2000;
  } 
  else {
    sigLeft->dx = 1/sampleFreq;
  }
  
  /* To play on one channel	   */
  if(samplesRight == NULL) {
    SoundWrite(file,sigLeft,NULL,YES,0.0,sf);
  }
  else {
    SizeSignal(sigRight,nbSamples,YSIG);
    for(i = 0; i < nbSamples; i++) {
      sigRight->Y[i] = samplesRight[i];
    }
    sigRight->dx = sigLeft->dx;
    SoundWrite(file,sigLeft,sigRight,YES,0.0,sf);
  }
}

/* 
 * Play a sound which is represented by a 'signal'.
 * If 'sampleFreq' <= 0 then the dx of the signal is used
 * to compute the sample frequency otherwise 'sampleFreq' is used.
 * If 'flagMaxVolume' is YES then the signal is normalized to 1 before played
 * otherwise it is kept as it is (the values are supposed to be between -1 and 1).
 */

void SoundPlay(SIGNAL sigLeft,SIGNAL sigRight,char flagMaxVolume, float sampleFreq)
{
  float max;
  int i;
  short *samplesLeft,*samplesRight;
  float t;

  if(sigRight && (sigLeft->size!=sigRight->size))
    Errorf("SoundPlay : the two channels have a different number of samples!");

  /* Compute the max of the signal if normalization is needed */
  if (flagMaxVolume) {
    max = 0;
    for(i=0;i<sigLeft->size;i++) {
      max = MAX(max,fabs(sigLeft->Y[i]));
    }
    /* REMI : normalization of stereo signal */
    if (sigRight) {
      for(i=0;i<sigLeft->size;i++) {
	max = MAX(max,fabs(sigRight->Y[i]));
      }
    }
  }
  else max = 1;
  
  /* Compute sample frequency */
  if (sampleFreq <= 0) sampleFreq = 1/sigLeft->dx;
  
  /* Allocation of array of short */
  samplesLeft = (short *) Malloc(sizeof(short)*sigLeft->size);
  for(i=0;i<sigLeft->size;i++) samplesLeft[i] = 32767*((sigLeft->Y[i])/max);
  /* One or two channels ?  */
  if (sigRight==NULL) samplesRight=NULL;
  else {
    samplesRight = (short *) Malloc(sizeof(short)*sigLeft->size);
    for(i=0;i<sigLeft->size;i++) samplesRight[i] = 32767*((sigRight->Y[i])/max);
  }
  /* Then play it */  
  XXSoundPlay(samplesLeft,samplesRight,sigLeft->size,sampleFreq);
  /* Desallocation */
  Free(samplesLeft);
  if(samplesRight) Free(samplesRight);
}

/*
 * Stop a currently playing sound
 */
void SoundStop(void)
{
  XXSoundStopPlaying();
}

/*
 * Recording a mono sound into 'signalLeft' or a stereo sound into 'signalLeft' and 'signalRight'.
 * See the help of XXSoundRecord.
 * Returns YES if a sound was recorded and NO otherwise.
 */
char SoundRecord(SIGNAL sigLeft,SIGNAL sigRight,
		 unsigned char soundQuality,
		 unsigned long customSampleFreq,unsigned char customBitsPerSample,
		 unsigned long maxNbSamples)
{
  unsigned long nbSamples;
  float sampleFreq;
  
  /* Empty the signals */
  ClearSignal(sigLeft);
  if(sigRight) ClearSignal(sigRight);

  /* Record sound */  
  if(sigRight==NULL) {
    XXSoundRecord(&(sigLeft->Y),NULL,
		  &nbSamples,&sampleFreq,
		  soundQuality,
		  customSampleFreq,customBitsPerSample,
		  maxNbSamples);
  }
  else {
    XXSoundRecord(&(sigLeft->Y),&(sigRight->Y),
		  &nbSamples,&sampleFreq,
		  soundQuality,
		  customSampleFreq,customBitsPerSample,
		  maxNbSamples);
  }
  /* Case when nothing was recorded	  */
  if (nbSamples == 0) return(NO);
  
  /* Init signal fields */
  sigLeft->sizeMallocY = nbSamples;
  sigLeft->size = nbSamples;
  sigLeft->dx = 1./sampleFreq;
  sigLeft->x0 = 0;
  sigLeft->firstp = 0;
  sigLeft->type = YSIG;
  sigLeft->lastp = nbSamples-1;
  if(sigRight!=NULL) {
    sigRight->sizeMallocY = nbSamples;
    sigRight->size = nbSamples;
    sigRight->dx = 1./sampleFreq;
    sigRight->x0 = 0;
    sigRight->firstp = 0;
    sigRight->type = YSIG;
    sigRight->lastp = nbSamples-1;
  }
  return(YES);
}

void C_Sound(char **argv)
{
  char *action;
  char *file,*format;
  int size;
  SoundFormat sf,*sf1;
  SIGNAL sigLeft=NULL,sigRight=NULL;
  float duration,start;
  /* Only for recording	 */
  unsigned long maxNbSamples = 0;
  unsigned char soundQuality;
  unsigned long customSampleFreq = 0;
  unsigned char customBitsPerSample = 16;
  float fCustomSampleFreq = 0.0;

  int first,sizeAsked;
  char flagNormalize,flagSample,flagMaxVolume;
  char opt;
  char *str;
  int i,j;
  int samplingRate;
  float max;
  LISTV lv;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  if (!strcmp(action,"info")) {
    argv = ParseArgv(argv,tSTR,&file,0);
    if (SoundInfo(file, &sf, &size)==NO) Errorf1("");
    lv = TNewListv();
    SetResultValue(lv);
    AppendInt2Listv(lv,size);
    AppendInt2Listv(lv,sf.samplingRate);
    AppendInt2Listv(lv,sf.nChannels);
    AppendStr2Listv(lv,sf.basicSoundFormat->name);
    return;
  }
  
  if(!strcmp(action,"read")) {
    argv = ParseArgv(argv,tSIGNAL,&sigLeft,tSIGNAL_,NULL,&sigRight,tSTR,&file,-1);
    
    if (*argv != NULL && **argv != '-') {
      argv = ParseArgv(argv,tSTR,&format,-1);
      if (strncmp(format,"raw",3)) Errorf("You must specify a 'raw' format");
      if (Name2SoundFormat(format,&sf) == NO) Errorf("Invalid format '%s'",format);
      sf1 = &sf;
    }
    else sf1 = NULL;
    
    duration = -1;
    start = -1;
    sizeAsked = -1;
    first = -1;
    flagNormalize = YES;
    flagSample = NO;
    while ((opt = ParseOption(&argv))) {
      switch(opt) {
      case 'd': 
	argv = ParseArgv(argv,tFLOAT,&duration,-1);
	if (duration <= 0) ErrorUsage(); 
	if (first != -1 || sizeAsked != -1) Errorf("You must not use at the same time the options s,d with S,D");
	break;
      case 'D': 
	argv = ParseArgv(argv,tINT,&sizeAsked,-1);
	if (sizeAsked <= 0) ErrorUsage(); 
	if (start != -1 || duration != -1) Errorf("You must not use at the same time the options s,d with S,D");
	flagSample = YES;
	break;
      case 's': 
	argv = ParseArgv(argv,tFLOAT,&start,-1);
	if (start < 0) ErrorUsage(); 
	if (first != -1 || sizeAsked != -1) Errorf("You must not use at the same time the options s,d with S,D");
	break;
      case 'S': 
	argv = ParseArgv(argv,tINT,&first,-1);
	if (first <= 0) ErrorUsage(); 
	if (start != -1 || duration != -1) Errorf("You must not use at the same time the options s,d with S,D");
	flagSample = YES;
	break;
      case 'n':
	flagNormalize = NO;	    
	break;
      default: ErrorOption(opt);
      }
    }
    NoMoreArgs(argv);
    
    if (flagSample) {
      duration = sizeAsked;
      start = first;
    }
    
    SoundRead(file,sigLeft,sigRight,flagSample,start,duration,flagNormalize,sf1);
    
    return;
  }
  
  if(!strcmp(action,"write")) {
    argv = ParseArgv(argv,tSIGNALI,&sigLeft,tSIGNALI_,NULL,&sigRight,tSTR,&file,tSTR_,"",&format,-1);
    if (sigRight != NULL && sigRight->size != sigLeft->size) Errorf("The signals should have the same size");
    if (*format != '\0') {
      if (Name2SoundFormat(format,&sf) == NO) Errorf("Unknown sound format '%s'",format);
      sf1 = &sf;
    }
    else sf1 = NULL;
    
    flagNormalize = YES;
    max = -1;
    while ((opt = ParseOption(&argv))) {
      switch(opt) {
      case 'n': 
	argv = ParseArgv(argv,tFLOAT_,-1.0,&max,-1);
	if (max == 0) Errorf("Bad Value for <max>=%g in '-n' option",max);
	flagNormalize = NO;	    
	break;
      default: ErrorOption(opt);
      }
    }
    NoMoreArgs(argv);
    
    SoundWrite(file,sigLeft,sigRight,flagNormalize,max,sf1);
    
    return;
  }
  
  if(!strcmp(action,"format")) {
    argv = ParseArgv(argv,tSTR_,"*",&str,-1);
    if (*argv == NULL) {
      for (i=0;theBasicSoundFormats[i].name !=NULL;i++) {
        if (MatchStr(theBasicSoundFormats[i].name,str)) {
          Printf("%s",theBasicSoundFormats[i].name);
          for (j=15;j>=strlen(theBasicSoundFormats[i].name);j--) Printf(" ");
          Printf(": %s\n",theBasicSoundFormats[i].info);
        }
      }
      for (i=0;i<nSoundFormats;i++) {
        if (MatchStr(theSoundFormats[i].name,str)) {
          Printf("%s",theSoundFormats[i].name);
          for (j=15;j>=strlen(theSoundFormats[i].name);j--) Printf(" ");
          Printf(": %s\n",theSoundFormats[i].basicSoundFormat->name);
        }
      }
    }
    else {
      argv = ParseArgv(argv,tSTR,&format,0);
	  if (Name2SoundFormat(format,&sf) == NO) Errorf("Sorry, I don't know the sound format '%s'",format);
      for (i=0;i<nSoundFormats;i++) {
        if (!strcmp(theSoundFormats[i].name,str)) break;
       }
      if (i == nSoundFormats) {
        if (nSoundFormats == NMAXSF-1) Errorf("Sorry, too many formats already defined (maximum is %d).",NMAXSF);
        i = nSoundFormats;
        nSoundFormats++;
	    theSoundFormats[i].name = CopyStr(str);
      }
	  theSoundFormats[i].basicSoundFormat = sf.basicSoundFormat;
	  theSoundFormats[i].samplingRate = sf.samplingRate;
	  theSoundFormats[i].nChannels = sf.nChannels;
	}
    return;  
  }

  if (!strcmp(action,"play")) {
    /*    argv = ParseArgv(argv,tSIGNALI,&sigLeft,tSIGNALI_,NULL,&sigRight,tINT_,-1,&samplingRate,-1); */
    argv = ParseArgv(argv,tSIGNALI,&sigLeft,-1);
    argv = ParseArgv(argv,tSIGNALI_,NULL,&sigRight,-1);
    argv = ParseArgv(argv,tINT_,-1,&samplingRate,-1);
    flagMaxVolume = YES;
    while(opt = ParseOption(&argv)) {
      switch(opt) {
      case 'n': flagMaxVolume = NO; break;
      default: ErrorOption(opt);
      }
    }
    NoMoreArgs(argv);
    Printf("Starts playing...");Flush();
    SoundPlay(sigLeft,sigRight,flagMaxVolume,samplingRate);
    Printf("Done\n");
    return;
  }
  
  if (!strcmp(action,"record")) {
    /* Read the first signal */
    argv = ParseArgv(argv,tSIGNAL,&sigLeft,-1);
    /* We interpret the next argument as a signal, if possible */
    ParseArgv(argv,tSIGNAL_,NULL,&sigRight,-1);
    /* If we did not read a signal, then we read the duration */
    if (sigRight == NULL) 
      argv = ParseArgv(argv,tFLOAT_,0.0,&duration,-1);
    /* If we read a signal, we may have confused <duration> with the name of a signal ! */
    else {
      /* If we cannot read <duration> now, it means what we believed was 
       * a valid signal name was indeed <duration>. So no signal 
       * was given as an input ! We must correct it and read <duration> */
      if (ParseFloat_(*(argv+1),0.0,&duration) == NO) {
	sigRight = NULL;
	argv = ParseArgv(argv,tFLOAT_,0.0,&duration,-1);
      }
      /* Else, we have now correctly read <sigRight> and <duration>  */
      else argv+=2;
    }
 
    /* Now we can read the quality */
    argv = ParseArgv(argv,tSTR_,"cd",&str,-1);
    /* Determine the sound quality */
    if (!strcmp(str,"cd")) {
      soundQuality = cdSoundQuality;
      NoMoreArgs(argv);
    }
    else if (!strcmp(str,"voice")) {
      soundQuality = voiceSoundQuality;
      NoMoreArgs(argv);
    }
    else if (!strcmp(str,"phone")) {
      soundQuality = phoneSoundQuality;
      NoMoreArgs(argv);
    }
    else if (!strcmp(str,"custom")) {
      soundQuality = customSoundQuality;
      /* REMI : this will be simplified when there is a tLONGINT type ! */
      argv = ParseArgv(argv,tFLOAT,&fCustomSampleFreq,tINT,&customBitsPerSample,0);
      customSampleFreq = (unsigned long)fCustomSampleFreq;
    }
    else Errorf("Bad Sound quality '%s'",str);

    // For non-interactive recording, we should convert the duration to a number of samples
    if(duration >= 0) {
      switch(soundQuality) {
      case cdSoundQuality :
	maxNbSamples = duration*44100;
	break;
      case voiceSoundQuality :
	maxNbSamples = duration*22050;
	break;
      case phoneSoundQuality :
	maxNbSamples = duration*8000;
	break;
      case customSoundQuality :
	maxNbSamples = duration*customSampleFreq;
      }
    }
    else Errorf("Bad duration %g !",duration);

    if (SoundRecord(sigLeft,sigRight,
		    soundQuality,customSampleFreq,customBitsPerSample,
		    maxNbSamples)) SetResultf("1");
    else SetResultf("0");
    return;
  }
  
  if (!strcmp(action,"stop")) {
    SoundStop();
    return;
  }

  Errorf("Bad action '%s'",action);  
  
}

/* EOF */
