#ifndef READMAF_H
#define READMAF_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif


int 
mafNextStretch (struct vscanFile *vf, 
                struct configFile *cf,
                struct vscanPopulation *vpop);

void
mafInsertLeadingGap (struct vscanBlock *vb);

void
mafInsertVscanGapBlock (struct vscanBlock *vb, 
                        struct vscanBlock *vb_prev);

#endif
