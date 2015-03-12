# -*-Perl-*-
## Test Harness Script
##

use strict;
use vars qw($DEBUG $TESTCOUNT);
$DEBUG = $ENV{'BIOPERLDEBUG'} || 0;

BEGIN {
    eval { require Test; };
    if( $@ ) {
        use lib 't';
    }
    use Test;
    #THIS NUMBER HAS TO BE MANUALLY UPDATED. COUNT THE NUMBER OF "ok"
    #in the script
    $TESTCOUNT = 9;
    plan tests => $TESTCOUNT;
}

my $command = "../src/variscan";
my $arg1;
my $arg2;
my $outfile = "/tmp/variscan.stdout";
my $errfile = "/tmp/variscan.stderr";
my $redirection = " 1>$outfile 2>$errfile";
my $commandstring = "";

if (-e $outfile) {unlink($outfile);}
if (-e $errfile) {unlink($errfile);}


################################################################################
# SARS.phy
################################################################################

$arg1 = "./data" . "/SARS.phy";
$arg2 = "./data"  . "/SARS.conf";

$commandstring = "$command $arg1 $arg2 $redirection";
printf("# $commandstring\n");
run($commandstring);

printf "Program finished but with errors: $errfile" unless(-z $errfile);

if( !-e $outfile || -z $outfile ) {
  warn( "variscan call crashed: $? [command $commandstring]\n");
  return undef;
}


my $output;

open ($output, $outfile) or die "$!";

#Start in alignment: 1
#End in alignment: 64082
#Num of analysed sites: 7434
#Num of discarded sites: 56648
#S          Eta        Pi         Theta      Tajima's D Fu&Li's D* Fu&Li's F*
#0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259

my $analysed = 0;
my $discarded = 0;
my $S = 0;
my $Eta = 0;
my $Eta_E = 0;
my $Pi = 0;
my $Theta = 0;
my $Tajima_D = 0;
my $FuLi_D_star = 0;
my $FuLi_F_star = 0;
my $insummary = 0;

while(<$output>) {
    if ($_ =~ /analysed sites: (\S+)/) {
        $analysed = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ($_ =~ /discarded sites: (\S+)/) {
        $discarded = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ( ($_ =~ /^#\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) && $insummary==1) {
        #S          Eta          Pi         Theta      Tajima_D FuLi_D_star FuLi_F_star
        #0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259
        $S =           sprintf ("%10lu", $1);
        $Eta =         sprintf ("%10lu", $2);
        $Eta_E =       sprintf ("%10lu", $3);
        $Pi =          sprintf ("%10.7f", $4);
        $Theta =       sprintf ("%10.7f", $5);
        $Tajima_D =    sprintf ("%10.7f", $6);
        $FuLi_D_star = sprintf ("%10.7f", $7);
        $FuLi_F_star = sprintf ("%10.7f", $8);
    }
}

#These are hard-coded values. Don't change the input files.

ok $analysed,    sprintf ("%10lu", 7434);
ok $discarded,   sprintf ("%10lu", 56648);
ok $S,           sprintf ("%10lu", 5130);
ok $Eta,         sprintf ("%10lu", 8074);
ok $Pi,          sprintf ("%10.7f", 0.3790499);
ok $Theta,       sprintf ("%10.7f", 0.4433024);
ok $Tajima_D,    sprintf ("%10.7f", -0.8552262);
ok $FuLi_D_star, sprintf ("%10.7f", -0.1965028);
ok $FuLi_F_star, sprintf ("%10.7f", -0.3825936);

close $output;
unlink($outfile);
unlink($errfile);

1;

################################################################################
# rp49_36.12.phy
################################################################################

$arg1 = "./data" . "/rp49_36.phy";
$arg2 = "./data"  . "/rp49_36.12.conf";

$commandstring = "$command $arg1 $arg2 $redirection";
printf("# $commandstring\n");
run($commandstring);

my $output;

open ($output, $outfile) or die "$!";

#Start in alignment: 1
#End in alignment: 64082
#Num of analysed sites: 7434
#Num of discarded sites: 56648
#S          Eta        Pi         Theta      Tajima's D Fu&Li's D* Fu&Li's F*
#0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259

my $analysed = 0;
my $discarded = 0;
my $S = 0;
my $Eta = 0;
my $Eta_E = 0;
my $Pi = 0;
my $Theta = 0;
my $Tajima_D = 0;
my $FuLi_D_star = 0;
my $FuLi_F_star = 0;
my $insummary = 0;

while(<$output>) {
    if ($_ =~ /analysed sites: (\S+)/) {
        $analysed = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ($_ =~ /discarded sites: (\S+)/) {
        $discarded = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ( ($_ =~ /^#\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) && $insummary==1) {
        #S          Eta          Pi         Theta      Tajima_D FuLi_D_star FuLi_F_star
        #0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259
        $S =           sprintf ("%10lu", $1);
        $Eta =         sprintf ("%10lu", $2);
        $Eta_E =       sprintf ("%10lu", $3);
        $Pi =          sprintf ("%10.7f", $4);
        $Theta =       sprintf ("%10.7f", $5);
        $Tajima_D =    sprintf ("%10.7f", $6);
        $FuLi_D_star = sprintf ("%10.7f", $7);
        $FuLi_F_star = sprintf ("%10.7f", $8);
    }
}

#These are hard-coded values. Don't change the input files.

ok $analysed,    sprintf ("%10lu", 1485);
ok $discarded,   sprintf ("%10lu", (1798-1485));
ok $S,           sprintf ("%10lu", 54);
ok $Eta,         sprintf ("%10lu", 56);
ok $Pi,          sprintf ("%10.7f",  0.0080104);
ok $Theta,       sprintf ("%10.7f",  0.0109638);
ok $Tajima_D,    sprintf ("%10.7f", -1.1140364);
ok $FuLi_D_star, sprintf ("%10.7f", -1.0673264);
ok $FuLi_F_star, sprintf ("%10.7f", -1.2553091);

close $output;
unlink($outfile);
unlink($errfile);

1;

################################################################################
# rp49_36.21.phy
################################################################################

$arg1 = "./data" . "/rp49_36.phy";
$arg2 = "./data"  . "/rp49_36.21.conf";

$commandstring = "$command $arg1 $arg2 $redirection";
printf("# $commandstring\n");
run($commandstring);

my $output;

open ($output, $outfile) or die "$!";

#Start in alignment: 1
#End in alignment: 64082
#Num of analysed sites: 7434
#Num of discarded sites: 56648
#S          Eta        Pi         Theta      Tajima's D Fu&Li's D* Fu&Li's F*
#0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259

my $analysed = 0;
my $discarded = 0;
my $S = 0;
my $Eta = 0;
my $S_inter = 0;
my $Pi = 0;
my $K = 0;
my $insummary = 0;

while(<$output>) {
    if ($_ =~ /analysed sites: (\S+)/) {
        $analysed = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ($_ =~ /discarded sites: (\S+)/) {
        $discarded = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ( ($_ =~ /^#\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) && $insummary==1) {
        #S          Eta          Pi         Theta      Tajima_D FuLi_D_star FuLi_F_star
        #0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259
        $S =           sprintf ("%10lu", $1);
        $Eta =         sprintf ("%10lu", $2);
        $S_inter =     sprintf ("%10lu", $3);
        $Pi =          sprintf ("%10.7f", $4);
        $K =           sprintf ("%10.7f", $5);

    }
}

#These are hard-coded values. Don't change the input files.

ok $analysed,    sprintf ("%10lu", 1482);
ok $discarded,   sprintf ("%10lu", (1798-1482));
ok $S,           sprintf ("%10lu", 54);
ok $Eta,         sprintf ("%10lu", 56);
ok $S_inter,     sprintf ("%10lu", 97);
ok $Pi,          sprintf ("%10.7f", 0.0080266);
ok $K,           sprintf ("%10.7f", 0.0395223);

close $output;
unlink($outfile);
unlink($errfile);

1;

################################################################################
# rp49_36.22.phy
################################################################################

$arg1 = "./data" . "/rp49_36.phy";
$arg2 = "./data"  . "/rp49_36.22.conf";

$commandstring = "$command $arg1 $arg2 $redirection";
printf("# $commandstring\n");
run($commandstring);

my $output;

open ($output, $outfile) or die "$!";

#Start in alignment: 1
#End in alignment: 64082
#Num of analysed sites: 7434
#Num of discarded sites: 56648
#S          Eta        Pi         Theta      Tajima's D Fu&Li's D* Fu&Li's F*
#0000005130 0000008074    0.37905    0.44330   -0.85523   -0.19650   -0.38259

my $analysed = 0;
my $discarded = 0;
my $S = 0;
my $Eta = 0;
my $Eta_E = 0;
my $Pi = 0;
my $FuLi_D = 0;
my $FuLi_F = 0;
my $FayWu_H = 0;
my $insummary = 0;

while(<$output>) {
    if ($_ =~ /analysed sites: (\S+)/) {
        $analysed = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ($_ =~ /discarded sites: (\S+)/) {
        $discarded = sprintf ("%10lu", $1);
        $insummary = 1;
    } elsif ( ($_ =~ /^#\s*(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) && $insummary==1) {
        $S =           sprintf ("%10lu", $1);
        $Eta =         sprintf ("%10lu", $2);
        $Eta_E =       sprintf ("%10lu", $3);
        $Pi =          sprintf ("%10.7f", $4);
        $FuLi_D =      sprintf ("%10.7f", $5);
        $FuLi_F =      sprintf ("%10.7f", $6);
        $FayWu_H =     sprintf ("%10.7f", $7);

    }
}

#These are hard-coded values. Don't change the input files.

print "Results compared to DnaSP\n";

ok $analysed,    sprintf ("%10lu", 1482);
ok $discarded,   sprintf ("%10lu", (1798-1482));
ok $S,           sprintf ("%10lu", 52);
ok $Eta,         sprintf ("%10lu", 54);
ok $Pi,          sprintf ("%10.7f",  0.0076385);
ok $FuLi_D,      sprintf ("%10.7f", -1.2165480);
ok $FuLi_F,      sprintf ("%10.7f", -1.4661024);
ok $FayWu_H,     sprintf ("%10.7f", -2.1437908);

close $output;
unlink($outfile);
unlink($errfile);

1;


################################################################################
# MAF FILE
################################################################################

# $arg1 = "./data" . " ";
# $arg2 = "./data"  . " ";
# $commandstring = "$command $arg1 $arg2 $redirection";
# run($commandstring);

1;

################################################################################
# XMFA FILE
################################################################################

# $arg1 = "./data" . " ";
# $arg2 = "./data"  . " ";
# $commandstring = "$command $arg1 $arg2 $redirection";
# run($commandstring);

1;

################################################################################
## FUNCTIONS
################################################################################

sub run {
    my @args = @_;
    my $sysstat = system(@args);
    if ($sysstat == -1) {
        print "failed to execute: $!\n";
    }
    elsif ($sysstat & 127) {
        printf "child died with signal %d, %s coredump\n",
            ($sysstat & 127),  ($sysstat & 128) ? 'with' : 'without';
    }
    else {
        #      successfull run
        #      printf "child exited with value %d\n", $sysstat >> 8;
    }
}

1;
