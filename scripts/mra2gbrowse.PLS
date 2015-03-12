#!/usr/local/bin/perl

# mra2gbrowse.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

mra2gbrowse.PLS - Generates a GBrowse intensity xyplot file from a mra
file

=head1 SYNOPSIS

perl mra2gbrowse.PLS input_data_file.mra 

perl mra2gbrowse.PLS 
  input_data_file.mra \
  --chr chr21
  --tag tag
  [--offset 332341]

=head1 DESCRIPTION

This script generates a GBrowse intensity xyplot file from a mra file
resulting from running runMRA.PLS.

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
my $xyplot_init;
$xyplot_init = <<HERE_TARGET;
#### Generated with mra2gbrowse.PLS
[expression]
glyph = xyplot
graph_type=boxes
fgcolor = black
bgcolor = darkslateblue
height=100
min_score = 0
max_score = 110
label=1
key=variscan_MRA_plots_for_$inputfile{'base'}
reference=$options{'chr'}
HERE_TARGET

my %outfile;
$outfile{'dir'} = File::Spec->curdir();
$outfile{'base'} = $inputfile{'base'};
$outfile{'ext'} = 'xyplot';

my $outfile = File::Spec->catfile( $outfile{'dir'}, "$outfile{'base'}\.$outfile{'ext'}" );

my $track_init;

my $tag = $options{'tag'};

open OUTFILE, "+>$outfile" or die;
print "Output file $outfile\n";
print OUTFILE $xyplot_init;
my ($name, $value);
foreach $name (@track_names) {
    print OUTFILE "\n##$name\n";
    my $level_start = $start;
    my $level_end = $start+$window-1;
    foreach $value ( @{$tracks{$name}} ) {
        $value =~ s/\s+$//;
        print OUTFILE "expression\t$name\_$tag\t$level_start\.\.$level_end\tscore=$value\n";
        $level_start += $window;
        $level_end += $window;
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



