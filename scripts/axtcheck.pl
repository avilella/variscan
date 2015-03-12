#!/usr/local/bin/perl -w

# axtcheck.pl
#
# Cared for by Albert Vilella  <avilella_at_ub_dot_edu>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

axtcheck.pl - check if there is more than 1 hit per reference
sequence coordinate

=head1 SYNOPSIS

axtcheck.pl [inputfile.axt]

=head1 DESCRIPTION

Check if there is more than 1 hit per reference sequence coordinate (overlap)

Type of overlaps:

partial

 ------------
    ------------

same start

 -------
 ----------

submatch

 -------------
   ---------


=head1 AUTHOR - Albert Vilella

Email 

Describe contact details here

=head1 CONTRIBUTORS

Stephan Hutter

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;


1;

use FileHandle;

my $file = shift;

if (!$file) {
    print ("Enter name of AXT file: ");
    $file = <>;
    chomp ($file);
}


unless (open (FILE,"< $file")) {
  die "Could not open file\n";
}

my $last_start=0;
my $last_end=0;
my $start=0;
my $end=0;
my $errors=0;
my $line=0;
my @line_split;
my @word_split;
my $type;

while (<FILE>) {

    s/^\s+//;
    s/\s+$//;
    $line++;

    if (!($_ =~ /^\d+/)) {
        # not a sequence
        next;
    }

    @line_split = split;

    @word_split = split /\./,$line_split[1],2;
    $start=$line_split[2];
    $end=$line_split[3];

    if ($last_start == 0) {
        $last_start = $start;
        $last_end = $end;
        next;
    }

    #check
    if ($start <= $last_end) {
        # this is bad!
        $errors++;
        my $overlap_rate = 0;
        if ( ($end < $last_end) ) {
            $overlap_rate = ($end-$start)/($last_end-$last_start);
            $type =     "  submatch";
            if ( ($start == $last_start) ) {
                $type = "same start";
            }
        } elsif ($end >= $last_end) {
            $overlap_rate = ($last_end-$start)/($last_end-$last_start);
            $type =     "   partial";
        } else {
            die "$!";
        }
        $overlap_rate = sprintf("%.3f", $overlap_rate);
        $start = sprintf("%09d", $start);
        $end = sprintf("%09d", $end);
        $last_start = sprintf("%09d", $last_start);
        $last_end = sprintf("%09d", $last_end);
        print ("Overlap: $start-$end $last_start-$last_end rate $overlap_rate type $type (line $line)\n");
    }

    # continue, to see if there is more wrong...
    $last_start = $start;
    $last_end = $end;
}

if ($errors == 0) {
    print ("File is OK!\n");
}
else {
    print ("File contains $errors instances of multiple blocks pointing to an already used coordinate!\n");
}
