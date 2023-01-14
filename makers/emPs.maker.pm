package emPs;

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
		"--name"          , "emPs",
		"src/emPs/emPsDocument.cpp",
		"src/emPs/emPsDocumentPanel.cpp",
		"src/emPs/emPsFileModel.cpp",
		"src/emPs/emPsFilePanel.cpp",
		"src/emPs/emPsFpPlugin.cpp",
		"src/emPs/emPsPagePanel.cpp",
		"src/emPs/emPsRenderer.cpp"
	)==0 or return 0;

	return 1;
}
