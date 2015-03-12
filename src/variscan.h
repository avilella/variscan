 /* Variscan - Nucleotide Variability for huge DNA sets */

#ifndef VARISCAN_H
#define VARISCAN_H

#ifndef COMMON_H
#include "common.h"
#endif

/* The tenth first character in the first stretch of a PHYLIP file are
   the names of the sequences */
#define PHYLIP_LABEL_LEN 10

#define ulround(a) ((unsigned long)((a)+0.5))
/* Round floating point val to nearest unsigned long. */

#define VSCAN_BLOCK_MIN 100
#define VSCAN_BLOCK_MAX 200
#define VSCAN_RAMBUFFER_MAX 500000


/* Type of segment */
typedef enum 
{
	/* MGA_GAP = MGA format. A segment of gaps that are not
	   aligned. In this case, we don't have the sequence, but only
	   the start and end. */
	VSCAN_GAP=0,

	/* MGA_EXA = MGA format. A segment of exact match. All the
	   sequences are identical. */
	MGA_EXA=1,

	/* VSCAN_ALI = MGA or PHYLIP segment of alignment. We have the
	   sequences. */
	VSCAN_ALI=2
} segmentTypeConst;

/* Type of input file */
typedef enum
{
	FORMAT_MGA=0,
	FORMAT_PHYLIP=1,
        FORMAT_AXT=2,
        FORMAT_MAF=3,
        FORMAT_HAPMAP=4,
        FORMAT_XMFA=5,
        FORMAT_MAV=6
} fileformat;

struct vscanFile {
	/* An alignment file. */
	char *fileName;			/* Name of file this is in. */
	struct lineFile *lf;            /* File is in. */
	int numSeq;                     /* Number of sequences in
					   file */
        int chosenNumSeq;               /* SH: Number of sequences chosen
                                           for analysis*/
        int chosenNumOut;               /* SH: Number of chosen outgroups*/
	struct dlList *vscanBlockList;  /* Pointer to the vscanBlock
					   that we are working on */
 	struct dlList *bdfBlockList; 
	struct vscanPolyArray *vpa;
	fileformat format;              /* Format of the file */
	unsigned long accum_filled;     /* accumulated length of the
					   sequences for all the
					   blocks in memory */
        int rambuffer_state;
        unsigned long lastPosInMem;
        unsigned long lastRefPosInMem;
};


struct vscanSeq {
	/* Container for one of the sequences of one of the vscanBlocks */
	int id;
        boolean IndivInStretch;               /* the indiv is present in this stretch */
	struct dlNode *node;                  /* each vscanInfo is a node in
						 a list inside vscanBlock */
	struct dyString *dySeq;               /* Dynammic string that
						 contains the nucleotides */
};

struct vscanBlock { 
	int id;
	struct dlNode *node;                     /* each vscanBlock is a
						    node in a list inside
						    vscanFile */
	struct dlList *vscanSeqList;             /* Pointer to the first
						    sequence in the block */
	struct dlList *chosenSeqList;             /* Pointer to the first
						     sequence in the block
                                                     that is used for analysis*/
        struct dlList *outgroupList;             /*Pointer to the first
                                                   outgroup in the block*/

        unsigned long start;
        unsigned long end;
        unsigned long len;
	
        unsigned long ref_start;
        unsigned long ref_end;
        unsigned long ref_len;
        
        segmentTypeConst segmentType;
};

struct baseCount {
        short A;
        short C;
        short G;
        short T;
        short Gap;
        short Amb;
        short valid;
};
 
struct vscanPolyArray {
	long size;
	long filled;
        long monomorph;
        long gapAmb;
	struct vscanPolySummary *vps;
	char **polyArray;
        
        long sizeDisc;
        long filledDisc;
        struct pos *listDisc;
 
        long sizeMonoGap;
        long filledMonoGap;
        struct monoGap *listMonoGap;

        long sizeRefGap;
        long filledRefGap;
        struct pos *listRefGap;
      
        int tempDisc;
        int tempPoly;
        int tempMonoGap;
        int tempRefGap;
};

struct monoGap {
        unsigned long position;
        unsigned long ref_position;
        short valid;
};

struct pos {
        unsigned long position;
        unsigned long ref_position;
};

struct vscanPolySummary {
	unsigned long position;
        unsigned long ref_position;
	short A;
	short C;
	short G;
	short T;
	short Gap;
	short Amb;
        short valid;
};

struct configFile {
	struct lineFile *lf;
	char *fileName;			       /* Name of config file. */

    boolean useMuts;
    boolean useLDSinglets;
    short runMode;
        
        boolean completeDeletion;
        boolean fixNum;
        short numNuc;
        
	boolean         boolSW;                /* SW yes or not */
	unsigned long   widthSW;               /* Width of the window */
	unsigned long   jumpSW;                /* Jump between windows */
	short           windowType;            /* 1 - windows of real positions, 
						  0 - windows of polymorphic positions */
	unsigned long   startPosition;         /* Interval of computation:
						  START */
	unsigned long   endPosition;           /* Interval of computation:
						  END */
        boolean refPos;
        boolean boolGap;                       /* SH: Use columns with gaps and 
                                                  ambiguities for analysis
                                                  yes or no*/
        short refSeq;
  
	boolean         boolLD;                /* LD yes or not */
	boolean         boolHD;                /* HD yes or not */
	boolean         boolPi;                /* Pi yes or not */
	boolean         boolDT;                /* Dtajima yes or not */
	boolean         boolFL;                /* FuLi yes or not */
  
	boolean         booltypeDT;            /* Segregating sites or total
						  number of mutations */
  
	/* Segregating sites means that we give value = 1 to a position that
	 * has any number of mutations, without counting how many mutations.
	 * Total Number of mutations means that we give the value = to the
	 * number of mutations in that position.
	 */
  
	/*boolean         *binSeqVector;*/          /* Which sequences the user
						   chooses */
	char            *outFilePrefix;         /* Prefix of the output
						   file */
        char *outFile;
        char            *bdfBlockFile;
};

struct vscanIndividual {
        int id;               /*simple id for the individual*/
        struct dlNode *node;  /*each vscanIndividual is a node in a list
                                inside vscanPopulation*/
        char *name;           /*name of the individual (for some of the file formats)*/
        int outgroup;         /*is the individual an outgroup?
                                0=ingroup, 1=close outgoup, 2=far outgroup, 
                                3=even further outgroup, ... */
        boolean seqChoice;    /*use this individual for analysis?*/
};

struct vscanPopulation {
/*         struct vscanPopulation *next;      /\*single linked list for  */
/*                                              multiple polulations*\/ */
/*         char *name;                        /\*name of population*\/ */
        struct dlList *vindivList;         /*pointer to the first individual*/
};

struct vscanOutgroup {
        int id;                   /* simple id for each outgroup */
        int phylo;                /* phylogenetic relationship to ingroup
                                     (simply the number in config file */
        struct dlNode *node;      /* each vscanOutgroup is a node inside
                                     outgroupList */
        struct vscanSeq *vos;     /* pointer to the sequence of the outgroup */
};



struct bdfBlock {
        int id;
        struct dlNode *node;
        unsigned long ref_start;
        unsigned long ref_end;
        unsigned long start;
        unsigned long end;
        char *feature;
};

struct analysis {
        struct variables *varSW;
        boolean recalcSW;
        boolean windowsFinished;
        boolean bdfBlocksFinished;
        
        struct variables *varBdf;

        double *a1;
        double *a2;
        long double **s;
};

struct variables {
        unsigned long start;
        unsigned long end;
        unsigned long mid;

        unsigned long ref_start;
        unsigned long ref_end;
        unsigned long ref_mid;

        unsigned long numSites;
        unsigned long discNucs;
        unsigned long segSites;
        unsigned long Eta;
        unsigned long Eta_E;
        unsigned long interSegs;

        double totalHZ;
        double totalTheta;
        double PiPerSite;
        double ThetaPerSite;
        double totalK;
        double KPerSite;

        double Theta_Pi_Fay_and_Wu;
        double Theta_H;

        double Tajima_D;
        double FuLi_D;
        double FuLi_F;
        double Fay_and_Wu_H;

        double Fu_Fs;
        double D_Lewontin;
        double D_Lewontin_abs;
        double D_prime;
        double D_prime_abs;
        double r_square;
        double Hd;
        int numHaps;
        unsigned long LD_sites;
        
        unsigned long cumNumSeq;
        double cum_a1;
        double cum_a2;
        double cum_a1nplus1;
};
        

void variscan(char *infile, char *outputFile);
/* Main function - read infile, write outfile/s, and call analysis
   functions */

void vfError(struct vscanFile *vf, char *format, ...);
/* Output an error message with filename and line number included. */

void cfError(struct configFile *cf, char *format, ...);
/* Output an error message with filename and line number included. */

struct vscanFile *vscanFileOpen(char *fileName);
/* Open infile, call getnumSeq and return struct */

void vscanFileFormatAndNumSeq(struct vscanFile *vf);
/* Get number of sequences. Reuse line */

struct analysis *initAnalysisVariables();

void resetAnalysisVariables(struct variables *var);

void newBdfBlockAnalysis(struct analysis *ana);

void initArrays( struct vscanFile *vf, 
                struct analysis *ana);
/*SH: writes chosenNumSeq and  chosenNumOut*/

int phylipNextStretch(struct vscanFile *vf, 
                      struct configFile *cf);
/* Read in next stretch. Return 1 at EOF. Return 2 when full for going
 * to analyse. */

void phylipFirstStretch(struct vscanFile *vf, 
                        struct configFile *cf, 
                        struct vscanPopulation *vpop);
/* Read the first stretch of a phylip file, discarding the names of
   the sequences. Return the length of the lines. */

struct dlNode *dlElementFromIx(struct dlNode *node, int num);
/* Jump num nodes forward since the head of the list. Return the
   node. For example, num=1 would return the second node */

unsigned long filterEachAliNode(struct vscanFile *vf, 
                                struct configFile *cf, 
                                int *block_id);
/* Jump to the next vscanInfo node which segmentType is VSCAN_ALI,
 * either inside the same block or the next one if last vscanInfo node
 * in that block. Return the end position of the last scanned
 * alignment. */

void lookForPolysinAliNode(struct vscanFile *vf, 
                           struct configFile *cf, 
                           struct vscanBlock *vb);
/* Read each column of an Ali and look for if it's polymorphic */

void checkColumnsIngroup (struct vscanFile *vf, 
                          struct configFile *cf,
                          struct baseCount *bc,
                          unsigned long pos,
                          unsigned long ref_pos,
                          char *polyColumn);

void checkColumnsRunMode21 (struct vscanFile *vf, 
                            struct configFile *cf,
                            struct baseCount *bc,
                            unsigned long pos,
                            unsigned long ref_pos,
                            char *polyColumn);

void checkColumnsRunMode22 (struct vscanFile *vf, 
                            struct configFile *cf,
                            struct baseCount *bc,
                            unsigned long pos,
                            unsigned long ref_pos,
                            char *polyColumn);

void checkPolarize (struct vscanFile *vf,
                    char *polyColumn,
                    unsigned long pos,
                    unsigned long ref_pos,
                    struct baseCount *bc);

void checkMonoWithOutgroup (struct vscanFile *vf, 
                            struct configFile *cf,
                            struct baseCount *bc,
                            unsigned long pos,
                            unsigned long ref_pos,
                            char *polyColumn,
                            char monoChar);

void trimBases(struct baseCount *bc, int fixedNum);

void randomChooseBases (struct baseCount *bc, 
                        int fixedNum);

char checkMonomorph (struct baseCount *bc);

boolean validBase (char base);



struct vscanBlock *createEmptyvscanBlock(struct dlList *vscanBlockList);
/* Return a new empty vscanBlock. */


struct vscanPolyArray *createEmptyvscanPolyArray(struct vscanFile *vf, 
                                                 unsigned long size);
/* Create a new array for polymorphic columns of "size" number of
 * positions and "numSeq" nucleotides for each position. Also create
 * the accompanying vscanPolySummary vector of size "size" */

void fillPolyColumn(struct vscanFile *vf, 
                    char *column, 
                    unsigned long position,
                    unsigned long ref_position, 
                    struct baseCount *bc);
/* Fill the polyArray with a polymorphic column, and the precalculated
 * frequency of each caracter. */

boolean nextlineFileStartsWith(char *line, 
                               int lineSize, 
                               char *string);
/* Check the start of the next line without actually getting
   it. Useful for preview lines that are to be taken later. */

struct vscanSeq *createEmptyvscanSeq(struct dlList *vscanSeqList);
/* Return a new empty vscanSeq */

struct vscanSeq *createOneSeqvscanSeq(char *string, 
                                      struct dlList *vscanSeqList);
/* Return a new vscanSeq with its string. */

void createNumOfSeqsvscanSeq (int num, 
                              struct dlList *vscanSeqList);
/* Create a list of new vscanSeq's inside vscanSeqList */


boolean try_analyse(int state, 
                    struct configFile *cf, 
                    struct vscanFile *vf, 
                    struct vscanPopulation *vpop, 
                    struct analysis *ana);
/* Knowing the state of the reading, go to analyse if
 * pertinent. Return TRUE if state is end_of_file. */

void analyse(struct configFile *cf, 
             struct vscanFile *vf, 
             struct vscanPopulation *vpop, 
             struct analysis *ana);

void filterPolymorphisms(struct configFile *cf, 
                         struct vscanFile *vf);
/* Check each position segment and evaluate to see if there is a
 * polymorphism or not. */

void checkGapNode (struct vscanFile *vf, 
                   struct vscanBlock *vb);

void fillDiscarded(struct vscanFile *vf, 
                   unsigned long position, 
                   unsigned long ref_position);
/*SH: fill discarded sites into a list*/

void fillMonomorphicGap(struct vscanFile *vf, 
                        unsigned long position, 
                        unsigned long ref_position, 
                        int valid);
/*SH: fill gapped sites into a list*/

char countBase (char base, 
                char reference_base, 
                struct baseCount *bc);
/* Count which nucleotide we have at a specific position for a
   specific sequence. Take into account reference_base when '.' is
   found. Return the base. */

void resetBaseCount (struct baseCount *bc);

void loadBlockDataFile(struct configFile *cf, 
                       struct vscanFile *vf);
/*SH: loads a bdf file*/

void createBdfBlock(int refPos, 
                    unsigned long start, 
                    unsigned long end, 
                    char *feature, 
                    struct dlList *list);

char *skipToWord(char *line, int n);

struct configFile *loadConfigFile(struct vscanFile *vf, 
                                  struct vscanPopulation *vpop, 
                                  char *fileName);
/* Loads a config file and returns the configFile options */

void checkConfigFile (struct configFile *cf, 
                      struct vscanFile *vf, 
                      struct vscanPopulation *vpop);

int
getParameter (char *word, 
              char *strOpts[], 
              int numOpts);

void
writeParameter (struct configFile *cf, 
                struct vscanFile *vf, 
                struct vscanPopulation *vpop, 
                char *words[], 
                int wordCount,
                int param);

boolean determineConfigBoolean(char *c, 
                               struct configFile *cf);

/* Determine if the boolean option in config file is 1 or 0. Call
 * error in either case. */

int fastBlockListCount(struct dlList *vscanBlockList);

#endif /* VARISCAN_H */

