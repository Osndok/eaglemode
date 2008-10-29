#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# Caption = Copy
# Descr =Copy one or more files and/or directories into another
# Descr =directory. Directories are copied recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be copied.
# Descr =
# Descr =  Target: The target directory, or a single target file
# Descr =          to be overwritten by a single source file.
# ButtonBgColor = #5A7
# ButtonFgColor = #000
# Hotkey = Ctrl+C
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoSources();
	ErrorIfNotSingleTarget();

	ConfirmIfSourcesAccrossDirs();

	my $message=
		"Are you sure to copy, overwriting any existing target files?\n".
		"\n".
		"From:\n".
		GetSrcListing().
		"\n".
		"To:\n".
		GetTgtListing()
	;
	Confirm("Copy",$message);

	SecondPassInTerminal("Copy");
}

my @src=GetSrc();
my @tgt=GetTgt();

my $e=TermRunAndSync(
	"cp",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-vfdpR"
	)
	: (
		"-fpR"
	),
	"--",
	@src,
	$tgt[0]
);

my @newTgt=();
if (-d $tgt[0]) {
	for (my $i=0; $i<@src; $i++) {
		my $f=fileparse($src[$i]);
		my $p=catfile($tgt[0],$f);
		if (-e $p) {
			push(@newTgt,$p);
		}
	}
}
if (@newTgt > 0) {
	SendSelectKS(@newTgt);
}
else {
	SendUpdate();
}

TermEnd($e);
