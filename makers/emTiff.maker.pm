package emTiff;

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

	my @libTiffFlags=();
	if ($options{'tiff-inc-dir'} eq '' && $options{'tiff-lib-dir'} eq '') {
		@libTiffFlags=split("\n",readpipe(
			"perl \"".$options{'utils'}."/PkgConfig.pl\" libtiff-4"
		));
	}
	if (!@libTiffFlags) {
		if ($options{'tiff-inc-dir'} ne '') {
			push(@libTiffFlags, "--inc-search-dir", $options{'tiff-inc-dir'});
		}
		if ($options{'tiff-lib-dir'} ne '') {
			push(@libTiffFlags, "--lib-search-dir", $options{'tiff-lib-dir'});
		}
		push(@libTiffFlags, "--link", "tiff");
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
		@libTiffFlags,
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emTiff",
		"src/emTiff/emTiffFpPlugin.cpp",
		"src/emTiff/emTiffImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
