package font2em;

use strict;
use warnings;

sub GetDependencies
{
	return ();
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
}

sub Build
{
	shift;
	my %options=@_;

	my @libFtFlags=split("\n",readpipe(
		"perl \"".$options{'utils'}."/PkgConfig.pl\" freetype2"
	));
	if (!@libFtFlags) {
		@libFtFlags=(
			"--inc-search-dir", "/usr/include/freetype2",
			"--link"          , "freetype"
		);
	}

	system(
		@{$options{'unicc_call'}},
		"--bin-dir"       , "bin",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		@libFtFlags,
		"--type"          , "cexe",
		"--name"          , "font2em",
		"src/font2em/font2em.c"
	)==0 or return 0;

	return 1;
}
