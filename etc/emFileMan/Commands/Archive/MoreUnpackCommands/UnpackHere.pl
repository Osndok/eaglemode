#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 51.0
# Interpreter = perl
# DefaultFor = .7z:.a:.ar:.arc:.arj:.bz:.bz2:.deb:.gz:.ipk:.jar:.lha:.lzh:.lzma:.lzo:.rar:.tar:.tar.bz:.tar.bz2:.tar.gz:.tar.lzma:.tar.lzo:.tar.xz:.tar.z:.taz:.tbz:.tbz2:.tgj:.tgz:.tlz:.txz:.tzo:.xz:.z:.zip:.zoo
# Caption = Unpack Here
# Descr =Unpack an archive file into the directory where the archive file
# Descr =is in.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The archive file.
# Descr =
# Descr =Following archive file formats are supported, provided that the
# Descr =corresponding system tools are installed: 7z, a, ar, arc, arj, bz,
# Descr =bz2, deb, gz, ipk, jar, lha, lzh, lzma, lzo, rar, tar, tar.bz,
# Descr =tar.bz2, tar.gz, tar.lzma, tar.lzo, tar.xz, tar.Z, taz, tbz, tbz2,
# Descr =tgj, tgz, tlz, txz, tzo, xz, Z, zip, zoo
# ButtonFgColor = #AAD
# Hotkey = Shift+Ctrl+U
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotFiles();

	my @tgt=GetTgt();
	my $archive=$tgt[0];
	my $tgtdir=dirname($archive);

	my $message=
		"Are you sure to unpack the archive into the same directory\n".
		"while overwriting any existing target files?\n".
		"\n".
		"From:\n".
		"  $archive\n".
		"\n".
		"Into:\n".
		"  $tgtdir"
	;
	Confirm("Unpack Here",$message);

	ChDirOrError($tgtdir);

	SecondPassInTerminal("Unpack Here");
}

my @src=GetSrc();

my @tgt=GetTgt();
my $archive=$tgt[0];

my $e=TermRunAndSync(
	catfile($ENV{'EM_DIR'},'res','emFileMan','scripts','emArch.sh'),
	"unpack",
	"--",
	$archive
);

SendUpdate();

TermEnd($e);
