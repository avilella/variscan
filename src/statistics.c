/* Variscan - Nucleotide Variability for huge DNA sets */

#include "statistics.h"

#ifndef WINDOW_H
#include "window.h"
#endif

void 
getFirstMonoGap (unsigned long start, 
                 struct vscanFile *vf)
{
        /*SH: get the first gapped monomorphic column on or after the position*/
        int i;

        for (i=vf->vpa->tempMonoGap; 
             (vf->vpa->listMonoGap[i].position) < start 
                     && i < vf->vpa->filledMonoGap - 1; 
             i++) {
                vf->vpa->tempMonoGap++;
        }
}

void
calculate_Fay_and_Wu_estimators (struct vscanPolySummary vps,
                                 char outgroup,
                                 int useMuts,
                                 struct variables *var)
{
        /* Calculate Fay_and_Wu estimators based on the derived variants */ 
        double i;
        short Si;
        double n = (double)vps.valid;
 
        for (i=1;i<vps.valid;i++) {
                Si = getDerivedVariants(vps,outgroup,(int)i,useMuts);
                var->Theta_Pi_Fay_and_Wu += 2*(double)Si*i*(n - i) / (n*(n-1));
                var->Theta_H += 2*(double)Si*i*i / (n*(n-1));
        }
}

short
getDerivedVariants (struct vscanPolySummary vps,
                    char outgroup,
                    int i,
                    int useMuts)
{
        /* Calculates the number of derived variants of a polymorphic
         * position */
        short Si=0;
        
        if (useMuts) {
                if (outgroup == 'A') {
                        if (vps.C == i) Si++;
                        if (vps.G == i) Si++;
                        if (vps.T == i) Si++;
                }
                else if (outgroup == 'C') {
                        if (vps.A == i) Si++;
                        if (vps.G == i) Si++;
                        if (vps.T == i) Si++;
                }
                else if (outgroup == 'G') {
                        if (vps.A == i) Si++;
                        if (vps.C == i) Si++;
                        if (vps.T == i) Si++;
                }
                else if (outgroup == 'T') {
                        if (vps.A == i) Si++;
                        if (vps.C == i) Si++;
                        if (vps.G == i) Si++;
                }
        }

        else {
                if (outgroup == 'A') {
                        if ((vps.valid - vps.A) == i) Si++;
                }
                else if (outgroup == 'C') {
                        if ((vps.valid - vps.C) == i) Si++;
                }
                else if (outgroup == 'G') {
                        if ((vps.valid - vps.G) == i) Si++;
                }
                else if (outgroup == 'T') {
                        if ((vps.valid - vps.T) == i) Si++;
                }
        }

        return Si;
}
                

double 
calculate_heterozygosity (double A, 
                          double C, 
                          double G, 
                          double T, 
                          double numSeq)
{ 
        /*SH: calculate the heterozygosity at a given column.*/

        return ((numSeq/(numSeq - 1.0)) * 
                (1.0 - (((A/numSeq)*(A/numSeq)) + 
                        ((C/numSeq)*(C/numSeq)) + 
                        ((G/numSeq)*(G/numSeq)) + 
                        ((T/numSeq)*(T/numSeq)))));
}


double
calculate_k (struct vscanPolySummary vps, 
             char outgroup)
{
        /* Calculate the distance between the ingroup and the outgroup
          at the given column*/

        if (outgroup == 'A') {
                return ((double)vps.valid - (double)vps.A) / (double)vps.valid;
        }
        else if (outgroup == 'C') {
                return ((double)vps.valid - (double)vps.C) / (double)vps.valid;
        }
        else if (outgroup == 'G') {
                return ((double)vps.valid - (double)vps.G) / (double)vps.valid;
        }
        else if (outgroup == 'T') {
                return ((double)vps.valid - (double)vps.T) / (double)vps.valid;
        }
        else {
                errAbort("invalid outgroup base in function calculate");
        }
}


double
calculate_Fu_and_Li_D (double Eta, 
                       double Eta_E, 
                       double cn,
                       double n,
                       double a1, 
                       double a2)
{
        /*Calculate Fu_and_li_D statistic: using formula 32 of Fu and
        Li (1993) Note that an = a1 and bn = a2*/
        double ud;
        double vd;

        double numerator;
        double denominator;

/*         cn = 2 * (n*a1 - 2*(n-1)) / ((n-1) * (n-2)); */

        vd = 1 + (a1*a1 / (a2 + a1*a1)) *
                (cn - (n+1)/(n-1));

        ud = a1 - 1 - vd;

        numerator = Eta - a1 * Eta_E;
        denominator = sqrt (ud*Eta + vd*Eta*Eta);

        return (numerator/denominator);
}


double
calculate_Fu_and_Li_F (double totalHZ,
                       double Eta, 
                       double Eta_E, 
                       double cn, 
                       double n, 
                       double a1, 
                       double a2, 
                       double a1nplus1)
{
        /*Calculate Fu_and_li_F statistic*/
        double uf;
        double vf;

        double numerator;
        double denominator;

        vf = ( cn + 2*(n*n + n + 3) / 
               (9*n*(n-1)) - 2/(n-1) ) / 
                (a1*a1 + a2);
        
        uf = ( 1 + (n+1)/(3*(n-1)) - 
               4*(n+1)/((n-1)*(n-1)) * 
               (a1nplus1 - (2*n/(n+1))) ) /
                a1 - vf;
        
        numerator = totalHZ - Eta_E;
        denominator = sqrt (uf*Eta + vf*Eta*Eta);

        return (numerator/denominator);
}


double
calculate_Fu_and_Li_D_star (double totalTheta, 
                            double Eta, 
                            double Eta_E, 
                            double n,
                            double a1, 
                            double a2)
{
        /* Calculate Fu_and_Li_D_star: using the formula of Simonsen
          et al. (1995), an = a1 and bn = a2*/

        double numerator;
        double denominator;

        double ud_star;
        double vd_star;

        vd_star = ( a2/(a1*a1) - 2/n * 
                    (1 + 1/a1 - a1 + a1/n) - 
                    1/(n*n) ) / (a1*a1 + a2);

        ud_star = ( ((n-1)/n - 1/a1) / a1 ) - vd_star;

        numerator = totalTheta - Eta_E * ((n-1)/n);
        denominator = sqrt(ud_star*Eta + vd_star*Eta*Eta);

        return (numerator/denominator);

}


double
calculate_Fu_and_Li_F_star (double totalHZ,
                            double Eta, 
                            double Eta_E, 
                            double n, 
                            double a1, 
                            double a2, 
                            double a1nplus1)
{
        /* Calculate Fu_and_Li_F_star*/

        double numerator;
        double denominator;

        double uf_star;
        double vf_star;

        vf_star = ( (2*n*n*n + 110*n*n - 255*n + 153) /
                    (9*n*n*(n-1)) + 
                    2*(n-1)*a1 / (n*n) - 
                    8*a2/n ) / (a1*a1 + a2);

        uf_star = ( (4*n*n + 19*n + 3 - 12*(n+1)*a1nplus1) /
                    (3*n*(n-1)) ) / a1 - vf_star;

        numerator = totalHZ - Eta_E * ((n-1)/n);
        denominator = sqrt(uf_star*Eta + vf_star*Eta*Eta);

        return (numerator/denominator);
}


double 
calculate_Tajima_D (double totalHZ, 
                    double totalTheta, 
                    double S, 
                    double n, 
                    double a1, 
                    double a2)
{
        /* Calculate Tajima_D: using the nomenclature
         * of Tajima (1993)*/

        double b1;
        double b2;
        
        double e1;
        double e2;
        
        double numerator;
        double denominator;

        b1 = (n + 1) / (3 * (n - 1));
        b2 = (2 * (n*n + n + 3)) / (9 * n * (n - 1));

        e1 = (b1 - (1 / a1)) / a1;
        e2 = (b2 - (n + 2) / (a1 * n) + (a2 / (a1 * a1))) / ((a1 * a1) + a2);

        numerator = totalHZ - totalTheta;
        denominator = sqrt((e1 * S) + (e2 * S * (S - 1)));

        return (numerator / denominator);
}


short 
getNumberOfMutations (struct vscanPolySummary vps)
{
        /*SH: Calculate and return the number of mutations at a polymorphic
         * column */
        
        short numMut = -1;

        if (vps.A > 0) numMut++;
        if (vps.C > 0) numMut++;
        if (vps.G > 0) numMut++;
        if (vps.T > 0) numMut++;

        return numMut;
}


short
getNumberOfSingletons (struct vscanPolySummary vps, int useMuts)
{
        /* The behaviour of this function depends on the useMuts parameter.
        If this is set to 1 the return value is the number of singletons
        in the column. If it is set to 0 the return value is either 0 for 
        no singletons present, or 1 for singletons present (regardless how
        many there are)*/
        
        short singlets = 0;

        if (vps.A == 1) singlets++;
        if (vps.C == 1) singlets++;
        if (vps.G == 1) singlets++;
        if (vps.T == 1) singlets++;
        
        if (singlets == 4) singlets--;

        if (useMuts == 0)
           if (singlets > 0) singlets = 1;
        
        return singlets;
}
        

short
getExternalMutations (struct vscanPolySummary vps, 
                      char outgroup, 
                      int useMuts)
{
        /* Calculate and return the number of external mutations,
           polarized by the outgroup. If useMuts is set to 0 return
           1 if external mutations are present, 0 otherwise*/

        short muts = 0;

        if (outgroup == 'A') {
                if (vps.C == 1) muts++;
                if (vps.G == 1) muts++;
                if (vps.T == 1) muts++;
        }
        else if (outgroup == 'C') {
                if (vps.A == 1) muts++;
                if (vps.G == 1) muts++;
                if (vps.T == 1) muts++;
        }
        else if (outgroup == 'G') {
                if (vps.A == 1) muts++;
                if (vps.C == 1) muts++;
                if (vps.T == 1) muts++;
        }
        else if (outgroup == 'T') {
                if (vps.A == 1) muts++;
                if (vps.C == 1) muts++;
                if (vps.G == 1) muts++;
        }
        else {
                errAbort("Invalid outgroup in getExternalMutations!");
        }

        if (useMuts == 0)
           if (muts > 0) muts = 1;
        
        return muts;
}


void
calculateStatistics (struct variables *var, 
                     struct configFile *cf,
                     struct analysis *ana)
{
        /* Calculates the statistics according to the runmode option */

        if (cf->runMode == 11){
                calcRunMode11Stats(var,cf,ana);
        }  

        else if (cf->runMode == 12){
                calcRunMode12Stats(var,cf,ana);
        }  

        else if (cf->runMode == 21){
                calcRunMode21Stats(var,cf);
        }  

        else if (cf->runMode == 22){
                calcRunMode22Stats(var,cf,ana);
        }  

        else if (cf->runMode == 31){
                calcRunMode31Stats(var,cf,ana);
        }  

        else {
                cfError(cf,"Unknown RunMode in calculateStatistics!");
        }
}


void
calcRunMode11Stats (struct variables *var, 
                    struct configFile *cf,
                    struct analysis *ana)
{
        /* Calculates the statistics according to the runmode option
         * 11. See the data/README.conf file for more info */

        double avgNumSeq;
        double avg_a1;
        
        /*SH: Now calculate the average values*/
        
        if (!cf->fixNum) {
                avgNumSeq = (double)var->cumNumSeq / (double)var->numSites;
                avg_a1 = var->cum_a1 / (double)var->numSites;
        }
        else {
                avgNumSeq = (double)cf->numNuc;
                avg_a1 = ana->a1[cf->numNuc];
        }

        /*SH: we can now calculate Theta and D and F statistics depending on
          cf->useMuts*/

        if (!cf->useMuts) {
                /*SH: use number of segregating sites*/
                var->totalTheta = (double)var->segSites / avg_a1;
        } else {
                /*SH: use number of total mutations*/
                var->totalTheta = (double)var->Eta / avg_a1;
        }
                
        /*SH: now calculate the per site values*/
        var->PiPerSite = var->totalHZ /(double)var->numSites;
        var->ThetaPerSite = var->totalTheta / (double)var->numSites;
}


void
calcRunMode12Stats (struct variables *var, 
                    struct configFile *cf,
                    struct analysis *ana)
{
        /* Calculates the statistics according to the runmode option
         * 12. See the data/README.conf file for more info */

        double avgNumSeq;
        double avg_a1;
        double avg_a2;
        double avg_a1nplus1;

        double Eta_calc;
                 
        /*SH: Now calculate the average values*/
        
        if (!cf->fixNum) {
                avgNumSeq = (double)var->cumNumSeq / (double)var->numSites;
                avg_a1 = var->cum_a1 / (double)var->numSites;
                avg_a2 = var->cum_a2 / (double)var->numSites;
                avg_a1nplus1 = var->cum_a1nplus1 / (double)var->numSites;
        }
        else {
                avgNumSeq = (double)cf->numNuc;
                avg_a1 = ana->a1[cf->numNuc];
                avg_a2 = ana->a2[cf->numNuc];
                avg_a1nplus1 = ana->a1[cf->numNuc+1];
        }

        /*SH: we can now calculate Theta and D and F statistics depending on
          cf->useMuts*/

        if (!cf->useMuts) /*SH: use number of segregating sites*/
                Eta_calc = (double)var->segSites;
        else /*SH: use number of total mutations*/
                Eta_calc = (double)var->Eta;
                
        var->totalTheta = Eta_calc / avg_a1;
        var->PiPerSite = var->totalHZ /(double)var->numSites;
        var->ThetaPerSite = var->totalTheta / (double)var->numSites;

        var->Tajima_D = calculate_Tajima_D(var->totalHZ, 
                                           var->totalTheta, 
                                           Eta_calc, 
                                           avgNumSeq, 
                                           avg_a1, 
                                           avg_a2);
        var->FuLi_D = calculate_Fu_and_Li_D_star(var->totalTheta, 
                                                 Eta_calc, 
                                                 (double)var->Eta_E, 
                                                 avgNumSeq,
                                                 avg_a1, 
                                                 avg_a2);
        var->FuLi_F = calculate_Fu_and_Li_F_star(var->totalHZ, 
                                                 Eta_calc, 
                                                 (double)var->Eta_E, 
                                                 avgNumSeq,
                                                 avg_a1, 
                                                 avg_a2,
                                                 avg_a1nplus1);
}


void
calcRunMode21Stats (struct variables *var, 
                   struct configFile *cf)
{
        /* Calculates the statistics according to the runmode option
         * 21. See the data/README.conf file for more info */

        var->KPerSite = var->totalK / (double)var->numSites;
        var->KPerSite = correctedDivergence(var->KPerSite);

        if (cf->numNuc > 1) {
                var->PiPerSite = var->totalHZ /(double)var->numSites;
        }

}


double
correctedDivergence (double K)
{
        /* Calculate and return the correction of interspecific
         * divergence using Jukes-Cantor*/

        if (K >= 0.75) {
                return 5;
        } else {
                return -0.75 * log(1.0 - (4.0/3.0)*K);
        }
}


void
calcRunMode22Stats (struct variables *var, 
                   struct configFile *cf,
                   struct analysis *ana)
{
        /* Calculates the statistics according to the runmode option
         * 22. See the data/README.conf file for more info */

        double avgNumSeq;
        double avg_a1;
        double avg_a2;
        double avg_a1nplus1;

        double cn;
        double Eta_calc;
        
        if (!cf->fixNum) {
                avgNumSeq = (double)var->cumNumSeq / (double)var->numSites;
                avg_a1 = var->cum_a1 / (double)var->numSites;
                avg_a2 = var->cum_a2 / (double)var->numSites;
                avg_a1nplus1 = var->cum_a1nplus1 / (double)var->numSites;

        }
        else {
                avgNumSeq = (double)cf->numNuc;
                avg_a1 = ana->a1[cf->numNuc];
                avg_a2 = ana->a2[cf->numNuc];
                avg_a1nplus1 = ana->a1[cf->numNuc+1];

        }

        cn = 2 * (avgNumSeq*avg_a1 - 2*(avgNumSeq-1)) / 
                ((avgNumSeq-1)*(avgNumSeq-2));

        var->Fay_and_Wu_H = var->Theta_Pi_Fay_and_Wu - var->Theta_H;
        
        if (!cf->useMuts) /*SH: use number of segregating sites*/
                Eta_calc = (double)var->segSites;
        else /*use number of total mutations*/
                Eta_calc = (double)var->Eta;   
                
        var->PiPerSite = var->totalHZ /(double)var->numSites;
               
        var->FuLi_D = calculate_Fu_and_Li_D(Eta_calc, 
                                            (double)var->Eta_E, 
                                            cn,
                                            avgNumSeq,
                                            avg_a1, 
                                            avg_a2);
        var->FuLi_F = calculate_Fu_and_Li_F(var->totalHZ,
                                            Eta_calc, 
                                            (double)var->Eta_E, 
                                            cn,
                                            avgNumSeq,
                                            avg_a1, 
                                            avg_a2,
                                            avg_a1nplus1);
}


void
calcRunMode31Stats (struct variables *var, 
                    struct configFile *cf,
                    struct analysis *ana)

{
        /* Calculates the statistics according to the runmode option
         * 31. See the data/README.conf file for more info */

        /* Not much calculated here, since most of the stuff is calculated
          in gatherVariables*/
       
        var->PiPerSite = var->totalHZ /(double)var->numSites;

        var->Fu_Fs = calculate_Fu_fs(cf->numNuc,
                                     var->numHaps,
                                     (long double)var->totalHZ,
                                     ana);
}


double
calculate_Fu_fs (int n, int k0, long double theta, struct analysis *ana)
{
        /* Calculates Fu_fs: using long double precision,
          because the stirling numbers can become pretty large.
          The result of Fu's Fs is then returned as a double.
          The calculation is basically the formula from 
          Fu (1997) page 916*/

        int i,k;
        long double Sn;
        long double Sprime = 0;
        long double Fs;

        Sn = theta;
        for (i=1;i<n;i++) {
                Sn *= theta + (long double)i;
        }

        for (k=k0;k<=n;k++) {
                Sprime += (long double)fabsl(ana->s[n][k]) * 
                        (long double)powl(theta,(long double)k) / Sn;
        }

        Fs = (long double)logl(Sprime / (1-Sprime));

        return (double)Fs;
}


void
gatherVariables (struct configFile *cf, 
                 struct vscanFile *vf, 
                 struct variables *var, 
                 struct analysis *ana, 
                 long stretchSites)
{
        /* Gather the appropriate variables for the chosen runMode*/

        if (cf->runMode == 11) {
                gatherRunMode11Vars (cf,vf,var,ana,stretchSites);
        }

        else if (cf->runMode == 12) {
                /*The variables needed are almost the same, so I collect them
                  in one function, and discriminate there*/
                gatherRunMode12Vars (cf,vf,var,ana,stretchSites);
        }

        else if (cf->runMode == 22) {
                /*The variables needed are almost the same, so I collect them
                  in one function, and discriminate there*/
                gatherRunMode22Vars (cf,vf,var,ana,stretchSites);
        }

        else if (cf->runMode == 21) {
                gatherRunMode21Vars (cf,vf,var);
        }
        
        else if (cf->runMode == 31) {
                gatherRunMode31Vars (cf,vf,var);
        }

        else {
                cfError(cf,"Unknown RunMode in gatherVariables!");
        }
}


void 
gatherRunMode12Vars (struct configFile *cf, 
                     struct vscanFile *vf, 
                     struct variables *var, 
                     struct analysis *ana, 
                     long stretchSites)
{
        /*SH: Calculates polymorphism statistics for the current
          window. This is only an intermediate computing step.
          Because we have to take sites with gaps into account, many
          parameters are calculated here. For the calculation of Theta
          we need an average value of a1, and for the calculation of
          Tajima's D we need an average value of a1, a2 and NumSeq
          over all analyzed sites in the window. Because of potential
          gaps a1, a2 and Numseq may vary among sites*/
        
        long stretchMonoGaps=0;
        long stretchSegSites=0;
        long stretchMonoNonGaps=0;
        
        /*SH: calculate statistics for all polymorphic columns inside the window*/
        stretchSegSites = gatherPolyVariables (var, cf, vf, ana);

        if (!cf->fixNum) {
                /*SH: Now check for monomorphic columns with gaps inside the window*/
                stretchMonoGaps = gatherMonoGapVariables (var, vf, ana);
                
                /*SH: Now that we have taken care of polymorphic and gapped sites, 
                  add the values for the monomorphic, non-gapped sites to the 
                  cumulative values*/
                
                stretchMonoNonGaps = stretchSites - stretchSegSites - stretchMonoGaps;
                
                var->cumNumSeq += (stretchMonoNonGaps * vf->chosenNumSeq);
                var->cum_a1 += (stretchMonoNonGaps * ana->a1[vf->chosenNumSeq]);
                var->cum_a2 += (stretchMonoNonGaps * ana->a2[vf->chosenNumSeq]);
        }
}

void 
gatherRunMode22Vars (struct configFile *cf, 
                     struct vscanFile *vf, 
                     struct variables *var, 
                     struct analysis *ana, 
                     long stretchSites)
{
        /*SH: Calculates polymorphism statistics for the current
          window. This is only an intermediate computing step.
          Because we have to take sites with gaps into account, many
          parameters are calculated here. For the calculation of Theta
          we need an average value of a1, and for the calculation of
          Tajima's D we need an average value of a1, a2 and NumSeq
          over all analyzed sites in the window. Because of potential
          gaps a1, a2 and Numseq may vary among sites*/
        
        long stretchMonoGaps=0;
        long stretchSegSites=0;
        long stretchMonoNonGaps=0;
        
        /*SH: calculate statistics for all polymorphic columns inside the window*/
        stretchSegSites = gatherPolyVariables (var, cf, vf, ana);

        if (!cf->fixNum) {
                /*SH: Now check for monomorphic columns with gaps inside the window*/
                stretchMonoGaps = gatherMonoGapVariables (var, vf, ana);
                
                /*SH: Now that we have taken care of polymorphic and gapped sites, 
                  add the values for the monomorphic, non-gapped sites to the 
                  cumulative values*/
                
                stretchMonoNonGaps = stretchSites - stretchSegSites - stretchMonoGaps;
                
                var->cumNumSeq += (stretchMonoNonGaps * vf->chosenNumSeq);
                var->cum_a1 += (stretchMonoNonGaps * ana->a1[vf->chosenNumSeq]);
                var->cum_a2 += (stretchMonoNonGaps * ana->a2[vf->chosenNumSeq]);
                var->cum_a1nplus1 += (stretchMonoNonGaps * ana->a1[vf->chosenNumSeq + 1]);
        }
}


void 
gatherRunMode11Vars (struct configFile *cf, 
                     struct vscanFile *vf, 
                     struct variables *var, 
                     struct analysis *ana, 
                     long stretchSites)
{
        /*SH: Calculates polymorphism statistics for the current
          window. This is only an intermediate computing step.
          Because we have to take sites with gaps into account, many
          parameters are calculated here. For the calculation of Theta
          we need an average value of a1, and for the calculation of
          Tajima's D we need an average value of a1, a2 and NumSeq
          over all analyzed sites in the window. Because of potential
          gaps a1, a2 and Numseq may vary among sites*/
        
        long stretchMonoGaps=0;
        long stretchSegSites=0;
        long stretchMonoNonGaps=0;
        
        /*SH: calculate statistics for all polymorphic columns inside the window*/
        stretchSegSites = gatherPolyVariables (var, cf, vf, ana);

        if (!cf->fixNum) {
                /*SH: Now check for monomorphic columns with gaps inside the window*/
                stretchMonoGaps = gatherMonoGapVariables (var, vf, ana);
                
                /*SH: Now that we have taken care of polymorphic and gapped sites, 
                  add the values for the monomorphic, non-gapped sites to the 
                  cumulative values*/
                
                stretchMonoNonGaps = stretchSites - stretchSegSites - stretchMonoGaps;
                
                var->cumNumSeq += (stretchMonoNonGaps * vf->chosenNumSeq);
                var->cum_a1 += (stretchMonoNonGaps * ana->a1[vf->chosenNumSeq]);
        }
}


long
gatherPolyVariables (struct variables *var,
                     struct configFile *cf, 
                     struct vscanFile *vf, 
                     struct analysis *ana)
{
        /*SH: Calculate statistics for all polymorphic columns inside
         * the window*/
        long stretchSegSites=0;
        int i;

        /*SH: get the first polymorphic postion inside the window*/
        getFirstPolymorphism (var->start, vf);

        for (i=vf->vpa->tempPoly; 
             (vf->vpa->vps[i].position) <= var->end 
                     && (vf->vpa->vps[i].position) >= var->start 
                     && i < vf->vpa->filled; 
             i++) {
                
                /*SH: Add a segregating site (for the stretch, later add up to the total window)*/
                stretchSegSites++;

                /*SH: Sum number of Mutations*/
                var->Eta += getNumberOfMutations(vf->vpa->vps[i]);

                if (cf->runMode > 10 && cf->runMode < 20) {
                        var->Eta_E += getNumberOfSingletons(vf->vpa->vps[i],
                                                            cf->useMuts);
                }
                else {
                        var->Eta_E += getExternalMutations(vf->vpa->vps[i],
                                                           vf->vpa->polyArray[i][vf->chosenNumSeq],
                                                           cf->useMuts);
                        calculate_Fay_and_Wu_estimators(vf->vpa->vps[i],
                                                        vf->vpa->polyArray[i][vf->chosenNumSeq],
                                                        cf->useMuts,
                                                        var);
                }
                        
                /*SH: calculate cumulative values we need for Theta and the D and F statistics*/
                if (!cf->fixNum) {
                        var->cumNumSeq += vf->vpa->vps[i].valid;
                        var->cum_a1 += ana->a1[vf->vpa->vps[i].valid];
                        var->cum_a2 += ana->a2[vf->vpa->vps[i].valid];
                        var->cum_a1nplus1 += ana->a1[vf->vpa->vps[i].valid + 1];

                        var->discNucs += vf->chosenNumSeq - vf->vpa->vps[i].valid;
                }
                
                /*Calculation of Pi*/
                var->totalHZ += calculate_heterozygosity((double)vf->vpa->vps[i].A,
                                                         (double)vf->vpa->vps[i].C,
                                                         (double)vf->vpa->vps[i].G,
                                                         (double)vf->vpa->vps[i].T,
                                                         (double)vf->vpa->vps[i].valid);
        }
        var->segSites += stretchSegSites;
        
        return stretchSegSites;
}


long
gatherMonoGapVariables (struct variables *var, 
                        struct vscanFile *vf, 
                        struct analysis *ana)
{
        /*SH: Calculate all the intermediate data needed for Theta and
         * Tajima's D for all monomorphic gapped columns*/
        long monoGaps=0;
        int i;

        /*SH: get the first gapped monomorphic column inside the window*/
        getFirstMonoGap (var->start,vf);

        for (i=vf->vpa->tempMonoGap; 
             vf->vpa->listMonoGap[i].position <= var->end  
                     && vf->vpa->listMonoGap[i].position >= var->start  
                     && i < vf->vpa->filledMonoGap;  
             i++) {

                monoGaps++;
                
                var->cumNumSeq += vf->vpa->listMonoGap[i].valid;
                var->cum_a1 += ana->a1[vf->vpa->listMonoGap[i].valid];
                var->cum_a2 += ana->a2[vf->vpa->listMonoGap[i].valid];
                var->cum_a1nplus1 += ana->a1[vf->vpa->listMonoGap[i].valid + 1];

                var->discNucs += vf->chosenNumSeq - vf->vpa->listMonoGap[i].valid;

        }
        
        return monoGaps;
}


void 
gatherRunMode21Vars (struct configFile *cf,
                     struct vscanFile *vf, 
                     struct variables *var) 
{
        /*Calculates statistics for the current window. This is an
           intermediate computing step.*/

        int i;
        short muts;
        
        getFirstPolymorphism (var->start, vf);

        for (i=vf->vpa->tempPoly; 
             (vf->vpa->vps[i].position) <= var->end 
                     && (vf->vpa->vps[i].position) >= var->start 
                     && i < vf->vpa->filled; 
             i++) {
                
                var->interSegs++;
                                   
                var->totalK += 
                        calculate_k(vf->vpa->vps[i],vf->vpa->polyArray[i][vf->chosenNumSeq]);
                
                if (cf->numNuc > 1) {
                        /*The number of segregating sites only
                          refers to the ingroup. So we can't just use 
                          the number of polys in the polyarray (these are the
                          interSegs = the number of segregating sites between
                          the two species). We have to check if each poly is
                          also polymorphic when just looking at the ingroup. 
                          The function getNumberOfMutations just considers the 
                          ingroup, so we are using it for this purpose*/
                        muts = getNumberOfMutations(vf->vpa->vps[i]);
                        var->Eta += muts;

                        if (muts > 0) {
                                var->segSites++;
                        }

                        var->totalHZ += 
                                calculate_heterozygosity((double)vf->vpa->vps[i].A,
                                                         (double)vf->vpa->vps[i].C,
                                                         (double)vf->vpa->vps[i].G,
                                                         (double)vf->vpa->vps[i].T,
                                                         (double)vf->vpa->vps[i].valid);
                }
        }
        
        if(!cf->fixNum) {
                /*check if there are any dicarded nucleotides in a site*/
                getFirstMonoGap (var->start,vf);

                for (i=vf->vpa->tempMonoGap; 
                     vf->vpa->listMonoGap[i].position <= var->end  
                     && vf->vpa->listMonoGap[i].position >= var->start  
                     && i < vf->vpa->filledMonoGap;  
                     i++) {
                          var->discNucs += vf->chosenNumSeq - vf->vpa->listMonoGap[i].valid;

                }

        }
}


void 
gatherRunMode31Vars (struct configFile *cf,
                     struct vscanFile *vf, 
                     struct variables *var)
{
        /*Calculates statistics for the current window. This is an
           intermediate computing step.*/

        int last;
        int i;

        getFirstPolymorphism (var->start, vf);

        
        for (i=vf->vpa->tempPoly; 
             (vf->vpa->vps[i].position) <= var->end 
                     && (vf->vpa->vps[i].position) >= var->start 
                     && i < vf->vpa->filled; 
             i++) {
                
                var->segSites++;

                var->totalHZ += 
                        calculate_heterozygosity((double)vf->vpa->vps[i].A,
                                                 (double)vf->vpa->vps[i].C,
                                                 (double)vf->vpa->vps[i].G,
                                                 (double)vf->vpa->vps[i].T,
                                                 (double)vf->vpa->vps[i].valid);
        }

        last = vf->vpa->tempPoly + var->segSites;

        if (var->segSites > 0) {
                countHaplotypes(vf->chosenNumSeq, 
                                vf->vpa->tempPoly,
                                last,
                                vf->vpa->polyArray,
                                var);
                
                calculateLinkage(vf->chosenNumSeq,
                                 vf->chosenNumOut,
                                 cf->useLDSinglets,
                                 vf->vpa->tempPoly,
                                 vf->vpa,
                                 var);
        }
        
        else {
                if (var->numSites >0) {
                        var->numHaps = 1;
                }
        }
}


void
calculateLinkage (int numSeq,
                  int outgroups,  
                  int useLDSinglets,
                  int first,
                  struct vscanPolyArray *vpa,
                  struct variables *var)
{
        /* Calculates linkage disequilibrium statistic and saves the
         * values in the appropriate _var_ data structure */

        unsigned long i,j;
        char *cn;
        double cumD=0;
        double cumDprime=0;
        double cumD_abs=0;
        double cumDprime_abs=0;
        double cumRsquare=0;
        unsigned long pairs=0;

        short coupl1;
        short coupl2;
        short repul1;
        short repul2;
        
        cn = needMem((var->segSites)*sizeof(char));

        
        /*get the most common nucleotide for each polymorphic site.
        If we have an outgroup to polarize mutations, the most common 
        nucleotide becomes the ancestral nucleotide. If we discard a
        polymorphic site for whatever reason, we set the common 
        nucleotide to 'N'. In consequence this site will be ignored 
        while calculating LD statistics.*/
        for (i=0;i<var->segSites;i++) {
                if (getNumberOfMutations(vpa->vps[first+i]) > 1) {
                        /*we have more than 2 variants, discard!*/
                        cn[i] = 'N';
                }
                else if (useLDSinglets == 0 && 
                         getNumberOfSingletons(vpa->vps[first+i],1) > 0) {
                        /*we don't want to use singletons, discard!*/
                        cn[i] = 'N';
                } 
                /*OK, we will use this site, now get the common nucleotide*/                                                                     
                else if (outgroups < 1){
                        /*we dont have an outgroup, so get the most common
                        nucleotide*/            
                        cn[i] = getCommonNuc(vpa->vps[first+i]);
                }
                else {
                        /*we have an outgroup, get the ancestral state*/
                        cn[i] = getAncestralNuc(vpa->vps[first+i],
                                                vpa->polyArray[first+i][numSeq]);
                }    
        }

        for (i=0;i<var->segSites;i++) {
                /*first column of comparison*/
                if (cn[i] != 'N') {
                        /*if it is a valid column*/
                        var->LD_sites++;
                        for (j=i+1;j<var->segSites;j++) {
                                /*start comparing*/
                                if (cn[j] != 'N') {
                                        /*if this is a valid column, calculate
                                          statistics*/
                                        getPhases(i+first,j+first,
                                                  cn[i],cn[j],
                                                  vpa->polyArray,numSeq,
                                                  &coupl1,
                                                  &coupl2,
                                                  &repul1,
                                                  &repul2);

                                        calculateLDstats((double)numSeq,
                                                         &cumD,
                                                         &cumD_abs,
                                                         &cumDprime,
                                                         &cumDprime_abs,
                                                         &cumRsquare,
                                                         (double)coupl1,
                                                         (double)coupl2,
                                                         (double)repul1,
                                                         (double)repul2);

                                        pairs++;
                                }
                        }
                }
        }

        if (pairs > 0) {
                var->D_Lewontin = cumD / (double)pairs;
                var->D_Lewontin_abs = cumD_abs / (double)pairs;
                var->D_prime = cumDprime / (double)pairs;
                var->D_prime_abs = cumDprime_abs / (double)pairs;
                var->r_square = cumRsquare / (double)pairs;
        }

        freeMem(cn);
}


void
calculateLDstats (double numSeq,
                  double *cumD,
                  double *cumD_abs,
                  double *cumDprime,
                  double *cumDprime_abs,
                  double *cumRsquare,
                  double coupl1,
                  double coupl2,
                  double repul1,
                  double repul2)
{
        
        /* Calculates linkage disequilibrium stats between two
         * positions and passes the results by reference */

        double D;
        double Dprime;
        double denom1;
        double denom2;
        
        D = (coupl1*coupl2 - repul1*repul2) / (numSeq*numSeq);

        *cumD += D;
        *cumD_abs += fabs(D);
        
        
        if (D>0) {
                denom1 = (coupl1+repul1)/numSeq * (coupl2+repul1)/numSeq;
                denom2 = (coupl1+repul2)/numSeq * (coupl2+repul2)/numSeq;
        }
        else {
                denom1 = (coupl1+repul1)/numSeq * (coupl1+repul2)/numSeq;
                denom2 = (coupl2+repul2)/numSeq * (coupl2+repul1)/numSeq;
        }

        if (denom1<denom2) 
                Dprime = D / denom1;
        else
                Dprime = D / denom2;
     

        *cumDprime += Dprime;
        *cumDprime_abs += fabs(Dprime);

        *cumRsquare += D*D / ((coupl1+repul1)/numSeq * (coupl2+repul2)/numSeq *
                              (coupl1+repul2)/numSeq * (coupl2+repul1)/numSeq);

}


void
getPhases (int col1, int col2, char cn1, char cn2, 
           char **polyArray, int numSeq,
           short *coupl1, short *coupl2,
           short *repul1, short *repul2)
{
        /* Determines the phases of the linkage disequilibrium calculation */
        int i;
        
        *coupl1 = 0;
        *coupl2 = 0;
        *repul1 = 0;
        *repul2 = 0;

        for (i=0;i<numSeq;i++) {
                if (polyArray[col1][i] == cn1) {
                        /*Nuc in column 1 is common*/
                        if (polyArray[col2][i] == cn2) {
                                /*Nuc in column 2 is common*/
                                (*coupl1)++;
                        }
                        else {
                                /*Nuc in column 2 is non-common*/
                                (*repul1)++;
                        }
                }

                else {
                        /*Nuc in column 1 is non-common*/
                        if (polyArray[col2][i] == cn2) {
                                /*Nuc in column 2 is common*/
                                (*repul2)++;
                        }
                        else {
                                /*Nuc in column 2 is non-common*/
                                (*coupl2)++;
                        }
                }
        }
}


char
getCommonNuc (struct vscanPolySummary vps)
{
       /* Determines the most common nucleotide for each polymorphic */
       /* site in the linkage disequilibrium stats:  */
       
       /* In this case, we dont have an outgroup defined, and to polarize */
       /* mutations, the most common nucleotide is taken. */
       
        short num=0;
        char cn='N';

        if (vps.A > num) {
                num = vps.A;
                cn = 'A';
        }
        if (vps.C > num) {
                num = vps.C;
                cn = 'C';
        }
        if (vps.G > num) {
                num = vps.G;
                cn = 'G';
        }
        if (vps.T > num) {
                num = vps.T;
                cn = 'T';
        }

        return cn;
}

char
getAncestralNuc (struct vscanPolySummary vps, char outgroup)
{
       /* Determines the most common nucleotide for each polymorphic */
       /* site in the linkage disequilibrium stats:  */
       
       /* In this case, we have an outgroup defined, and to polarize */
       /* mutations, the most common nucleotide becomes the ancestral
        * nucleotide.*/

        if (outgroup == 'A') {
                if (vps.A > 0) return 'A';
                else return 'N';
        }        

        else if (outgroup == 'C') {
                if (vps.C > 0) return 'C';
                else return 'N';
        }        

        else if (outgroup == 'G') {
                if (vps.G > 0) return 'G';
                else return 'N';
        }        

        else if (outgroup == 'T') {
                if (vps.T > 0) return 'T';
                else return 'N';
        }        

        else return 'N';
}

/* Haplotype Diversity */

/* Explanation: We have a vector which is going to be a semimatrix of
 * the comparisons between each sequence. For sequence 0, the
 * comparisons are in position 0 to numSeq-1-1 (comparing 0 to
 * 1,2,3,...,numSeq-1). For sequence 1, the comparisons are from
 * numSeq-1 to (numSeq*2)-1-(numSeq-i). 
 *
 * Example with 5 sequences:
 * 0 1 2 3       (Compares seq 0 with seq 1,2,3 and 4)
 * 4 5 6	 (Compares seq 1 with seq 2,3 and 4)
 * 7 8		 (Compares seq 2 with seq 3 and 4)
 * 9		 (Compares seq 3 with 4)
*/

void
countHaplotypes (int numSeq, 
                 int first, 
                 int last,                     
                 char **polyArray,
                 struct variables *var)
{

        int i,j;
        int column;
        short *matrix;
        int zerocount;
        int pairwise;
        int matrix_pos = 0;
        
        short *oldHaps;
        int freq=0;
        double sum=0;
        

        pairwise = (numSeq*numSeq - numSeq) / 2;
        zerocount = pairwise;

	matrix = needMem(pairwise*sizeof(short));
	
        for (i=0;i<pairwise;i++) {
		matrix[i]=0;
	}

        for (column=first;column<=last && zerocount != 0;column++) {
                comparePairwise(column,numSeq,polyArray,matrix,&zerocount);
        }
        

        oldHaps = needMem(numSeq*(sizeof(short)));

        for (i=0;i<numSeq;i++) {
                oldHaps[i] = 0;
        }

        matrix_pos = 0;

        for (i=0;i<numSeq;i++) {

                if (!oldHaps[i]) {
                        /*this is a new haplotype, with the initial frequency
                          of 1*/
                        var->numHaps++;
                        freq = 1;
                        
                        for (j=i+1;j<numSeq;j++) {
                                if (matrix[matrix_pos] == 0) {
                                        /*the haplotypes are the same, so 
                                          update oldHaps, and increase
                                          frequency*/
                                        freq++;
                                        oldHaps[j] = 1;
                                }
                                matrix_pos++;
                        }
                        /*sum up the squares of the frequency to later 
                          calculate Hd*/
                        sum += freq*freq;
                }
                
                else {
                        /*this is an old haplotype, so just ignore
                          the row*/
                        matrix_pos += numSeq - i - 1;
                }
        }

        /*make sure the sum is based on the true frequency of the haplotype,
          not just the simple count*/
        sum /= (double)(numSeq*numSeq);

        /*calculate haplotype diversity*/
        var->Hd = ((double)numSeq/((double)numSeq-1)) * (1-sum);

        freeMem(matrix);
        freeMem(oldHaps);
}

void
comparePairwise (int col,
                 int numSeq,
                 char **polyArray,
                 short *matrix,
                 int *zerocount)
{
        /* Compare a pair of elements in the haplotype diversity stats
         * and update the matrix */

        int i,j;
        int matrix_pos=0;

        for (i=0;i<numSeq;i++) {
                for (j=i+1;j<numSeq;j++) {
                        if (matrix[matrix_pos] == 0) {
                                if (polyArray[col][i] != polyArray[col][j]) {
                                        matrix[matrix_pos] = 1;
                                        *zerocount--;
                                }
                        }
                        matrix_pos++;
                }
        }
}
