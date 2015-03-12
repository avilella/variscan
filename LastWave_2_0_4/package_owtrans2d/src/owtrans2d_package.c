/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'owtrans2d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1998-2002  Emmanuel Bacry, Jerome Fraleu.             */
/*                                                                          */
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
#include "owtrans2d.h"

/*
 * The file that contains CProcs for Image 
 *
 */

extern  void C_OWt2d();
extern  void C_OWt2r();
extern  void C_OWt2f();


static CProc owtrans2Commands[] = {
      "owt2d", C_OWt2d,"{{{[<owtrans2>=objCur] <noct>} {Performs wavelet decomposition of the image (in 0) in <owtrans2> on <noct> octaves.}}}", 
      "owt2r", C_OWt2r,"{{{[<owtrans2>=objCur] [<image>=0]} {Performs wavelet reconstruction and stores the result in the <image> \
(default is original image).}}}", 
      "owt2f",C_OWt2f,"{{{<waveletName>} {Sets the default wavelet that will be used for the next decomposition. It must be one among \
'Haar', 'Daub4', 'Daub6', 'Daub8', 'Antonini', 'Villa', 'Adelson', 'Brislawn', 'Brislawn2', 'Villa1', \
'Villa2', 'Villa3', 'Villa4', 'Villa5', 'Villa6', 'Odegard'}}}",

  NULL,NULL,NULL
};

static  CProcTable  owtrans2Table = {owtrans2Commands, "owtrans2d", "Commands related to the owtrans2d package"};


/***********************************
 * 
 * Loading/Adding the owtrans2d package
 *
 ***********************************/

int tOWTRANS2,tOWTRANS2_; 

void LoadOWtrans2Package(void) 
{ 
  tOWTRANS2 = AddVariableTypeValue(owtrans2Type,&tsOWtrans2, NULL);
  tOWTRANS2_ = tOWTRANS2+1;
  
  AddCProcTable(&owtrans2Table);
  
  /* Init the wavelets */
  InitOWavelet2();
} 


void DeclareOWtrans2Package(void)
{  
  DeclarePackage("owtrans2d",LoadOWtrans2Package,1998,"2.0","G.Davies, E.Bacry and J.Fraleu",
"Package allowing to perform 2d orthogonal wavelet decomposition/reconstruction.");
}

