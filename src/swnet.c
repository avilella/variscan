#include "swnet.h"
#ifndef WINDOW_H
#include "window.h"
#endif
#include <limits.h>

unsigned long
getFirstNetWindow (struct bdfBlock *bb, 
                   struct configFile *cf, 
                   struct vscanFile *vf, 
                   struct analysis *ana)
{
        unsigned long newEndSW;
        
        ana->varSW->start = bb->start;
        ana->varSW->ref_start = bb->ref_start;
        
        getFirstDiscarded(ana->varSW->start, vf);
        getFirstRefGap(ana->varSW->start, vf);

        newEndSW = calculateEndAndMidNetSW(ana->varSW->start,cf, vf, ana);
        
        /*SH: if the window end gets bigger than bb->end, then the first window
          is also the last window. If we have everything in memory, we are done*/
        if (newEndSW >= bb->end && bb->end <= vf->lastPosInMem) {
                ana->varSW->end = bb->end;
                ana->varSW->ref_end = bb->ref_end;
                calculateFinalMidNetSW (vf,ana);
                getNetNumSites(vf, ana);
                calculateRefMidSW(vf,ana);
                return ana->varSW->end;
        }

        /*SH: if the window end moves out of range, abort calculations
         * and retry after reading more*/

        if (newEndSW > vf->lastPosInMem) {
                ana->recalcSW = TRUE;
                return newEndSW;
        }
                
        ana->varSW->end = newEndSW;
        ana->varSW->numSites = cf->widthSW;
        calculateRefEndAndMidSW(vf,ana);
 
        return ana->varSW->end;
}

unsigned long
netWindowLimits (struct configFile *cf, 
                       struct vscanFile *vf, 
                       struct analysis *ana, 
                       struct bdfBlock *bb) 
/* Determine start and end for the current window (of the sliding
 * window analyses). Modify start and end according to the options
 * given in configFile (widthSW, jumpSW, booltypeSW) */
{
        /*SH: here we perform the window jump. We check if there are
          any discarded sites inbetween the old and the new start
          site. If so, the jump is extended accordingly*/
        
        unsigned long newStartSW;
        unsigned long newEndSW;
        int discBackup = vf->vpa->tempDisc;
        int polyBackup = vf->vpa->tempPoly;

        newStartSW = performNetJump(vf,cf,ana);

        /*SH: if we jumped out of range, abort calculations and retry 
          after reading more*/
                if (newStartSW > vf->lastPosInMem) {
                        ana->recalcSW = TRUE;
                        return ULONG_MAX;
                }
        newEndSW = calculateEndAndMidNetSW(newStartSW,cf, vf, ana);
        
        /*SH: if the window grows out of range, abort calculations 
          and retry after reading more*/
                if (newEndSW > vf->lastPosInMem) {
                        ana->recalcSW = TRUE;
                        return newEndSW;
                }
                
                if (newEndSW > bb->end){
                        /*The window will be calculated in 
                          determineFinalWindow*/
                        vf->vpa->tempDisc = discBackup;
                        vf->vpa->tempPoly = polyBackup;
                        return newEndSW;
                }
        
        /*SH: start and end have been calculated, now calculate 
          ref_start and ref_end and ref_mid*/

                calculateRefStartSW(vf,ana, newStartSW);
                
                ana->varSW->start = newStartSW;        
                ana->varSW->end = newEndSW;

                calculateRefEndAndMidSW(vf,ana);

                /*numSites is the defined width*/
                ana->varSW->numSites = cf->widthSW;
       
        return ana->varSW->end;
}

unsigned long 
performNetJump (struct vscanFile *vf, struct configFile *cf, struct analysis *ana)
{
/*get the new start after the jump*/
        unsigned long newStartSW;
        int i;

        newStartSW = ana->varSW->start + cf->jumpSW;
       
        /*SH: for each discarded site inbetween the start points, the jump is extended by 1*/
        for (i=vf->vpa->tempDisc; 
             vf->vpa->listDisc[i].position <= newStartSW 
                     && vf->vpa->listDisc[i].position >= ana->varSW->start 
                     && i < vf->vpa->filledDisc; 
             i++) {
                newStartSW++;
                /*automatically push up the tempDisc variable, so it is inside 
                  the new window after finishing the jump*/
                vf->vpa->tempDisc++;
        }

        return newStartSW;
}

unsigned long
calculateEndAndMidNetSW (unsigned long newStartSW, 
                         struct configFile *cf, 
                         struct vscanFile *vf, 
                         struct analysis *ana)
{
        /*SH: here we check if there are any discarded sites inside the window,
          because the window size is based on number of informative sites,
          not total sites. The end of the window is pushed upstream 
          accordingly. The midpoint is also calculated in this function*/
        
        unsigned long newEndSW;
/*         int firstDiscarded=0; */
        int i;

        newEndSW = newStartSW + cf->widthSW - 1;
        ana->varSW->mid = newStartSW + (cf->widthSW / 2);

        /*SH: for each discarded site inbetween the start and end
         * points, the end is pushed upstream*/

        for (i=vf->vpa->tempDisc; 
             vf->vpa->listDisc[i].position <= newEndSW 
                     && vf->vpa->listDisc[i].position >= ana->varSW->start 
                     && i < vf->vpa->filledDisc; 
             i++) {
                newEndSW++;
                /*the midpoint is also pushed up. If this window is not valid,
                  the midpoint will be recalculated in the next run*/
                if (vf->vpa->listDisc[i].position <= ana->varSW->mid) {
                        ana->varSW->mid++;
                }
        }

        return newEndSW;
}


void
netFinalWindow (struct configFile *cf, 
                      struct vscanFile *vf, 
                      struct analysis *ana, 
                      struct bdfBlock *bb)
{
        /*SH: the very last window of the analysis is usually smaller than 
          the rest. here we calculate the effective size of the last window 
          and send it to analysis*/
        
        unsigned long lastStartSW;
        
        if (ana->varSW->end == vf->lastPosInMem && !vf->rambuffer_state) {
                /*we hit the end of the file with the last regular window!
                 so we don't need to do any more calculations*/
                ana->windowsFinished = TRUE;
                return;
        }

        
        lastStartSW = performNetJump(vf,cf,ana);
        
        /*SH: if we jumped out of range, end calculations, no more 
          informative sites*/
                if (lastStartSW > vf->lastPosInMem) {
                        /*SH: nothing left to do*/
                        ana->windowsFinished = TRUE;
                        return;
                }
      
                calculateRefStartSW(vf,ana, lastStartSW);
                ana->varSW->start = lastStartSW;
        
        
        if (bb->end < vf->lastPosInMem) {
                /*the end of the bdf block is in memory*/
                ana->varSW->end = bb->end;
                ana->varSW->ref_end = bb->ref_end;
        }
        else {
                /*the end of the bdf is not in memory, so it is truncated*/
                ana->varSW->end = vf->lastPosInMem;
                ana->varSW->ref_end = vf->lastRefPosInMem;
        }
        
        /*since the last window is smaller than the defined width,
        get the width now. The tempDisc variable is already in place*/
        getNetNumSites(vf,ana);
        
        /*calculate the last midpoint*/
        calculateFinalMidNetSW (vf,ana);

        /*and the ref_mid*/
        calculateRefMidSW (vf,ana);
                
}

void
calculateFinalMidNetSW (struct vscanFile *vf, struct analysis *ana)
{
        int i;
                
        /*SH: Determine the final midpoint*/
                ana->varSW->mid = ana->varSW->start + (ana->varSW->numSites / 2);

                for (i=vf->vpa->tempDisc; 
                     vf->vpa->listDisc[i].position <= ana->varSW->mid 
                             && vf->vpa->listDisc[i].position >= ana->varSW->start 
                             && i < vf->vpa->filledDisc; 
                     i++) {
                        ana->varSW->mid++;
                }

}
