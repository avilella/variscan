#!/usr/local/bin/perl

# mavfilter.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

mavfilter.PLS - Filters mav (multiple alignment vertical format) files:

(a) Get rid of overlapping nucleotides with respect of the reference
sequence. The rules for getting rid of overlaps are:

(a1) If the end of stretchA overlaps with the beginning of stretchB,
given in this order, the start of stretchB is chopped off until it
concatenates with the end of stretchB.

(a2) If stretchB is contained in stretchA, i.e., that the start of
stretchB overlaps the end of stretchA _and_ the end of stretchB is before
the end of stretchA, stretchB is eliminated.

(b) Get rid of positions that not map with the reference sequence, but
only with the other sequences (gaps and 'N's)

=head1 SYNOPSIS

perl mavfilter.PLS inputfile.mav

=head1 DESCRIPTION

This script will filter the inputfile.mav to give the same file but
without overlapping stretches. It will also give a log file of what
has been modified.

=head1 AUTHOR - Albert Vilella

Email 

Describe contact details here

=head1 CONTRIBUTORS

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;


1;

use strict;
use File::Basename; #Used for filename and dirname parsing
use File::Spec;     #Used for filename and dirname parsing


my $inputfile = shift;

my %inputfile;
($inputfile{'base'}, $inputfile{'dir'}, $inputfile{'ext'}) = fileparse($inputfile,'\.\w+?');

my %outfile;
$outfile{'dir'} = File::Spec->curdir();
$outfile{'base'} = $inputfile{'base'};
$outfile{'ext'} = 'filt.mav';

my $outfile = File::Spec->catfile( $outfile{'dir'}, "$outfile{'base'}\.$outfile{'ext'}" );

my %logfile;
$logfile{'dir'} = File::Spec->curdir();
$logfile{'base'} = $inputfile{'base'};
$logfile{'ext'} = 'filt.log';

my $logfile = File::Spec->catfile( $logfile{'dir'}, "$logfile{'base'}\.$logfile{'ext'}" );

open INFILE, "$inputfile" or die "$!";
open OUTFILE, ">$outfile" or die "$!";
open LOGFILE, ">$logfile" or die "$!";

my $header = 0;
my $prev_header = 0;
my %start;
my %end;
my $discount = 0;
my $modified_start = 0;

$start{$header} = 0;
$end{$header} = 1;

print "File output: $outfile\n";

while(<INFILE>) {
    if (/^\%\>\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)/) {
        $header++;
        $start{$header} = $3;
        $end{$header} = $4;
        $prev_header = $header-1;
        if ($start{$header} <= $end{$prev_header}) {
            #If stretch is inside the previous
            if ($end{$header} <= $end{$prev_header}) {
                #skip the whole stretch
                print LOGFILE "$1 $2 $start{$header} $end{$header} #skipped\n";
                while ($_ =~ /^\S+/) {
                    $_ = <INFILE>;
                }
            } else {
                #reformat header, discount some lines
                $discount = $end{$prev_header} - $start{$header} + 1;
                $modified_start = $start{$header} + $discount;
                print LOGFILE "$1 $2 $start{$header} $end{$header} -> $1 $2 $modified_start $end{$header}\n";
                print OUTFILE "\%\>    $1   $2   $modified_start    $end{$header}\n";
            }
        } else {
            print OUTFILE $_;
        }
    } elsif ($discount > 0) {
        unless ($_ =~ /^\%\+/) {
            $discount--;
            next;
        } else {
            print OUTFILE $_;
        }
    } else {
        unless (/^ / || /^-/) {
            print OUTFILE $_;
        }
    }
}

close INFILE;
close OUTFILE;
close LOGFILE;

1;
