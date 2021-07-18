package emWebp;

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
			"webp-inc-dir",
			"",
			"webp-inc-dir=<dir>\n".
			"  A directory where the WebP include files are."
		],
		[
			"webp-lib-dir",
			"",
			"webp-lib-dir=<dir>\n".
			"  A directory where the WebP libraries are."
		]
	);
}

sub Build
{
	shift;
	my %options=@_;

	my @libWebpFlags=();
	if ($options{'webp-inc-dir'} eq '' && $options{'webp-lib-dir'} eq '') {
		@libWebpFlags=split("\n",readpipe(
			"perl \"".$options{'utils'}."/PkgConfig.pl\" libwebp"
		));
	}
	if (!@libWebpFlags) {
		if ($options{'webp-inc-dir'} ne '') {
			push(@libWebpFlags, "--inc-search-dir", $options{'webp-inc-dir'});
		}
		if ($options{'webp-lib-dir'} ne '') {
			push(@libWebpFlags, "--lib-search-dir", $options{'webp-lib-dir'});
		}
		push(@libWebpFlags, "--link", "webp");
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
		@libWebpFlags,
		"--link"          , "emCore",
		"--type"          , "dynlib",
		"--name"          , "emWebp",
		"src/emWebp/emWebpFpPlugin.cpp",
		"src/emWebp/emWebpImageFileModel.cpp"
	)==0 or return 0;

	return 1;
}
