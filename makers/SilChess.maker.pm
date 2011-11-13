package SilChess;

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
	return ();
}

sub GetExtraBuildOptions
{
	return (
		[
			"OldSilChess",
			"no",
			"OldSilChess=yes|no\n".
			"  Whether to build the old stand-alone executables too (default: no)."
		]
	);
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
		"--name"          , "SilChess",
		"src/SilChess/SilChessControlPanel.cpp",
		"src/SilChess/SilChessFpPlugin.cpp",
		"src/SilChess/SilChessMachine.cpp",
		"src/SilChess/SilChessModel.cpp",
		"src/SilChess/SilChessPanel.cpp",
		"src/SilChess/SilChessRayTracer.cpp"
	)==0 or return 0;

	if ($options{'OldSilChess'} eq 'yes') {
		system(
			'perl', "$options{'utils'}/MakeDirs.pl",
			"obj/2"
		)==0 or return 0;
		system(
			@{$options{'unicc_call'}},
			"--math",
			"--rtti",
			"--exceptions",
			"--bin-dir"       , "bin",
			"--lib-dir"       , "lib",
			"--obj-dir"       , "obj/2",
			"--inc-search-dir", "include",
			"--type"          , "cexe",
			"--name"          , "silchess",
			"src/SilChess/SilChess.cpp",
			"src/SilChess/SilChessMachine.cpp"
		)==0 or return 0;
		if ($Config{'osname'} ne 'MSWin32') {
			system(
				'perl', "$options{'utils'}/MakeDirs.pl",
				"obj/3"
			)==0 or return 0;
			system(
				@{$options{'unicc_call'}},
				"--math",
				"--rtti",
				"--exceptions",
				"--bin-dir"       , "bin",
				"--lib-dir"       , "lib",
				"--obj-dir"       , "obj/3",
				"--inc-search-dir", "include",
				"--inc-search-dir", "/usr/X11R6/include",
				"--lib-search-dir", "/usr/X11R6/lib64",
				"--lib-search-dir", "/usr/X11R6/lib",
				"--link"          , "Xm",
				"--link"          , "Xt",
				"--link"          , "X11",
				"--type"          , "wexe",
				"--name"          , "xsilchess",
				"src/SilChess/SilChessMachine.cpp",
				"src/SilChess/SilChessRayTracer.cpp",
				"src/SilChess/XSilChess.cpp"
			)==0 or return 0;
		}
	}
	elsif ($options{'OldSilChess'} ne 'no') {
		die("Illegal value for option 'OldSilChess', stopped");
	}

	return 1;
}
