#include "swcolumn.h"
#ifndef WINDOW_H
#include "window.h"
#endif
#include <limits.h>

unsigned long
getFirstColumnWindow(struct bdfBlock *bb, 
                     struct configFile *cf, 
                     struct vscanFile *vf, 
                     struct analysis *ana)
{
        unsigned long newEndSW;

        ana->varSW->start = bb->start;
        ana->varSW->ref_start = bb->ref_start;
        
        getFirstDiscarded(ana->varSW->start, vf);
        getFirstRefGap(ana->varSW->start, vf);

        newEndSW = ana->varSW->start + cf->widthSW - 1;
 
        /*SH: if the window end gets bigger than bb->end, then the first window
          is also the last window*/
        if (newEndSW >= bb->end && bb->end <= vf->lastPosInMem) {
                ana->varSW->end = bb->end;
                ana->varSW->ref_end = bb->ref_end;
                /*calculate the last midpoint*/
                ana->varSW->mid = ana->varSW->start + 
                                (ana->varSW->end - ana->varSW->start)/2;
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
        ana->varSW->mid = ana->varSW->start + (cf->widthSW / 2);
        getNetNumSites(vf, ana);

        calculateRefEndAndMidSW(vf,ana);

        return newEndSW;

}


unsigned long
columnWindowLimits (struct configFile *cf, 
                       struct vscanFile *vf, 
                       struct analysis *ana, 
                       struct bdfBlock *bb) 
/* Determine start and end for the current window (of the sliding
 * window analyses). Modify start and end according to the options
 * given in configFile (widthSW, jumpSW, booltypeSW) */
{
         /*SH: here we perform the window jump. If the window gets to large
         we return ULONG_MAX*/
        
        unsigned long newStartSW;
        unsigned long newEndSW;
        int discBackup = vf->vpa->tempDisc;
        int polyBackup = vf->vpa->tempPoly;

        /*jump upstream*/
        newStartSW = ana->varSW->start + cf->jumpSW;

        /*SH: if we jumped out of range, abort calculations and retry 
        after reading more*/
        if (newStartSW > vf->lastPosInMem) {
           ana->recalcSW = TRUE;
           return ULONG_MAX;
        }
        
        
        newEndSW = newStartSW + cf->widthSW - 1;
        /*set the midpoint*/
        ana->varSW->mid = newStartSW + (cf->widthSW / 2);
        
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

          /*get the net sites in our window*/
          getFirstDiscarded(ana->varSW->start,vf);
          getNetNumSites(vf,ana);
       
        return ana->varSW->end;
}


void
columnFinalWindow (struct configFile *cf, 
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

        
        lastStartSW = ana->varSW->start + cf->jumpSW;
        
        /*SH: if we jumped out of range, end calculations, no more 
          informative sites*/
          if (lastStartSW > vf->lastPosInMem) {
             /*SH: nothing left to do*/
             ana->windowsFinished = TRUE;
             return;
          }
          
          calculateRefStartSW(vf,ana,lastStartSW);
          ana->varSW->start = lastStartSW;
        
        if (bb->end < vf->lastPosInMem) {
                /*the end of the bdf block is in memory*/
                ana->varSW->end = bb->end;
                ana->varSW->ref_end = bb->ref_end;
        }
        else {
                /*the end of the bdf is not in memory, so it is truncated.
                The last position is the one we have in memory*/
                ana->varSW->end = vf->lastPosInMem;
                ana->varSW->ref_end = vf->lastRefPosInMem;
        }
        
        getFirstDiscarded (ana->varSW->start,vf);
        getNetNumSites(vf,ana);
        
        /*calculate the last midpoint*/
        ana->varSW->mid = ana->varSW->start + 
                        (ana->varSW->end - ana->varSW->start)/2;
        /*and the ref_mid*/
        calculateRefMidSW (vf,ana);
}
