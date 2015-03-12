/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
/*                                                                          */
/*      Copyright (C) 2002 Remi Gribonval.                                  */
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
/*  XX_AUDIO.c   The Machine dependent Audio functions                      */
/*                                                                          */
/****************************************************************************/

//
// Machine dependent Audio interface 
//
//REMI : added a second channel of samples,
//       "stop" functions
//       maxNbSamples for recording
//       changed 'int' to 'unsigned long'

// Play a sound on the default output device, at the given sampling frequency.
// The sound may be mono, in which case sampleRight must be NULL.
extern void XXSoundPlay(short *samplesLeft,short *samplesRight,
			unsigned long nSamples,float sampleFreq);
// Stop the currently playing sound
extern void XXSoundStopPlaying(void);

// The various recording sound qualities. 
// Any implementation of XXSoundRecord must either
// -record sound at the required sampling frequency 
//  with AT LEAST the given number of bits per sample
// -generate an error saying the sound quality is not available
// REMARK : if this list is changed in a further version of LastWave,
// the option "record" of C_Sound in the file package_sound/src/sound.c
// and its help line should be updated accordingly (not to mention
// the fact that the implementations of XXSoundRecord should also be updated)
enum {
  cdSoundQuality = 1, // 44.1  khz, 16 bits 
  voiceSoundQuality,  // 22.05 khz,  8 bits
  phoneSoundQuality,  //  8    khz,  8 bits
  customSoundQuality  // custom   ,  custom
};

// Record a sound from the default input device that supports the
// recording quality.
// If there is no such device, an error should be generated.
// For all but the customQuality, the fields 'customSampleFreq' 
// and 'customBitsPerSample' are not used.
// The recording may be in mono or stereo, depending whether
// 'samplesRight'==NULL. The output samples are put in 'samplesLeft' 
// (and 'samplesRight' for stereo recording ), they are 'normalized' 
// between -1 and 1.
// The recording mode may be
// - interactive : it only stops with user interaction, for example 
//                 when a 'stop' button is pressed or
//                 the XXSoundStopRecording function (if implemented) is called
// - non-interactive : it makes user interaction impossible until
//                     the specified number of samples has been recorded, or an
//                     error has occured.
// The recording mode is specified by the value of 'maxNbSamples':
// - set maxNbSamples == 0 for interactive recording 
// - a nonzero values specifies how many samples should be recorded
//        => WARNING : be CAREFUL with the duration/memory needed!
//
// Return value : 
//    *nbSamples is set to the number of samples actually recorded. 
//      (If 0 it did not record anything)
//    *sampleFreq to the actual sampling frequency
//
// REMARK : 
// which soundQualities are implemented is allowed to depend 
// on the machine implementation.
extern void XXSoundRecord(float **samplesLeft,float **samplesRight, 
			  unsigned long *nbSamples,float *sampleFreq,
			  unsigned char soundQuality,
			  unsigned long customSampleFreq,unsigned char customBitsPerSample,
			  unsigned long maxNbSamples);

// Stop the current recording
extern void XXSoundStopRecording(void);

//EOF
