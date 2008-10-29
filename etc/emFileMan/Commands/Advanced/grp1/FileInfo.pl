#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 1.0
# Interpreter = perl
# Caption = File Info
# Descr =Show some general information about a file or directory.
# Descr =This calls "stat", "file" and "md5sum" in a terminal.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The file or directory to be inspected.
# Hotkey = Ctrl+I
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleTarget();

	SecondPassInTerminal("File Info");
}

print("Results of \"stat\":\n");
system('stat','--',GetTgt());

print("\nResults of \"file\":\n");
system('file','--',GetTgt());

print("\nResults of \"md5sum\":\n");
system('md5sum','--',GetTgt());

TermEndByUser(0);
