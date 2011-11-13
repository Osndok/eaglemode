#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;


sub generate_theme
	# Arguments: tallness, display name, file
{
	my $tallness = shift;
	my $DisplayName = shift;
	my $fileName = shift;

	my $BackgroundColor = '{238 240 248}';
	my $SourceSelectionColor = '{166 245 220}';
	my $TargetSelectionColor = '{255 189 225}';
	my $NormalNameColor = '{47 48 50}';
	my $ExeNameColor = '{45 110 50}';
	my $DirNameColor = '{45 48 180}';
	my $FifoNameColor = '{211 47 131}';
	my $BlkNameColor = '{129 102 48}';
	my $ChrNameColor = '{115 129 48}';
	my $SockNameColor = '{197 47 199}';
	my $OtherNameColor = '{170 74 62}';
	my $PathColor = '{99 108 118}';
	my $SymLinkColor = '{60 123 133}';
	my $LabelColor = '{160 170 182}';
	my $InfoColor = '{141 153 165}';
	my $FileContentColor = $BackgroundColor;
	my $DirContentColor = '{17 85 238}';

	my $Height = $tallness;

	my $f=($tallness/(1.0/3.0)+1.0)/2.0;

	my $MinAltVW=25.0/$f;
	my $MinContentVW=45.0/$f;

	my $BackgroundX = 0.0;
	my $BackgroundY = 0.0;
	my $BackgroundW = 1.0;
	my $BackgroundH = $tallness;
	my $BackgroundRX = 0;
	my $BackgroundRY = 0;

	my $OuterBorderX = 0.0;
	my $OuterBorderY = 0.0;
	my $OuterBorderW = 1.0;
	my $OuterBorderH = $tallness;
	my $OuterBorderImg = 'CloudOuterBorder.tga';
	my $OuterBorderImgL = 256.0;
	my $OuterBorderImgT = 192.0;
	my $OuterBorderImgR = 256.0;
	my $OuterBorderImgB = 192.0;
	my $OuterBorderL = $OuterBorderImgL*$OuterBorderW/1536.0;
	my $OuterBorderT = $OuterBorderImgT*$OuterBorderH/896.0;
	my $OuterBorderR = $OuterBorderImgR*$OuterBorderW/1536.0;
	my $OuterBorderB = $OuterBorderImgB*$OuterBorderH/896.0;

	my $x1=$OuterBorderL;
	my $y1=$OuterBorderT;
	my $x2=1.0-$OuterBorderR;
	my $y2=$tallness-$OuterBorderB;

	my $NameX=$x1;
	my $NameY=$y1;
	my $NameW=$x2-$x1;
	my $NameH=0.09*$NameW;
	my $NameAlignment = "left";

	my $PathY=$NameY+$NameH*1.1;
	my $PathH=$NameH*0.2;
	my $PathAlignment = "bottom-left";

	my $DirInnerBorderImg = "CloudInnerBorder.tga";
	my $DirInnerBorderImgL = 192.0;
	my $DirInnerBorderImgT = 128.0;
	my $DirInnerBorderImgR = 192.0;
	my $DirInnerBorderImgB = 128.0;
	my $DirInnerBorderY=$PathY+$PathH;
	my $DirInnerBorderH=$y2-$DirInnerBorderY;
	my $DirInnerBorderT = $DirInnerBorderImgT*$DirInnerBorderH/760.0;
	my $DirInnerBorderB = $DirInnerBorderImgB*$DirInnerBorderH/760.0;
	my $DirContentY=$DirInnerBorderY+$DirInnerBorderT;
	my $DirContentH=$DirInnerBorderH-$DirInnerBorderT-$DirInnerBorderB;
	my $DirContentW=$DirContentH/$tallness;
	my $DirInnerBorderW=$DirContentW*1248.0/(1248.0-$DirInnerBorderImgL-$DirInnerBorderImgR);
	my $DirInnerBorderL = $DirInnerBorderImgL*$DirInnerBorderW/1248.0;
	my $DirInnerBorderR = $DirInnerBorderImgR*$DirInnerBorderW/1248.0;
	my $DirInnerBorderX=$x2-$DirInnerBorderW;
	my $DirContentX=$DirInnerBorderX+$DirInnerBorderL;

	my $FileInnerBorderY=$DirInnerBorderY + $PathH * 0.2;
	my $FileInnerBorderH=$y2 - $DirInnerBorderB * 0.2 - $FileInnerBorderY;
	my $FileInnerBorderW=$FileInnerBorderH/$tallness;
	my $FileInnerBorderX=$x2 - $DirInnerBorderR * 0.2 - $FileInnerBorderW;
	my $FileInnerBorderL=0;
	my $FileInnerBorderT=0;
	my $FileInnerBorderR=0;
	my $FileInnerBorderB=0;
	my $FileInnerBorderImg="";
	my $FileInnerBorderImgL=0;
	my $FileInnerBorderImgT=0;
	my $FileInnerBorderImgR=0;
	my $FileInnerBorderImgB=0;
	my $FileContentX=$FileInnerBorderX;
	my $FileContentY=$FileInnerBorderY;
	my $FileContentW=$FileInnerBorderW;
	my $FileContentH=$FileInnerBorderH;

	my $AltY=$PathY;
	my $AltH=$PathH;

	my $PathX=$FileInnerBorderX;

	my $InfoX=$x1;
	my $InfoY=$PathY;
	my $InfoW=$PathX-$PathH*0.5-$InfoX;
	my $InfoH=$y2-$InfoY;
	my $InfoAlignment = "top-left";

	my $AltLabelX = 0.0;
	my $AltLabelY = 0.0;
	my $AltLabelW = 1.0;
	my $AltLabelH = 0.012;
	my $AltLabelAlignment = "left";
	my $AltPathX = $AltLabelX;
	my $AltPathY = $AltLabelY + $AltLabelH * 1.1;
	my $AltPathH = $AltLabelH * 2.5;
	my $AltPathAlignment = "bottom-left";
	my $AltAltY = $AltPathY;
	my $AltAltH = $AltPathH;
	my $AltInnerBorderX = 0.0;
	my $AltInnerBorderY = $AltPathY + $AltPathH + $AltPathH * 0.2;
	my $AltInnerBorderW = 1.0;
	my $AltInnerBorderH = $AltInnerBorderW*$tallness;
	my $AltInnerBorderL = 0;
	my $AltInnerBorderT = 0;
	my $AltInnerBorderR = 0;
	my $AltInnerBorderB = 0;
	my $AltInnerBorderImg = "";
	my $AltInnerBorderImgL = 0;
	my $AltInnerBorderImgT = 0;
	my $AltInnerBorderImgR = 0;
	my $AltInnerBorderImgB = 0;
	my $AltContentX=$AltInnerBorderX;
	my $AltContentY=$AltInnerBorderY;
	my $AltContentW=$AltInnerBorderW;
	my $AltContentH=$AltInnerBorderH;

	my $AltW=$AltH*$AltInnerBorderW/($AltInnerBorderY+$AltInnerBorderH);
	my $AltX=$FileInnerBorderX+$FileInnerBorderW-$AltW;
	my $PathW=$AltX-$PathX-$PathH*0.4;

	my $AltAltW=$AltAltH*$AltW/$AltH;
	my $AltAltX=1.0-$AltAltW;
	my $AltPathW=$AltAltX-$AltPathX-$AltPathH*0.4;

	my $fh;
	open($fh,">",$fileName);

	print($fh
		"#%rec:emFileManTheme%#\n".
		"\n".
		"DisplayName = \"$DisplayName\"\n".
		"BackgroundColor = $BackgroundColor\n".
		"SourceSelectionColor = $SourceSelectionColor\n".
		"TargetSelectionColor = $TargetSelectionColor\n".
		"NormalNameColor = $NormalNameColor\n".
		"ExeNameColor = $ExeNameColor\n".
		"DirNameColor = $DirNameColor\n".
		"FifoNameColor = $FifoNameColor\n".
		"BlkNameColor = $BlkNameColor\n".
		"ChrNameColor = $ChrNameColor\n".
		"SockNameColor = $SockNameColor\n".
		"OtherNameColor = $OtherNameColor\n".
		"PathColor = $PathColor\n".
		"SymLinkColor = $SymLinkColor\n".
		"LabelColor = $LabelColor\n".
		"InfoColor = $InfoColor\n".
		"FileContentColor = $FileContentColor\n".
		"DirContentColor = $DirContentColor\n".
		"Height = $Height\n".
		"BackgroundX = $BackgroundX\n".
		"BackgroundY = $BackgroundY\n".
		"BackgroundW = $BackgroundW\n".
		"BackgroundH = $BackgroundH\n".
		"BackgroundRX = $BackgroundRX\n".
		"BackgroundRY = $BackgroundRY\n".
		"OuterBorderX = $OuterBorderX\n".
		"OuterBorderY = $OuterBorderY\n".
		"OuterBorderW = $OuterBorderW\n".
		"OuterBorderH = $OuterBorderH\n".
		"OuterBorderL = $OuterBorderL\n".
		"OuterBorderT = $OuterBorderT\n".
		"OuterBorderR = $OuterBorderR\n".
		"OuterBorderB = $OuterBorderB\n".
		"OuterBorderImg = \"$OuterBorderImg\"\n".
		"OuterBorderImgL = $OuterBorderImgL\n".
		"OuterBorderImgT = $OuterBorderImgT\n".
		"OuterBorderImgR = $OuterBorderImgR\n".
		"OuterBorderImgB = $OuterBorderImgB\n".
		"NameX = $NameX\n".
		"NameY = $NameY\n".
		"NameW = $NameW\n".
		"NameH = $NameH\n".
		"NameAlignment = $NameAlignment\n".
		"PathX = $PathX\n".
		"PathY = $PathY\n".
		"PathW = $PathW\n".
		"PathH = $PathH\n".
		"PathAlignment = $PathAlignment\n".
		"InfoX = $InfoX\n".
		"InfoY = $InfoY\n".
		"InfoW = $InfoW\n".
		"InfoH = $InfoH\n".
		"InfoAlignment = $InfoAlignment\n".
		"FileInnerBorderX = $FileInnerBorderX\n".
		"FileInnerBorderY = $FileInnerBorderY\n".
		"FileInnerBorderW = $FileInnerBorderW\n".
		"FileInnerBorderH = $FileInnerBorderH\n".
		"FileInnerBorderL = $FileInnerBorderL\n".
		"FileInnerBorderT = $FileInnerBorderT\n".
		"FileInnerBorderR = $FileInnerBorderR\n".
		"FileInnerBorderB = $FileInnerBorderB\n".
		"FileInnerBorderImg = \"$FileInnerBorderImg\"\n".
		"FileInnerBorderImgL = $FileInnerBorderImgL\n".
		"FileInnerBorderImgT = $FileInnerBorderImgT\n".
		"FileInnerBorderImgR = $FileInnerBorderImgR\n".
		"FileInnerBorderImgB = $FileInnerBorderImgB\n".
		"FileContentX = $FileContentX\n".
		"FileContentY = $FileContentY\n".
		"FileContentW = $FileContentW\n".
		"FileContentH = $FileContentH\n".
		"DirInnerBorderX = $DirInnerBorderX\n".
		"DirInnerBorderY = $DirInnerBorderY\n".
		"DirInnerBorderW = $DirInnerBorderW\n".
		"DirInnerBorderH = $DirInnerBorderH\n".
		"DirInnerBorderL = $DirInnerBorderL\n".
		"DirInnerBorderT = $DirInnerBorderT\n".
		"DirInnerBorderR = $DirInnerBorderR\n".
		"DirInnerBorderB = $DirInnerBorderB\n".
		"DirInnerBorderImg = \"$DirInnerBorderImg\"\n".
		"DirInnerBorderImgL = $DirInnerBorderImgL\n".
		"DirInnerBorderImgT = $DirInnerBorderImgT\n".
		"DirInnerBorderImgR = $DirInnerBorderImgR\n".
		"DirInnerBorderImgB = $DirInnerBorderImgB\n".
		"DirContentX = $DirContentX\n".
		"DirContentY = $DirContentY\n".
		"DirContentW = $DirContentW\n".
		"DirContentH = $DirContentH\n".
		"AltX = $AltX\n".
		"AltY = $AltY\n".
		"AltW = $AltW\n".
		"AltH = $AltH\n".
		"AltLabelX = $AltLabelX\n".
		"AltLabelY = $AltLabelY\n".
		"AltLabelW = $AltLabelW\n".
		"AltLabelH = $AltLabelH\n".
		"AltLabelAlignment = $AltLabelAlignment\n".
		"AltPathX = $AltPathX\n".
		"AltPathY = $AltPathY\n".
		"AltPathW = $AltPathW\n".
		"AltPathH = $AltPathH\n".
		"AltPathAlignment = $AltPathAlignment\n".
		"AltAltX = $AltAltX\n".
		"AltAltY = $AltAltY\n".
		"AltAltW = $AltAltW\n".
		"AltAltH = $AltAltH\n".
		"AltInnerBorderX = $AltInnerBorderX\n".
		"AltInnerBorderY = $AltInnerBorderY\n".
		"AltInnerBorderW = $AltInnerBorderW\n".
		"AltInnerBorderH = $AltInnerBorderH\n".
		"AltInnerBorderL = $AltInnerBorderL\n".
		"AltInnerBorderT = $AltInnerBorderT\n".
		"AltInnerBorderR = $AltInnerBorderR\n".
		"AltInnerBorderB = $AltInnerBorderB\n".
		"AltInnerBorderImg = \"$AltInnerBorderImg\"\n".
		"AltInnerBorderImgL = $AltInnerBorderImgL\n".
		"AltInnerBorderImgT = $AltInnerBorderImgT\n".
		"AltInnerBorderImgR = $AltInnerBorderImgR\n".
		"AltInnerBorderImgB = $AltInnerBorderImgB\n".
		"AltContentX = $AltContentX\n".
		"AltContentY = $AltContentY\n".
		"AltContentW = $AltContentW\n".
		"AltContentH = $AltContentH\n".
		"MinContentVW = $MinContentVW\n".
		"MinAltVW = $MinAltVW\n"
	);

	close($fh);
}


my $ThemeName = basename($0);
$ThemeName =~ s/(^gen)|(.pl$)//g;

generate_theme(7.0/12.0 , "${ThemeName}" , "../${ThemeName}.emFileManTheme");
