package emJpeg;

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
			"jpeg-inc-dir",
			"",
			"jpeg-inc-dir=<dir>\n".
			"  A directory where the jpeg include files are."
		],
		[
			"jpeg-lib-dir",
			"",
			"jpeg-lib-dir=<dir>\n".
			"  A directory where the jpeg libraries are."
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
		$options{'jpeg-inc-dir'} ne '' ? (
			"--inc-search-dir", $options{'jpeg-inc-dir'}
		) : (),
		$options{'jpeg-lib-dir'} ne '' ? (
			"--lib-search-dir", $options{'jpeg-lib-dir'}
		) : (),
		"--link"          , "jpeg",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emJpeg",
		"src/emJpeg/emJpegFpPlugin.cpp",
		"src/emJpeg/emJpegImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
