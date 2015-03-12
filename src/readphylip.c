#include "readphylip.h"

#ifndef VARISCAN_H
#include "variscan.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

void 
phylipFirstStretch (struct vscanFile *vf, 
                    struct configFile *cf, 
                    struct vscanPopulation *vpop) 
{
	/* Read the first stretch of a FORMAT_PHYLIP file, which
         includes the names of the sequences SH: write the name of the
         sequences into the corresponding vscanIndividual*/
	struct lineFile *lf = vf->lf;
	char *line;
	char *cloneline;
	char *blankline;
        int lineSize;
        int lenLine=0;
        int ref_gaps;
        
	struct vscanBlock *vb;
	struct vscanSeq *vs;

        /*variables I need for writing the names*/
        struct dlNode *node;
        struct vscanIndividual *vindiv;


	/* point to the block and create a new vscaninfo */
        vb = createEmptyvscanBlock(vf->vscanBlockList);

	/* loop blank lines*/
	while (lenLine == 0) {
		lineFileNext(lf, &line, &lineSize);

                blankline = cloneString(line);
                eraseWhiteSpace(blankline);
                lenLine = strlen(blankline);
                freeMem(blankline);
	}

	for (node = vpop->vindivList->head; !dlEnd(node); node = node->next) {
		
                /*SH: The first 10 characters of the current line are cloned.
                  After that, possible whitespace is erased*/

                vindiv = node->val;
                vindiv->name = cloneStringZ(line,PHYLIP_LABEL_LEN);
                eraseWhiteSpace(vindiv->name);
                
                /* chop the first 10 characters, to avoid the name of the sequence */
		cloneline = cloneString(line+PHYLIP_LABEL_LEN);
		eraseWhiteSpace(cloneline);

		/* determine line length (only once)*/
		if (node == vpop->vindivList->head) {
			vb->len = strlen(cloneline);
		}
                
                /*if this is the reference sequence, get the length*/
                if (vindiv->id == cf->refSeq) {
                        ref_gaps = countChars(cloneline,'-');
                        vb->ref_len = vb->len - ref_gaps;
                }
                
		/* create new vscanseqs and fill with the nucleotides */
		vs = createOneSeqvscanSeq(cloneline, vb->vscanSeqList);
                freeMem(cloneline);
		lineFileNext(lf, &line, &lineSize);
	}
	/* determine the end of the sequence in the block until now */
	vb->start = 1;
        vb->end = vb->len;
        
        if (vb->ref_len == 0) {
                vb->ref_start = vb->ref_end = 0;
        }
        else {
                vb->ref_start = 1;
                vb->ref_end = vb->ref_start + vb->ref_len - 1;
        }

	vb->segmentType = VSCAN_ALI;
        vf->accum_filled = vb->end;
}

boolean
phylipNextStretch (struct vscanFile *vf, 
                   struct configFile *cf) 
{
	/* Read in next stretch. Return FALSE at EOF. Return 2 when
	   full for going to analyse. */
	struct lineFile *lf = vf->lf;
	char *line;
	char *cloneline;
        char *blankline;
	int lineSize;
	int lenLine = 0;

	int i;
        int ref_gaps;

	struct vscanBlock *vb;
	struct vscanBlock *vb_tail;
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

        vb_tail = vf->vscanBlockList->tail->val;
        vb = createEmptyvscanBlock(vf->vscanBlockList);

	/*read stretch */
	for (i=0; i<(vf->numSeq);i++) {
		cloneline = cloneString(line);
		eraseWhiteSpace(cloneline);
			
                if (i == 0) {
                        vb->len = strlen(cloneline);
                }

                if (i+1 == cf->refSeq) {
                        ref_gaps = countChars(cloneline,'-');
                        vb->ref_len = vb->len - ref_gaps;
                }
		
                vs = createOneSeqvscanSeq(cloneline, vb->vscanSeqList);
    
		lineFileNext(lf, &line, &lineSize);
                freeMem(cloneline);

	}

        vb->start = vb_tail->end + 1;
	vb->end = vb->start + vb->len - 1;

        if (vb->ref_len == 0) {
                vb->ref_start = vb->ref_end = vb_tail->ref_end;
        }
        else {
                vb->ref_start = vb_tail->ref_end + 1;
                vb->ref_end = vb->ref_start + vb->ref_len - 1;
        }

	vb->segmentType = VSCAN_ALI;
	
        vf->accum_filled += vb->len;

        if (vf->accum_filled >= VSCAN_RAMBUFFER_MAX) {
                return 2;
        }
        else {
                return 1;
        }
}

