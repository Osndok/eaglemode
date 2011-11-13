package font2em;

use strict;
use warnings;
use Config;

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

	if ($Config{'osname'} ne 'MSWin32') {
		my @libFtFlags=();
		my $str=readpipe('pkg-config --cflags --libs freetype2');
		if ($str) {
			foreach my $f (split(/ /,$str)) {
				if (substr($f,0,2) eq '-I') {
					push(@libFtFlags,'--inc-search-dir',substr($f,2));
				}
				elsif (substr($f,0,2) eq '-L') {
					push(@libFtFlags,'--lib-search-dir',substr($f,2));
				}
				elsif (substr($f,0,2) eq '-l') {
					push(@libFtFlags,'--link',substr($f,2));
				}
			}
		}
		else {
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
	}

	return 1;
}
