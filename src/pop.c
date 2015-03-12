#include "pop.h"

#ifndef DLIST_H
#include "dlist.h"
#endif

struct vscanPopulation *createEmptyPopulation ()
{
        struct vscanPopulation *vpop;

        AllocVar(vpop);
        vpop->vindivList = newDlList();
        return vpop;
}

void createNumberOfIndividuals (struct vscanPopulation *vpop, int numSeq)
{
        
        struct vscanIndividual *vindiv;
        int i;

        for (i=0;i<numSeq;i++) {
                vindiv = createEmptyIndividual(vpop->vindivList);
                vindiv->id = i+1;
        }        
}



struct vscanIndividual *createEmptyIndividual (struct dlList *vindivList)
{
  struct vscanIndividual *vindiv;
  AllocVar(vindiv);

  vindiv->node = dlAddValTail(vindivList, vindiv);
  
  return vindiv;
}

void writeIndivNames (struct vscanPopulation *vpop)
{
        struct dlNode *node;
        struct vscanIndividual *vindiv;
        char IndivName[128];
        int i = 0;
        for (node = vpop->vindivList->head; !dlEnd(node); node = node->next) {
                vindiv = node->val;
                sprintf(IndivName,"Seq %d",i+1);
                vindiv->name = cloneString(IndivName);
                i++;
        }
}

void popOutput (struct vscanPopulation *vpop)
{
        struct dlNode *node;
        struct vscanIndividual *vindiv;
        
        printf("\nPopulation parameters:\n");
        for (node = vpop->vindivList->head; !dlEnd(node); node = node->next) {
                vindiv = node->val;
                printf ("vindiv_id %d\tname %s\tseqChoice %d\toutgroup %d\n", 
                         vindiv->id, vindiv->name, vindiv->seqChoice, vindiv->outgroup);
        }
}


void
insertOutgroup (struct dlList *outgroupList, struct vscanOutgroup *vo)
{
        /*SH: here we insert an outgroup into the the correct
          position of outgroupList*/

        struct dlNode *vo_node;
        struct vscanOutgroup *vo_old;

        if (outgroupList->head == NULL) {
                /*SH: outgroupList is empty, add as head*/
                dlAddValHead(outgroupList, vo);
        }
        else {
                /*SH: check if phylo is larger or equal to the current tail.
                  If so, add as new tail*/
                vo_node = outgroupList->tail;
                vo_old = vo_node->val;
                
                if (vo->phylo >= vo_old->phylo) {
                        vo->node = dlAddValTail(outgroupList, vo);                      
                }

                else {
                        /*SH: Jump through list and insert vo at the appropriate position*/
                        vo_node = outgroupList->head;
                        vo_old = vo_node->val;

                        while (1) {
                                if (vo->phylo < vo_old->phylo) {
                                        vo->node = dlAddValBefore (vo_node, vo);
                                        break;
                                }
                                vo_node = vo_node->next;
                                vo_old = vo_node->val;
                        }
                }                         
        }
}

void
createNewOutgroup (struct dlList *outgroupList, 
                   struct vscanIndividual *vindiv, 
                   struct vscanSeq *vs)
{
        /*SH: create a new outgroup and assign the basic variables.
          move the node to the correct position inside outgroupList*/
        struct vscanOutgroup *vo;
        AllocVar(vo);
        vo->phylo = vindiv->outgroup;
        vo->vos = vs;
        insertOutgroup(outgroupList, vo);
}


void 
chooseSequences (struct vscanPopulation *vpop, struct vscanFile *vf)
{
        /*SH: here we choose the individuals defined by SeqChoice in the config File.
          Selected ingroup sequences are nodes inside chosenSeqList. 
          This is done for all vscanBlocks currently in memory*/

        struct dlNode *vindiv_node;
        struct vscanIndividual *vindiv;

        struct dlNode *vs_node;
        struct vscanSeq *vs;
        
        struct dlNode *chosenVs_node;

        struct dlNode *vo_node;
        struct vscanOutgroup *vo;

        struct dlNode *vb_node;
        struct vscanBlock *vb;

        int k;

        for (vb_node = vf->vscanBlockList->head; !dlEnd(vb_node); vb_node = vb_node->next) {
                vb = vb_node->val;
                
                if (vb->segmentType == VSCAN_ALI || vb->segmentType == 3) {
                        /*SH: if we have an ALI (or hapmap format), choose seqs and outgroups*/
                        vindiv_node = vpop->vindivList->head;

                        for (vs_node = vb->vscanSeqList->head; !dlEnd(vs_node); vs_node = vs_node->next) {
                                vs = vs_node->val;
                                vindiv = vindiv_node->val;

         
                                if (vindiv->seqChoice && !vindiv->outgroup) {
                                        chosenVs_node = dlAddValTail(vb->chosenSeqList, vs);
                                }
                                
                                if (vindiv->seqChoice && vindiv->outgroup > 0) {
                                        createNewOutgroup(vb->outgroupList, vindiv, vs);
                                }
                                
                                vindiv_node = vindiv_node->next;
                        }
                        
                        /*SH: give the outgroups consecutive numbers*/
                        k=0;
                        for (vo_node = vb->outgroupList->head; !dlEnd(vo_node); vo_node = vo_node->next) {
                                vo = vo_node->val;
                                vo->id = ++k;
                        }
                }
        }
}

