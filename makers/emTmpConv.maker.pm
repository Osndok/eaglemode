package emTmpConv;

use strict;
use warnings;

sub GetDependencies
{
	return ('emCore','emFileMan');
}

sub IsEssential
{
	return 0;
}

sub GetFileHandlingRules
{
	return ('+exec:^lib/emTmpConv/emTmpConvProc$');
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
		"--link"          , "emFileMan",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emTmpConv",
		"src/emTmpConv/emTmpConvFpPlugin.cpp",
		"src/emTmpConv/emTmpConvFramePanel.cpp",
		"src/emTmpConv/emTmpConvModel.cpp",
		"src/emTmpConv/emTmpConvModelClient.cpp",
		"src/emTmpConv/emTmpConvPanel.cpp"
	)==0 or return 0;

	system(
		'perl', "$options{'utils'}/MakeDirs.pl",
		"lib/emTmpConv"
	)==0 or return 0;

	system(
		@{$options{'unicc_call'}},
		"--bin-dir"       , "lib/emTmpConv",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		"--type"          , "cexe",
		"--name"          , "emTmpConvProc",
		"src/emTmpConv/emTmpConvProc.c"
	)==0 or return 0;

	return 1;
}
