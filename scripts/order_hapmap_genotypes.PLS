#!/usr/local/bin/perl

# order_hapmap_genotypes.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

order_hapmap_genotypes.PLS - DESCRIPTION 

=head1 SYNOPSIS

perl order_hapmap_genotypes.PLS genotype-chrwathever-popwhatever.txt

=head1 DESCRIPTION

This script will order the genotypes by its position in the
chromosome, and give a resulting
genotype-chrwathever-popwhatever.txt.hapmap file.

=head1 AUTHOR - Albert Vilella

Email 

Describe contact details here

=head1 CONTRIBUTORS

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;

my $inputfile = shift;

open INPUTFILE, "$inputfile" or die "$!";

my %entries;
my $position;
my $header;
print "Loading input file...\n";
while (<INPUTFILE>) {
    if (/^rs#/) {
        $header = $_;
        next;
    }
    #rs1000050 C/T Chr1 159923840 + ncbi_b34 
    if (/\S+\s+\S+\s+\S+\s+(\S+)\s+/) { #\
        $position = sprintf("%09d", $1);
            $entries{$position} = $_;
    } else {
        die "parsing error: $!";
    }
}

print "Writing output file $inputfile.hapmap... \n";
open OUTPUTFILE,">$inputfile.hapmap" or die "$!";
print OUTPUTFILE "$header";
foreach my $entry (sort keys %entries) {
    print OUTPUTFILE "$entries{$entry}";
}

1;
