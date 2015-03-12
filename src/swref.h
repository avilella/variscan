#ifndef SWREF_H
#define SWREF_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

unsigned long 
getFirstRefWindow (struct bdfBlock *bb,
                   struct configFile *cf,
                   struct vscanFile *vf, 
                   struct analysis *ana);
/*first window*/

void
calculateAlignmentEndAndMidSW (struct vscanFile *vf,
                               struct analysis *ana);
/*gets end and mid in the alignment*/

void
calculateAlignmentStartSW (struct vscanFile *vf, 
                           struct analysis *ana, 
                           unsigned long newRefStartSW);
/*gets start in the alignment*/

unsigned long
refWindowLimits (struct configFile *cf, 
                 struct vscanFile *vf, 
                 struct analysis *ana, 
                 struct bdfBlock *bb);
/*sliding window process*/

unsigned long
calculateEndAndMidRefSW (unsigned long newRefStartSW, 
                         struct configFile *cf, 
                         /*struct vscanFile *vf,*/
                         struct analysis *ana);
/*ref_end and ref_mid of the new window*/

void
refFinalWindow (struct configFile *cf, 
                struct vscanFile *vf, 
                struct analysis *ana, 
                struct bdfBlock *bb);
/*final window*/

void
calculateAlignmentMidSW (struct vscanFile *vf,
                         struct analysis *ana);
/*mid in the alignment of the final window*/
#endif
