package emPng;

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

	my @libPngFlags=();
	if (
		$options{'png-inc-dir'} eq '' && $options{'png-lib-dir'} eq '' &&
		$options{'zlib-inc-dir'} eq '' && $options{'zlib-lib-dir'} eq ''
	) {
		@libPngFlags=split("\n",readpipe(
			"perl \"".$options{'utils'}."/PkgConfig.pl\" libpng zlib"
		));
	}
	if (!@libPngFlags) {
		if ($options{'png-inc-dir'} ne '') {
			push(@libPngFlags, "--inc-search-dir", $options{'png-inc-dir'});
		}
		if ($options{'zlib-inc-dir'} ne '') {
			push(@libPngFlags, "--inc-search-dir", $options{'zlib-inc-dir'});
		}
		if ($options{'png-lib-dir'} ne '') {
			push(@libPngFlags, "--lib-search-dir", $options{'png-lib-dir'});
		}
		if ($options{'zlib-lib-dir'} ne '') {
			push(@libPngFlags, "--lib-search-dir", $options{'zlib-lib-dir'});
		}
		push(@libPngFlags, "--link", "png");
		push(@libPngFlags, "--link", "z");
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
		@libPngFlags,
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emPng",
		"src/emPng/emPngDecode.c",
		"src/emPng/emPngFpPlugin.cpp",
		"src/emPng/emPngImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
