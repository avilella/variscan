/* Main file */

#ifdef VARISCAN

#ifndef VARISCAN_H
#include "variscan.h"
#endif

#include "statistics.h"
#include "pop.h"
#include "errabort.h"
#include "memalloc.h"
#include "common.h"
#include "dystring.h"
#include "linefile.h"
#include "dlist.h"
#include "free.h"
#include "readmaf.h"
#include "readmga.h"
#include "readaxt.h"
#include "readhapmap.h"
#include "readxmfa.h"
#include "readmav.h"
#include "ran1.h"
#include "output.h"
#include <limits.h>

/*SH: external variable to seed the random number generator*/
long seed; 

void
variscan (char *infile, char *configfile) 
{
	/* Main function: we basically open the input file and the config
	   file, read the options from the config file, then start reading
	   the input file. We check if we have enough part readed to go
	   analyse, then analyse a chunk, free that part from memory, and
	   keep reading. And so on until the end of the file. */ 
	struct configFile *cf;
	struct vscanFile *vf;
	struct vscanPolyArray *vpa;
	struct vscanPopulation *vpop;
        struct analysis *ana;
	int end_of_file = FALSE;
	
	/* Open input file */
        
	vf = vscanFileOpen(infile);

        fprintf(stderr, "#Opening input file...\n");
	vscanFileFormatAndNumSeq(vf);
		
	/*SH: create a population*/
        vpop = createEmptyPopulation();

        /*SH: if we have a FORMAT_MAF or FORMAT_XMFA or FORMAT_MAV,
          the individuals will be created while reading the config
          file */
        if ( !((vf->format == FORMAT_MAF) || 
               (vf->format == FORMAT_XMFA) || 
               (vf->format == FORMAT_MAV)) ) {
                createNumberOfIndividuals(vpop,vf->numSeq);
        }
                
        fprintf(stderr, "#Loading conf file...\n");
	cf = loadConfigFile(vf, vpop, configfile);
        checkConfigFile(cf,vf,vpop);
        loadBlockDataFile(cf, vf);

        fprintf(stderr, "#Initializing variables...\n");
        ana = initAnalysisVariables(cf);
        
        initArrays(vf, ana);

        vpa = createEmptyvscanPolyArray(vf, VSCAN_BLOCK_MIN);

        fprintf(stderr, "#Reading stretches...\n");
	/* Read in segments */
	if (vf->format == FORMAT_MGA) {
                /*SH: write the standard MGA names for sequences*/
                writeIndivNames(vpop);
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = mgaNextSegment(vf);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
		}
	} else if (vf->format == FORMAT_PHYLIP) {
		phylipFirstStretch(vf, cf, vpop);
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = phylipNextStretch(vf,cf);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
		}
	} else if (vf->format == FORMAT_AXT) {
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = axtNextStretch(vf);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
		}
        } else if (vf->format == FORMAT_MAF) {
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = mafNextStretch(vf,cf,vpop);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
                }
        } else if (vf->format == FORMAT_HAPMAP) {
                /* TODO */
                hapmapHeader(vf, vpop);
                /* DEBUG */
                /* printPops(vpop); */
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = hapmapNextIndividual(vf,cf);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
                }
        } else if (vf->format == FORMAT_XMFA) {
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = xmfaNextStretch(vf,cf,vpop);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
                }
        } else if (vf->format == FORMAT_MAV) {
		while (end_of_file == FALSE && !ana->bdfBlocksFinished) {
			vf->rambuffer_state = mavNextStretch(vf,vpop);
			end_of_file = try_analyse(vf->rambuffer_state, cf, vf, vpop, ana);
                }
        }

	/* closing and freeing */
        fprintf(stderr, "#Closing data structures...\n");
        vscanAnalysisFree(&ana,vf->chosenNumSeq);
        vscanPopFree(&vpop);
        fprintf(stderr, "#Closing files...\n");
	configFileFree(&cf);
	vscanFileFree(&vf);
}

struct vscanFile *
vscanFileOpen(char *fileName) 
{
	/* Open input file, read and verify number of seqs. */
	/*   struct vscanFile *vf; */
	struct lineFile *lf;
	struct vscanFile *vf;
  
	AllocVar(vf);
        vf->accum_filled = 0;
	vf->lf = lf = lineFileOpen(fileName, TRUE);
	vf->fileName = cloneString(fileName);
	
	vf->vscanBlockList = newDlList();
	vf->bdfBlockList = newDlList();
        
        vf->rambuffer_state=1;
        
        return vf;
}

void
resetAnalysisVariables(struct variables *var)
{
        /*SH: resets the variables for the analysis of a
          new window or BDF block*/

        var->numSites=0;
        var->segSites=0;
        var->discNucs=0;
        var->Eta=0;
        var->Eta_E=0;
        var->interSegs=0;
        
        var->totalHZ=0;
        var->totalTheta=0;
        var->PiPerSite=0;
        var->ThetaPerSite=0;
        var->totalK=0;
        var->KPerSite=0;

        var->Theta_Pi_Fay_and_Wu=0;
        var->Theta_H=0;

        var->Tajima_D=0;
        var->FuLi_D=0;
        var->FuLi_F=0;
        var->Fay_and_Wu_H=0;

        var->Fu_Fs=0;
        var->D_Lewontin=0;
        var->D_Lewontin_abs=0;
        var->D_prime=0;
        var->D_prime_abs=0;
        var->r_square=0;
        var->Hd=0;
        var->numHaps=0;
        var->LD_sites=0;

        var->cumNumSeq=0;
        var->cum_a1=0;
        var->cum_a2=0;
        var->cum_a1nplus1=0;
}


void
newBdfBlockAnalysis(struct analysis *ana)
{
        /*SH: set the values to initial state for a new round
          of analysis*/
        
        ana->recalcSW=FALSE;

        ana->varSW->start=0;
        ana->varSW->end=0;
        ana->varSW->mid=0;

        ana->varSW->ref_start=0;
        ana->varSW->ref_end=0;
        ana->varSW->ref_mid=0;

        ana->varBdf->start=0;
        ana->varBdf->end=0;
        ana->varBdf->mid=0;

        ana->varBdf->ref_start=0;
        ana->varBdf->ref_end=0;
        ana->varBdf->ref_mid=0;

        ana->windowsFinished=FALSE;
        ana->bdfBlocksFinished=FALSE;
        
        resetAnalysisVariables(ana->varBdf);
        resetAnalysisVariables(ana->varSW);
}

struct analysis *
initAnalysisVariables()
{

        /*SH: create a container for the variables needed for
          analysis of a BDF block and a sliding window*/
        struct analysis *ana;
        struct variables *varSW;
        struct variables *varBdf;

        AllocVar(ana);
        AllocVar(varSW);
        AllocVar(varBdf);

        ana->varSW = varSW;
        ana->varBdf = varBdf;
      
        newBdfBlockAnalysis(ana);

        return ana;
}


void 
vscanFileFormatAndNumSeq(struct vscanFile *vf) 
{
	/* Get file format and number of sequences. Reuse line. */

	struct lineFile *lf = vf->lf;
	char *line;
	char *cloneline;
	char *words[255];
	int lineSize;
	int wordCount;

	/* Calling with retLine */
	if (!lineFileNext(lf, &line, &lineSize)) {
		vfError(vf, "Expecting some kind of header while getting the number of sequences (vscanFileFormatAndNumSeq)");
	}

	cloneline = cloneString(line);
	wordCount = chopLine(cloneline, words);
	if (wordCount < 2) {
		vfError(vf, "Expecting header while getting the number of sequences (vscanFileFormatAndNumSeq)");
	}

	/* latter we check if it is still zero */
	vf->numSeq = 0;
  
        /* It is a FORMAT_PHYLIP file */
        if (endsWith(vf->fileName, "phy") ||
            endsWith(vf->fileName, "phylip") ||
            endsWith(vf->fileName, "phlp") ||
            endsWith(vf->fileName, "phyl") ||
            endsWith(vf->fileName, "ph")
                ) {
                vf->numSeq = intExp(words[0]);
                long numsites = intExp(words[1]);
		vf->format = FORMAT_PHYLIP;
                if (numsites < 1) {
                        vfError(vf, "Expecting positive number of sites (vscanFileFormatAndNumSeq)");
                }

        /* It is a FORMAT_AXT file */
        } else if (endsWith(vf->fileName, "axt")) {
                vf->numSeq = 2;
                vf->format = FORMAT_AXT;
                lineFileReuse(lf);
                
        /* It is a FORMAT_MGA file */
        } else if (endsWith(vf->fileName, "mga") || 
                   endsWith(vf->fileName, "align")
                ) {
		vf->numSeq = wordCount;
		vf->format = FORMAT_MGA;
                /* Starts with MGA_EXA */
                if ( nextlineFileStartsWith(line, lineSize, "Exact") ) {
                        vf->numSeq = wordCount - 1;
                        lineFileReuse(lf);
                        /* If segment is of any other kind there are (wordCount) sequences in file */
                /* Starts with MGA_ALI or MGA_GAP  */
                } else if ( (wordCount > 1) && ((0 < countChars(line, ':')) || (0 < countChars(line, '!'))) ) {
                        vf->numSeq = wordCount;
                        lineFileReuse(lf);
                }
        } else if (endsWith(vf->fileName, "maf")) {
                vf->format = FORMAT_MAF;
                /* the number of sequences will be determined while reading the config file*/
                vf->numSeq = -1;
        } else if (endsWith(vf->fileName, "hapmap") || endsWith(vf->fileName, "txt")) {
                vf->format = FORMAT_HAPMAP;
                vf->numSeq = (wordCount-11)*2;
                lineFileReuse(lf);
        } else if (endsWith(vf->fileName, "xmfa")) {
                vf->format = FORMAT_XMFA;
                lineFileReuse(lf);
                /* the number of sequences will be determined while reading the config file*/
                vf->numSeq = -1;
        } else if (endsWith(vf->fileName, "mav")) {
                vf->format = FORMAT_MAV;
                lineFileReuse(lf);
                /* the number of sequences will be determined while reading the config file*/
                vf->numSeq = -1;
        } else {
		vfError(vf, "Unable to determine the format of input file (vscanFileFormatAndNumSeq)");
	}
	if (vf->numSeq == 0) {
		vfError(vf, "Unable to determine the number of sequences in input file (vscanFileFormatAndNumSeq)");  
	}
        freeMem(cloneline);
}

void
checkConfigFile (struct configFile *cf, 
                 struct vscanFile *vf, 
                 struct vscanPopulation *vpop)
{
        struct dlNode *vindiv_node;
        struct vscanIndividual *vindiv;

        for (vindiv_node = vpop->vindivList->head; 
             !dlEnd(vindiv_node); 
             vindiv_node = vindiv_node->next) {
                
                vindiv=vindiv_node->val;
                if (vindiv->seqChoice && vindiv->outgroup == 0) {
                        vf->chosenNumSeq++;
                }
                if (vindiv->seqChoice && vindiv->outgroup > 0) {
                        vf->chosenNumOut++;
                }
        }
        
        /*set some variables before checking*/

        if (cf->runMode > 10 && cf->runMode < 20) {
                vf->chosenNumOut = 0;
        }

        if (cf->completeDeletion) {
                cf->numNuc = vf->chosenNumSeq;
                cf->fixNum = 1;
        }
        
        /*now check some general parameters*/

        if (cf->runMode < 11 || cf->runMode > 51) 
                cfError(cf,"Please select a Runmode between 11 and 51!");

        if (cf->boolSW && cf->widthSW < 1)
                cfError(cf,"The width of the sliding window defined in WidthSW must be 1 or larger!");

        if (cf->boolSW && cf->jumpSW < 1)
                cfError(cf,"The jump of the sliding window defined in JumpSW must be 1 or larger!");

        if (cf->boolSW && (cf->windowType < 0 || cf->windowType > 3))
                cfError(cf,"The WindowType parameter must be between 0 and 3!");

        if (cf->refSeq < 1 || cf->refSeq > vf->numSeq)
                cfError(cf,"The reference sequence defined in RefSeq must be a value between 1 and the number of sequences in your alignment file!");

        if (vf->chosenNumSeq < cf->numNuc) 
                cfError(cf,"You have less ingroup sequences chosen in the SeqChoice vector than required by the NumNuc setting! Please choose at least the amount of sequences defined by NumNuc!");

        /*check if things are configured correctly for each runmode*/

        if (cf->runMode == 11) {

                if (vf->numSeq < 2)
                        cfError(cf,"You need at least 2 sequences in your alignment file to run in RunMode 11!");
                
                if (vf->chosenNumSeq < 2) 
                        cfError(cf,"You have less than 2 ingroup sequences chosen in the SeqChoice vector! Please choose 2 or more ingroup seqences!");

                if (!cf->completeDeletion && cf->numNuc < 2) 
                        cfError(cf,"The value of NumNuc is too low! Please set NumNuc to 2 or higher!");
                 
        }

        if (cf->runMode == 12) {

                if (vf->numSeq < 4)
                        cfError(cf,"You need at least 4 sequences in your alignment file to run in RunMode 12!");

                if (vf->chosenNumSeq < 4) 
                        cfError(cf,"You have less than 4 ingroup sequences chosen in the SeqChoice vector! Please choose 4 or more ingroup seqences!");

                if (!cf->completeDeletion && cf->numNuc < 4) 
                        cfError(cf,"The value of NumNuc is too low! Please set NumNuc to 4 or higher!");
                 
        }
        
        if (cf->runMode == 21) {
                
                if (vf->numSeq < 2)
                        cfError(cf,"You need at least 2 sequences in your alignment file to run in RunMode 21!");

                if (vf->chosenNumSeq < 1) 
                        cfError(cf,"You have no ingroup sequences chosen in the SeqChoice vector! Please choose 1 or more ingroup seqences!");
                
                if (!cf->completeDeletion && cf->numNuc < 1) 
                        cfError(cf,"The value of NumNuc is too low! Please set NumNuc to 1 or higher!");
                
                if (vf->chosenNumOut < 1) 
                        cfError(cf,"You don't have any outgroups defined! Make sure you have at least 1 outgroup defined by the Outgroup and SeqChoice vectors!");
        }

        if (cf->runMode == 22) {
                
                if (vf->numSeq < 4)
                        cfError(cf,"You need at least 4 sequences in your alignment file to run in RunMode 22!");

                if (vf->chosenNumSeq < 3) 
                        cfError(cf,"You have less than 3 ingroup sequences chosen in the SeqChoice vector! Please choose 3 or more ingroup seqences!");
                
                if (!cf->completeDeletion && cf->numNuc < 3) 
                        cfError(cf,"The value of NumNuc is too low! Please set NumNuc to 3 or higher!");
                
                if (vf->chosenNumOut < 1) 
                        cfError(cf,"You don't have any outgroups defined! Make sure you have at least 1 outgroup defined by the Outgroup and SeqChoice vectors!");
                
        }
                
        if (cf->runMode == 31) {
                
                if (!cf->completeDeletion) 
                        cfError(cf,"Complete deletion is obligatory when running in RunMode 31! Please set CompleteDeletion to 1!");
                
                if (vf->numSeq < 2)
                        cfError(cf,"You need at least 2 sequences in your alignment file to run in RunMode 11!");

                if (vf->chosenNumSeq < 2) 
                        cfError(cf,"You don't have enough ingroup sequences chosen to perform analysis! Please choose at least 2 ingroup sequences in SeqChoice!");
                
                if (!cf->boolSW)
                        cfError(cf,"RunMode 31 only works with sliding windows! Please set SlidingWindow to 1!");
        }
}


void
initArrays(struct vscanFile *vf, 
           struct analysis *ana)
{
        /*Precalculates arrays needed when calculating statistics.
          Also, the random number generator is seeded here*/

        int i,j,n,k;
        
        /*SH: generate a vector for a1 and a2 values later used for
          Theta, Tajima's D and Fu&Li statistics*/
        
        ana->a1 = needMem((vf->chosenNumSeq + 2) * sizeof(double));
        ana->a2 = needMem((vf->chosenNumSeq + 2) * sizeof(double));

        for (i=0;i<=vf->chosenNumSeq+1;i++) {
                ana->a1[i] = 0;
                ana->a1[i] = 0;
               
                for (j=1;j<i;j++) {
                        ana->a1[i] += 1/(double)j;
                        ana->a2[i] += 1/(double)(j*j);
                }
        }

        /*generate an array with stirling numbers of first kind
          for Fu's Fs*/

        ana->s = needMem((vf->chosenNumSeq+1)*(sizeof(long double *)));
        
        for (i=0;i<=vf->chosenNumSeq;i++) {
                ana->s[i] = needMem((i+2)*(sizeof(long double)));
        }

        /*initial values*/
        ana->s[0][0]=1;
        for (n=1;n<=vf->chosenNumSeq;n++) {
                ana->s[n][0]=0;
                ana->s[n-1][n]=0;
        }
        
        /*calculate stirling numbers*/
        for (n=1;n<=vf->chosenNumSeq;n++) {
                for (k=1;k<=n;k++) {
                        ana->s[n][k] = ana->s[n-1][k-1] - (n-1)*ana->s[n-1][k];
                }
        }

        /*SH: get a seed for the random number generator*/
        seed = -(long)time(NULL);
}


boolean
nextlineFileStartsWith(char *line, 
                       int lineSize, 
                       char *string) 
{
	/* Check the start of the next line without actually getting
	   it. Useful for preview lines that are to be taken later. */
	if (startsWith(string, line + lineSize)) {
		return TRUE;
	} 
	return FALSE;
}

struct dlNode *
dlElementFromIx(struct dlNode *node, 
                int ix) 
{
        /* Return the ix'th element in dlList.  Returns NULL
	 * if no such element. */
	int i;
	for (i=0;i<ix;i++) {
		if (node == NULL) return NULL;
		node = node->next;
	}
	return node;
}

struct vscanBlock *
createEmptyvscanBlock(struct dlList *vscanBlockList)
{
	/* Return a new empty vscanBlock having created the dlLists
         * inside. */
	struct vscanBlock *vb;
	struct vscanBlock *vb_tail;
	AllocVar(vb);
	vb->vscanSeqList = newDlList();
	vb->chosenSeqList = newDlList();
	vb->outgroupList = newDlList();

        vb->start=0;
        vb->end=0;
        vb->len=0;
	
        vb->ref_start=0;
        vb->ref_end=0;
        vb->ref_len=0;

        /*SH: changed the calculation of the id, because
          I create and insert vscanBlocks in readmaf.c
          outside of this function*/
        if (dlEmpty(vscanBlockList)) {
                vb->id = 1;
        }
        else {
                vb_tail = vscanBlockList->tail->val;
                vb->id = vb_tail->id + 1;
        }
        
	vb->node = dlAddValTail(vscanBlockList, vb);
	return vb;
}


struct vscanSeq *
createEmptyvscanSeq(struct dlList *vscanSeqList)
{
        /* Return a new empty vscanSeq */
        struct vscanSeq *tail;
	struct vscanSeq *vs;
	
        AllocVar(vs);

        if (dlEmpty(vscanSeqList)) {
                vs->id = 1;
        }
        
        else {
                tail = vscanSeqList->tail->val;
                vs->id = tail->id + 1;
        }

	vs->node = dlAddValTail(vscanSeqList, vs);
	vs->dySeq = newDyString(0);
        vs->IndivInStretch = FALSE;
	return vs;
}

struct vscanSeq *
createOneSeqvscanSeq(char *string, 
                     struct dlList *vscanSeqList)
{
        /* Return singleton vscanSeq. */
	struct vscanSeq *vs;

        vs = createEmptyvscanSeq(vscanSeqList);
	dyStringAppend(vs->dySeq, string);
	return vs;
}

void
createNumOfSeqsvscanSeq (int num, 
                         struct dlList *vscanSeqList)
{
        /* Create a list of new vscanSeq's inside vscanSeqList */
	struct vscanSeq *vs;
	int i;

	for (i=0;i<num;i++) {
		vs = createEmptyvscanSeq(vscanSeqList);
	}
}

boolean
try_analyse (int state, 
             struct configFile *cf, 
             struct vscanFile *vf, 
             struct vscanPopulation *vpop, 
             struct analysis *ana)
{
	/* Knowing the state of the reading, go to analyse if
	 * pertinent. Return TRUE if state is end_of_file. */
	if (state == 2) {
		/* we have enough readed to go analyse that, then free
		 * the analysed part */
		analyse(cf, vf, vpop, ana);
		return FALSE;
	} else if (state == 0) {
		/* end of file */
		/* Last round of analyses */
		analyse(cf, vf, vpop, ana);
		return TRUE;
	} else if (state == 1) {
		/* we need to read more */
		return FALSE;
	} else {
		vfError(vf, "Unknown value of state in try_analyse");
                return FALSE;
	}
}



void
analyse (struct configFile *cf, 
         struct vscanFile *vf, 
         struct vscanPopulation *vpop, 
         struct analysis *ana) 
{
	static struct dlNode *bbNode=NULL;
        static struct bdfBlock *bb=NULL;
        struct vscanBlock *vb_tail;

        boolean doMore=TRUE;

        /* DEBUG */
        /* printBlocks(vf); */

        /*If the runMode is 51, dump the file and do nothing more*/
        if (cf->runMode == 51) {
                dumpPhylip(vf,vpop);
                freeAnalysedPart(vf, cf, ana, bb);
                return;
        }

        /*SH: if vpa was freed, create a new one*/

        if (vf->vpa == NULL){
                vf->vpa = createEmptyvscanPolyArray(vf, VSCAN_BLOCK_MIN);
        }
        
        /* this is done while reading the file */
        if (vf->format != FORMAT_HAPMAP) {
           chooseSequences(vpop, vf);
           filterPolymorphisms(cf, vf);
        }
        
        /* DEBUG */
        /* printPolyArray(vf); */
        
        vb_tail = vf->vscanBlockList->tail->val;
        
        vf->lastPosInMem = vb_tail->end;
        vf->lastRefPosInMem = vb_tail->ref_end;

        /*SH: get the first BDF block*/
        if (bbNode == NULL) {
                bbNode = vf->bdfBlockList->head;
                bb = bbNode->val;
                printBdfHead(bb,cf);
       }
       
        fprintf(stderr, "#Reading and analysing stretch - pos %lu...\n", vf->lastPosInMem);
        while (doMore) {
                /*SH: if the start of the current BDF block is in memory,
                  start with the analysis*/
                if ((!cf->refPos && bb->start <= vf->lastPosInMem) ||
                    (cf->refPos && bb->ref_start <= vf->lastRefPosInMem)) {
                        calculateBdfBoundaries(bb, vf, cf);
                        checkBdfStart(vf,bb);                        
                        doMore = calculateBdfStretch(vf, cf, bb, ana);
                        if (cf->boolSW) {
                                calculateSlidingWindow(bb,cf,vf,ana);
                        }
                }

                /*SH: if not, read more*/
                else {
                        doMore = FALSE;
                }
                
                if ((!cf->refPos && bb->end <= vf->lastPosInMem) ||
                    (cf->refPos && bb->ref_end <= vf->lastRefPosInMem)) {
                        /*SH: if we have finished reading the BDF block,
                          calculate the statistics for the whole
                          BDF block*/
                        if (cf->runMode != 31) {
                                calculateStatistics(ana->varBdf,cf, ana);
                        }
                        printBdfBlockResults(bb, ana->varBdf, cf);
                        
                        /*SH: if it was the last block, we are done*/
                        if (bbNode == vf->bdfBlockList->tail) {
                                ana->bdfBlocksFinished = TRUE;
                                doMore = FALSE;
                        }
                        
                        /*SH: else goto the next BDF block*/
                        else {
                                bbNode = bbNode->next;
                                bb = bbNode->val;
                                newBdfBlockAnalysis(ana);
                                printBdfHead(bb,cf);
                        }
                }
        }
        
        freeAnalysedPart(vf, cf, ana, bb);
}

void
filterPolymorphisms(struct configFile *cf, 
                    struct vscanFile *vf)
{
	/* Check each position and evaluate to see if there is a
	 * polymorphism or not. Create an array and fill it up with
	 * each polymorphic position */
	struct vscanBlock *vb;
	struct dlNode *vb_node;

        for (vb_node = vf->vscanBlockList->head; !dlEnd(vb_node); vb_node=vb_node->next) {
                vb = vb_node->val;
                
                if (vb->segmentType == VSCAN_ALI) {
                        lookForPolysinAliNode(vf,cf, vb);
                }

                if (vb->segmentType == VSCAN_GAP) {
                        checkGapNode(vf,vb);
                }
        }
}


void
checkGapNode (struct vscanFile *vf, 
              struct vscanBlock *vb)
{
        /*SH: for now, just add all positions to the descarded list*/
        unsigned long pos;
        unsigned long ref_pos;
        unsigned long i;

        pos = vb->start;
        ref_pos = vb->ref_start;
        
        for (i=0; i < vb->len; i++) {
                fillDiscarded(vf, pos, ref_pos);
                pos++;
                ref_pos++;
        }
}

struct vscanPolyArray *
createEmptyvscanPolyArray(struct vscanFile *vf, 
                          unsigned long size)
{
	/* Create a new array for polymorphic columns of "size" number
	 * of positions and "numSeq" nucleotides for each
	 * position. Also create the accompanying vscanPolySummary
	 * vector of size "size" */
	unsigned long i=0;

	AllocVar(vf->vpa);
        
        vf->vpa->tempDisc = 0;
        vf->vpa->tempPoly = 0;
        vf->vpa->tempMonoGap = 0;
        vf->vpa->tempRefGap = 0;
        
        vf->vpa->gapAmb=0;
        vf->vpa->monomorph=0;
        vf->vpa->filled=0;
	vf->vpa->size=size;
	vf->vpa->vps = needMem(size*sizeof(struct vscanPolySummary));
	
        vf->vpa->polyArray = needMem(size*sizeof(char *));
	do {
		vf->vpa->polyArray[i]=needLargeZeroedMem((vf->chosenNumSeq+vf->chosenNumOut+1)*sizeof(char));
		i++;
	} while (i<size);


        /*SH: because we need to know which sites have been discarded in the analysis part,
          the sites are stored in a simple array. It's similar to polyArray*/
        vf->vpa->sizeDisc=size;
        vf->vpa->filledDisc=0;
        vf->vpa->listDisc = needMem(size*sizeof(struct pos));

        /*SH: in this list we store the sites that have gaps in the reference sequence
         this allows us to keep track of the ref_pos for each column of the alignment*/
        vf->vpa->sizeRefGap=size;
        vf->vpa->filledRefGap=0;
        vf->vpa->listRefGap = needMem(size*sizeof(struct pos));

        /*SH: for pairwise deletion we also need to know the monomorphic sites with gaps.
          These are stored in the listMonoGaps*/
        vf->vpa->sizeMonoGap=size;
        vf->vpa->filledMonoGap=0;
        vf->vpa->listMonoGap = needMem(size*sizeof(struct monoGap));

	return vf->vpa;
}

void
fillDiscarded(struct vscanFile *vf, 
              unsigned long position, 
              unsigned long ref_position)
{
        /*SH: fill the listDisc with the discarded sites*/

        if (vf->vpa->filledDisc == vf->vpa->sizeDisc) {
                unsigned long newSizeDisc = (vf->vpa->sizeDisc) + (vf->vpa->sizeDisc);
                vf->vpa->listDisc = needMoreMem(vf->vpa->listDisc, 
                                                (vf->vpa->sizeDisc)*sizeof(struct pos), 
                                                newSizeDisc*sizeof(struct pos));
                vf->vpa->sizeDisc = newSizeDisc;
        }
        
        vf->vpa->listDisc[vf->vpa->filledDisc].position = position;
        vf->vpa->listDisc[vf->vpa->filledDisc].ref_position = ref_position;
        vf->vpa->filledDisc++;
}

void
fillReferenceGap(struct vscanFile *vf, 
                 unsigned long position, 
                 unsigned long ref_position)
{
        /*SH: fill the listRefGap with the sites*/

        if (vf->vpa->filledRefGap == vf->vpa->sizeRefGap) {
                unsigned long newSizeRefGap = (vf->vpa->sizeRefGap) + (vf->vpa->sizeRefGap);
                vf->vpa->listRefGap = needMoreMem(vf->vpa->listRefGap, 
                                                (vf->vpa->sizeRefGap)*sizeof(struct pos), 
                                                newSizeRefGap*sizeof(struct pos));
                vf->vpa->sizeRefGap = newSizeRefGap;
        }
        
        vf->vpa->listRefGap[vf->vpa->filledRefGap].position = position;
        vf->vpa->listRefGap[vf->vpa->filledRefGap].ref_position = ref_position;
        vf->vpa->filledRefGap++;
}

void
fillMonomorphicGap(struct vscanFile *vf, 
                   unsigned long position, 
                   unsigned long ref_position, 
                   int valid)
{
        /*SH: fill the listMonoGap with the gapped, monomorphic sites*/

        if (vf->vpa->filledMonoGap == vf->vpa->sizeMonoGap) {
                unsigned long newSizeMonoGap = (vf->vpa->sizeMonoGap) + (vf->vpa->sizeMonoGap);
                vf->vpa->listMonoGap = needMoreMem(vf->vpa->listMonoGap, 
                                                   vf->vpa->sizeMonoGap*sizeof(struct monoGap), 
                                                   newSizeMonoGap*sizeof(struct monoGap));
                                                   
                vf->vpa->sizeMonoGap = newSizeMonoGap;
        }
        
        vf->vpa->listMonoGap[vf->vpa->filledMonoGap].position = position;
        vf->vpa->listMonoGap[vf->vpa->filledMonoGap].ref_position = ref_position;
        vf->vpa->listMonoGap[vf->vpa->filledMonoGap].valid = valid;
                
        vf->vpa->filledMonoGap++;
}

void
fillPolyColumn(struct vscanFile *vf, 
               char *column, 
               unsigned long position, 
               unsigned long ref_position,
               struct baseCount *bc)
{
	/* Fill the polyArray with a polymorphic column, and the
	 * precalculated frequency of each caracter. */
	unsigned long i;
        
	if (((vf->vpa->filled)) == (vf->vpa->size)) {
		unsigned long newSize = (vf->vpa->size) + (vf->vpa->size);
		vf->vpa->vps = needMoreMem( vf->vpa->vps, 
					    (vf->vpa->size)*sizeof(struct vscanPolySummary), 
					    newSize*sizeof(struct vscanPolySummary) );
		vf->vpa->polyArray = needMoreMem( vf->vpa->polyArray,
						  (vf->vpa->size)*sizeof(char *),
						  newSize*sizeof(char *));
		vf->vpa->size = newSize;
		i = vf->vpa->filled;
		do {
                        vf->vpa->polyArray[i]=needLargeZeroedMem((vf->chosenNumSeq+vf->chosenNumOut+1)*sizeof(char));
			i++;
		} while (i<newSize);
	}
	memcpy((vf->vpa->polyArray[vf->vpa->filled]), column, strlen(column));
	vf->vpa->vps[vf->vpa->filled].position = position;
	vf->vpa->vps[vf->vpa->filled].ref_position = ref_position;
	vf->vpa->vps[vf->vpa->filled].A  = bc->A;
	vf->vpa->vps[vf->vpa->filled].C  = bc->C;
	vf->vpa->vps[vf->vpa->filled].G  = bc->G;
	vf->vpa->vps[vf->vpa->filled].T  = bc->T;
	vf->vpa->vps[vf->vpa->filled].Gap= bc->Gap;
	vf->vpa->vps[vf->vpa->filled].Amb= bc->Amb;
	vf->vpa->vps[vf->vpa->filled].valid= bc->valid;
        
	vf->vpa->filled++;
}


void
lookForPolysinAliNode (struct vscanFile *vf, 
                       struct configFile *cf, 
                       struct vscanBlock *vb) 
{
        /* Read each column of an Ali and look for if it's
         * polymorphic */
	
        struct baseCount *bc;

	char reference_base = '_';

        char *posRefString;
        char *nucRefString;

	unsigned long ul = 0;
        int ref_gaps = 0;

        unsigned long pos;
        unsigned long ref_pos;

	struct dlNode *vs_node;
	struct vscanSeq *vs;
        struct dlNode *vo_node;
        struct vscanOutgroup *vo;
        
	int j;
	char *polyColumn;

        bc = needMem(sizeof(struct baseCount));
        resetBaseCount(bc);

        polyColumn = needMem((vf->chosenNumSeq + vf->chosenNumOut + 1)*sizeof(char));
        polyColumn[vf->chosenNumSeq + vf->chosenNumOut] = '\0';

        /*SH: We have to deal with 2 reference sequences here: One is reference 
          sequence for the bases, meaning the FIRST sequence in the paragraph,
          which defines the bases for any '.'s in the other sequences.
          The other reference sequence is the sequence that defines the reference
          position, used in the BDF blocks. These two sequences may be the same, 
          but this does not have to be the case*/

        vs_node = vb->vscanSeqList->head;
        vs = vs_node->val;

        nucRefString = vs->dySeq->string;

        vs_node = dlElementFromIx(vb->vscanSeqList->head, cf->refSeq - 1);
        vs = vs_node->val;

        posRefString = vs->dySeq->string;

        
        /*SH: if a specific site is to be saved, and the ref sequence contains a gap, 
          then the LAST valid ref_position is assigned to that site.
          If the ref_len for this specific block is 0, then we have to adjust the
          ref_gaps corrector, or the assigned ref_position will be wrong*/

        if (vb->ref_len == 0) {
                ref_gaps--;
        }
	
	for (ul = 0; ul < vb->len; ul++) {
                /*SH: now work the way through the alignment. First, take care of 
                  the stuff that has to be done for the reference sequences*/

                reference_base = nucRefString[ul];

                pos = ul + vb->start;

                if (posRefString[ul] == '-') {
                        ref_gaps++;
                        ref_pos = ul + vb->ref_start - ref_gaps;
                        fillReferenceGap(vf, pos, ref_pos);
                }
                else {
                        ref_pos = ul + vb->ref_start - ref_gaps;
                }       
                
                /*SH: now loop through the chosen ingroup sequences, and look for
                  polymorphic sites*/
		vs_node = vb->chosenSeqList->head;
		vs = vs_node->val;
			
                for (j=0;j<vf->chosenNumSeq;j++) {
                        
                        polyColumn[j] = countBase(vs->dySeq->string[ul], reference_base, bc);
 
                        if (j != (vf->chosenNumSeq)-1) {
                                vs_node = vs_node->next;
                                vs = vs_node->val;
                        }
                }

                /*SH: if we we are in runMode 21 or 22, read the outgroups.
                In case of runMode 31 it might also be necessary to read 
                outgroups. Do so, if any are defined*/
                if (cf->runMode == 21 || 
                cf->runMode == 22 ||
                (cf->runMode == 31 && vf->chosenNumOut > 0)) {
                        vo_node = vb->outgroupList->head;
                        vo = vo_node->val;

                        for (j=0;j<vf->chosenNumOut;j++) {
                                
                                polyColumn[vf->chosenNumSeq + j] = toupper(vo->vos->dySeq->string[ul]);
                                
/*                                 if (j != (vf->chosenNumOut)-1) { */
/*                                         vo_node = vo_node->next; */
/*                                         vo = vo_node->val; */
/*                                 } */
                                if (j == (vf->chosenNumOut)-1)
                                        continue;
                                vo_node = vo_node->next;
                                vo = vo_node->val;
                        }
                }
                
                if ((cf->runMode > 10 && cf->runMode < 20) || cf->runMode == 31) {
                        checkColumnsIngroup(vf,cf,bc,pos,ref_pos,polyColumn);
                }
                else if (cf->runMode == 21) {
                        checkColumnsRunMode21(vf,cf,bc,pos,ref_pos,polyColumn);
                }
                else if (cf->runMode == 22) {
                        checkColumnsRunMode22(vf,cf,bc,pos,ref_pos,polyColumn);
                }
                
                resetBaseCount(bc);
        }
        
        freeMem(polyColumn);
        freeMem(bc);
}

void
checkColumnsIngroup (struct vscanFile *vf, 
                     struct configFile *cf,
                     struct baseCount *bc,
                     unsigned long pos,
                     unsigned long ref_pos,
                     char *polyColumn)
{
        int invalidBases;
        char monoChar;

        bc->valid = bc->A + bc->C + bc->G + bc->T;
        invalidBases = bc->Gap + bc->Amb;

        if (bc->valid < cf->numNuc) {
                /* not enough bases for analysis, so discard*/
                fillDiscarded(vf,pos,ref_pos);
                return;
        }
        
        monoChar = checkMonomorph(bc);
        
        if (monoChar != 'N') {
                /*we have a monomorphic site, so we don't need
                  the column*/
                if (!cf->fixNum && invalidBases > 0) {
                        /* but there are gaps and/or ambs in the column
                           we have to store if we use fixNum=0*/
                        fillMonomorphicGap(vf,pos,ref_pos,bc->valid);
                }
        }

        else {
                /*we have a polymorphic site*/
                if (cf->fixNum && bc->valid > cf->numNuc) {
                        /*we want to use just a fixed number of bases,
                          but we have too many. Randomly pick the
                          right amount befor saving the column*/
                        randomChooseBases(bc,cf->numNuc);
                        /*check if the column is still polymorphic
                          after choosing bases*/
                        monoChar = checkMonomorph(bc);
                }
                        
                if (monoChar == 'N') {
                        fillPolyColumn(vf, polyColumn, pos, ref_pos, bc);
                }
        }
}

void
checkColumnsRunMode22 (struct vscanFile *vf, 
                       struct configFile *cf,
                       struct baseCount *bc,
                       unsigned long pos,
                       unsigned long ref_pos,
                       char *polyColumn)
{
        char monoChar;
        short invalidBases;
        
        bc->valid = bc->A + bc->C + bc->G + bc->T;
        invalidBases = bc->Gap + bc->Amb;

        if (bc->valid < cf->numNuc) {
                /* not enough bases for analysis, so discard*/
                fillDiscarded(vf,pos,ref_pos);
                return;
        }
        
        monoChar = checkMonomorph(bc);
        
        if (monoChar == 'N') {
                /*we have a polymorphic site, check if the outgroup
                 is informative, if not discard the site*/
                if (!validBase(polyColumn[vf->chosenNumSeq])) {
                        /*the outgroup is a gap or amb, so we can't
                          polarize mutations*/
                        fillDiscarded(vf,pos,ref_pos);
                        return;
                }

                else {
                        /*the outgroup is a valid base, so randomly
                          choose ingroup bases (if necessary), then 
                          check if we are able to polarize mutations,
                          if not the column will be discarded*/
                        if (cf->fixNum && bc->valid > cf->numNuc) {
                                randomChooseBases(bc,cf->numNuc);
                                monoChar = checkMonomorph(bc);
                        }
                        
                        if (monoChar == 'N') {
                                /*the ingroup is still polymorphic after
                                  choosing columns*/
                                checkPolarize(vf,polyColumn,pos,ref_pos,bc);
                        }
                }
        }
        
        else if (!cf->fixNum && invalidBases > 0) {
                /*ingroup is monomorphic, but there are gaps 
                  and/or ambs in the ingroup. Save if we
                  are not using a fixed Number of nucleotides*/
                fillMonomorphicGap(vf,pos,ref_pos,bc->valid);
        }
} 

void
checkPolarize (struct vscanFile *vf,
               char *polyColumn,
               unsigned long pos,
               unsigned long ref_pos,
               struct baseCount *bc)
{
        if (polyColumn[vf->chosenNumSeq] == 'A') {
                if (bc->A > 0) {
                        fillPolyColumn(vf,polyColumn,pos,ref_pos,bc);
                }
                else {
                        fillDiscarded(vf,pos,ref_pos);
                }
        }

        else if (polyColumn[vf->chosenNumSeq] == 'C') {
                if (bc->C > 0) {
                        fillPolyColumn(vf,polyColumn,pos,ref_pos,bc);
                }
                else {
                        fillDiscarded(vf,pos,ref_pos);
                }
        }

        else if (polyColumn[vf->chosenNumSeq] == 'G') {
                if (bc->G > 0) {
                        fillPolyColumn(vf,polyColumn,pos,ref_pos,bc);
                }
                else {
                        fillDiscarded(vf,pos,ref_pos);
                }
        }


        else if (polyColumn[vf->chosenNumSeq] == 'T') {
                if (bc->T > 0) {
                        fillPolyColumn(vf,polyColumn,pos,ref_pos,bc);
                }
                else {
                        fillDiscarded(vf,pos,ref_pos);
                }
        }

        else {
                vfError(vf,"Unknown base in checkPolarize!");
        }
}

void
checkColumnsRunMode21 (struct vscanFile *vf, 
                       struct configFile *cf,
                       struct baseCount *bc,
                       unsigned long pos,
                       unsigned long ref_pos,
                       char *polyColumn)
{
        char monoChar;

        bc->valid = bc->A + bc->C + bc->G + bc->T;

        if (bc->valid < cf->numNuc) {
                /* not enough bases for analysis, so discard*/
                fillDiscarded(vf,pos,ref_pos);
                return;
        }
        
        monoChar = checkMonomorph(bc);
        
        if (monoChar != 'N') {
                /*we have a monomorphic site, but we might want to save it for
                  RunMode 21*/
                checkMonoWithOutgroup(vf,cf,bc,
                                      pos,ref_pos,
                                      polyColumn,monoChar);
        }
        
        else {
                /*we have a polymorphic site*/
                if (validBase(polyColumn[vf->chosenNumSeq])) {
                        /*the outgroup is valid, so store this column*/
                        if (cf->fixNum && bc->valid > cf->numNuc) {
                                randomChooseBases(bc,cf->numNuc);
                                monoChar = checkMonomorph(bc);
                        }
                        
                        if (monoChar == 'N') {
                                /*the ingroup is still polymorphic after
                                  choosing columns*/
                                fillPolyColumn(vf,polyColumn,pos,ref_pos,bc);
                        }
                        else {
                                /*the ingroup is monomorphic now, but the 
                                  outgroup might still differ, so check that*/
                                checkMonoWithOutgroup(vf,cf,bc,
                                                      pos,ref_pos,
                                                      polyColumn,monoChar);
                        }
                }
                
                else {
                        /*the outgroup is an invalid base, so 
                          discard this site*/
                        fillDiscarded(vf,pos,ref_pos);
                }
        }
}

void checkMonoWithOutgroup (struct vscanFile *vf, 
                            struct configFile *cf,
                            struct baseCount *bc,
                            unsigned long pos,
                            unsigned long ref_pos,
                            char *polyColumn,
                            char monoChar)
{
        int invalidBases;

        invalidBases = bc->Gap + bc->Amb;
        
        if (polyColumn[vf->chosenNumSeq] != monoChar) {
                /*the outgroup differs from the ingroup,
                  so this column will now be treated as a
                  polymorphic column*/
                if (validBase(polyColumn[vf->chosenNumSeq])) {
                        /*we have a valid outgroup, so save
                          the column, and make sure we have the
                          right amount of bases if fixNum=1*/
                        if (cf->fixNum && bc->valid > cf->numNuc) {
                                trimBases(bc,cf->numNuc);
                        }
                        fillPolyColumn(vf,polyColumn,pos,ref_pos,bc);
                }
                else {
                        /*the outgroup base is invalid, so 
                          discard the site*/
                        fillDiscarded(vf,pos,ref_pos);
                }
                
        }
        else if (!cf->fixNum && invalidBases > 0) {
                /*everything is monomorphic, but there are gaps 
                  and/or ambs in the ingroup*/
                fillMonomorphicGap(vf,pos,ref_pos,bc->valid);
        }
        
}


void trimBases(struct baseCount *bc, int fixedNum)
{
        /*Sets a monomorphic column to the number of bases
          defined by fixedNum*/

        if (bc->A > 0) bc->A = fixedNum;
        else if (bc->C > 0) bc->C = fixedNum;
        else if (bc->G > 0) bc->G = fixedNum;
        else  bc->T = fixedNum;

        bc->valid = fixedNum;
}
                               
void randomChooseBases (struct baseCount *bc, int fixedNum)
{
        /*ramdomly removes bases from a polymorphic column,
          until the number defined by fixedNum is reached*/
        
        float boundA;
        float boundC;
        float boundG;
        float num; 

        while (bc->valid > fixedNum) {

                boundA = (float) bc->A / bc->valid;
                boundC = (float) bc->C / bc->valid + boundA;
                boundG = (float) bc->G / bc->valid + boundC;
                
                num = ran1(&seed);
                
                if (num <= boundA && bc->A > 0)      bc->A--;
                else if (num <= boundC && bc->C > 0) bc->C--;
                else if (num <= boundG && bc->G > 0) bc->G--;
                else if (num <= 1 && bc->T > 0)      bc->T--;
                else errAbort ("Invalid random number: %f!",num);

                bc->valid--;
        }
}

char
checkMonomorph (struct baseCount *bc)
{
        /*checks if a column is monomorph*/

        if      (bc->A == bc->valid) return 'A';
        else if (bc->C == bc->valid) return 'C';
        else if (bc->G == bc->valid) return 'G';
        else if (bc->T == bc->valid) return 'T';
        else                         return 'N';
}
                
boolean
validBase (char base)
{
        /*checks if base is A,C,G or T*/

        if (base == 'A' ||
            base == 'C' ||
            base == 'G' ||
            base == 'T') {
                return TRUE;
        }
        else {
                return FALSE;
        }
}

char
countBase (char base, 
           char reference_base, 
           struct baseCount *bc)
{
	/* Count which nucleotide we have at a specific position for a
	   specific sequence. Take into account reference_base when
	   '.' is found. Return the base. */

        if (base == '.') {
                base = reference_base;
        }
        
	if (base == '-')                       {(bc->Gap)++;return base;
	} else if (base == 'a' || base == 'A') {(bc->A)++;  return toupper(base);
	} else if (base == 'c' || base == 'C') {(bc->C)++;  return toupper(base);
	} else if (base == 'g' || base == 'G') {(bc->G)++;  return toupper(base);
	} else if (base == 't' || base == 'T') {(bc->T)++;  return toupper(base);
	} else {                                (bc->Amb)++;return base;
	}
}

void
resetBaseCount (struct baseCount *bc)
{
        bc->A = 0;
        bc->C = 0;
        bc->G = 0;
        bc->T = 0;
        bc->Gap = 0;
        bc->Amb = 0;
}

int
fastBlockListCount(struct dlList *vscanBlockList)
{
        struct vscanBlock *vb_head;
        struct vscanBlock *vb_tail;

        vb_head = vscanBlockList->head->val;
        vb_tail = vscanBlockList->tail->val;

        return vb_tail->id - vb_head->id + 1;
}

char *
skipToWord(char *line, 
           int n)
{
        /*SH: skips n-1 words in the current line, returns pointer to the
          n-th word. Will NOT zero-terminate the n-th word, so whitespace
          behind the pointer will be part of the word*/

        int i;
        char *word;
        
        word = skipLeadingSpaces(line);
        for (i=0;i<n-1;i++) {
                word = skipToSpaces(word);
                word = skipLeadingSpaces(word);
        }
        return word;
}

void
createBdfBlock(int refPos, 
               unsigned long start, 
               unsigned long end, 
               char *feature, 
               struct dlList *list)
{
        static int bdfBlockId;
        struct bdfBlock *bb;

        AllocVar(bb);

        bb->id = ++bdfBlockId;

        if (refPos) {
                bb->ref_start = start;
                bb->ref_end = end;
                bb->start = 0;
                bb->end = 0;
                
        }
        else {
                bb->start = start;
                bb->end = end;
                bb->ref_start = 0;
                bb->ref_end = 0;
        }
        
        bb->feature = feature;
        bb->node = dlAddValTail(list, bb);
}
      

void
loadBlockDataFile(struct configFile *cf, 
                  struct vscanFile *vf)
{
        /*SH: Read the blockdata file, and store its' contents into
         * the structure*/
        
        struct lineFile *lf;
        char *line;
        int lineSize;

        char *words[2];
        int wordCount;

        char *feature = NULL;
        char configFeature[23];

        unsigned long start;
        unsigned long end;

        if (cf->bdfBlockFile == NULL) {
                if (cf->endPosition == 0) {
                        cf->endPosition = ULONG_MAX;
                }
                sprintf(configFeature,"Defined by config file");
                feature = cloneString(configFeature);
                createBdfBlock(cf->refPos,cf->startPosition,cf->endPosition,feature,vf->bdfBlockList);
        }
        
        else {
                lf = lineFileOpen(cf->bdfBlockFile, TRUE);
                
                while (lineFileNext(lf, &line, &lineSize)) {
                        feature = skipToWord(line,3);
                        feature = cloneString(feature);
                        
                        wordCount = chopLine(line, words);
                        start = atol(words[0]);
                        end = atol(words[1]);
                        
                        createBdfBlock(cf->refPos,start,end,feature,vf->bdfBlockList);
                }
                
                lineFileClose(&lf);
        }
}


struct configFile *
loadConfigFile(struct vscanFile *vf, 
               struct vscanPopulation *vpop, 
               char *fileName) 
{
	/* Loads a config file and returns the configFile options */
	struct configFile *cf;
	struct lineFile *lf;
	char *line;
	char *cloneline;
        char *realline;
	int lineSize;

	char *words[255];
	int wordCount;

	char *strOpts[]={"IndivNames", "RunMode", "UseMuts",
                         "CompleteDeletion", "FixNum", "NumNuc", 
                         "SlidingWindow","WidthSW", "JumpSW","WindowType",
			 "StartPos","EndPos","RefPos", "BlockDataFile",
			 "SeqChoice","Outgroup", "RefSeq", "UseLDSinglets"
        };
	int numOpts = 18; int fullOpts = 0;
        int param;

 	/* copy the name of the file in configFile */
	AllocVar(cf);
	/* file open */
	cf->fileName = cloneString(fileName);
	cf->lf = lineFileOpen(fileName, TRUE);
	lf = cf->lf;

	while (lineFileNext(lf, &line, &lineSize)) {
                cloneline = cloneString(line);
                realline = skipLeadingSpaces(cloneline);

                if (strlen(realline) == 0) {
                        /* SH: empty line*/
                        freeMem(cloneline);
                        continue;
                               }
                if (startsWith("#",realline)) {
                        /*SH: a comment*/
                        freeMem(cloneline);
                        continue;
                }
                        
		wordCount = chopLine(cloneline, words);
                param = getParameter(words[0], strOpts, numOpts);
                writeParameter(cf,vf,vpop,words,wordCount,param);
                fullOpts++;
                freeMem(cloneline);
        }
        
        if (fullOpts != numOpts) {
                cfError(cf,"Missing parameter in config file!");
        }

        lineFileClose(&lf);
        return cf;
}

int
getParameter (char *word, 
              char **strOpts, 
              int numOpts)
{
        int i;
        
        for (i=0;i<numOpts;i++) {
                if (!sameWord(strOpts[i], word))
                        continue;
                return i;
        }

        errAbort ("Unknown parameter in config file: %s",word);
}

void
writeParameter (struct configFile *cf, 
                struct vscanFile *vf, 
                struct vscanPopulation *vpop, 
                char *words[], 
                int wordCount,
                int param)
{

        int i;
        struct dlNode *node;
        struct vscanIndividual *vindiv;

        switch (param) {
                
        /* IndivNames */
        case 0: 
                if ((vf->format == FORMAT_MAF) || (vf->format == FORMAT_MAV)) {
                        /*SH: only do this if we have a MAF or MAV file */
                        vf->numSeq = wordCount - 2;
                        if (vf->numSeq == 0)
                           cfError(cf,"No individuals defined in IndivNames!");
                        
                        createNumberOfIndividuals(vpop,vf->numSeq);
                        
                        for (i=0; i<(vf->numSeq); i++) {
                                node = dlElementFromIx(vpop->vindivList->head,i);
                                vindiv = node->val;
                                vindiv->name = cloneString(words[2+i]);
                        }
                }
                break;
                        
        /* "RunMode" */
        case 1: 
                cf->runMode = atoi(words[2]);
                break;

        /* "UseMuts" */
        case 2: 
                cf->useMuts = determineConfigBoolean(words[2],cf);
                break;

        /* CompleteDeletion */
        case 3: 
                cf->completeDeletion = determineConfigBoolean(words[2],cf);
                break;

        /* FixNum */
        case 4: 
                cf->fixNum = determineConfigBoolean(words[2],cf);
                break;

        /* NumNuc */
        case 5: 
                cf->numNuc = atoi(words[2]);
                break;

        /* SlidingWindow */
        case 6: 
                cf->boolSW = determineConfigBoolean(words[2],cf);
                break;

        /* WidthSW */
        case 7: 
                cf->widthSW = atol(words[2]);
                break;

        /* JumpSW */
        case 8: 
                cf->jumpSW = atol(words[2]);
                break;
                
        /* WindowType */
        case 9:
                cf->windowType = atoi(words[2]);
                break;

        /* StartPos */
        case 10: 
                cf->startPosition = atol(words[2]);
                break;

        /* EndPos */
        case 11:
                cf->endPosition = atol(words[2]);
                break;

        /* "RefPos" */
        case 12:
                cf->refPos = determineConfigBoolean(words[2],cf);
                break;

        /* "BlockDataFile" */
        case 13:
                if ((wordCount == 2) || sameString(words[2],"none")) cf->bdfBlockFile = NULL;
                else cf->bdfBlockFile = cloneString(words[2]);
                break;

        /* "SeqChoice" */
        case 14: 
                /* case where we dont know how many sequences do we have yet */
                if (vf->numSeq == -1) {
                        char dummyname[23] = "";
                        vf->numSeq = wordCount - 2;
                        createNumberOfIndividuals(vpop,vf->numSeq);
                        
                        for (i=0; i<(vf->numSeq); i++) {
                                node = dlElementFromIx(vpop->vindivList->head,i);
                                vindiv = node->val;
                                sprintf(dummyname,"%d",(i+1));
                                vindiv->name = cloneString(dummyname);
                        }
                } 
                /* case where we know how many sequences we have */
                if (sameWord(words[2],"all")) {
                        /*shortcut for selecting all individuals*/
                        for (node=vpop->vindivList->head;!dlEnd(node);node=node->next) {
                                vindiv = node->val;
                                vindiv->seqChoice = 1;
                        }
                        break;
                }
                if (wordCount != (vf->numSeq)+2) {
                        cfError(cf,"err: config file. unexpected number of values for SeqChoice");
                }
                
                for (i=0; i<(vf->numSeq); i++) {
                        if (!sameString(words[2+i],"1") && !sameString(words[2+i],"0")) {
                                cfError(cf,"err: config file. only 0 and 1 are valid entries for SeqChoice");
                        }
                        
                        node = dlElementFromIx(vpop->vindivList->head,i);
                        vindiv = node->val;
                                                
                        vindiv->seqChoice = atoi(words[2+i]);
                }
                break;

        /* "Outgroup" */
        case 15:
                /*shortcut for selecting no outgroups*/
                if (sameWord(words[2],"none")) {
                        for (node=vpop->vindivList->head;!dlEnd(node);node=node->next) {
                                vindiv = node->val;
                                vindiv->outgroup = 0;
                        }
                        break;
                }

                /*shortcut for selecting the first individual to be the only outgroup*/
                if (sameWord(words[2],"first")) {
                        for (node=vpop->vindivList->head;!dlEnd(node);node=node->next) {
                                vindiv = node->val;
                                if (node == vpop->vindivList->head) {
                                        vindiv->outgroup = 1;
                                }
                                else {
                                        vindiv->outgroup = 0;
                                }
                        }
                        break;
                }

                /*shortcut for selecting the last individual to be the only outgroup*/
                if (sameWord(words[2],"last")) {
                        for (node=vpop->vindivList->head;!dlEnd(node);node=node->next) {
                                vindiv = node->val;
                                if (node == vpop->vindivList->tail) {
                                        vindiv->outgroup = 1;
                                }
                                else {
                                        vindiv->outgroup = 0;
                                }
                        }
                        break;
                }

                if (wordCount != (vf->numSeq)+2) {
                        cfError(cf,"err: config file. unexpected number of parameters for Outgroup");
                }
                
                for (i=0; i<(vf->numSeq); i++) {
                        node = dlElementFromIx(vpop->vindivList->head,i);
                        vindiv = node->val;
                        vindiv->outgroup = atoi(words[2+i]);
                }
                break;

        /*  "RefSeq" */
        case 16:
                cf->refSeq = atoi(words[2]);
                break;
        
        /* UseLDSinglets */
        case 17:
                cf->useLDSinglets = determineConfigBoolean(words[2],cf);
                break;
        }
}

boolean
determineConfigBoolean(char *c, 
                       struct configFile *cf) 
{
	/* Determine if the boolean option in config file is 1 or 0. Call error in either case. */
	boolean b = 0;
	if (sameString(c,"1")) {
		b = 1;
	}
	else if (sameString(c,"0")) {
		b = 0;
	}
	else {
		cfError(cf,"Error while determining if option is 1 or 0");
                return b;
	}
	return b;
}

void vfError(struct vscanFile *vf, char *format, ...)
/* Output an error message with filename and line number included. */
{
        va_list args;
        va_start(args, format);
        vaWarn(format, args);
        errAbort("Line %d of %s", vf->lf->lineIx, vf->fileName);
        va_end(args);
}

void cfError(struct configFile *cf, char *format, ...)
/* Output an error message with filename and line number included. */
{
        va_list args;
        va_start(args, format);
        vaWarn(format, args);
        errAbort("Line %d of %s", cf->lf->lineIx, cf->fileName);
        va_end(args);
}

/* static void */
/* cfError(struct configFile *cf,  */
/*         char *message)  */
/* { */
/* 	/\* Print config file error message. *\/ */
/* 	errAbort("%s\nLine %d of %s", message,  */
/* 		 cf->lf->lineIx, cf->fileName); */
/* } */

/* static void */
/* vfWarn(struct vscanFile *vf, char *message) */
/* /\* Print vscan file warn message. *\/ */
/* { */
/* 	printf("%s\nLine %d of %s", message,  */
/* 	       vf->lf->lineIx, vf->fileName); */
/* } */

/* static void */
/* vfSyntax(struct vscanFile *vf)  */
/* { */
/* 	/\* General syntax error message. *\/ */
/* 	vfError(vf, "Can't cope with file syntax"); */
/* } */

void
usage() 
{
	/* Explain usage and exit. */
	errAbort(
		"-------------------------------------------------------\n"
		"Variscan 2.0 - Nucleotide Variability for huge DNA sets\n"
		"-------------------------------------------------------\n"
		"          For mor information please visit:\n"
		"          http://www.ub.es/softevol/variscan\n"
		"\n"
		"          Usage:\n"
		"          ./variscan sequence_file config_file\n"
		"-------------------------------------------------------\n");
}

int
main(int argc, char *argv[]) 
{
	if (argc!=3) usage();
	else variscan (argv[1],argv[2]);
  
	return 0;
}

#endif /* VARISCAN */

