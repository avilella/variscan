#!/usr/local/bin/perl

# mra2bed.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

mra2bed.PLS - Generates a UCSC wiggle bed file from a mra file

=head1 SYNOPSIS

perl mra2bed.PLS input_data_file.mra 

perl mra2bed.PLS 
  input_data_file.mra \
  --chr chr21 \
  --tag tag \
  [--offset 332341]

=head1 DESCRIPTION

This script generates a UCSC wiggle bed file from a mra file resulting
from running runMRA.PLS.

The chromosome identifier will be something like 'chr21', for tracks
that map to chr21 in the species you are studying.

The offset (optional) will mark the point from which to start the
track in the chromosome. For example, if you data is from a region
that starts at position 3323415, the first reference in the track will
be (3323415 + start), where 'start' is the value specified at the
runMRA Header in the mra file.

=head1 AUTHOR - Albert Vilella

Email 

Describe contact details here

=head1 CONTRIBUTORS

Additional contributors names and emails here

=cut


# Let the code begin...

use strict;
use Carp;           #Used for read_file and write_file
use Fcntl;          #Used for read_file and write_file
use File::Basename; #Used for filename and dirname parsing
use File::Spec;     #Used for filename and dirname parsing
use Getopt::Long;   #Used for GetOptions

my $inputfile = shift;
my %inputfile;
($inputfile{'base'}, $inputfile{'dir'}, $inputfile{'ext'}) = fileparse($inputfile,'\.\w+?');

#Options
my %options;
$options{'chr'} = '';
$options{'tag'} = '';
$options{'off_set'} = 0;

#Override options from command line
GetOptions(
	   'chr=s'  => \$options{'chr'},
	   'tag=s'  => \$options{'tag'},
	   'offset=i'  => \$options{'offset'},
          );

#Extract columns from input file
print "Reading input file $inputfile (may take a while)...\n";
my @inputfile_lines = read_file( $inputfile, err_mode => 'quiet' ) ;
carp( "$inputfile can't be read\n" ) unless @inputfile_lines && defined $inputfile_lines[0] ;

my ($start, $end, $name, $window);
my %tracks;
my @track_names;
foreach $_ (@inputfile_lines) {
    next if (/^\#/);
    if (/reference_positions (\d+)-(\d+)/) {
        ($start,$end) = ($1, $2);
    } elsif (/name (\S+)/) {
        $name = $1;
        push @track_names, $name;
    } elsif (/window (\d+)/) {
        $window = $1;
    } elsif (/^-?\d/) {
        push( @{$tracks{$name}}, $_ );
    }
}

my ($chrstart, $chrend);
$chrstart = $start+$options{'offset'};
$chrend = $end+$options{'offset'};
my $bed_init;
$bed_init = <<HERE_TARGET;
#### Generated with mra2bed.PLS
browser position $options{'chr'}:$chrstart-$chrend
HERE_TARGET

my %outfile;
$outfile{'dir'} = File::Spec->curdir();
$outfile{'base'} = $inputfile{'base'};
$outfile{'ext'} = 'bed';

my $outfile = File::Spec->catfile( $outfile{'dir'}, "$outfile{'base'}\.$outfile{'ext'}" );

my $track_init;
my $fixedStep_init = "fixedStep chrom=$options{'chr'} start=$start step=$window span=$window\n";
my $tag = $options{'tag'};

open OUTFILE, "+>$outfile" or die;
print "Output file $outfile\n";
print OUTFILE $bed_init;
my ($name, $value);
foreach $name (@track_names) {
    $track_init = "track type=wiggle_0 name=$name$tag visibility=full color=000,000,255 altColor=255,000,000 gridDefault=on maxHeightPixels=120:60:30 graphType=bar\n";
    print OUTFILE $track_init;
    print OUTFILE $fixedStep_init;
    foreach $value ( @{$tracks{$name}} ) {
        $value =~ s/\s+$//;
        print OUTFILE "$value\n";
    }
}

close OUTFILE;

1;

################################################################################
## FUNCTIONS
################################################################################

sub read_file {

  my( $file_name, %args ) = @_ ;

  my $buf ;
  my $buf_ref = $args{'buf_ref'} || \$buf ;

  my $mode = O_RDONLY ;
  $mode |= O_BINARY if $args{'binmode'} ;

  local( *FH ) ;
  sysopen( FH, $file_name, $mode ) or
      carp "Can't open $file_name: $!" ;

  my $size_left = -s FH ;

  while ( $size_left > 0 ) {

    my $read_cnt = sysread( FH, ${$buf_ref},
                            $size_left, length ${$buf_ref} ) ;

    unless( $read_cnt ) {

      carp "read error in file $file_name: $!" ;
      last ;
    }

    $size_left -= $read_cnt ;
  }

  # handle void context (return scalar by buffer reference)
  return unless defined wantarray ;

  # handle list context
  return split(m|$/|g, ${$buf_ref}) if wantarray ;

  # handle scalar context
  return ${$buf_ref} ;
}



