#ifndef READXMFA_H
#define READXMFA_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif


int 
xmfaNextStretch (struct vscanFile *vf, 
                 struct configFile *cf, 
                 struct vscanPopulation *vpop);

void
xmfaInsertLeadingGap (struct vscanBlock *vb);

void
xmfaInsertVscanGapBlock (struct vscanBlock *vb,
                         struct vscanBlock *vb_prev);

#endif
