package emTreeDump;

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
		"--name"          , "emTreeDump",
		"src/emTreeDump/emTreeDumpControlPanel.cpp",
		"src/emTreeDump/emTreeDumpFileModel.cpp",
		"src/emTreeDump/emTreeDumpFilePanel.cpp",
		"src/emTreeDump/emTreeDumpFpPlugin.cpp",
		"src/emTreeDump/emTreeDumpRec.cpp",
		"src/emTreeDump/emTreeDumpRecPanel.cpp",
		"src/emTreeDump/emTreeDumpUtil.cpp"
	)==0 or return 0;

	return 1;
}
