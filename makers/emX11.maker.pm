package emX11;

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
	return (
		[
			"X11-inc-dir",
			"",
			"X11-inc-dir=<dir>\n".
			"  A directory where the X11 include subdirectory is."
		],
		[
			"X11-lib-dir",
			"",
			"X11-lib-dir=<dir>\n".
			"  A directory where the X11 libraries are."
		]
	);
}

sub Build
{
	shift;
	my %options=@_;

	if ($Config{'osname'} eq "MSWin32") {
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
		$options{'X11-inc-dir'} ne '' ? (
			"--inc-search-dir", $options{'X11-inc-dir'}
		) : (
			"--inc-search-dir", "/usr/X11R6/include"
		),
		$options{'X11-lib-dir'} ne '' ? (
			"--lib-search-dir", $options{'X11-lib-dir'}
		) : (
			"--lib-search-dir", "/usr/X11R6/lib64",
			"--lib-search-dir", "/usr/X11R6/lib"
		),
		"--link"          , "X11",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emX11",
		"src/emX11/emX11Clipboard.cpp",
		"src/emX11/emX11ExtDynamic.cpp",
		"src/emX11/emX11GUIFramework.cpp",
		"src/emX11/emX11Screen.cpp",
		"src/emX11/emX11WindowPort.cpp"
	)==0 or return 0;

	return 1;
}
