#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 52.0
# Interpreter = perl
# Caption = Unpack New Dir
# Descr =Create a new subdirectory and unpack an archive file into it. The
# Descr =name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The archive file.
# Descr =
# Descr =  Target: The directory in which the new subdirectory shall be
# Descr =          created.
# Descr =
# Descr =Following archive file formats are supported, provided that the
# Descr =corresponding system tools are installed: 7z, a, ar, arc, arj, bz,
# Descr =bz2, deb, gz, ipk, jar, lha, lzh, lzma, lzo, rar, tar, tar.bz,
# Descr =tar.bz2, tar.gz, tar.lzma, tar.lzo, tar.xz, tar.Z, taz, tbz, tbz2,
# Descr =tgj, tgz, tlz, txz, tzo, xz, Z, zip, zoo
# ButtonFgColor = #AAD
# Hotkey = Meta+U
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleSource();
	ErrorIfNotSingleTarget();
	ErrorIfSourcesNotFiles();
	ErrorIfTargetsNotDirs();

	my @src=GetSrc();
	my @tgt=GetTgt();

	my $dir=$tgt[0];

	my $name=basename($src[0]);
	my $i=rindex($name,'.');
	if ($i<=0) {
		$name.='.unpacked';
	}
	else {
		$name=substr($name,0,$i);
		$i=rindex($name,'.');
		if ($i>0) {
			my $s2=lc(substr($name,$i));
			if ($s2 eq '.tar') {
				$name=substr($name,0,$i);
			}
		}
	}

	$name=Edit(
		"Unpack New Dir",
		"Please enter a name for a new subdirectory in\n".
		"\n".
		"  $dir\n".
		"\n".
		"in order to unpack the archive file\n".
		"\n".
		"  $src[0]\n".
		"\n".
		"into that new subdirectory.",
		$name
	);

	my $path=catfile($dir,$name);

	if (-e $path) {
		Error("A file or directory of that name already exists");
	}

	SetFirstPassResult($path);

	SecondPassInTerminal("Unpack New Dir");
}

my @src=GetSrc();
my $path=GetFirstPassResult();

my $e=TermRun(
	"mkdir",
	"-v",
	"--",
	$path
);

if ($e == 0) {
	$e=TermChDir($path);
	if ($e == 0) {
		$e=TermRunAndSync(
			catfile($ENV{'EM_DIR'},'res','emFileMan','scripts','emArch.sh'),
			"unpack",
			"--",
			$src[0]
		);
	}
}

if (-e $path) {
	SendSelectKS($path);
}
else {
	SendUpdate();
}

TermEnd($e);
