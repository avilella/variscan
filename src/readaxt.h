#ifndef READAXT_H
#define READAXT_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif


int
axtNextStretch (struct vscanFile *vf);

void
axtInsertLeadingGap (struct vscanBlock *vb);

void
axtInsertVscanGapBlock (struct vscanBlock *vb, 
                        struct vscanBlock *vb_prev);


#endif
