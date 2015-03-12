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

#include <Controls.h>
#include <TextEdit.h>
#include <Types.h>
#include <Windows.h>
#include <ToolUtils.h>
#include <Scrap.h>
#include <LowMem.h>
#include <Devices.h>

#include "MacTextWindow.h"
#include "MacMenu.h"
#include "Mac.h"
#include "lastwave.h"

/*	Menuhandles ... */
static MenuHandle appleMenu;
static MenuHandle fileMenu;
static MenuHandle editMenu;

/************************************************************************/
/************************************************************************/
static OSErr MyTEFromScrap(void)
{
	long length, scrapOffset;

	if ((length = GetScrap(0L,'TEXT',&scrapOffset))<0)
		return (length);

	if (length>32000)
		return (teScrapSizeErr);

	HLock(TEScrapHandle());
	if ((length = GetScrap(TEScrapHandle(),'TEXT',&scrapOffset))<0) {
		HUnlock(TEScrapHandle());
		return (length);
	}
	HUnlock(TEScrapHandle());
 	LMSetTEScrpLength(length);
	return (noErr);
}

static OSErr MyTEToScrap(void)
{
	long	error;

	ZeroScrap();
	HLock(TEScrapHandle());
	error = PutScrap(LMGetTEScrpLength(), 'TEXT', *TEScrapHandle());
	HUnlock(TEScrapHandle());

	return error;
}

/****************************************************************/
/****************************************************************/
void SetupMenus(void)
{
	/*	Create the Apple menu ... */
	appleMenu = NewMenu(APPLEID,"\p");
	AppendMenu(appleMenu, "\pAbout LastWave;(-");
	AppendResMenu(appleMenu, 'DRVR');
	InsertMenu(appleMenu, 0);

	/*	Create the File menu ... */
	fileMenu = NewMenu(FILEID, "\pFile");
	AppendMenu(fileMenu, "\p(New;(Open...;(Close...;Save/S;(-;Page Setup;Print.../P;(-;Quit/Q");
	InsertMenu(fileMenu, 0);

	/*	Create the Edit menu ... */
	editMenu = NewMenu(EDITID, "\pEdit");
	AppendMenu(editMenu, "\p(Undo/Z;(-;(Cut/X;(Copy/C;Paste/V;(Clear;(-;Select All/A");
	InsertMenu(editMenu, 0);

	DrawMenuBar();
}

/****************************************************************/
/****************************************************************/
void UpdateMenuItems(void)
{
  WindowPtr frontFrame;
  TEXTWINDOW frontTWindow;

  frontFrame = FrontWindow();  
  frontTWindow = IsTextWindow(frontFrame);
  
  DisableItem(fileMenu, FILENEW);
  DisableItem(fileMenu, FILEOPEN);
  DisableItem(fileMenu, FILECLOSE);
  DisableItem(fileMenu, FILESAVE);
  DisableItem(fileMenu, FILEPRINT);
  DisableItem(fileMenu, FILEPAGESETUP);
  EnableItem(appleMenu, APPLEABOUT);
  EnableItem(editMenu, FILEQUIT);
  DisableItem(editMenu, EDITCUT);
  EnableItem(editMenu, EDITCOPY);
  EnableItem(editMenu, EDITPASTE);
  DisableItem(editMenu, EDITCLEAR);
  DisableItem(editMenu, EDITSELECTALL);

  if (frontTWindow == NULL) return;
  
  if (frontTWindow->state != PRINTFING) {
  	
   if ((*frontTWindow->textEdit)->lineStarts[(*frontTWindow->textEdit)->nLines-1] <= 
	   (*frontTWindow->textEdit)->selStart) 
      EnableItem(editMenu, EDITPASTE);

   if ((*frontTWindow->textEdit)->selStart != (*frontTWindow->textEdit)->selEnd) 
  	  EnableItem(editMenu, EDITCOPY);

  }
}


/****************************************************************/
/****************************************************************/

void DoEditCut(TEXTWINDOW window)
{
	if (IsInEditRange(window->selStart, window->textEdit) && 
	    (*window->textEdit)->selStart != (*window->textEdit)->selEnd) {
		TECut(window->textEdit);
		MyTEToScrap();
		TWUpdateScrollbar(window);
	} 
}

/****************************************************************/
/****************************************************************/
void DoEditCopy(TEXTWINDOW window)
{
  if ((*window->textEdit)->selStart != (*window->textEdit)->selEnd) {
		TECopy(window->textEdit);
		MyTEToScrap();
  } 
}

/****************************************************************/
/****************************************************************/
void DoEditPaste(TEXTWINDOW window)
{
  Handle handle;
  long scrapOffset;
  long length;
  char *str;
  
  handle = NewHandle(0);
  if ((length = GetScrap(handle,'TEXT',&scrapOffset)) < 0) return;

  SetHandleSize(handle,length+1);

  HLock(handle);
  (*handle)[length] = 0;

  str = (char *) *handle;
  while (*str != '\0') {
    TermBufferPushKey(*str);
    str++;
  }
  
  HUnlock(handle);
}
  
  
/****************************************************************/
/****************************************************************/
void DoEditClear(TEXTWINDOW window)
{
  if (IsInEditRange(window->selStart, window->textEdit) && 
      (*window->textEdit)->selStart != (*window->textEdit)->selEnd) {
		TEDelete(window->textEdit);
		TWUpdateScrollbar(window);
	} 
}

/****************************************************************/
/****************************************************************/
void DoEditSelectAll(TEXTWINDOW window)
{
  TESetSelect(0, 32767, window->textEdit);
  TWUpdateScrollbar(window); 
}


/****************************************************************/
/****************************************************************/

void DoMenuChoice(long menuValue)
{
  short 	theMenu 		= HiWord(menuValue);
  short 	theMenuItem 	= LoWord(menuValue);
  Str255 	accName;
  WindowPtr frontFrame;
  TEXTWINDOW frontTWindow;  

  frontFrame = FrontWindow();  
  frontTWindow = IsTextWindow(frontFrame);

  switch(theMenu) {
  
  case APPLEID:
    switch (theMenuItem) {
    case APPLEABOUT:
	  /*MTDoAboutBox(); */
	  break;
    default: /*	ie the apple menu items ... */
	  GetMenuItemText(appleMenu, theMenuItem, accName);
	  OpenDeskAcc(accName);
	  break;
    }
	break;

  case FILEID:
    switch (theMenuItem) {
	case FILENEW:
	  NewTextWindow(nRowsDefault,nColsDefault,NULL,TEXTFILE);
	  break;
	case FILECLOSE:
	 DeleteTextWindow(frontTWindow);
	case FILESAVE:
	  /*MTDoSaveText(); */
	  break;
	case FILEPAGESETUP:
	  /*	MTDoPageSetup(); */
	  break;
	case FILEPRINT:
	  /*	MTDoPrintText(); */
	  break;
	case FILEQUIT:
      TWSetQuit();
	  break;
	default:
	  break;
	}
	break;
		
  case EDITID:
	switch (theMenuItem) {
	case EDITCUT:
	  DoEditCut(frontTWindow);
	  break;
	case EDITCOPY:
	  DoEditCopy(frontTWindow);
	  break;
	case EDITPASTE:
	  DoEditPaste(frontTWindow);
	  break;
	case EDITCLEAR:
	  DoEditClear(frontTWindow);
	  break;
	case EDITSELECTALL:
	  DoEditSelectAll(frontTWindow);
	  break;
	default:
	  break;
	}
  break;

  default:
	break;
  }

  HiliteMenu(0);
}

#endif



