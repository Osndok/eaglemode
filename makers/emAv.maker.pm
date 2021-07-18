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
	return ('+exec:^lib/emAv/emAvServerProc_(vlc|xine)$');
}

sub GetExtraBuildOptions
{
	return (
		[
			"emAv",
			"vlc",
			"emAv=<name>[,<name>]...\n".
			"  Which emAv server adapters to compile. Possible names are: vlc, xine"
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
		$Config{'osname'} eq 'MSWin32' ? (
			"--link"        , "user32",
			"--link"        , "advapi32"
		):(),
		"--type"          , "dynlib",
		"--name"          , "emAv",
		"src/emAv/emAvClient.cpp",
		"src/emAv/emAvFileControlPanel.cpp",
		"src/emAv/emAvFileModel.cpp",
		"src/emAv/emAvFilePanel.cpp",
		"src/emAv/emAvFpPlugin.cpp",
		"src/emAv/emAvImageConverter.cpp",
		"src/emAv/emAvImageConverter_AVX2.cpp",
		"src/emAv/emAvLibDirCfg.cpp",
		"src/emAv/emAvServerModel.cpp",
		"src/emAv/emAvStates.cpp"
	)==0 or return 0;

	system(
		'perl', "$options{'utils'}/MakeDirs.pl",
		"lib/emAv"
	)==0 or return 0;

	foreach my $name (split(',', $options{'emAv'})) {
		if ($name eq 'vlc') {
			my @libvlcFlags=split("\n",readpipe(
				"perl \"".$options{'utils'}."/PkgConfig.pl\" libvlc"
			));
			if (!@libvlcFlags) {
				@libvlcFlags=("--link","vlc");
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
				"--link"          , "pthread",
				"--type"          , "cexe",
				"--name"          , "emAvServerProc_vlc",
				"src/emAv/emAvServerProc_vlc.c"
			)==0 or return 0;
		}
		elsif ($name eq 'xine') {
			my @libXineFlags=();
			if ($options{'xine-inc-dir'} eq '' && $options{'xine-lib-dir'} eq '') {
				@libXineFlags=split("\n",readpipe(
					"perl \"".$options{'utils'}."/PkgConfig.pl\" libxine"
				));
			}
			if (!@libXineFlags) {
				if ($options{'xine-inc-dir'} ne '') {
					push(@libXineFlags, "--inc-search-dir", $options{'xine-inc-dir'});
				}
				if ($options{'xine-lib-dir'} ne '') {
					push(@libXineFlags, "--lib-search-dir", $options{'xine-lib-dir'});
				}
				push(@libXineFlags, "--link", "xine");
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
				@libXineFlags,
				"--link"          , "pthread",
				"--type"          , "cexe",
				"--name"          , "emAvServerProc_xine",
				"src/emAv/emAvServerProc_xine.c"
			)==0 or return 0;
		}
		else {
			print(STDERR "Unkown value for emAv option: $name\n");
			return 0;
		}
	}

	return 1;
}
