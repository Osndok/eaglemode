package emWnds;

use strict;
use warnings;
use Config;

sub GetDependencies
{
	return ('emCore');
}

sub IsEssential
{
	return 1;
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

	if ($Config{'osname'} ne "MSWin32" and $Config{'osname'} ne "cygwin") {
		return 1;
	}

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
		"--link"          , "user32",
		"--link"          , "gdi32",
		"--type"          , "dynlib",
		"--name"          , "emWnds",
		"src/emWnds/emWndsClipboard.cpp",
		"src/emWnds/emWndsGUIFramework.cpp",
		"src/emWnds/emWndsScheduler.cpp",
		"src/emWnds/emWndsScreen.cpp",
		"src/emWnds/emWndsWindowPort.cpp"
	)==0 or return 0;

	return 1;
}
