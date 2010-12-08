#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = perl
# Caption = Copy As
# Descr =Copy a single file or directory into any directory while giving
# Descr =the copy another name. The name is asked. Directories are copied
# Descr =recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The file or directory to be copied.
# Descr =
# Descr =  Target: The target directory for the copy.
# ButtonBgColor = #7A8
# ButtonFgColor = #000
# Hotkey = Meta+C
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleSource();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	my @src=GetSrc();
	my $oldPath=$src[0];
	my ($oldName, $oldDir)=fileparse($oldPath);
	my @tgt=GetTgt();
	my $newDir=$tgt[0];

	my $newName=Edit(
		"Copy As",
		"Please enter a name for a copy of\n\n  $oldPath\n\nin\n\n  $newDir",
		$oldName
	);

	if (-e catfile($newDir,$newName)) {
		Error("A file or directory with that name already exists.");
	}

	SetFirstPassResult($newName);

	SecondPassInTerminal("Copy As");
}

my @src=GetSrc();
my $oldPath=$src[0];
my @tgt=GetTgt();
my $newDir=$tgt[0];
my $newName=GetFirstPassResult();
my $newPath=catfile($newDir,$newName);

my $e=TermRunAndSync(
	"cp",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-vfdpR"
	)
	: (
		"-fpR"
	),
	"--",
	$oldPath,
	$newPath
);

if (-e $newPath) {
	SendSelectKS($newPath);
}
else {
	SendUpdate();
}

TermEnd($e);
