#!/usr/bin/perl
#[[BEGIN PROPERTIES]]
# Type = Command
# Order = 2.0
# Interpreter = perl
# Caption = New File
# Descr =Create a new empty file. The name is asked.
# Descr =
# Descr =Selection details:
# Descr =
# Descr =  Source: Ignored.
# Descr =
# Descr =  Target: The directory in which the new file
# Descr =          shall be created.
# ButtonBgColor = #8AB
# ButtonFgColor = #000
# Hotkey = Ctrl+F
#[[END PROPERTIES]]

use strict;
use warnings;
BEGIN { require "$ENV{'EM_DIR'}/res/emFileMan/scripts/cmd-util.pl"; }

if (IsFirstPass()) {

	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();

	my @tgt=GetTgt();
	my $dir=$tgt[0];
	my $name;
	for (my $i=1; ; $i++) {
		$name="newfile$i";
		if (!-e catfile($dir,$name)) {
			last;
		}
	}

	$name=Edit(
		"New File",
		"Please enter a name for a new empty file in:\n\n$dir",
		$name
	);

	if (-e catfile($dir,$name)) {
		Error("A file or directory with that name already exists.");
	}

	SetFirstPassResult($name);

	SecondPassInTerminal("New File");
}

my @tgt=GetTgt();
my $dir=$tgt[0];
my $name=GetFirstPassResult();
my $path=catfile($dir,$name);

my $e=TermRunAndSync(
	"touch",
	"--",
	$path
);

if (-e $path) {
	SendSelectKS($path);
}
else {
	SendUpdate();
}

TermEnd($e);
