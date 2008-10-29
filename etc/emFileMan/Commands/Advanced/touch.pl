#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 7.0
# Interpreter = perl
# Caption = touch
# Descr =Change the time stamp of one or more files and/or directories.
# Descr =The new time stamp is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories which shall get the new time
# Descr =          stamp.
# Hotkey = Meta+T
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();

	ConfirmIfTargetsAccrossDirs();

	my $tm=Edit(
		"touch",
		"Please enter a new time stamp for the target(s), or leave\n".
		"blank for using the current time. The time stamp format is:\n".
		"[[CC]YY]MMDDhhmm[.ss]",
		""
	);

	SetFirstPassResult($tm);

	SecondPassInTerminal("touch");
}

my $tm=GetFirstPassResult();

my $e=TermRunAndSync(
	"touch",
	($tm ne "") ? ("-t", $tm) : (),
	"--",
	GetTgt()
);

SendUpdate();

TermEnd($e);
