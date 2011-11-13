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

	if ($Config{'osname'} eq "MSWin32") {
		print(STDERR "PROBLEM: emSvg has not yet been ported to Windows.\n");
		return 0;
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

	my @librsvgFlags=();
	my $str=readpipe('pkg-config --cflags --libs librsvg-2.0');
	if (!$str) { return 0; }
	foreach my $f (split(/ /,$str)) {
		if (substr($f,0,2) eq '-I') {
			push(@librsvgFlags,'--inc-search-dir',substr($f,2));
		}
		elsif (substr($f,0,2) eq '-L') {
			push(@librsvgFlags,'--lib-search-dir',substr($f,2));
		}
		elsif (substr($f,0,2) eq '-l') {
			push(@librsvgFlags,'--link',substr($f,2));
		}
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
		"--type"          , "cexe",
		"--name"          , "emSvgServerProc",
		"src/emSvg/emSvgServerProc.c"
	)==0 or return 0;

	return 1;
}
