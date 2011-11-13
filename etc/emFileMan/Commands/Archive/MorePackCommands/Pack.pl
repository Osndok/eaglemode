#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 20.0
# Interpreter = perl
# Caption = Pack
# Descr =Create an archive from files and directories.
# Descr =The name of the archive file is asked. The archive
# Descr =type is specified by the file name suffix.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be packed.
# Descr =
# Descr =  Target: The directory in which the archive file
# Descr =          shall be created.
# Hotkey = Shift+Meta+P
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoSources();
	ErrorIfSourcesAccrossDirs();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	my @src=GetSrc();
	my @tgt=GetTgt();
	my ($srcName0,$srcDir)=fileparse($src[0]);
	my $dir=$tgt[0];
	my $name = "";
	if (@src == 1) {
		$name = $srcName0 . ".";
	}

	$name=Edit(
		"Pack",
		"Please enter a file name for the new archive in:\n".
		"\n".
		"  $dir\n".
		"\n".
		"The file name suffix specifies the archive type and must be one of:\n".
		"  .7z\n".
		"  .ar | .a | .deb | .ipk\n".
		"  .arc\n".
		"  .arj\n".
		"  .bz2\n".
		"  .gz\n".
		"  .lzh | .lha\n".
		"  .lzma\n".
		"  .lzo\n".
		"  .tar\n".
		"  .tar.bz2 | .tbz2 | .tgj\n".
		"  .tar.gz | .tgz\n".
		"  .tar.lzma | .tlz\n".
		"  .tar.lzo | .tzo\n".
		"  .tar.xz | .txz\n".
		"  .xz\n".
		"  .zip | .jar\n".
		"  .zoo",
		$name
	);

	if (-e catfile($dir,$name)) {
		Error("A file or directory of that name already exists.");
	}

	ChDirOrError($srcDir);

	SetFirstPassResult($name);

	SecondPassInTerminal("Pack");
}

my @src=GetSrc();
my @tgt=GetTgt();
my ($srcName0,$srcDir)=fileparse($src[0]);
my $dir=$tgt[0];
my $name=GetFirstPassResult();
my $path=catfile($dir,$name);

my @srcNames;
for (my $i=0; $i<@src; $i++) {
	my $n=fileparse($src[$i]);
	push(@srcNames,$n);
}

my $e=TermRunAndSync(
	catfile($ENV{'EM_DIR'},'res','emFileMan','scripts','emArch.sh'),
	"pack",
	"--",
	$path,
	@srcNames
);

if (-e $path) {
	SendSelect($path);
}
else {
	SendUpdate();
}

TermEnd($e);
