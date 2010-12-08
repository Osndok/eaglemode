#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 4.0
# Interpreter = perl
# Caption = Clone
# Descr =Create a copy of a file or directory within the same
# Descr =parent directory. The name for the copy is asked. If
# Descr =a directory is copied, it is copied recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file or directory to be cloned.
# ButtonBgColor = #796
# ButtonFgColor = #000
# Hotkey = Ctrl+V
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleTarget();
	ErrorIfRootTargets();

	my @tgt=GetTgt();
	my $oldPath=$tgt[0];
	my ($oldName, $dir)=fileparse($oldPath);

	my $newName=Edit(
		"Clone",
		"Please enter a name for a copy of:\n\n$oldPath",
		$oldName
	);

	if (-e catfile($dir,$newName)) {
		Error("A file or directory with that name already exists.");
	}

	SetFirstPassResult($newName);

	SecondPassInTerminal("Clone");
}

my @tgt=GetTgt();
my $oldPath=$tgt[0];
my ($oldName, $dir)=fileparse($oldPath);
my $newName=GetFirstPassResult();
my $newPath=catfile($dir,$newName);

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
	SendSelect($newPath);
}
else {
	SendUpdate();
}

TermEnd($e);
