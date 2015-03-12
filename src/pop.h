/* Variscan - Nucleotide Variability for huge DNA sets */

#ifndef POP_H
#define POP_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

struct vscanPopulation *createEmptyPopulation ();
/*create a population for the input*/

void createNumberOfIndividuals (struct vscanPopulation *vpop, int numSeq);

struct vscanIndividual *createEmptyIndividual (struct dlList *vindivList);
/*create a new individual*/

void writeIndivNames (struct vscanPopulation *vpop);
/*write the standard Seq 1, Seq 2, etc. into the corresponding
 * vscanIndividuals*/

void popOutput (struct vscanPopulation *vpop);
/*output to check if things are working*/

void insertOutgroup (struct dlList *outgroupList, 
                     struct vscanOutgroup *vo);
/*SH: here we insert an outgroup into the the correct
  position of outgroupList*/

void createNewOutgroup (struct dlList *outgroupList, 
                        struct vscanIndividual *vindiv, 
                        struct vscanSeq *vs);
/*SH: create a new outgroup and assign the basic variables.
  move the node to the correct position inside outgroupList*/

void chooseSequences (struct vscanPopulation *vpop, 
                      struct vscanFile *vf);
/*SH: here we choose the individuals defined by SeqChoice in the
  config File.  Selected ingroup sequences are nodes inside
  chosenSeqList.  This is done for all vscanBlocks currently in
  memory*/


#endif /* POP_H */
