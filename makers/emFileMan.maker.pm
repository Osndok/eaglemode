package emFileMan;

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
	if ($Config{'osname'} eq "MSWin32") {
		return ('-install:^res/emFileMan/scripts/(cmd-util\.pl|emArch\.sh)$');
	}
	else {
		return ('-install:^res/emFileMan/scripts/(cmd-util\.js|msleep\.js)$');
	}
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
		"--name"          , "emFileMan",
		"src/emFileMan/emDirEntry.cpp",
		"src/emFileMan/emDirEntryAltPanel.cpp",
		"src/emFileMan/emDirEntryPanel.cpp",
		"src/emFileMan/emDirFpPlugin.cpp",
		"src/emFileMan/emDirModel.cpp",
		"src/emFileMan/emDirPanel.cpp",
		"src/emFileMan/emDirStatFpPlugin.cpp",
		"src/emFileMan/emDirStatPanel.cpp",
		"src/emFileMan/emFileLinkFpPlugin.cpp",
		"src/emFileMan/emFileLinkModel.cpp",
		"src/emFileMan/emFileLinkPanel.cpp",
		"src/emFileMan/emFileManConfig.cpp",
		"src/emFileMan/emFileManControlPanel.cpp",
		"src/emFileMan/emFileManModel.cpp",
		"src/emFileMan/emFileManSelInfoPanel.cpp",
		"src/emFileMan/emFileManTheme.cpp",
		"src/emFileMan/emFileManViewConfig.cpp"
	)==0 or return 0;

	return 1;
}
