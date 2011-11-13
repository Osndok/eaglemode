package emTest;

use strict;
use warnings;

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
			"all-from-emTest",
			"no",
			"all-from-emTest=yes|no\n".
			"  Whether to build all test programs (default: no)."
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
		"--name"          , "emTestPanel",
		"src/emTest/emTestPanel.cpp",
		"src/emTest/emTestPanelFpPlugin.cpp"
	)==0 or return 0;

	if ($options{'all-from-emTest'} eq 'yes') {
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
			"--type"          , "cexe",
			"--name"          , "emTestContainers",
			"src/emTest/emTestContainers.cpp"
		)==0 or return 0;
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
			"--type"          , "cexe",
			"--name"          , "emTestThreads",
			"src/emTest/emTestThreads.cpp"
		)==0 or return 0;
	}
	elsif ($options{'all-from-emTest'} ne 'no') {
		die("Illegal value for option 'all-from-emTest', stopped");
	}

	return 1;
}
