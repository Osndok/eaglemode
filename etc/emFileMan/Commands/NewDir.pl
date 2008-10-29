#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = perl
# Caption = New Dir
# Descr =Create a new empty subdirectory. The name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The directory in which the new subdirectory
# Descr =          shall be created.
# ButtonBgColor = #88C
# ButtonFgColor = #000
# Hotkey = Ctrl+D
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	my @tgt=GetTgt();
	my $dir=$tgt[0];
	my $name;
	for (my $i=1; ; $i++) {
		$name="newdir$i";
		if (!-e catfile($dir,$name)) {
			last;
		}
	}

	$name=Edit(
		"New Dir",
		"Please enter a name for a new subdirectory in:\n\n$dir",
		$name
	);

	if (-e catfile($dir,$name)) {
		Error("A file or directory with that name already exists.");
	}

	SetFirstPassResult($name);

	SecondPassInTerminal("New Dir");
}

my @tgt=GetTgt();
my $dir=$tgt[0];
my $name=GetFirstPassResult();
my $path=catfile($dir,$name);

my $e=TermRunAndSync(
	"mkdir",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-v"
	)
	: (
	),
	"--",
	$path
);

if (-e $path) {
	SendSelectKS($path);
}
else {
	SendUpdate();
}

TermEnd($e);
