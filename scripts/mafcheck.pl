#/usr/bin/perl -w

# mafcheck.pl
#
# Cared for by Stephan Hutter <>
#
# Copyright Stephan Hutter
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

mafcheck.pl - check if there is more than 1 block per reference
sequence coordinate

=head1 SYNOPSIS

Give standard usage here

=head1 DESCRIPTION

Describe the object here

=head1 AUTHOR - Stephan Hutter

Email 

Describe contact details here

=head1 CONTRIBUTORS

Albert Vilella <avilella_at_ub_dot_edu>

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;


1;

use FileHandle;


print ("Enter name of MAF file: ");
$file = <>;
chomp ($file);

unless (open (FILE,"< $file")) {
  die "Could not open file\n";
}

print ("Enter name of reference individual: ");
$ref = <>;
chomp ($ref);

$last_start=0;
$last_end=0;
$errors=0;
$line=0;

while (<FILE>) {

    s/^\s+//;
    s/\s+$//;
    $line++;

    if (!($_ =~ /^s/)) {
        # not a sequence
        next;
    }

    @l = split;

    @w = split /\./,$l[1],2;
    $name = $w[0];

    if ($name ne $ref) {
        next;
    }

    $start=$l[2];

    $end=$start + $l[3] - 1;

    if ($last_start == 0) {
        $last_start = $start;
        $last_end = $end;
        next;
    }

    #check the individual
    if ($start <= $last_end) {
        # this is bad!
        $errors++;
        print ("Start value $start in line $line is too low!\n");
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
