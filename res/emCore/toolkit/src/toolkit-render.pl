#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;


# Configuration
my $SrcFile             = 'toolkit.blend';
my $OutDir              = 'toolkit-rendered.tmp';
my $RenderResolutionX   = 3200;
my $RenderResolutionY   = 3200;
my $RenderPercentage    = 100;
my $PreRenderPercentage = 25;
my $RenderTileX         = 200;
my $RenderTileY         = 200;
my $RenderDevice        = 'GPU';
my $RenderSamples       = 25000;
my $PreRenderSamples    = 100;
my @ScenesToIgnore      = ('Background');


sub Help
{
	print(
		"Usage: perl $0 <command>\n".
		"Possible commands:\n".
		"    list               - list scene names.\n".
		"    render             - render all scenes which are not already rendered.\n".
		"    pre render         - like above, but quick preview.\n".
		"    render <scene>     - render a scene.\n".
		"    pre render <scene> - like above, but quick preview.\n"
	);
}


sub ListScenes
{
	my $pyLstCmd=
		"import bpy\n".
		"for scene in bpy.data.scenes:\n".
		"  print('SceneName:'+scene.name)\n"
	;
	my @scLst=
		`blender -noaudio --background "$SrcFile" --python-expr "$pyLstCmd"`
	;
	if ($?) {
		print(STDERR "Could not get scene list from ".$SrcFile."\n");
		exit(1);
	}

	my @scenes=();
	foreach my $ln (@scLst) {
		if ($ln =~ /^SceneName:(.*)$/) {
			my $scene = $1;
			my $ignore = 0;
			foreach my $sceneToIgnore (@ScenesToIgnore) {
				if ($sceneToIgnore eq $scene) {
					$ignore = 1;
				}
			}
			if (!$ignore) {
				push(@scenes,$scene);
			}
		}
	}

	return @scenes;
}


sub RenderScene
{
	my $scene=shift;

	print("--- $scene ---\n");

	# Create output directory if not existent.
	if (!-e($OutDir)) {
		if (!mkdir($OutDir)) {
			print(STDERR "$!\n");
			exit(1);
		}
	}

	# Output file.
	my $outFile=$OutDir.'/'.$scene.'.png';

	# Temporary template and name for output file.
	my $tmpFileTmpl="/tmp/toolkit-render-$$-####.png";
	my $tmpFile    ="/tmp/toolkit-render-$$-0000.png";

	# Render...
	my $cmd=
		'blender'.
		' -noaudio'.
		' --background'.
		' '.$SrcFile.
		' --scene "'.$scene.'"'.
		' --python-expr \''.
		'import bpy'.
		'; bpy.data.scenes["'.$scene.'"].render.image_settings.file_format="PNG"'.
		'; bpy.data.scenes["'.$scene.'"].render.image_settings.color_depth="16"'.
		'; bpy.data.scenes["'.$scene.'"].render.resolution_x='.$RenderResolutionX.
		'; bpy.data.scenes["'.$scene.'"].render.resolution_y='.$RenderResolutionY.
		'; bpy.data.scenes["'.$scene.'"].render.resolution_percentage='.$RenderPercentage.
		'; bpy.data.scenes["'.$scene.'"].render.tile_x='.$RenderTileX.
		'; bpy.data.scenes["'.$scene.'"].render.tile_y='.$RenderTileY.
		'; bpy.data.scenes["'.$scene.'"].cycles.device="'.$RenderDevice.'"'.
		'; bpy.data.scenes["'.$scene.'"].cycles.samples='.$RenderSamples.
		'\''.
		' --render-output "'.$tmpFileTmpl.'"'.
		' --render-frame 0'
	;
	print("running: $cmd\n\n");
	if (system($cmd) != 0) {
		exit(1);
	}

	# Move result...
	$cmd="mv \"$tmpFile\" \"$outFile\"";
	print("\nrunning: $cmd\n");
	if (system($cmd) != 0) {
		exit(1);
	}
}


# Current directory must be the directory this script file is in.
chdir(dirname($0));


# Parse args and do the things.
if (@ARGV<=0) {
	Help();
	exit(0);
}
while (@ARGV>0) {
	my $arg=shift(@ARGV);
	if ($arg eq 'list' ) {
		foreach my $scene (ListScenes()) {
			print($scene."\n");
		}
		exit(0);
	}
	elsif ($arg eq 'pre' ) {
		$RenderPercentage=$PreRenderPercentage;
		$RenderSamples=$PreRenderSamples;
	}
	elsif ($arg eq 'render' ) {
		if (@ARGV>0) {
			foreach my $scene (@ARGV) {
				RenderScene($scene);
			}
		}
		else {
			foreach my $scene (ListScenes()) {
				if (!-e($OutDir.'/'.$scene.'.png')) {
					RenderScene($scene)
				}
			}
		}
		exit(0);
	}
	else {
		Help();
		exit(1);
	}
}
