#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 6.0
# Interpreter = perl
# Caption = chown
# Descr =Change the owner (user and group) of one or more files and/or
# Descr =directories.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The files and directories whose owner shall be changed.
# Hotkey = Ctrl+O
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNoTargets();

	ConfirmIfTargetsAccrossDirs();

	my @tgt=GetTgt();
	my $oldUser;
	my $oldGroup;
	for (my $i=0; $i<@tgt; $i++) {
		my @s=stat($tgt[$i]);
		if ($i==0) {
			$oldUser=$s[4];
			$oldGroup=$s[5];
		}
		else {
			if ($oldUser != $s[4]) {
				$oldUser=undef;
				if (!defined($oldGroup)) { last; }
			}
			if ($oldGroup != $s[5]) {
				$oldGroup=undef;
				if (!defined($oldUser)) { last; }
			}
		}
	}
	my $oldOwner="";
	if (defined($oldUser)) { $oldOwner .= getpwuid($oldUser); }
	if (defined($oldGroup)) { $oldOwner .= ':' . getgrgid($oldGroup); }

	my $newOwner=Edit(
		"chown",
		"Please enter a new owner for the target(s). You can give a user\n".
		"or a group or both. A group has to be preceded by a colon. User\n".
		"and group can be specified by name or ID.\n".
		"\n".
		"Examples:\n".
		"  root:dialout   Set user and group.\n".
		"  0              Set user only, by ID.\n".
		"  :dialout       Set group only.",
		$oldOwner
	);

	SetFirstPassResult($newOwner);

	SecondPassInTerminal("chown");
}

my $e=TermRunAndSync(
	"chown",
	($Config{'osname'} eq 'linux' or $Config{'osname'} eq 'cygwin') ? (
		"-v"
	)
	: (
	),
	"--",
	GetFirstPassResult(),
	GetTgt()
);

SendUpdate();

TermEnd($e);
