#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 53.1
# Interpreter = perl
# Caption = grep -i
# Descr =Search with a text pattern in files and directories
# Descr =recursively, and show the matching lines. The text
# Descr =pattern is asked. The search is case-insensitive.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: Files and directories to be searched.
# ButtonBgColor = #BBB
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();

	my $pat=Edit(
		"grep -i",
		"Please enter the text pattern for the search. It's\n".
		"a regular expression (see grep(1) for details).",
		""
	);

	SetFirstPassResult($pat);

	SecondPassInTerminal("grep -i");
}

my $pat=GetFirstPassResult();

my $e=TermRun('grep','-i','-r','-n','-e',$pat,GetTgt());

if ($e==256) {
	print("\nNo match found.\n");
	$e=0;
}

TermEndByUser($e);
