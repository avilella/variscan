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


/********************************************************************/
/*                                                                  */
/*   unix_system.c :                                                    */
/*                                                                  */
/********************************************************************/

#include "lastwave.h"
#include "xx_system.h"

#ifdef LASTWAVE_UNIX_SYSTEM

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>



char *XXGetHistoryFile()
{
  return(".LastWaveHistory");
}

char *XXConvertFilename(char *file)
{
  return(file);
}

void XXSartup(int *argc, char ***argv)
{
  /* Skip the frirst argument */
  (*argc)--;
  (*argv)++;
  
  /* If next arguyment is '-b' then no graphic */
  if ((*argc) > 0 && !strcmp((*argv)[0],"-b")) {
    SetGraphicMode(NO);
    (*argv)++;
    (*argc)--;
  }
  else SetGraphicMode(YES);
}

void XXGetSystemInfo(char **home,char **system,char **computer,char **termType,char **scriptDir, char **user)
{

  *user = getenv("USER");
  *home = getenv("HOME");
  *system = "unix";
  *termType = getenv("TERM");
  *scriptDir = getenv("LWSOURCEDIR");
  if (*scriptDir == NULL) *scriptDir = ".";

#ifdef HP
  *computer = "hp";
#endif
#ifdef DEC
  *computer = "dec";
#endif
#ifdef LINUX
  *computer = "linux";
#endif
#ifdef CYGWIN
  *computer = "cygwin";
#endif
#ifdef SGI
  *computer = "sgi";
#endif
#ifdef SUN
  *computer = "sun";
#endif
#ifdef RS6000
  *computer = "rs6000";
#endif
#ifdef OTHER
  *computer = "other";
#endif

}


/* Create a directory */
char XXCreateDirectory(char *directory,char *name)
{
  static char str[1000];
  sprintf(str,"%s/%s",directory,name);
  
  if (mkdir(str,S_IROTH | S_IXGRP | S_IRWXU) != 0) return(NO);
  return(YES);
}

/*
 * Get the filenames of the current directory
 */

char *XXGetFilenames(char *directory)
{
  static DIR *dir = NULL;
  struct dirent *de;

  /* Initialization of the function */
  if (directory != NULL) {
    if (dir != NULL) closedir(dir);
    dir = opendir(directory);
    if (dir == NULL) return(NULL);
    return(directory);
  }

  /* Read the next filename till it reaches something different from . and .. */
  de = readdir(dir);
  while (de != NULL && (!strcmp(de->d_name,".") || !strcmp(de->d_name,".."))) de = readdir(dir);

  /* End of the directory */
  if (de == NULL) {
    closedir(dir);
    dir = NULL;
    return(NULL);
  }

  /* Returns the next filename */
  return(de->d_name);
}


/*
 * Get a filename info
 */

void XXGetFilenameInfo(char *filename,int *type,int *size)
{
  struct stat buf;

  if (stat(filename,&buf) == -1) {
    *type = UnknownTypeFile;
    return;
  }

  if (type != NULL) {
    if (S_ISDIR(buf.st_mode)) *type = DirectoryFile;
    else if (S_ISREG(buf.st_mode)) *type = RegularFile;
    else *type = UnknownTypeFile;
  }

  if (size != NULL) {
    *size = buf.st_size;
  }
}


/*
 * Change current directory
 */

void XXChangeDirectory(char *directory)
{
  if (chdir(directory) == -1) Errorf("XXChangeDirectory() : Could not change to '%s'",directory);
} 
 
void XXStartup(int *argc, char ***argv)
{ 
  (*argc)--;
  (*argv)++;
  if ((*argc) > 0 && !strcmp((*argv)[0],"-b")) {
    SetGraphicMode(NO);
    (*argv)++;
    (*argc)--;
  }
  else SetGraphicMode(YES);

}

#endif
