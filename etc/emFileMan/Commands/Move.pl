#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 5.0
# Interpreter = perl
# Caption = Move
# Descr =Move one or more files and/or directories into another
# Descr =directory.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories to be moved.
# Descr =
# Descr =  Target: The target directory, or a single target file
# Descr =          to be overwritten by a single source file.
# ButtonBgColor = #BB5
# ButtonFgColor = #000
# Hotkey = Ctrl+M
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoSources();
	ErrorIfNotSingleTarget();
	ErrorIfRootSources();

	ConfirmIfSourcesAccrossDirs();

	my $message=
		"Are you sure to move, overwriting any existing target files?\n".
		"\n".
		"From:\n".
		GetSrcListing().
		"\n".
		"To:\n".
		GetTgtListing()
	;
	Confirm("Move",$message);

	SecondPassInTerminal("Move");
}

my @src=GetSrc();
my @tgt=GetTgt();

my $e=TermRunAndSync(
	"mv",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-vf"
	)
	: (
		"-f"
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
