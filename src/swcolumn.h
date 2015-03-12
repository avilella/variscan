#ifndef SWCOLUMN_H
#define SWCOLUMN_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

unsigned long
getFirstColumnWindow(struct bdfBlock *bb, 
                     struct configFile *cf, 
                     struct vscanFile *vf, 
                     struct analysis *ana);
/*first window of the analysis*/

unsigned long
columnWindowLimits (struct configFile *cf, 
                       struct vscanFile *vf, 
                       struct analysis *ana, 
                       struct bdfBlock *bb);
/*determine the sliding windows*/

void
columnFinalWindow (struct configFile *cf, 
                      struct vscanFile *vf, 
                      struct analysis *ana, 
                      struct bdfBlock *bb);
/*final window of analysis*/                                           
#endif
