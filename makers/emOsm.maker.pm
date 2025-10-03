package emOsm;

use strict;
use warnings;

sub GetDependencies
{
	return ('emCore','emPng','emJpeg');
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
		"--link"          , "emPng",
		"--link"          , "emJpeg",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emOsm",
		"src/emOsm/emOsmConfig.cpp",
		"src/emOsm/emOsmControlPanel.cpp",
		"src/emOsm/emOsmFileModel.cpp",
		"src/emOsm/emOsmFilePanel.cpp",
		"src/emOsm/emOsmFpPlugin.cpp",
		"src/emOsm/emOsmTileCache.cpp",
		"src/emOsm/emOsmTileCacheCleaner.cpp",
		"src/emOsm/emOsmTileDownloader.cpp",
		"src/emOsm/emOsmTilePanel.cpp"
	)==0 or return 0;

	return 1;
}
