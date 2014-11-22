#!/usr/bin/perl

# Formats the input text file for our Cache Simulator
# Removes empty lines from the end of the file and removes excess spaces
#
# Output format is: n address
# Wrriten by Sean Koppenhafer 11/15/2014
# ECE485 Cache Simulator - Luis Santiago, Sean Koppenhafer, Steve Pierce

use warnings;
use strict;
use Getopt::Long;

my $filename = undef;
my $output_filename = "formatted_trace.txt";
my $read_file;
my $write_file;

GetOptions(
				"f=s" => \$filename,
			);

die("Enter the filename of the input you wanted formatted with -f \"filename\"\n") unless defined ($filename);

#Open the files
open($read_file, "<", $filename) or die "Cannot open file $filename\n";
open($write_file, ">", $output_filename) or die "Cannot create file $output_filename\n";

my $formatted_string;

#Read in the unformatted text and format it
while( my $row = <$read_file> ) {
	chomp $row;

	#Looks for rows that have our format but with too many spaces
	#Then makes it so there is only 1 space between operation and address
	if($row =~ m/^(\d)\s+([\da-f]+)/i) {
		$formatted_string = "$1 $2\n";
		print $write_file $formatted_string;
	}
	elsif($row =~ m/^(\d)/) {   #Is a print or reset operation
		$formatted_string = "$1 0\n";	#Provide a dummy address
		print $write_file $formatted_string;
}	}

close($read_file);
close($write_file);

