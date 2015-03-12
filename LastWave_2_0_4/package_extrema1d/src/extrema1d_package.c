/*..........................................................................*/
/*                                                                          */
/*      L a s t W a v e    P a c k a g e 'extrema1d' 2.0                    */
/*                                                                          */
/*      Copyright (C) 1999-2002 Emmanuel Bacry                              */
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
/*  ext.c         Functions which deal with the ext package           */
/*                                                                          */
/****************************************************************************/



#include "lastwave.h"
#include "extrema1d.h"



/***********************************
 * 
 *   CProcs related to extrema
 *
 ***********************************/

/* In ext_alloc.c */
extern void C_ECopy(),C_SetExtrep(),C_Extrep(),C_Ext();

/* In ext_compute.c */
extern void C_ComputeExtrep();

/* In ext_chain.c */
extern void C_Chain(),C_ChainDelete(),C_ChainMax();
extern void C_ExtlisToSig();

/* In ext_file.c */
extern void C_EWrite(),C_ERead();


static CProc extrema1dCommands[] = {

  /* 
   * CProcs in ext_alloc.c
   */

  "setextrep",C_SetExtrep,"{{{OLD LASTWAVE COMMAND} {NOT TO BE USED}}}",

  "extrep",C_Extrep,"{{{closest [<extrep>] <x> <y>} {Gets the extremum which is closest to <x> <y> (or returns null).}}}",
  "ext",C_Ext,"{{{remove <ext>} {Removes an extremum from its extrema representation.}} \
{{<removechain> <ext>} {Removes the extrema chain the extremum <ext> belongs to.}}}",
  "ecopy",C_ECopy,"{{{<extrepIn> <extrepOut>} {Copies an extrema representation into another.}}}",
    
  /* 
   * CProcs in ext_compute.c
   */
   "extrema",C_ComputeExtrep,"{{{[<wtrans>=objCur] [<extrep>=<wtrans>] [-icC] [-e <threshold>]} {Computes the extrema representation \
associated to the wavelet transform <wtrans> and puts it in variable <extrep>. It returns the number of extrema found. Let us note that \
this command leads to unstable results at large sacles (i.e., too many extrema). In order to make the computation stable, by default, \
the 'extrema' command chain the extrema (using the 'chain' command) and delete the extrema (using the 'chaindelete' command) which have not been chained. In most cases, it works \
perfectly. If you want to disable this behavior you should use the '-C' option. Options are : \n\
-i : The extremum positions are computed without using interpolation and thus always correspond to a sample of the original signal \n\
-c : Causal effects are taken into account (The extrema are computed only between the 'firstp' and 'lastp' indexes of each wavelet coefficient signal). \n\
-C : Disable computation of chains and deletion of extrema which do not belong to the chains. \n\
-e : Sets the <threshold> below which no maxima will be considered.}}}",

  /* 
   * CProcs in ext_chain.c
   */
  "chain",C_Chain,"{{{[<extrep>=objCur] [-d]} {Chains the extrema of the extrema representation <extrep> and deletes all the extrema which are not part of \
chains that reach the smallest scale (unless option '-d' is specified). It returns the number of chains found.}}}",
  "chaindelete",C_ChainDelete,"{{{[<extrep>=objCur]} {deletes all the extrema of the extrema representation <extrep> which are not part of \
chains that reach the smallest scale. It returns the number of chains found. This command is called (by default) by the 'chain' command.}}}",  
  "chainmax",C_ChainMax,"{{{[<extrep>=objCur] [<exponent>=0]} {Replaces the wavelet trasnform value of an extremum by the maximum value along its chain. This routine allows to perform the \
scale adaptative version of the WTMM method. The coefficients are first multiply by aMin*(2^(exponent*(o-1+v/nvoice))). This is useful in the case \
some maxima chains do not decay when going to small scales.}}}",
  "extlistosig",C_ExtlisToSig,"{{{<ext> <signal>} {Converts the extrema list pointed by <ext> into a <signal>}}}",


  /* 
   * CProcs in ext_file.c
   */
  "ewrite",C_EWrite,"{{{[<extrep>=objCur] (<file> | <stream>) [-h]} {Writes an extrema representation in a <file> or in a <stream> (with no header if '-h').}}}",
  "eread",C_ERead,"{{{[<extrep>=objCur] (<file> | <stream>)} {Reads an extrema representation from a <file> or a <stream>.}}}",

   NULL,NULL,NULL
};


static CProcTable  extTable = {extrema1dCommands, "extrema1d","Commands related to 1d wavelet transform extrema"};



/***********************************
 * 
 * Loading the extrema package
 *
 ***********************************/

int tEXT, tEXT_;
int tEXTREP, tEXTREP_;

static void LoadExtrema1dPackage(void)
{
  tEXT = AddVariableTypeValue(extType, &tsExt, NULL);
  tEXT_ = tEXT+1;

  tEXTREP = AddVariableTypeValue(extrepType, &tsExtrep, NULL);
  tEXTREP_ = tEXTREP+1;
  
  AddCProcTable(&extTable);
  
  DefineGraphExtrep();
}


void DeclareExtrema1dPackage(void)
{
  DeclarePackage("extrema1d",LoadExtrema1dPackage,1999,"2.0","B.Audit, E.Bacry, J.F.Muzy and C.Vaillant",
             "Package allowing to deal with 1d wavelet transform local extrema.");
}
