/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry.                             */
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



extern void XXInitTerminal(void);
extern void XXCloseTerminal(void);
extern void XXTerminalFlush(void);
extern void XXTerminalBeep(void);
extern void  XXTerminalPrintStr(char *str);  
extern void  XXTerminalPrintErrStr(char *str);
extern void  XXTerminalPrintChar(char c);  
extern void  XXTerminalCursorGoForward(int n);
extern void  XXTerminalCursorGoBackward(int n);
extern void  XXTerminalInsertChar(char c,char *str);
extern void  XXTerminalDeleteInsertChar(int n, char *str);
extern void XXTerminalMoveWindow(int x,int y);
extern void XXTerminalResizeWindow(int w,int h);


