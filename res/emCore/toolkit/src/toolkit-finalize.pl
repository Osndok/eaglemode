#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;
use File::stat;
use File::Temp;

# Configuration
my $SrcDir='toolkit-rendered.tmp';
my $TgtDir='..';

my $tmpDir = File::Temp->newdir(); # Removed automatically

sub Fin
{
	my $name=shift;
	my $scale=shift;
	my $crop=shift;

	my $srcFile=$SrcDir.'/'.$name.'.png';
	if (-e($srcFile)) {

		my $tgtFile=$TgtDir.'/'.$name.'.tga';
		if (!-e($tgtFile) || stat($srcFile)->mtime > stat($tgtFile)->mtime) {

			print("--- $name ---\n");

			my $tmpFile1=$tmpDir.'/'.$name.'-'.$$.'-tmp1.png';
			my $tmpFile2=$tmpDir.'/'.$name.'-'.$$.'-tmp2.png';

			my $cmd="convert \"$srcFile\" -resize '$scale!' \"$tmpFile1\"";
			print("$cmd\n");
			if (system($cmd) != 0) { exit(1); }

			$cmd="perl ./invalpha.pl \"$tmpFile1\" \"$tmpFile2\"";
			print("$cmd\n");
			if (system($cmd) != 0) { exit(1); }

			$cmd="convert \"$tmpFile2\" -crop '$crop' -compress RLE \"$tgtFile\"";
			print("$cmd\n");
			if (system($cmd) != 0) { exit(1); }
		}
	}
}

# Current directory must be the directory this script file is in.
chdir(dirname($0));

Fin('Button'          , '800x800', '658x658+76+76');
Fin('ButtonBorder'    , '800x800', '704x704+48+48');
Fin('ButtonChecked'   , '800x800', '648x648+76+76');
Fin('ButtonPressed'   , '800x800', '648x648+76+76');
Fin('CheckBox'        , '800x800', '380x380+210+210');
Fin('CheckBoxPressed' , '800x800', '380x380+210+210');
Fin('CustomRectBorder', '800x800', '450x450+175+175');
Fin('GroupBorder'     , '800x800', '592x592+104+104');
Fin('GroupInnerBorder', '800x800', '470x470+165+165');
Fin('IOField'         , '800x800', '572x572+114+114');
Fin('PopupBorder'     , '800x800', '320x320+240+240');
Fin('RadioBox'        , '800x800', '380x380+210+210');
Fin('RadioBoxPressed' , '800x800', '380x380+210+210');
Fin('Splitter'        , '800x800', '300x300+250+250');
Fin('SplitterPressed' , '800x800', '300x300+250+250');
Fin('Tunnel'          , '800x800', '200x200+300+300');

print(
	"\n".
	"************************************************************\n".
	"*** After this, the images should be optimized manually: ***\n".
	"*** Clean out or smooth areas where alpha is near zero.  ***\n".
	"************************************************************\n"
);
