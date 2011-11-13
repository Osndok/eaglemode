package emPng;

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
			"png-inc-dir",
			"",
			"png-inc-dir=<dir>\n".
			"  A directory where the libpng include files are."
		],
		[
			"png-lib-dir",
			"",
			"png-lib-dir=<dir>\n".
			"  A directory where the libpng libraries are."
		],
		[
			"zlib-inc-dir",
			"",
			"zlib-inc-dir=<dir>\n".
			"  A directory where the zlib include files are."
		],
		[
			"zlib-lib-dir",
			"",
			"zlib-lib-dir=<dir>\n".
			"  A directory where the zlib libraries are."
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
		$options{'png-inc-dir'} ne '' ? (
			"--inc-search-dir", $options{'png-inc-dir'}
		) : (),
		$options{'zlib-inc-dir'} ne '' ? (
			"--inc-search-dir", $options{'zlib-inc-dir'}
		) : (),
		$options{'png-lib-dir'} ne '' ? (
			"--lib-search-dir", $options{'png-lib-dir'}
		) : (),
		$options{'zlib-lib-dir'} ne '' ? (
			"--lib-search-dir", $options{'zlib-lib-dir'}
		) : (),
		"--link"          , "png",
		"--link"          , "z",
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emPng",
		"src/emPng/emPngFpPlugin.cpp",
		"src/emPng/emPngImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
