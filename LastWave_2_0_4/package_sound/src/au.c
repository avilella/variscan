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


#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"
#include	"pcm.h"
#include	"ulaw.h"
#include	"alaw.h"


/*------------------------------------------------------------------------------
** Macros to handle big/little endian issues.
*/

#	define	MAKE_MARKER(a,b,c,d)  \
       (IsCPULittleEndian ? ((a)|((b)<<8)|((c)<<16)|((d)<<24)) : (((a)<<24)|((b)<<16)|((c)<<8)|(d)))

#define DOTSND_MARKER	(MAKE_MARKER ('.', 's', 'n', 'd')) 
#define DNSDOT_MARKER	(MAKE_MARKER ('d', 'n', 's', '.')) 

/*------------------------------------------------------------------------------
** Typedefs.
*/
 
typedef	struct
{	int		dataoffset ;
	int		datasize ;
	int		encoding ;
    int		samplerate ;
    int		channels ;
} AU_FMT ;


/*------------------------------------------------------------------------------
** Private static functions.
*/

static	int	__au_close			(SF_PRIVATE *psf) ;

static
int get_encoding (unsigned int format, unsigned int	bitwidth)
{	if (format == SF_FORMAT_ULAW)
		return 1 ;
		
	if (format == SF_FORMAT_ALAW)
		return 27 ;

	if (format != SF_FORMAT_PCM)
		return 0 ;

	switch (bitwidth)
	{	case	8  : return 2 ;
		case	16 : return 3 ;
		case	24 : return 4 ;
		case	32 : return	5 ;
		default : break ;
		} ;
	return 0 ;
} /* get encoding */

static
void	endswap_au_fmt (AU_FMT *pau_fmt)
{	pau_fmt->dataoffset = ENDSWAP_INT (pau_fmt->dataoffset) ;
	pau_fmt->datasize   = ENDSWAP_INT (pau_fmt->datasize) ;
	pau_fmt->encoding   = ENDSWAP_INT (pau_fmt->encoding) ;
    pau_fmt->samplerate = ENDSWAP_INT (pau_fmt->samplerate) ;
    pau_fmt->channels   = ENDSWAP_INT (pau_fmt->channels) ;
} /* endswap_au_fmt */

/*------------------------------------------------------------------------------
** Public functions.
*/

int 	__au_open_read	(SF_PRIVATE *psf)
{	AU_FMT			au_fmt ;
	unsigned int	marker, dword ;
	int				big_endian_file ;
	
	fread (&marker, sizeof (marker), 1, psf->file) ;
	if (marker == DOTSND_MARKER)
		big_endian_file = 1 ;
	else if (marker == DNSDOT_MARKER)
		big_endian_file = 0 ;
	else
		return SFE_AU_NO_DOTSND ;
		
	__psf_sprintf (psf, "%D\n", marker) ;
	
	fread (&au_fmt, sizeof (AU_FMT), 1, psf->file) ;
	
	if (__CPU_IS_LITTLE_ENDIAN__ && big_endian_file)
		endswap_au_fmt (&au_fmt) ;
	else if (__CPU_IS_BIG_ENDIAN__ && ! big_endian_file)
		endswap_au_fmt (&au_fmt) ;

	__psf_sprintf (psf, "  Data Offset : %d\n", au_fmt.dataoffset) ;
	
	if (au_fmt.dataoffset + au_fmt.datasize != psf->filelength)
	{	dword = psf->filelength - au_fmt.dataoffset ;
		__psf_sprintf (psf, "  Data Size   : %d (should be %d)\n", au_fmt.datasize, dword) ;
		au_fmt.datasize = dword ;
		}
	else
		__psf_sprintf (psf, "  Data Size   : %d\n", au_fmt.datasize) ;
		
 	psf->dataoffset = au_fmt.dataoffset ;

 	psf->current  = 0 ;
	psf->endian   = big_endian_file ? SF_ENDIAN_BIG : SF_ENDIAN_LITTLE ;
	psf->sf.seekable = SF_TRUE ;
 	
 	if (fseek (psf->file, psf->dataoffset, SEEK_SET))
		return SFE_BAD_SEEK ;

	psf->close = (func_close) __au_close ;
	
	psf->sf.samplerate	= au_fmt.samplerate ;
	psf->sf.channels 	= au_fmt.channels ;
					
	/* Only fill in type major. */
	psf->sf.format = big_endian_file ? SF_FORMAT_AU : SF_FORMAT_AULE ;

	psf->sf.sections 	= 1 ;

	__psf_sprintf (psf, "  Encoding    : ") ;

	switch (au_fmt.encoding)
	{	case  1 :	__psf_sprintf (psf, "%d => %s\n", au_fmt.encoding, "8-bit ISDN u-law") ;
					psf->sf.pcmbitwidth = 16 ;	/* After decoding */
					psf->bytewidth   	= 1 ;	/* Before decoding */
					
					psf->sf.format |= SF_FORMAT_ULAW ;
					
					psf->read_short  = (func_short)  __ulaw_read_ulaw2s ;
					psf->read_int    = (func_int)    __ulaw_read_ulaw2i ;
					psf->read_double = (func_double) __ulaw_read_ulaw2d ;
					break ;
										
		case  2 :	__psf_sprintf (psf, "%d => %s\n", au_fmt.encoding, "8-bit linear PCM") ;
					psf->sf.pcmbitwidth = 8 ;
					psf->bytewidth      = BITWIDTH2BYTES (psf->sf.pcmbitwidth) ;

					psf->sf.format |= SF_FORMAT_PCM ;
					
					psf->read_short  = (func_short)  __pcm_read_sc2s ;
					psf->read_int    = (func_int)    __pcm_read_sc2i ;
					psf->read_double = (func_double) __pcm_read_sc2d ;
					break ;

		case  3 :	__psf_sprintf (psf, "%d => %s\n", au_fmt.encoding, "16-bit linear PCM") ;
					psf->sf.pcmbitwidth = 16 ;
					psf->bytewidth      = BITWIDTH2BYTES (psf->sf.pcmbitwidth) ;

					psf->sf.format |= SF_FORMAT_PCM ;
					
					if (big_endian_file)
					{	psf->read_short  = (func_short)  __pcm_read_bes2s ;
						psf->read_int    = (func_int)    __pcm_read_bes2i ;
						psf->read_double = (func_double) __pcm_read_bes2d ;
						}
					else
					{	psf->read_short  = (func_short)  __pcm_read_les2s ;
						psf->read_int    = (func_int)    __pcm_read_les2i ;
						psf->read_double = (func_double) __pcm_read_les2d ;
						} ;
					break ;

		case  4 :	__psf_sprintf (psf, "%d => %s\n", au_fmt.encoding, "24-bit linear PCM") ;
					psf->sf.pcmbitwidth = 24 ;
					psf->bytewidth      = BITWIDTH2BYTES (psf->sf.pcmbitwidth) ;

					psf->sf.format |= SF_FORMAT_PCM ;
					
					if (big_endian_file)
					{	psf->read_short  = (func_short)  __pcm_read_bet2s ;
						psf->read_int    = (func_int)    __pcm_read_bet2i ;
						psf->read_double = (func_double) __pcm_read_bet2d ;
						}
					else
					{	psf->read_short  = (func_short)  __pcm_read_let2s ;
						psf->read_int    = (func_int)    __pcm_read_let2i ;
						psf->read_double = (func_double) __pcm_read_let2d ;
						} ;
					break ;

		case  5 :	__psf_sprintf (psf, "%d => %s\n", au_fmt.encoding, "32-bit linear PCM") ;
					psf->sf.pcmbitwidth = 32 ;
					psf->bytewidth      = BITWIDTH2BYTES (psf->sf.pcmbitwidth) ;
					
					psf->sf.format |= SF_FORMAT_PCM ;
					
					if (big_endian_file)
					{	psf->read_short  = (func_short)  __pcm_read_bei2s ;
						psf->read_int    = (func_int)    __pcm_read_bei2i ;
						psf->read_double = (func_double) __pcm_read_bei2d ;
						}
					else
					{	psf->read_short  = (func_short)  __pcm_read_lei2s ;
						psf->read_int    = (func_int)    __pcm_read_lei2i ;
						psf->read_double = (func_double) __pcm_read_lei2d ;
						} ;
					break ;
					
		case  27 :	__psf_sprintf (psf, "%d => %s\n", au_fmt.encoding, "8-bit ISDN A-law") ;
					psf->sf.pcmbitwidth = 16 ;	/* After decoding */
					psf->bytewidth   	= 1 ;	/* Before decoding */
					
					psf->sf.format |= SF_FORMAT_ALAW ;
					
					psf->read_short  = (func_short)  __alaw_read_alaw2s ;
					psf->read_int    = (func_int)    __alaw_read_alaw2i ;
					psf->read_double = (func_double) __alaw_read_alaw2d ;
					break ;
										
		default :   __psf_sprintf (psf, "%d => Unknown!!\n", au_fmt.encoding) ;
					return SFE_AU_UNKNOWN_FORMAT ;
					break ;
		} ;

	__psf_sprintf (psf, "  Sample Rate : %d\n", au_fmt.samplerate) ;
	__psf_sprintf (psf, "  Channels    : %d\n", au_fmt.channels) ;

	psf->blockwidth = psf->sf.channels * psf->bytewidth ;

	if (psf->blockwidth)
		psf->sf.samples = au_fmt.datasize / psf->blockwidth ;

	psf->datalength = psf->filelength - psf->dataoffset ;

	return 0 ;
} /* __au_open_read */

/*------------------------------------------------------------------------------
*/

int 	__au_open_write	(SF_PRIVATE *psf)
{	AU_FMT			au_fmt ;
	unsigned int	dword, encoding, format, subformat, big_endian_file ;
	
	format = psf->sf.format & SF_FORMAT_TYPEMASK ;
	subformat = psf->sf.format & SF_FORMAT_SUBMASK ;
	if (format == SF_FORMAT_AU)
		big_endian_file = 1 ;
	else if (format == SF_FORMAT_AULE)
		big_endian_file = 0 ;
	else
		return	SFE_BAD_OPEN_FORMAT ;
		
	if (subformat == SF_FORMAT_ULAW || subformat == SF_FORMAT_ALAW)
		psf->bytewidth = 1 ;
	else
		psf->bytewidth = BITWIDTH2BYTES (psf->sf.pcmbitwidth) ;
		
	psf->endian      = big_endian_file ? SF_ENDIAN_BIG : SF_ENDIAN_LITTLE ;
	psf->sf.seekable = SF_TRUE ;
	psf->blockwidth  = psf->bytewidth * psf->sf.channels ;
 	psf->dataoffset  = 6 * sizeof (dword) ;
	psf->datalength  = psf->blockwidth * psf->sf.samples ;
	psf->filelength  = psf->datalength + psf->dataoffset ;
	psf->error       = 0 ;

	encoding = get_encoding (subformat, psf->bytewidth * 8) ;
	if (! encoding)
		return	SFE_BAD_OPEN_FORMAT ;

	au_fmt.dataoffset = 24 ;
	au_fmt.datasize   = psf->datalength ;
	au_fmt.encoding   = encoding ;
	au_fmt.samplerate = psf->sf.samplerate ;
	au_fmt.channels   = psf->sf.channels ;
	
	if (__CPU_IS_LITTLE_ENDIAN__ && big_endian_file)
		endswap_au_fmt (&au_fmt) ;
	else if (__CPU_IS_BIG_ENDIAN__ && ! big_endian_file)
		endswap_au_fmt (&au_fmt) ;
	
	
	dword = big_endian_file ? DOTSND_MARKER : DNSDOT_MARKER ;	/* Marker */
	fwrite (&dword, sizeof (dword), 1, psf->file) ;
	
	fwrite (&au_fmt, sizeof (AU_FMT), 1, psf->file) ;
	
	psf->close = (func_close) __au_close ;
	
	switch (encoding)
	{	case  1 :	/* 8-bit Ulaw encoding. */
					psf->write_short  = (func_short)  __ulaw_write_s2ulaw ;
					psf->write_int    = (func_int)    __ulaw_write_i2ulaw ;
					psf->write_double = (func_double) __ulaw_write_d2ulaw ;
					break ;
	
		case  2 :	/* 8-bit linear PCM. */
					psf->write_short  = (func_short)  __pcm_write_s2sc ;
					psf->write_int    = (func_int)    __pcm_write_i2sc ;
					psf->write_double = (func_double) __pcm_write_d2sc ;
					break ;

		case  3 :	/* 16-bit linear PCM. */
					if (big_endian_file)
					{	psf->write_short  = (func_short)  __pcm_write_s2bes ;
						psf->write_int    = (func_int)    __pcm_write_i2bes ;
						psf->write_double = (func_double) __pcm_write_d2bes ;
						}
					else
					{	psf->write_short  = (func_short)  __pcm_write_s2les ;
						psf->write_int    = (func_int)    __pcm_write_i2les ;
						psf->write_double = (func_double) __pcm_write_d2les ;
						} ;
					break ;

		case  4 :	/* 24-bit linear PCM */
					if (big_endian_file)
					{	psf->write_short  = (func_short)  __pcm_write_s2bet ;
						psf->write_int    = (func_int)    __pcm_write_i2bet ;
						psf->write_double = (func_double) __pcm_write_d2bet ;
						}
					else
					{	psf->write_short  = (func_short)  __pcm_write_s2let ;
						psf->write_int    = (func_int)    __pcm_write_i2let ;
						psf->write_double = (func_double) __pcm_write_d2let ;
						} ;
					break ;

		case  5 :	/* 32-bit linear PCM. */
					if (big_endian_file)
					{	psf->write_short  = (func_short)  __pcm_write_s2bei ;
						psf->write_int    = (func_int)    __pcm_write_i2bei ;
						psf->write_double = (func_double) __pcm_write_d2bei ;
						}
					else
					{	psf->write_short  = (func_short)  __pcm_write_s2lei ;
						psf->write_int    = (func_int)    __pcm_write_i2lei ;
						psf->write_double = (func_double) __pcm_write_d2lei ;
						} ;
					break ;
					
		case  27 :	/* 8-bit Alaw encoding. */
					psf->write_short  = (func_short)  __alaw_write_s2alaw ;
					psf->write_int    = (func_int)    __alaw_write_i2alaw ;
					psf->write_double = (func_double) __alaw_write_d2alaw ;
					break ;
	
		default :   break ;
		} ;
		
	return 0 ;
} /* au_open_write */

/*------------------------------------------------------------------------------
*/

int	__au_close	(SF_PRIVATE  *psf)
{	unsigned int	dword ;

	if (psf->mode == SF_MODE_WRITE)
	{	/*  Now we know for certain the length of the file we can
		 *  re-write correct values for the datasize header element.
		 */

		fseek (psf->file, 0, SEEK_END) ;
		psf->filelength = ftell (psf->file) ;

		psf->datalength = psf->filelength - psf->dataoffset ;
		fseek (psf->file, 2 * sizeof (dword), SEEK_SET) ;
		
		if (psf->endian == SF_ENDIAN_BIG)
			dword = H2BE_INT (psf->datalength) ;
		else if (psf->endian == SF_ENDIAN_LITTLE)
			dword = H2LE_INT (psf->datalength) ;
		else
			dword = 0xFFFFFFFF ;
		fwrite (&dword, sizeof (dword), 1, psf->file) ;
		} ;

	if (psf->fdata)
		free (psf->fdata) ;
	psf->fdata = NULL ;
	
	return 0 ;
} /* __au_close */


/*=========================================================================
*/

