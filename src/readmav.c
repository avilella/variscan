#include "readmav.h"

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif

/* See "doc/inputformats.txt" for information about this format */

int
mavNextStretch (struct vscanFile *vf, 
                /*struct configFile *cf,*/ 
                struct vscanPopulation *vpop)
{
        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;
        char *blankline;
        char *cloneline;
        char *words[256];
        int wordCount=0;

        char *indivName;

        int lenLine=0;

        struct vscanBlock *vb;
        struct vscanBlock *vb_prev;
        struct dlNode *vi_node;
        struct vscanIndividual *vindiv;
        struct dlNode *vs_node;
        struct vscanSeq *vs;

	/* skip all non-alignment lines */
        
	while (lenLine == 0) {
		if (!lineFileNext(lf, &line, &lineSize)) {
			return FALSE;
		}
                blankline = cloneString(line);
                eraseWhiteSpace(blankline);
                lenLine = strlen(blankline);
                freeMem(blankline);
	}
        
        /* read the alignment */
        
        cloneline = cloneString(line);

        /*create a new block to store the alignment*/
        vb = createEmptyvscanBlock(vf->vscanBlockList);

        /*create some empty seqs*/        
        createNumOfSeqsvscanSeq(vf->numSeq,vb->vscanSeqList);

	/* AVB parsing header of the stretch */
	while (startsWith("%",cloneline)) {
                wordCount = chopLine(cloneline,words);

                /* AVB Indiv line */
                if (startsWith("%+",cloneline)) {
                        indivName = cloneString(words[1]);

                        vi_node = vpop->vindivList->head;
                        vindiv = vi_node->val;
                        vs_node = vb->vscanSeqList->head;
                        vs = vs_node->val;

                        /*AVB checking if correct vscanSeq*/
                        while (!sameWord(vindiv->name,indivName)) {
                                vi_node = vi_node->next;
                                if (vi_node->next == NULL){
                                        errAbort("Unknown individual in line %d of %s: %s",vf->lf->lineIx,vf->fileName,indivName);
                                }
                                vindiv = vi_node->val;
                                
                                vs_node = vs_node->next;
                                vs = vs_node->val;
                        }
                        vs->IndivInStretch = TRUE;

                        /* We have to know that this IndivName is
                         * present in this stretch. What about adding
                         * a "IndivInStretch" property in the vseq
                         * node inside the vb. */
                        freeMem(indivName);
                /* First line in the header contains ref_start and ref_end */
                } else if (startsWith("%>",cloneline)) {
                        /* Start, end and len for the reference sequence */
                        vb->ref_start = atol(words[3]) + 1;
                        vb->ref_end = atol(words[4]);
                        vb->ref_len = vb->ref_end - vb->ref_start + 1;
                        vb->len = vb->ref_len;
                } else {
                        errAbort("Critical error in alignment file! Bad header line!");
                }

                freeMem(cloneline);

		lineFileNext(lf, &line, &lineSize);
		cloneline = cloneString(line);
        }

	/* Parsing stretch nucleotides */
	while (!startsWith("%",cloneline) && 0 != strlen(cloneline)) {
                /* This condition is to get rid of positions that don't
                 * map into the reference sequence */
                if (!startsWith(" ",cloneline) && !startsWith("-",cloneline)) {
                /* KLUDGE - Adding the string char by char may be
                 * awfully slow!! */
                        int numPresentInStretch = 0;
                        for (vs_node = vb->vscanSeqList->head; !dlEnd(vs_node); vs_node = vs_node->next) {
                                vs = vs_node->val;
                                /* TODO - Shall we check for cf->chosenSeqs here */

                                /* TODO - Check that the order in of
                                 * IndivNames in the config files
                                 * corresponds consistently to the
                                 * order in the input file in all
                                 * stretches */
                                if (vs->IndivInStretch == TRUE) {
                                        if (' ' != cloneline[numPresentInStretch]) {
                                                dyStringAppendC(vs->dySeq, cloneline[numPresentInStretch]);
                                        } else {
                                                dyStringAppendC(vs->dySeq, 'N');
                                        }
                                        numPresentInStretch++;
                                } else {
                                        dyStringAppendC(vs->dySeq, 'N');
                                }
                        }
                } else {
                        /* CHECK - This line starts with a space or a
                         * dash, so the ref sequence is not present
                         * here. This can change in subsequent
                         * releases. */
                }
                lineFileNext(lf, &line, &lineSize);
                cloneline = cloneString(line);
                
                vb->segmentType = VSCAN_ALI;
        }

        freeMem(cloneline);

        /*check if the reference sequence was there*/
        if (vb->ref_start == 0) {
                vb_prev = vb->node->prev->val;
                errAbort("Critical error if alignment file! Reference sequence not found in paragraph! Last valid reference start: %lu (Input file line: %lu)",vb_prev->ref_start, vf->lf->lineIx);
        }
        
        vb->segmentType = VSCAN_ALI;
        vf->accum_filled += vb->len;
        
        /*check if there is a GAP between the last vscanBlocks*/
        if (vb->id == 1) {
                if (vb->ref_start != 1) {
                        mavInsertLeadingGap(vb);
                }
        }
        else {
                vb_prev = vb->node->prev->val;
                if (vb->ref_start > (vb_prev->ref_end + 1)) {
                        mavInsertVscanGapBlock(vb, vb_prev);
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
mavInsertLeadingGap(struct vscanBlock *vb)
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
mavInsertVscanGapBlock (struct vscanBlock *vb,
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
