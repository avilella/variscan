/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'compress2d' 2.0                   */
/*                                                                          */
/*      Copyright (C) 1998-2002 Emmanuel Bacry, Jerome Fraleu.              */
/*      emails : fraleu@cmap.polytechnique.fr                               */
/*               lastwave@cmap.polytechnique.fr                             */
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
#include "compress2d.h"

/*
 * 
 *
 */

extern void C_Decode2(), C_Code2();

static CProc compress2dCommands[] = {

 "code2",C_Code2,"{{{[<owtrans2>=objCur] <filename> <quantStep>} {A uniform quantization is performed on the <owtrans2> \
using a quantification step <quantStep> (except around the value 0 for which the quantization step is 2*<quantStep>). \
Then it performs an arithmetic coding and stores the result in <filename>}}}", 
 "decode2",C_Decode2,"{{{[<owtrans2>] <filename> [<image>]} {From a file <filename> which has been generated using the \
command 'code2', it decodes the wavelet transform of the image in <wtrans> and then runs the reconstruction and puts the \
reconstructed image in the level 0 of the <owtrans2> or in the <image> if specified}}}",
 
  NULL,NULL,NULL
};

static  CProcTable  compress2dTable = {compress2dCommands, "compress2d", "Commands related to the compress2d package"};

void LoadCompress2Package(void) 
{ 
  AddCProcTable(&compress2dTable);
} 


void DeclareCompress2Package(void)
{  
  DeclarePackage("compress2d",LoadCompress2Package,1998,"2.0","G.Davies, E.Bacry and J.Fraleu",
  "Package allowing to compress images using a uniform quantization of the 2d orthogonal wavelet transform along with arithmetic coding.");
}

