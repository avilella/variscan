#ifndef OUTPUT_H
#define OUTPUT_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

/* void printBdfHead(struct bdfBlock *bb, struct configFile *cf); */
void printBdfHead (struct bdfBlock *bb, 
                   struct configFile *cf);

/* void printWindowResults (struct variables *var); */
void printWindowResults (struct variables *var, 
                         struct configFile *cf);

/* void printBdfBlockResults(struct bdfBlock *bb, struct variables *var); */
void printBdfBlockResults (struct bdfBlock *bb, 
                           struct variables *var, 
                           struct configFile *cf);

void dumpNextPara (FILE *dump, 
                   struct vscanFile *vf, 
                   struct vscanBlock *vb);

void dumpFirstPara (FILE *dump, 
                    struct vscanFile *vf, 
                    struct vscanPopulation *vpop, 
                    struct vscanBlock *vb);

void dumpPhylip (struct vscanFile *vf, 
                 struct vscanPopulation *vpop);

void printPolyArray (struct vscanFile *vf);

void printBlocks (struct vscanFile *vf);

void printPops (struct vscanPopulation *vpop);

#endif
