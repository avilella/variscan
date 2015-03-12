#!/usr/bin/perl -w

# gff2bdf.pl
#
# Cared for by Stephan Hutter <>
#
# Copyright Stephan Hutter
#
# You may distribute this module under the same terms as perl itself

# POD documentation - main docs before the code

=head1 NAME

gff2bdf.pl - generate a bdf file from a gff file

=head1 SYNOPSIS

Give standard usage here

=head1 DESCRIPTION

Describe the object here

=head1 AUTHOR - Stephan Hutter

Email 

Describe contact details here

=head1 CONTRIBUTORS

Albert Vilella <avilella_at_ub_dot_edu>

=cut

# Let the code begin...

use strict;


1;

use FileHandle;

STDOUT->autoflush(1);


if (@ARGV != 2) {
  die "Usage: perl gff2bdf.pl INPUT_FILE OUTPUT_FILE\n";
}

#test filenames
my $gff = $ARGV[0];
my $bdf = $ARGV[1];

#standard features
my $coding = "CDS";
my $intron = "intron";
my $gene = "gene";

#set global end
my $global_end = 0;

my $gene_ori = 0;
my @gffline;
my $pattern;
my @sorted_genes;
my @genes;
my $list_ref;
my @list;
my $read_key;
my $count;
my $key;
my $len;
my $num;
my $min;
my $max;
my $dist_5;
my $dist_3;
my $block_len;
my $block_pos;
my $custom;
my $flank_ori;
my $flank_mode;
my $five_flank;
my $three_flank;
my $both_flank;
my $field;
my @fields;
my $id;
my $gene_3;
my $gene_5;
my $IR_start;
my $IR_end;
my $IR_len;
my $BDF_start;
my $BDF_end;
my $rest_space;

# print ("Enter name of input GFF file: ");

# $gff = <>;
# chomp ($gff);

unless (open (GFFFILE,"< $gff")) {
    die "Could not open $gff!\n";
}

# print ("Enter name of output BDF file: ");

# $bdf = <>;
# chomp ($bdf);

unless (open (BDFFILE,"> $bdf")) {
    die "Could not create $bdf!\n";
}


main_menu();

print ("\n");
print ("Please wait...\n");


while (<GFFFILE>) {
    chomp;
    s/#.*//;
    s/^\s+//;
    s/\s+$//;
    next unless length;

    @gffline = split /\s+/,$_,9;

    check_source();
    check_line();
}


if ($pattern == 21 || $pattern =~ /^23/) {

    @sorted_genes = sort { $a->[3] <=> $b->[3] } @genes;

    check_IR();
}


for $list_ref ( sort { $a->[0] <=> $b->[0] } @list ) {
        print BDFFILE ("$$list_ref[0]\t$$list_ref[1]\t$$list_ref[2]\n");
}

close(GFFFILE);
close(BDFFILE);






######################################################################
# SUBROUTINES
######################################################################


######################################################################
# READING STUFF
######################################################################

sub get_key {
    # tried doing this with the Term:ReadKey module but maybe it's better
    # to not depend on external modules
    
    $read_key = "";
    while(length $read_key != 1) {
      $read_key = <STDIN>;
      chomp ($read_key);
    }
    return $read_key;
}

sub get_natural {

    $num = <STDIN>;
    chomp ($num);

    while ($num =~ /\D/ || $num eq '0') {
        print ("Please enter a natural number larger 0 or hit RETURN for default: ");
        $num = <STDIN>;
        chomp ($num);
    }

    return $num;
}


#####################################################################
# MENU
#####################################################################


sub main_menu {

    print ("\n");
    print ("Select regions of interest:\n");
    print ("[1] Coding regions (CDS)\n");
    print ("[2] Non-coding regions\n");
    print ("[3] Custom feature\n");
    print ("[4] Quit\n");

    $key = 0;

    while ($key =~ /[^1-4]/) {
        $key = get_key();
    }

    if ($key == 4) {
        die "Goodbye!\n";
    }
    else {
        $pattern = $key;
        print ("\n");
    }

    if ($pattern == 1) {
        coding_menu();
    }

    elsif ($pattern == 2) {
        non_coding_menu();
    }

    elsif ($pattern == 3) {
        custom_menu();
    }
}

sub coding_menu {
#pattern 1
    ask_limits("coding sequences");
    gene_ori_menu("coding sequences");
}

sub ask_limits {

    print ("\n");
    print ("$_[0] will be filtered...\n");
    print ("Enter minimal length of $_[0] (default = no minimum): ");

    $min = get_natural();

    if ($min eq '') {
        $min = 0;
    }

    print ("\n");
    $max = -1;

    while ($max < $min && $max != 0) {

        print ("Enter maximal length of $_[0] (default = no maximum): ");

        $max = get_natural();

        if ($max eq '') {
            $max = 0;
        }

        if ($max < $min && $max != 0) {
            print ("The maximal length must be larger or equal the minimal length!\n");
        }
    }
}

sub non_coding_menu {
#pattern 2
    print ("\n");

    print ("What kind of non-coding regions?\n");
    print ("[1] Introns and intergenic regions (IR)\n");
    print ("[2] Introns only\n");
    print ("[3] Intergenic regions only\n");
    print ("[4] Quit\n");

    $key = 0;

    while ($key =~ /[^1-4]/) {
        $key = get_key();
    }

    if ($key == 4) {
        die "Goodbye!\n";
    }
    else {
        $pattern .= $key;
#        print ("$pattern\n");
    }

    if ($pattern == 21) {
        intron_IR_menu();
    }

    elsif ($pattern == 22) {
        intron_menu();
    }

    elsif ($pattern == 23) {
        IR_menu();
    }
}

sub intron_IR_menu {
#pattern 21
    ask_limits("introns and IRs");
    $flank_mode = 0;
    $gene_ori = 1;
}

sub intron_menu {
#pattern 22
    ask_limits("introns");
    gene_ori_menu("introns");
}

sub IR_menu {
#pattern 23
    print ("\n");

    print ("How do want to analyse the intergenic regions?\n");
    print ("[1] Analyse IRs within defined size limits\n");
    print ("[2] Analyze blocks of fixed size in IRs\n");
    print ("[3] Analyze blocks with fixed distances to flanking genes\n");
    print ("[4] Quit\n");

    $key = 0;

    while ($key =~ /[^1-4]/) {
        $key = get_key();
    }

    if ($key == 4) {
        die "Goodbye!\n";
    }
    else {
        $pattern .= $key;
#        print ("$pattern\n");
    }

    if ($pattern == 231) {
        IR_limits_menu();
    }

    elsif ($pattern == 232) {
        IR_block_menu();
    }

    elsif ($pattern == 233) {
        IR_position_menu();
    }
}

sub IR_limits_menu {
#pattern 231

    ask_limits("intergenic regions");

    $dist_5 = 0;
    $dist_3 = 0;

    flanking_ori_menu();
}

sub IR_block_menu {
#pattern 232
    print ("\n");
    print ("Enter fixed length of blocks (default: 1000): ");

    $block_len = get_natural;

    if ($block_len eq '') {
        $block_len = 1000;
    }

    block_position_menu();
    flanking_ori_menu();
}

sub IR_position_menu {
#pattern 233

    ask_dist_5();
    ask_dist_3();
    flanking_ori_menu();
}


sub block_position_menu {

    print ("\n");

    print ("Define the position of the blocks inside the intergenic region\n");
    print ("[1] Place block relative to the 5'-flanking gene\n");
    print ("[2] Place block relative to the 3'-flanking gene\n");
    print ("[3] Place block in the middle of the intergenic region\n");
    print ("[4] Quit\n");

    $block_pos = 0;

    while ($block_pos =~ /[^1-4]/) {
        $block_pos = get_key();
    }

    if ($block_pos == 4) {
        die "Goodbye!\n";
    }

    elsif ($block_pos == 1) {
        $dist_3 = 0;
        ask_dist_5();
    }

    elsif ($block_pos == 2) {
        $dist_5 = 0;
        ask_dist_3();
    }

    elsif ($block_pos == 3) {
        $dist_5 = 0;
        $dist_3 = 0;
    }
}

sub ask_dist_5 {

    print("\n");

    print("Enter the number of nucleotides between the block \nand the 5'-flanking gene (default = 0): ");

    $dist_5 = get_natural();

    if ($dist_5 eq '') {
        $dist_5 = 0;
    }
}

sub ask_dist_3 {

    print("\n");

    print("Enter the number of nucleotides between the block \nand the 3'-flanking gene (default = 0): ");

    $dist_3 = get_natural();

    if ($dist_3 eq '') {
        $dist_3 = 0;
    }
}

sub custom_menu {
#pattern 3

    print ("\n");
    print ("Enter the feature to filter: ");

    $custom = <STDIN>;
    chomp ($custom);

    ask_limits($custom);
    gene_ori_menu($custom);
}

sub gene_ori_menu {

    print ("\n");

    print ("On which strand should the $_[0] be located?\n");
    print ("[1] On either strand\n");
    print ("[2] Only on the + strand\n");
    print ("[3] Only on the - strand\n");
    print ("[4] Quit\n");

    while ($gene_ori =~ /[^1-4]/) {
        $gene_ori = get_key();
    }

    if ($gene_ori == 4) {
        die "Goodbye!\n";
    }

    elsif ($gene_ori == 2) {
        $gene_ori = '+';
    }

    elsif ($gene_ori == 3) {
        $gene_ori = '-';
    }
}

sub flanking_ori_menu {

    print ("\n");

    print ("Define the orientation of the flanking genes\n");
    print ("[1] Orientation of flanking genes doesn't matter\n");
    print ("[2] Only the 5'-flanking gene matters\n");
    print ("[3] Only the 3'-flanking gene matters\n");
    print ("[4] Both flanking genes matter\n");
    print ("[5] Quit\n");

    $flank_ori = 0;

    while ($flank_ori =~ /[^1-5]/) {
        $flank_ori = get_key();
    }

    if ($flank_ori == 5) {
        die "Goodbye!\n";
    }

    elsif ($flank_ori == 1) {
        $flank_mode = 0;
    }

    elsif ($flank_ori == 2) {
        $flank_mode = 1;
        $three_flank = 0;
        five_flank_menu();
    }

    elsif ($flank_ori == 3) {
        $flank_mode = 1;
        $five_flank = 0;
        three_flank_menu();
    }

    elsif ($flank_ori == 4) {
        both_flank_menu();
    }
}

sub five_flank_menu {

    print ("\n");

    print ("Enter the orientation of the 5'-flanking gene\n");
    print ("[1] 5'-flanking gene is on the + strand\n");
    print ("[2] 5'-flanking gene is on the - strand\n");
    print ("[3] Quit\n");

    $five_flank = 0;

    while ($five_flank =~ /[^1-3]/) {
        $five_flank = get_key();
    }

    if ($five_flank == 3) {
        die "Goodbye!\n";
    }

    elsif ($five_flank == 1) {
        $five_flank = '+';
    }

    elsif ($five_flank == 2) {
        $five_flank = '-';
    }

}

sub three_flank_menu {

    print ("\n");

    print ("Enter the orientation of the 3'-flanking gene\n");
    print ("[1] 3'-flanking gene is on the + strand\n");
    print ("[2] 3'-flanking gene is on the - strand\n");
    print ("[3] Quit\n");

    $three_flank = 0;

    while ($three_flank =~ /[^1-3]/) {
        $three_flank = get_key();
    }

    if ($three_flank == 3) {
        die "Goodbye!\n";
    }

    elsif ($three_flank == 1) {
        $three_flank = '+';
    }

    elsif ($three_flank == 2) {
        $three_flank = '-';
    }
}

sub both_flank_menu {

    print ("\n");

    print ("Enter the orientation of the flanking genes\n");
    print ("[1] Both flanking genes on the + strand\n");
    print ("[2] Both flanking genes on the - strand\n");
    print ("[3] Both flanking genes on the same strand\n");
    print ("[4] 5'-flanking gene on +, 3'-flanking gene on -\n");
    print ("[5] 5'-flanking gene on -, 3'-flanking gene on +\n");
    print ("[6] Flanking genes on opposite strands\n");
    print ("[7] Quit\n");

    $both_flank = 0;

    while ($both_flank =~ /[^1-7]/) {
        $both_flank = get_key();
    }

    if ($both_flank == 7) {
        die "Goodbye!\n";
    }

    elsif ($both_flank == 1) {
        $flank_mode = 1;
        $five_flank = '+';
        $three_flank = '+';
    }

    elsif ($both_flank == 2) {
        $flank_mode = 1;
        $five_flank = '-';
        $three_flank = '-';
    }

    elsif ($both_flank == 3) {
        $flank_mode = 2;
    }

    elsif ($both_flank == 4) {
        $flank_mode = 1;
        $five_flank = '+';
        $three_flank = '-';
    }

    elsif ($both_flank == 5) {
        $flank_mode = 1;
        $five_flank = '-';
        $three_flank = '+';
    }

    elsif ($both_flank == 6) {
        $flank_mode = 3;
    }
}


#######################################################################
# LINE CHECKING
#######################################################################

sub check_line {

    if ($pattern == 1) {
        check_feature($coding);
    }

    elsif ($pattern == 21) {
        check_feature($intron);
        check_genes();
    }

    elsif ($pattern == 22) {
        check_feature($intron);
    }

    elsif ($pattern =~ /^23/) {
        check_genes();
    }

    elsif ($pattern == 3) {
        check_feature($custom);
    }
}


sub check_feature {


    if (!($gffline[2] =~ /\b$_[0]\b/i)) {
        return;
    }


    if (!check_gene_ori()) {
        return;
    }

    $len = $gffline[4] - $gffline[3] + 1;

    if ($len < $min) {
        return;
    }

    if ($max > 0) {
        if ($len > $max) {
            return;
        }
    }

    filter_feature_id();

    push @list, [($gffline[3], $gffline[4], $id."strand ".$gffline[6])];
}

sub check_gene_ori {

    if ($gene_ori eq '1') {
        return 1;
    }

    if ($gene_ori eq $gffline[6]) {
            return 1;
    }

    return 0;
}

sub filter_feature_id {

    @fields = split /;/,$gffline[8];

    $id = "";

    foreach $field (@fields) {
        $field =~ s/^\s+//;
        $field =~ s/\s+$//;

        if ($field =~ /^ID/i) {
            $id = $field;
            $id =~ s/ID //i;
            $id =~ s/"//g;
            $id .= "; ";
            last;
        }
    }
}

sub check_intron_IR {

    check_feature($intron);
    check_genes();

}

sub check_genes {

    if ($gffline[2] =~ /\b$gene\b/i) {
        push @genes, [@gffline];
    }
}

sub check_IR {

    $count = 0;

    if ($sorted_genes[0]->[3] > 1) {
        unshift @sorted_genes, [(0,0,0,0,0,0,0,0,"Start of chromosome - 1")];
    }

    if ($sorted_genes[$#sorted_genes]->[4] < $global_end) {
        push @sorted_genes, [(0,0,$global_end + 1,0,0,0,0,0,"End of chromosome + 1")];
    }

    foreach $gene_3 (@sorted_genes) {

        $count++;

        if ($count == 1) {
            $gene_5 = $gene_3;
            next;
        }

        $IR_start = $gene_5->[4] + 1;
        $IR_end = $gene_3->[3] - 1;
        $IR_len = $IR_end - $IR_start + 1;

        if ($IR_len <= 0) {
            $gene_5 = $gene_3;
            next;
        }

        if ($IR_start != 1 && $IR_end != $global_end) {
            if (!check_flanking()) {
                $gene_5 = $gene_3;
                next;
            }
        }

        if ($pattern == 21 || $pattern == 231) {
            if (!check_IR_len()) {
                $gene_5 = $gene_3;
                next;
            }

            $BDF_start = $IR_start;
            $BDF_end = $IR_end;
        }

        elsif ($pattern == 232) {
            if (!check_block()) {
                $gene_5 = $gene_3;
                next;
            }

            calc_IR_block();

        }

        elsif ($pattern == 233) {
            if (!check_fixed_dist()) {
                $gene_5 = $gene_3;
                next;
            }

            $BDF_start = $IR_start + $dist_5;
            $BDF_end = $IR_end - $dist_3;
        }


        push @list, [($BDF_start, $BDF_end, "Intergenic region")];

        $gene_5 = $gene_3;

    }

}

sub check_IR_len {

    if ($min > 0) {
        if ($IR_len < $min) {
            return 0;
        }
    }

    if ($max > 0) {
        if ($IR_len > $max) {
            return 0;
        }
    }
    return 1;
}

sub check_block {

    if ($IR_len < $block_len + $dist_5 + $dist_3) {
        return 0;
    }

    return 1;
}

sub check_fixed_dist {

    if ($IR_len <= $dist_5 + $dist_3) {
        return 0;
    }

    return 1;
}

sub calc_IR_block {

    if ($block_pos == 1) {
        $BDF_start = $IR_start + $dist_5;
        $BDF_end = $BDF_start + $block_len - 1;
    }

    elsif ($block_pos == 2) {
        $BDF_end = $IR_end - $dist_3;
        $BDF_start = $BDF_end - $block_len +1;
    }

    elsif ($block_pos == 3) {
        $rest_space = $IR_len - $block_len;
        $BDF_start = int($rest_space / 2) + $IR_start;
        $BDF_end = $BDF_start + $block_len - 1;
    }
}

sub check_flanking {

    if ($flank_mode == 0) {
        return 1;
    }

    elsif ($flank_mode == 1) {
        if (($five_flank ne '0') && ($gene_5->[6] ne $five_flank)) {
            return 0;
        }

        if (($three_flank ne '0') && ($gene_3->[6] ne $three_flank)) {
            return 0;
        }
    }

    elsif ($flank_mode == 2) {
        if ($gene_5->[6] ne $gene_3->[6]) {
            return 0;
        }
    }

    elsif ($flank_mode == 3) {
        if ($gene_5->[6] eq $gene_3->[6]) {
            return 0;
        }
    }

    return 1;
}

sub check_source {

    if ($gffline[2] =~ /\bsource\b/i) {
        $global_end = $gffline[4];
    }
}
