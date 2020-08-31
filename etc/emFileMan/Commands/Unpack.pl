#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 19.0
# Interpreter = perl
# Caption = Unpack
# Descr =Unpack an archive file into a directory.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The archive file.
# Descr =
# Descr =  Target: The directory in which the unpacked files and
# Descr =          directories shall be created.
# Descr =
# Descr =Following archive file formats are supported, provided that the
# Descr =corresponding system tools are installed: 7z, a, ar, bz, bz2, deb,
# Descr =gz, ipk, jar, lzma, lzo, tar, tar.bz, tar.bz2, tar.gz, tar.lzma,
# Descr =tar.lzo, tar.xz, tar.Z, taz, tbz, tbz2, tgj, tgz, tlz, txz, tzo,
# Descr =xz, Z, zip
# Icon = unpack_file.tga
# Hotkey = Ctrl+U
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleSource();
	ErrorIfNotSingleTarget();
	ErrorIfSourcesNotFiles();
	ErrorIfTargetsNotDirs();

	my $message=
		"Are you sure to unpack the archive while overwriting any existing\n".
		"target files?\n".
		"\n".
		"From:\n".
		GetSrcListing().
		"\n".
		"Into:\n".
		GetTgtListing()
	;
	Confirm("Unpack",$message);

	my @tgt=GetTgt();
	ChDirOrError($tgt[0]);

	SecondPassInTerminal("Unpack");
}

my @src=GetSrc();

my $e=TermRunAndSync(
	catfile($ENV{'EM_DIR'},'res','emFileMan','scripts','emArch.sh'),
	"unpack",
	"--",
	$src[0]
);

SendUpdate();

TermEnd($e);
