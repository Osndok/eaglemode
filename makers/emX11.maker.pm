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

	my @libX11Flags=();
	if ($options{'X11-inc-dir'} eq '' && $options{'X11-lib-dir'} eq '') {
		@libX11Flags=split("\n",readpipe(
			"perl \"".$options{'utils'}."/PkgConfig.pl\" x11"
		));
	}
	if (!@libX11Flags) {
		if ($options{'X11-inc-dir'} ne '') {
			push(@libX11Flags, "--inc-search-dir", $options{'X11-inc-dir'});
		}
		elsif (-e "/usr/X11R7/include") {
			push(@libX11Flags, "--inc-search-dir", "/usr/X11R7/include");
		}
		elsif (-e "/usr/X11R6/include") {
			push(@libX11Flags, "--inc-search-dir", "/usr/X11R6/include");
		}
		elsif (-e "/usr/X11/include") {
			push(@libX11Flags, "--inc-search-dir", "/usr/X11/include");
		}
		if ($options{'X11-lib-dir'} ne '') {
			push(@libX11Flags, "--lib-search-dir", $options{'X11-lib-dir'});
		}
		elsif (-e "/usr/X11R7/lib64" || -e "/usr/X11R7/lib") {
			push(@libX11Flags, "--lib-search-dir", "/usr/X11R7/lib64");
			push(@libX11Flags, "--lib-search-dir", "/usr/X11R7/lib");
		}
		elsif (-e "/usr/X11R6/lib64" || -e "/usr/X11R6/lib") {
			push(@libX11Flags, "--lib-search-dir", "/usr/X11R6/lib64");
			push(@libX11Flags, "--lib-search-dir", "/usr/X11R6/lib");
		}
		elsif (-e "/usr/X11/lib64" || -e "/usr/X11/lib") {
			push(@libX11Flags, "--lib-search-dir", "/usr/X11/lib64");
			push(@libX11Flags, "--lib-search-dir", "/usr/X11/lib");
		}
		push(@libX11Flags, "--link", "X11");
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
		@libX11Flags,
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emX11",
		"src/emX11/emX11Clipboard.cpp",
		"src/emX11/emX11ExtDynamic.cpp",
		"src/emX11/emX11GUIFramework.cpp",
		"src/emX11/emX11Screen.cpp",
		"src/emX11/emX11ViewRenderer.cpp",
		"src/emX11/emX11WindowPort.cpp"
	)==0 or return 0;

	return 1;
}
