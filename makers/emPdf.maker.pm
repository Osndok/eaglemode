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

	my @libpopplerglibFlags=split("\n",readpipe(
		"perl \"".$options{'utils'}."/PkgConfig.pl\" gtk+-2.0 poppler-glib"
	));
	if (!@libpopplerglibFlags) {
		@libpopplerglibFlags=("--link","poppler-glib");
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
		$Config{'osname'} eq 'MSWin32' ? ("--link", "user32"):(),
		"--type"          , "cexe",
		"--name"          , "emPdfServerProc",
		"src/emPdf/emPdfServerProc.c"
	)==0 or return 0;

	return 1;
}
