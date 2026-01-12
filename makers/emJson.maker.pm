package emJson;

use strict;
use warnings;

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
		"--name"          , "emJson",
		"src/emJson/emJsonElement.cpp",
		"src/emJson/emJsonException.cpp",
		"src/emJson/emJsonKeyMap.cpp",
		"src/emJson/emJsonParser.cpp",
		"src/emJson/emJsonPositionTracker.cpp"
	)==0 or return 0;

	return 1;
}
