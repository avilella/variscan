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



#include "lastwave.h"

#include "tabulate.h"


// MANU->REMI : a terme j'incluerais ces fonctions  dans le 
// package signal (mais ANTISYMETRIC me gene
// et en plus j'ai un autre pading possible qui est 'pad'

/*
 * To deal with the various border types
 */

char  Name2BorderType(char *name)
{
  if(!strcmp(name,"pad0")) return(BorderPad0);
  else if(!strcmp(name,"per")) return(BorderPeriodic);
  else if(!strcmp(name,"mirror")) return(BorderMirror);
  else Errorf("Unknown borderType name '%s'",name);

  //This should never be reached but the compiler may want to be sure the function returns something
  return(-1);
}

/* REMI : J'ai change les noms pour etre homogene !! */

char * BorderType2Name(char borderType)
{
    if(!BorderTypeIsOK(borderType))
    {
	Errorf("BorderType2Name : Unknown borderType %d",borderType);
    }

    switch(borderType)
    {
    case BorderPad0 :
	return("pad0");
    case BorderPeriodic :
	return("per");
    case BorderMirror :
	return("mirror");
    default :
	Errorf("Unknown borderType %d",borderType);
	return("");
   }
}

/* EOF */

