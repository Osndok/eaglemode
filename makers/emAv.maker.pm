package emAv;

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
	return ('+exec:^lib/emAv/emAvServerProc_xine$');
}

sub GetExtraBuildOptions
{
	return (
		[
			"emAv",
			$Config{'osname'} eq "MSWin32" ? "vlc" : "xine",
			"emAv=<name>[,<name>]...\n".
			"  Which emAv server adapters to compile. Possible names are: xine, vlc"
		],
		[
			"xine-inc-dir",
			"",
			"xine-inc-dir=<dir>\n".
			"  A directory where the xine-lib include files are."
		],
		[
			"xine-lib-dir",
			"",
			"xine-lib-dir=<dir>\n".
			"  A directory where the xine-lib libraries are."
		]
	);
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
		"--name"          , "emAv",
		"src/emAv/emAvClient.cpp",
		"src/emAv/emAvFileControlPanel.cpp",
		"src/emAv/emAvFileModel.cpp",
		"src/emAv/emAvFilePanel.cpp",
		"src/emAv/emAvFpPlugin.cpp",
		"src/emAv/emAvServerModel.cpp",
		"src/emAv/emAvStates.cpp"
	)==0 or return 0;

	system(
		'perl', "$options{'utils'}/MakeDirs.pl",
		"lib/emAv"
	)==0 or return 0;

	foreach my $name (split(',', $options{'emAv'})) {
		if ($name eq 'xine') {
			system(
				@{$options{'unicc_call'}},
				"--math",
				"--rtti",
				"--exceptions",
				"--bin-dir"       , "lib/emAv",
				"--lib-dir"       , "lib",
				"--obj-dir"       , "obj",
				"--inc-search-dir", "include",
				$options{'xine-inc-dir'} ne '' ? (
					"--inc-search-dir", $options{'xine-inc-dir'}
				) : (),
				$options{'xine-lib-dir'} ne '' ? (
					"--lib-search-dir", $options{'xine-lib-dir'}
				) : (),
				"--link"          , "xine",
				"--link"          , "pthread",
				"--type"          , "cexe",
				"--name"          , "emAvServerProc_xine",
				"src/emAv/emAvServerProc_xine.c"
			)==0 or return 0;
		}
		elsif ($name eq 'vlc') {
			my @libvlcFlags=();
			my $str=readpipe('pkg-config --cflags --libs libvlc');
			if (!$str) { return 0; }
			foreach my $f (split(/\s+/,$str)) {
				if (substr($f,0,2) eq '-I') {
					push(@libvlcFlags,'--inc-search-dir',substr($f,2));
				}
				elsif (substr($f,0,2) eq '-L') {
					push(@libvlcFlags,'--lib-search-dir',substr($f,2));
				}
				elsif (substr($f,0,2) eq '-l') {
					push(@libvlcFlags,'--link',substr($f,2));
				}
			}
			system(
				@{$options{'unicc_call'}},
				"--math",
				"--rtti",
				"--exceptions",
				"--bin-dir"       , "lib/emAv",
				"--lib-dir"       , "lib",
				"--obj-dir"       , "obj",
				"--inc-search-dir", "include",
				@libvlcFlags,
				"--type"          , "cexe",
				"--name"          , "emAvServerProc_vlc",
				"src/emAv/emAvServerProc_vlc.c"
			)==0 or return 0;
		}
		else {
			print(STDERR "Unkown value for emAv option: $name\n");
			return 0;
		}
	}

	return 1;
}
