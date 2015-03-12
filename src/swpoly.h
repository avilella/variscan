#ifndef SWPOLY_H
#define SWPOLY_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

unsigned long
getFirstPolyWindow (struct bdfBlock *bb, 
                    struct configFile *cf, 
                    struct vscanFile *vf, 
                    struct analysis *ana);
/*first window*/

unsigned long
polyWindowLimits (struct configFile *cf, 
                  struct vscanFile *vf, 
                  struct analysis *ana, 
                  struct bdfBlock *bb);
/*sliding window process*/

unsigned long 
performPolyJump (struct vscanFile *vf, 
                 struct configFile *cf, 
                 struct analysis *ana);
/*jump to get the new start (and ref_start)*/

unsigned long
calculateEndAndMidPolySW (struct configFile *cf, 
                          struct vscanFile *vf, 
                          struct analysis *ana);
/*get the new end and mid (and ref_end, ref_mid)*/

void
polyFinalWindow (struct configFile *cf, 
                 struct vscanFile *vf, 
                 struct analysis *ana, 
                 struct bdfBlock *bb);
/*last window*/

void
calculateFinalMidPolySW (struct vscanFile *vf, 
                         struct analysis *ana);
/*last mid and ref_mid*/
#endif
