#!/usr/bin/perl
#-------------------------------------------------------------------------------
# pack_exe.pl
#
# Copyright (C) 2010-2011 Oliver Hamann.
#
# Homepage: http://eaglemode.sourceforge.net/
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 3 as published by the
# Free Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
# more details.
#
# You should have received a copy of the GNU General Public License version 3
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------

use strict;
use warnings;
use File::Basename;
BEGIN { require (dirname($0).'/common.pm'); }

# Have an empty temporary directory.
my $tmpDir=catfile(Var('TMP_DIR'),Var('NAME').'-exe-packing');
if (-e $tmpDir) { RemoveTree($tmpDir); }
CreateDirPath($tmpDir);

# Copy source.
my $srcDir=catfile($tmpDir,Var('NAME').'-'.Var('VERSION'));
CreateDirPath($srcDir);
print("Copying source to $srcDir\n");
my $oldDir=getcwd();
chdir(Var('PRJ_DIR')) or die;
my @paths=readpipe "perl make.pl list listdirs=yes filter=-clean-private";
if (!@paths) {
	die("make.pl failed, stopped");
}
for (my $i=0; $i<@paths; $i++) {
	my $path=$paths[$i];
	$path =~ s/\x{a}|\x{d}//;
	my $tgtPath=catfile($srcDir,$path);
	if (-d($path)) {
		if (!mkpath($tgtPath)) {
			die "failed to create directory \"$tgtPath\": $!";
		}
	}
	else {
		if (!copy($path,$tgtPath)) {
			die "failed to copy \"$path\" to \"$tgtPath\": $!";
		}
	}
}
chdir($oldDir) or die;

# Compile it
system(
	'perl',
	catfile($srcDir,'make.pl'),
	'build',
	'continue=no',
	'projects=not:emAv,emPdf,emSvg'
)==0 || exit 1;

# Install it
my $installDir=catfile($tmpDir,Var('NAME'));
system(
	'perl',
	catfile($srcDir,'make.pl'),
	'install',
	'dir='.$installDir
)==0 || exit 1;

# Remove source tree.
RemoveTree($srcDir);

# Install third-party software.
my $tpDir=Var('WIN_THIRDPARTY_DIR');
if (length($tpDir)>0 && -e $tpDir) {
	CopyTree($tpDir,$installDir);
}

# Create uninstall commands.
my $uninstallCommands="";
sub list_helper
{
	my $subPath=shift;
	my $fullPath=$installDir.$subPath;

	if (-d $fullPath) {
		my $dh;
		if (!opendir($dh,$fullPath)) {
			die "failed to read directory $fullPath";
		}
		while (defined(my $name=readdir($dh))) {
			if ($name ne '.' && $name ne '..') {
				list_helper($subPath.'\\'.$name);
			}
		}
		closedir($dh);
		$uninstallCommands.='  RMDir "$INSTDIR'.$subPath.'"'."\n";
	}
	else {
		$uninstallCommands.='  Delete "$INSTDIR'.$subPath.'"'."\n";
	}
}
list_helper('');

# Generate the NSIS script.
my $nsiFile=Var('NAME').'-'.Var('VERSION').'.nsi';
my $exeFile=Var('NAME').'-'.Var('VERSION').'-setup.exe';
CreateFile(
	catfile($tmpDir,$nsiFile),
	'Name "'.Var('TITLE').' '.Var('VERSION').'"'."\n".
	'OutFile "'.$exeFile.'"'."\n".
	'InstallDir "$PROGRAMFILES\\'.Var('TITLE').'"'."\n".
	'RequestExecutionLevel admin'."\n".
	'Page components'."\n".
	'Page directory'."\n".
	'Page instfiles'."\n".
	'UninstPage uninstConfirm'."\n".
	'UninstPage instfiles'."\n".
	'Section "'.Var('TITLE').'"'."\n".
	'  SectionIn RO'."\n".
	'  SetOutPath $INSTDIR'."\n".
	'  File /r "'.Var('NAME').'\\*.*"'."\n".
	'  WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'.Var('TITLE').'" "DisplayName" "'.Var('TITLE').'"'."\n".
	'  WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'.Var('TITLE').'" "DisplayIcon" "$INSTDIR\\res\\icons\\'.Var('NAME').'.ico"'."\n".
	'  WriteRegStr HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'.Var('TITLE').'" "UninstallString" "$INSTDIR\\uninstall.exe"'."\n".
	'  WriteRegDWORD HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'.Var('TITLE').'" "NoModify" 1'."\n".
	'  WriteRegDWORD HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'.Var('TITLE').'" "NoRepair" 1'."\n".
	'  WriteUninstaller "uninstall.exe"'."\n".
	'SectionEnd'."\n".
	'Section "Start Menu Shortcuts"'."\n".
	'  CreateDirectory "$SMPROGRAMS\\'.Var('TITLE').'"'."\n".
	'  CreateShortCut "$SMPROGRAMS\\'.Var('TITLE').'\\Documentation.lnk" "$INSTDIR\\doc\\html\\index.html"'."\n".
	'  CreateShortCut "$SMPROGRAMS\\'.Var('TITLE').'\\'.Var('TITLE').'.lnk" "$INSTDIR\\'.Var('NAME').'.wsf" "" "$INSTDIR\\res\\icons\\'.Var('NAME').'.ico" 0'."\n".
	'  CreateShortCut "$SMPROGRAMS\\'.Var('TITLE').'\\Uninstall.lnk" "$INSTDIR\\uninstall.exe" "" "$INSTDIR\\uninstall.exe" 0'."\n".
	'SectionEnd'."\n".
	'Section "Desktop Icon"'."\n".
	'  CreateShortCut "$DESKTOP\\'.Var('TITLE').'.lnk" "$INSTDIR\\'.Var('NAME').'.wsf" "" "$INSTDIR\\res\\icons\\'.Var('NAME').'.ico" 0'."\n".
	'SectionEnd'."\n".
	'Section "Uninstall"'."\n".
	'  DeleteRegKey HKLM "Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\'.Var('TITLE').'"'."\n".
	'  Delete "$DESKTOP\\'.Var('TITLE').'.lnk"'."\n".
	'  Delete "$SMPROGRAMS\\'.Var('TITLE').'\\Documentation.lnk"'."\n".
	'  Delete "$SMPROGRAMS\\'.Var('TITLE').'\\'.Var('TITLE').'.lnk"'."\n".
	'  Delete "$SMPROGRAMS\\'.Var('TITLE').'\\Uninstall.lnk"'."\n".
	'  RMDir "$SMPROGRAMS\\'.Var('TITLE').'"'."\n".
	'  Delete "$INSTDIR\\uninstall.exe"'."\n".
	$uninstallCommands.
	'SectionEnd'."\n"
);

# Create the installer executable.
my $cmd='"'.Var('WIN_NSIS_DIR').'\\MakeNSIS" '.$nsiFile;
print("Running: $cmd\n");
my $oldDir2=getcwd();
chdir($tmpDir) or die;
system($cmd)==0 || exit 1;
chdir($oldDir2) or die;

# Copy resulting files into output directory.
CreateDirPath(Var('PKG_DIR'));
CopyFile(catfile($tmpDir,$nsiFile),Var('PKG_DIR'));
CopyFile(catfile($tmpDir,$exeFile),Var('PKG_DIR'));

# Remove temporary directory.
RemoveTree($tmpDir);
