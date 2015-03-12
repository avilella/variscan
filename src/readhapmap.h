#ifndef READHAPMAP_H
#define READHAPMAP_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif


int 
hapmapHeader (struct vscanFile *vf,
              struct vscanPopulation *vpop);

int 
hapmapNextIndividual (struct vscanFile *vf,
                      struct configFile *cf);

void 
writeHapmapNames (struct vscanPopulation *vpop, 
                  char *line, 
                  int count);

/* unsigned long  */
/* hapmapCountAlleles (struct vscanFile *vf); */


#endif
