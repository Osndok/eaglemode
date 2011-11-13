package emAv;

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
	return ('+exec:^lib/emAv/emAvServerProc_xine$');
}

sub GetExtraBuildOptions
{
	return (
		[
			"xine-inc-dir",
			"",
			"xine-inc-dir=<dir>\n".
			"  A directory where the xine-lib include files are."
		],
		[
			"xine-lib-dir",
			"",
			"xine-lib-dir=<dir>\n".
			"  A directory where the xine-lib libraries are."
		]
	);
}

sub Build
{
	shift;
	my %options=@_;

	if ($Config{'osname'} eq "MSWin32") {
		print(STDERR "PROBLEM: emAv has not yet been ported to Windows.\n");
		return 0;
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
		"--type"          , "dynlib",
		"--name"          , "emAv",
		"src/emAv/emAvClient.cpp",
		"src/emAv/emAvFileControlPanel.cpp",
		"src/emAv/emAvFileModel.cpp",
		"src/emAv/emAvFilePanel.cpp",
		"src/emAv/emAvFpPlugin.cpp",
		"src/emAv/emAvServerModel.cpp",
		"src/emAv/emAvStates.cpp"
	)==0 or return 0;

	system(
		'perl', "$options{'utils'}/MakeDirs.pl",
		"lib/emAv"
	)==0 or return 0;

	system(
		@{$options{'unicc_call'}},
		"--math",
		"--rtti",
		"--exceptions",
		"--bin-dir"       , "lib/emAv",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		$options{'xine-inc-dir'} ne '' ? (
			"--inc-search-dir", $options{'xine-inc-dir'}
		) : (),
		$options{'xine-lib-dir'} ne '' ? (
			"--lib-search-dir", $options{'xine-lib-dir'}
		) : (),
		"--link"          , "xine",
		"--link"          , "pthread",
		"--type"          , "cexe",
		"--name"          , "emAvServerProc_xine",
		"src/emAv/emAvServerProc_xine.c"
	)==0 or return 0;

	return 1;
}
