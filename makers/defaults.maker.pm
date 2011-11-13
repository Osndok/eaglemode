package defaults;

use strict;
use warnings;
use Config;

sub GetDependencies
{
	return ();
}

sub IsEssential
{
	return 1;
}

sub GetFileHandlingRules
{
	return (

		# Clear all flags for all files.
		'-clean-install-exec-private-nobackup:.*',

		# Set the exec flag for all files ending with '.sh', '.pl',
		# '.exe' or '.dll' (It's Cygwin which needs exec flags on
		# DLLs).
		'+exec:\.(sh|pl|exe|dll)$',

		# Set the exec flag for the 'bin' directory tree.
		'+exec:^bin(/|$)',

		# Set the clean flag for several directory trees.
		'+clean:^(bin|lib|obj|packages)(/|$)',

		# Set the install flag for several directory trees.
		'+install:^(bin|doc|'.
		($Config{'osname'} eq "MSWin32" ? 'etcw' : 'etc').
		'|lib|include|res)(/|$)',

		# But clear the clean and install flags for all paths containing
		# the name 'src'. (e.g. res/xxx/src/yyy would not be installed,
		# but res/xxx/src.old/yyy would!)
		'-clean-install:(^|/)src(/|$)',

		# And do not install the postscript documentation on Windows.
		$Config{'osname'} eq "MSWin32" ? ('-install:^doc/ps(/|$)') : ()

	);
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
		'perl', "$options{'utils'}/MakeDirs.pl",
		'bin',
		'lib',
		'obj'
	)==0 or return 0;

	return 1;
}
