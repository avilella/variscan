/*..........................................................................*/
/*                                                                          */
/*  L a s t W a v e    P a c k a g e 'wtmm1d' 2.0                           */
/*                                                                          */
/*      Copyright (C) 1998-2002 Benjamin Audit.                             */
/*      email : audit@ebi.ac.uk                                             */
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

#include "wtmm1d.h"


extern void C_Pf(char **argv);
extern void C_SetPf(char **argv);

static CProc partitionFunctionCommands[] = {
  "pf",C_Pf,
  "{{do1Scale <pf> <signal> [<oct> <voi>] [-c]} {Computes the partition "
  "function for"
  " the ((oct-1)*nVoice+voi)th scale. It has to be the next uncomputed scale. "
  "The default computes the right scale.}} "

  "{{read [<pf>] [<filename> | <stream>=stdin]} {Reads a partition function "
  "from a file or a stream. (it can''t be a stream associated to string "
  "neither to the terminal). It returns <pf> or a new partition function "
  "if none were specified on the command line. "
  "The partition function may be in ascii or binary format. It knows how "
  "to deal with big and little endian.\n"
  "WARNING: When there is no last argument then stdin is used but it MUST"
  "have been redirected to a stream associated to a FILE *}} "

  "{{write <pf> [<filename>| <stream> = stdout]} "
  "{Writes the partition function "
  "in a file or a stream using an ascii format. "
  "(it can''t be a stream associated to string).\n"
  "WARNING: When there is no last argument then stdout is used but it MUST"
  "         have been redirected to a stream associated to a FILE *}} "

  "{{writebin <pf> <filename>} "
  "{Writes the partition function in a file using a binary format.}} "

  "{{copy <pfin> <pfout>} {Old Lastwave 1.7 command.\nNot to be used, "
  "Use the generic copy function instead.\n"
  "(Copies <pfin> into <pfout>.)}} "

  "{{wtmm [<pf>] <extrep> <q-listv|q-signal>} "
  "{Computes the partition functions using the extrema representation. "
  "It is the so-called WTMM method. It returns a new partition function "
  "if none were specified on the command line or <pf> itself.}} "

  "{{cont [<pf>] <wtrans> <q-listv|q-signal> [-c]} "
  "{Computes the continuous partition functions. "
  "It returns a new partition function if none were specified on the "
  "command line or <pf> itself.\n"
  "  -c : Causal effects are taken into account (The partition functions are "
  "computed only between the 'firstp' and 'lastp' indexes of each wavelet "
  "coefficient signal).}} "

  "{{get|getq|geti <type>=t|h|d|st|sh|sd <pf> <q>|<{q1 q2 ...} [-i]} {Gets the "
  "partition functions T(q), H(q), D(q), stddev[H(q)], stddev[T(q)] or stddev[D(q)] "
  "according to <type> and q-values and returns them as one <&signal> or "
  "a <&listv> of signals. (stddev[T|H|D(q)] are the "
  "standard deviation in intensive mode, the extensive mode is not available.)\n"
  "  -i : when <pf> is the result of the averaging of different partition "
  "functions it asks for the mean in intensive mode (it is the arithmetical "
  "mean of the partition functions). The default is to ask the mean in "
  "extensive mode (it is as if the partition functions had been computed on "
  "one very long signal).\n"
  " geti : the same as above but q values are rounded to the nearest integer and "
  "interpreted as q-indexes (remember that internally q values are ordered).}} " 

  "{{add <pf1> <pf2> [-n]} {Returns the sum of <pf1> and <pf2>. <pf1> "
  "and <pf2> have to be compatible for addition: they have to be the result "
  "of the same type of analysis (same WT on the same range of scale on the "
  "same number of signals of the same size). If the qlist are strickly "
  "different then it just appends the values of q, if the qlist are "
  "strickly equal it computes the means of the partiton functions else the "
  "pf''s are not compatible.\n"
  "  -n : non standard addition. It lets you add pf''s that would otherwise be"
  " incompatible. pf2 number of octaves may be smaller than pf1 number of "
  "octaves. pf''s may have been computed on signal of different size. "
  "(When you use this option the signalSize field becomes the total size of "
  "all the analysed signals).}} "

  "{{init <pf> <method> <aMin> <nOct> <nVoice> <signalSize> <qlist>} "
  "{Not documented.}} "

  "{{reset <pf>} {sets all the partition functions to 0, <pf> is ready for"
  " a new computation using do1Scale.}} ",

  "setpf",C_SetPf,
  "{{method <pf> [<method name>]} {Sets/Gets the type of wavelet transform "
  "that was performed. For now, the method is just referred to by the wavelet "
  "name. This is used in order to avoid adding two pf''s computed using "
  "different wavelets.}} "

  "{{amin <pf> [<amin>]} {Sets/Gets the smallest scale of the partition "
  "functions.}} "

  "{{noct <pf>} {Gets the number of octave of the partition functions.}} "

  "{{nvoice <pf>} {Gets the number of voice of the partition functions.}} "

  "{{nscale <pf>} {Gets the number of computed scales of the partition "
  "functions.}} "

  "{{sigsize <pf> [<signal size>]} {Sets/Gets the size of the original signals"
  " on which were computed the partition functions. "
  "(In the case you used the non standard addition, it is the sum of these "
  "fields. So, if the pf''s were "
  "the result of standard addition this field is not correct).}} "

  "{{signumber <pf>} {Gets the number of signals that were used to compute "
  "the partition functions.}} "

/*  "{{dim <pf> [<dim>]} {Sets/Gets the dimension of the signals used to compute"
  " the partition functions.}} " */

  "{{qnumber <pf>} {Gets the number of q on which were computed the partition "
  "functions.}} "

  "{{qlist <pf> [<qlist>]} {Sets/Gets the qList for which the partition "
  "functions have been computed.\n"
  "WARNING: when you set a new qList it erases all the previous results.}} ",
 
  NULL,NULL,NULL
};
static CProcTable partitionFunctionTable = 
{partitionFunctionCommands,"wtmm1d",
 "Commands to deal with the partition functions used in the Wavelet Transform "
 "Modulus Maxima method for signals"};


/***********************************
 * 
 * Loading/Adding the wtmm1d package
 *
 ***********************************/

int tPF,tPF_;
extern TypeStruct tsPartitionFunction;

static void LoadWtmm1dPackage(void)
{
  tPF = AddVariableTypeValue(partitionFunctionType,
			     &tsPartitionFunction,
			     NULL);
  tPF_ = tPF+1;

  AddCProcTable(&partitionFunctionTable);
}

void DeclareWtmm1dPackage(void)
{
  DeclarePackage("wtmm1d",LoadWtmm1dPackage,1998,"2.0","B.Audit",
		 "Package allowing to compute the different quantities "
		 "involved in the Wavelet Transform Modulus Maxima method for "
		 "singular signals");
}

