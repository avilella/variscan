/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   A n s i   2 . 0                                   */
/*                                                                          */
/*      Copyright (C) 2000 Emmanuel Bacry.                                  */
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
/*   ansi_audio.c :                                                 */
/*                                                                  */
/********************************************************************/

#include "lastwave.h"
#include "xx_audio.h"

#ifdef LASTWAVE_ANSI_AUDIO
/*
 * Dealing With sounds : do nothing !
 */

/* Function to play a sound. */
void XXSoundPlay(short *samplesLeft,short *samplesRight,
		 unsigned long nbSamples,float sampleFreq)
{
  Warningf("XXSoundPlay : not implemented in ANSI");
}

/* Function to stop playing a sound. */
void XXSoundStopPlaying(void)
{
  Warningf("XXSoundStopPlaying : not implemented in ANSI");
}


/* Function to record a sound */
void XXSoundRecord(float **samplesLeft,float **samplesRight,
		   unsigned long *nbSamples,float *sampleFreq,
		   unsigned char soundQuality,
		   unsigned long customSampleFreq,unsigned char customBitsPerSample,
		   unsigned long maxNbSamples)
{
  Warningf("XXSoundRecord : not implemented in ANSI");
  *nbSamples  = 0;
  *sampleFreq = 1.0;
  return;
}

/* Function to stop recording a sound. */
void XXSoundStopRecording(void)
{
  Warningf("XXSoundStopRecording : not implemented in ANSI");
}


#endif //LASTWAVE_ANSI_AUDIO


