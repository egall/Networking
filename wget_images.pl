#!/usr/local/bin/perl

# Image file is a file consisting of webpages to extract images from
open (IMGFILE, 'imgur_files.txt');

$i = 0;
# Each line of input file should be new webpage
while (<IMGFILE>) {
	chomp;
	print "$_\n";
	`wget -P ./ -O QvsA$i.jpg -A.jpg,.jpeg,.gif,.png "$_"`;
	$i++;
}
close(IMGFILE)
