/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    1 . 5                                            */
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




/****************************************************************************/
/*                                                                          */
/*  terminal.h   This file contains the functons for managing the terminal  */
/*                                                                          */
/****************************************************************************/


/*
 * Two simple functions to set or get the graphic mode
 */
extern char flagGraphicMode;
extern void SetGraphicMode(char);
#define GraphicMode (flagGraphicMode == YES)


/*
 * Initialization and closing of the terminal
 */
extern void TerminalInit(void);
extern void TerminalClose(void);


/* fflush type function */
extern void Flush(void);

/* Beep function */
extern void Beep(void);


#include "xx_terminal.h"









