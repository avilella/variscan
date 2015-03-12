#ifndef READMGA_H
#define READMGA_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

int
mgaNextSegment (struct vscanFile *vf);

unsigned long
checkvscanBlockStart (struct vscanBlock *vb);

void
readMgaAli (struct vscanFile *vf, struct vscanBlock *vb);

void
annotateMgaGap (struct vscanBlock *vb, char *firstWord);

void
skipUnneededExas (struct vscanFile *vf);

#endif
