# vs2bed.pls
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

vs2bed.PLS - Generates a Bed intensity bed file from a vs file

=head1 SYNOPSIS

perl vs2bed.PLS \
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

Will give 3 bed tracks: one with column 9, tagged S, one with column
11, tagged pi, and one with column 12, tagged theta.

=head1 DESCRIPTION

Describe the object here

=head1 AUTHOR - Pablo Librado

Describe contact details here

=head1 CONTRIBUTORS

Additional contributors names here
Albert Vilella

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
$options{'merged'};
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
	   'offset=s'  => \$options{'offset'},
           'merged' =>\$options{'merged'}
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
            print "column ".$chosen_col." can´t be used\n";
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
die "$inputfile cannot be read. Check that the file exists.\n" 
    unless (@inputfile_lines && defined $inputfile_lines[0]);

my @comments;
my @coordinates;
my $stat_col = $options{'stat_col'};
my (%min,%max, %entries);

my ($refstart,$refend,$determined_num_cols);
my (@starts,@ends);
foreach my $chosen_col (values %corresp_hash) {
    $min{$chosen_col} = 9999999999;
    $max{$chosen_col} = -9999999999;
}
foreach my $line (@inputfile_lines) {
    if ($line=~/^\#/) {next;}
    my @values = split (/\s+/, $line);
    $determined_num_cols=scalar(@values)-1;
    push(@starts,$values[1]);
    push(@ends,$values[2]);
    foreach my $chosen_col (values %corresp_hash) {
        next if ($chosen_col>$determined_num_cols);
        next unless (defined $values[$chosen_col]);
        push (@{ $entries{$chosen_col} }, $values[$chosen_col]);
        $min{$chosen_col} = 
            $values[$chosen_col] if ($values[$chosen_col] < $min{$chosen_col});
        $max{$chosen_col} = 
            $values[$chosen_col] if ($values[$chosen_col] > $max{$chosen_col});
    }
}

if ($options{'offset'}) {
    ($refstart,$refend)=($options{'offset'}=~/(\d+)-(\d+)/);
}
my $window=$starts[1]-$starts[0];
my $start= $starts[0]+$refstart;
my $end= $ends[scalar(@ends)-1]+$refend;

@inputfile_lines = ();
undef @inputfile_lines;
#FIXME: add rescale option: rescale inputfile_line scores to 0-100 if
#rescaled option is chosen

print "Generating file header...\n";
my %bed_init;
my $counter = 0;

#FIXME: add commented lines from vs file (see vs2browse.PLS)
foreach my $chosen_col (values %corresp_hash) {
next if ($chosen_col > $determined_num_cols);
    $bed_init{$chosen_col} = "\n";
    $bed_init{$chosen_col} .= <<HERE_TARGET;
#### Generated with vs2bed.PLS
browser position chr$options{'chr'}:$start-$end
HERE_TARGET
$counter++;
}

my %outfile;my $outfile;
$outfile{'dir'} = File::Spec->curdir();
$outfile{'base'} = $inputfile{'base'};
$outfile{'ext'} = 'vs.bed';
$counter = 0;
my %rev_corresp=reverse %corresp_hash;

foreach my $chosen_col (sort(values %corresp_hash)) {
    next if ($chosen_col>$determined_num_cols);
    my $track_init;
    my $fixedStep_init = 
        "fixedStep chrom=chr$options{'chr'} start=$start step=$window span=$window\n";

    if ($options{'merged'} and $counter==0) {
        #FIXME: Use \.pi_theta_{etc}\. instead of alltags
        $outfile = File::Spec->catfile
            ( 
             $outfile{'dir'}, "$outfile{'base'}\.alltags\.$outfile{'ext'}" 
            );
        open OUTFILE, "+>$outfile" or die;
    }
    unless ($options{'merged'}) {
        $outfile = File::Spec->catfile
            ( 
             $outfile{'dir'}, "$outfile{'base'}\.$rev_corresp{$chosen_col}\.$outfile{'ext'}" 
            );
        open OUTFILE, "+>$outfile" or die;
    }
    print "Output file for $rev_corresp{$chosen_col}: $outfile" if ($counter == 0 and $options{'merged'} or ($options{'merged'}==undef));
    print " -- needs GBrowse version >= 1.62 for proper render\n" if (($min{$chosen_col}) < 0);
    print "\n";
    #foreach my $comment (@comments) {
    #    print OUTFILE "$comment\n";
    #}
    print OUTFILE $bed_init{$chosen_col};
    print OUTFILE "##$rev_corresp{$chosen_col}\n";
    my $entry = 0;
    $track_init = "track type=wiggle_0 name=$rev_corresp{$chosen_col} visibility=full color=000,000,255 altColor=255,000,000 gridDefault=on maxHeightPixels=120:60:30 graphType=bar\n";
    print OUTFILE $track_init;
    print OUTFILE $fixedStep_init;
    foreach my $value ( @{$entries{$chosen_col}} ) {
        print OUTFILE "$value\n";
    }
    1;
    close OUTFILE if($options{'merged'}==undef or($options{'merged'} and ($counter+1)==scalar(keys(%corresp_hash))));
    $counter++;
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


