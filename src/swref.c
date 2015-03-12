#include "swref.h"
#ifndef WINDOW_H
#include "window.h"
#endif
#include <limits.h>

unsigned long 
getFirstRefWindow (struct bdfBlock *bb,
                   struct configFile *cf,
                   struct vscanFile *vf, 
                   struct analysis *ana)
{
        unsigned long newRefEndSW;

        ana->varSW->start = bb->start;
        ana->varSW->ref_start = bb->ref_start;
        
        getFirstDiscarded(ana->varSW->start, vf);
        getFirstRefGap(ana->varSW->start, vf);

        newRefEndSW = ana->varSW->ref_start + cf->widthSW - 1;
 
        /*SH: if the window end gets bigger than bb->ref_end, 
          then the first window is also the last window*/
        if (newRefEndSW >= bb->ref_end && bb->ref_end <= vf->lastRefPosInMem) {
                ana->varSW->end = bb->end;
                ana->varSW->ref_end = bb->ref_end;
                ana->varSW->ref_mid = ana->varSW->ref_start + 
                        (ana->varSW->ref_end - ana->varSW->ref_start)/2;
                calculateAlignmentMidSW(vf,ana);
                getNetNumSites(vf, ana);
                return ana->varSW->end;
        }

        /*SH: if the window end moves out of range, 
          abort calculations and retry after reading more*/
        if (newRefEndSW > vf->lastRefPosInMem) {
                ana->recalcSW = TRUE;
                return ULONG_MAX;
        }

        ana->varSW->ref_end = newRefEndSW;
        ana->varSW->ref_mid = ana->varSW->ref_start + (cf->widthSW / 2);

        calculateAlignmentEndAndMidSW(vf,ana);
        getNetNumSites(vf, ana);

        /*returns the end in the ALIGNMENT*/
        return ana->varSW->end;
}

void
calculateAlignmentEndAndMidSW (struct vscanFile *vf,struct analysis *ana)
{
/*get end and mid, given start, ref_end, ref_mid and ref_start*/
        int i;

        ana->varSW->end = ana->varSW->start + 
                (ana->varSW->ref_end - ana->varSW->ref_start);
        ana->varSW->mid = ana->varSW->start + 
                (ana->varSW->ref_mid - ana->varSW->ref_start);
        

        for (i=vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].ref_position < ana->varSW->ref_end
                     && vf->vpa->listRefGap[i].ref_position >= ana->varSW->ref_start
                     && i < vf->vpa->filledRefGap;
             i++) {
                ana->varSW->end++;
                if (vf->vpa->listRefGap[i].ref_position < ana->varSW->ref_mid) {
                        ana->varSW->mid++;
                }
        }
}

void
calculateAlignmentStartSW (struct vscanFile *vf, 
                           struct analysis *ana, 
                           unsigned long newRefStartSW)
{
/*get new start given old and new ref_start*/
        int i;

        /*SH: ana->varSW->ref_start is still the ref_start 
          position BEFORE the jump!*/
        
        ana->varSW->start += (newRefStartSW - ana->varSW->ref_start);

        for (i=vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].ref_position < newRefStartSW 
                     && vf->vpa->listRefGap[i].ref_position >= ana->varSW->ref_start 
                     && i < vf->vpa->filledRefGap;
             i++) {
                ana->varSW->start++;
                /*automatically push tempRefGap upstream to the new start*/
                vf->vpa->tempRefGap++;
        }
}

unsigned long
refWindowLimits (struct configFile *cf, 
                 struct vscanFile *vf, 
                 struct analysis *ana, 
                 struct bdfBlock *bb) 
/* Determine start and end for the current window (of the sliding
 * window analyses). Modify start and end according to the options
 * given in configFile (widthSW, jumpSW, booltypeSW) */
{
        
        unsigned long newRefStartSW;
        unsigned long newRefEndSW;
        int discBackup = vf->vpa->tempDisc;
        int polyBackup = vf->vpa->tempPoly;

        newRefStartSW = ana->varSW->ref_start + cf->jumpSW;

        /*SH: if we jumped out of range, abort calculations and retry 
          after reading more*/
        if (newRefStartSW > vf->lastRefPosInMem) {
                ana->recalcSW = TRUE;
                return ULONG_MAX;
        }

        /*get ref_end and ref_mid*/
        newRefEndSW = calculateEndAndMidRefSW(newRefStartSW,cf, ana);
        
        /*SH: if the window grows out of range, abort calculations 
          and retry after reading more*/
        if (newRefEndSW > vf->lastRefPosInMem) {
                ana->recalcSW = TRUE;
                return ULONG_MAX;
        }
                
        if (newRefEndSW > bb->ref_end){
                /*The window will be calculated in 
                  determineFinalWindow*/
                vf->vpa->tempDisc = discBackup;
                vf->vpa->tempPoly = polyBackup;
                return ULONG_MAX;
        }

        
        /*SH: ref_start, ref_mid and ref_end have been calculated, now calculate 
          start in the alignment*/

        calculateAlignmentStartSW(vf,ana, newRefStartSW);
                
        ana->varSW->ref_start = newRefStartSW;        
        ana->varSW->ref_end = newRefEndSW;

        /*now calculate end and mid in the alignment*/
        calculateAlignmentEndAndMidSW(vf,ana);

        /*get numSites*/
        getFirstDiscarded(ana->varSW->start,vf);
        getNetNumSites(vf,ana);
      
        /*returns the end in the ALIGNMENT*/
        return ana->varSW->end;
}


unsigned long
calculateEndAndMidRefSW (unsigned long newRefStartSW, 
                         struct configFile *cf, 
                         /*struct vscanFile *vf,*/ 
                         struct analysis *ana)
{
        unsigned long newRefEndSW;

        /*These positions are ALL in the reference sequence!!!*/
        newRefEndSW = newRefStartSW + cf->widthSW - 1;
        ana->varSW->ref_mid = newRefStartSW + (cf->widthSW / 2);

        return newRefEndSW;
}

void
refFinalWindow (struct configFile *cf, 
                struct vscanFile *vf, 
                struct analysis *ana, 
                struct bdfBlock *bb)
{
        /*SH: the very last window of the analysis is usually smaller than 
          the rest. here we calculate the effective size of the last window 
          and send it to analysis*/
        
        unsigned long lastRefStartSW;
        
        if (ana->varSW->end == vf->lastPosInMem && !vf->rambuffer_state) {
                /*we hit the end of the file with the last regular window!
                  so we don't need to do any more calculations*/
                ana->windowsFinished = TRUE;
                return;
        }

        
        lastRefStartSW = ana->varSW->ref_start + cf->jumpSW;
        
        /*SH: if we jumped out of range, end calculations, no more 
          informative sites*/
        if (lastRefStartSW > vf->lastRefPosInMem) {
                /*SH: nothing left to do*/
                ana->windowsFinished = TRUE;
                return;
        }
        
        calculateAlignmentStartSW(vf,ana, lastRefStartSW);
        ana->varSW->ref_start = lastRefStartSW;
        
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
        
        /*get final numSites*/
        getFirstDiscarded (ana->varSW->start,vf);
        getNetNumSites(vf,ana);
        
        /*calculate the last ref_mid*/
        ana->varSW->ref_mid = ana->varSW->ref_start + 
                (ana->varSW->ref_end - ana->varSW->ref_start)/2;

        /*and the mid in the alignment*/
        calculateAlignmentMidSW (vf,ana);
                
}

void
calculateAlignmentMidSW (struct vscanFile *vf,struct analysis *ana)
{
/*calculate mid, given start, ref_mid and ref_start*/
        int i;

        ana->varSW->mid = ana->varSW->start + 
                (ana->varSW->ref_mid - ana->varSW->ref_start);

        for (i=vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].ref_position < ana->varSW->ref_mid
                     && vf->vpa->listRefGap[i].ref_position >= ana->varSW->ref_start
                     && i < vf->vpa->filledRefGap;
             i++) {
                ana->varSW->mid++;
        }
}
