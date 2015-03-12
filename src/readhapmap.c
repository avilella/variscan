#include "readhapmap.h"

#ifndef VARISCAN_H
#include "variscan.h"
#endif

#ifndef DLIST_H
#include "dlist.h"
#endif

#ifndef DYSTRING_H
#include "dystring.h"
#endif

#ifndef LINEFILE_H
#include "linefile.h"
#endif

#ifndef POP_H
#include "pop.h"
#endif

#include <limits.h>

/* static void vfError(struct vscanFile *vf, char *message); */
/* static void cfError(struct configFile *cf, char *message); */


int 
hapmapHeader (struct vscanFile *vf,
              struct vscanPopulation *vpop) 
{
       /* Read the header line of a hapmap genotype data dump file */
       
       /* Example: */
       /* rs# SNPalleles chrom pos strand genome_build center protLSID assayLSID panelLSID NA06985 NA06991 NA06993 NA06993.dup NA06994 NA07000 NA07019 NA07022 NA07029 NA07034 NA07048 NA07055 NA07056 NA07345 NA07348 NA07357 NA10830 NA10831 NA10835 NA10838 NA10839 NA10846 NA10847 NA10851 NA10854 NA10855 NA10856 NA10857 NA10859 NA10860 NA10861 NA10863 NA11829 NA11830 NA11831 NA11832 NA11839 NA11840 NA11881 NA11882 NA11992 NA11993 NA11993.dup NA11994 NA11995 NA12003 NA12003.dup NA12004 NA12005 NA12006 NA12043 NA12044 NA12056 NA12057 NA12144 NA12145 NA12146 NA12154 NA12155 NA12156 NA12156.dup NA12234 NA12236 NA12239 NA12248 NA12248.dup NA12249 NA12264 NA12707 NA12716 NA12717 NA12740 NA12750 NA12751 NA12752 NA12753 NA12760 NA12761 NA12762 NA12763 NA12801 NA12802 NA12812 NA12813 NA12814 NA12815 NA12864 NA12865 NA12872 NA12873 NA12874 NA12875 NA12878 NA12891 NA12892 */
       
       /* rs2693665 A/T Chr7 26700834 - ncbi_b34 perlegen urn:lsid:perlegen.hapmap.org:Protocol:Genotyping_1.0.0:1 urn:lsid:perlegen.hapmap.org:Assay:PS03973492:1 urn:lsid:dcc.hapmap.org:Panel:CEPH-30-trios:1 AA AA NN AA AT AT AT AA AA AT AT AA AT AA AA AA AT TT AT AA AA AA AT AA AA AT AA AA AA AA AT AT AA AT AA AT AA AA AA AA AA AA AA AA AT AA AA AA AA AA AA AA AA AA AA AA AT AA AT AT AT AA AT AT AA AA TT AT AA AA AT AT AA TT AA AT AA AA AA TT AA AA AA AA AA AT AT AT AT AA TT AA TT AT TT */
               
/* Col1: refSNP rs# identifier at the time of release (NB might merge */
/*       with another rs# in the future) */
/* Col2: SNP alleles according to dbSNP */
/* Col3: chromosome that SNP maps to */
/* Col4: chromosome position of SNP, in basepairs on reference sequence */
/* Col5: strand of reference sequence that SNP maps to */
/* Col6: version of reference sequence assembly (currently NCBI build34) */
/* Col7: HapMap genotyping center that produced the genotypes */
/* Col8: LSID for HapMap protocol used for genotyping */
/* Col9: LSID for HapMap assay used for genotyping */
/* Col10: LSID for panel of individuals genotyped */
/* Col11: QC-code, currently 'QC+' for all entries (for future use) */
/* Col12 and on: observed genotypes of samples, one per column, sample */
/*       identifiers in column headers (Coriell catalog numbers, example: */
/*       NA10847). Duplicate samples have .dup suffix. */

       /* word[0] = rs# */
       /* word[1] = SNPalleles */
       /* word[2] = chrom */
       /* word[3] = pos */
       /* word[4] = strand */
       /* word[5] = genome_build */
       /* word[6] = center */
       /* word[7] = protLSID */
       /* word[8] = assayLSID */
       /* word[9] = panelLSID */
       /* word[10] = QC-code */
       /* word[11] to word [11+numseq] = NAxxxxx */

	struct lineFile *lf = vf->lf;
	char *line;
        int lineSize;

        struct vscanBlock *vb;

	/* Calling with retLine */
	if (!lineFileNext(lf, &line, &lineSize)) {
		vfError(vf, "Expecting header line (hapmapHeader)");
	}
        writeHapmapNames(vpop, line, 11);

        /*SH: we are creating a pseudo-vscanBlock, so all functions
          depending on vscanBlocks will work. Since the length of the
          chromosome is unknown, the start is set to 1 and the end is 
          set to ULONG_MAX - 1, since ULONG_MAX is sometimes used as 
          a checking value*/
        vb = createEmptyvscanBlock(vf->vscanBlockList);
        vb->start = vb->ref_start = 1;
        vb->end = vb->ref_end = ULONG_MAX - 1;
        vb->len = vb->ref_len = ULONG_MAX - 1;
        
        /*SH: set the type to 3, so the program just ignores the block
          in other functions*/
        vb->segmentType = 3;
        
        /* create empty sequences and choose them */
        createNumOfSeqsvscanSeq(vf->numSeq,vb->vscanSeqList);
        chooseSequences(vpop,vf);
        
        return 0;
}


int 
hapmapNextIndividual (struct vscanFile *vf,
                      struct configFile *cf)
{
        /* Read next individual in hapmap genotype dump data file */

        /* Example: */

       /* rs2693665 A/T Chr7 26700834 - ncbi_b34 perlegen urn:lsid:perlegen.hapmap.org:Protocol:Genotyping_1.0.0:1 urn:lsid:perlegen.hapmap.org:Assay:PS03973492:1 urn:lsid:dcc.hapmap.org:Panel:CEPH-30-trios:1 AA AA NN AA AT AT AT AA AA AT AT AA AT AA AA AA AT TT AT AA AA AA AT AA AA AT AA AA AA AA AT AT AA AT AA AT AA AA AA AA AA AA AA AA AT AA AA AA AA AA AA AA AA AA AA AA AT AA AT AT AT AA AT AT AA AA TT AT AA AA AT AT AA TT AA AT AA AA AA TT AA AA AA AA AA AT AT AT AT AA TT AA TT AT TT */
               
       /* word[0] = rs# */
       /* word[1] = SNPalleles */
       /* word[2] = chrom */
       /* word[3] = pos */
       /* word[4] = strand */
       /* word[5] = genome_build */
       /* word[6] = center */
       /* word[7] = protLSID */
       /* word[8] = assayLSID */
       /* word[9] = panelLSID */
       /* word[10] to word [10+numseq] = NAxxxxx */

	struct lineFile *lf = vf->lf;
        struct dlNode *vb_node,*vs_node, *vo_node;
        struct vscanBlock *vb;
        struct vscanSeq *vs;
        struct vscanOutgroup *vo;
	char *line;
        int lineSize;
	char *cloneline;
	char *words[255];
	int wordCount;
        char *polyColumn, *fullColumn;
        struct baseCount *bc;
        int i;
        int count = 10;
        unsigned long pos = 0;
        unsigned long ref_pos = 0;

	/* Calling with retLine */
	if (!lineFileNext(lf, &line, &lineSize)) {
                return 0;
	}
	cloneline = cloneString(line);
	wordCount = chopLine(cloneline, words);
	if (wordCount < count+1) {
		vfError(vf,"Expecting more than 11 words in line (hapmapNextIndividual)");
	}
	wordCount = chopLine(cloneline, words);

        bc = needMem(sizeof(struct baseCount));
        resetBaseCount(bc);

        polyColumn = needMem((vf->chosenNumSeq + vf->chosenNumOut + 1)*sizeof(char));
        polyColumn[vf->chosenNumSeq + vf->chosenNumOut] = '\0';

        /* a vector to store the whole segregating site in */
        fullColumn = needMem((vf->numSeq + 1 )*sizeof(char));
        fullColumn[vf->numSeq] = '\0';
        
        for (i=0; i < vf->numSeq/2; i++) {
            fullColumn[i*2] = words[i+11][0];
            fullColumn[i*2+1] = words[i+11][1];
        }
        
        ref_pos = pos = ulround(doubleExp(words[3]));
        
        /* select the ingroup and outgroups */
        vb_node = vf->vscanBlockList->head;
        vb = vb_node->val;
        
        i=0;
        for (vs_node = vb->chosenSeqList->head; !dlEnd(vs_node); vs_node = vs_node->next) {
            vs = vs_node->val;
            polyColumn[i++] = countBase(fullColumn[vs->id-1], fullColumn[0], bc);
        }
        
        i=0;
        for (vo_node = vb->outgroupList->head; !dlEnd(vo_node); vo_node = vo_node->next) {
            vo = vo_node->val;
            polyColumn[vf->chosenNumSeq + i++] = countBase(fullColumn[vo->vos->id-1], fullColumn[0], bc);
        }
         
        if (cf->runMode == 11 || cf->runMode == 12 || cf->runMode == 31) {
                checkColumnsIngroup(vf,cf,bc,pos,ref_pos,polyColumn);
/*                 if (cf->runMode == 31) { */
/*                         cfError(cf, "When calculating LD, haplotypes and Fu's Fs your data must be phased! Make sure your HAPMAP file is using phased data or the results will be invalid!"); */
/*                 } else { */
/*                         checkColumnsIngroup(vf,cf,bc,pos,ref_pos,polyColumn); */
/*                 } */
        }
        else if (cf->runMode == 21) {
                checkColumnsRunMode21(vf,cf,bc,pos,ref_pos,polyColumn);
        }
        else if (cf->runMode == 22) {
                checkColumnsRunMode22(vf,cf,bc,pos,ref_pos,polyColumn);
        }
        
        /* fillPolyColumn(vf, polyColumn, pos, ref_pos, bc); */

        freeMem(fullColumn);
        freeMem(polyColumn);
        freeMem(cloneline);
        freeMem(bc);
        return 1;
}

void writeHapmapNames (struct vscanPopulation *vpop, 
                       char *line, 
                       int count)
{
        /* Write the name of the individuals in the vpop structure */
        struct dlNode *node;
        struct vscanIndividual *vindiv = NULL;
	char *cloneline;
	char *words[255];
	int wordCount;
        int i = 0;
	struct dyString *dystrName;
        boolean done_first = FALSE;

	cloneline = cloneString(line);
	wordCount = chopLine(cloneline, words);
	if (wordCount < count+1) {
		errAbort("Expecting header line (hapmapHeader)");
	}
	wordCount = chopLine(cloneline, words);

	dystrName = newDyString(0);

        for (node = vpop->vindivList->head; !dlEnd(node); node = node->next) {
                if (done_first == FALSE) {
                        vindiv = node->val;
                        dyStringAppend(dystrName, words[count+i]);
                        dyStringAppend(dystrName, ".allele1");
                        vindiv->name = cloneString(dystrName->string);
                        done_first = TRUE;
                } else {
                        vindiv = node->val;
                        dyStringAppend(dystrName, words[count+i]);
                        dyStringAppend(dystrName, ".allele2");
                        vindiv->name = cloneString(dystrName->string);
                        done_first = FALSE;
                }
                dyStringClear(dystrName);
        }
        freeMem(cloneline);
        dyStringFree(&dystrName);
}


/* unsigned long  */
/* hapmapCountAlleles (struct vscanFile *vf)  */
/* { */
/*        /\* Read the header line of a hapmap genotype data dump file and */
/*         * return the number of alleles, which is the double of the */
/*         * number of samples *\/ */
       
/*        /\* Example: *\/ */
/*        /\* rs# SNPalleles chrom pos strand genome_build center protLSID assayLSID panelLSID NA06985 NA06991 NA06993 NA06993.dup NA06994 NA07000 NA07019 NA07022 NA07029 NA07034 NA07048 NA07055 NA07056 NA07345 NA07348 NA07357 NA10830 NA10831 NA10835 NA10838 NA10839 NA10846 NA10847 NA10851 NA10854 NA10855 NA10856 NA10857 NA10859 NA10860 NA10861 NA10863 NA11829 NA11830 NA11831 NA11832 NA11839 NA11840 NA11881 NA11882 NA11992 NA11993 NA11993.dup NA11994 NA11995 NA12003 NA12003.dup NA12004 NA12005 NA12006 NA12043 NA12044 NA12056 NA12057 NA12144 NA12145 NA12146 NA12154 NA12155 NA12156 NA12156.dup NA12234 NA12236 NA12239 NA12248 NA12248.dup NA12249 NA12264 NA12707 NA12716 NA12717 NA12740 NA12750 NA12751 NA12752 NA12753 NA12760 NA12761 NA12762 NA12763 NA12801 NA12802 NA12812 NA12813 NA12814 NA12815 NA12864 NA12865 NA12872 NA12873 NA12874 NA12875 NA12878 NA12891 NA12892 *\/ */
       
/*        /\* rs2693665 A/T Chr7 26700834 - ncbi_b34 perlegen urn:lsid:perlegen.hapmap.org:Protocol:Genotyping_1.0.0:1 urn:lsid:perlegen.hapmap.org:Assay:PS03973492:1 urn:lsid:dcc.hapmap.org:Panel:CEPH-30-trios:1 AA AA NN AA AT AT AT AA AA AT AT AA AT AA AA AA AT TT AT AA AA AA AT AA AA AT AA AA AA AA AT AT AA AT AA AT AA AA AA AA AA AA AA AA AT AA AA AA AA AA AA AA AA AA AA AA AT AA AT AT AT AA AT AT AA AA TT AT AA AA AT AT AA TT AA AT AA AA AA TT AA AA AA AA AA AT AT AT AT AA TT AA TT AT TT *\/ */
               
/* /\* Col1: refSNP rs# identifier at the time of release (NB might merge *\/ */
/* /\*       with another rs# in the future) *\/ */
/* /\* Col2: SNP alleles according to dbSNP *\/ */
/* /\* Col3: chromosome that SNP maps to *\/ */
/* /\* Col4: chromosome position of SNP, in basepairs on reference sequence *\/ */
/* /\* Col5: strand of reference sequence that SNP maps to *\/ */
/* /\* Col6: version of reference sequence assembly (currently NCBI build34) *\/ */
/* /\* Col7: HapMap genotyping center that produced the genotypes *\/ */
/* /\* Col8: LSID for HapMap protocol used for genotyping *\/ */
/* /\* Col9: LSID for HapMap assay used for genotyping *\/ */
/* /\* Col10: LSID for panel of individuals genotyped *\/ */
/* /\* Col11: QC-code, currently 'QC+' for all entries (for future use) *\/ */
/* /\* Col12 and on: observed genotypes of samples, one per column, sample *\/ */
/* /\*       identifiers in column headers (Coriell catalog numbers, example: *\/ */
/* /\*       NA10847). Duplicate samples have .dup suffix. *\/ */

/*        /\* word[0] = rs# *\/ */
/*        /\* word[1] = SNPalleles *\/ */
/*        /\* word[2] = chrom *\/ */
/*        /\* word[3] = pos *\/ */
/*        /\* word[4] = strand *\/ */
/*        /\* word[5] = genome_build *\/ */
/*        /\* word[6] = center *\/ */
/*        /\* word[7] = protLSID *\/ */
/*        /\* word[8] = assayLSID *\/ */
/*        /\* word[9] = panelLSID *\/ */
/*        /\* word[10] = QC-code *\/ */
/*        /\* word[11] to word [11+numseq] = NAxxxxx *\/ */

/* 	struct lineFile *lf = vf->lf; */
/* 	char *line; */
/*         int lineSize; */
/*         unsigned long numAlleles; */
/* 	char *cloneline; */
/* 	char *words[10000]; */
/* 	unsigned long wordCount; */

/* 	/\* Calling with retLine *\/ */
/* 	if (!lineFileNext(lf, &line, &lineSize)) { */
/* 		vfError(vf, "Expecting header line (hapmapHeader)"); */
/* 	} */

/* 	cloneline = cloneString(line); */

/* 	wordCount = chopLine_ul(cloneline, words); */
/*         if (wordCount < 11) { */
/* 		errAbort("Expecting more than 11 words in line (hapmapNextIndividual)"); */
/*         } */
/*        /\* word[11] to word [11+numseq] = NAxxxxx *\/ */
/*         numAlleles = (wordCount - 10)*2; */
/*         lineFileReuse(vf->lf); */
        
/*         return numAlleles; */
/* } */



