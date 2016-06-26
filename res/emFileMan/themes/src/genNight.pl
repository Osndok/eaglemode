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

	my $BackgroundColor = '{44 51 69}';
	my $SourceSelectionColor = '{4 84 24}';
	my $TargetSelectionColor = '{106 9 31}';
	my $NormalNameColor = '{226 231 255 195}';
	my $ExeNameColor = '{71 220 89 192}';
	my $DirNameColor = '{85 130 255 244}';
	my $FifoNameColor = '{242 82 152 190}';
	my $BlkNameColor = '{212 141 52 194}';
	my $ChrNameColor = '{163 192 46 191}';
	my $SockNameColor = '{221 76 218 191}';
	my $OtherNameColor = '{243 60 34 188}';
	my $PathColor = '{174 188 230 163}';
	my $SymLinkColor = '{186 255 254 219}';
	my $LabelColor = '{174 188 230 85}';
	my $InfoColor = '{174 188 230 103}';
	my $FileContentColor = '{44 51 69}';
	my $DirContentColor = '{34 40 56}';

	my $Height = $tallness;

	my $frame=sqrt($tallness)*0.025;

	my $BackgroundX = 0.0;
	my $BackgroundY = 0.0;
	my $BackgroundW = 1-$frame*(140.0/150.0);
	my $BackgroundH = $tallness-$frame*(140.0/150.0);
	my $BackgroundRX = $frame;
	my $BackgroundRY = $frame;

	my $OuterBorderX = 0.0;
	my $OuterBorderY = 0.0;
	my $OuterBorderW = 1.0;
	my $OuterBorderH = $tallness;
	my $OuterBorderImg = 'NightOuterBorder.tga';
	my $OuterBorderImgL = 270.0;
	my $OuterBorderImgT = 350.0;
	my $OuterBorderImgR = 390.0;
	my $OuterBorderImgB = 310.0;
	my $OuterBorderL = $frame*($OuterBorderImgL/150.0);
	my $OuterBorderT = $frame*($OuterBorderImgT/150.0);
	my $OuterBorderR = $frame*($OuterBorderImgR/150.0);
	my $OuterBorderB = $frame*($OuterBorderImgB/150.0);

	my $x1=$BackgroundX+$frame*1.3;
	my $y1=$BackgroundY+$frame*1.3;
	my $x2=$BackgroundX+$BackgroundW-$frame*1.3;
	my $y2=$BackgroundY+$BackgroundH-$frame*1.3;

	my $NameX=$x1;
	my $NameY=$y1;
	my $NameW=$x2-$x1;
	my $NameH=0.099*$NameW;
	my $NameAlignment = "left";

	my $contentFrame=$frame*0.21;

	my $PathY=$NameY+$NameH+$contentFrame*1.2;
	my $PathH=$contentFrame*3.0;
	my $PathAlignment = "bottom-left";

	my $FileInnerBorderY=$PathY+$PathH+$contentFrame*0.16;
	my $FileInnerBorderH=$y2-$FileInnerBorderY;
	my $FileInnerBorderImg = "NightInnerBorder.tga";
	my $FileInnerBorderImgL = 288.0;
	my $FileInnerBorderImgT = 288.0;
	my $FileInnerBorderImgR = 288.0;
	my $FileInnerBorderImgB = 288.0;
	my $FileInnerBorderL = $contentFrame*($FileInnerBorderImgL/280.0);
	my $FileInnerBorderT = $contentFrame*($FileInnerBorderImgT/280.0);
	my $FileInnerBorderR = $contentFrame*($FileInnerBorderImgR/280.0);
	my $FileInnerBorderB = $contentFrame*($FileInnerBorderImgB/280.0);
	my $FileContentY=$FileInnerBorderY+$contentFrame;
	my $FileContentH=$FileInnerBorderH-$contentFrame-$contentFrame;
	my $FileContentW=$FileContentH/$tallness;
	my $FileInnerBorderW=$FileContentW+$contentFrame+$contentFrame;
	my $FileInnerBorderX=$x2-$FileInnerBorderW;
	my $FileContentX=$FileInnerBorderX+$contentFrame;

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
	my $InfoW=$FileInnerBorderX-$x1-$contentFrame;
	my $InfoH=$y2-$InfoY;
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
	my $AltInnerBorderImg = "NightInnerBorder.tga";
	my $AltInnerBorderImgL = 288.0;
	my $AltInnerBorderImgT = 288.0;
	my $AltInnerBorderImgR = 288.0;
	my $AltInnerBorderImgB = 288.0;
	my $AltInnerBorderL = $altContentFrame*($AltInnerBorderImgL/280.0);
	my $AltInnerBorderT = $altContentFrame*($AltInnerBorderImgT/280.0);
	my $AltInnerBorderR = $altContentFrame*($AltInnerBorderImgR/280.0);
	my $AltInnerBorderB = $altContentFrame*($AltInnerBorderImgB/280.0);
	my $AltContentX=$AltInnerBorderX+$altContentFrame;
	my $AltContentY=$AltInnerBorderY+$altContentFrame;
	my $AltContentW=$AltInnerBorderW-$altContentFrame-$altContentFrame;
	my $AltContentH=$AltContentW*$tallness;
	my $AltInnerBorderH=$AltContentH+$altContentFrame+$altContentFrame;

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

generate_theme(3.0/8.0 , "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}1.emFileManTheme");
generate_theme(9.0/16.0, "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}2.emFileManTheme");
generate_theme(3.0/4.0 , "${DisplayName}", "theme_${ThemeName}.tga", "../${ThemeName}3.emFileManTheme");
