# vs2gbrowse.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

vs2gbrowse.PLS - Generates a GBrowse intensity xyplot file from a vs
file

=head1 SYNOPSIS

perl vs2gbrowse.PLS \
  input_file.vs \
  --stat_col 11:12 \
  --chr 21 \
  [--tag pi:theta] \
  [--offset 23243] \

Options in [option] are optional.

Stat_col can be called with one column number or with more than one
column separated by ":". The tag given to the column will correspond to
the tag in "--tag" option. Example:

  --stat_col 9:11:12
  --tag S:pi:theta

Will give 3 xyplot files: one with column 9, tagged S, one with column
11, tagged pi, and one with column 12, tagged theta.

=head1 DESCRIPTION

Describe the object here

=head1 AUTHOR - Albert Vilella

Email

Describe contact details here

=head1 CONTRIBUTORS

Pablo Librado

=cut


# Let the code begin...

use strict;
use Carp;           #Used for read_file and write_file
use Fcntl;          #Used for read_file and write_file
use Env;            #Used for expanding shell variables in LW directories
use File::Basename; #Used for filename and dirname parsing
use File::Spec;     #Used for filename and dirname parsing
use Getopt::Long;   #Used for GetOptions

#Options
my %options;
$options{'stat_col'} = 0;
$options{'chr'} = '';
$options{'offset'} = 0;
my $name = '';

#Read config file
my $inputfile = shift;
my %inputfile;

($inputfile{'base'}, $inputfile{'dir'}, $inputfile{'ext'}) =
    fileparse($inputfile,'\.\w+?');

#Read Options from command line
GetOptions(
	   'stat_col=s'  => \$options{'stat_col'},
	   'chr=s'  => \$options{'chr'},
	   'tag=s'  => \$options{'tag'},
	   'offset=i'  => \$options{'offset'},
          );
my $counter=0;
my %corresp_hash;
my @stat_cols = split /\:/,$options{'stat_col'} if ($options{'stat_col'});
my @tag_names=split /\:/,$options{'tag'} if ($options{'tag'});
if (scalar(@tag_names)<=scalar(@stat_cols)) {
    foreach my $chosen_col (@stat_cols){
        if ($chosen_col>=6) {
          unless($tag_names[$counter]) {
            print "Unexistent tag for column " .
                $chosen_col .
                    ", the column number will be used as the tag instead.\n";
            $tag_names[$counter]=$chosen_col;
        }
            $corresp_hash{$tag_names[$counter]}=$chosen_col;
        } elsif ($chosen_col<6) {
            print "column ".$chosen_col." cannot be used\n";
        }
        $counter++;
    }
} elsif (scalar(@tag_names)>scalar(@stat_cols)) {
    die "Different number of tags and number of chosen columns have been specified. Please check\n";
  }
#Extract columns from input file
print "Reading input file (might take a while)...\n";
my @inputfile_lines =
    read_file( $inputfile, err_mode => 'quiet' ) ;
die "$inputfile can't be read. Check that the file exists.\n"
    unless (@inputfile_lines && defined $inputfile_lines[0]);

my @comments;
my @coordinates;
my $stat_col = $options{'stat_col'};
my (%entries,%inits,%min,%max);

my ($refstart,$refend,$stat,$min_score,$max_score, $determined_num_cols);
foreach my $chosen_col (values %corresp_hash) {
    $min{$chosen_col} = 9999999999;
    $max{$chosen_col} = -9999999999;
}

foreach $_ (@inputfile_lines) {
    if (/^\#/) {
        push @comments, $_;
        next;
    }
    my @values = split (/\s+/, $_);
    $determined_num_cols = scalar(@values)-1;
    $refstart = sprintf("%010d", $values[1]);
    $refend = sprintf("%010d", $values[2]);
    push @coordinates, "$refstart\.\.$refend";
    foreach my $chosen_col (values %corresp_hash) {
        next if ($chosen_col > $determined_num_cols);
        push (@{ $entries{$chosen_col} }, $values[$chosen_col]);
        $min{$chosen_col} = 
            $values[$chosen_col] if ($values[$chosen_col] < $min{$chosen_col});
        $max{$chosen_col} = 
            $values[$chosen_col] if ($values[$chosen_col] > $max{$chosen_col});
    }
}

@inputfile_lines = ();
undef @inputfile_lines;
#FIXME: add rescale option: rescale inputfile_line scores to 0-100 if
#rescaled option is chosen

print "Generating file header...\n";
my %xyplot_init;
my $counter = 0;
if ($options{'tag'}) {
@tag_names = split /\:/,$options{'tag'} ;
} else {
@tag_names = sort {$a <=> $b} keys %entries;
}
foreach my $chosen_col (values %corresp_hash) {
next if ($chosen_col > $determined_num_cols);
$xyplot_init{$chosen_col} = "\n";
$xyplot_init{$chosen_col} .= <<HERE_TARGET;
#### Generated with vs2gbrowse.PLS
[expression]
glyph = xyplot
graph_type = boxes
fgcolor = darkslateblue
bgcolor = white
height = 100
min_score = $min{$chosen_col}
max_score = $max{$chosen_col}
label = 1
key = $tag_names[$counter].$inputfile{'base'}
reference = chr$options{'chr'}
HERE_TARGET
$counter++;
}

my %outfile;
$outfile{'dir'} = File::Spec->curdir();
$outfile{'base'} = $inputfile{'base'};
$outfile{'ext'} = 'vs.xyplot';

$counter = 0;
foreach my $chosen_col (sort {$a <=> $b} keys %entries ) {
    next if ($chosen_col>$determined_num_cols);
    my $outfile = File::Spec->catfile
        ( 
         $outfile{'dir'}, "$outfile{'base'}\.$tag_names[$counter]\.$outfile{'ext'}" 
        );
    open OUTFILE, "+>$outfile" or die;
    print "Output file for $tag_names[$counter]: $outfile";
    print " -- needs GBrowse version >= 1.62 for proper render\n" if (($min{$chosen_col}) < 0);
    print "\n";
    foreach my $comment (@comments) {
        print OUTFILE "$comment\n";
    }
    print OUTFILE $xyplot_init{$chosen_col};
    print OUTFILE "\n##$tag_names[$counter]\n";
    my $entry = 0;
    foreach my $coord_pair (@coordinates) {
        print OUTFILE "expression\t$tag_names[$counter]\t$coord_pair\tscore=$entries{$chosen_col}[$entry]\n" 
            unless ($entries{$chosen_col}[$entry] =~ /nan/);
        $entry++;
    }
    $counter++;
    close OUTFILE;
}

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


