#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = perl
# Caption = Move As
# Descr =Move a single file or directory into any directory while giving it
# Descr =another name. The name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The file or directory to be moved and renamed.
# Descr =
# Descr =  Target: The target directory into which the file or directory
# Descr =          shall be moved.
# ButtonBgColor = #BB8
# Hotkey = Meta+M
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleSource();
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();
	ErrorIfRootSources();

	my @src=GetSrc();
	my $oldPath=$src[0];
	my ($oldName, $oldDir)=fileparse($oldPath);
	my @tgt=GetTgt();
	my $newDir=$tgt[0];

	my $newName=Edit(
		"Move As",
		"Please enter a name for a movement of\n\n  $oldPath\n\ninto\n\n  $newDir",
		$oldName
	);

	if (-e catfile($newDir,$newName)) {
		Error("A file or directory with that name already exists.");
	}

	SetFirstPassResult($newName);

	SecondPassInTerminal("Move As");
}

my @src=GetSrc();
my $oldPath=$src[0];
my @tgt=GetTgt();
my $newDir=$tgt[0];
my $newName=GetFirstPassResult();
my $newPath=catfile($newDir,$newName);

my $e=TermRunAndSync(
	"mv",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-vf"
	)
	: (
		"-f"
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
