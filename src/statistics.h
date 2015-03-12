/* Variscan - Nucleotide Variability for huge DNA sets */

#ifndef STATISTICS_H
#define STATISTICS_H

#ifndef VARISCAN_H
#include "variscan.h"
#endif

#ifndef WINDOW_H
#include "window.h"
#endif


void getFirstMonoGap (unsigned long start, 
                      struct vscanFile *vf);

double calculate_heterozygosity (double A, 
                                 double C, 
                                 double G, 
                                 double T, 
                                 double numSeq);

double calculate_k (struct vscanPolySummary vps, 
                    char outgroup);

void calculate_Fay_and_Wu_estimators (struct vscanPolySummary vps,
                                      char outgroup,
                                      int useMuts,
                                      struct variables *var);

short getDerivedVariants (struct vscanPolySummary vps,
                          char outgroup,
                          int i,
                          int useMuts);

double calculate_Fu_and_Li_D (double Eta, 
                              double Eta_E, 
                              double cn, 
                              double n, 
                              double a1, 
                              double a2);


double calculate_Fu_and_Li_F (double totalHZ, 
                              double Eta, 
                              double Eta_E, 
                              double cn, 
                              double n, 
                              double a1, 
                              double a2, 
                              double a1nplus1);


double calculate_Tajima_D (double totalHZ, 
                           double totalTheta, 
                           double S, 
                           double n, 
                           double a1, 
                           double a2);

short getNumberOfMutations (struct vscanPolySummary vps);

short getExternalMutations (struct vscanPolySummary vps, 
                            char outgroup,
                            int useMuts);

short getNumberOfSingletons (struct vscanPolySummary vps, 
                             int useMuts);

void calculateStatistics (struct variables *var, 
                          struct configFile *cf,
                          struct analysis *ana);

void calcRunMode11Stats (struct variables *var, 
                         struct configFile *cf,
                         struct analysis *ana);

void calcRunMode12Stats (struct variables *var, 
                         struct configFile *cf,
                         struct analysis *ana);

void calcRunMode21Stats (struct variables *var, 
                         struct configFile *cf);

double correctedDivergence (double D);

void calcRunMode22Stats (struct variables *var, 
                         struct configFile *cf,
                         struct analysis *ana);

void calcRunMode31Stats (struct variables *var, 
                         struct configFile *cf,
                         struct analysis *ana);

double calculate_Fu_fs (int n, 
                        int k0, 
                        long double theta, 
                        struct analysis *ana);

void gatherVariables (struct configFile *cf, 
                      struct vscanFile *vf, 
                      struct variables *var, 
                      struct analysis *ana, 
                      long stretchSites);

void gatherRunMode12Vars (struct configFile *cf, 
                          struct vscanFile *vf, 
                          struct variables *var, 
                          struct analysis *ana, 
                          long stretchSites);

void gatherRunMode22Vars (struct configFile *cf, 
                          struct vscanFile *vf, 
                          struct variables *var, 
                          struct analysis *ana, 
                          long stretchSites);

void gatherRunMode11Vars (struct configFile *cf, 
                          struct vscanFile *vf, 
                          struct variables *var, 
                          struct analysis *ana, 
                          long stretchSites);

long gatherPolyVariables (struct variables *var, 
                          struct configFile *cf, 
                          struct vscanFile *vf, 
                          struct analysis *ana);

long gatherMonoGapVariables (struct variables *var, 
                             struct vscanFile *vf, 
                             struct analysis *ana);

void gatherRunMode21Vars (struct configFile *cf,
                          struct vscanFile *vf, 
                          struct variables *var);

void gatherRunMode31Vars (struct configFile *cf,
                          struct vscanFile *vf, 
                          struct variables *var);

void calculateLinkage (int numSeq,
                       int outgroups,
                       int useLDSinglets, 
                       int first,
                       struct vscanPolyArray *vpa,
                       struct variables *var);

void calculateLDstats (double numSeq,
                       double *cumD,
                       double *cumD_abs,
                       double *cumDprime,
                       double *cumDprime_abs,
                       double *cumRsquare,
                       double coupl1,
                       double coupl2,
                       double repul1,
                       double repul2);

void getPhases (int col1, 
                int col2, 
                char cn1, 
                char cn2, 
                char **polyArray, 
                int numSeq,
                short *coupl1, 
                short *coupl2,
                short *repul1, 
                short *repul2);

char getCommonNuc (struct vscanPolySummary vps);

char getAncestralNuc (struct vscanPolySummary vps, char outgroup);

void countHaplotypes (int numSeq, 
                      int first, 
                      int last,                     
                      char **polyArray,
                      struct variables *var);

void comparePairwise (int col,
                      int numSeq,
                      char **polyArray,
                      short *matrix,
                      int *zerocount);

void OLDcountHaplotypes (int numSeq, 
                         unsigned long start, 
                         unsigned long end, 
                         struct vscanPolyArray *vpa);

int differentSeqs (int i, 
                   int j, 
                   unsigned long *column, 
                   unsigned long start, 
                   unsigned long end, 
                   struct vscanPolyArray *vpa);

#endif /* STATISTICS_H */
