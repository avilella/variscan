#!/usr/local/bin/perl
use strict;

# filter_overlaps.PLS
#
# Cared for by Albert Vilella <avilella at ub dot edu>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

filter_overlaps.PLS - filters multiple alignments for which there are
overlapped subalignments (hits) on the same reference sequence
position. Generates a new alignment with only the best hit in terms of
either score or coverage ("mode" option).

=head1 SYNOPSIS

  #filter_overlaps.PLS -format axt -file chr2.axt -mode score > chr2.bestscores.axt

=head1 DESCRIPTION

This script filters multiple alignment results discarding those
overlapped subaligments (hits) that cover the same reference sequence
position. The chosen best hit is the one that's covering a wider range
for a group of initial reference position hits (best coverage mode)
or the one that choses the best score (best score mode).


=head1 AUTHOR - Albert Vilella

Email avilella at ub dot edu

=cut1

=head1 CONTRIBUTORS

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;


1;
