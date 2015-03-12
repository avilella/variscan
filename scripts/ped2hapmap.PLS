# ped2hapmap.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

ped2hapmap.PLS - DESCRIPTION 

=head1 SYNOPSIS

ped2hapmap.PLS -hapl hapAB_list.txt -coord snp_coord.txt [-chr 15]

=head1 DESCRIPTION

This script will convert the pair of files containing phased haplotype
and snp coordinates into the hapmap data dump format. The resulting
file can be used as phased data.

CAUTIONS:

- The snps in the hapAB_list.txt file should be ordered in a "hapA -
  hapB" fashion:
                 *
--------------------------------------------------------------------------------
INDIV_02526-hapA G A G A A G T A A A G G C A A T A G C C C A G C A T C C 
INDIV_02526-hapB G A G A A G G G A A A A T G G C G A T C C A A C C G T T 
INDIV_02540-hapA G A G A A G T A A A G G T A A T A G C C A A G C A T C C 
INDIV_02540-hapB G A G A A G G G A A G A T G G C G A T C C A A C C T T T 
[...]
--------------------------------------------------------------------------------

- The entries in the snp_coord.txt file should correspond to the
  position in the "+" strand in the same chromosome and ordered by
  coordinate position (not rs id):

--------------------------------------------------------------------------------
rs498548	26000508
rs522024	26000718
rs528621	26001484
[...]
--------------------------------------------------------------------------------

  Where the first entry, rs498548, will correspond to the first SNP
  column in the hapAB_list.txt (marked with an asterisk above).

=head1 AUTHOR - Albert Vilella

Email 

Describe contact details here

=head1 CONTRIBUTORS

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;
use Getopt::Long;

my ($hapsfile,$coordsfile,$chr);
$chr = 1;

GetOptions(
	   'h|hapl|haplotypes:s' => \$hapsfile,
 	   'c|coord|coords|coordinates:s' => \$coordsfile,
 	   'chr:s' => \$chr,
          );

open (COORDS,$coordsfile) or die "could not open $coordsfile: $!";
my %coords;
my %haps;
my $coord_num = sprintf("%09d", 0);
print STDERR "Loading coordinates...\n";
while (<COORDS>) {
    $coord_num = sprintf("%09d", $coord_num+1);
    if ($_ =~ /(\S+)\s+(\S+)/) {
        my $coord = sprintf("%09d", $2);
        $coords{$coord_num}{rs} = $1;
        $coords{$coord_num}{coord} = $2;
    }
}
close COORDS;
my $snp_size = $coord_num;
my %entries_header;

my $hap_num = sprintf("%09d", 0);
open (HAPS,$hapsfile) or die "could not open $hapsfile: $!";
print STDERR "Loading haplotypes...\n";
while (<HAPS>) {
    if ($_ =~ /(\S+\-hap)A\s+(.+)/) {
        $hap_num = sprintf("%09d", $hap_num+1);
        $entries_header{$hap_num}{hapA} = $1;
        @{ $haps{$hap_num}{hapA} } = split /\ /,$2;
    } elsif ($_ =~ /(\S+\-hap)B\s+(.+)/) {
        $entries_header{$hap_num}{hapB} = $1;
        @{ $haps{$hap_num}{hapB} } = split /\ /,$2;
    }
}
close HAPS;

my $entries_header = "rs# SNPalleles chrom pos strand genome_build center protLSID assayLSID panelLSID QC_code ";
# rs# SNPalleles chrom pos strand genome_build center protLSID assayLSID panelLSID QC_code NA06985 NA06991 NA06993 NA06993.dup NA06994 NA07000 NA07019 NA07022 NA07029 NA07034 NA07048 NA07055 NA07056 NA07345 NA07348 NA07357 NA10830 NA10831 NA10835 NA10838 NA10839 NA10846 NA10847 NA10851 NA10854 NA10855 NA10856 NA10857 NA10859 NA10860 NA10861 NA10863 NA11829 NA11830 NA11831 NA11832 NA11839 NA11840 NA11881 NA11882 NA11992 NA11993 NA11993.dup NA11994 NA11995 NA12003 NA12003.dup NA12004 NA12005 NA12006 NA12043 NA12044 NA12056 NA12057 NA12144 NA12145 NA12146 NA12154 NA12155 NA12156 NA12156.dup NA12234 NA12236 NA12239 NA12248 NA12248.dup NA12249 NA12264 NA12707 NA12716 NA12717 NA12740 NA12750 NA12751 NA12752 NA12753 NA12760 NA12761 NA12762 NA12763 NA12801 NA12802 NA12812 NA12813 NA12814 NA12815 NA12864 NA12865 NA12872 NA12873 NA12874 NA12875 NA12878 NA12891 NA12892
foreach my $hap (sort keys %coords) {
    $entries_header .= 
        "$entries_header{$hap}{hapA}" . "A" . " " . "$entries_header{$hap}{hapB}" . "B" . " ";
}

my %entries;
foreach my $coord_num (sort keys %coords) {
    $entries{$coord_num}{metainfo} = "$coords{$coord_num}{rs} N/N Chr$chr $coords{$coord_num}{coord} + genome_build center protLSID assayLSID panelLSID QC_code ";
}
%coords = undef;

print STDERR "Generating hapmap entries...\n";
foreach my $snp (1..$snp_size) {
    my $snp_string;
    foreach my $indiv (1..$hap_num) {
        $indiv = sprintf("%09d",$indiv);
        $snp_string .= "${ $haps{$indiv}{hapA}}[($snp-1)]"."${ $haps{$indiv}{hapB}}[($snp-1)]"." ";
    }
    $snp = sprintf("%09d",$snp);
    print "$entries{$snp}{metainfo}";
    print "$snp_string\n";
}

1;
