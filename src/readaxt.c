#include "readaxt.h"

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef READMAF_H
#include "readmaf.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif


int
axtNextStretch (struct vscanFile *vf)
{
        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;

        char *blankline;
        char *cloneline;
        char *words[256];
        int wordCount;

        int lenLine=0;

        struct vscanBlock *vb;
        struct vscanBlock *vb_prev;

        struct vscanSeq *vs;

	/* loop blank lines*/
	
	while (lenLine == 0) {
		if (!lineFileNext(lf, &line, &lineSize)) {
			return FALSE;
		}
                blankline = cloneString(line);
                eraseWhiteSpace(blankline);
                lenLine = strlen(blankline);
                freeMem(blankline);
	}

        cloneline = cloneString(line);
        wordCount = chopLine(cloneline,words);

        vb = createEmptyvscanBlock(vf->vscanBlockList);
        vb_prev = vb->node->prev->val;

        vb->ref_start = atol(words[2]);
        vb->ref_end = atol(words[3]);
        vb->ref_len = vb->ref_end - vb->ref_start + 1;

        freeMem(cloneline);

        lineFileNext(lf, &line, &lineSize);
        
        cloneline = cloneString(line);
        eraseWhiteSpace(cloneline);
                
        vb->len = strlen(cloneline);

        
        vs = createOneSeqvscanSeq(cloneline, vb->vscanSeqList);
        freeMem(cloneline);

        lineFileNext(lf, &line, &lineSize);
        
        cloneline = cloneString(line);
        eraseWhiteSpace(cloneline);

        vs = createOneSeqvscanSeq(cloneline, vb->vscanSeqList);
        freeMem(cloneline);

        vb->segmentType = VSCAN_ALI;
        vf->accum_filled += vb->len;

        /*check if there is a GAP between the last vscanBlocks
         this piece of code is copied 1:1 in readmaf.c*/
        if (vb->id == 1) {
                if (vb->ref_start != 1) {
                        axtInsertLeadingGap(vb);
                }
        }
        else {
                vb_prev = vb->node->prev->val;
                if (vb->ref_start > (vb_prev->ref_end + 1)) {
                        axtInsertVscanGapBlock(vb, vb_prev);
                }    
                else if (vb->ref_start <= vb_prev->ref_end) {
                        vfError(vf,"Overlapping blocks found. Run a filter first! Block %d - end %lu, block %d - start %lu",
                                vb_prev->id,
                                vb_prev->ref_end,
                                vb->id,
                                vb->ref_start);
                }    
        }
 
        /*calculate the alignment positions of the block*/

        if (vb->id == 1) {
                /*the block is still the first, so vb->ref_start
                  _should_ be 1*/
                vb->start = vb->ref_start;
        }

        else {
                /*vb_prev might be a new GAP block, so set the pointer again*/
                vb_prev = vb->node->prev->val;
                vb->start = vb_prev->end + 1;
        }

        vb->end = vb->start + vb->len - 1;
 
        if (vf->accum_filled >= VSCAN_RAMBUFFER_MAX) {
                return 2;
        }
        else {
                return 1;
        }

}

void
axtInsertLeadingGap(struct vscanBlock *vb)
{
        struct vscanBlock *vb_gap;

        AllocVar(vb_gap);

        vb_gap->segmentType = VSCAN_GAP;

        /*these will remain empty, just initialising for reference*/
	vb_gap->vscanSeqList = newDlList();
	vb_gap->chosenSeqList = newDlList();
	vb_gap->outgroupList = newDlList();

        /*assign positions*/
        vb_gap->ref_start = 1;
        vb_gap->ref_end = vb->ref_start - 1;
        vb_gap->ref_len = vb_gap->ref_end - vb_gap->ref_start + 1;

        vb_gap->start = vb_gap->ref_start;
        vb_gap->end = vb_gap->ref_end;
        vb_gap->len = vb_gap->ref_len;

        /*give the blocks the correct id*/
        vb_gap->id = vb->id;
        vb->id++;

        /*insert the GAP block*/
        vb_gap->node = dlAddValBefore(vb->node, vb_gap);
}

void
axtInsertVscanGapBlock (struct vscanBlock *vb,
                        struct vscanBlock *vb_prev)
{
        struct vscanBlock *vb_gap;

        AllocVar(vb_gap);

        vb_gap->segmentType = VSCAN_GAP;

        /*these will remain empty, just initialising for reference*/
	vb_gap->vscanSeqList = newDlList();
	vb_gap->chosenSeqList = newDlList();
	vb_gap->outgroupList = newDlList();

        vb_gap->ref_start = vb_prev->ref_end + 1;
        vb_gap->ref_end = vb->ref_start - 1;
        vb_gap->ref_len = vb_gap->ref_end - vb_gap->ref_start + 1;

        /*assign total alignment positions*/
        vb_gap->len = vb_gap->ref_len;
        vb_gap->start = vb_prev->end + 1;
        vb_gap->end = vb_gap->start + vb_gap->len - 1;

        /*give the blocks the correct id*/
        vb_gap->id = vb->id;
        vb->id++;
        
        /*insert the GAP block*/
        vb_gap->node = dlAddValBefore(vb->node, vb_gap);
        
}
