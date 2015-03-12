/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e   K e r n e l   2 . 0 .4                            */
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


/********************************************************************/
/*                                                                  */
/*   mac_system.c :                                                    */
/*                                                                  */
/********************************************************************/

#include "computer.h"

#ifdef LASTWAVE_MAC_SYSTEM

#include "lastwave.h"
#include "xx_system.h"
#include "xx_audio.h"
#include "signals.h"

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


#ifndef PtoCstr

static char theCString[256];

static char *PtoCstr(Str255 pstring)
{
  unsigned char *p = (unsigned char *) pstring;
  int len;
  
  len = p[0];
  
  strncpy(theCString,(char*) (p+1),len);
  theCString[len] = '\0';
  
  return(theCString);
}

#endif

char *XXGetHistoryFile()
{
  return("LastWaveHistory");
}

char *XXConvertFilename(char *file)
{
  static char filename[400];
  char * f = filename +1;
  char *  f1 = file;
  char flagDir = NO;
  char flagFather = NO;
  char flagRoot = NO;
  
  f = file;
  f1 = filename;
   
   
  if (*f == '\0') {
    *f1 = '\0';
    return(filename);
  }

  if (*f == '.'  && *(f+1)== '\0') {
    *f1 = ':';
    *(f1+1) = '\0';
    return(filename);
  }

  if (*f == '.'  && *(f+1)== '.' && *(f+2)== '.') {
    *f1 = ':';
    *(f1+1) = ':';
    *(f1+2) = '\0';
    return(filename);
  }
  
  if (*f == '/') {
    f++;
    flagRoot = YES;
  }
  else if (*f == '.' && *(f+1)== '/') {
    f+=2;
    *f1++ = ':';
  }
  else if (*f == '.' && *(f+1)== '.' && *(f+2)== '/') {
    f+=3;
    *f1++ = ':';
    *f1++ = ':';
    flagFather = YES;
  }
  
   
  /* We consider we are right after a / */
    
  while (*f) {
   
    if (*f == '.') {
      f++;
      if (*f == '/' || *f == '\0') {
        if (*f == '/') f++;
      }
      
      else if (*f == '.') {
        f++;
        if (*f == '/' || *f == '\0') {
          if (*f == '/') f++;
          *f1++ = ':'; 
          if (flagFather == NO) *f1++ = ':'; 
          flagFather = YES;
          continue;
        }
        else {
          *f1++ = '.';
          *f1++ = '.';
          while (*f && *f != '/') *f1++ = *f++;
          if (*f == '/') f++;
        }
      }
      else {         
        *f1++ = '.';
        while (*f && *f != '/') *f1++ = *f++;
        if (*f == '/') f++;
      }      
    }
    else {
      if (flagRoot == NO && flagFather == NO) *f1++ = ':';  
      while (*f && *f != '/') *f1++ = *f++;
      if (*f == '/') f++;
    }
    flagFather = NO;
    flagRoot = NO;
  }
  
  *f1 = '\0';
  
  
  return(filename);
          
/*          
    }
    else if (*f == '.') {
      f++;
      if (*f == '.') {
        f++;
        *f1++ = ':';
        *f1++ = ':';
      }
      else if (*f == '/') {
        f++;
        if (*f1
        


  if (*f1 == '.' && *(f1+1) == '\0') f1++;
  if (*f1 == '.' && *(f1+1) == '/') f1+=2;
  if (*f1 == '/') f1++;
  while (*f1 != '\0') {
    if (*f1 != '/') *f++ = *f1++;
    else { 
      *f++ = ':';
      f1++;
      flagDir = YES;
    }
  }
  *f = '\0';
  
  if (flagDir == YES && *(filename+1) != ':') {
    *filename = ':';
    return(filename);
  }
  else return(filename+1); */
}


void XXStartup(int *argc, char ***argv)
{
  *argc = 0;
}

void XXGetSystemInfo(char **home,char **system,char **computer,char **termType,char **scriptDir,char **user)
{
  *user = "unkown";
  *home = "";
  *system = "mac";
  *computer = "mac";
  *termType = "mac";
  *scriptDir = "scripts";
}


/* The directory Id of the lastwave directory */
static long theDirId;
static char flagInitDir = NO;

static void _InitDirectory(void)
{
  static CInfoPBRec	cipbr;				
  static HFileInfo	*fpb = (HFileInfo *)&cipbr;
  static DirInfo	*dpb = (DirInfo *) &cipbr;
  short rc;

  /* default volume */
  fpb->ioVRefNum = 0;	
  
  /* default directory */
  fpb->ioFDirIndex = 0;  
      
  /* Look for the dirId of the current directory */ 
  fpb->ioNamePtr = CtoPstr("");
  rc = PBGetCatInfo( &cipbr, false );
  if (rc || !(fpb->ioFlAttrib & 16)) theDirId = 0;
  else theDirId = dpb->ioDrDirID;
  
  flagInitDir = YES;
}


/* Create a directory */
char XXCreateDirectory(char *directory,char *name)
{
  CInfoPBRec	cipbr;				
  HFileInfo	*fpb = (HFileInfo *)&cipbr;
  DirInfo	*dpb = (DirInfo *) &cipbr;
  short rc;
  long dirId,ndirId;
  int answ;
  
  if (flagInitDir == NO) _InitDirectory();

  /* default volume */
  fpb->ioVRefNum = 0;	
  
  /* default directory */
  fpb->ioDirID = theDirId;
  fpb->ioFDirIndex = 0;
      
  /* Look for the dirId of the  directory */ 
  fpb->ioNamePtr = CtoPstr(ConvertFilename(directory));
  rc = PBGetCatInfo( &cipbr, false );
  if (rc || !(fpb->ioFlAttrib & 16)) return(NO);
  dirId = dpb->ioDrDirID;
  
  switch (DirCreate(fpb->ioVRefNum,dirId,CtoPstr(name),&ndirId)) {
  case noErr : answ = YES; break;
  default : answ = NO;
  }
  
  PtoCstr((unsigned char *) name);
  PtoCstr((unsigned char *) directory);
}

/*
 * Get the filenames of the current directory
 */

 
char *XXGetFilenames(char *dirName)
{
    static char *dir = NULL;
    static long dirID;
    static Str255 filename;
    static short idx = 1;
    
	static CInfoPBRec	cipbr;				
	static HFileInfo	*fpb = (HFileInfo *)&cipbr;
	static DirInfo	*dpb = (DirInfo *) &cipbr;
	short rc;

	char *file;

    if (flagInitDir == NO) _InitDirectory();

    /* Initialization */
    if (dirName != NULL) {
      if (dir != NULL) Free(dir);
      dir = CopyStr(ConvertFilename(dirName));

      /* default volume */
	  fpb->ioVRefNum = 0;		
	  
	  /* default directory */
      fpb->ioDirID = theDirId;
      fpb->ioFDirIndex = 0;
      
	  /* Look for the dirId of this directory */ 
	  fpb->ioNamePtr = CtoPstr(dir);
	  rc = PBGetCatInfo( &cipbr, false );
	  if (rc) return(NULL);
	  if (!(fpb->ioFlAttrib & 16)) return(NULL);
	  dirID = dpb->ioDrDirID;
	 
	  /* buffer to receive name */ 	  
	  fpb->ioNamePtr = filename;	
	  idx = 1;
	  return(dirName);
    } 
    else if (dir == NULL) Errorf("XXGetFilenames1() : Weired Error");
    
    /* Looking for file number idx in directory dirId */
    fpb->ioDirID = dirID;		
    fpb->ioFDirIndex = idx;
    rc = PBGetCatInfo( &cipbr, false );
    
    /* No More files */
	if (rc) {
	  Free(dir);
	  dir = NULL;
	  return(NULL);	
	}

	file = PtoCstr(filename);
	idx++;
	
	/* Case it is a directory */
	if (fpb->ioFlAttrib & 16) return(file);
	
	/* Case it is a regular file */
    return(file);
}


/*
 * Get a filename info
 */

void XXGetFilenameInfo(char *filename,int *type,int *size)
{
    static char *dir = NULL;
    static long dirID;
    static short idx = 1;
    
	static CInfoPBRec	cipbr;				
	static HFileInfo	*fpb = (HFileInfo *)&cipbr;
	static DirInfo	*dpb = (DirInfo *) &cipbr;
	short rc;

    if (flagInitDir == NO) _InitDirectory();
    
    /* default volume */
	fpb->ioVRefNum = 0;		
	  
	/* default directory */
    fpb->ioDirID = theDirId;
    fpb->ioFDirIndex = 0;
     
	/* Look for the file in this directory */ 
	fpb->ioNamePtr = CtoPstr(ConvertFilename(filename));
	
	rc = PBGetCatInfo( &cipbr, false );
	if (rc) {
	  *type = UnknownTypeFile;
	  return;
	}
	if (!(fpb->ioFlAttrib & 16)) {
	  *type = RegularFile;
	  return;
	}
	*type = DirectoryFile;
	
	*size = 0;
	return;
}


/*
 * Change current directory
 */

void XXChangeDirectory(char *directory)
{
  Errorf("XXChangeDirectory() : Not Implemented");
} 

#endif

