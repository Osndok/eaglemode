#!/usr/bin/perl

use strict;
use warnings;

my $blenderFile = 'toolkit.blend';

my $pyScript=
	"import bpy\n".
	"for scene in bpy.data.scenes:\n".
	"  scene.render.image_settings.file_format = 'PNG'\n".
	"  scene.render.image_settings.color_depth = '16'\n".
	"  scene.render.resolution_x = 3200\n".
	"  scene.render.resolution_y = 3200\n".
	"  scene.render.resolution_percentage = 25\n".
	"  scene.render.tile_x = 200\n".
	"  scene.render.tile_y = 200\n".
	"  scene.cycles.device = 'GPU'\n".
	"  scene.cycles.samples = 25\n".
	"  scene.cycles.preview_samples = 5\n".
	"  scene.cycles.sample_clamp_direct = 2\n".
	"  scene.cycles.sample_clamp_indirect = 1\n".
	"  scene.cycles.caustics_reflective = 0\n".
	"  scene.cycles.caustics_refractive = 0\n".
	"  scene.cycles.blur_glossy = 5.0\n".
	"for world in bpy.data.worlds:\n".
	"  world.cycles.sample_as_light = 0\n".
	"  world.cycles.sample_map_resolution = 256\n".
	"  world.cycles.max_bounces = 1024\n".
	"bpy.ops.wm.save_mainfile()\n"
;

my $cmd="blender -noaudio --background \"$blenderFile\" --python-expr \"$pyScript\"";
print("\nrunning: $cmd\n");
if (system($cmd) != 0) {
	exit(1);
}
