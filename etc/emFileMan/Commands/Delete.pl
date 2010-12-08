#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 7.0
# Interpreter = perl
# Caption = Delete
# Descr =Remove one or more files and/or directories.
# Descr =Directories are removed recursively.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories to be removed.
# ButtonBgColor = #E56
# ButtonFgColor = #000
# Hotkey = Delete
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();
	ErrorIfRootTargets();

	ConfirmIfTargetsAccrossDirs();

	my $message="Are you sure to remove definitively";
	foreach my $t (GetTgt()) {
		if (-d $t) {
			$message.=" and recursively";
			last;
		}
	}
	$message.=":\n\n";
	$message.=GetTgtListing();
	Confirm("Delete",$message);

	SecondPassInTerminal("Delete");
}

my $e=TermRunAndSync(
	"rm",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-vfr"
	)
	: (
		"-fr"
	),
	"--",
	GetTgt()
);

SendUpdate();

TermEnd($e);
