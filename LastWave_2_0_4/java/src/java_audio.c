/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   1 . 6 . 1                           */
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
/*   java_audio.c :                                                 */
/*                                                                  */
/********************************************************************/

#include "computer.h"

#ifdef LASTWAVE_JAVA_AUDIO

#include "lastwave.h"
#include "xx_audio.h"
/*
 * Dealing With sounds : do nothing !
 */

/* Function to play a sound. */
void XXSoundPlay(short *samplesLeft,short *samplesRight,
		 unsigned long nbSamples,float sampleFreq)
{
  printf("XXSoundPlay being called\n");
}

/* Function to stop playing a sound. */
void XXSoundStopPlaying(void)
{
  printf("XXSoundStopPlaying being called\n");
}


/* Function to record a sound */
void XXSoundRecord(float **samplesLeft,float **samplesRight,
		   unsigned long *nbSamples,float *sampleFreq,
		   unsigned char soundQuality,
		   unsigned long customSampleFreq,unsigned char customBitsPerSample,
		   unsigned long maxNbSamples)
{
  printf("XXSoundRecord being called\n");
  return;
}

/* Function to stop recording a sound. */
void XXSoundStopRecording(void)
{
  printf("XXSoundStopRecording being called\n");
}


#endif //LASTWAVE_JAVA_AUDIO


