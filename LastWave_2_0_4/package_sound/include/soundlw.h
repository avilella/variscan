/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'sound' 2.0                        */
/*                                                                          */
/*      For the 'sndfile' library                                           */
/*      Copyright (C) 1999 Erik de Castro Lopo <erikd@zip.com.au>           */
/*                                                                          */
/*      For everything else                                                 */
/*      Copyright (C) 1999,2000,2001,2002 Emmanuel Bacry.                   */
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


/* 
 * The basic sound format structure
 */
typedef struct basicSoundFormat {
  char *name;     /* The name of the format */
  int format;     /* The corresponding format for the sndfile library */
  int bitWidth;   /* The number of bits used for each sample (before compression) */
  char *info;     /* A one-line info about the format */
} BasicSoundFormat;

/* 
 * This structure allows to define new names for existing formats.
 * Since formats can be used as extensions of sound filenames, 
 * this system allows to associate a given extension to a format.
 * The sampling rate and the number of channels is stored only in the case of raw formats.
 */
 
typedef struct soundFormat {
  char *name;                          /* The new name */
  BasicSoundFormat* basicSoundFormat;  /* The basic format it points to */
  int samplingRate;                    /* If different from 0, stores the sampling rate (raw format only) */
  int nChannels;                       /* If different from 0, stores the number of chanels (raw format only) */
} SoundFormat;

extern char Name2SoundFormat(char *str, SoundFormat *sf);
extern BasicSoundFormat *Code2BasicSoundFormat(int formatCode, int bitWidth);
extern char SoundInfo(char *file, SoundFormat *sf, int *size);

//REMI: Functions to read and write sound files to/from SIGNAL structures
//      Prototypes are only available to files where the SIGNAL structure is already defined
#ifdef SIGNALS_H
extern void SoundRead(char *file,SIGNAL sigLeft, SIGNAL sigRight, char flagSample, float start, float duration, char flagNormalize,SoundFormat *sf1);
extern void SoundWrite(char *file,SIGNAL sigLeft, SIGNAL sigRight, char flagNormalize, float max, SoundFormat *sf);
extern void SoundPlay(SIGNAL sigLeft,SIGNAL sigRight,char flagMaxVolume, float sampleFreq);
extern char SoundRecord(SIGNAL sigLeft,SIGNAL sigRight,
			unsigned char soundQuality,
			unsigned long customSampleFreq,unsigned char customBitsPerSample,
			unsigned long maxnbSamples);
#endif// SIGNALS_H

//REMI: Generic ANSI interface to the above read/write functions
extern void SoundWriteSamples(char *file,short *samplesLeft,short *samplesRight,unsigned long nbSamples,float sampleFreq,SoundFormat *sf);
extern void SoundReadSamples(char *file,float ** samplesLeft, float **samplesRight, 
			     unsigned long *nbSamples, float *sampleFreq,
			     char flagSample, float start, float duration, 
			     char flagNormalize,SoundFormat *sf1);




