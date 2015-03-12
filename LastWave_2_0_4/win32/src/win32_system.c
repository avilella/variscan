/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0                               */
/*                                                                          */
/*      Copyright (C) 2001 Emmanuel Bacry, Remi Gribonval.                  */
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


/********************************************************************/
/*                                                                  */
/*   win32_system.c :                                               */
/*                                                                  */
/********************************************************************/

#include "computer.h"

#ifdef LASTWAVE_WIN32_SYSTEM
#include <windows.h>
//#include <mmsystem.h>

//#include "lastwave.h"
#include "xxmisc.h"
//#include "signals.h"
//#include "soundlw.h"
#include <stdio.h>
extern void Errorf(char *format,...);
extern void Warningf(char *format,...);
extern void Printf(char *format,...);
extern void Flush(void);
extern char* CharAlloc(int size);
extern float *FloatAlloc(int size);
extern void Free(void * ptr);

char *XXGetHistoryFile()
{
  return(".LastWaveHistory");
}

char *XXConvertFilename(char *file)
{
  static char* filename = NULL;
  static unsigned int filename_size  = 0;
  
  char * f  = file;
  char * f1 = NULL;
  
  /* (re)allocation if necessary */
  if(filename == NULL) {
    filename = CharAlloc(512);
    filename_size = 512;
  }
  while(strlen(file)+1 > filename_size) {
    filename       = realloc(filename,2*filename_size*sizeof(char));
    filename_size *= 2;
  }
  
  f1 = filename;
  while(*f != '\0') {
    if(*f != '/') {
      *f1 = *f;
    }
    else {
      *f1 = '\\';
    }
    f++;
    f1++;
  }
  *f1 = '\0';
  return(filename);
}


void XXStartup(int *argc, char ***argv)
{
  *argc = 0;
}

void XXGetSystemInfo(char **home,char **system,char **computer,char **termType,char **scriptDir)
{
  *home = "";
  *system = "win32";
  *computer = "win32";
#ifdef LASTWAVE_WIN32_GRAPHICS
  *termType = "win32";
#endif
#ifdef LASTWAVE_JAVA_GRAPHICS
  *termType = "java";
#endif
  *scriptDir = getenv("LWSOURCEDIR");
  if(*scriptDir==NULL)
    *scriptDir = "scripts";
}


/*
 * Get the filenames of the current directory
 */


char *XXGetFilenames(char *dirName)
{
  static char *extDirName = NULL;
  static HANDLE file = NULL;
  static WIN32_FIND_DATA fileData;
  
  // Allocating (ONCE)
  if(extDirName == NULL) {
    if((extDirName=CharAlloc(MAX_PATH))==NULL)
      Errorf("XXGetFilenames : Mem. Alloc failed for static extDirName");
  }
  
  // Initialization of the function
  if(dirName != NULL) {
    if(strlen(dirName)+1>MAX_PATH) {
      Errorf("XXGetFilenames : dirName is too long");
    }
    //		Printf("Searching files in directory '%s'\n",dirName);
    sprintf(extDirName,"%s/*",dirName);
    if(file != NULL) { 
      FindClose(file);
      file = NULL;
    }
    return(NULL);
  }
  
  // Read the next filename till it reaches something different from . and .. 
  if(file == NULL) {
    //		Printf("Searching first file\n");
    file = FindFirstFile(extDirName,&fileData);
    //		if(file != NULL) Printf("... found first file '%s'\n",fileData.cFileName);
  }
  else {
    if(FindNextFile(file,&fileData)==0) {
      FindClose(file);
      file = NULL;
      return(NULL);
    }
  }
  while(file != NULL && 
	((!strcmp(fileData.cFileName,".")) || (!strcmp(fileData.cFileName,"..")) ||
	 (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))) {
    //		Printf("Skipping file %s\n",fileData.cFileName);
    if(FindNextFile(file,&fileData)==0) {
      FindClose(file);
      file = NULL;
      return(NULL);
    }
  }
  
  //	if(file != NULL) Printf("... found file '%s'\n",fileData.cFileName);
  return(fileData.cFileName);
}


/*
 * Get a filename info
 */

void XXGetFilenameInfo(char *filename,int *type,int *size)
{
  DWORD fileAttribute;
  HANDLE file;
  WIN32_FIND_DATA fileData;
  
  
  fileAttribute = GetFileAttributes(filename); 
  if(fileAttribute == 0xFFFFFFFF)
    Errorf("XXGetFilenameInfo : Windows function 'GetFileAttributes' failed");
  
  if(fileAttribute & FILE_ATTRIBUTE_DIRECTORY) {
    *type = DirectoryFile;
    file = FindFirstFile(filename,&fileData);
    *size = (fileData.nFileSizeHigh * MAXDWORD) + fileData.nFileSizeLow;
  }
  else if(fileAttribute & FILE_ATTRIBUTE_NORMAL) {
    *type = RegularFile;
    file = FindFirstFile(filename,&fileData);
    *size = (fileData.nFileSizeHigh * MAXDWORD) + fileData.nFileSizeLow;
    FindClose(file);
  }
  else {
    *type = UnknownTypeFile;
    file = FindFirstFile(filename,&fileData);
    *size = (fileData.nFileSizeHigh * MAXDWORD) + fileData.nFileSizeLow;
  }
}


/*
 * Change current directory
 */

void XXChangeDirectory(char *directory)
{
  Errorf("XXChangeDirectory() : Not Implemented in Win32");
} 

#endif//LASTWAVE_WIN32_SYSTEM

