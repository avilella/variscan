#ifndef FREE_H
#define FREE_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

void freeAnalysedPart(struct vscanFile *vf,
		      struct configFile *cf,
                      struct analysis *ana,
                      struct bdfBlock *bb);
/* Free the blocks that have been analysed */

int checkForFree(struct vscanBlock *vb_head, 
                 struct vscanFile *vf,
		 struct configFile *cf,
                 struct analysis *ana, 
                 struct bdfBlock *bb);
/*check if the current head block can be freed*/

void vscanFileFree(struct vscanFile **pVf);
/* Free vscan file. */

void configFileFree(struct configFile **pCf);
/* Free config file. */

void vscanPolyArrayFree(struct vscanPolyArray **pvpa);
/* SH: Free the complete vscanPolyArray*/

void vscanBlockFree(struct vscanBlock **pvb);
/* Free a single vscanBlock. */

void vscanBlockFreeList(struct dlList **pList);
/* Free a list of vscanBlocks. */

void vscanOutgroupFreeList(struct dlList **pList);
/* Free a list of vscanOutgroups. */

void vscanOutgroupFree(struct vscanOutgroup **pvo);
/* Free a single vscanOutgroup. */

void vscanSeqFree(struct vscanSeq **pvs);
/* Free a single vscanSeq. */

void vscanSeqFreeList(struct dlList **pList);
/* Free a list of vscanSeqs. */

void chosenSeqFreeList(struct dlList **pList);
/* Free the list of chosen Seqs, but not the values*/

void vscanPopFree(struct vscanPopulation **pvpop);

void vscanIndivListFree(struct dlList **pList);

void vscanIndividualFree(struct vscanIndividual **pvindiv);

void bdfBlockFreeList(struct dlList **pList);

void bdfBlockFree(struct bdfBlock **pbb);

void vscanAnalysisFree(struct analysis **pana, int num);

#endif
