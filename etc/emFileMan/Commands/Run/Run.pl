#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = perl
# Caption = Run
# Descr =Run a file.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file to be run. It should be an executable.
# Descr =
# Descr =Hint: The current working directory is set to the parent
# Descr =directory of the target file.
# Hotkey = Ctrl+R
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

ErrorIfNotSingleTarget();
ErrorIfTargetsNotFiles();

my @tgt=GetTgt();

Confirm(
	"Run",
	"Are you sure to run:\n".
	"\n".
	"$tgt[0]\n"
);

ChDirOrError(dirname($tgt[0]));

ExecOrError($tgt[0]);
