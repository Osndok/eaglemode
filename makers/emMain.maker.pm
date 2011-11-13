package emMain;

use strict;
use warnings;
use Config;

sub GetDependencies
{
	return ('emCore');
}

sub IsEssential
{
	return 1;
}

sub GetFileHandlingRules
{
	return (
		'+install:^README$',
		$Config{'osname'} eq 'MSWin32' ? (
			'+install:^eaglemode.wsf$'
		)
		: (
			'+install:^eaglemode.sh$'
		)
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
		@{$options{'unicc_call'}},
		"--math",
		"--rtti",
		"--exceptions",
		"--bin-dir"       , "bin",
		"--lib-dir"       , "lib",
		"--obj-dir"       , "obj",
		"--inc-search-dir", "include",
		"--link"          , "emCore",
		"--type"          , "wexe",
		"--name"          , "eaglemode",
		"src/emMain/emBookmarks.cpp",
		"src/emMain/emCoreConfigPanel.cpp",
		"src/emMain/emMain.cpp",
		"src/emMain/emMainConfig.cpp",
		"src/emMain/emMainContentPanel.cpp",
		"src/emMain/emMainPanel.cpp",
		"src/emMain/emMainWindow.cpp",
		"src/emMain/emStarFieldPanel.cpp",
		"src/emMain/emVirtualCosmos.cpp"
	)==0 or return 0;

	return 1;
}
