#include "readxmfa.h"

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif

#ifndef KXTOK_H
#include "kxTok.h"
#endif

/* static struct kxTok *tok; */

int
xmfaNextStretch (struct vscanFile *vf, 
                 struct configFile *cf,
                 struct vscanPopulation *vpop)
{

/*         Read next stretch of a xmfa file, which can vary in the
 *         number of elements from a minimum of 2 to the maximum of
 *         species/individuals involved in the whole file */

        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;
        char *loopline;
        char *cloneline;
        char *words[256];
        int wordCount=0;

        boolean loop = TRUE;
        
        unsigned long seqname = 0;

        struct vscanBlock *vb;
        struct vscanBlock *vb_prev;
        struct dlNode *vi_node;
        struct vscanIndividual *vindiv;
        struct dlNode *vs_node;
        struct vscanSeq *vs;

	/* skip all non-alignment lines: although the possibility of
         * adding lines of text before the first stretch is not
         * mentioned in the file format definition, the first stretch
         * will always start with a ">" character, so it's worth to
         * look for that */
        while (wordCount == 0 || loop) {
		if (!lineFileNext(lf, &line, &lineSize)) {
			return FALSE;
		}
                
                loopline = cloneString(line);
		wordCount = chopLine(loopline, words);
                
                if (startsWith(">",words[0])) {
                        loop = FALSE;
                }
                freeMem(loopline);
	}
        
        /* read the alignment */
        cloneline = cloneString(line);
        wordCount = chopLine(cloneline,words);

        /*create a new block to store the alignment*/
        vb = createEmptyvscanBlock(vf->vscanBlockList);
        
        /*create some empty seqs*/        
        createNumOfSeqsvscanSeq(vf->numSeq,vb->vscanSeqList);

        vi_node = vpop->vindivList->head;
        vindiv = vi_node->val;
        vs_node = vb->vscanSeqList->head;
        vs = vs_node->val;

        while ( !(startsWith("=",words[0])) ) {
                /* processing header */
                if (startsWith(">",words[0])) {
                        struct kxTok *tok;
                        tok = kxTokenize(words[0], FALSE);
                        /* $20 = ">" */
                        /* $21 = "1:3412560" */
                        /* $22 = "-" */
                        /* $23 = "3437757" */
                        /* $24 = "end" */

                        if (tok != NULL) {
                                char *ptr;
                                tok = tok->next;
                                if (strstr(tok->string,":")) {
                                        seqname = ulround(doubleExp(tok->string));
                                        if ((unsigned long)cf->refSeq == (unsigned long)seqname) {
                                                /* reference sequence, then
                                                 * look for ref_start and ref_end */
                                                ptr = strstr(tok->string,":");
                                                ptr++;
                                                vb->ref_start = atol(ptr);
                                                /* skip and get ref_end token */
                                                tok = tok->next;
                                                tok = tok->next;
                                                vb->ref_end = atol(tok->string);
                                                vb->ref_len = vb->ref_end - vb->ref_start + 1;
                                        }
                                }
                        } else {
                                vfError(vf,"Error parsing xmfa file");
                        }
                } else {
                        /* processing sequence line */
                        /*loop to the correct vscanSeq*/
                        while (seqname != ulround(doubleExp(vindiv->name))) {
                                vi_node = vi_node->next;
                                if (vi_node->next == NULL){
                                        errAbort("Unknown individual in line %d of %s: %s",vf->lf->lineIx,vf->fileName,seqname);
                                }
                                vindiv = vi_node->val;

                                vs_node = vs_node->next;
                                vs = vs_node->val;
                        }
                        dyStringAppend(vs->dySeq, line);
                        freeMem(cloneline);
                }
		if (!lineFileNext(lf, &line, &lineSize)) {
			return FALSE;
		}
		cloneline = cloneString(line);
                wordCount = chopLine(cloneline,words);
        }

        /*check if the reference sequence was there*/
        if (vb->ref_start == 0) {
                vb_prev = vb->node->prev->val;
                errAbort("Critical error if alignment file! Reference sequence not found in paragraph! Last valid reference start: %lu",vb_prev->ref_start);
        }

        /* TODO: check if this is correct */
        /* get the global alignment length (including any gaps)*/
        vb->len = vs->dySeq->stringSize;

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
                        xmfaInsertLeadingGap(vb);
                }
        }
        else {
                vb_prev = vb->node->prev->val;
                if (vb->ref_start > (vb_prev->ref_end + 1)) {
                        xmfaInsertVscanGapBlock(vb, vb_prev);
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
xmfaInsertLeadingGap(struct vscanBlock *vb)
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
xmfaInsertVscanGapBlock (struct vscanBlock *vb,
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
