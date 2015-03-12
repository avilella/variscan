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




/****************************************************************************/
/*                                                                          */
/*  terminal.c   This file contains the functons for managing the terminal  */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include <stdarg.h>
#include <time.h>









/*************************************************************************
 *
 *               Init and Close the terminal
 *
 ************************************************************************/

/* Initialization of the terminal */
void InitTerminal(void)
{
  XXInitTerminal();
}

/* Closing of the terminal */
void CloseTerminal(void)
{
  XXCloseTerminal();
}


/*************************************************************************
 *
 *               Manage the batchmode and graphic flags
 *
 ************************************************************************/

char flagGraphicMode = YES;

/* Set the batch mode */
void SetGraphicMode(char flag)
{
  flagGraphicMode = flag;
}




/*
 * Make a beep
 */
void Beep()
{
  XXTerminalBeep();
}

/*
 * Flushing the terminal output (both the output and error streams)
 */
void Flush()
{
  XXTerminalFlush();
}




