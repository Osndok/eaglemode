#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 51.0
# Interpreter = perl
# Caption = diff
# Descr =Compare two files or directories and show the difference.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: First file or directory.
# Descr =
# Descr =  Target: Second file or directory.
# ButtonBgColor = #BBB
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleSource();
	ErrorIfNotSingleTarget();

	SecondPassInTerminal("diff");
}

my @src=GetSrc();
my @tgt=GetTgt();

my $e=TermRun('diff','-r',$src[0],$tgt[0]);

if ($e==0) {
	print("\nNo differences found.\n");
}
elsif ($e==256) {
	$e=0;
}

TermEndByUser($e);
