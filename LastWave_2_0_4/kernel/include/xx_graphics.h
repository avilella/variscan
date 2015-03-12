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



/****************************************************************************/
/*                                                                          */
/*  XX_GRAPHICS.h      The Machine dependent Graphic functions                 */
/*                                                                          */    
/****************************************************************************/

extern void XXOpenGraphics(void);
extern void XXCloseGraphics(void);

extern void XXSetColor(FRAME frame,unsigned long pixel);
extern void XXSetLineStyle(FRAME frame,int flag);
extern void XXSetPenSize(FRAME frame,int size);
extern void XXSetPenMode(FRAME frame,int mode);

extern void XXDrawLine(FRAME frame,int x,int y,int x1,int y1);
extern void XXDrawPoint(FRAME frame,int x,int y);
extern void XXDrawEllipse(FRAME frame,int x,int y,int r1, int r2);
extern void XXFillEllipse(FRAME frame,int x,int y,int r1, int r2);

extern void XXFlush(void);

extern void XXDrawRect(FRAME frame,int x,int y,int dx,int dy);
extern void XXFillRect(FRAME frame,int x,int y,int dx,int dy);
extern void XXSetClipRect(FRAME frame, int x, int y, int w, int h);

extern void XXAllocPixMap(int w,int h,unsigned char **pData,int *pRowBytes);
extern void XXDisplayPixMap(FRAME frame,int winX,int winY);
extern void XXDeletePixMap(void);
extern char XXIsDisplayBLittle(void);

extern void XXChangeFrame(FRAME frame,char *title, int x,int y,int w,int h);
extern void XXDeleteFrame(FRAME frame);
FRAME XXGetFrontFrame(void);
void XXFrontFrame(FRAME frame);
extern FRAME XXNewFrame(char *title, int x,int y,int w,int h);

extern void XXAutoRepeatOn(void);
extern void XXAutoRepeatOff(void);



extern void XXGetNextEvent(EVENT event,int flagWait);

extern int XXSetColormap(unsigned short red[],unsigned short green[],unsigned short blue[],
                         unsigned long pixels[], int nCols,int flagSharedColormap, 
                         int mouseMode,unsigned short mouseRed, unsigned short mouseGreen, unsigned short mouseBlue);

extern void XXAnimateColor(unsigned long c, unsigned short r, unsigned short g, unsigned short b);
extern int XXNumOfColors(void);
extern int XXDepth(void);
extern int XXIsBWScreen(void);
extern char *XXScreenType(void);
extern void XXScreenRect(int *x, int *y, int *w, int *h);

extern void XXBgColor(unsigned short *r,unsigned short *g,unsigned short *b);
extern void XXFgColor(unsigned short *r,unsigned short *g,unsigned short *b);

extern void XXFontMatch(char *name,char flagSize,int size,char flagStyle,unsigned char style);
extern void XXDrawString(FRAME frame,int x,int y,char *str);
extern int XXGetStringWidth(FONT,char *text);
extern char *XXGetDefaultFont(FONTID *id,int *size);
extern void XXGetFontInfo(FONT id, int *ascent, int *descent, int *interline);
extern void XXSetFont(FRAME frame,FONT font);
extern char XXExistFont(char *name,int size, unsigned char style,FONTID *pid);



