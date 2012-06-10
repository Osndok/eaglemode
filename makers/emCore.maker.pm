package emCore;

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
		# src directories are not installed by default, but we need one:
		'+install:^doc/examples/CppApiExamples/PluginExample/src(/|$)'
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
		$Config{'osname'} eq 'MSWin32' ? (
			"--link", "user32",
			"--link", "advapi32",
			"--link", "shell32"
		)
		: $Config{'osname'} eq 'linux' ? (
			"--link", "pthread",
			"--link", "dl"
		)
		: $Config{'osname'} eq 'cygwin' ? (
		)
		: (
			"--link", "pthread"
		),
		"--type"          , "dynlib",
		"--name"          , "emCore",
		"src/emCore/emATMatrix.cpp",
		"src/emCore/emAvlTree.cpp",
		"src/emCore/emClipboard.cpp",
		"src/emCore/emColor.cpp",
		"src/emCore/emConfigModel.cpp",
		"src/emCore/emContext.cpp",
		"src/emCore/emCoreConfig.cpp",
		"src/emCore/emCrossPtr.cpp",
		"src/emCore/emCursor.cpp",
		"src/emCore/emEngine.cpp",
		"src/emCore/emErrorPanel.cpp",
		"src/emCore/emFileModel.cpp",
		"src/emCore/emFilePanel.cpp",
		"src/emCore/emFontCache.cpp",
		"src/emCore/emFpPlugin.cpp",
		"src/emCore/emGUIFramework.cpp",
		"src/emCore/emImage.cpp",
		"src/emCore/emImageFile.cpp",
		"src/emCore/emInput.cpp",
		"src/emCore/emInstallInfo.cpp",
		"src/emCore/emList.cpp",
		"src/emCore/emMiniIpc.cpp",
		"src/emCore/emModel.cpp",
		"src/emCore/emPainter.cpp",
		"src/emCore/emPanel.cpp",
		"src/emCore/emPriSchedAgent.cpp",
		"src/emCore/emProcess.cpp",
		"src/emCore/emRec.cpp",
		"src/emCore/emRecFileModel.cpp",
		"src/emCore/emRes.cpp",
		"src/emCore/emScheduler.cpp",
		"src/emCore/emScreen.cpp",
		"src/emCore/emSigModel.cpp",
		"src/emCore/emSignal.cpp",
		"src/emCore/emStd1.cpp",
		"src/emCore/emStd2.cpp",
		"src/emCore/emString.cpp",
		"src/emCore/emSubViewPanel.cpp",
		"src/emCore/emThread.cpp",
		"src/emCore/emTimer.cpp",
		"src/emCore/emTmpFile.cpp",
		"src/emCore/emToolkit.cpp",
		"src/emCore/emView.cpp",
		"src/emCore/emViewInputFilter.cpp",
		"src/emCore/emWindow.cpp"
	)==0 or return 0;

	return 1;
}
