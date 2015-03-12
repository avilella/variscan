#ifndef READPHYLIP_H
#define READPHYLIP_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

void phylipFirstStretch (struct vscanFile *vf, struct configFile *cf, struct vscanPopulation *vpop);

boolean phylipNextStretch (struct vscanFile *vf, struct configFile *cf);

#endif
