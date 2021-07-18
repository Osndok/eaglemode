package emJpeg;

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

	my @libJpegFlags=();
	if ($options{'jpeg-inc-dir'} eq '' && $options{'jpeg-lib-dir'} eq '') {
		@libJpegFlags=split("\n",readpipe(
			"perl \"".$options{'utils'}."/PkgConfig.pl\" libjpeg"
		));
	}
	if (!@libJpegFlags) {
		if ($options{'jpeg-inc-dir'} ne '') {
			push(@libJpegFlags, "--inc-search-dir", $options{'jpeg-inc-dir'});
		}
		if ($options{'jpeg-lib-dir'} ne '') {
			push(@libJpegFlags, "--lib-search-dir", $options{'jpeg-lib-dir'});
		}
		push(@libJpegFlags, "--link", "jpeg");
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
		@libJpegFlags,
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emJpeg",
		"src/emJpeg/emJpegFpPlugin.cpp",
		"src/emJpeg/emJpegImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
