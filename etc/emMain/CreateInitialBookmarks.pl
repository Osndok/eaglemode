#!/usr/bin/perl

use strict;
use warnings;
use Config;
use File::Basename;


#============================== Helper Functions ===============================

sub QuoteRecString
{
	my $str=shift;

	$str=~s/\\/\\\\/g;
	$str=~s/\"/\\\"/g;
	$str="\"$str\"";
	return $str;
}

sub QuotePanelName
{
	my $str=shift;

	$str=~s/\\/\\\\/g;
	$str=~s/:/\\:/g;
	return $str;
}


#================ Parse args and start writing the output file =================

if (@ARGV != 1 or substr($ARGV[0],0,1) eq '-') {
	print(STDERR "ERROR: Illegal arguments.\nUsage: $0 <output file>\n");
	exit(1);
}
my $fh;
if (!open($fh,">",$ARGV[0])) {
	print(STDERR "ERROR: Cannot open or create \"$ARGV[0]\": $!.\n");
	exit(1);
}
print($fh "#\%rec:emBookmarks\%#\n");


#=============================== Bookmark: Help ================================

print($fh '
Bookmark: {
	Name = "Help"
	Description = "This brings you to the documentation area."
	Icon = "help.tga"
	Hotkey = "F1"
	LocationIdentity = ":"
	LocationRelX = -0.36326
	LocationRelY = -0.37791
	LocationRelA = 0.00621
}
');


#============================= Bookmark: Home Dir ==============================

my $loc='::FS::';
if ($Config{'osname'} eq 'MSWin32') {
	my $homeDir=$ENV{'USERPROFILE'};
	for (my $i=0; $i<length($homeDir); ) {
		my $j=index($homeDir,'\\',$i);
		if ($j<$i) { $j=length($homeDir); }
		if ($j>$i and ($j!=2 or substr($homeDir,1,1) ne ':')) {
			$loc = $loc . '::' . QuotePanelName(substr($homeDir,$i,$j-$i));
		}
		$i=$j+1;
	}
}
else {
	my $homeDir=$ENV{'HOME'};
	for (my $i=0; $i<length($homeDir); ) {
		my $j=index($homeDir,'/',$i);
		if ($j<$i) { $j=length($homeDir); }
		if ($j>$i) {
			$loc = $loc . '::' . QuotePanelName(substr($homeDir,$i,$j-$i));
		}
		$i=$j+1;
	}
}
print($fh '
Bookmark: {
	Name = "Home"
	Description = "This brings you to your home directory."
	Icon = "home.tga"
	Hotkey = "F6"
	LocationIdentity = ' . QuoteRecString($loc) . '
	VisitAtProgramStart = yes
}
');


#============================= Bookmark: Root Dir ==============================

print($fh '
Bookmark: {
	Name = "Root"
	Description = "This brings you to the root directory of the file system."
	Icon = "root.tga"
	Hotkey = "F7"
	LocationIdentity = "::FS::"
}
');


#========================== Bookmark: Virtual Cosmos ===========================

print($fh '
Bookmark: {
	Name = "Virtual Cosmos"
	Icon = "virtual_cosmos.tga"
	Hotkey = "F8"
	LocationIdentity = ":"
}
');


#============================= Bookmark: SilChess ==============================

print($fh '
Bookmark: {
	Name = "Chess"
	Icon = "silchess.tga"
	LocationIdentity = "::Chess1:"
}
');


#=============================== Bookmark: Mines ===============================

print($fh '
Bookmark: {
	Name = "Mines"
	Icon = "mines.tga"
	LocationIdentity = "::Mines1:"
}
');


#============================== Bookmark: Netwalk ==============================

print($fh '
Bookmark: {
	Name = "Netwalk"
	Icon = "netwalk.tga"
	LocationIdentity = "::Netwalk1:"
}
');


#=============================== Bookmark: Clock ===============================

print($fh '
Bookmark: {
	Name = "Clock"
	Icon = "clock.tga"
	LocationIdentity = "::Clock1:"
}
');


#=================================== The End ===================================

close($fh);
