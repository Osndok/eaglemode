package emTiff;

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
			"tiff-inc-dir",
			"",
			"tiff-inc-dir=<dir>\n".
			"  A directory where the tiff include files are."
		],
		[
			"tiff-lib-dir",
			"",
			"tiff-lib-dir=<dir>\n".
			"  A directory where the tiff libraries are."
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
		$options{'tiff-inc-dir'} ne '' ? (
			"--inc-search-dir", $options{'tiff-inc-dir'}
		) : (),
		$options{'tiff-lib-dir'} ne '' ? (
			"--lib-search-dir", $options{'tiff-lib-dir'}
		) : (),
		"--link"          , "tiff",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emTiff",
		"src/emTiff/emTiffFpPlugin.cpp",
		"src/emTiff/emTiffImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
