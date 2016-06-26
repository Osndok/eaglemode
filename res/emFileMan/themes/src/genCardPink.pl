#!/usr/bin/perl

use strict;
use warnings;
use File::Basename;


sub generate_theme
	# Arguments: tallness, display name, display icon, file
{
	my $tallness = shift;
	my $DisplayName = shift;
	my $DisplayIcon = shift;
	my $fileName = shift;

	my $BackgroundColor = '{221 214 224}';
	my $SourceSelectionColor = '{158 232 168}';
	my $TargetSelectionColor = '{254 156 177}';
	my $NormalNameColor = '{31 30 34}';
	my $ExeNameColor = '{0 90 0}';
	my $DirNameColor = '{0 28 184}';
	my $FifoNameColor = '{229 0 144}';
	my $BlkNameColor = '{136 89 0}';
	my $ChrNameColor = '{102 122 0}';
	my $SockNameColor = '{208 20 208}';
	my $OtherNameColor = '{193 24 0}';
	my $PathColor = '{87 83 97}';
	my $SymLinkColor = '{69 138 138}';
	my $LabelColor = '{153 146 161}';
	my $InfoColor = '{129 125 140}';
	my $FileContentColor = '{221 214 224}';
	my $DirContentColor = '{193 160 204}';

	my $Height = $tallness;

	my $frame=sqrt($tallness)*0.041;

	my $BackgroundX = 0.0;
	my $BackgroundY = 0.0;
	my $BackgroundW = 1-$frame*(150.0/150.0);
	my $BackgroundH = $tallness-$frame*(150.0/150.0);
	my $BackgroundRX = $frame;
	my $BackgroundRY = $frame;

	my $OuterBorderX = 0.0;
	my $OuterBorderY = 0.0;
	my $OuterBorderW = 1.0;
	my $OuterBorderH = $tallness;
	my $OuterBorderImg = 'CardOuterBorder.tga';
	my $OuterBorderImgL = 250.0;
	my $OuterBorderImgT = 260.0;
	my $OuterBorderImgR = 390.0;
	my $OuterBorderImgB = 340.0;
	my $OuterBorderL = $frame*($OuterBorderImgL/150.0);
	my $OuterBorderT = $frame*($OuterBorderImgT/150.0);
	my $OuterBorderR = $frame*($OuterBorderImgR/150.0);
	my $OuterBorderB = $frame*($OuterBorderImgB/150.0);

	my $x1=$BackgroundX+$frame*0.71;
	my $y1=$BackgroundY+$frame*0.66;
	my $x2=$BackgroundX+$BackgroundW-$frame*0.66;
	my $y2=$BackgroundY+$BackgroundH-$frame*0.66;

	my $NameX=$x1;
	my $NameY=$y1;
	my $NameW=$x2-$x1;
	my $NameH=0.0976*$NameW;
	my $NameAlignment = "left";

	my $contentFrame=$frame*0.132;

	my $PathY=$NameY+$NameH+$contentFrame*1.2;
	my $PathH=$contentFrame*3.0;
	my $PathAlignment = "bottom-left";

	my $FileInnerBorderY=$PathY+$PathH+$contentFrame*0.001;
	my $FileInnerBorderH=$y2-$FileInnerBorderY;
	my $FileInnerBorderL = $contentFrame;
	my $FileInnerBorderT = $contentFrame;
	my $FileInnerBorderR = $contentFrame;
	my $FileInnerBorderB = $contentFrame;
	my $FileInnerBorderImg = "CardInnerBorder.tga";
	my $FileInnerBorderImgL = 350.0;
	my $FileInnerBorderImgT = 350.0;
	my $FileInnerBorderImgR = 350.0;
	my $FileInnerBorderImgB = 350.0;
	my $FileContentY=$FileInnerBorderY+$FileInnerBorderT;
	my $FileContentH=$FileInnerBorderH-$FileInnerBorderT-$FileInnerBorderB;
	my $FileContentW=$FileContentH/$tallness;
	my $FileInnerBorderW=$FileContentW+$FileInnerBorderL+$FileInnerBorderR;
	my $FileInnerBorderX=$x2-$FileInnerBorderW;
	my $FileContentX=$FileInnerBorderX+$FileInnerBorderL;

	my $DirInnerBorderX=$FileInnerBorderX;
	my $DirInnerBorderY=$FileInnerBorderY;
	my $DirInnerBorderW=$FileInnerBorderW;
	my $DirInnerBorderH=$FileInnerBorderH;
	my $DirInnerBorderL=$FileInnerBorderL;
	my $DirInnerBorderT=$FileInnerBorderT;
	my $DirInnerBorderR=$FileInnerBorderR;
	my $DirInnerBorderB=$FileInnerBorderB;
	my $DirInnerBorderImg=$FileInnerBorderImg;
	my $DirInnerBorderImgL=$FileInnerBorderImgL;
	my $DirInnerBorderImgT=$FileInnerBorderImgT;
	my $DirInnerBorderImgR=$FileInnerBorderImgR;
	my $DirInnerBorderImgB=$FileInnerBorderImgB;
	my $DirContentX=$FileContentX;
	my $DirContentY=$FileContentY;
	my $DirContentW=$FileContentW;
	my $DirContentH=$FileContentH;

	my $AltY=$PathY;
	my $AltH=$PathH;

	my $PathX=$FileInnerBorderX;

	my $InfoX=$x1;
	my $InfoY=$PathY;
	my $InfoW=$FileInnerBorderX-$InfoX-$contentFrame*3.4;
	my $InfoH=$y2-$InfoY-$contentFrame*0.6;
	my $InfoAlignment = "top-left";

	my $altContentFrame=$contentFrame/$FileInnerBorderW;

	my $AltLabelX = 0.0;
	my $AltLabelY = 0.0;
	my $AltLabelW = 1.0;
	my $AltLabelH = 2.0*$altContentFrame;
	my $AltLabelAlignment = "left";
	my $AltPathX = $AltLabelX;
	my $AltPathY = $AltLabelY+$AltLabelH;
	my $AltPathH = 5.0*$altContentFrame;
	my $AltPathAlignment = "bottom-left";
	my $AltAltY = $AltPathY;
	my $AltAltH = $AltPathH;
	my $AltInnerBorderX=0.0;
	my $AltInnerBorderY=$AltPathY+$AltPathH+$altContentFrame*0.16666665;
	my $AltInnerBorderW=1.0;
	my $AltInnerBorderL = $altContentFrame;
	my $AltInnerBorderT = $altContentFrame;
	my $AltInnerBorderR = $altContentFrame;
	my $AltInnerBorderB = $altContentFrame;
	my $AltInnerBorderImg = "CardInnerBorder.tga";
	my $AltInnerBorderImgL = 350.0;
	my $AltInnerBorderImgT = 350.0;
	my $AltInnerBorderImgR = 350.0;
	my $AltInnerBorderImgB = 350.0;
	my $AltContentX=$AltInnerBorderX+$AltInnerBorderL;
	my $AltContentY=$AltInnerBorderY+$AltInnerBorderT;
	my $AltContentW=$AltInnerBorderW-$AltInnerBorderL-$AltInnerBorderR;
	my $AltContentH=$AltContentW*$tallness;
	my $AltInnerBorderH=$AltContentH+$AltInnerBorderT+$AltInnerBorderB;

	my $AltW=$AltH*$AltInnerBorderW/($AltInnerBorderY+$AltInnerBorderH);
	my $AltX=$x2-$AltW;
	my $PathW=$AltX-$PathX-$PathH*0.4;

	my $AltAltW=$AltAltH*$AltW/$AltH;
	my $AltAltX=1.0-$AltAltW;
	my $AltPathW=$AltAltX-$AltPathX-$AltPathH*0.4;

	my $MinContentVW=26.0/sqrt($Height);
	my $MinAltVW=15.0/sqrt($Height);

	my $px=1.0-$BackgroundW-$BackgroundX*2;
	my $py=$Height-$BackgroundH-$BackgroundY*2;
	my $DirPaddingL=$px;
	my $DirPaddingT=$py;
	my $DirPaddingR=0.0;
	my $DirPaddingB=0.0;
	my $LnkPaddingL=$px*0.5;
	my $LnkPaddingT=$py*0.5;
	my $LnkPaddingR=-$px*0.5;
	my $LnkPaddingB=-$py*0.5;

	my $fh;
	open($fh,">",$fileName);

	print($fh
		"#%rec:emFileManTheme%#\n".
		"\n".
		"DisplayName = \"$DisplayName\"\n".
		"DisplayIcon = \"$DisplayIcon\"\n".
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
		"MinAltVW = $MinAltVW\n".
		"DirPaddingL = $DirPaddingL\n".
		"DirPaddingT = $DirPaddingT\n".
		"DirPaddingR = $DirPaddingR\n".
		"DirPaddingB = $DirPaddingB\n".
		"LnkPaddingL = $LnkPaddingL\n".
		"LnkPaddingT = $LnkPaddingT\n".
		"LnkPaddingR = $LnkPaddingR\n".
		"LnkPaddingB = $LnkPaddingB\n"
	);

	close($fh);
}


my $ThemeName = basename($0);
$ThemeName =~ s/(^gen)|(.pl$)//g;

my $DisplayName = $ThemeName;
if ($DisplayName =~ /^Card(.+)$/) {
	$DisplayName = "Card/$1";
}

generate_theme(3.0/8.0 , "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}1.emFileManTheme");
generate_theme(9.0/16.0, "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}2.emFileManTheme");
generate_theme(3.0/4.0 , "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}3.emFileManTheme");
