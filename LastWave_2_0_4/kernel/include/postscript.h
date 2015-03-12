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
/*  postscript.h      The PostScrpit Graphic functions                        */
/*                                                                          */    
/****************************************************************************/

extern char flagPSMode;

#define PSMode (flagPSMode == YES)

extern void PSClose(void);
extern void PSOpen(GOBJECT object,char *filename);
extern void PSSetLineStyle(int flag);
extern void PSSetPenSize(int size);
extern void PSSetPenMode(int mode);
extern void PSSetFont(FONT font);
extern void PSDrawLine(int x1,int y1,int x2,int y2);
extern void PSDrawPoint(int x,int y);
extern void PSDrawEllipse(int x,int y,int w, int h);
extern void PSFillEllipse(int x,int y,int w, int h);
extern void PSDrawRect(int x,int y,int dx,int dy);
extern void PSFillRect(int x,int y,int dx,int dy);
extern void PSDisplayPixMap(int winX,int winY);
extern void PSDrawString(char *str,int hPositionMode,int i,int vPositionMode,int j);
extern void PSInitPixMap(int nRows,int nCols);
extern void PSSetPixelPixMap(int i, int j, unsigned long color);
extern void PSSetColor(unsigned long color);
extern void PSSetClipRect(int x, int y, int w,int h);
extern void PSMaxString(char *str1);
