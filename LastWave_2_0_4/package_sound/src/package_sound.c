/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'sound' 2.0                        */
/*                                                                          */
/*      For the 'sndfile' library                                           */
/*      Copyright (C) 1999 Erik de Castro Lopo <erikd@zip.com.au>           */
/*                                                                          */
/*      For everything else                                                 */
/*      Copyright (C) 1999-2002 Emmanuel Bacry.     						*/
/*      Copyright (C) 2002 Remi Gribonval                                   */
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


/***********************************
 * 
 *          CProcs related to signals
 *
 ***********************************/


extern void C_Sound();


static CProc soundCommands[] = {

 /* WARNING the 'play' action must be the first one listed in the help */
 "sound",C_Sound,"{{{play <sigL> [<sigR>] [<sampleFreq>] [-n]} {WARNING :\
On Unix computers, there is no builtin command to play audio signals. \
On Macintosh and Windows (Cygwin) computers, it works just fine. \n\n\
This command plays a signal <sigL> in mono or a pair of (left/right) signals \
<sigL> <sigR> in stereo using the default sound output device. \n\n\
By default, a mono signal is normalized (between -1 and 1) before playing, \
and a stereo signal is jointly normalized (i.e. the relative level of the \
two channels is preserved). Option '-n' allows to avoid normalization \
(both <sigL> and <sigR> are then assumed to have values between -1 and 1, \
otherwise clicks will occur). \n\n\
If <sampleFreq> is not specified then the 'dx' field of <sigL> is used to \
compute the sample frequency, otherwise <sampleFreq> is used.}} \
{{stop} {WARNING : Works only on Windows computers. Stop a currently playing sound.}} \
{{record <sigL> [<sigR>] [<soundQuality>=cd [<sampleFreq> <nBitsPerSample>]]} {WARNING :\
Works only on Macintosh and Windows (Cygwin) computers. Records a sound using default \
sound input device that supports the <soundQuality> and sets it in in \
<sigL> (mono recording) or <sigL> <sigR> (stereo recording). The recorded values are \
between -1 and 1 and the 'dx' field is set using the sample frequency. \n\
By default the 'quality' is 'cd'. The various possible qualities are\n\
 'cd'      :       44100  Hertz, (at least)              16   bits;\n\
 'voice'   :       22050  Hertz, (at least)               8   bits;\n\
 'phone'   :        8000  Hertz, (at least)               8   bits;\n\
 'custom'  : <sampleFreq> Hertz, (at least) <nBitsPerSample>  bits.\n\
Only for 'custom' quality you can (and must!) specify the sampling rate \
and number of bits/sample.}} \
{{write <sigL> [<sigR>] <filename> [<format>] [-n [<max>]]} {Writes in file <filename> the sounds <sigL> (and <sigR> \
if specified). If <format> is not specified then the suffix of the <filename> is used to understand what the format is. \
In case of raw formats the suffix should also contain the sampling rate and the number of channels. This is done using \
the syntax <rawformat>_<samplingRate>_<nChannels>. Thus for instance a file named 'file.raw16_le_44100_1' is a raw file \
using 16 bits integers, little endian, a sampling rate of 44100Hz and a single channel. In order to avoid writing \
very long suffices, you can rename them using the 'sound format' command. \
Before being written in the file, the signals are normalized in order to use the whole dynamic range. If '-n' is set this is no longer true. \
The <max> parameter specifies what should be considered as the the maximum value. \
If <max> is not specified then it is taken to be the maximum available in the corresponding format (e.g., the range is [-32768,32767] \
if the format uses 16 bits integers).}} \
{{read <sigL> [<sigR>] <filename> [<rawFormat>] [-(s|S) <startingPoint>] [-(d|D) <duration>] [-n]} \
{Reads the signals <sigL> and <sigR> from a sound file. \
You can specify a <sigR> only if the soundfile has more than 1 channel (such as a stereo file). \
You can specify the starting sample the signals should be read from and the duration that should be read using \
the options -d and -s (<startingPoint> and <duration> are expressed in seconds) or -D and -S \
(<startingPoint> and <duration> are expressed in seconds). If the option '-n' is not set \
then the maximum format range is renormalized to [-1,1] otherwise it is kept as it is. \
If the file is in raw format, and if you do not use the '-r' option \
it will try to use the extension of the <filename> (if there is one) in order to understand what the format is (i.e., number of bytes, endianness, sampling rate and the number of channels). \
If you want to specify the parameters of raw format files, you must specify the <rawFormat> to use (this is useless if the filename has an extension which specifies the format it is in).}} \
{{info <filename>} {Gets some information about the soundfile <filename>. It returns a listv made of \
<nbSamples> <samplingRate> <nChannels> <soundFormat>}} \
{{format <matchExpr> [<format>]} {If <format> is not specified, then it just lists all the available sound formats. If it is specified \
it defines a new format named <MatchExpr> corresponding to format <format>. Let us note that in the raw format names you can specify the \
sampling rate and the number of channels. For instance the format 'raw16_le_44100_1' refers to raw files \
using 16 bits integers, little endian, a sampling rate of 44000Hz and a single channel. In order to avoid writing \
very long suffices, you can rename them using this command. For instance, to rename that format into a simple name such as \
'mysnd', you should type 'format mysnd raw16_le_44100_1' (you should put it in your startup so that the format is defined at each runtime). \
Then you can use the '.mysnd' suffix.}}}",


	
  NULL,NULL,NULL
};

static CProcTable  soundTable = {soundCommands, "sound", "Commands related to dealing with sounds."};



/***********************************
 * 
 * Loading/Adding the sound package
 *
 ***********************************/

static void LoadSoundPackage(void)
{
  AddCProcTable(&soundTable);
}


void DeclareSoundPackage(void)
{
  DeclarePackage("sound",LoadSoundPackage,1999,"2.0","E.Bacry and R.Gribonval(for the package and the adaptation of the 'libsndfile' library) and E. de Castro Lopo (for the original 'sndfile' library)",
             "Package allowing to deal with sounds. To learn about this package you should run the corresponding Demos. The read/write procedures have been taken from the 'sndfile' C-library made by \
Erik de Castro Lopo (erikd@zip.com.au). The sound package has a single command 'sound' that mainly allows : 1) to read/write sound files using different formats \
(mainly aiff, wav, au, raw and lw for LastWave file format obtained via the standard read/write commands), 2) to play a sound (on Unix computers you need to configure the system, please just type 'sound play' to learn about \
how to configure it and 3) to record a sound (this feature is available only on Macintosh and Windows computers.");
}

