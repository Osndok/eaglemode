#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 8.0
# Interpreter = perl
# DefaultFor = directory
# Caption = Terminal
# Descr =Open a terminal with a shell where the current working directory is
# Descr =set to a given directory.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The directory to be used as the current working directory.
# Descr =          If a file is selected as the target, the parent directory
# Descr =          of that file is used.
# ButtonBgColor = #A9A
# ButtonFgColor = #000
# Hotkey = Ctrl+T
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

ErrorIfNotSingleTarget();

my @tgt=GetTgt();
my $dir=$tgt[0];
if (! -d $dir) {
	$dir=dirname($dir);
}
ChDirOrError($dir);

ExecOrError('xterm');
