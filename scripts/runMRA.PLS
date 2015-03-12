#!/usr/local/bin/perl

# runMRA.PLS
#
# Cared for by Albert Vilella <>
#
# Copyright Albert Vilella
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

runMRA.PLS - Run the MRA analysis over the results of variscan using LastWave

=head1 SYNOPSIS

perl runMRA.PLS input_file.vs input_config_file.lw.conf

perl runMRA.PLS \
  input_file.vs \
  input_config_file.lw.conf \
  --ref_col 3 \
  --stat_col 12 \
  --sub_a 5 \
  --sub_b 8 \
  --sub_c 10 \
  --lwpath /path/to/LastWave_2_0_4 \
  [--filter D4.o] \
  [--lwsourcedir /path/to/lastwave_scripts_dir] \
  [--lwbinary /path/to/LastWave_2_0_4/bin/arch/lw] \

=head1 DESCRIPTION

This perl script collects the options for the MRA analysis, edits the
lastwave script for the analysis and executes LastWave with the edited
script.

This results in a file with the reconstructed signals from the MRA,
that can be used, for example, to generate the wiggle tracks in
vgenomic web browsers.

=head1 AUTHOR - Albert Vilella

=head1 CONTRIBUTORS

Additional contributors names and emails here

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
$options{'ref_col'} = 0;
$options{'sub_a'} = 0;
$options{'sub_b'} = 0;
$options{'sub_c'} = 0;
$options{'filter'} = '';
$options{'lwpath'} = '';
$options{'lwsourcedir'} = '';
$options{'lwbinary'} = '';

#Override options from command line
GetOptions(
	   'stat_col=i'  => \$options{'stat_col'},
	   'ref_col=i'  => \$options{'ref_col'},
           'sub_a=i' => \$options{'sub_a'},
           'sub_b=i' => \$options{'sub_b'},
           'sub_c=i' => \$options{'sub_c'},
           'filter=s' => \$options{'filter'},
           'lwpath=s' => \$options{'lwpath'},
           'lwsourcedir=s' => \$options{'lwsourcedir'},
           'lwbinary=s' => \$options{'lwbinary'},
          );

#Read config file
my $inputfile = shift;
my $configfile = shift;
my %inputfile;
my %configfile;
($inputfile{'base'}, $inputfile{'dir'}, $inputfile{'ext'}) = fileparse($inputfile,'\.\w+?');
($configfile{'base'}, $configfile{'dir'}, $configfile{'ext'}) = fileparse($configfile,'\.\w+?');

print "Reading config file...\n";
my @configfile_lines = read_file( $configfile, err_mode => 'quiet' ) ;
carp( "$configfile can't be read. Check that the file exists.\n" ) unless @configfile_lines && defined $configfile_lines[0] ;

#Read configfile
foreach $_ (@configfile_lines) {
    next if (/^#/);
    if ( /(\S+)\s*=\s*(\S+)\s*/ ) {
        if (exists $options{$1}) {
            $options{$1} = $2;
        }
    }
}

@configfile_lines = ();
undef @configfile_lines;

$ENV{LWPATH}="$options{'lwpath'}";

my $lwsourcedir = "$options{'lwsourcedir'}";
unless ($lwsourcedir) {
    $lwsourcedir = File::Spec->catfile( $options{'lwpath'}, "scripts");
    $ENV{LWSOURCEDIR} = $lwsourcedir;
}
my $lwbinary = "$options{'lwbinary'}";
$lwbinary = `echo -n $lwbinary`;
unless ($lwbinary) {
    if (($^O =~ /dec_osf|linux|unix|bsd|solaris|darwin/i)) {
        my $arch = $^O;
        $lwbinary = File::Spec->catfile( $options{'lwpath'}, "bin", "$arch", "lw" );
    }
}
if( !-x $lwbinary ) {
    #FIXME die if not executable
#    die "Failed to find LastWave executable in $lwbinary. Check lwpath option: $!";
}


# Extract columns from input file
print "Reading input file $inputfile (may take a while)...\n";
my @inputfile_lines = read_file( $inputfile, err_mode => 'quiet' ) ;
carp( "$inputfile can't be read. Check that the file exists.\n" ) unless @inputfile_lines && defined $inputfile_lines[0] ;

my @comments;
my @entries;
my $start = -1;
my $end = $2;
my $pattern;
my $regexp;
my $stat_col = $options{'stat_col'};
my $ref_col = $options{'ref_col'};

#Building regexp
$pattern = '\s*';
my $counter = $ref_col;
while ($counter - 1 > 0) {
    $pattern .= '\S+\s+';
    $counter--;
}
$pattern .= '(\S+)\s+';
$counter = $stat_col - $ref_col - 1;
while ($counter > 0) {
    $pattern .= '\S+\s+';
    $counter--;
}
$pattern .= '(\S+)';

foreach $_ (@inputfile_lines) {
    if (/^\#/) {
        push @comments, $_;
        next;
    }
    $regexp = qr/^${pattern}/i;
    if ($_ =~ /$regexp/) {
        my $index = sprintf("%010d", $1);
        push @entries, "$index\t$2\n";
        if ($start == -1) {
            $start = $1;
        }
    } else {
        die "error while parsing inputfile, line: \n$_\n,  $!";
    }
    $end = $1;
}

@inputfile_lines = ();
undef @inputfile_lines;


print "Checking if subdivisions are adequate...\n";
my $statsize = scalar(@entries);
my $stat_octaves;

#Counting octaves
while (2**($stat_octaves+1) < $statsize) {
    $stat_octaves++;
}

if (($options{'sub_a'} > $stat_octaves) ||
    ($options{'sub_b'} > $stat_octaves) ||
    ($options{'sub_c'} > $stat_octaves)
   ) {die "Do not set any of the subdivisions to more than $stat_octaves for this input file: $!";}

if (($options{'sub_a'} > $options{'sub_b'}) ||
    ($options{'sub_b'} > $options{'sub_c'})
   ) {die "Subdivisions should be sub_a <= sub_b <= sub_c: $!";}

#Creating lastwave input file
my %outfile;
$outfile{'dir'} = File::Spec->curdir();
$outfile{'base'} = $inputfile{'base'};
$outfile{'ext'} = 'col';
my $outfile = File::Spec->catfile( $outfile{'dir'}, "$outfile{'base'}\.$outfile{'ext'}" );

open OUTFILE, "+>$outfile" or die;

foreach $_ (@entries) {
    print OUTFILE $_;
}
close OUTFILE;

#Running LastWave
my $script_init = '';
my %mrafile;
$mrafile{'dir'} = File::Spec->curdir();
$mrafile{'base'} = $inputfile{'base'};
$mrafile{'ext'} = 'mra';

my $mrafile = File::Spec->catfile( $mrafile{'dir'}, "$mrafile{'base'}\.$mrafile{'ext'}" );

($script_init = <<HERE_TARGET) =~ s/^\s+//g;
    # This options come from runMRA.PLS
    #
    # Name of the input file
    inputfile = "$outfile"
    # Name of the output file for the MRA analysis
    mrafile = "$mrafile"
    #Number of levels
    levels = $stat_octaves
    # Subdivisions
    sub_a = $options{'sub_a'}
    sub_b = $options{'sub_b'}
    sub_c = $options{'sub_c'}
    # Wavelets mother filter
    filter = "$options{'filter'}"
    #
    # Options come from runMRA.PLS
HERE_TARGET

print "Creating LastWave script...\n";
my %lwscript;
$lwscript{'dir'} = File::Spec->curdir();
$lwscript{'base'} = $inputfile{'base'};
$lwscript{'ext'} = 'lws';
my $lwscript = File::Spec->catfile( $lwscript{'dir'}, "$lwscript{'base'}\.$lwscript{'ext'}" );

open LWSCRIPT, "+>$lwscript" or die;
print LWSCRIPT $script_init;

while(<DATA>) {
    next if /^\#/;
    print LWSCRIPT $_;
}

close LWSCRIPT;

# Creating LastWave call for execution
my %logfile;
$logfile{'dir'} = File::Spec->curdir();
$logfile{'base'} = $inputfile{'base'};
$logfile{'ext'} = 'log';
my $logfile = File::Spec->catfile( $logfile{'dir'}, "$logfile{'base'}\.$logfile{'ext'}" );

my $commandstring = "$lwbinary -b < $lwscript > $logfile";
printf("Running LastWave with the following command (may take a while):\n");
printf("# $commandstring\n");
# FIXME put startup stuff inside lwscript
run($commandstring);

print "Output file $mrafile is being written\n";

unlink $outfile;
unlink $lwscript;
unlink $logfile;

#Report results
print "Done.\n\n";

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

sub run {
    my @args = @_;
    my $sysstat = system(@args);
    if ($sysstat == -1) {
        print "failed to execute: $!\n";
    }
    elsif ($sysstat & 127) {
        printf "child died with signal %d, %s coredump\n",
            ($sysstat & 127),  ($sysstat & 128) ? 'with' : 'without';
    }
    else {
        #      successfull run
        #      printf "child exited with value %d\n", $sysstat >> 8;
    }
}


#################################################################
#LastWave script template

__DATA__

# Script template for running MRA in LastWave
# Albert Vilella

verbose = 2
if (verbose > 0) { printf '----runMRA-- Defining new wtrans w        \n\n'}
w = [new &wtrans]

if (verbose > 0) { printf '----runMRA-- Defining new wtrans w2        \n\n'}
w2 = [new &wtrans]

if (verbose > 0) { printf '----runMRA-- Defining new wtrans w3        \n\n'}
w3 = [new &wtrans]

if (verbose > 0) { printf '----runMRA-- Defining new wtrans w4        \n\n'}
w4 = [new &wtrans]

if (verbose > 0) { printf '----runMRA-- Defining new wtrans w5        \n\n'}
w5 = [new &wtrans]

if (verbose > 0) { printf '----runMRA-- Defining new signal stat        \n\n'}
stat = [new &signal]

if (verbose > 0) { printf '----runMRA-- Defining new signal s2        \n\n'}
s2 = [new &signal]

if (verbose > 0) { printf '----runMRA-- Defining new signal s3        \n\n'}
s3 = [new &signal]

if (verbose > 0) { printf '----runMRA-- Defining new signal s4        \n\n'}
s4 = [new &signal]

if (verbose > 0) { printf '----runMRA-- Defining new signal s5        \n\n'}
s5 = [new &signal]

if (verbose > 0) { printf '----runMRA-- Defining new signal ref        \n\n'}
ref = [new &signal]

if (verbose > 0) { printf '----runMRA-- Reading dataset           \n\n'}

if (verbose > 0) { printf '----runMRA-- Reading stat           \n\n'}
read stat inputfile 2

if (verbose > 0) { printf '----runMRA-- creating ref signal      \n\n'}
read ref inputfile 1

if (verbose > 0) { printf '----runMRA-- Setting orthogonal wavelet filter          \n\n'}
owtf w filter

if (verbose > 0) { printf '----runMRA-- padding signal to be of size 2^n      \n\n'}
stat_oldsize = stat.size

if (verbose > 0) { printf '----runMRA-- calculating number of octaves      \n\n'}
stat_octaves = 2
while (2^(stat_octaves+1) < stat.size) { stat_octaves = stat_octaves + 1 }
if (stat_octaves > 19) { stat_octaves = 19 }

if (verbose > 0) { printf '----runMRA-- octaves 1 is high freq and octave $stat_octaves is low freq \n\n'}

if (verbose > 0) { printf '----runMRA-- padding signal to be of size 2^n      \n\n'}
padd stat *bmirror
stat_newsize = stat.size
offset = ceil((stat_newsize-stat_oldsize)/2)

if (verbose > 0) { printf '----runMRA-- Orthogonal wavelet decomposition of signal    \n\n'}
w.A[0,0] = stat
owtd w stat_octaves
copy w w2
copy w w3
copy w w4
copy w w5

if (verbose > 0) { printf '----runMRA-- Defining octaves to separate components         \n\n'}

w2_first = 1
w2_last = sub_a - 1
w3_first = sub_a
w3_last = sub_b - 1
w4_first = sub_b
w4_last = sub_c - 1
w5_first = sub_c
w5_last = stat_octaves

if (verbose > 0) { printf '----avb-- separating components         \n\n'}

#w2
foreach i (sub_a:1:stat_octaves) {
	w2.D[i,0]=0:#w2.D[i,0].size:0
	w2.A[i,0]=0:#w2.A[i,0].size:0
}

#w3
foreach i (1:1:sub_a-1) {
	w3.D[i,0]=0:#w3.D[i,0].size:0
	w3.A[i,0]=0:#w3.A[i,0].size:0
}
foreach i (sub_b:1:stat_octaves) {
	w3.D[i,0]=0:#w3.D[i,0].size:0
	w3.A[i,0]=0:#w3.A[i,0].size:0
}

#w4
foreach i (1:1:sub_b-1) {
	w4.D[i,0]=0:#w4.D[i,0].size:0
	w4.A[i,0]=0:#w4.A[i,0].size:0
}
foreach i (sub_c:1:stat_octaves) {
	w4.D[i,0]=0:#w4.D[i,0].size:0
	w4.A[i,0]=0:#w4.A[i,0].size:0
}

#w5
foreach i (1:1:sub_c-1) {
	w5.D[i,0]=0:#w5.D[i,0].size:0
	w5.A[i,0]=0:#w5.A[i,0].size:0
}


if (verbose > 0) { printf '----avb-- Reconstructing decomposed signal         \n\n'}
owtr w2 s2
owtr w3 s3
owtr w4 s4
owtr w5 s5

if (verbose > 0) { printf '----avb-- Reference positions         \n\n'}
padded_start = offset
padded_end = offset+stat_oldsize
refstart = ref[0]
refend = ref[(ref.size)-1]
window = ref[1]-ref[0]

# mra tracks

if (verbose > 0) { printf '----avb-- Generating mra tracks file. May take a while      \n\n'}
k = round(stat_oldsize/10)

printf "runMRA Header\n" :: >$mrafile
printf "reference_positions %d-%d\n" refstart refend :: >>$mrafile
printf "total_num_levels %d\n" stat_octaves :: >>$mrafile
printf "name mra_lev_%d-%d\n" w2_first w2_last  :: >>$mrafile
printf "size %d\n" stat_oldsize :: >>$mrafile
printf "window %d\n" window :: >>$mrafile
printf "min %f\n" min(s2) :: >>$mrafile
printf "max %f\n" max(s2) :: >>$mrafile
printf "binary no\n" :: >>$mrafile
printf "End of Header\n" :: >>$mrafile
if (verbose > 1) {printf "name mra_lev_%d-%d\n" w2_first w2_last}
foreach i padded_start:1:(padded_end - 1) {
	value = s2[i]
	if ((verbose > 1) && ((i%k) == 0)) {printf "*"}
	printf "%1.6f\n" value :: >>$mrafile
}
if (verbose > 1) {printf "\n"}

if (sub_a != sub_b) {
    printf "runMRA Header\n" :: >>$mrafile
    printf "reference_positions %d-%d\n" refstart refend :: >>$mrafile
    printf "total_num_levels %d\n" stat_octaves :: >>$mrafile
    printf "name mra_lev_%d-%d\n" w3_first w3_last  :: >>$mrafile
    printf "size %d\n" stat_oldsize :: >>$mrafile
    printf "window %d\n" window :: >>$mrafile
    printf "min %f\n" min(s3) :: >>$mrafile
    printf "max %f\n" max(s3) :: >>$mrafile
    printf "binary no\n" :: >>$mrafile
    printf "End of Header\n" :: >>$mrafile
    if (verbose > 1) {printf "name mra_lev_%d-%d\n" w3_first w3_last}
    foreach i padded_start:1:(padded_end - 1) {
    	value = s3[i]
    	if ((verbose > 1) && ((i%k) == 0)) {printf "*"}
    	printf "%1.6f\n" value :: >>$mrafile
    }
    if (verbose > 1) {printf "\n"}
}

if (sub_a != sub_c) {
    printf "runMRA Header\n" :: >>$mrafile
    printf "reference_positions %d-%d\n" refstart refend :: >>$mrafile
    printf "total_num_levels %d\n" stat_octaves :: >>$mrafile
    printf "name mra_lev_%d-%d\n" w4_first w4_last  :: >>$mrafile
    printf "size %d\n" stat_oldsize :: >>$mrafile
    printf "window %d\n" window :: >>$mrafile
    printf "min %f\n" min(s4) :: >>$mrafile
    printf "max %f\n" max(s4) :: >>$mrafile
    printf "binary no\n" :: >>$mrafile
    printf "End of Header\n" :: >>$mrafile
    if (verbose > 1) {printf "name mra_lev_%d-%d\n" w4_first w4_last}
    foreach i padded_start:1:(padded_end - 1) {
    	value = s4[i]
    	if ((verbose > 1) && ((i%k) == 0)) {printf "*"}
    	printf "%1.6f\n" value :: >>$mrafile
    }
    if (verbose > 1) {printf "\n"}
}

printf "runMRA Header\n" :: >>$mrafile
printf "reference_positions %d-%d\n" refstart refend :: >>$mrafile
printf "total_num_levels %d\n" stat_octaves :: >>$mrafile
printf "name mra_lev_%d-%d\n" w5_first w5_last  :: >>$mrafile
printf "size %d\n" stat_oldsize :: >>$mrafile
printf "window %d\n" window :: >>$mrafile
printf "min %f\n" min(s5) :: >>$mrafile
printf "max %f\n" max(s5) :: >>$mrafile
printf "binary no\n" :: >>$mrafile
printf "End of Header\n" :: >>$mrafile
if (verbose > 1) {printf "name mra_lev_%d-%d\n" w5_first w5_last}
foreach i padded_start:1:(padded_end - 1) {
	value = s5[i]
	if ((verbose > 1) && ((i%k) == 0)) {printf "*"}
	printf "%1.6f\n" value :: >>$mrafile
}
if (verbose > 1) {printf "\n"}

# mra tracks end

# End of LastWave script template
