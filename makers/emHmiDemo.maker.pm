package emHmiDemo;

use strict;
use warnings;

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
	return ();
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
		"--name"          , "emHmiDemo",
		"src/emHmiDemo/emHmiDemoAnalogDisplay.cpp",
		"src/emHmiDemo/emHmiDemoButton.cpp",
		"src/emHmiDemo/emHmiDemoCone.cpp",
		"src/emHmiDemo/emHmiDemoControls.cpp",
		"src/emHmiDemo/emHmiDemoConveyor.cpp",
		"src/emHmiDemo/emHmiDemoFile.cpp",
		"src/emHmiDemo/emHmiDemoFillIndicator.cpp",
		"src/emHmiDemo/emHmiDemoFlowIndicator.cpp",
		"src/emHmiDemo/emHmiDemoFpPlugin.cpp",
		"src/emHmiDemo/emHmiDemoMixer.cpp",
		"src/emHmiDemo/emHmiDemoMonitors.cpp",
		"src/emHmiDemo/emHmiDemoPanel.cpp",
		"src/emHmiDemo/emHmiDemoPiece.cpp",
		"src/emHmiDemo/emHmiDemoPieceGroup.cpp",
		"src/emHmiDemo/emHmiDemoPump.cpp",
		"src/emHmiDemo/emHmiDemoStation.cpp",
		"src/emHmiDemo/emHmiDemoTank.cpp"
	)==0 or return 0;

	return 1;
}
