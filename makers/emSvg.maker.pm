package emSvg;

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
	return ('+exec:^lib/emSvg/emSvgServerProc$');
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
		"--name"          , "emSvg",
		"src/emSvg/emSvgFileModel.cpp",
		"src/emSvg/emSvgFilePanel.cpp",
		"src/emSvg/emSvgFpPlugin.cpp",
		"src/emSvg/emSvgServerModel.cpp"
	)==0 or return 0;

	system(
		'perl', "$options{'utils'}/MakeDirs.pl",
		"lib/emSvg"
	)==0 or return 0;

	my @librsvgFlags=split("\n",readpipe(
		"perl \"".$options{'utils'}."/PkgConfig.pl\" librsvg-2.0"
	));
	if (!@librsvgFlags) {
		@librsvgFlags=("--link","rsvg-2");
	}

	system(
		@{$options{'unicc_call'}},
		"--math",
		"--rtti",
		"--exceptions",
		"--bin-dir"       , "lib/emSvg",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		@librsvgFlags,
		$Config{'osname'} eq 'MSWin32' ? ("--link", "user32"):(),
		"--type"          , "cexe",
		"--name"          , "emSvgServerProc",
		"src/emSvg/emSvgServerProc.c"
	)==0 or return 0;

	return 1;
}
