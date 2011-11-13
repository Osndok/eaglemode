#!/usr/bin/perl
#-------------------------------------------------------------------------------
# cmd-util.pl
#
# Copyright (C) 2007-2008,2010 Oliver Hamann.
#
# Homepage: http://eaglemode.sourceforge.net/
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 3 as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
# more details.
#
# You should have received a copy of the GNU General Public License version 3
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------

use strict;
use warnings;
use Config;
use File::Basename;
use File::Spec::Functions;
use IO::Handle;
# Hint: never 'use File::stat' here, otherwise we would have to adapt all the
# command scripts which use the normal stat function.


#================================ Configuration ================================

# Whether to run sync at the end of commands. Can be overloaded with the
# environment variable EM_FM_SYNC.
my $Sync='no';
if (exists($ENV{'EM_FM_SYNC'})) { $Sync=$ENV{'EM_FM_SYNC'}; }

# Terminal colors.
my $TermBg   ='#aaaaaa';
my $TermFg   ='#000000';
my $TCNormal ="\e[0m";
my $TCInfo   ="\e[0;34m";
my $TCSuccess="\e[1;32m";
my $TCError  ="\e[1;31m";
my $TCClose  ="\e[1;37m";


#======================= Parse arguments / private stuff =======================

if ($Config{'osname'} eq 'MSWin32') {
	Error("The file manager commands do not function on Windows.");
}

my $Pass;
my $FirstPassResult;
my @Src;
my @Tgt;

{
	my $i=0;
	if ($ARGV[$i] eq "pass2") {
		$Pass=2;
		$FirstPassResult=$ARGV[$i+1];
		$i+=2;
		if (exists($ENV{'EM_SAVED_LD_LIBRARY_PATH'})) {
			$ENV{'LD_LIBRARY_PATH'}=$ENV{'EM_SAVED_LD_LIBRARY_PATH'};
			delete($ENV{'EM_SAVED_LD_LIBRARY_PATH'});
		}
	}
	else {
		$Pass=1;
		$FirstPassResult="";
	}
	if (@ARGV < $i+2) { die "bad arguments"; }
	my $srcCnt=$ARGV[$i++];
	my $tgtCnt=$ARGV[$i++];
	if (@ARGV != $i+$srcCnt+$tgtCnt) { die "bad arguments"; }
	@Src=@ARGV[$i..($i+$srcCnt-1)];
	$i+=$srcCnt;
	@Tgt=@ARGV[$i..($i+$tgtCnt-1)];
}


#============================== First/second pass ==============================

sub SecondPassInTerminal
	# Restart the whole command script in a terminal.
	# This function does not return.
	# Arguments: <title> [, "-hold"]
	# The argument "-hold" is useful for debugging: the terminal does not
	# close on exit.
{
	my $x=int($ENV{'EM_X'}+($ENV{'EM_WIDTH'}-510)/2);
	my $y=int($ENV{'EM_Y'}+($ENV{'EM_HEIGHT'}-350)/2);
	if ($x < 0) { $x=0; }
	if ($y < 0) { $y=0; }
	if (exists($ENV{'LD_LIBRARY_PATH'})) {
		$ENV{'EM_SAVED_LD_LIBRARY_PATH'}=$ENV{'LD_LIBRARY_PATH'};
		# Because LD_LIBRARY_PATH is cleared through xterm on some systems.
	}
	ExecOrError(
		'xterm',
		'-sb',
		'-sl','1000', # don't make this too large (slows down)
		'-bg',$TermBg,
		'-fg',$TermFg,
		'-geometry',"80x24+${x}+${y}",
		'-T',@_,
		'-e',
		'perl',
		$0,
		'pass2',
		$FirstPassResult,
		($#Src)+1,
		($#Tgt)+1,
		@Src,
		@Tgt
	);
}


sub IsFirstPass
	# Returns non-zero if the script has not yet been restarted for the
	# second pass.
{
	return $Pass == 1;
}


sub SetFirstPassResult
	# Set a single scalar value (no reference), which can be re-get in the
	# second pass.
{
	$FirstPassResult=$_[0];
}


sub GetFirstPassResult
	# Get the value set with SetFirstPassResult.
{
	return $FirstPassResult;
}


#============================= Get the selections ==============================

sub GetSrc
	# Get list of source-selected files.
{
	return @Src;
}


sub GetTgt
	# Get list of target-selected files.
{
	return @Tgt;
}


sub GetSrcListing
	# Get a string containing a listing of source-selected files, each on a
	# separate line, with some indent.
{
	my $l="";
	for (my $i=0; $i<@Src; $i++) {
		$l .= "  " . $Src[$i] . "\n";
	}
	return $l;
}


sub GetTgtListing
	# Get a string containing a listing of target-selected files, each on a
	# separate line, with some indent.
{
	my $l="";
	for (my $i=0; $i<@Tgt; $i++) {
		$l .= "  " . $Tgt[$i] . "\n";
	}
	return $l;
}


#======================== Sending commands to eaglemode ========================

sub SendUpdate
	# Require Eagle Mode to reload changed files and directories.
{
	system(
		catfile($ENV{'EM_DIR'},"bin","emSendMiniIpc"),
		$ENV{'EM_FM_SERVER_NAME'},
		"update"
	);
}


sub SendSelect
	# Require Eagle Mode to select other targets. It will even reload
	# changed files and directories. As usual, the source selection is set
	# from the old target selection.
	# Arguments: <file>, [<file>...]
{
	system(
		catfile($ENV{'EM_DIR'},"bin","emSendMiniIpc"),
		$ENV{'EM_FM_SERVER_NAME'},
		"select",
		$ENV{'EM_COMMAND_RUN_ID'},
		@_
	);
}


sub SendSelectKS
	# Like SendSelect, but the source selection is not modified.
{
	system(
		catfile($ENV{'EM_DIR'},"bin","emSendMiniIpc"),
		$ENV{'EM_FM_SERVER_NAME'},
		"selectks",
		$ENV{'EM_COMMAND_RUN_ID'},
		@_
	);
}


sub SendSelectCS
	# Like SendSelect, but the source selection is cleared.
{
	system(
		catfile($ENV{'EM_DIR'},"bin","emSendMiniIpc"),
		$ENV{'EM_FM_SERVER_NAME'},
		"selectcs",
		$ENV{'EM_COMMAND_RUN_ID'},
		@_
	);
}


#================================ Basic dialogs ================================

sub Dlg
	# Low-level function for calling emShowStdDlg.
{
	my $w=400;
	my $h=300;
	my $x=int($ENV{'EM_X'}+($ENV{'EM_WIDTH'}-$w)/2);
	my $y=int($ENV{'EM_Y'}+($ENV{'EM_HEIGHT'}-$h)/2);
	if ($x < 0) { $x=0; }
	if ($y < 0) { $y=0; }

	my $e=system(
		catfile($ENV{'EM_DIR'},"bin","emShowStdDlg"),
		'-geometry',
		"${w}x${h}+${x}+${y}",
		@_
	);
	return $e==0 ? 1 : 0;
}


sub DlgRead
	# Like Dlg, but read pipe.
{
	my $hdl;
	my $pid=open($hdl,'-|');
		# ??? Requires Perl 5.8 (or 5.6???) and does not work on every OS.
	if (!$pid) {
		# Child process
		my $r=Dlg(@_);
		print(":$r");
		exit(0);
	}
	my $res;
	read($hdl,$res,1000000);
	if (!defined($res)) { return undef; }
	if (substr($res,length($res)-2,2) ne ":1") { return undef; }
	$res=substr($res,0,length($res)-2);
	while (length($res)>0 && ord(substr($res,length($res)-1,1))<32) {
		$res=substr($res,0,length($res)-1);
	}
	return $res;
}


sub Message
	# Show a message dialog.
	# Arguments: <title>, <message>
{
	Dlg("message",@_);
}


sub Error
	# Show an error message in a dialog box and exit.
	# Arguments: <error message>
{
	Message("Error",@_);
	exit(1);
}


sub Warning
	# Show a warning message in a dialog box (does not exit).
	# Arguments: <warning message>
{
	Message("Warning",@_);
}


sub Confirm
	# Show a message dialog with OK and Cancel buttons. Exits on
	# cancellation.
	# Arguments: <title>, <message>
{
	if (!Dlg("confirm",@_)) { exit(1); }
}


sub Edit
	# Show a dialog for editing a string. Exits on cancellation.
	# Arguments: <title>, <question>, <initial string value>
	# Returns: the string
{
	my $res=DlgRead("edit",@_);
	if (!defined($res)) { exit(1); }
	return $res;
}


sub PasswordEdit
	# Like Edit, but for editing a password.
{
	my $res=DlgRead("pwedit",@_);
	if (!defined($res)) { exit(1); }
	return $res;
}


#============================== Selection errors ===============================

sub ErrorIfNoSources
{
	if (@Src<1) { Error("No source selected."); }
}


sub ErrorIfNoTargets
{
	if (@Tgt<1) { Error("No target selected."); }
}


sub ErrorIfMultipleSources
{
	if (@Src>1) { Error("Multiple sources selected."); }
}


sub ErrorIfMultipleTargets
{
	if (@Tgt>1) { Error("Multiple targets selected."); }
}


sub ErrorIfNotSingleSource
{
	ErrorIfNoSources();
	ErrorIfMultipleSources();
}


sub ErrorIfNotSingleTarget
{
	ErrorIfNoTargets();
	ErrorIfMultipleTargets();
}


sub ErrorIfSourcesNotDirs
{
	for (my $i=0; $i<@Src; $i++) {
		if (! -d $Src[$i]) { Error("Non-directory selected as source."); }
	}
}


sub ErrorIfTargetsNotDirs
{
	for (my $i=0; $i<@Tgt; $i++) {
		if (! -d $Tgt[$i]) { Error("Non-directory selected as target."); }
	}
}


sub ErrorIfSourcesNotFiles
{
	for (my $i=0; $i<@Src; $i++) {
		if (! -f $Src[$i]) { Error("Non-file selected as source."); }
	}
}


sub ErrorIfTargetsNotFiles
{
	for (my $i=0; $i<@Tgt; $i++) {
		if (! -f $Tgt[$i]) { Error("Non-file selected as target."); }
	}
}


sub ErrorIfSourcesAccrossDirs
{
	if (@Src>1) {
		my ($f0,$d0)=fileparse($Src[0]);
		for (my $i=1; $i<@Src; $i++) {
			my ($f,$d)=fileparse($Src[$i]);
			if ($d ne $d0) {
				Error(
					"Sources selected from different directories."
				);
			}
		}
	}
}


sub ErrorIfTargetsAccrossDirs
{
	if (@Tgt>1) {
		my ($f0,$d0)=fileparse($Tgt[0]);
		for (my $i=1; $i<@Tgt; $i++) {
			my ($f,$d)=fileparse($Tgt[$i]);
			if ($d ne $d0) {
				Error(
					"Targets selected from different directories."
				);
			}
		}
	}
}


sub ErrorIfRootSources
{
	for (my $i=0; $i<@Src; $i++) {
		if ($Src[$i] eq '/') { Error("Root directory selected as source."); }
	}
}


sub ErrorIfRootTargets
{
	for (my $i=0; $i<@Tgt; $i++) {
		if ($Tgt[$i] eq '/') { Error("Root directory selected as target."); }
	}
}


#=========================== Selection confirmations ===========================

sub ConfirmIfSourcesAccrossDirs
{
	if (@Src>1) {
		my ($f0,$d0)=fileparse($Src[0]);
		for (my $i=1; $i<@Src; $i++) {
			my ($f,$d)=fileparse($Src[$i]);
			if ($d ne $d0) {
				Confirm("Warning",
					"Sources are selected from different directories.\n".
					"Are you sure this is correct?"
				);
				last;
			}
		}
	}
}


sub ConfirmIfTargetsAccrossDirs
{
	if (@Tgt>1) {
		my ($f0,$d0)=fileparse($Tgt[0]);
		for (my $i=1; $i<@Tgt; $i++) {
			my ($f,$d)=fileparse($Tgt[$i]);
			if ($d ne $d0) {
				Confirm("Warning",
					"Targets are selected from different directories.\n".
					"Are you sure this is correct?"
				);
				last;
			}
		}
	}
}


sub ConfirmToOpenIfManyTargets
{
	my $n=@Tgt;
	if ($n>10) {
		Confirm("Warning","Do you really want to open $n files at once?");
	}
}


#==================== Further helpers for dialoged session =====================

sub ChDirOrError
	# Change the current directory. On error, show an error message and exit.
	# Arguments: <directory>
{
	if (!chdir($_[0])) {
		Error("Cannot chdir to '$_[0]': $!");
	}
}


sub ExecOrError
	# Start a program and exit, show an error message if starting fails.
	# Arguments: <program> [,<arguments>...]
{
	if (!exec({$_[0]} @_)) { # 'if' required only for suppressing a warning.
		Error("Failed to run $_[0]: $!");
	}
	# never coming here
}


#====================== Helpers for the terminal session =======================

sub TermRun
	# Print and run a program, return the exit status (non-zero on error).
	# Arguments: <program> [,<arguments>...]
{
	print("\n${TCInfo}Running: ");
	for (my $i=0; $i<@_; $i++) {
		print("$_[$i] ");
	}
	print("${TCNormal}\n\n");
	return system({$_[0]} @_);
}


sub TermSync
	# Print and run the sync command, return the exit status (non-zero on error).
	# This is now disabled by default (see $Sync in configuration more above).
{
	if ((lc($Sync) eq 'yes' || lc($Sync) eq 'true' || $Sync eq '1')) {
		return TermRun("sync");
	}
	else {
		return 0;
	}
}


sub TermRunAndSync
	# Combination of TermRun and TermSync
{
	my $e=TermRun(@_);
	$e|=TermSync();
	return $e;
}


sub TermChDir
	# Change the current directory, return non-zero on error.
	# Arguments: <directory>
{
	print("\n${TCInfo}Setting current directory: $_[0]${TCNormal}\n");
	if (!chdir($_[0])) {
		print("Cannot chdir to '$_[0]': $!");
		return 1;
	}
	return 0;
}


sub TermEnd
	# End the terminal session: Print a message whether there was an error.
	# Wait for user input on error. Then exit.
	# Arguments: <non-zero for error>
{
	if ($_[0]!=0) {
		print(
			"\n".
			"${TCError}ERROR!${TCNormal}\n".
			"\n".
			"${TCClose}Read the messages, then press enter or close the terminal.${TCNormal}\n"
		);
		readline(*STDIN);
		exit(1);
	}
	else {
		print(
			"\n".
			"${TCSuccess}SUCCESS!${TCNormal}\n".
			"\n"
		);
		sleep(1);
		exit(0);
	}
}


sub TermEndByUser
	# Like TermEnd, but always let the user close the terminal.
	# Arguments: <non-zero for error>
{
	if ($_[0]!=0) {
		print(
			"\n".
			"${TCError}ERROR!${TCNormal}\n".
			"\n".
			"${TCClose}Read the messages, then press enter or close the terminal.${TCNormal}\n"
		);
		readline(*STDIN);
		exit(1);
	}
	else {
		print(
			"\n".
			"${TCClose}Read the messages, then press enter or close the terminal.${TCNormal}\n"
		);
		readline(*STDIN);
		exit(0);
	}
}


#==================== Hi-level functions for frequent cases ====================

sub OpenSingleTargetFileWith
{
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotFiles();
	my @tgt=GetTgt();
	ChDirOrError(dirname($tgt[0]));
	ExecOrError(@_,$tgt[0]);
}


sub OpenSingleTargetDirWith
{
	ErrorIfNotSingleTarget();
	ErrorIfTargetsNotDirs();
	my @tgt=GetTgt();
	ExecOrError(@_,$tgt[0]);
}


sub OpenSingleTargetWith
{
	ErrorIfNotSingleTarget();
	my @tgt=GetTgt();
	ExecOrError(@_,$tgt[0]);
}


sub OpenTargetFilesWith
{
	ErrorIfNoTargets();
	ErrorIfTargetsNotFiles();
	ConfirmToOpenIfManyTargets();
	my @tgt=GetTgt();
	ChDirOrError(dirname($tgt[0]));
	ExecOrError(@_,@tgt);
}


sub OpenTargetDirsWith
{
	ErrorIfNoTargets();
	ErrorIfTargetsNotDirs();
	ConfirmToOpenIfManyTargets();
	my @tgt=GetTgt();
	ExecOrError(@_,@tgt);
}


sub OpenTargetsWith
{
	ErrorIfNoTargets();
	ConfirmToOpenIfManyTargets();
	my @tgt=GetTgt();
	ExecOrError(@_,@tgt);
}


sub PackType
{
	my $type=shift;

	if (IsFirstPass()) {

		ErrorIfNoSources();
		ErrorIfSourcesAccrossDirs();
		ErrorIfNotSingleTarget();
		ErrorIfTargetsNotDirs();

		my @src=GetSrc();
		my @tgt=GetTgt();
		my ($srcName0,$srcDir)=fileparse($src[0]);
		my $dir=$tgt[0];
		my $name = "archive";
		if (@src == 1) {
			$name = $srcName0;
		}
		$name = $name . '.' . $type;

		$name=Edit(
			"Pack $type",
			"Please enter a name for the new $type archive in:\n\n$dir",
			$name
		);

		if (-e catfile($dir,$name)) {
			Error("A file or directory of that name already exists.");
		}

		ChDirOrError($srcDir);

		SetFirstPassResult($name);

		SecondPassInTerminal("Pack $type");
	}

	my @src=GetSrc();
	my @tgt=GetTgt();
	my ($srcName0,$srcDir)=fileparse($src[0]);
	my $dir=$tgt[0];
	my $name=GetFirstPassResult();
	my $path=catfile($dir,$name);

	my @srcNames;
	for (my $i=0; $i<@src; $i++) {
		my $n=fileparse($src[$i]);
		push(@srcNames,$n);
	}

	my $e=TermRunAndSync(
		catfile($ENV{'EM_DIR'},'res','emFileMan','scripts','emArch.sh'),
		"pack",
		"-f",
		"$type",
		"--",
		$path,
		@srcNames
	);

	if (-e $path) {
		SendSelect($path);
	}
	else {
		SendUpdate();
	}

	TermEnd($e);
}


#===============================================================================

return 1; # Because this file is used like a module.
