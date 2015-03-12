#include "swpoly.h"
#ifndef WINDOW_H
#include "window.h"
#endif
#include <limits.h>

unsigned long
getFirstPolyWindow (struct bdfBlock *bb, 
                    struct configFile *cf, 
                    struct vscanFile *vf, 
                    struct analysis *ana)
{

        unsigned long newEndSW;

        ana->varSW->start = bb->start;
        ana->varSW->ref_start = bb->ref_start;
        
        getFirstPolymorphism(ana->varSW->start,vf);
        getFirstDiscarded(ana->varSW->start, vf);

        newEndSW = calculateEndAndMidPolySW(cf,vf,ana);

        if (newEndSW == ULONG_MAX) {
                /*we don't have enough polymorphic positions stored
                  in the array*/

                if (bb->end <= vf->lastPosInMem) {
                        /*there are no more polymorphic positions
                          for this BDF block*/
                        ana->varSW->end = bb->end;
                        ana->varSW->ref_end = bb->ref_end;
                        calculateFinalMidPolySW (vf,ana);
                        getNetNumSites(vf, ana);
                        return ana->varSW->end;
                }
                else {
                        /*we have to read more*/
                        ana->recalcSW = TRUE;
                        return ULONG_MAX;
                }
        }

        if (newEndSW > bb->end) {
                /*first window is the last*/
                ana->varSW->end = bb->end;
                ana->varSW->ref_end = bb->ref_end;
                calculateFinalMidPolySW (vf,ana);
                getNetNumSites(vf, ana);
                return ana->varSW->end;
        }

        ana->varSW->end = newEndSW;
        getNetNumSites(vf, ana);

        return newEndSW;
}

unsigned long
polyWindowLimits (struct configFile *cf, 
                  struct vscanFile *vf, 
                  struct analysis *ana, 
                  struct bdfBlock *bb) 
/* Determine start and end for the current window (of the sliding
 * window analyses). Modify start and end according to the options
 * given in configFile (widthSW, jumpSW, booltypeSW) */
{
        unsigned long newStartSW;
        unsigned long newEndSW;
        int discBackup = vf->vpa->tempDisc;
        int polyBackup = vf->vpa->tempPoly;

        /*this also calculates ref_start*/
        newStartSW = performPolyJump(vf,cf,ana);

        /*SH: if we jumped out of range, abort calculations and retry 
          after reading more*/
        if (newStartSW > vf->lastPosInMem) {
                ana->recalcSW = TRUE;
                return ULONG_MAX;
        }
        
        /*here, also ref_mid and ref_end are calculated*/
        newEndSW = calculateEndAndMidPolySW(cf, vf, ana);
        
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
        

        /*everything looks good. assign start and end*/
        ana->varSW->start = newStartSW;        
        ana->varSW->end = newEndSW;
 
        /*get the numSites*/
        getFirstDiscarded(ana->varSW->start,vf);
        getNetNumSites(vf,ana);
       
        return ana->varSW->end;
}

unsigned long 
performPolyJump (struct vscanFile *vf, struct configFile *cf, struct analysis *ana)
{
        unsigned long newStartSW;
        int startIndex;

        startIndex = vf->vpa->tempPoly + cf->jumpSW;
        
        if (startIndex >= vf->vpa->filled) {
                return ULONG_MAX;
        }
 
        newStartSW = vf->vpa->vps[startIndex].position;
        ana->varSW->ref_start = vf->vpa->vps[startIndex].ref_position;
        
        /*set tempPoly to the first poly in the window, which is the
          start position (logically)*/
        vf->vpa->tempPoly = startIndex;

        return newStartSW;
}

unsigned long
calculateEndAndMidPolySW (struct configFile *cf, 
                          struct vscanFile *vf, 
                          struct analysis *ana)
{
        int newEndSW;
        int endIndex;
        int midIndex;


        endIndex = vf->vpa->tempPoly + cf->widthSW - 1;

        if (endIndex >= vf->vpa->filled) {
                /*not enough polys in array*/
                return ULONG_MAX;
        }
                
        midIndex = vf->vpa->tempPoly + (cf->widthSW / 2);

        newEndSW = vf->vpa->vps[endIndex].position;
        
        /*calculate the mid position and also the Ref_positions. If the
          window doesn't work out (is too large), they will simply be
          ignored later, when calculating the final window*/
        ana->varSW->mid = vf->vpa->vps[midIndex].position;
        ana->varSW->ref_mid = vf->vpa->vps[midIndex].ref_position;
        ana->varSW->ref_end = vf->vpa->vps[endIndex].ref_position;
        
        return newEndSW;
}

void
polyFinalWindow (struct configFile *cf, 
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

        /*gets start and ref_start*/
        lastStartSW = performPolyJump(vf,cf,ana);
        
        /*SH: if we jumped out of range, end calculations, no more 
          informative sites*/
        if (lastStartSW > vf->lastPosInMem) {
                /*SH: nothing left to do*/
                ana->windowsFinished = TRUE;
                return;
        }
        
     
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
        
        /*get last numSites*/
        getFirstDiscarded (ana->varSW->start,vf);
        getNetNumSites(vf,ana);
        
        /*calculate the last midpoint and ref_mid*/
        calculateFinalMidPolySW (vf,ana);
               
}

void
calculateFinalMidPolySW (struct vscanFile *vf, struct analysis *ana)
{
        int i;
        int lastIndex;
        int midIndex;
               
        /*SH: Determine the final midpoint and ref_mid*/
                
        lastIndex = vf->vpa->tempPoly;
        for (i = vf->vpa->tempPoly;
             vf->vpa->vps[i].position <= ana->varSW->end &&
                     i < vf->vpa->filled;
             i++) {
                lastIndex++;
        }

        midIndex = vf->vpa->tempPoly + ((lastIndex - vf->vpa->tempPoly) / 2);
        ana->varSW->mid = vf->vpa->vps[midIndex].position;
        ana->varSW->ref_mid = vf->vpa->vps[midIndex].ref_position;

}
