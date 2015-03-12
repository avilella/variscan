/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
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

#ifndef COMPUTER_H

#define COMPUTER_H

/********************************************************************/
/*                                                                  */
/*   computer.h :                  computer definition              */
/*                                                                  */
/********************************************************************/

#define MAC

#ifdef HP
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef LINUX
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef DEC
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef SUN
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef SGI
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef RS6000
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef OTHER
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_ANSI_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef CYGWIN
#define LASTWAVE_UNIX_SYSTEM
#define LASTWAVE_WIN32_AUDIO
#define LASTWAVE_X11_GRAPHICS
#undef  MAC
#endif

#ifdef WIN32
#define LASTWAVE_WIN32_SYSTEM
#define LASTWAVE_WIN32_AUDIO
#define LASTWAVE_JAVA_GRAPHICS
#undef  MAC
/* REMI 22/08/2001 : 
 * ugly fixes of some problems when compiling with// Microsoft Visual C++ 6.0 :
 */ 
#undef GetCommandLine
/*   isnan is not defined but _isnan is ! */
#define isnan _isnan
#endif
/*WIN32*/

#ifdef MAC
#define LASTWAVE_MAC_AUDIO
#define LASTWAVE_MAC_GRAPHICS
#define LASTWAVE_MAC_SYSTEM
#endif

#ifdef JAVA
#define LASTWAVE_JAVA_GRAPHICS
#undef LASTWAVE_ANSI_GRAPHICS
#undef LASTWAVE_MAC_GRAPHICS
#undef LASTWAVE_WIN32_GRAPHICS
#endif

#ifdef LASTWAVE_ANSI_GRAPHICS
#undef LASTWAVE_X11_GRAPHICS
#undef LASTWAVE_MAC_GRAPHICS
#undef LASTWAVE_WIN32_GRAPHICS
#undef LASTWAVE_JAVA_GRAPHICS
#endif
/* LASTWAVE_ANSI_GRAPHICS */


#endif


