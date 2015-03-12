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
/*  color.c   Functions which deal with the colors                          */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"




/****************************************************************
 *
 *    The color structures
 *
 ****************************************************************/
 

/* 
 * A color will be adressed by a unsigned long of the form
 *
 *    cciiiiii
 *
 * where each letter represents a group of 4 bits.
 * The colormaps  are coded on the high byte (cc) and the iiiiii codes the color index inside this colormap. 
 * Actually the colormaps are coded just in the 7 high bits of 'cc'. The higher bit is used for inversing a 
 * color map (thus there are 128 possible colormaps which are decomposed as 64 color maps and their inversed).
 * The lowest bit of 'cc' is used to code the color. It is always off unless the color corresponds to the special
 * value 'invisible'. 
 *
 * When parsing a color the whole 'cciiiiii' is returned. When parsing a colormap the unsigned long
 * 'cc000000' is returned (not just 'cc') so that you can just add a colormap number to a color index
 * (for building images).
 */
 
/*
 * The color structure
 */

typedef struct color {
  char *name;                    /* The name of the color (or NULL) */
  unsigned short red,blue,green; /* Values for Red Blue and Green */
  unsigned long pixel;           /* The corresponding pixel value */
  unsigned short index;          /* Index for internal use */
} Color, *COLOR;



/*
 * The colormap  structure
 */

typedef struct colorMap {
  char *name;     /* Name of the colormap */
  Color *colors;  /* The colors of the colormap */
  int size;       /* The number of colors */
} ColorMap, *COLORMAP;


/*
 * The global array of all the colormaps 
 * The first one (index 0) corresponds to the colors that are named (maximum is 256)
 * The other ones correspond to regular colormaps 
 */
#define ColorMapNMax 64
static COLORMAP theColorMaps[ColorMapNMax];

/* The current number of defined colormaps in "theColorMaps"  (except index 0) */
static int nColorMaps;

/* The current colormap  (could be inversed) */
static int colorMapCur;


/* Some colors */
unsigned long bgColor, fgColor, mColor;
const unsigned long invisibleColor = 0x1000000;

static int theMouseMode = MouseInverse;


/****************************************************************
 *
 *    Miscellenaous functions
 *
 ****************************************************************/
 
/*
 * Check RGB values 
 */  

static char CheckRGB(float *pr,float *pg,float *pb)
{
  char flagOK = YES;

  if (*pr < 0 || *pr > 65535) {
    flagOK = NO;
    PrintfErr("Warning : Bad value for red value of color %.8g\n",*pr);
    *pr = MAX(*pr,0); *pr = MIN(*pr,65535);
  }
  if (*pg < 0 || *pg > 65535) {
    flagOK = NO;
    PrintfErr("Warning : Bad value for green value of color %.8g\n",*pr);
    *pg = MAX(*pg,0); *pg = MIN(*pg,65535);
  }
  if (*pb < 0 || *pb > 65535) {
    flagOK = NO;
    PrintfErr("Warning : Bad value for blue value of color %.8g\n",*pb);
    *pb = MAX(*pb,0); *pr = MIN(*pb,65535);
  }
  return(flagOK);
}


/*
 * Check HSV values 
 */

static char CheckHSV(float *ph,float *ps,float *pv)
{
  char flagOK = YES;

  if (*ph < 0 || *ph > 360) {
    *ph += 100*360;
    *ph = *ph - ((int) (*ph/360))*360; 
  }
  if (*ps < 0 || *ps > 1) {
    flagOK = NO;
    PrintfErr("Warning : Bad value for saturation value of color %.8g\n",*ps);
    *ps = MAX(*ps,0); *ps = MIN(*ps,1);
  }
  if (*pv < 0 || *pv > 1) {
    flagOK = NO;
    PrintfErr("Warning : Bad value for saturation value of color %.8g\n",*pv);
    *pv = MAX(*pv,0); *pv = MIN(*pv,1);
  }

  return(flagOK);
}


/*
 * Convert hue, saturation and value to red, blue green
 * Hue : 0 is red, 240 blue and 360 red again
 * Saturation and value : between 0 and 1
 */

static void HSVToRGB_(float *h, float *s, float *v)
{
  int ih;
  float f,p,q,t;
  float r,g,b;

  CheckHSV(h,s,v);
  f = p = q = t = 0;
  *h = (*h)/60.;
  ih = (int) (*h);
  f = (*h) - ih;
  p = (*v)*(1.-(*s));
  q =  (*v)*(1.-(*s)*f);
  t =  (*v)*(1.-((*s)*(1.-f)));
  switch (ih % 6) {
  case 0: r = *v; g = t; b = p; break;
  case 1: r = q; g = *v; b = p; break;
  case 2: r = p; g = *v; b = t; break;
  case 3: r = p; g = q; b = *v; break;
  case 4: r = t; g = p; b = *v; break;
  case 5: r = *v; g = p; b = q; break;
  }
  *h = (int) 65535*r;
  *s = (int) 65535*g;
  *v = (int) 65535*b;
}



/****************************************************************
 *
 *    Dealing with named colors
 *
 ****************************************************************/
 
/* Get the named color "name" (or return new index) */
static int GetNamedColor(char *name)
{
  int i;
  
  for (i=0;i<theColorMaps[0]->size;i++) {
    if (theColorMaps[0]->colors[i].name == NULL) break;
    if (!strcmp(name,theColorMaps[0]->colors[i].name)) break;
  }
  
  if (i == theColorMaps[0]->size) return(-1);

  return(i);
}


/* Defining a named color using rgb values */
void DefineNamedColorRGB(char *name,float red, float green, float blue)
{
  int c;
  COLOR color;
  
  CheckRGB(&red,&green,&blue);

  c = GetNamedColor(name);
  if (c == -1) Errorf("DefineNamedColorRGB() : Sorry, too many named colors (there are %d)",theColorMaps[0]->size);
  
  /* Get the color */
  color = &(theColorMaps[0]->colors[c]);
  
  /* If it exists we just modify the values */
  if (color->name != NULL) {

    /* We display a message if values are different */
    if (color->red != (unsigned short) red || color->green != (unsigned short) green || color->blue != (unsigned short) blue)
      PrintfErr("Color '%s' = [%d %d %d] becomes [%d %d %d]\n",name,(int) color->red,(int) color->green,(int) color->blue,
	                                                                (int) red,(int) green,(int) blue);
    color->red = red;	                                                                
    color->blue = blue;	                                                                
    color->green = green;
    color->pixel = 0;
    color->index = 0;
  }

  /* Otherwise we create the color */
  else {
    color->name = CopyStr(name);
    color->red = red;	                                                                
    color->blue = blue;	                                                                
    color->green = green;
    color->pixel = 0;
    color->index = 0;
  }
}

/* Defining a named color using hsv values */
void DefineNamedColorHSV(char *name,float hue, float saturation, float value)
{
  CheckHSV(&hue,&saturation,&value);
  HSVToRGB_(&hue,&saturation,&value);
  DefineNamedColorRGB(name,hue,saturation,value);
}


/****************************************************************
 *
 *    Dealing with regular colors
 *
 ****************************************************************/
 

/* Defining a color using rgb values */
void DefineColorRGB(int cm, int col,float red, float green, float blue)
{
  COLOR color;
  
  CheckRGB(&red,&green,&blue);

  if (theColorMaps[cm] == NULL) Errorf("DefineColorRGB() : Undefined colormap index %d",cm);
  
  if (col >= theColorMaps[cm]->size) Errorf("DefineColorRGB() : Color index too large %d",col);
  
  /* Get the color */
  color = &(theColorMaps[cm]->colors[col]);

  color->red = red;	                                                                
  color->blue = blue;	                                                                
  color->green = green;
  color->pixel = 0;
  color->index = 0;
}

/* Defining a color using hsv values */
void DefineColorHSV(int cm, int col,float hue, float saturation, float value)
{
  CheckHSV(&hue,&saturation,&value);
  HSVToRGB_(&hue,&saturation,&value);
  DefineColorRGB(cm,col,hue,saturation,value);
}



/****************************************************************
 *
 *    Dealing with color maps
 *
 ****************************************************************/

#define ColorMapMask 0xBF
#define ColorMapInverseMask 0x40
#define ColorMapShift 25

/* Getting a colormap named "name" */
static int GetColorMap(char *name)
{
  int i;
  int n = nColorMaps;
  
  
  if (name == NULL) {
    if (theColorMaps[0] != NULL) return(0);
    return(-1);
  }

  if (*name == '_') Errorf("GetColorMap() : Weird error");
  
  for (i=1;i<ColorMapNMax && n != 0;i++) {
    if (theColorMaps[i] == NULL) continue; 
    if (!strcmp(name,theColorMaps[i]->name)) return(i);
    n--;
  }
  
  return(-1);
}
    
  
/* Deleting a colormap */
static void DeleteColorMap(char *name)
{
  int cm,i;
  
  if (name == NULL) return;
  
  cm = GetColorMap(name);
  
  if (cm == -1) Errorf("DeleteColorMap() : color map '%s' does not exist",name);

  if (theColorMaps[cm]->colors != NULL) {
    Free(theColorMaps[cm]->colors);
    theColorMaps[cm]->colors = NULL;
  }  

  if (theColorMaps[cm]->name != NULL) Free(theColorMaps[cm]->name);
  Free(theColorMaps[cm]);
  
  theColorMaps[cm] = NULL;
  
  nColorMaps--;
  
  if (cm == (colorMapCur&ColorMapMask) && nColorMaps != 0) {
    for (i=1;i<ColorMapNMax;i++) {
      if (theColorMaps[i] != NULL) break;
    }
    
    if (i != ColorMapNMax) colorMapCur = i;
  }
  
}
        
/* Get the colormap name from its id */
char *GetColorMapName (unsigned long cm) 
{
  unsigned int i;
  char flagInverse; 
  char *str;
  
  i = cm>>ColorMapShift;
  flagInverse = i & ColorMapInverseMask;
  i = i & ColorMapMask;  

  if (i>=ColorMapNMax || i == 0) Errorf("GetColorMapName() : Bad colormap id '%d'",i);

   if (theColorMaps[i] == NULL || theColorMaps[i]->name == NULL) Errorf("GetColorMapName() : Bad colormap id '%d'",i);
  
  if (!flagInverse) return(theColorMaps[i]->name);
  str = CharAlloc(strlen(theColorMaps[i]->name)+2);
  strcpy(str,"_");
  strcpy(str+1,theColorMaps[i]->name);
  TempPtr(str);
  
  return(str);
}  
    
/* Modifying (if it exists) or creating a colormap named "name" so that it will have "nb" colors */
static int CGetColorMap(char *name, int nb)
{
  int cm,i;
  
  if (nb <= 0) Errorf("CGetColorMap() : 'nb' should be positive");
  if (name != NULL && *name == '_') Errorf("CGetColorMap() : Sorry, you cannot create a color map whose name starts with a '_'");
  
  cm = GetColorMap(name);

  /* If Colormap already exists we delete it */
  if (cm != -1) {
    
    for (i=0;i<theColorMaps[cm]->size;i++) {
      if (theColorMaps[cm]->colors[i].name != NULL) {
        Free(theColorMaps[cm]->colors[i].name);
        theColorMaps[cm]->colors[i].name = NULL;
      }
    }
    if (theColorMaps[cm]->colors != NULL) {
      Free(theColorMaps[cm]->colors);
      theColorMaps[cm]->colors = NULL;
    }
  }
  
  /* Otherwise we must create the structure */
  else {
    for (i=0;i<ColorMapNMax;i++) {
      if (theColorMaps[i] == NULL) break; 
    }
    if (i == ColorMapNMax) Errorf("CGetColorMap() : Sorry, too many colormaps (maximum is %d)",ColorMapNMax);
    cm = i;
    theColorMaps[cm] = (COLORMAP) Malloc(sizeof(struct colorMap));
    theColorMaps[cm]->colors = NULL;
    theColorMaps[cm]->size = 0;
    if (name != NULL) {
      theColorMaps[cm]->name = CopyStr(name);
      nColorMaps++;
    }
  }
  
  /* Then we allocate the array of colors */
  if (nb !=  0) {
    theColorMaps[cm]->colors = (Color *) Malloc(sizeof(struct color)*nb);       
    theColorMaps[cm]->size = nb;
    for (i=0;i<nb;i++) {
      theColorMaps[cm]->colors[i].name = NULL;
      theColorMaps[cm]->colors[i].pixel = 0;
    }
  }

  return(cm);
}    

/* Get the number of colors of a given colormap */
int ColorMapSize(unsigned long colorMap)
{
  int cm = (colorMap>>ColorMapShift)&ColorMapMask;
  
  if (theColorMaps[cm]==NULL) Errorf("ColorMapSize() : Bad colormap");
  
  return(theColorMaps[cm]->size);
}

/***********************************************************************************
 *
 *  Dealing with pixels
 *
 ***********************************************************************************/
unsigned long Color2Pixel(unsigned long color)
{
  unsigned long c;
  int cm;
  char flagInverse;
  
  if (color & invisibleColor) Errorf("Color2Pixel() : Unexpected 'invisible' color");
  
  c = color & 0xFFFFFF;
  cm = color >> ColorMapShift;
  flagInverse = cm&ColorMapInverseMask;
  cm = cm&ColorMapMask;
  
  if (theColorMaps[cm] == NULL) Errorf("Bad Color number");
  if (theColorMaps[cm]->size <= c) Errorf("Bad Color number");
  
  if (flagInverse) c = theColorMaps[cm]->size-c-1;
  
  return(theColorMaps[cm]->colors[c].pixel);
}  

void Color2RGB(unsigned long color, unsigned short *r,unsigned short *g,unsigned short *b)
{
  unsigned long c;
  int cm;
  char flagInverse;

  if (color & invisibleColor) Errorf("Color2RGB() : Unexpected 'invisible' color");
  
  c = color & 0xFFFFFF;
  cm = color >> ColorMapShift;
  flagInverse = cm&ColorMapInverseMask;
  cm = cm&ColorMapMask;
  
  if (theColorMaps[cm] == NULL) Errorf("Bad Color map number");
  if (theColorMaps[cm]->size <= c) Errorf("Bad Color %d number");

  if (flagInverse) c = theColorMaps[cm]->size-c-1;
  
  *r = theColorMaps[cm]->colors[c].red;
  *g = theColorMaps[cm]->colors[c].green;
  *b = theColorMaps[cm]->colors[c].blue;
}  

char *GetColorName(unsigned long color)
{
  unsigned long c;
  int cm;
  static char str[50];
  char flagInverse;
  
  if (color & invisibleColor) return("invisible");
  
  c = color & 0xFFFFFF;
  cm = color >> ColorMapShift;
  flagInverse = cm&ColorMapInverseMask;
  cm = cm&ColorMapMask;
  
  if (theColorMaps[cm] == NULL) Errorf("Bad Color number");
  if (theColorMaps[cm]->size <= c) Errorf("Bad Color number");
  
  if (cm == 0) return(theColorMaps[cm]->colors[c].name);

  if (flagInverse) sprintf(str,"%d_%s",c,theColorMaps[cm]->colors[c].name);
  else sprintf(str,"%d%s",c,theColorMaps[cm]->colors[c].name);
  
  return(str);
}  


/***********************************************************************************
 *
 *  Parsing a colormap name
 *
 ***********************************************************************************/
 
unsigned long GetColorMapCur(void)
{
  return(colorMapCur<<ColorMapShift);
 }
 

/* Get a colormap from string 'arg' which has already been evaluated */   
static char ParseColorMapInt_(char *arg, int defVal, int *colormap)
{
  int cm;
  char flagInverse;
  
  *colormap = defVal;
  if (arg == NULL) {
    SetErrorf("ParseColorMap__() : NULL string cannot be converted to a color");
    return(NO);
  }

  if (*arg == '_') {
    if (*(arg+1) == '\0') {
      if (colorMapCur&ColorMapInverseMask) *colormap = (colorMapCur-ColorMapInverseMask) ;
      else *colormap = (colorMapCur+ColorMapInverseMask);
      return(YES);
    }
    flagInverse = YES;
    arg++;
  } else flagInverse = NO;
  
  cm = GetColorMap(arg);
  if (cm == -1) {
    SetErrorf("ParseColorMap_() : Bad colormap name '%s'",arg);
    return(NO);
  }

  *colormap = cm;
  if (flagInverse) *colormap += ColorMapInverseMask;
  
  return(YES);
}

static void ParseColorMapInt(char *arg, int *colormap)
{
  if (ParseColorMapInt_(arg,0,colormap) == NO) Errorf1("");
}


/* Parse a color map, with evaluation  */
char ParseColorMap_(char *arg, unsigned long defVal, unsigned long *colormap)
{
  int cm;
  char *str;
  
  *colormap = defVal;
  
  if (ParseStr_(arg,NULL,&str) == NO) return(NO);
  
  if (ParseColorMapInt_(str,-1,&cm) == NO) return(NO);
  
  *colormap = cm  << ColorMapShift;
  
  return(YES);
}

void ParseColorMap(char *arg, unsigned long *colormap)
{
  if (ParseColorMap_(arg,0,colormap) == NO) Errorf1("");
}


/***********************************************************************************
 *
 *  Parsing a color name
 *
 ***********************************************************************************/
 
  
char ParseColor_(char *arg, unsigned long defVal, unsigned long *color)
{
  char *endp;
  unsigned long l;
  int cm,i;
  VALUE val;
  float f;
  char *str;
  
  val = NULL;

  if (!ParseFloatStrLevel_(levelCur,arg,&f,&str)) {
    *color = defVal;
    return(NO);
  }
  
  *color = defVal;
  
  if (arg == NULL) {
    SetErrorf("ParseColor_() : NULL string cannot be converted to a color");
    return(NO);
  }

  /* Case of a string */
  if (str != NULL) {
  
    /* Particular case : FG and BG, invisible */
    if (!strcmp("FG",str)) {
      *color = fgColor;
      return(YES);
    }
    if (!strcmp("BG",str)) {
      *color = bgColor;
      return(YES);
    }
    if (!strcmp("invisible",str)) {
      *color = invisibleColor;
      return(YES);
    }
    
    /* Otherwise let's try to read a number and colormap afterwards */
    endp = NULL;
    l = strtoul(str,&endp,10);
    
    /* If we could not then it must be a symbolic name */
    if (endp == str) {
      i = GetNamedColor(str);
      if (i == -1 || theColorMaps[0]->colors[i].name == NULL) {
        SetErrorf("ParseColor_() : Unknown color %s",arg);
        return(NO);
      }
      *color = i;
  
      return(YES);
    }
        
    /* Otherwise, we got the number let's get a colormap name */
    if (!ParseColorMapInt_(endp,0,&cm)) {
      SetErrorf("ParseColor_() : Bad colormap name '%s'",endp);
      return(NO);
    }
  }

  /* Case of a number (default colomap is used) */
  else {
    if (f != (int) f || f < 0) {
      SetErrorf("ParseColor_() : '%g' is not a color",f);
      return(NO);
    }
    l = (int) f;
    
    cm = colorMapCur;
    if (theColorMaps[cm] == NULL) {
      SetErrorf("ParseColor_() : Bad current colormap");
      return(NO);
    }
  }
  
  /* Then just get the color 'l' associated to the colormap 'cm' */
  if (l >= theColorMaps[cm]->size) {
    SetErrorf("ParseColor_() : Bad index color '%d'",l);
    return(NO);
  }
    
  *color = (((unsigned long) cm) << 25) + l;
  return(YES);
}
     
void ParseColor(char *arg, unsigned long *color)
{
  if (ParseColor_(arg,0,color) == NO) Errorf1("");
}



/*******************************************************************
 *
 *  Build the colormap
 *
 *******************************************************************/
 
#define MaxNColors 4000

int BuildColormap(char flagShared, char mouseMode)
{
  static unsigned short red[MaxNColors], green[MaxNColors], blue[MaxNColors];
  static unsigned long pixels[MaxNColors];
  
  int nColors,ncm,i,j,k,last;
  int nAllocColors;

  nColors = 0;
  ncm = nColorMaps+1;


  /*
   * In the case the Depth is greater than 8 we do not check
   * wether two colors are the same. Otherwise we have
   * to check it.
   */
  if (WDepth() > 8) {
    for (i=0;i<ColorMapNMax;i++) {
      if (ncm == 0) break;
      if (theColorMaps[i] != NULL) {
        for (j=0;j<theColorMaps[i]->size;j++) {
          if (i == 0 && theColorMaps[0]->colors[j].name == NULL) break;
          if (nColors >= MaxNColors) Errorf("BuildColormap() : Sorry too many colors allocated (max is %d)",MaxNColors);
          red[nColors] = theColorMaps[i]->colors[j].red;
          green[nColors] = theColorMaps[i]->colors[j].green;
          blue[nColors] = theColorMaps[i]->colors[j].blue;
          theColorMaps[i]->colors[j].index = nColors;
          nColors++;
        }
        ncm--;
      }
    }
  }
  else {
    last = 0;
    for (i=0;i<ColorMapNMax;i++) {
      if (ncm == 0) break;
      if (theColorMaps[i] != NULL) {
        for (j=0;j<theColorMaps[i]->size;j++) {
          if (i == 0 && theColorMaps[0]->colors[j].name == NULL) break;
          theColorMaps[i]->colors[j].index = MaxNColors;
          for (k=0;k<last;k++) {
            if (red[k] == theColorMaps[i]->colors[j].red && green[k] == theColorMaps[i]->colors[j].green && blue[k] == theColorMaps[i]->colors[j].blue) {
              theColorMaps[i]->colors[j].index = k;
              break;
            }
          }
          if (theColorMaps[i]->colors[j].index == MaxNColors) {
            if (nColors >= MaxNColors) Errorf("BuildColormap() : Sorry too many colors allocated (max is %d)",MaxNColors);
            red[nColors] = theColorMaps[i]->colors[j].red;
            green[nColors] = theColorMaps[i]->colors[j].green;
            blue[nColors] = theColorMaps[i]->colors[j].blue;
            theColorMaps[i]->colors[j].index = nColors;
            nColors++;
          }
          if (i == 0) last = nColors;
        }
        ncm--;
        last = nColors;
      }
    }
  }
  
        
  /* Build the colormap */
  nAllocColors = WSetColormap(red,green,blue,pixels,nColors,flagShared,mouseMode);

  ncm = nColorMaps+1;
  for (i=0;i<128;i++) {
    if (ncm == 0) break;
    if (theColorMaps[i] != NULL) {
      for (j=0;j<theColorMaps[i]->size;j++) {
         if (i == 0 && theColorMaps[0]->colors[j].name == NULL) break;
         theColorMaps[i]->colors[j].pixel = pixels[theColorMaps[i]->colors[j].index];
       }
    
      ncm--;
    }
  }

  /* Refresh the windows ???? */
/*  if (theWindowsHT && flagRefresh) {
    for (r = 0; r<theWindowsHT->nRows;r++) {
      for (e = theWindowsHT->rows[r]; e != NULL; e = e->next) { 
        DrawWholeGObject((GOBJECT) e, YES);
      }
    }
  } */
        
  return(nAllocColors);    
      
}


/*
 * Init 
 */
void ColorsInit(void)
{
  int i;
  unsigned short r1,g1,b1,r2,g2,b2;

  nColorMaps = 0;
  for (i=0;i<128;i++) theColorMaps[i] = NULL;
  
  WBgColor(&r1,&g1,&b1);
  WFgColor(&r2,&g2,&b2);

  CGetColorMap(NULL,256);
  DefineNamedColorRGB("bgDefault",r1,g1,b1);
  DefineNamedColorRGB("fgDefault",r2,g2,b2);

  bgColor = 0;
  fgColor = 1;
  mColor = 1;
  theMouseMode = Mouse1Color;
  
  colorMapCur = CGetColorMap("bw",2); 
  DefineColorRGB(colorMapCur,0,r1,g1,b1);
  DefineColorRGB(colorMapCur,1,r2,g2,b2);
   
  BuildColormap(YES,theMouseMode);
}


/*
 * Setting background/foreground/mouse colors
 */

void C_SetColor(char **argv)
{
  char *action;
  unsigned long color;

  while (1) {

    if (*argv == NULL) break;   
    argv = ParseArgv(argv,tWORD,&action,-1);

    if (!strcmp(action,"-bg")) {
      argv = ParseArgv(argv,tCOLOR,&color,-1);
	  bgColor = color;
	}

    else if (!strcmp(action,"-fg")) {
      argv = ParseArgv(argv,tCOLOR,&color,-1);
	  fgColor = color;
	}

    else if (!strcmp(action,"-mouse")) {
      if (ParseColor_(*argv,0,&color)) {
	    mColor = color;
	    theMouseMode = Mouse1Color;
	  }
	  else theMouseMode = MouseInverse;
	  BuildColormap(YES,theMouseMode);
	}	
  }    

  NoMoreArgs(argv);    
}


/* 
 * The main command for managing new colors
 */

void C_Color(char **argv)
{
  char *action,*mode,*name;
  float x1,x2,x3;
  char flagRGB,flagMap;
  int index,cm,i;
  char flagInverse;
  unsigned long color;
  LISTV lv,lv1;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  flagMap = NO;
  
  /* 'nnew' action */
  if (!strcmp(action,"nnew")) {
    argv = ParseArgv(argv,tSTR,&name,tWORD,&mode,tFLOAT,&x1,tFLOAT,&x2,tFLOAT,&x3,0);
    if (!IsValidSymbol(name)) Errorf("Invalid color name '%s'",name);
    if (!strcmp(mode,"rgb")) flagRGB = YES;
    else if (!strcmp(mode,"hsv")) flagRGB = NO;
    else ErrorUsage();
    if (flagRGB) DefineNamedColorRGB(name,x1,x2,x3);
    else DefineNamedColorHSV(name,x1,x2,x3);
  }

  /* 'inew' action */
  else if (!strcmp(action,"inew")) {
    ParseArgv(argv,tSTR_,NULL,&name,-1);
    if (name != NULL) {
      if (*name == '_') Errorf("You cannot create a new color in an inversed color map.");
      cm = GetColorMap(name);
      if (cm == -1) name = NULL;
      else argv++;
    } 
    if (name = NULL) {
      if (theColorMaps[colorMapCur&ColorMapMask] == NULL) Errorf("Sorry current color map is not defined");
      name = theColorMaps[colorMapCur&ColorMapMask]->name;
      cm = GetColorMap(name);
    }        
    if (cm == -1) Errorf("Unknown Color Map '%s'",name);
    argv = ParseArgv(argv,tINT,&index,tWORD,&mode,tFLOAT,&x1,tFLOAT,&x2,tFLOAT,&x3,0);
    if (!strcmp(mode,"rgb")) flagRGB = YES;
    else if (!strcmp(mode,"hsv")) flagRGB = NO;
    else ErrorUsage();
    if (flagRGB) DefineColorRGB(cm,index,x1,x2,x3);
    else DefineColorHSV(cm,index,x1,x2,x3);
  }

  /* 'ilist' action */
  else if (!strcmp(action,"ilist")) {
    flagInverse = NO;
    if (ParseInt_(*argv,0,&index) || *argv == NULL) {
      cm = colorMapCur;
    }
    else {
      ParseColorMapInt(*argv,&cm);
      argv++;
    }
    argv = ParseArgv(argv,tINT_,-1,&index,0);
    lv = TNewListv();
    SetResultValue(lv);
    if (index == -1) {
      if (!(cm&ColorMapInverseMask)) {
        cm = cm &ColorMapMask;
        for (i=0;i<theColorMaps[cm]->size;i++) {
          lv1 = TNewListv();
          AppendInt2Listv(lv1,theColorMaps[cm]->colors[i].red);
          AppendInt2Listv(lv1,theColorMaps[cm]->colors[i].green);
          AppendInt2Listv(lv1,theColorMaps[cm]->colors[i].blue);
          AppendValue2Listv(lv, (VALUE) lv1);
        }
      } else {
        cm = cm &ColorMapMask;
        for (i=theColorMaps[cm]->size-1; i>= 0;i--) {
          lv1 = TNewListv();
          AppendInt2Listv(lv1,theColorMaps[cm]->colors[i].red);
          AppendInt2Listv(lv1,theColorMaps[cm]->colors[i].green);
          AppendInt2Listv(lv1,theColorMaps[cm]->colors[i].blue);
          AppendValue2Listv(lv, (VALUE) lv1);
        }
      }
    }
    else if (index < 0 || index >= theColorMaps[cm &ColorMapMask]->size) Errorf("Bad color index '%d'",index);
    else if (!(cm&ColorMapInverseMask)) {
       cm = cm &ColorMapMask;
       AppendInt2Listv(lv,theColorMaps[cm]->colors[index].red);
       AppendInt2Listv(lv,theColorMaps[cm]->colors[index].green);
       AppendInt2Listv(lv,theColorMaps[cm]->colors[index].blue);
    }
    else {
       cm = cm &ColorMapMask;
       index = theColorMaps[cm]->size-1-index;
       AppendInt2Listv(lv,theColorMaps[cm]->colors[index].red);
       AppendInt2Listv(lv,theColorMaps[cm]->colors[index].green);
       AppendInt2Listv(lv,theColorMaps[cm]->colors[index].blue);
    }
  }
  
  /* 'nlist' action */
  else if (!strcmp(action,"nlist")) {
    argv = ParseArgv(argv,tSTR_,"*",&name,0);  
    lv = TNewListv();
    SetResultValue(lv);
    for (i=0;i<theColorMaps[0]->size;i++) {
      if (theColorMaps[0]->colors[i].name == NULL) break;
      if (MatchStr(theColorMaps[0]->colors[i].name,name)) {
        lv1 = TNewListv();
        AppendStr2Listv(lv1,theColorMaps[0]->colors[i].name);
        AppendInt2Listv(lv1,theColorMaps[0]->colors[i].red);
        AppendInt2Listv(lv1,theColorMaps[0]->colors[i].green);
        AppendInt2Listv(lv1,theColorMaps[0]->colors[i].blue);
        AppendValue2Listv(lv, (VALUE) lv1);
      }
    }
  }    
  
  /* 'animate' action */
  else if (!strcmp(action,"animate")) {
    argv = ParseArgv(argv,tCOLOR,&color,tWORD,&mode,tFLOAT,&x1,tFLOAT,&x2,tFLOAT,&x3,0);
    if (!strcmp(mode,"rgb")) flagRGB = YES;
    else if (!strcmp(mode,"hsv")) flagRGB = NO;
    else ErrorUsage();
    if (!flagRGB) HSVToRGB_(&x1,&x2,&x3);
    WAnimateColor(color,(short unsigned) x1,(short unsigned) x2,(short unsigned) x3);
  }    

  /* 'nb' action */
  else if (!strcmp(action,"nb")) {
    NoMoreArgs(argv);
    SetResultInt(WNumOfColors());
  }    
  
  /* 'install' action */
  else if (!strcmp(action,"install")) {
    NoMoreArgs(argv);
    flagMap = YES;
  }

  else Errorf("Unknown action '%s'",action);
  
  if (flagMap) SetResultInt(BuildColormap(YES,theMouseMode));
}


/* 
 * The main command for managing new colors
 */

void C_ColorMap(char **argv)
{
  char *action,*name;
  int size,cm,n,i;
  LISTV lv;
  
  argv = ParseArgv(argv,tWORD,&action,-1);
  
  /* 'new' action */  
  if (!strcmp(action,"new")) {
    argv = ParseArgv(argv,tSTR_,NULL,&name,tINT,&size,0);
    if (name == NULL) {
      if (theColorMaps[colorMapCur&ColorMapMask] == NULL) Errorf("Sorry current color map is not defined");
      name = theColorMaps[colorMapCur&ColorMapMask]->name;
    }
    else {
      if (!IsValidSymbol(name)) Errorf("Invalid colormap name '%s'",name);
    }
    CGetColorMap(name,size);
  }
  
  /* 'current' action */
  else if (!strcmp(action,"current")) {
    argv = ParseArgv(argv,tSTR_,NULL,&name,0);
    if (name == NULL) {
      if (colorMapCur&ColorMapInverseMask && theColorMaps[colorMapCur&ColorMapMask] != NULL) SetResultf("_%s",theColorMaps[colorMapCur&ColorMapMask]->name);
      else if (theColorMaps[colorMapCur] != NULL) SetResultStr(theColorMaps[colorMapCur]->name);
      else SetResultStr("None");
    }
    else {
      ParseColorMapInt(name,&cm);      
      colorMapCur = cm;
    }
  }

  /* 'delete' action */
  else if (!strcmp(action,"delete")) {
    argv = ParseArgv(argv,tSTR_,NULL,&name,0);
    if (name == NULL) {
      if (theColorMaps[colorMapCur&ColorMapMask] != NULL) name = theColorMaps[colorMapCur&ColorMapMask]->name;
      else Errorf("Sorry current color map is not defined");
    }
    DeleteColorMap(name);
  }

  /* 'size' action */
  else if (!strcmp(action,"size")) {
    argv = ParseArgv(argv,tSTR_,NULL,&name,0);
    if (name == NULL) {
      if (theColorMaps[colorMapCur&ColorMapMask] != NULL) name = theColorMaps[colorMapCur&ColorMapMask]->name;
      else Errorf("Sorry current color map is not defined");
    }
    if (*name == '_') name++;
    cm = GetColorMap(name);
    if (cm == -1) Errorf("Unknown color map '%s'",name);
    SetResultInt(theColorMaps[cm]->size);
  }
 
  /* list action */
  else if (!strcmp(action,"list")) {
    argv = ParseArgv(argv,tSTR_,"*",&name,0);
    n = nColorMaps;
    lv = TNewListv();
    SetResultValue(lv);
    for (i=1;i<ColorMapNMax;i++) {
      if (n == 0) break;
      if (theColorMaps[i] != NULL && MatchStr(theColorMaps[i]->name,name)) {
        AppendStr2Listv(lv,theColorMaps[i]->name); 
        n--;
      }
    }
  }
       
 
  else Errorf("Unknown action '%s'",action);
}



