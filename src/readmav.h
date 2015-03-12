#ifndef READMAV_H
#define READMAV_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif


int 
mavNextStretch (struct vscanFile *vf, 
                /*struct configFile *cf,*/
                struct vscanPopulation *vpop);

void
mavInsertLeadingGap (struct vscanBlock *vb);

void
mavInsertVscanGapBlock (struct vscanBlock *vb,
                        struct vscanBlock *vb_prev);

#endif
