package emPdf;

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
	return ('+exec:^lib/emPdf/emPdfServerProc$');
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
		print(STDERR "PROBLEM: emPdf has not yet been ported to Windows.\n");
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
		"--name"          , "emPdf",
		"src/emPdf/emPdfFileModel.cpp",
		"src/emPdf/emPdfFilePanel.cpp",
		"src/emPdf/emPdfFpPlugin.cpp",
		"src/emPdf/emPdfPagePanel.cpp",
		"src/emPdf/emPdfServerModel.cpp"
	)==0 or return 0;

	system(
		'perl', "$options{'utils'}/MakeDirs.pl",
		"lib/emPdf"
	)==0 or return 0;

	my @libpopplerglibFlags=();
	my $str=readpipe('pkg-config --cflags --libs gtk+-2.0 poppler-glib');
	if (!$str) { return 0; }
	foreach my $f (split(/ /,$str)) {
		if (substr($f,0,2) eq '-I') {
			push(@libpopplerglibFlags,'--inc-search-dir',substr($f,2));
		}
		elsif (substr($f,0,2) eq '-L') {
			push(@libpopplerglibFlags,'--lib-search-dir',substr($f,2));
		}
		elsif (substr($f,0,2) eq '-l') {
			push(@libpopplerglibFlags,'--link',substr($f,2));
		}
	}

	system(
		@{$options{'unicc_call'}},
		"--math",
		"--rtti",
		"--exceptions",
		"--bin-dir"       , "lib/emPdf",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		@libpopplerglibFlags,
		"--type"          , "cexe",
		"--name"          , "emPdfServerProc",
		"src/emPdf/emPdfServerProc.c"
	)==0 or return 0;

	return 1;
}
