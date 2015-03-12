#include "free.h"

#ifndef LINEFILE_H
#include "linefile.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

#ifndef DLIST_H
#include "dlist.h"
#endif

void
freeAnalysedPart (struct vscanFile *vf,
		  struct configFile *cf,
                  struct analysis *ana, 
                  struct bdfBlock *bb)
{
	/* Free the blocks that have been analysed */
	struct dlNode *node_head;
        struct vscanBlock *vb_head;
       
        /*SH: free the polyarray. Thanks for the code, Albert.
         The rest was a piece of cake :P*/
        vscanPolyArrayFree(&vf->vpa);
                
        node_head = vf->vscanBlockList->head;
        vb_head = node_head->val;

       /*SH: as long as the head doesn't overlap with the current start position
          of the SW, or there is no SW, it can be freed*/

        while (checkForFree(vb_head,vf,cf,ana,bb)) { 
                if (vb_head->segmentType == VSCAN_ALI) {
                        vf->accum_filled -= vb_head->len;
                }
                
                vf->vscanBlockList->head = vf->vscanBlockList->head->next;
                vscanBlockFree(&vb_head);
                freeMem(node_head);
                
                /* New head after removing previous one */
                node_head = vf->vscanBlockList->head;
                vb_head = node_head->val;
	}
}

int
checkForFree(struct vscanBlock *vb_head, 
             struct vscanFile *vf,
	     struct configFile *cf,
             struct analysis *ana, 
             struct bdfBlock *bb)
{
        struct bdfBlock *bb_next;

        /*SH: check if we have reached the minimal length of 
          the vscanBlockList*/
        if (2 > fastBlockListCount(vf->vscanBlockList)) {
                return FALSE;
        }
        
        /*SH: if a SW analysis is in process, and the vb overlaps
          with the current window, stop the freeing*/
        if (ana->varSW->start != 0 && vb_head->end >= ana->varSW->start) {
                return FALSE;
        }

        /*SH: if the start of the next bdf block overlaps with
          the current vscanBlock, keep it in memory*/
        
        if (bb->node != vf->bdfBlockList->tail) {
                bb_next = bb->node->next->val;
                
                if (!cf->refPos) {
                        /*the reference positions are not yet calculated for
                          the next BDF block*/
                        if (vb_head->end >= bb_next->start) {
                                return FALSE;
                        }
                }
                else {
                        /*the alignment positions are not yet calculated for
                          the next BDF block*/
                        if (vb_head->ref_end >= bb_next->ref_start) {
                                return FALSE;
                        }
                }
	}
	
        return TRUE;
}
        



void
vscanFileFree(struct vscanFile **pVf) 
{
	/* Free vscan file. */
	struct vscanFile *vf = *pVf;
	if (vf != NULL)
	{
		lineFileClose(&vf->lf);
		freeMem(vf->fileName);
		vscanBlockFreeList(&vf->vscanBlockList);
                bdfBlockFreeList(&vf->bdfBlockList);
                freez(pVf);
	}
}

void
configFileFree(struct configFile **pCf) 
{
	/* Free config file. */
	struct configFile *cf = *pCf;
	if (cf != NULL)
	{
		freeMem(cf->fileName);
                if (cf->bdfBlockFile != NULL) {
                        freeMem(cf->bdfBlockFile);
                }
                if (cf->outFile != NULL) {
                        freeMem(cf->outFile);
                }
                
		freez(pCf);
	}
}

void
vscanPolyArrayFree(struct vscanPolyArray **pvpa)
{
        /*SH: Free the complete vscanPolyArray*/
        struct vscanPolyArray *vpa = *pvpa;
        long i = 0;
        
        if (vpa != NULL)
        {
                freeMem(vpa->listDisc);
                freeMem(vpa->listMonoGap);
                freeMem(vpa->listRefGap);
                freeMem(vpa->vps);
                
                i = (vpa->size) - 1;
                do {
                        freeMem(vpa->polyArray[i]);
                        i--;
                } while (i>=0);

                freeMem(vpa->polyArray);
                freez(pvpa); 
        }
}


void
vscanBlockFree(struct vscanBlock **pvb)
{
	/* Free a single vscanBlock. */
	struct vscanBlock *vb = *pvb;

	if (vb != NULL)
	{
		if (vb->segmentType == VSCAN_ALI) {
                        vscanSeqFreeList(&vb->vscanSeqList);
                        chosenSeqFreeList(&vb->chosenSeqList);
                        vscanOutgroupFreeList(&vb->outgroupList);
                }
/* 		dlRemove(vb->node); */
		freez(pvb);
	}
}

void
vscanOutgroupFreeList(struct dlList **pList)
{

	/* Free a list of vscanOutgroups. */
	struct dlList *list = *pList;
	struct vscanOutgroup *vo;
	if (list != NULL)
	{
		struct dlNode *node;
		for (node = list->head; node->next != NULL; node = node->next) {
			vo = node->val;
                        vscanOutgroupFree(&vo);
                }
		/*         freeMem(node->val); */
		freeDlList(pList);
	}
}


void
vscanOutgroupFree(struct vscanOutgroup **pvo)
{
	/* Free a single vscanOutgroup. */
	struct vscanOutgroup *vo = *pvo;
/* 	struct vscanSeq *vs = *pvs; */

	if (vo != NULL)
	{
/*                 vs = pvo->vos; */
/*                 freez(vs); */
		freez(pvo);
	}
}

void
vscanSeqFreeList(struct dlList **pList)
{
	/* Free a list of vscanSeqs. */
	struct dlList *list = *pList;
	struct vscanSeq *vs;
	if (list != NULL)
	{
		struct dlNode *node;
		for (node = list->head; node->next != NULL; node = node->next) {
			vs = node->val;
                        vscanSeqFree(&vs);
                }
		/*         freeMem(node->val); */
		freeDlList(pList);
	}
}

void
vscanSeqFree(struct vscanSeq **pvs)
{
	/* Free a single vscanSeq. */
	struct vscanSeq *vs = *pvs;
	if (vs != NULL)
	{
		freeDyString(&vs->dySeq);
		freez(pvs);
	}
}


void
vscanBlockFreeList(struct dlList **pList)
{
	/* Free a list of vscanBlocks. */
	struct dlList *list = *pList;
        struct vscanBlock *vb;
	if (list != NULL)
	{
		struct dlNode *node;
		for (node = list->head; node->next != NULL; node = node->next){
			vb = node->val;
                        vscanBlockFree(&vb);
		/*         freeMem(node->val); */
                }
		freeDlList(pList);
	}
}

void
chosenSeqFreeList(struct dlList **pList)
{
        /* Free the list of chosen Seqs, but not the values*/
        struct dlList *list = *pList;
        if (list != NULL)
        {
                freeDlList(pList);
        }
}

void
vscanPopFree(struct vscanPopulation **pvpop)
{
	/* Free a single vscanPopulation. */
        struct vscanPopulation *vpop = *pvpop;

        if (vpop != NULL)
        {
                vscanIndivListFree(&vpop->vindivList);
                freez(pvpop);
        }
}

void
vscanIndivListFree(struct dlList **pList)
{
        /* Free the vindivList*/
        struct dlList *list = *pList;
        struct vscanIndividual *vindiv;
        if (list != NULL)
        {
		struct dlNode *node;
		for (node = list->head; node->next != NULL; node = node->next){
                        vindiv = node->val;
                        vscanIndividualFree(&vindiv);
                }
                freeDlList(pList);
        }
}
                
void
vscanIndividualFree(struct vscanIndividual **pvindiv)
{
        /* Free a single Individual */
        struct vscanIndividual *vindiv = *pvindiv;
        if (vindiv != NULL)
        {
                freeMem(vindiv->name);
                freez(pvindiv);
        }
}


void
bdfBlockFreeList(struct dlList **pList)
{
        /* Free the bdfBlockList */
        struct dlList *list = *pList;
        struct bdfBlock *bb;
        if (list != NULL)
        {
		struct dlNode *node;
		for (node = list->head; node->next != NULL; node = node->next){
                        bb = node->val;
                        bdfBlockFree(&bb);
                }
                freeDlList(pList);
        }
}

void
bdfBlockFree(struct bdfBlock **pbb)
{
        /* Free a bdfBlock*/
        struct bdfBlock *bb = *pbb;
        if (bb != NULL)
        {
                freeMem(bb->feature);
                freez(pbb);
        }
}

void
vscanAnalysisFree(struct analysis **pana, int num)
{
        /* Free the analysis variables*/
        struct analysis *ana = *pana;
        int i;

        if (ana != NULL)
        {
                freeMem(ana->a1);
                freeMem(ana->a2);
                freeMem(ana->varSW);
                freeMem(ana->varBdf);
                
                for (i=0;i<=num;i++) {
                        freeMem(ana->s[i]);
                }
                freeMem(ana->s);
                
                freez(pana);
        }
}
