package emClock;

use strict;
use warnings;
use Config;

sub GetDependencies
{
	return ('emCore');
}

sub IsEssential
{
	return 0;
}

sub GetFileHandlingRules
{
	return ('+exec:^lib/emClock/emTimeZonesProc$');
}

sub GetExtraBuildOptions
{
	return ();
}

sub Build
{
	shift;
	my %options=@_;

	system(
		@{$options{'unicc_call'}},
		"--math",
		"--rtti",
		"--exceptions",
		"--bin-dir"       , "bin",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emClock",
		"src/emClock/emAlarmClockModel.cpp",
		"src/emClock/emAlarmClockPanel.cpp",
		"src/emClock/emClockDatePanel.cpp",
		"src/emClock/emClockFileModel.cpp",
		"src/emClock/emClockFpPlugin.cpp",
		"src/emClock/emClockHandsPanel.cpp",
		"src/emClock/emClockPanel.cpp",
		"src/emClock/emStopwatchPanel.cpp",
		"src/emClock/emTimeZonesModel.cpp",
		"src/emClock/emWorldClockPanel.cpp"
	)==0 or return 0;

	if ($Config{'osname'} ne "MSWin32") {
		system(
			'perl', "$options{'utils'}/MakeDirs.pl",
			"lib/emClock"
		)==0 or return 0;
		system(
			@{$options{'unicc_call'}},
			"--bin-dir"       , "lib/emClock",
			"--lib-dir"       , "lib",
			"--obj-dir"       , "obj",
			"--inc-search-dir", "include",
			"--type"          , "cexe",
			"--name"          , "emTimeZonesProc",
			"src/emClock/emTimeZonesProc.c"
		)==0 or return 0;
	}

	return 1;
}
