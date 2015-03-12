#include "readmaf.h"

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif


int
mafNextStretch (struct vscanFile *vf, 
                struct configFile *cf, 
                struct vscanPopulation *vpop)
{
        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;
        char *loopline;
        char *cloneline;
        char *words[256];
        int wordCount=0;

        boolean loop = TRUE;
        
        char *nameString;
        char *name[1];
        int nameCount;

        struct vscanBlock *vb;
        struct vscanBlock *vb_prev;
        struct dlNode *vi_node;
        struct vscanIndividual *vindiv;
        struct dlNode *vs_node;
        struct vscanSeq *vs;

	/* skip all non-alignment lines */
        
	
        while (wordCount == 0 || loop) {
		if (!lineFileNext(lf, &line, &lineSize)) {
			return FALSE;
		}
                
                loopline = cloneString(line);
		wordCount = chopLine(loopline, words);
                
                if (sameWord(words[0],"s")) {
                        loop = FALSE;
                }
                freeMem(loopline);
	}
        
        /* read the alignment */
        
        cloneline = cloneString(line);
        wordCount = chopLine(cloneline,words);

        /*create a new block to store the alignment*/
        vb = createEmptyvscanBlock(vf->vscanBlockList);
        /* get the global alignment length (including any gaps)*/
        vb->len = strlen(words[6]);
        

        /*create some empty seqs*/        
        createNumOfSeqsvscanSeq(vf->numSeq,vb->vscanSeqList);

	while (sameWord(words[0],"s")) {

                /* In all MAF files I have seen up to now, the src field
                 contains the name of the individual (or species) plus an 
                 extra feature (usually the chromosome or contig), devided
                 by a "."*/
                nameString = cloneString(words[1]);
                nameCount = chopByChar(nameString,'.',name,1);

                /* If the names are not in the "name.feature" format, 
                   assign the whole chunk as the name*/
                if (nameCount == 0) {
                        name[0] = words[1];
                }

                vi_node = vpop->vindivList->head;
                vindiv = vi_node->val;
                vs_node = vb->vscanSeqList->head;
                vs = vs_node->val;

                /*loop to the correct vscanSeq*/
                while (!sameWord(vindiv->name,name[0])) {
                        vi_node = vi_node->next;
                        if (vi_node->next == NULL){
                                errAbort("Unknown individual in line %d of %s: %s",vf->lf->lineIx,vf->fileName,name[0]);
                        }
                        vindiv = vi_node->val;

                        vs_node = vs_node->next;
                        vs = vs_node->val;
                }

                /*if this is the defined reference sequence*/ 
                if (vindiv->id == cf->refSeq) {
                        /*The start of the sequences are zero-based
                          numbers in MAF files! So increment the start
                          by one!*/
                        vb->ref_start = atol(words[2]) + 1;
                        vb->ref_len = atol(words[3]);
                        vb->ref_end = vb->ref_start + vb->ref_len - 1;
                }


                dyStringAppend(vs->dySeq, words[6]);
                freeMem(nameString);
                freeMem(cloneline);

		lineFileNext(lf, &line, &lineSize);
		cloneline = cloneString(line);
                wordCount = chopLine(cloneline,words);
        }

        freeMem(cloneline);

        /*check if the reference sequence was there*/
        if (vb->ref_start == 0) {
                vb_prev = vb->node->prev->val;
                errAbort("Critical error in alignment file! Reference sequence not found in paragraph! Last valid reference start: %lu",vb_prev->ref_start);
        }
        
        /* fill up the empty seqs with "N"s */
        
        for (vs_node = vb->vscanSeqList->head; !dlEnd(vs_node); vs_node = vs_node->next) {
                vs = vs_node->val;
                if (vs->dySeq->stringSize == 0) {
                        dyStringAppendMultiC(vs->dySeq,'N', vb->len);
                }
        }
        
        vb->segmentType = VSCAN_ALI;
        vf->accum_filled += vb->len;
        

        /*check if there is a GAP between the last vscanBlocks*/
        if (vb->id == 1) {
                if (vb->ref_start != 1) {
                        mafInsertLeadingGap(vb);
                }
        }
        else {
                vb_prev = vb->node->prev->val;
                if (vb->ref_start > (vb_prev->ref_end + 1)) {
                        mafInsertVscanGapBlock(vb, vb_prev);
                }    
                else if (vb->ref_start <= vb_prev->ref_end) {
                        errAbort("Overlapping blocks found. Run a filter first! Block %d - end %lu, block %d - start %lu",
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
mafInsertLeadingGap(struct vscanBlock *vb)
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
mafInsertVscanGapBlock (struct vscanBlock *vb, 
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
