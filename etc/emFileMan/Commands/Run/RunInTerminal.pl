#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = perl
# Caption = Run In Terminal
# Descr =Open a terminal and run a file in it.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file to be run. It should be an executable.
# Descr =
# Descr =Hint: The current working directory is set to the parent
# Descr =directory of the target file.
# Hotkey = Meta+R
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotFiles();

	my @tgt=GetTgt();

	Confirm(
		"Run In Terminal",
		"Are you sure to run\n".
		"\n".
		"  $tgt[0]\n".
		"\n".
		"in a terminal?\n"
	);

	ChDirOrError(dirname($tgt[0]));

	SecondPassInTerminal($tgt[0]);
}

my @tgt=GetTgt();

my $e=TermRun($tgt[0]);

SendUpdate();

TermEndByUser($e);
