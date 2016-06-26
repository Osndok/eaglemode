#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;
use File::stat;

# Configuration
my $SrcDir='icons-rendered.tmp';
my $MaxSize='416x256';
my $OwnDistributeDir='..';
my @OtherDistributeDirs=(
	'../../emFileMan/icons',
	'../../emMain'
);

# Current directory must be the directory this script file is in.
chdir(dirname($0));

# Loop for all rendered png files.
my $dirHandle;
opendir($dirHandle,$SrcDir);
while (defined(my $filename=readdir($dirHandle))) {
	if ($filename =~ /^(.*)[.]png$/) {
		my $name=$1;

		# Source file path.
		my $srcFile=$SrcDir.'/'.$name.'.png';

		# Target file path.
		my $tgtFile=$OwnDistributeDir.'/'.$name.'.tga';
		foreach my $otherDir (@OtherDistributeDirs) {
			my $otherTgtFile=$otherDir.'/'.$name.'.tga';
			if (-e($otherTgtFile)) {
				$tgtFile=$otherTgtFile;
			}
		}

		# Is the target file not up-to-date?
		if (!-e($tgtFile) || stat($srcFile)->mtime > stat($tgtFile)->mtime) {

			# Do the trimming, resizing and format conversion with ImageMagick.
			my $cmd="convert \"$srcFile\" -trim -resize '$MaxSize>' -compress RLE \"$tgtFile\"";
			print("$cmd\n");
			if (system($cmd) != 0) { exit(1); }
		}
	}
}
closedir($dirHandle);
