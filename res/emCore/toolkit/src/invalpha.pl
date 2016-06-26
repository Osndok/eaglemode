#!/usr/bin/perl
#-------------------------------------------------------------------------------
# invalpha.pl
#
# Copyright (C) 2015 Oliver Hamann.
#
# Homepage: http://eaglemode.sourceforge.net/
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 3 as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
# more details.
#
# You should have received a copy of the GNU General Public License version 3
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------

use strict;
use warnings;
use File::Temp;

#------------------------------- Parse arguments -------------------------------

my $helpText =
	"usage: $0  <source png file> <target png file>\n".
	"This programs performs the inverse operation of painting an alpha image\n".
	"on a background. It takes the <source png file> as the result of an\n".
	"alpha operation, and generates <target png file> as the corresponding\n".
	"alpha image, when painted on a certain background. That background is\n".
	"assumed to be uni-colored with the color value of the pixel at (0,0) of\n".
	"the <source png file>. Thus, the pixel at (0,0) is assumed to be fully\n".
	"transparent. Source and target files can have a precision of 16 bits per\n".
	"color component. Any alpha channel in the source file is ignored.\n"
;

if (@ARGV != 2) {
	print(STDERR $helpText);
	exit(1);
}
my $srcPngFile = $ARGV[0];
my $tgtPngFile = $ARGV[1];


#----------------------- Invent names of temporary files -----------------------

my $tmpDir = File::Temp->newdir(); # Removed automatically

my $srcPnmFile = $tmpDir."/src.pnm";
my $rgbPnmFile = $tmpDir."/rgb.pnm";
my $alphaPgmFile = $tmpDir."/alpha.pgm";


#------------- Convert source png to pnm, open it, and read header -------------

if (system("pngtopnm < $srcPngFile > $srcPnmFile") != 0) {
	exit(1);
}

my $srcHandle;
if (!open($srcHandle,$srcPnmFile)) {
	print(STDERR "Could not read $srcPnmFile: ".$!."\n");
	exit(1);
}

my $line=readline($srcHandle);
if ($line !~ /^P([1-6])$/) {
	print(STDERR "Unexpected format in $srcPnmFile: $line\n");
	exit(1);
}
my $srcFormat=$1;

$line=readline($srcHandle);
if ($line !~ /^([0-9]+) ([0-9]+)$/) {
	print(STDERR "Unexpected size in $srcPnmFile: $line\n");
	exit(1);
}
my $srcWidth=$1;
my $srcHeight=$2;

$line=readline($srcHandle);
if ($line !~ /^([0-9]+)$/) {
	print(STDERR "Unexpected maxVal in $srcPnmFile: $line\n");
	exit(1);
}
my $srcMaxVal=$1;


#----------------- Open target pnm/pgm files and write headers -----------------

my $tgtMaxVal=65535;

my $rgbHandle;
open($rgbHandle,">",$rgbPnmFile);
print($rgbHandle "P6\n");
print($rgbHandle "$srcWidth $srcHeight\n");
print($rgbHandle "$tgtMaxVal\n");

my $alphaHandle;
open($alphaHandle,">",$alphaPgmFile);
print($alphaHandle "P5\n");
print($alphaHandle "$srcWidth $srcHeight\n");
print($alphaHandle "$tgtMaxVal\n");


#-------------------- Read, calculate, and write pixels... ---------------------

sub readVal
{
	my $fh=shift;
	if ($srcMaxVal <= 255) {
		my $val;
		read($fh, $val, 1);
		return ord($val)/$srcMaxVal;
	}
	else {
		my $hi;
		my $lo;
		read($fh, $hi, 1);
		read($fh, $lo, 1);
		return int(ord($hi)*256+ord($lo))/$srcMaxVal;
	}
}

sub writeVal
{
	my $fh=shift;
	my $val=shift;
	if ($val<0.0) { $val=0.0; }
	if ($val>1.0) { $val=1.0; }
	$val=int($val*$tgtMaxVal+0.5);
	print($fh chr(int($val/256)));
	print($fh chr(int($val%256)));
}

my $bgRed = 0.0;
my $bgGrn = 0.0;
my $bgBlu = 0.0;

for (my $i=0; $i<$srcWidth*$srcHeight; $i++) {
	my $srcRed = readVal($srcHandle);
	my $srcGrn = readVal($srcHandle);
	my $srcBlu = readVal($srcHandle);

	if ($i==0) {
		$bgRed = $srcRed;
		$bgGrn = $srcGrn;
		$bgBlu = $srcBlu;
	}

	my $alpha = 0.0;
	my $t;

	if ($srcRed>$bgRed) { $t=($srcRed-$bgRed)/(1-$bgRed); }
	elsif ($srcRed<$bgRed) { $t=1-$srcRed/$bgRed; }
	else { $t=0; }
	if ($alpha<$t) { $alpha=$t; }

	if ($srcGrn>$bgGrn) { $t=($srcGrn-$bgGrn)/(1-$bgGrn); }
	elsif ($srcGrn<$bgGrn) { $t=1-$srcGrn/$bgGrn; }
	else { $t=0; }
	if ($alpha<$t) { $alpha=$t; }

	if ($srcBlu>$bgBlu) { $t=($srcBlu-$bgBlu)/(1-$bgBlu); }
	elsif ($srcBlu<$bgBlu) { $t=1-$srcBlu/$bgBlu; }
	else { $t=0; }
	if ($alpha<$t) { $alpha=$t; }

	my $red;
	my $grn;
	my $blu;
	if ($alpha<=0) {
		$red=0.5;
		$grn=0.5;
		$blu=0.5;
	}
	elsif ($alpha>=1) {
		$red=$srcRed;
		$grn=$srcGrn;
		$blu=$srcBlu;
	}
	else {
		$red=($srcRed-$bgRed*(1-$alpha))/$alpha;
		$grn=($srcGrn-$bgGrn*(1-$alpha))/$alpha;
		$blu=($srcBlu-$bgBlu*(1-$alpha))/$alpha;
	}

	writeVal($rgbHandle, $red);
	writeVal($rgbHandle, $grn);
	writeVal($rgbHandle, $blu);
	writeVal($alphaHandle, $alpha);
}


#----------------------------- Close files handles -----------------------------

close($srcHandle);
close($rgbHandle);
close($alphaHandle);


#--------------------------- Convert pnm+pgm to png ----------------------------

if (system("pnmtopng -alpha $alphaPgmFile < $rgbPnmFile > $tgtPngFile") != 0) {
	exit(1);
}
