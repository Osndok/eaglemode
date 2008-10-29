package emClock;

use strict;
use warnings;

sub GetDepedencies
{
	return ('emCore');
}

sub IsEssential
{
	return 0;
}

sub GetFileHandlingRules
{
	return ();
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

	return 1;
}
