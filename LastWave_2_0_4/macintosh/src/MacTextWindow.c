

/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   M a c i n t o s h   2 . 0                         */
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

#include "computer.h"

#ifdef LASTWAVE_MAC_GRAPHICS

#include "lastwave.h"

#include <Controls.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>

#include "MacTextWindow.h"

#include <Icons.h>
#include <Processes.h>
#include <Fonts.h>
#include <Sound.h>
#include <ToolUtils.h>

#include "Mac.h"


/*
 * If there is no routine for converting C string to Pascal string we must add our own
 */
 
#ifndef CtoPstr

static Str255 thePascalString;

static void _C2Pstr(char *cstring, Str255 pstring)
{
  unsigned char *p = (unsigned char *) pstring;
  char *c = cstring;
  int l;
  
  strncpy((char *) (p+1),c,254);
  l = strlen(c);
  if (l>255) l = 255;
  
  p[0] = l;  
}

#define CtoPstr(c) (_C2Pstr(c,thePascalString),thePascalString)

#endif


/* 
 *  The text windows variables
 */

#define NTW 20                         
static int nTextWindows;
static TEXTWINDOW theTextWindows[NTW];


/*
 *  Initialization of the text windows
 */
 
void XXInitTextWindows(void)
{
  int i = 0;
  
  for(i=0;i<NTW;i++) theTextWindows[i] = 0;
  
  nTextWindows = 0; 
}


/*
 *   Move the thumb of the scroll bar
 */
 
static void TWMoveScrollBox(ControlHandle theControl, short scrollDistance)
{
	short oldSetting;
	short maxValue;
	short setting;
	
	oldSetting = GetControlValue(theControl);
	maxValue = GetControlMaximum(theControl);
	setting = oldSetting - scrollDistance;
	if (setting < 0) {
		setting = 0;
	} else {
		if (setting > maxValue) {
			setting = maxValue;
		}
	}
	SetControlValue(theControl, setting);
}


/*
 * Adjust the text (when the scrollbar moved)
 */
 
static void TWAdjustText(TEXTWINDOW window)
{
	/*	Since we can only do an adjust text on the textWindow ... */
	TEHandle theTE = window->textEdit;
	short change;
	
	change = (*theTE)->viewRect.top - (*theTE)->destRect.top -
			 GetControlValue(window->vScrollbar) * (*theTE)->lineHeight;
	if (change != 0) TEScroll(0, change, theTE);
}


/*
 * Adjust the text handle view when the window has been resized
 */
 
static void TWSetTextView(TEXTWINDOW window)
{
	TEHandle theTE = window->textEdit;
	WindowPtr theWP = (WindowPtr) window;
	
	(*theTE)->viewRect = theWP->portRect;
	
	/*	Adjust for the scrollbars ... */
	(*theTE)->viewRect.right -= 16;
	
	InsetRect(&(*theTE)->viewRect, 4, 4);
	
	window->linesInWindow = ((*theTE)->viewRect.bottom - (*theTE)->viewRect.top) /
								(*theTE)->lineHeight;
	(*theTE)->viewRect.bottom = (*theTE)->viewRect.top +
								((*theTE)->lineHeight * window->linesInWindow);
	(*theTE)->destRect.right = (*theTE)->viewRect.right;
	TECalText(theTE);
}

/*
 * Change the size of the window using the mouse on the grow box
 */
 
void TWResizeWindow(TEXTWINDOW window,int w, int h)
{
	GrafPtr savePort;
	Rect scrollRect;
	short cntlheight,lineHeight = (*window->textEdit)->lineHeight;
	WindowPtr theWindow = (WindowPtr) window;

	GetPort(&savePort);
	SetPort(theWindow);
	
	EraseRect(&theWindow->portRect);

	SizeWindow(theWindow,w,h,false);

	InvalRect(&theWindow->portRect);
	TWSetTextView(window);

	cntlheight = theWindow->portRect.top - 1;
	MoveControl(window->vScrollbar,
			theWindow->portRect.right - 15,
			cntlheight);
	
	/*	16 pixels wide and top - bottom of screen plus 1 at both ends ... */
	cntlheight = theWindow->portRect.bottom - theWindow->portRect.top - 13;

	SizeControl(window->vScrollbar, 16, cntlheight);
	scrollRect = (*(window->vScrollbar))->contrlRect;
	ValidRect(&scrollRect);
	
	TWUpdateScrollbar(window);
	TWAdjustText(window);
	
	SetPort(savePort);
}
 
 
void TWGrowWindow(TEXTWINDOW window, Point thePoint)
{
	GrafPtr savePort;
	Rect dragRect;
	long result;
	short lineHeight = (*window->textEdit)->lineHeight;
	WindowPtr theWindow = (WindowPtr) window;
    
	GetPort(&savePort);
	SetPort(theWindow);
	
    dragRect = (**grayRgnHandle).rgnBBox;
	result = GrowWindow(theWindow, thePoint, &dragRect);
	if (result == 0) return;

	SetPort(savePort);

    TWResizeWindow(window,(((LoWord(result) - 24) / CharWidth('0')) * CharWidth('0')) + 24,
			   (((HiWord(result) - 8) / lineHeight) * lineHeight) + 8);
}


/*
 * Action related to the real time scrolling 
 */

static TEXTWINDOW theActionTextWindow;

static pascal void VActionProc(ControlHandle theControl, short part)
{
	TEHandle theTE = theActionTextWindow->textEdit;
	short scrollAdjust;

	if (part == 0) {
		return;
	}

	switch (part) {
		case kControlUpButtonPart:
		case kControlDownButtonPart:
			scrollAdjust = 1;
			break;
		case kControlPageUpPart:
		case kControlPageDownPart:
			scrollAdjust = ((*theTE)->viewRect.bottom - (*theTE)->viewRect.top) /
			   		 	   ((*theTE)->lineHeight);
			break;
	}

	if ((part == kControlDownButtonPart) || (part == kControlPageDownPart)) {
		scrollAdjust = -scrollAdjust;
	}
	TWMoveScrollBox(theControl, scrollAdjust);
	TWAdjustText(theActionTextWindow);
}

/*
 * Update the scrollbar according to the text handle
 */
 
void TWUpdateScrollbar(TEXTWINDOW window)
{
	short topLineNumber;
	short lines;
	TEHandle theTE = window->textEdit;

	topLineNumber = ((*theTE)->viewRect.top - (*theTE)->destRect.top) /
					((*theTE)->lineHeight);
	
	lines = (*theTE)->nLines - window->linesInWindow;
	/*	Following is to correct a bug in TextEdit where if the last char */
	/*	is an eol the line count will be wrong ... */
	if (((*theTE)->teLength > 0) &&
		((*(*theTE)->hText)[(*theTE)->teLength - 1] == 0x0d)) lines++;
		
	if (lines < 0) lines = 0;

	SetControlMaximum(window->vScrollbar, lines);
	SetControlValue(window->vScrollbar, topLineNumber);
	
	window->scrollbarDirty = false;
}


/*
 *  Refresh of a text window 
 */
 
void TWRefresh(TEXTWINDOW window)
{
	GrafPtr savePort;
	WindowPtr windowPtr = (WindowPtr) window;
	
	GetPort(&savePort);
	SetPort(windowPtr);

	BeginUpdate(windowPtr);
	EraseRect(&windowPtr->portRect);
	DrawControls(windowPtr);
	DrawGrowBox(windowPtr);
	TEUpdate(&windowPtr->portRect,window->textEdit);

	EndUpdate(windowPtr);

	SetPort(savePort);
}

/**********************************************
 *	Check if insertion point is in edit range ??? STart > first ??
 **********************************************/
Boolean IsInEditRange(short first, TEHandle te)
{
	if (((*te)->selStart < first) || (*te)->selEnd < first) {
		SysBeep(10);
		return false;
	} else
		return true;
}


/********************************************************
 * Handle Mouse Events in the content of a text window 
 ********************************************************/
void TWDoContentClick(TEXTWINDOW window, EventRecord *theEvent)
{
	short part;
	ControlHandle theControl;
	GrafPtr savePort;
    WindowPtr windowPtr = (WindowPtr) window;

	GetPort(&savePort);
	SetPort(windowPtr);
	GlobalToLocal(&theEvent->where);

	if ((part = FindControl(theEvent->where, windowPtr, &theControl)) == 0) {
		/*	Check to see if the user clicked in the textWindow ... */
		if (PtInRect(theEvent->where, &((*window->textEdit)->viewRect))) {
		
		    /* We must remember the last position of the cursor */
		    if ((*window->textEdit)->selStart == (*window->textEdit)->selEnd) {
		      window->selStart = (*window->textEdit)->selStart;
		    }
		    
		    /* Then perform the click */
			TEClick(theEvent->where,(theEvent->modifiers & shiftKey) != 0,window->textEdit);
			
			/* If we have not selected something, go back to the cursor */
			if ((*window->textEdit)->selStart == (*window->textEdit)->selEnd) {
			  TESetSelect(window->selStart, window->selStart, window->textEdit);  
			}
		
			
/*			if ((*window->textEdit)->selStart == (*window->textEdit)->selEnd) {
			  if (!IsInEditRange(window->selStart, window->textEdit))
				TESetSelect(window->selStart, window->selStart, window->textEdit);
			  else if ((*window->textEdit)->lineStarts[(*window->textEdit)->nLines-1] > 
			           (*window->textEdit)->selStart)
			    TESetSelect(oldStart, oldEnd, window->textEdit);
			} */
			TWUpdateScrollbar(window);
		}
	} else if (part == kControlIndicatorPart) {
		TrackControl(theControl, theEvent->where, 0L);
		TWAdjustText(window);
	} else {
		ControlActionUPP tempUPP;
		theActionTextWindow = window;
		tempUPP = NewControlActionProc(VActionProc);
		TrackControl(theControl, theEvent->where, tempUPP);
		DisposeRoutineDescriptor(tempUPP);
	}
	SetPort(savePort);
}


/******************************************
 * Handle activate events of Text windows
 ******************************************/
Boolean HandleTWUpdateActivateEvent(TEXTWINDOW window, EventRecord *theEvent)
{
/*   if (FrontWindow() == (WindowPtr) window) { */
		if (theEvent->what == updateEvt) TWRefresh(window);
		else {	/* must be an activate/deactivate event */
			if (theEvent->modifiers & activeFlag) {
				TEActivate(window->textEdit);
				ShowControl(window->vScrollbar);
			} else {
				TEDeactivate(window->textEdit);
				HideControl(window->vScrollbar);
			}
			DrawGrowBox((WindowPtr) window);
		}
		return true;
	/*}
	return false; */
}


/*************************************
 * Allocation of a text window
 *************************************/
TEXTWINDOW NewTextWindow(int nRows,int nCols,char *title, TWTYPE type)
{
	Rect aRect = {0,0,1,1};
	FontInfo fi;
	short lineHeight, screenHeight, screenWidth;
    GrafPtr saveport;
    TEXTWINDOW window;
    char str[100];
    
    /* Test if it is possible to open a new one */
    if (nTextWindows == NTW) return(NULL);
    
	GetPort(&saveport);
	window = (TEXTWINDOW) NewPtr(sizeof(struct TextWindow));
	if (window == 0) return(NULL);

	if (NewCWindow((Ptr)window, &aRect, AppName, false, 
	              documentProc, (WindowPtr) -1L, false, 0L) == 0L)
		return(NULL);

	SetPort((WindowPtr) window);

    GetFNum(CtoPstr("monaco"),&(window->fontId));
    window->fontSize = 9;
    window->fontType = normal;
    window->nColumns = nCols;
    window->nRows = nRows;
    window->state = IDLE;
    window->selStart = 0;
    window->buffer = XXNewTextWindowBuffer(bufferSizeDefault);
    window->type = type;
    window->scrollbarDirty = false;
    
	/*	Set the font information ... */
	TextFont(window->fontId);
	TextSize(window->fontSize);
	TextFace(window->fontType);
	GetFontInfo(&fi);
	lineHeight = fi.ascent + fi.descent + fi.leading;

	aRect.bottom = 2*4 + window->nRows*lineHeight + 4;	/*	2*indent + ... + titlebar */
	aRect.right = 2*4 + window->nColumns*CharWidth('0') + 16,	/*	2*indent + ... + scrollbar */

	screenHeight = qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - GetMBarHeight() - 24;	/*	screen height ... */
	screenWidth = qd.screenBits.bounds.right - qd.screenBits.bounds.left;

	if (aRect.bottom > screenHeight)
		aRect.bottom = ((screenHeight - 2*4 - 4) / lineHeight) * lineHeight + 2*4 + 4;

	if (aRect.right > screenWidth) /*	The window is wider than the screen */
		aRect.right = ((screenWidth - 2*4 - 16 - 20) / CharWidth('0')) * CharWidth('0') + 2*4 + 16;
		MoveWindow ((WindowPtr)window,
					((qd.screenBits.bounds.right - qd.screenBits.bounds.left - aRect.right) / 2),
					((screenHeight - aRect.bottom) / 5 + GetMBarHeight() + 24),
					false);

	/*	Now make it the correct size ... */
	SizeWindow((WindowPtr)window, aRect.right, aRect.bottom, true);

	/*	Create the vertical scrollbar ... */
	aRect = ((WindowPtr)window)->portRect;
	aRect.left = aRect.right - 15;
	aRect.right += 1;
	aRect.bottom -= 14;
	aRect.top -= 1;

	window->vScrollbar = NewControl((WindowPtr)window, &aRect, "\p",
									 true, 0, 0, 0, scrollBarProc, 0L);


	if (title != NULL) strcpy(str,title);
	else sprintf(str,"Untitled %d",nTextWindows);
	SetWTitle((WindowPtr) window, CtoPstr(str));

	ShowWindow((WindowPtr)window);

	/*	Create the TEHandle ... */
	aRect = qd.thePort->portRect;
	aRect.right -= 15;

	InsetRect(&aRect, 4, 4);

	window->textEdit = TENew(&aRect, &aRect);

	TEAutoView(true, window->textEdit);
	
	window->linesInWindow = window->nRows;	

	TWRefresh(window);

	SetPort(saveport);
	
	theTextWindows[nTextWindows++] = window;
	
	return(window);
}

/*************************************
 * Desallocation of a text window
 *************************************/
void DeleteTextWindow(TEXTWINDOW window)
{
  int i;
  
  if (window == NULL) return;
  
  i = 0;
  while(theTextWindows[i] != window && i < nTextWindows) i++;

  if (i != nTextWindows) {
    for (;i<nTextWindows-1;i++) theTextWindows[i] = theTextWindows[i+1];
    theTextWindows[nTextWindows-1] = NULL;
    nTextWindows--;
  }
  
  XXDeleteTextWindowBuffer(window->buffer);

  KillControls((WindowPtr) window);
  TEDispose(window->textEdit);
  CloseWindow((WindowPtr) window);
	
  DisposePtr((Ptr) window);
}

/*************************************
 * Desallocation of all text window
 *************************************/
void DeleteAllTextWindows(void)
{
  int i;
  
  for(i=0;i<nTextWindows;i++) DeleteTextWindow(theTextWindows[i]);
}


/*
 * Test wether a window is a text window
 */
 
TEXTWINDOW IsTextWindow(WindowPtr win)
{
  int i;
   
  for (i=0;i<nTextWindows;i++) if (((WindowPtr) theTextWindows[i]) == win) break;
  
  if (i == nTextWindows) return(NULL);
  return(theTextWindows[i]);
}

/*
 *  The Quit request
 */

static char flagQuit = 0; 
  
void TWSetQuit(void)
{
  flagQuit = 1;
}

char TWQuit(void)
{
  return(flagQuit);
}

#endif
