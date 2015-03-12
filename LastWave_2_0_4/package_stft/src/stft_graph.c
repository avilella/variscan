/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'stft' 2.0                         */
/*                                                                          */
/*      Copyright (C) 1997-2002 R.Gribonval, E.Bacry                        */
/*      email  : remi.gribonval@inria.fr                                    */
/*               lastwave@cmapx.polytechnique.fr                            */
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
/*      stft_graph.c     Functions for displaying stft                      */
/*                                                                          */
/****************************************************************************/


#include "lastwave.h"
#include "stft.h"



/* The GOBJECT structure for displaying a STFT */
typedef struct graphStft {

    GObjectFields; 
  
    /* The stft to be displayed */
    STFT stft;
    
    char   flagCausal;
    char   flagDb;
    float exponent;	/* the exponent to display it in log value or not */

    /* The colormap used */
    unsigned long cm;

} GraphStft, *GRAPHSTFT;

/* The corresponding class */      
GCLASS theGraphStftClass = NULL;


/* Initialization of the GraphStft structure */
static void _InitGraphStft(GOBJECT o)
{
    GRAPHSTFT graph;

    graph 	= (GRAPHSTFT) o;

    graph->stft = NULL;
  
    graph->cm 	= GetColorMapCur();
  
    graph->bgColor 	= invisibleColor;

    graph->rectType 	= SmallRect;
      
    graph->flagCausal 	= NO;
    graph->flagDb 	= YES;
    graph->exponent 	= 70.0;
}


/* Deleting the content of a GraphStft */
static void _DeleteContentGraphStft(GOBJECT o)
{
    GRAPHSTFT graph;

    graph = (GRAPHSTFT) o;

    if (graph->stft != NULL) graph->stft = DeleteStft(graph->stft);
}


/* The setg method */
static int _SetGraphStft (GOBJECT o, char *field, char**argv)
{
    GRAPHSTFT graph;
    STFT stft;
    float f1;
    int flag;
    LISTV lv;

    /* The help command */
    if (o == NULL) 
    { /* REMI */
        SetResultStr("{{{graph [<stft>]} {Gets/Sets the stft to be displayed by the GraphStft. (The '-cgraph' field is equivalent to that field).}} \
{{cm [<colormap>]} {Sets/Gets the colormap that will be used to display the stft.}} \
{{db [<flagOnOff>]} {Sets/Gets the decibel-display flag. If it is on, the energy is displayed in decibel.}} \
{{causal [<flagOnOff>]} {Sets/Gets the causal-display flag. If it is on, only the region not affected by border effects is displayed.}} \
{{expo [<exponent>]} {Sets/Gets the exponent used for display. If '-db' is off, then the <exponent> is not used. If '-db' is on, then the same quantity is displayed in decibel.}}}");
    return(YES);
  }
   
      
    graph = (GRAPHSTFT) o;
    stft  = graph->stft;
  
  
    /* the 'graph' and 'cgraph' fields */
    if (!strcmp(field,"graph") || !strcmp(field,"cgraph")) 
    {
	if (*argv == NULL) 
	{
	    SetResultValue(graph->stft);
	    return(YES);
	}
	argv = ParseArgv(argv,tSTFT,&stft,0);
	CheckStft(stft);
	if(stft->flagUpToDate == NO)
	    Errorf("_SetGraphStft : stft is out of date");
	if (graph->stft != NULL) graph->stft = DeleteStft(graph->stft);
	graph->stft = stft;
	AddRefValue(stft);

	o->rx = TimeId2Time(stft,-((float)stft->tRate)/2.);// tRate is 'int'and needs to be converted
	o->rw = TimeId2Time(stft,stft->signalSize+stft->tRate/2.);
	o->rw-=	o->rx;
	o->ry = FreqId2Freq(stft,-((float)stft->fRate)/2.);// fRate is 'int'and needs to be converted
	o->rh = FreqId2Freq(stft,GABOR_NYQUIST_FREQID+stft->fRate/2.);
	o->rh-=	o->ry;
	UpdateGlobalRectGObject(o);    
	return(YES);
    }

    /* The colormap field */
    if (!strcmp(field,"cm")) 
    {
	if (*argv == NULL) 
	{
	    SetResultStr(GetColorMapName(graph->cm));
	    return(YES);  
	}    
	argv = ParseArgv(argv,tCOLORMAP,&(graph->cm),0);
	return(YES);
    }

    /* The db field */
    if (!strcmp(field,"db")) 
    {
	if (*argv == NULL) 
	{
	    SetResultInt((int)graph->flagDb);
	    return(YES);  
	}    
	argv = ParseArgv(argv,tINT,&flag,0);
	graph->flagDb = (flag!=0);
	return(YES);
    }

    /* The expo field */
    if (!strcmp(field,"expo")) 
    {
	if (*argv == NULL) 
	{
	    SetResultFloat(graph->exponent);
	    return(YES);  
	}    
	argv = ParseArgv(argv,tFLOAT,&f1,0);
	graph->exponent = f1;
	return(YES);
    }

    /* The causal field */
    if (!strcmp(field,"causal")) 
    {
	if (*argv == NULL) 
	{
	    SetResultInt((int) graph->flagCausal);
	    return(YES);  
	}    
	argv = ParseArgv(argv,tINT,&flag,0);
	graph->flagCausal = (flag!=0);
	return(YES);
    }

    /* The 'rect' field */
    if (!strcmp(field,"rect")) 
    {
	NoMoreArgs(argv);
	if (stft == NULL) 
	{
           lv = TNewListv();
           SetResultValue(lv);
           AppendFloat2Listv(lv,0);
           AppendFloat2Listv(lv,0);
           AppendFloat2Listv(lv,0);
           AppendFloat2Listv(lv,0); 
           return(YES);
	}

	o->rx = TimeId2Time(stft,-((float)stft->tRate)/2.);// tRate is 'int'and needs to be converted
	o->rw = TimeId2Time(stft,stft->signalSize+stft->tRate/2.);
	o->rw-= o->rx;
	o->ry = FreqId2Freq(stft,-((float)stft->fRate)/2.);// fRate is 'int'and needs to be converted
	o->rh = FreqId2Freq(stft,GABOR_NYQUIST_FREQID+stft->fRate/2.);
	o->rh-=	o->ry;
	UpdateGlobalRectGObject(o);    

        lv = TNewListv();
        SetResultValue(lv);
    
        AppendFloat2Listv(lv,o->rx);
        AppendFloat2Listv(lv,o->ry);
        AppendFloat2Listv(lv,o->rw);
        AppendFloat2Listv(lv,o->rh);
	return(YES);
    }

    return(NO);
}


/* The drawing procedure */
static void _DrawGraphStft (WINDOW win, GOBJECT obj, int x, int y,int w,int h)
{
    GRAPHSTFT graph;
    STFT stft;
    int nColors;

    int nRow,nCol;
    int color;

    int   maxTimeId,maxFreqId;
    float maxMap;
    float mapValue;

    int i,j;

    float time,freq;

    char flagCausal;
    int timeIdMin,timeIdMax,freqIdMin,freqIdMax;
    int timeId,freqId;
    int dt,df;

    /* Some inits */
    graph = (GRAPHSTFT) obj;
    stft = graph->stft;
    flagCausal = graph->flagCausal;
    if (stft == NULL) 
	return;
    CheckStft(stft);
    if(stft->flagUpToDate == NO)
	Errorf("_DrawGraphStft : stft is out of date");

    /* Get the number of colors of the colormap */
    nColors = ColorMapSize(graph->cm);

    /* Allocation */
    nCol = w;
    nRow = h;

    WInitPixMap(nRow,nCol);

    timeIdMin = 0;
    timeIdMax = stft->signalSize-1;
    freqIdMin = 0;
    freqIdMax = GABOR_NYQUIST_FREQID;

    /* Image size vs data size */

    /* Computes the maximum of the map to renormalize */
    if(GetStftMax(stft,flagCausal,timeIdMin,timeIdMax,freqIdMin,freqIdMax,&maxTimeId,&maxFreqId,&maxMap)==NO) {
      Warningf("_DrawGRaphStft : empty range");
      maxMap = 1.0;
    }
    if(maxMap == 0.0) maxMap = 1.0;

    Global2Local(obj,0,0,&time,&freq);
    freqId = (int) (Freq2FreqId(stft,freq)+.5);	  
    timeId = (int) (Time2TimeId(stft,time)+.5);	  
    Global2Local(obj,1,1,&time,&freq);
    df = fabs((int) (Freq2FreqId(stft,freq)+.5)-freqId);	  
    dt = fabs((int) (Time2TimeId(stft,time)+.5)-timeId);	  
    df = (int) (df/2.0+.5);
    dt = (int) (dt/2.0+.5);

    /* Set the pixmap */
    /* Loop on the rows */
    for(i = 0; i < nRow; i++)
    {
	/* Loop on the columns */
	for(j = 0; j < nCol; j++)
	{
	    /* Computes the extremal coordinates 
	     * for 'timeId' (j = winX)  
	     * and 'freqId' (i = winY) 
	     * for these pixels
	     */
	    Global2Local(obj,x+j,y+i,&time,&freq);

	    timeId = (int) (Time2TimeId(stft,time)+.5);	  
	    timeId = stft->tRate*((timeId+stft->tRate/2)/stft->tRate);
	    timeId = MAX(timeId,0);
            timeId = MIN(timeId,stft->signalSize-1);
	    timeIdMin = timeId-dt;
	    timeIdMax = timeId+dt;
	    timeIdMin = MAX(timeIdMin,0);
	    timeIdMax = MIN(timeIdMax,stft->signalSize-1);

	    freqId = (int) (Freq2FreqId(stft,freq)+.5);	  
	    freqId = stft->fRate*((freqId+stft->fRate/2)/stft->fRate);
	    freqId = MAX(freqId,0);
	    freqId = MIN(freqId,GABOR_NYQUIST_FREQID);
	    freqIdMin = freqId-df;
	    freqIdMax = freqId+df;
	    freqIdMin = MAX(freqIdMin,0);
	    freqIdMax = MIN(freqIdMax,GABOR_NYQUIST_FREQID);
	    
	    /* REMI */
            if(GetStftMax(stft,flagCausal,timeIdMin,timeIdMax,freqIdMin,freqIdMax,&maxTimeId,&maxFreqId,&mapValue)==NO)
              mapValue = 0.0;
	    /* Renormalizing the pixel value */
	    mapValue /= maxMap;
	  
	    /* Replace the map by its log/power if necessary */
	    if(graph->flagDb)
	    {
		if(mapValue != 0.0) 
		{
		    /* Taking decibels */
		    mapValue = 10*log10(mapValue);
		    /* Normalizing by exponent */
		    mapValue /= graph->exponent;
		    mapValue += 1;
		    mapValue = MAX(mapValue,0);
		}
	    }
	  
	    /* Setting the pixel color */
	    color = (int) (nColors * mapValue);
	    color = (color>=nColors ? nColors-1 : color);
	    color = (color<0 ? 0 : color);
	    color += graph->cm;

	    WSetPixelPixMap(i,j,color);
	}
    }
  
    WDisplayPixMap(win,x,y);  
}


/* Defining the GraphStft gclass */  
void DefineGraphStft(void)
{
    theGraphStftClass = NewGClass("GraphStft",theGObjectClass,"stft"); 
    theGraphStftClass->nbBytes = sizeof(GraphStft);
    theGraphStftClass->init = _InitGraphStft;
    theGraphStftClass->deleteContent = _DeleteContentGraphStft;
    theGraphStftClass->set = _SetGraphStft;
    theGraphStftClass->draw = _DrawGraphStft;   
    theGraphStftClass->varType = stftType;
    theGraphStftClass->flags &= ~(GClassMoveResize+GClassLocalCoor);
    theGraphStftClass->info = "Graphic Class that allows to display Stft";
}


/* EOF */

