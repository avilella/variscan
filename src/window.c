#ifndef VARISCAN_H
#include "variscan.h"
#endif
#ifndef SWCOLUMN_H
#include "swcolumn.h"
#endif
#ifndef SWNET_H
#include "swnet.h"
#endif
#ifndef SWPOLY_H
#include "swpoly.h"
#endif
#ifndef SWREF_H
#include "swref.h"
#endif
#ifndef DLIST_H
#include "dlist.h"
#endif
#ifndef WINDOW_H
#include "window.h"
#endif
#ifndef STATISTICS_H
#include "statistics.h"
#endif
#ifndef FREE_H
#include "free.h"
#endif
#ifndef OUTPUT_H
#include "output.h"
#endif
#include <limits.h>

void
calculateSlidingWindow (struct bdfBlock *bb, 
                        struct configFile *cf, 
                        struct vscanFile *vf, 
                        struct analysis *ana)
{
        /*Here we calculate the the parameters of windows we use in
          a sliding window analysis. Start and end positions in the 
          alignment an reference sequence are calculated, as well as
          the respective midpoints and the number of net sites in the
          window. Also the function for calculating statistics is
          called*/

	unsigned long windowEnd=0;


        /*SH: for the very first window*/
        if (ana->varSW->start == 0 || ana->varSW->end == 0) {
                windowEnd = getFirstWindow(bb, cf, vf, ana);
        }
        
        /*SH: if the window fell out of range during the last calculation,
         calculate it now*/
        if (ana->recalcSW) {
                ana->recalcSW = FALSE;
                getFirstDiscarded(ana->varSW->start, vf);
                if (cf->windowType != 2) {
                        /*windowType 2 doesn't need refGaps for
                          calculations, but*/
                        getFirstRefGap(ana->varSW->start, vf);
                }
                else {
                        /*it needs polys*/
                        getFirstPolymorphism(ana->varSW->start, vf);
                }
                windowEnd = determineWindowLimits(cf, vf, ana, bb);
        }

        /*SH: as long as the window fits into the polyArray, do the calculations*/
        while (windowEnd < bb->end && windowEnd < vf->lastPosInMem) {
                gatherVariables(cf,vf,ana->varSW,ana,ana->varSW->numSites);
                calculateStatistics(ana->varSW, cf, ana);
                printWindowResults(ana->varSW,cf);
                resetAnalysisVariables(ana->varSW);
                windowEnd = determineWindowLimits(cf, vf, ana, bb);
        }
        
        /*SH: if we have reached the defined end for analysis,
          or the end of the file calculate the final window*/
        if (bb->end <= vf->lastPosInMem || !vf->rambuffer_state) {
                if (windowEnd != bb->end && windowEnd != vf->lastPosInMem) {
                        determineFinalWindow(cf, vf, ana, bb);
                }
                /*before calculating the final window, make sure we didn't
                  already do that (by chance) before*/
                if (!ana->windowsFinished) {
                        gatherVariables(cf,vf,ana->varSW,ana,ana->varSW->numSites);
                        calculateStatistics(ana->varSW, cf, ana);
                        printWindowResults(ana->varSW,cf);
                        ana->windowsFinished = TRUE;
                }
        }
        
}

unsigned long
getFirstWindow (struct bdfBlock *bb, 
                struct configFile *cf, 
                struct vscanFile *vf, 
                struct analysis *ana)
{
        /*SH: determine the first window*/
        unsigned long newEndSW = 0;
        
        if (cf->windowType == 0) {
                /* the window is based on columns*/
                newEndSW = getFirstColumnWindow(bb,cf,vf,ana);
        }

        else if (cf->windowType == 1) {
                /* window is based on net sites*/
                newEndSW = getFirstNetWindow(bb,cf,vf,ana);
        }

        else if (cf->windowType == 2) {
                /*window is based on polymorphic positions*/
                newEndSW = getFirstPolyWindow(bb,cf,vf,ana);
        }

        else if (cf->windowType == 3) {
                /*window is based on polymorphic positions*/
                newEndSW = getFirstRefWindow(bb,cf,vf,ana);
        }

        else {
                vfError(vf, "Unknown windowType in getFirstWindow()!");
        }
       
        return newEndSW;
}


void
calculateRefEndAndMidSW (struct vscanFile *vf,struct analysis *ana)
{
        int i;
        /*this function calculates the reference end and midpoint,
        given the ref_start and the start and end in the alignment
        Function used by runModes 0 and 1*/

        ana->varSW->ref_end = 
                ana->varSW->ref_start + (ana->varSW->end - ana->varSW->start);
        ana->varSW->ref_mid = 
                ana->varSW->ref_start + (ana->varSW->mid - ana->varSW->start);

        for (i=vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].position <= ana->varSW->end
                     && vf->vpa->listRefGap[i].position >= ana->varSW->start
                     && i < vf->vpa->filledRefGap;
             i++) {
                ana->varSW->ref_end--;
                if (vf->vpa->listRefGap[i].position <= ana->varSW->mid) {
                        ana->varSW->ref_mid--;
                }
        }

        if (ana->varSW->ref_end <= ana->varSW->ref_start ||
            ana->varSW->ref_end == ULONG_MAX ) {
                /*ULONG_MAX is actually -1 in this case, since we are using
                  unsigned numbers*/
                ana->varSW->ref_end = ana->varSW->ref_start;
                ana->varSW->ref_mid = ana->varSW->ref_start;
        }
}

void
calculateRefMidSW (struct vscanFile *vf,struct analysis *ana)
{
/*gets the ref_mid. Function used by runModes 0 and 1*/
        int i;

        ana->varSW->ref_mid = 
                ana->varSW->ref_start + (ana->varSW->mid - ana->varSW->start);

        for (i=vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].position <= ana->varSW->mid
                     && vf->vpa->listRefGap[i].position >= ana->varSW->start
                     && i < vf->vpa->filledRefGap;
             i++) {
                ana->varSW->ref_mid--;
        }

        if (ana->varSW->ref_mid < ana->varSW->ref_start ||
            ana->varSW->ref_mid == ULONG_MAX ) {
                /*ULONG_MAX is actually -1 in this case, since we are using
                  unsigned numbers*/
                ana->varSW->ref_mid = ana->varSW->ref_start;
        }
}

void
calculateRefStartSW (struct vscanFile *vf, 
                     struct analysis *ana, 
                     unsigned long newStartSW)
{
/*gets the ref_start. Used by ruModes 0 and 1*/
        int i;

        /*SH: ana->varSW->start is still the start position BEFORE the jump!*/
        
        ana->varSW->ref_start += (newStartSW - ana->varSW->start);

        for (i=vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].position <= newStartSW 
                     && vf->vpa->listRefGap[i].position >= ana->varSW->start 
                     && i < vf->vpa->filledRefGap;
             i++) {
                ana->varSW->ref_start--;
                /*automatically push tempRefGap upstream to the new start*/
                vf->vpa->tempRefGap++;
        }
}     
                     

unsigned long
determineWindowLimits (struct configFile *cf, 
                       struct vscanFile *vf, 
                       struct analysis *ana, 
                       struct bdfBlock *bb) 
{
/*Sliding window process: the appropriate function for the chosen runMode
is called. The end (in the alignment) is returned.*/      
        unsigned long newEndSW = 0;

        if (cf->windowType == 0) {
                /* the window is based on columns*/
                newEndSW = columnWindowLimits(cf,vf,ana,bb);
        }

        else if (cf->windowType == 1) {
                /* window is based on net sites*/
                newEndSW = netWindowLimits(cf,vf,ana,bb);
        }

        else if (cf->windowType == 2) {
                /*window is based on polymorphic positions*/
                newEndSW = polyWindowLimits(cf,vf,ana,bb);
        }

        else if (cf->windowType == 3) {
                /*window is based on polymorphic positions*/
                newEndSW = refWindowLimits(cf,vf,ana,bb);
       }

        else {
                vfError(vf, "Unknown windowType in determineWindowLimits()!");
        }
        
        return newEndSW;
}


void
getNetNumSites (struct vscanFile *vf, struct analysis *ana)
{
        int i;

        /*calculate the width of the window*/
        ana->varSW->numSites = ana->varSW->end - ana->varSW->start + 1;
        
        /*SH: for each discarded site inside the window, the effective width 
          is reduced by 1*/
        for (i=vf->vpa->tempDisc; 
             vf->vpa->listDisc[i].position <= ana->varSW->end 
                     && vf->vpa->listDisc[i].position >= ana->varSW->start 
                     && i < vf->vpa->filledDisc; 
             i++) {
                ana->varSW->numSites--;
        }
}


void 
getFirstRefGap (unsigned long position, struct vscanFile *vf)
{
        /*SH: get the index of the first gap in the reference sequence
          on or after the position*/
        int i;

        for (i=vf->vpa->tempRefGap; 
             vf->vpa->listRefGap[i].position < position && i < vf->vpa->filledRefGap - 1; 
             i++) {
                vf->vpa->tempRefGap++;
        }
}        



void 
getFirstDiscarded (unsigned long position, struct vscanFile *vf)
{
        /*SH: get the index of the first dicarded site on or after the position*/
/*         int firstDiscarded=0; */
        int i;

        for (i=vf->vpa->tempDisc; 
             vf->vpa->listDisc[i].position < position && i < vf->vpa->filledDisc - 1; 
             i++) {
                vf->vpa->tempDisc++;
        }
}        

void 
getFirstPolymorphism (unsigned long start, struct vscanFile *vf)
{
        /*SH: get the index of the first polymorphic site on or after the position*/
        int i;

        for (i=vf->vpa->tempPoly; (vf->vpa->vps[i].position) < start && i < vf->vpa->filled-1; i++) {
                vf->vpa->tempPoly++;
         }
}  

void
determineFinalWindow (struct configFile *cf, 
                      struct vscanFile *vf, 
                      struct analysis *ana, 
                      struct bdfBlock *bb)
{
        /*SH: the very last window of the analysis is usually smaller than 
          the rest. here we call the function defined by the runMode*/

        if (cf->windowType == 0) {
                /* the window is based on columns*/
                columnFinalWindow(cf,vf,ana,bb);
        }

        else if (cf->windowType == 1) {
                /* window is based on net sites*/
                netFinalWindow(cf,vf,ana,bb);
        }

        else if (cf->windowType == 2) {
                /*window is based on polymorphic positions*/
                polyFinalWindow(cf,vf,ana,bb);
        }

        else if (cf->windowType == 3) {
                /*window is based on polymorphic positions*/
                 refFinalWindow(cf,vf,ana,bb);
       }

        else {
                vfError(vf,"Unknown windowType in determineFinalWindow()!");
        }
            
}

long
getBdfNumSites(struct vscanFile *vf, struct variables *var)
{
        /*SH: calculates the number of sites for the BDF block
          we are currently analysing, quit similar to getNetNumSites
          for windows, but I prefer to have this in a seperate function*/
        unsigned long sites;
        long i;

        sites = var->end - var->start + 1;
        
        getFirstDiscarded(var->start,vf);
        
        for (i=vf->vpa->tempDisc;
             vf->vpa->listDisc[i].position <= var->end && i < vf->vpa->filledDisc;
             i++) {
                sites--;
        }

        var->numSites += sites;
        return sites;
}


boolean
calculateBdfStretch(struct vscanFile *vf, 
                    struct configFile *cf, 
                    struct bdfBlock *bb, 
                    struct analysis *ana)
{
        /*SH: gets the boundries of the stretch wich is needed for 
          analysis of the current BDF block*/
        
        long stretchSites;
        
        /*SH: backup the temp variables of the polyarray*/
        int tempDiscBdf = vf->vpa->tempDisc;
        int tempPolyBdf = vf->vpa->tempPoly;
        int tempMonoGapBdf = vf->vpa->tempMonoGap;

        if (ana->varBdf->end == vf->lastPosInMem) {
                /*SH: we already hit the end of the alignment
                  with the last run, so don't gather any more
                  variables*/
                return FALSE;
        }

       
        if (ana->varBdf->start == 0){
                ana->varBdf->start = bb->start;
        }
        else {
                ana->varBdf->start = ana->varBdf->end + 1;
        }
        if (bb->end < vf->lastPosInMem){
                ana->varBdf->end = bb->end;
        }
        else {
                ana->varBdf->end = vf->lastPosInMem;
        }

        /*SH: collect the variables for the current stretch*/
        stretchSites = getBdfNumSites(vf, ana->varBdf);
        if (cf->runMode != 31) {
                gatherVariables(cf,vf,ana->varBdf,ana,stretchSites);
        }

        /*SH: reset the temp variables for the sliding window analysis*/
        vf->vpa->tempDisc = tempDiscBdf;
        vf->vpa->tempPoly = tempPolyBdf;
        vf->vpa->tempMonoGap = tempMonoGapBdf;

        if (bb->end < vf->lastPosInMem) return TRUE;
        else return FALSE;
        
}

void
calculateBdfBoundaries(struct bdfBlock *bb, 
                       struct vscanFile *vf, 
                       struct configFile *cf)
{
        struct vscanBlock *vb;
        
        if (cf->refPos) {
                /*SH: The BDF blocks are defined by reference sequence
                  coordinates, so we have to calculate the positions
                  in the alignment*/
                
                /*check, if ref_start is "0". If so, then the first position
                to analysis is the first position in memory*/
                
                if (bb->ref_start == 0) {
                    vb = vf->vscanBlockList->head->val;
                    
                    if (vb->ref_start == 0) {
                        bb->ref_start = 1;
                    }
                    else {
                        bb->ref_start = vb->ref_start;
                    }    
                    bb->start = vb->start;
                }    
                
                calculateBdfAlignmentBoundaries(bb,vf);
        }     
        else {
                /*SH: The alignment positions are given, so we have to 
                  get the reference positions*/
                  
                if (bb->start == 0) {
                    vb = vf->vscanBlockList->head->val;
                    bb->ref_start = vb->ref_start;
                    bb->start = vb->start;
                }    
                  
                  
                calculateBdfRefseqBoundaries(bb,vf);
        }
}
        

void
calculateBdfAlignmentBoundaries(struct bdfBlock *bb, struct vscanFile *vf)
{
        struct dlNode *vb_node;
        struct vscanBlock *vb;
        int i;
        int numGaps=0;
        long refGapBackup;
        
        vb_node = vf->vscanBlockList->head;
        vb = vb_node->val;
        
        if (bb->start == 0) {
                
                while (vb_node != vf->vscanBlockList->tail && vb->ref_end < bb->ref_start) {
                        vb_node = vb_node->next;
                        vb = vb_node->val;
                }
                
                getFirstRefGap(vb->start,vf);
                
                for (i=vf->vpa->tempRefGap; 
                     vf->vpa->listRefGap[i].ref_position < bb->ref_start &&
                             i < vf->vpa->filledRefGap;
                     i++) {
                        numGaps++;
                }
                
                bb->start = vb->start + (bb->ref_start - vb->ref_start) + numGaps;
                numGaps = 0;
        }

        if (bb->end == 0 || bb->end == ULONG_MAX) {

                if (vf->lastRefPosInMem < bb->ref_end) {
                        
                        if (!vf->rambuffer_state) {
                                bb->end = vf->lastPosInMem;
                                bb->ref_end = vf->lastRefPosInMem;
                        }
                        else {
                                bb->end = ULONG_MAX;
                        }
                }

                else {
                        
                        while (vb_node != vf->vscanBlockList->tail && vb->ref_end < bb->ref_end) {
                                vb_node = vb_node->next;
                                vb = vb_node->val;
                        }
                        
                   
                        refGapBackup = vf->vpa->tempRefGap;
                        getFirstRefGap(vb->start,vf);
                        
                        for (i = vf->vpa->tempRefGap; 
                             vf->vpa->listRefGap[i].ref_position < bb->ref_end &&
                                     i < vf->vpa->filledRefGap;
                             i++) {
                                numGaps++;
                        }
                        
                        bb->end = vb->start + (bb->ref_end - vb->ref_start) + numGaps;
                        vf->vpa->tempRefGap = refGapBackup;
                }
        }
}

void
calculateBdfRefseqBoundaries(struct bdfBlock *bb, struct vscanFile *vf)
{
        struct dlNode *vb_node;
        struct vscanBlock *vb;
        int i;
        int numGaps=0;
        long refGapBackup;

        vb_node = vf->vscanBlockList->head;
        vb = vb_node->val;

        if (bb->ref_start == 0) {
                
                while (vb_node != vf->vscanBlockList->tail && vb->end < bb->start) {
                        vb_node = vb_node->next;
                        vb = vb_node->val;
                }
                
                getFirstRefGap(vb->start,vf);
                
                for (i=vf->vpa->tempRefGap; 
                     vf->vpa->listRefGap[i].position < bb->start &&
                             i < vf->vpa->filledRefGap;
                     i++) {
                        numGaps++;
                }
                
                bb->ref_start = vb->ref_start + (bb->start - vb->start) - numGaps;
                numGaps = 0;

                if (vb->ref_len == 0) {
                        bb->ref_start++;
                }
        }
        
        if (bb->ref_end == 0 || bb->ref_end == ULONG_MAX) {

                if (vf->lastPosInMem < bb->end) {
                        
                        if (!vf->rambuffer_state) {
                                bb->ref_end = vf->lastRefPosInMem;
                                bb->end = vf->lastPosInMem;
                        }
                        else {
                                bb->ref_end = ULONG_MAX;
                        }
                }

                else {
                        
                        while (vb_node != vf->vscanBlockList->tail && vb->end < bb->end) {
                                vb_node = vb_node->next;
                                vb = vb_node->val;
                        }
                        
                   
                        refGapBackup = vf->vpa->tempRefGap;
                        getFirstRefGap(vb->start,vf);
                        
                        for (i = vf->vpa->tempRefGap; 
                             vf->vpa->listRefGap[i].position <= bb->end &&
                                     i < vf->vpa->filledRefGap;
                             i++) {
                                numGaps++;
                        }
                        
                        bb->ref_end = vb->ref_start + (bb->end - vb->start) - numGaps;
                        vf->vpa->tempRefGap = refGapBackup;

                        if (vb->ref_len == 0) {
                                bb->ref_end++;
                        }
                        
                        if (bb->ref_end < bb->ref_start) {
                                bb->ref_start = bb->ref_end;
                        }
                }
        }
}
        

void
checkBdfStart (struct vscanFile *vf, struct bdfBlock *bb)
{
        struct bdfBlock *last_bb;

        /*checks if the bdfBlock overlaps with the last bdfBlock
          and rewinds the tempVariables if necessary*/

        if (bb->id > 1) {
        
                last_bb = bb->node->prev->val;
        
                if (bb->start <= last_bb->end) {
                
                        if (vf->vpa->tempDisc > 0) {
                                rewindTempDisc (vf, bb);
                        }
                
                        if (vf->vpa->tempPoly > 0) {
                                rewindTempPoly (vf, bb);
                        }
                
                        if (vf->vpa->tempMonoGap > 0) {
                                rewindTempMonoGap (vf, bb);
                        }
                
                        if (vf->vpa->tempRefGap > 0) {
                                rewindTempRefGap (vf, bb);
                        }
                }
        }
}


void 
rewindTempDisc (struct vscanFile *vf, struct bdfBlock *bb)
{
        int i;
        for (i = vf->vpa->tempDisc;
             vf->vpa->listDisc[i].position >= bb->start &&
                     vf->vpa->tempDisc >= 0;
             i--) {
                vf->vpa->tempDisc = i;
        }
        
}

void 
rewindTempPoly (struct vscanFile *vf, struct bdfBlock *bb)
{
        int i;
        for (i = vf->vpa->tempPoly;
             vf->vpa->vps[i].position >= bb->start &&
                     vf->vpa->tempPoly >= 0;
             i--) {
                vf->vpa->tempPoly = i;
        }
        
}

void 
rewindTempMonoGap (struct vscanFile *vf, struct bdfBlock *bb)
{
        int i;
        for (i = vf->vpa->tempMonoGap;
             vf->vpa->listMonoGap[i].position >= bb->start &&
                     vf->vpa->tempMonoGap >= 0;
             i--) {
                vf->vpa->tempMonoGap = i;
        }

}

void 
rewindTempRefGap (struct vscanFile *vf, struct bdfBlock *bb)
{
        int i;
        for (i = vf->vpa->tempRefGap;
             vf->vpa->listRefGap[i].position >= bb->start &&
                     vf->vpa->tempRefGap >= 0;
             i--) {
                vf->vpa->tempRefGap = i;
        }

}


void
resetTempVariables (struct vscanFile *vf)
{
        vf->vpa->tempDisc = 0;
        vf->vpa->tempPoly = 0;
        vf->vpa->tempMonoGap = 0;
        vf->vpa->tempRefGap = 0;
}
