package defaults;

use strict;
use warnings;

sub GetDepedencies
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
		# DLL's).
		'+exec:\.(sh|pl|exe|dll)$',

		# Set the clean, install and exec flags for the 'bin' directory
		# tree.
		'+clean+install+exec:^bin(/|$)',

		# Set the clean and install flags for the 'lib' directory tree.
		'+clean+install:^lib(/|$)',

		# Set the clean flag for the 'obj' directory tree.
		'+clean:^obj(/|$)',

		# Set the install flag for further directory trees.
		'+install:^(doc|etc|include|res)(/|$)',

		# But clear the clean and install flags for all paths containing
		# the name 'src'. (e.g. res/xxx/src/yyy would not be installed,
		# but res/xxx/src.old/yyy would!)
		'-clean-install:(^|/)src(/|$)'

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
