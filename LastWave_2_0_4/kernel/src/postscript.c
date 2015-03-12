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
/*  postscript.c      The Postcript window manager                          */
/*                                                                          */    
/****************************************************************************/

#include "lastwave.h"

#define INCH 2.54 /* centimeter */



char flagPSMode = NO;

/* Size of a page in inches */
#define Page_X  8.5  
#define Page_Y  11   


/* Size of the object to be printed (in inches) */
static char flagAdaptYSize = YES;
static float lx = 6.0; 
static float ly = -1;

static float dx = 0; /* position in inches on the paper of the left corner */
static float dy = 0;

static float rx;  /* nb of laser points rx per screen pixels */
static float ry;

static FILE *psStream = NULL; /* current postscript file */
static GOBJECT psObject;          /* Object to be printed */

static int flagInPath = NO; /* for newpath and closepath commands */

static float linewidth = .7;

static FONT currentFont = NULL;

/*
 * General Command to handle postcript
 */
 
void C_PS(char **argv)
{
  char *action;
  float rx,ry,rw,rh;
  LISTV lv;
  
  if (PSMode) Errorf("Sorry cannot use 'ps' command while drawing a postcript file");
  
  /* Get the action name */
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* The 'csize' action  */
  if (!strcmp(action,"csize")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      if (!flagAdaptYSize) {
        AppendFloat2Listv(lv,lx*INCH);
        AppendFloat2Listv(lv,ly*INCH);
      }
      else {
        AppendFloat2Listv(lv,lx*INCH);
        AppendFloat2Listv(lv,-1.);
      }
      return;
    }
    argv = ParseArgv(argv,tFLOAT,&rw,tFLOAT_,-1.0,&rh,0);
    if (rw <= 0 || (rh <=0 && rh != -1)) Errorf("Sorry the size of the drawing should be strictly positive");
    lx = rw/INCH;
    if (rh == -1) {
      flagAdaptYSize = YES;
      ly = -1;
    }
    else {
      flagAdaptYSize = NO;
      ly = rh/INCH;
    }
    return;
  }

  /* The 'isize' action  */
  if (!strcmp(action,"isize")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      if (!flagAdaptYSize) {
        AppendFloat2Listv(lv,lx);
        AppendFloat2Listv(lv,ly);
      }
      else  {
        AppendFloat2Listv(lv,lx);
        AppendFloat2Listv(lv,-1);
      }
      return;
    }
    argv = ParseArgv(argv,tFLOAT,&rw,tFLOAT_,-1.0,&rh,0);
    if (rw <= 0 || (rh <=0 && rh != -1)) Errorf("Sorry the size of the drawing should be strictly positive");
    lx = rw;
    if (rh == -1) {
      flagAdaptYSize = YES;
      ly = -1;
    }
    else {
      flagAdaptYSize = NO;
    ly = rh;
    }
    return;
  }
  
  /* The 'ipos' action  */
  if (!strcmp(action,"ipos")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,dx);
      AppendFloat2Listv(lv,dy);
      return;
    }
    argv = ParseArgv(argv,tFLOAT,&rx,tFLOAT,&ry,0);
    dx = rx;
    dy = ry;
    return;
  }
  
  /* The 'cpos' action  */
  if (!strcmp(action,"cpos")) {
    if (*argv == NULL) {
      lv = TNewListv();
      SetResultValue(lv);
      AppendFloat2Listv(lv,dx*INCH);
      AppendFloat2Listv(lv,dy*INCH);
      return;
    }
    argv = ParseArgv(argv,tFLOAT,&rx,tFLOAT,&ry,0);
    dx = rx/INCH;
    dy = ry/INCH;
    return;
  }

  /* The 'linewidth' action  */
  if (!strcmp(action,"linewidth")) {
    if (*argv == NULL) {
      SetResultFloat(linewidth);
      return;
    }
    argv = ParseArgv(argv,tFLOAT,&rx,0);
    if (rx <= 0) Errorf("The linewidth should be a strictly positive number");
    linewidth = rx;
    return;
  }
  
  Errorf("Unknown action '%s'",action);
}


 
static void PSClosePathIfOpen()
{
  if (flagInPath == YES) {
     fprintf(psStream,"st\n");
     flagInPath = NO;
  }
}

void PSClose(void)
{
  if (flagPSMode != YES) return;

  PSClosePathIfOpen();
  fprintf(psStream,"showpage\n");

  FClose(psStream);
  flagPSMode = NO;
}

/* Start a session  */
void PSOpen(GOBJECT obj,char *filename)
{
  float page_x = Page_X*72;

  if (flagPSMode == YES) {
    Warningf("PSOpen() : I had to close the former postscript file");
    PSClose();
  }
  psObject = obj;

  psStream = FOpen(filename,"w");

  if (psStream == NULL) {
    flagPSMode = NO;
    Errorf("PSOpen() : Failed to open the ps file %s",filename);
  }
  
  /* The size in x has been fixed, now we compute the size in y and the corresponding ratios */
  rx = 72.0*lx/psObject->w;
  if (flagAdaptYSize) {
    ry = rx;
    ly = ry*psObject->h/72.0;
  }
  else ry = 72*ly/psObject->h; 
              
  fprintf(psStream,"%%!PS-Adobe-2.0\n");
  fprintf(psStream,"%%%%Creator : LastWave Kernel 1.5.1\n");
  fprintf(psStream,"%%%%BoundingBox: %d %d %d %d\n",(int) (dx*72-.5),(int) (dy*72-.5),(int) (psObject->w*rx+.5),(int) (psObject->h*ry+.5));
  fprintf(psStream,"/mt {/y exch def /x exch def x y moveto} def\n");
  fprintf(psStream,"/lt {/y exch def /x exch def x y lineto} def\n");
  fprintf(psStream,"/rlt {/y exch def /x exch def x y rlineto} def\n");
  fprintf(psStream,"/np {newpath} def\n");
  fprintf(psStream,"/cp {closepath} def\n");
  fprintf(psStream,"/st {stroke} def\n");
  fprintf(psStream,"/sc {scale} def\n");
  fprintf(psStream,"/fi {fill} def\n");
  fprintf(psStream,"/tr {translate} def\n");
  fprintf(psStream,"/slw {setlinewidth} def\n");
  fprintf(psStream,"/sd {setdash} def\n");
  fprintf(psStream,"/ic {initclip} def\n");
  fprintf(psStream,"/rc {rectclip} def\n");
  fprintf(psStream,"/rightx {exch stringwidth pop sub} def\n");
  fprintf(psStream,"/midx {exch stringwidth pop 2 div sub} def\n");
  fprintf(psStream,"/upy {ascent sub} def\n");
  fprintf(psStream,"/downy {descent add} def\n");
  fprintf(psStream,"/midy {ascent descent sub 2 div sub} def\n");
  fprintf(psStream,"/midupy {ascent 2 div sub} def\n");
  fprintf(psStream, "/cf {selectfont 0 0 mt (|jg~A) false charpath pathbbox /ascent exch def pop -1 mul /descent exch def pop} def\n");

  fprintf(psStream,"%g %g tr\n",dx*72,dy*72);
 
  flagPSMode = YES;
  
  currentFont = NULL;
}

#define GetX(x) (rx*(x)) 
#define GetY(y) ((psObject->h-(y))*ry) 


/***************************************************
 *
 * Postscript version of  the drawing routines     
 *
 ***************************************************/

void PSDrawLine(int x1,int y1,int x2,int y2)
{
  static float oldX,oldY; /* former coordinates */

  if (flagInPath == NO) {
    fprintf(psStream,"newpath\n");
    fprintf(psStream,"%.1f %.1f mt\n",GetX(x1),GetY(y1));
    fprintf(psStream,"%.1f %.1f lt\n",GetX(x2),GetY(y2));
    flagInPath = YES;
  }
  else if (oldX == x1 && oldY == y1) 
    fprintf(psStream,"%.1f %.1f lt\n",GetX(x2),GetY(y2));
  else {
    fprintf(psStream,"%.1f %.1f mt\n",GetX(x1),GetY(y1));
    fprintf(psStream,"%.1f %.1f lt\n",GetX(x2),GetY(y2));
  }

  oldX = x2;
  oldY = y2;
}


static char *maxStr = NULL;
static int maxStrLength = 0;

void PSMaxString(char *str1)
{
  maxStr = str1; 
  maxStrLength = strlen(maxStr);
}


void PSDrawString(char *str,int hPositionMode,int i,int vPositionMode,int j)
{
  char c;

  PSClosePathIfOpen();
 
  /* Let us first fix the horizontal position */
  switch (hPositionMode) {
  
    /* Justify left */
    case HPositionLeftStr : 
      fprintf(psStream,"/sx %.1f def\n",i*rx);
      break;

    /* Justify right each line separately */
    case HPositionRightNStr : 
      fprintf(psStream,"/sx (%s) %.1f rightx def\n",str,i*rx);
      break;

    /* Justify right all the lines as a block */
    case HPositionRight1Str : 
      c = maxStr[maxStrLength];
      maxStr[maxStrLength] = '\0';
      fprintf(psStream,"/sx (%s) %.1f rightx def\n",maxStr,i*rx);
      maxStr[maxStrLength] = c;
      break;
    
    /* Justify middle each line separately */
    case HPositionMiddleNStr : 
      fprintf(psStream,"/sx (%s) %.1f midx def\n",str,i*rx);
      break;

    /* Justify middle all the lines as a block */
    case HPositionMiddle1Str : 
      c = maxStr[maxStrLength];
      maxStr[maxStrLength] = '\0';
      fprintf(psStream,"/sx (%s) %.1f midx def\n",maxStr,i*rx);
      maxStr[maxStrLength] = c;
      break;
  }

  /* Let us then fix the vertical position */
  switch (vPositionMode) {
    
    case VPositionBaseStr:       
      fprintf(psStream,"/sy %.1f def\n",GetY(j));
      break;

    case VPositionUpStr:
      fprintf(psStream,"/sy %.1f upy def\n",GetY(j));
      break;

    case VPositionDownStr:
      fprintf(psStream,"/sy %.1f downy def\n",GetY(j));
      break;

    case VPositionMiddleStr:
      fprintf(psStream,"/sy %.1f midy def\n",GetY(j));
      break;

    case VPositionMiddleUpStr:
      fprintf(psStream,"/sy %.1f midupy def\n",GetY(j));
      break;
  }
  
  fprintf(psStream,"sx sy mt (%s) show \n",str);
}

void PSSetLineStyle(int flag)
{
  PSClosePathIfOpen();
  if (flag == LineDash) fprintf(psStream,"[3] 3 sd\n");
  else fprintf(psStream,"[] 3 sd\n");
}

void PSSetPenSize(int size)
{
  PSClosePathIfOpen();
  fprintf(psStream,"%.1f slw\n",(float) size*rx*linewidth);
}


void PSSetPenMode(int mode)
{
}

void PSSetFont(FONT font)
{ 

  PSClosePathIfOpen();
  
  if (font == currentFont) return;
  currentFont = font;
  
  fprintf(psStream,"/%s",font->fontName);
  if (font->style == FontItalic) fprintf(psStream,"-Oblique");
  if (font->style == FontBold) fprintf(psStream,"-Bold");
  if (font->style == FontBold + FontItalic) fprintf(psStream,"-BoldOblique");

  fprintf(psStream," %d cf\n",font->size); 
}

void PSDrawEllipse(int x,int y, int w, int h)
{ 
  w--;
  h--;
  PSClosePathIfOpen();
  fprintf(psStream,"np\n");
  if (rx*w == ry*h) fprintf(psStream,"%.1f %.1f %.1f 0 360 arc\n",GetX(x+w/2.),GetY(y+h/2.),w*rx/2);
  else {
    fprintf(psStream,"%g %g tr\n",GetX(x+w/2.),GetY(y+h/2.)); 
    fprintf(psStream,"%g %g sc\n",1.0,ry*h/(rx*w)); 
    fprintf(psStream,"%g %g %g 0 360 arc\n",0.,0.,w*rx/2);
    fprintf(psStream,"%g %g sc\n",1.0,rx*w/(ry*h)); 
    fprintf(psStream,"%g %g tr\n",-GetX(x+w/2.),-GetY(y+h/2.)); 
  }
  fprintf(psStream,"cp\n");
  fprintf(psStream,"st\n");
}

void PSFillEllipse(int x,int y, int w, int h)
{ 
  w--;
  h--;
  PSClosePathIfOpen();
  fprintf(psStream,"np\n");
  if (rx*w == ry*h) fprintf(psStream,"%.1f %.1f %.1f 0 360 arc\n",GetX(x+w/2.),GetY(y+h/2.),w*rx/2);
  else {
    fprintf(psStream,"%g %g tr\n",GetX(x+w/2.),GetY(y+h/2.)); 
    fprintf(psStream,"%g %g sc\n",1.0,ry*h/(rx*w)); 
    fprintf(psStream,"%g %g %g 0 360 arc\n",0.,0.,w*rx/2);
    fprintf(psStream,"%g %g sc\n",1.0,rx*w/(ry*h)); 
    fprintf(psStream,"%g %g tr\n",-GetX(x+w/2.),-GetY(y+h/2.)); 
  }
  fprintf(psStream,"cp\n");
  fprintf(psStream,"fi\n");
}

void PSDrawPoint(int x,int y)
{
  PSFillEllipse(x,y,1,1);
}

void PSDrawRect(int x,int y,int dx,int dy)
{ 
  dy--;
  dx--; 
  PSDrawLine(x,y,x+dx,y);
  PSDrawLine(x+dx,y,x+dx,y+dy);
  PSDrawLine(x+dx,y+dy,x,y+dy);
  PSDrawLine(x,y+dy,x,y); 
}

void PSFillRect(int x,int y,int dx,int dy)
{
  dy--;
  dx--;
  PSClosePathIfOpen();
  fprintf(psStream,"np\n");
  fprintf(psStream,"%.1f %.1f mt\n",GetX(x),GetY(y));
  fprintf(psStream,"%.1f %.1f rlt\n",0.,-dy*ry);
  fprintf(psStream,"%.1f %.1f rlt\n",dx*rx,0.);
  fprintf(psStream,"%.1f %.1f rlt\n",0.,dy*ry);
  fprintf(psStream,"cp\n");
  fprintf(psStream,"fi\n");
}


void PSSetColor(unsigned long color)
{
  unsigned short r1,g1,b1;

  PSClosePathIfOpen();
   
  Color2RGB(color,&r1,&g1,&b1);

  fprintf(psStream,"%g %g %g setrgbcolor\n",r1/65535.0,g1/65535.0,b1/65535.0);
}

void PSSetClipRect(int x, int y, int w,int h)
{
  PSClosePathIfOpen();

  y += h;
  
  x-=1;
  y+=1;
  w+=2;
  h+=2;

  fprintf(psStream,"ic\n");
  fprintf(psStream,"%.1f %.1f %.1f %.1f rc\n",GetX(x),GetY(y),w*rx,h*ry);
}


static unsigned char *psPixmapR = NULL;
static unsigned char *psPixmapG = NULL;
static unsigned char *psPixmapB = NULL;
static int nRows;
static int nCols;

static void PSDeletePixMap(void)
{
  if (psPixmapR != NULL) {
    Free(psPixmapR);
    Free(psPixmapG);
    Free(psPixmapB);
    psPixmapR = NULL;
    psPixmapG = NULL;
    psPixmapB = NULL;
  }
}

void PSInitPixMap(int nR,int nC)
{
  PSDeletePixMap();
   
  nRows = nR;
  nCols = nC;
  psPixmapR = (unsigned char *) Calloc(nCols*nRows,sizeof(unsigned char));
  psPixmapG = (unsigned char *) Calloc(nCols*nRows,sizeof(unsigned char));
  psPixmapB = (unsigned char *) Calloc(nCols*nRows,sizeof(unsigned char));
}

void PSSetPixelPixMap(int i, int j, unsigned long color)
{
  unsigned short r,g,b;

  if (i < 0 || i >= nRows) {
    Warningf("PSSetPixelPixMap() : Bad 'i' index %d",i);
    return;
  }
  if (j < 0 || j >= nCols) {
    Warningf("PSSetPixelPixMap() : Bad 'j' index %d",j);
    return;
  }

  Color2RGB(color, &r,&g,&b);

  *(psPixmapR+nCols*i+j) = r/256;
  *(psPixmapG+nCols*i+j) = g/256;	
  *(psPixmapB+nCols*i+j) = b/256;
}


static void PSDisplayImage_(int winX, int winY)
{
  unsigned char *pm;
  int i,j;

  fprintf(psStream,"/chaineR %d string def\n",nCols);  
  fprintf(psStream,"/chaineG %d string def\n",nCols);  
  fprintf(psStream,"/chaineB %d string def\n",nCols);  
  fprintf(psStream,"/setundercolorremoval          where{pop{pop 0}setundercolorremoval}{}ifelse\n");  
  fprintf(psStream,"/setblackoverprint             where{pop true setblackoverprint}{}ifelse\n");  


  fprintf(psStream,"%d %d %d ",nCols,nRows,8);
  fprintf(psStream,"[%g 0 0 %g %d %d]\n",1/rx,-1/ry,-winX,-winY+psObject->h);

  fprintf(psStream,"{currentfile chaineR readhexstring pop}\n");
  fprintf(psStream,"{currentfile chaineG readhexstring pop}\n");
  fprintf(psStream,"{currentfile chaineB readhexstring pop}\n");
  fprintf(psStream,"true      3         colorimage\n");

  /* Loop on each row */
  for(i=0;i<nRows;i++) {
      
    /* Loop on each component */
    pm = psPixmapR;
    while (1) {

      /* Loop on each column */
      for(j=0;j<nCols;j++) {
        fprintf(psStream,"%02x",pm[i*nCols+j]);
      }
    
      if (pm == psPixmapR) pm = psPixmapG;
      else if (pm == psPixmapG) pm = psPixmapB;
      else break;

      fprintf(psStream,"\n\n");
    }

    fprintf(psStream,"\n\n\n");
  }  
} 
 
void PSDisplayPixMap(int winX,int winY)
{
  PSDisplayImage_(winX,winY);
  PSDeletePixMap();
}



