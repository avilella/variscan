#ifndef SWNET_H
#define SWNET_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

unsigned long
getFirstNetWindow (struct bdfBlock *bb, 
                   struct configFile *cf, 
                   struct vscanFile *vf, 
                   struct analysis *ana);
/*first window*/

unsigned long
netWindowLimits (struct configFile *cf, 
                 struct vscanFile *vf, 
                 struct analysis *ana, 
                 struct bdfBlock *bb);
/*sliding process*/

unsigned long 
performNetJump (struct vscanFile *vf, 
                struct configFile *cf, 
                struct analysis *ana);
/*get the new start after the jump*/

unsigned long
calculateEndAndMidNetSW (unsigned long newStartSW, 
                         struct configFile *cf, 
                         struct vscanFile *vf, 
                         struct analysis *ana);
/*get end and midpoint after jump (return end)*/

void
netFinalWindow (struct configFile *cf, 
                struct vscanFile *vf, 
                struct analysis *ana, 
                struct bdfBlock *bb);
/*final window*/

void
calculateFinalMidNetSW (struct vscanFile *vf, 
                        struct analysis *ana);
/*final midpoint*/
#endif
