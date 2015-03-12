#include "readmga.h"

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


/* static void  */
/* vfError(struct vscanFile *vf, char *message)  */
/* { */
/* 	/\* Print vscan file error message. *\/ */
/* 	errAbort("%s\nLine %d of %s", message,  */
/* 		 vf->lf->lineIx, vf->fileName); */
/* } */

int
mgaNextSegment (struct vscanFile *vf)
{
        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;
        
        char *blankline;
        int lenLine=0;

        char *initline;
        char *initwords[256];
        int initCount;

        char *cloneline;
        char *words[256];
        int wordCount;

        char *start;

        struct vscanBlock *vb;
        struct vscanBlock *vb_prev;

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
        

        /*read the initial line line of the paragraph*/

        initline = cloneString(line);
	initCount = chopLine(initline, initwords);

        /*check for the paragraph type*/

        if (startsWith("Seq",initwords[0])) {
                /*we are in the middle of an ALI*/
                
                vb = createEmptyvscanBlock(vf->vscanBlockList);

                vb->start = checkvscanBlockStart(vb);
                if (vb->node->prev) {
                        vb_prev = vb->node->prev->val;
                        vb->ref_start = vb_prev->ref_end + 1;
                } else {
                        vb->ref_start = 0;
                }
                lineFileReuse(lf);
                readMgaAli(vf, vb);

                vf->accum_filled += vb->len;
        }
        
        else {
                /*read the next line*/
                
                lineFileNext(lf,&line,&lineSize);
                
                cloneline = cloneString(line);
                wordCount = chopLine(cloneline,words);
                
                if (startsWith("Exact",words[0])) {
                        /*we have an EXA*/
                        vb = createEmptyvscanBlock(vf->vscanBlockList);
                        vb->ref_len = atol(initwords[0]);
                        
                        if (vb->ref_len == 1) {
                                /*we have to take care of the "!" character*/
                                *(initwords[1])++;
                        }
                        
                        vb->ref_start = atol(initwords[1]) + 1;
                        vb->ref_end = vb->ref_start + vb->ref_len - 1;
                        
                        vb->start = checkvscanBlockStart(vb);

                        vb->len = vb->ref_len;
                        vb->end = vb->start + vb->len - 1;

                        vb->segmentType = MGA_EXA;

                        skipUnneededExas(vf);
                }       
                
                else if (startsWith("Seq",words[0])) {
                        /*we have the start of an ALI*/
                        vb = createEmptyvscanBlock(vf->vscanBlockList);
                                
                        if (sameWord("-",initwords[0])) {
                                /*if the ref sequence only contains gaps,
                                  ref_start and ref_end are set to the
                                  LAST valid nucleotide. Here we set it
                                  to NEXT valid nucleotide, but this will
                                  be corrected in readMgaAli*/
                                vb_prev = vb->node->prev->val;
                                vb->ref_start = vb_prev->ref_end + 1;
                        }
                        
                        else {
                                if (startsWith("!",initwords[0])) {
                                        /*we have a SNP*/
                                        start = initwords[0];
                                }
                                else {
                                        /*we have a normal ALI*/
                                        start = strchr(initwords[0],':');
                                }
                                *(start)++;
                                vb->ref_start = atol(start) + 1;
                        }

                        vb->start = checkvscanBlockStart(vb);
                        
                        lineFileReuse(lf);
                        readMgaAli(vf, vb);

                        vf->accum_filled += vb->len;
                }

                else if (startsWith("Gap",words[0])) {
                        /*we have a GAP*/
                        vb = createEmptyvscanBlock(vf->vscanBlockList);
                        annotateMgaGap(vb, initwords[0]);
                        
                } else {
                        vfError(vf,"Non paragraph type in MGA");
                }
        }
       
        
        if (vf->accum_filled >= VSCAN_RAMBUFFER_MAX) {
                return 2;
        }
        else {
                return 1;
        }

}


unsigned long
checkvscanBlockStart (struct vscanBlock *vb)
{
        struct vscanBlock *vb_prev;
        unsigned long start;

        if (vb->id == 1) {
                start = vb->ref_start;
        }

        else {
                vb_prev = vb->node->prev->val;
                start = vb_prev->end + 1;
        }
        
        return start;
}


void
readMgaAli (struct vscanFile *vf, struct vscanBlock *vb)
{
        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;
        
        char *cloneline;
        int wordCount;
        char *words[256];
        int num_ref_gaps;

        struct dlNode *vs_node;
        struct vscanSeq *vs;
        
        int i;
        
        lineFileNext(lf,&line,&lineSize);

        cloneline = cloneString(line);
        wordCount = chopLine(cloneline,words);

        vb->len = strlen(words[2]);
        vb->end = vb->start + vb->len - 1;

        num_ref_gaps = countChars(words[2],'-');
        vb->ref_len = vb->len - num_ref_gaps;

        vb->ref_end = vb->ref_start + vb->ref_len - 1;
        
        if (vb->ref_len == 0) {
                vb->ref_start--;
        }
        
        vb->segmentType = VSCAN_ALI;
        
        createNumOfSeqsvscanSeq(vf->numSeq, vb->vscanSeqList);
        
        vs_node = vb->vscanSeqList->head;
        vs = vs_node->val;
        dyStringAppend(vs->dySeq, words[2]);

        freeMem(cloneline);

        for (i=0;i<vf->numSeq-1 ;i++) {
                vs_node = vs_node->next;
                vs = vs_node->val;

                lineFileNext(lf,&line,&lineSize);
                cloneline = cloneString(line);
                wordCount = chopLine(cloneline,words);

                dyStringAppend(vs->dySeq, words[2]);
                freeMem(cloneline);
        }
}

void
annotateMgaGap (struct vscanBlock *vb, char *firstWord)
{
        char *c;
        struct vscanBlock *vb_prev;

        vb->segmentType = VSCAN_GAP;
        vb_prev = vb->node->prev->val;
        
        if (sameWord(firstWord, "-")) {
                /*length of the GAP in the ref sequence is 0*/

                vb->ref_len = 0;
                /*ref_start and ref_end are the LAST valid nucleotide*/
                vb->ref_start = vb->ref_end = vb_prev->ref_end;
        }

        else if (startsWith("!",firstWord)) {
                c = firstWord;
                *(c)++;
                vb->ref_len = 1;
                vb->ref_start = atol(c) + 1;
                vb->ref_end = vb->ref_start;
        }

        else {
                /*get the lenght of the ref sequence*/
                vb->ref_len = atol(firstWord);
        
                c = strchr(firstWord, ':');
                *(c)++;
                
                vb->ref_start = atol(c) + 1;
                c = strchr(c, '-');
                *(c)++;
                vb->ref_end = atol(c) + 1;
        }

        /*add total alignment positions*/
        checkvscanBlockStart(vb);
        
        vb->len = vb->ref_len;
        vb->end = vb->start + vb->len;
}


void
skipUnneededExas (struct vscanFile *vf)
{
        struct lineFile *lf = vf->lf;
        char *line;
        int lineSize;
        char *dummy[2];
        char *cloneline;
        int wordCount=1;

        dummy[0] = NULL;
        while (wordCount) {
                lineFileNext(lf,&line,&lineSize);
                cloneline = cloneString(line);
                wordCount = chopLine(cloneline, dummy);
                freeMem(cloneline);
        }
}
