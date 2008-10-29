#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 50.0
# Interpreter = perl
# Caption = wc
# Descr =Show the number of lines, words and bytes in files.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files to be inspected.
# ButtonBgColor = #BBB
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();
	ErrorIfTargetsNotFiles();

	SecondPassInTerminal("wc");
}

my $e=TermRun(
	"wc",
	GetTgt()
);

TermEndByUser($e);
