/*
** Copyright (C) 1999 Erik de Castro Lopo <erikd@zip.com.au>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/* sndfile.h -- system-wide definitions */

#ifndef SNDFILE_H
#define SNDFILE_H

#include "computer.h"

#include	<stdio.h>
#include	<stdlib.h>

// REMI:
//Some machine dependent stuff is here,
//shouldn't we put it in computer.h instead ?

/* For the Metrowerks CodeWarrior Pro Compiler (mainly MacOS) */
#ifdef LASTWAVE_MAC_SYSTEM 
#include	<unix.h>
#endif//LASTWAVE_MAC_SYSTEM

#ifdef LASTWAVE_UNIX_SYSTEM
#include	<sys/types.h>
#endif//LASTWAVE_UNIX_SYSTEM

#ifdef LASTWAVE_WIN32_SYSTEM
#include    <sys\types.h>
typedef long off_t;
#endif//LASTWAVE_WIN32_SYSTEM

/* The following file types can be read and written.
** A file type would consist of a major type (ie SF_FORMAT_WAV) bitwise
** ORed with a minor type (ie SF_FORMAT_PCM). SF_FORMAT_TYPEMASK and
** SF_FORMAT_SUBMASK can be used to separate the major and minor file
** types.
*/

enum
{	SF_FORMAT_WAV		= 0x10000,		/* Microsoft WAV format (big endian). */
	SF_FORMAT_AIFF		= 0x20000,		/* Apple/SGI AIFF format (little endian). */
	SF_FORMAT_AU		= 0x30000,		/* Sun/NeXT AU format (big endian). */
	SF_FORMAT_AULE		= 0x40000,		/* DEC AU format (little endian). */
	SF_FORMAT_RAW		= 0x50000,		/* RAW PCM data. */
	SF_FORMAT_PAF		= 0x60000,		/* Ensoniq PARIS file format. */
	SF_FORMAT_SVX		= 0x70000,		/* Amiga IFF / SVX8 / SV16 format. */
	SF_FORMAT_NIST		= 0x80000,		/* Sphere NIST format. */
	SF_FORMAT_LW		= 0x90000,		/* LastWave format. */
	
	SF_FORMAT_PCM		= 0x0001,		/* PCM data in 8, 16, 24 or 32 bits. */
	SF_FORMAT_FLOAT		= 0x0002,		/* 32 bit floats. */
	SF_FORMAT_ULAW		= 0x0003,		/* U-Law encoded. */
	SF_FORMAT_ALAW		= 0x0004,		/* A-Law encoded. */
	SF_FORMAT_IMA_ADPCM = 0x0005,		/* IMA ADPCM. */
	SF_FORMAT_MS_ADPCM  = 0x0006,		/* Microsoft ADPCM. */

	SF_FORMAT_PCM_BE	= 0x0007,		/* Big endian PCM data. */
	SF_FORMAT_PCM_LE  	= 0x0008,		/* Little endian PCM data. */
	SF_FORMAT_PCM_S8	= 0x0009,		/* Signed 8 bit PCM. */
	SF_FORMAT_PCM_U8  	= 0x000A,		/* Unsigned 8 bit PCM. */
	
	SF_FORMAT_SVX_FIB	= 0x000B, 		/* SVX Fibonacci Delta encoding. */
	SF_FORMAT_SVX_EXP	= 0x000C, 		/* SVX Exponential Delta encoding. */
	
	SF_FORMAT_SUBMASK	= 0xFFFF,		
	SF_FORMAT_TYPEMASK	= 0x7FFF0000
} ;

#define	SF_FORMAT_RAW_BE	SF_FORMAT_PCM_BE
#define	SF_FORMAT_RAW_LE	SF_FORMAT_PCM_LE
#define	SF_FORMAT_RAW_S8	SF_FORMAT_PCM_S8
#define	SF_FORMAT_RAW_U8	SF_FORMAT_PCM_U8

/* A SNDFILE* pointer can be passed around much like stdio.h's FILE* pointer. */

typedef	void	SNDFILE ;

/* A pointer to a SF_INFO structure is passed to sf_open_read () and filled in. 
** On write, the SF_INFO structure is filled in by the user and passed into  
** sf_open_write ().
*/

typedef	struct
{	unsigned int	samplerate ;
	unsigned int	samples ;
	unsigned int	channels ;
	unsigned int	pcmbitwidth ;
	unsigned int	format ;
	unsigned int	sections ;
	unsigned int	seekable ;
} SF_INFO ;


/* Open the specified file for read or write. On error, this will
** return a NULL pointer. To find the error number, pass NULL to
** sf_error ().
*/

SNDFILE* 	sf_open_read	(const char *path, SF_INFO *wfinfo) ;
SNDFILE* 	sf_open_write	(const char *path, const SF_INFO *wfinfo) ;

/* Return the current error for SNDFILE or zero for no error. */

int		sf_perror		(SNDFILE *sndfile) ;
int		sf_error_str	(SNDFILE *sndfile, char* str, size_t len) ;

size_t	sf_get_header_info	(SNDFILE *sndfile, char* buffer, size_t bufferlen, size_t offset) ;
size_t	sf_get_info			(SNDFILE *sndfile, char* buffer, size_t bufferlen, size_t offset) ;

/* Return TRUE if format is a valid combination of SF_FORMAT_* values. */

int		sf_format_check	(const SF_INFO *info) ;

/* Return the maximum absolute sample value in the SNDFILE. */

double	sf_signal_max	(SNDFILE *sndfile) ;

/* Seek within the waveform data chunk of the SNDFILE. sf_seek () uses 
** the same values for whence (SEEK_SET, SEEK_CUR and SEEK_END) as
** stdio.h functions lseek () and fseek ().
** An offset of zero with whence set to SEEK_SET will position the 
** read / write pointer to the first data sample.
** On success sf_seek returns the current position in (multi-channel) 
** samples from the start of the file.
** On error sf_seek returns -1.
*/

off_t	sf_seek 		(SNDFILE *sndfile, off_t frames, int whence) ;

/* Functions for reading/writing the waveform data of a sound file.
*/

size_t	sf_read_raw		(SNDFILE *sndfile, void *ptr, size_t bytes) ;
size_t	sf_write_raw 	(SNDFILE *sndfile, void *ptr, size_t bytes) ;

/* Functions for reading and writing the data chunk in terms of frames. 
** The number of items actually read/written = frames * number of channels.
**     sf_xxxx_raw		read/writes the raw data bytes from/to the file
**     sf_xxxx_uchar	passes data in the unsigned char format
**     sf_xxxx_char		passes data in the signed char format
**     sf_xxxx_short	passes data in the native short format
**     sf_xxxx_int		passes data in the native int format
**     sf_xxxx_float	passes data in the native float format
**     sf_xxxx_double	passes data in the native double format
** For the double format, if the normalize flag is TRUE, the read/write 
** operations will use floats/doubles in the rangs [-1.0 .. 1.0] to 
** represent the minimum and maximum values of the waveform irrespective
** of the bitwidth of the input/output file.
** All of these read/write function return number of frames read/written.
*/

size_t	sf_readf_short	(SNDFILE *sndfile, short *ptr, size_t frames) ;
size_t	sf_writef_short	(SNDFILE *sndfile, short *ptr, size_t frames) ;

size_t	sf_readf_int	(SNDFILE *sndfile, int *ptr, size_t frames) ;
size_t	sf_writef_int 	(SNDFILE *sndfile, int *ptr, size_t frames) ;

size_t	sf_readf_double	(SNDFILE *sndfile, double *ptr, size_t frames, int normalize) ;
size_t	sf_writef_double(SNDFILE *sndfile, double *ptr, size_t frames, int normalize) ;

/* Functions for reading and writing the data chunk in terms of items. 
** Otherwise similar to above.
** All of these read/write function return number of items read/written.
*/

size_t	sf_read_short	(SNDFILE *sndfile, short *ptr, size_t items) ;
size_t	sf_write_short	(SNDFILE *sndfile, short *ptr, size_t items) ;

size_t	sf_read_int		(SNDFILE *sndfile, int *ptr, size_t items) ;
size_t	sf_write_int 	(SNDFILE *sndfile, int *ptr, size_t items) ;

size_t	sf_read_float	(SNDFILE *sndfile, float *ptr, size_t items) ;
size_t	sf_write_float 	(SNDFILE *sndfile, float *ptr, size_t items) ;

size_t	sf_read_double	(SNDFILE *sndfile, double *ptr, size_t items, int normalize) ;
size_t	sf_write_double	(SNDFILE *sndfile, double *ptr, size_t items, int normalize) ;

/* Close the SNDFILE. Returns 0 on success, or an error number. */

int		sf_close		(SNDFILE *sndfile) ;

#endif	/* SNDFILE_H */




