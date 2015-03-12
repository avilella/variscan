#ifndef WINDOW_H
#define WINDOW_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

/*SH: stuff needed for the sliding window analysis*/

void calculateSlidingWindow (struct bdfBlock *bb, 
                             struct configFile *cf, 
                             struct vscanFile *vf, 
                             struct analysis *ana);

unsigned long getFirstWindow (struct bdfBlock *bb,
                              struct configFile *cf,
                              struct vscanFile *vf, 
                              struct analysis *ana);

unsigned long determineWindowLimits (struct configFile *cf, 
                                     struct vscanFile *vf, 
                                     struct analysis *ana, 
                                     struct bdfBlock *bb);


void getFirstDiscarded (unsigned long position, 
                        struct vscanFile *vf);

void getFirstPolymorphism (unsigned long start,
                           struct vscanFile *vf);

void determineFinalWindow (struct configFile *cf, 
                           struct vscanFile *vf, 
                           struct analysis *ana, 
                           struct bdfBlock *bb);

void getFirstRefGap (unsigned long position, 
                     struct vscanFile *vf);

void calculateRefStartSW (struct vscanFile *vf, 
                          struct analysis *ana, 
                          unsigned long newStartSW);

void calculateRefEndAndMidSW (struct vscanFile *vf, 
                              struct analysis *ana);

void calculateRefMidSW (struct vscanFile *vf, 
                        struct analysis *ana);
                
void getNetNumSites (struct vscanFile *vf, 
                     struct analysis *ana);


/*SH: stuff needed for the BDF block analysis*/

long getBdfNumSites (struct vscanFile *vf, 
                     struct variables *var);

boolean calculateBdfStretch(struct vscanFile *vf, 
                            struct configFile *cf, 
                            struct bdfBlock *bb, 
                            struct analysis *ana);

void calculateBdfBoundaries(struct bdfBlock *bb, 
                            struct vscanFile *vf, 
                            struct configFile *cf);

void calculateBdfAlignmentBoundaries(struct bdfBlock *bb, 
                                     struct vscanFile *vf);

void calculateBdfRefseqBoundaries(struct bdfBlock *bb, 
                                  struct vscanFile *vf);

void checkBdfStart (struct vscanFile *vf, 
                    struct bdfBlock *bb);

void rewindTempDisc (struct vscanFile *vf, 
                     struct bdfBlock *bb);

void rewindTempPoly (struct vscanFile *vf, 
                     struct bdfBlock *bb);

void rewindTempMonoGap (struct vscanFile *vf, 
                        struct bdfBlock *bb);

void rewindTempRefGap (struct vscanFile *vf, 
                       struct bdfBlock *bb);

void resetTempVariables (struct vscanFile *vf);

#endif
