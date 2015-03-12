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

#include	<stdarg.h>
#include	<string.h>

#include	"sndfile.h"
#include	"sfendian.h"
#include	"common.h"

/*-----------------------------------------------------------------------------------------------
 */

void	__endswap_short_array (short *ptr, int len)
{	int k ;
	for (k = 0 ; k < len ; k++)
		ptr[k] = ((((ptr[k])>>8)&0xFF)|(((ptr[k])&0xFF)<<8)) ;
} /* __endswap_short_array */

void	__endswap_int_array (int *ptr, int len)
{	int k ;
	for (k = 0 ; k < len ; k++)
		ptr[k] = ((((ptr[k])>>24)&0xFF)|(((ptr[k])>>8)&0xFF00)|
					(((ptr[k])&0xFF00)<<8)|(((ptr[k])&0xFF)<<24)) ;		
} /* __endswap_int_array */

/*-----------------------------------------------------------------------------------------------
 */

#define __psf_putchar(a,b)									\
			if ((a)->strindex < SF_BUFFER_LEN - 1)			\
			{	(a)->strbuffer [(a)->strindex++] = (b) ;	\
				(a)->strbuffer [(a)->strindex] = 0 ;		\
				} ;

void __psf_sprintf (SF_PRIVATE *psf, char *format, ...)
{	va_list	ap ;
	int     d, tens, shift ;
	char    c, *strptr, istr [5] ;

	va_start(ap, format);
	
	/* printf ("__psf_sprintf : %s\n", format) ; */
	
	while ((c = *format++))
	{	if (c != '%')
		{	__psf_putchar (psf, c) ;
			continue ;
			} ;
		
		switch((c = *format++)) 
		{	case 's': /* string */
					strptr = va_arg (ap, char *) ;
					while (*strptr)
						__psf_putchar (psf, *strptr++) ;
					break;
		    
			case 'd': /* int */
					d = va_arg (ap, int) ;

					if (d == 0)
					{	__psf_putchar (psf, '0') ;
						break ;
						} 
					if (d < 0)
					{	__psf_putchar (psf, '-') ;
						d = -d ;
						} ;
					tens = 1 ;
					while (d / tens >= 10) 
						tens *= 10 ;
					while (tens > 0)
					{	__psf_putchar (psf, '0' + d / tens) ;
						d %= tens ;
						tens /= 10 ;
						} ;
					break;
					
			case 'X': /* hex */
					d = va_arg (ap, int) ;
					
					if (d == 0)
					{	__psf_putchar (psf, '0') ;
						break ;
						} ;
					shift = 28 ;
					while (! ((0xF << shift) & d))
						shift -= 4 ;
					while (shift >= 0)
					{	c = (d >> shift) & 0xF ;
						__psf_putchar (psf, (c > 9) ? c + 'A' - 10 : c + '0') ;
						shift -= 4 ;
						} ;
					break;
					
			case 'c': /* char */
			  //REMI 					c = va_arg (ap, char) ;
					c = (char)va_arg (ap, int) ;
					__psf_putchar (psf, c);
					break;
					
			case 'D': /* int2str */
					d = va_arg (ap, int);
					*((int*) istr) = d ;
					istr [4] = 0 ;
					strptr = &(istr [0]) ;
					while (*strptr)
					{	c = *strptr++ ;
						__psf_putchar (psf, c) ;
						} ;
					break;
					
			default :
					__psf_putchar (psf, '?') ;
					__psf_putchar (psf, c) ;
					__psf_putchar (psf, '?') ;
					break ;
			} /* switch */
		} /* while */

	va_end(ap);
	return ;
} /* __psf_sprintf */

/*------------------------------------------------------------------------------
**  Format specifiers for __psf_hprintf are as follows
**		m	- marker - four bytes - no endian problems
**		w	- two byte value - little endian
**		W	- two byte value - big endian
**		l	- four byte value - little endian
**		L	- four byte value - big endian
**		s   - string preceded by a little endian four byte length
**		S   - string preceded by a big endian four byte length
**		b	- binary data (see below)
**
**	To write a word followed by a long (both little endian) use:
**		__psf_hprintf ("wl", wordval, longval) ;
**
**	To write binary data use:
**		__psf_hprintf ("b", &bindata, sizeof (bindata)) ;
*/

/* These macros may seem a bit messy but do prevent problems with processors which 
** seg. fault when asked to write an int or short to a non-int/short aligned address.
*/

#define	put_int(psf,x)	{if (IsCPULittleEndian) 	{	(psf)->header [(psf)->headindex++] = (x) & 0xFF ;				\
								(psf)->header [(psf)->headindex++] = ((x) >>  8) & 0xFF ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) & 0xFF ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 24) & 0xFF ;   } \
						  else { \
						    (psf)->header [(psf)->headindex++] = ((x) >> 24) & 0xFF ;		\
								(psf)->header [(psf)->headindex++] = ((x) >> 16) & 0xFF ;		\
								(psf)->header [(psf)->headindex++] = ((x) >>  8) & 0xFF ;		\
								(psf)->header [(psf)->headindex++] = (x) & 0xFF ;   } \
						}
                                                                        
#define	put_short(psf,x)	{if (IsCPULittleEndian) 	{	(psf)->header [(psf)->headindex++] = (x) & 0xFF ;				\
								(psf)->header [(psf)->headindex++] = ((x) >> 8) & 0xFF ;   } \
							 else {	(psf)->header [(psf)->headindex++] = ((x) >> 8) & 0xFF ;		\
								(psf)->header [(psf)->headindex++] = (x) & 0xFF ;   }}


void	__psf_hprintf (SF_PRIVATE *psf, char *format, ...)
{	va_list	argptr ;
	unsigned int 	longdata ;
	unsigned short	worddata ;
	void	*bindata ;
	size_t	size ;
	char    c, *strptr ;
	
	va_start(argptr, format);
	
	while ((c = *format++))
	{	switch (c)
		{	case 'm' : 
					longdata = va_arg (argptr, unsigned int) ;
					put_int (psf, longdata) ;
					break ;
					
			case 'l' :
					longdata = va_arg (argptr, unsigned int) ;
					longdata = H2LE_INT (longdata) ;
					put_int (psf, longdata) ;
					break ;

			case 'L' :
					longdata = va_arg (argptr, unsigned int) ;
					longdata = H2BE_INT (longdata) ;
					put_int (psf, longdata) ;
					break ;
					
			case 'w' :
			  //REMI					worddata = va_arg (argptr, unsigned short) ;
					worddata = (unsigned short)va_arg (argptr, int) ;
					worddata = H2LE_SHORT (worddata) ;
					put_short (psf, worddata) ;
					break ;

			case 'W' :
			  //REMI					worddata = va_arg (argptr, unsigned short) ;
					worddata = (unsigned short)va_arg (argptr, int) ;
					worddata = H2BE_SHORT (worddata) ;
					put_short (psf, worddata) ;
					break ;
					
			case 'b' :
					bindata = va_arg (argptr, void *) ;
					size    = va_arg (argptr, size_t) ;
					memcpy (&(psf->header [psf->headindex]), bindata, size) ;
					psf->headindex += size ;
					break ;
					
			case 's' :
					strptr = va_arg (argptr, char *) ;
					size    = strlen (strptr) + 1 ;
					size   += (size & 1) ;
					longdata = H2LE_INT (size) ;
					put_int (psf, longdata) ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					break ;
					
			case 'S' :
					strptr = va_arg (argptr, char *) ;
					size    = strlen (strptr) + 1 ;
					size   += (size & 1) ;
					longdata = H2BE_INT (size) ;
					put_int (psf, longdata) ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					break ;
					
			default : break ;
			} ;
		} ;
	
	va_end(argptr);
	return ;
} /* __psf_hprintf */


/*-----------------------------------------------------------------------------------------------
*/

void	__psf_hsetf (SF_PRIVATE *psf, unsigned int marker, char *format, ...)
{	va_list	argptr ;
	unsigned int 	longdata, oldheadindex ;
	unsigned short	worddata ;
	void	*bindata ;
	size_t	size ;
	char    c, *strptr ;
	
	/* Save old head index. */
	oldheadindex = psf->headindex ;
	psf->headindex = 0 ;
	
	/* Find the marker. */
	while (psf->headindex < oldheadindex)
	{	if (*((unsigned int*) &(psf->header[psf->headindex])) == marker)
			break ;
		psf->headindex += 4 ;
		} ;
		
	/* If not found return. */
	if (psf->headindex >= oldheadindex)
		return ;

	/* Move past marker. */
	psf->headindex += 4 ;

	va_start(argptr, format);
	
	while ((c = *format++))
	{	switch (c)
		{	case 'm' : 
					longdata = va_arg (argptr, unsigned int) ;
					put_int (psf, longdata) ;
					break ;
					
			case 'l' :
					longdata = va_arg (argptr, unsigned int) ;
					longdata = H2LE_INT (longdata) ;
					put_int (psf, longdata) ;
					break ;

			case 'L' :
					longdata = va_arg (argptr, unsigned int) ;
					longdata = H2BE_INT (longdata) ;
					put_int (psf, longdata) ;
					break ;
					
			case 'w' :
			  //REMI					worddata = va_arg (argptr, unsigned short) ;
					worddata = (unsigned short)va_arg (argptr, int) ;
					worddata = H2LE_SHORT (worddata) ;
					put_short (psf, worddata) ;
					break ;

			case 'W' :
			  //REMI					worddata = va_arg (argptr, unsigned short) ;
					worddata = (unsigned short)va_arg (argptr, int) ;
					worddata = H2BE_SHORT (worddata) ;
					put_short (psf, worddata) ;
					break ;
					
			case 'b' :
					bindata = va_arg (argptr, void *) ;
					size    = va_arg (argptr, size_t) ;
					memcpy (&(psf->header [psf->headindex]), bindata, size) ;
					psf->headindex += size ;
					break ;
					
			case 's' :
					strptr = va_arg (argptr, char *) ;
					size    = strlen (strptr) + 1 ;
					size   += (size & 1) ;
					longdata = H2LE_INT (size) ;
					put_int (psf, longdata) ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					break ;
					
			case 'S' :
					strptr = va_arg (argptr, char *) ;
					size    = strlen (strptr) + 1 ;
					size   += (size & 1) ;
					longdata = H2BE_INT (size) ;
					put_int (psf, longdata) ;
					memcpy (&(psf->header [psf->headindex]), strptr, size) ;
					psf->headindex += size ;
					break ;
					
			default : break ;
			} ;
		} ;
	
	va_end(argptr);
	
	psf->headindex = oldheadindex ;
	return ;
} /* __psf_hsetf */
