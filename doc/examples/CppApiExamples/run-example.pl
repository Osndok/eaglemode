#!/usr/bin/perl

use strict;
use warnings;
use Config;
use File::Spec::Functions;
use File::Basename;
use Cwd;

if (@ARGV!=1) {
	print(STDERR "Usage: perl ".basename($0)." <cpp-file>\n");
	exit(1);
}

# Have path of Eagle Mode directory.
my $em_dir=$ENV{'EM_DIR'};
if (!$em_dir) {
	my $oldCwd=getcwd();
	chdir(dirname($0));
	my $scripDir=getcwd();
	chdir($oldCwd);
	my @dirs=(
		dirname(dirname(dirname($scripDir))),
		'/usr/local/eaglemode',
		'/usr/lib/eaglemode',
		'/opt/eaglemode'
	);
	for my $d (@dirs) {
		if (-e "$d/include/emCore") {
			$em_dir=$d;
			last;
		}
	}
	if (!$em_dir) {
		print(STDERR
			"Cannot find installation directory of Eagle Mode. Please ".
			"set the EM_DIR environment variable and try again.\n"
		);
		exit(1);
	}
}

# Have path for temporary binary.
my $exeFile=File::Spec->tmpdir()."/em-compiled-example-$>";
if ($Config{'osname'} eq 'MSWin32' || $Config{'osname'} eq 'cygwin') {
	$exeFile.='.exe';
}

# Compile and link the example.
my $err=system(
	'gcc',
	'-Wall',
	"-I$em_dir/include",
	"-L$em_dir/lib",
	$ARGV[0],
	'-lemCore',
	'-lstdc++',
	'-o',
	$exeFile
);

# Set the EM_DIR environment variable.
$ENV{'EM_DIR'}=$em_dir;

# Set an environment variable so that libraries in the lib directory are found.
if (!$err) {
	my $var='LD_LIBRARY_PATH';
	my $sep=':';
	if ($Config{'osname'} eq 'darwin') {
		$var='DYLD_LIBRARY_PATH';
	}
	elsif ($Config{'osname'} eq 'MSWin32' || $Config{'osname'} eq 'cygwin') {
		$var='PATH';
		$sep=';';
	}
	my $libDir=$em_dir.'/lib';
	my $newPath=$libDir;
	my $oldPath=$ENV{$var};
	if (!$oldPath) { $oldPath=''; }
	for my $s (split(/[$sep]/,$oldPath)) {
		if ($s ne $libDir) { $newPath.=$sep.$s; }
	}
	$ENV{$var}=$newPath;
}

# Run the temporary binary.
if (!$err) {
	$err=system($exeFile);
}

# Delete the temporary binary.
unlink($exeFile);

# Done.
exit($err);
