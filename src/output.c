#include "output.h"

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

void
printBdfHead(struct bdfBlock *bb, struct configFile *cf)
{
        /*SH: prints the head of the output for a BDF block*/
        printf("#Analysing block num.%d\n",bb->id);
        printf("#Feature: %s\n",bb->feature);
        
        if (cf->boolSW) {
                printf("#Performing sliding window analysis with:\n");
                printf("#Width: %lu\tSlide: %lu\n",cf->widthSW,cf->jumpSW);
/*                     "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 " */
                printf("# RefStart     Refend     RefMid      Start        End   Midpoint   NumSites    Missing ");
                
                switch (cf->runMode) {
                        
                case 11:
/*                             "1234567890 1234567890 1234567890 1234567890\n" */
                        printf("         S        Eta         Pi      Theta\n");
                        break;
                        
                case 12:
/*                             "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                        printf("         S        Eta      Eta_E         Pi      Theta   Tajima_D FuLi_Dstar FuLi_Fstar\n");
                        break;

                case 21:
/*                             "1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                        printf("         S        Eta    S_inter         Pi          K\n");
                        break;
                        
                case 22:
/*                             "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                        printf("         S        Eta      Eta_E         Pi     FuLi_D     FuLi_F    FayWu_H\n");
                        break;
                        
                case 31:
/*                             "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                        printf("  LD_sites          D        |D|         D'       |D'|        r^2          h         Hd         Pi      Fu_Fs\n");
                        break;
                        
                }
        }
}

void
printWindowResults (struct variables *var, struct configFile *cf)
{
        /*SH: prints the results of the window analysis*/
 
        printf("%10lu %10lu %10lu %10lu %10lu %10lu %10lu %10.7f ",
               var->ref_start,
               var->ref_end,
               var->ref_mid,
               var->start,
               var->end,
               var->mid,
               var->numSites,
               (double)var->discNucs/(double)var->numSites);
        
        switch (cf->runMode) {
                
        case 11:
                printf("%10lu %10lu %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->PiPerSite,
                       var->ThetaPerSite);
                break;

        case 12:
                printf("%10lu %10lu %10lu %10.7f %10.7f %10.7f %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->Eta_E,
                       var->PiPerSite,
                       var->ThetaPerSite,
                       var->Tajima_D,
                       var->FuLi_D,
                       var->FuLi_F);
                break;
                
        case 21:
                printf("%10lu %10lu %10lu %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->interSegs,
                       var->PiPerSite,
                       var->KPerSite);
                break;
                
        case 22:
                printf("%10lu %10lu %10lu %10.7f %10.7f %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->Eta_E,
                       var->PiPerSite,
                       var->FuLi_D,
                       var->FuLi_F,
                       var->Fay_and_Wu_H);
                break;
                
        case 31:
                printf("%10lu %10.7f %10.7f %10.7f %10.7f %10.7f %10d %10.7f %10.7f %10.7f\n",
                       var->LD_sites,
                       var->D_Lewontin,
                       var->D_Lewontin_abs,
                       var->D_prime,
                       var->D_prime_abs,
                       var->r_square,
                       var->numHaps,
                       var->Hd,
                       var->PiPerSite,
                       var->Fu_Fs);
                break;
        }
}


/* static boolean opened = FALSE; */
/* static FILE *output; */

/*                 if (BINARY) { */
/*                         if (opened == TRUE) { */
/*                                 output = fopen ("output.bin","wb"); */
/*                                 opened = TRUE; */
/*                                 fwrite((char *)(&(var->PiPerSite)),sizeof(float),1,output); */
/*                         } */
/*                         else { */
/*                                 fwrite((char *)(&(var->PiPerSite)),sizeof(float),1,output); */
/*                         } */
/*                 } else { */
/*                         printf("%09lu %.5f\n", */
/*                                var->mid, */
/*                                var->PiPerSite); */
/*                 } */



void
printBdfBlockResults(struct bdfBlock *bb, struct variables *var, struct configFile *cf)
{
        /*SH: prints the results for the complete BDF block*/
        printf("#Analysis for the complete block:\n");
        printf("#Config file: %s\n", cf->fileName);
        printf("#Options: useMuts: %d, runMode: %d, completeDeletion: %d, fixNum: %d, numNuc: %d, startPosition: %lu, endPosition: %lu\n", 
               cf->useMuts, 
               cf->runMode, 
               cf->completeDeletion, 
               cf->fixNum, 
               cf->numNuc,
               cf->startPosition, 
               cf->endPosition);
        printf("#Start in reference sequence: %lu\n",bb->ref_start);
        printf("#End in reference sequence: %lu\n",bb->ref_end);
        printf("#Start in alignment: %lu\n",bb->start);
        printf("#End in alignment: %lu\n",bb->end);
        printf("#Num of analysed sites: %lu\n",var->numSites);
        printf("#Num of discarded sites: %lu\n",(bb->end - bb->start + 1 - var->numSites));
        printf("#Num of dicarded nucleotides per site: %f\n",(double)var->discNucs/(double)var->numSites);
        
        switch (cf->runMode) {
                
        case 11:
/*                     "1234567890 1234567890 1234567890 1234567890\n" */
                printf("#        S        Eta         Pi      Theta\n");
                
                printf("#%9lu %10lu %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->PiPerSite,
                       var->ThetaPerSite);
                break;
                
        case 12:
/*                     "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                printf("#        S        Eta      Eta_E         Pi      Theta   Tajima_D FuLi_Dstar FuLi_Fstar\n");
                
                printf("#%9lu %10lu %10lu %10.7f %10.7f %10.7f %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->Eta_E,
                       var->PiPerSite,
                       var->ThetaPerSite,
                       var->Tajima_D,
                       var->FuLi_D,
                       var->FuLi_F);
                break;
                
        case 21:
/*                     "1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                printf("#        S        Eta    S_inter         Pi          K\n");
                
                printf("#%9lu %10lu %10lu %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->interSegs,
                       var->PiPerSite,
                       var->KPerSite);
                break;
                
        case 22:
/*                     "1234567890 1234567890 1234567890 1234567890 1234567890 1234567890 1234567890\n" */
                printf("#        S        Eta      Eta_E         Pi     FuLi_D     FuLi_F    FayWu_H\n");
                
                printf("#%9lu %10lu %10lu %10.7f %10.7f %10.7f %10.7f\n",
                       var->segSites,
                       var->Eta,
                       var->Eta_E,
                       var->PiPerSite,
                       var->FuLi_D,
                       var->FuLi_F,
                       var->Fay_and_Wu_H);
                break;
        }
}

void
dumpPhylip (struct vscanFile *vf, struct vscanPopulation *vpop) 
{
        struct dlNode *vb_node;
        struct vscanBlock *vb;

        static FILE *dump;
        static int lastVb=0;
        int i;
/*         int j; */
        int numlen;
        int numpos;
        char buffer[256];
        int listlen;
        
        if (vf->format == FORMAT_MGA) {
                vfError(vf,"Dumping of MGA files is not supported!");
        }
        
        vb_node = vf->vscanBlockList->head;
        vb = vb_node->val;

        listlen = fastBlockListCount(vf->vscanBlockList);
        for (i=0;i<listlen;i++) {
                if (vb->id > lastVb) {
                        lastVb++;
                        
                        if (lastVb == 1) {
                                dump = fopen ("phydump.phy","w");
                                dumpFirstPara(dump,vf,vpop,vb);
                        }
                        else {
                                dumpNextPara(dump,vf,vb);
                        }
                        
                }

                vb_node = vb_node->next;
                vb = vb_node->val;
        }

        if (!vf->rambuffer_state) {
                numlen = sprintf(buffer,"%d",vf->numSeq);
                fseek(dump,numlen + 1,SEEK_SET);

                vb = vf->vscanBlockList->tail->val;
                numpos = sprintf(buffer,"%lu",vb->end);

                fprintf(dump,"%s",buffer);

                for (i=0;i<10 - numpos;i++) {
                        fputc(' ',dump);
                }

                fclose(dump);
        }

}

void
dumpFirstPara (FILE *dump, struct vscanFile *vf, struct vscanPopulation *vpop, struct vscanBlock *vb)
{

        struct dlNode *vs_node;
        struct vscanSeq *vs;
        
        struct dlNode *vi_node;
        struct vscanIndividual *vi;
        int i;
        unsigned long j;

        char name[PHYLIP_LABEL_LEN + 1];

        fprintf(dump,"%d ##########\n",vf->numSeq);
        
        vi_node = vpop->vindivList->head;
        vi = vi_node->val;

        vs_node = vb->vscanSeqList->head;
        vs = vs_node->val;

        for (i=0;i<vf->numSeq;i++) {
                strncpy(name,vi->name,PHYLIP_LABEL_LEN);
                name[PHYLIP_LABEL_LEN] = '\0';
                fprintf(dump,"%s",name);
                
                for (j=0;j<PHYLIP_LABEL_LEN - strlen(name) + 1;j++) {
                        fputc(' ',dump);
                }
                
                if (vb->segmentType == VSCAN_ALI) {
                        fprintf(dump,"%s\n",vs->dySeq->string);

                        vs_node = vs_node->next;
                        vs = vs_node->val;
                }

                if (vb->segmentType == VSCAN_GAP){
                        for (j=0;j<vb->len;j++) {
                                fputc('N',dump);
                        }
                        fputc('\n',dump);
                }

                vi_node = vi_node->next;
                vi = vi_node->val;
                        
        }
        fputc('\n',dump);
}
                

        

void
dumpNextPara (FILE *dump, struct vscanFile *vf, struct vscanBlock *vb)
{
        struct dlNode *vs_node;
        struct vscanSeq *vs;
        int i;
        unsigned long j;

        vs_node = vb->vscanSeqList->head;
        vs = vs_node->val;
                
        for (i=0;i<vf->numSeq;i++) {
          
                if (vb->segmentType == VSCAN_ALI) {
                        fprintf(dump,"%s\n",vs->dySeq->string);
                        
                        vs_node = vs_node->next;
                        vs = vs_node->val;
                        
                }
                        
                if (vb->segmentType == VSCAN_GAP){
                        for (j=0;j<vb->len;j++) {
                                fputc('N',dump);
                        }
                        fputc('\n',dump);
                }
                
        }
        
        fputc('\n',dump);
}

void 
printPolyArray (struct vscanFile *vf)
{
        /*SH: just the output of the current array and parameters in memory,
          so I can check if everything is working*/

        int i;
        for (i=0;i<(vf->vpa->filled);i++) {
                printf("%s\t%lu\t%d %d %d %d %d\t",
                       vf->vpa->polyArray[i],vf->vpa->vps[i].position,
                       vf->vpa->vps[i].A, vf->vpa->vps[i].C,
                       vf->vpa->vps[i].G, vf->vpa->vps[i].T,
                       vf->vpa->vps[i].valid);
        
                printf("%d\n",vf->chosenNumSeq);
        }
        
        printf("\nDiscarded sites:\n");
        for (i=0;i<vf->vpa->filledDisc;i++) {
/*                 printf("%u\n",vf->vpa->listDisc[i]); */
                printf("%lu\n",vf->vpa->listDisc[i].position);
        }


        printf("\nNumber of chosen sequences: %d\n", vf->chosenNumSeq);
        printf("Number of chosen outgroups: %d\n\n", vf->chosenNumOut);

        printf("Number of gapped or ambiguous columns: %lu\n", vf->vpa->gapAmb);
        printf("Number of discarded columns: %lu\n", vf->vpa->filledDisc);
        printf("Number of monomorphic columns: %lu\n", vf->vpa->monomorph);
        printf("Number of polymorphic columns: %lu\n", vf->vpa->filled);

}

void
printBlocks (struct vscanFile *vf)
{
        /*another test output. prints all vscanBlocks
          currently in memory*/

        struct vscanBlock *vb;
        struct dlNode *vb_node;

        struct vscanSeq *vs;
        struct dlNode *vs_node;
        
        struct vscanOutgroup *vo;
        struct dlNode *vo_node;
        
        for (vb_node = vf->vscanBlockList->head;
             !dlEnd(vb_node);
             vb_node=vb_node->next) {
                vb=vb_node->val;
                printf("\n");
                printf("vb_id=%d\tvb_type=%d\n",vb->id,vb->segmentType);
                printf("start=%lu\tend=%lu\tlen=%lu\n",
                       vb->start,
                       vb->end,
                       vb->len);
                printf("ref_start=%lu\tref_end=%lu\tref_len=%lu\n",
                       vb->ref_start,
                       vb->ref_end,
                       vb->ref_len);
                
                printf ("All sequences:\n");
                for (vs_node = vb->vscanSeqList->head;
                     !dlEnd(vs_node);
                     vs_node=vs_node->next) {
                        vs=vs_node->val;
                        printf("Seq %d\t%s\n",vs->id,vs->dySeq->string);
                }
                printf("\n");

                printf ("Chosen ingroup sequences:\n");
                for (vs_node = vb->chosenSeqList->head;
                     !dlEnd(vs_node);
                     vs_node=vs_node->next) {
                        vs=vs_node->val;
                        printf("Seq %d\t%s\n",vs->id,vs->dySeq->string);
                }
                printf("\n");

                printf ("Chosen outgroup sequences:\n");
                for (vo_node = vb->outgroupList->head;
                     !dlEnd(vo_node);
                     vo_node=vs_node->next) {
                        vo=vo_node->val;
                        printf("Seq %d\t%s\n",vo->vos->id,vo->vos->dySeq->string);
                }
                printf("\n");

        }
}

void printPops (struct vscanPopulation *vpop)
{
     struct dlNode *node;
     struct vscanIndividual *indiv;
     
     for (node = vpop->vindivList->head;
         !dlEnd(node);
         node = node->next) {
              
              indiv = node->val;
              printf("id=%d\tname=%s\toutgroup=%d\tseqChoice=%d\n",
              indiv->id,
              indiv->name,
              indiv->outgroup,
              indiv->seqChoice);
     }
}
