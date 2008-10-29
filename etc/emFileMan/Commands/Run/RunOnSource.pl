#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 3.0
# Interpreter = perl
# Caption = Run On Source
# Descr =Run a file with the source paths as the arguments.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: The files and directories whose paths are to be
# Descr =          taken as the arguments.
# Descr =
# Descr =  Target: The file to be run. It should be an executable.
# Descr =
# Descr =Hints: Here, the current working directory is set to the
# Descr =parent directory of the first source argument. The order
# Descr =of arguments may differ from the order of selecting them.
# Hotkey = Shift+Ctrl+R
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

ErrorIfNoSources();
ConfirmIfSourcesAccrossDirs();
ErrorIfNotSingleTarget();
ErrorIfTargetsNotFiles();

my @src=GetSrc();
my @tgt=GetTgt();

my $message=
	"Are you sure to take these paths as arguments:\n".
	"\n".
	GetSrcListing().
	"\n".
	"And run:\n".
	"\n".
	GetTgtListing()
;

Confirm("Run On Source",$message);

ChDirOrError(dirname($src[0]));

ExecOrError($tgt[0],@src);
