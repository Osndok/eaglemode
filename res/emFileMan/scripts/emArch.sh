#!/bin/sh

#===============================================================================
#================================= Description =================================
#===============================================================================

Description="\
emArch.sh

Copyright (C) 2007-2008,2010-2011 Oliver Hamann.

Homepage: http://eaglemode.sourceforge.net/

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU General Public License version 3 as published by the
Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License version 3 for
more details.

You should have received a copy of the GNU General Public License version 3
along with this program. If not, see <http://www.gnu.org/licenses/>.

DESCRIPTION:

This shell script program can pack and unpack archive files of various formats
by running the appropriate system tools. It is used by emTmpConv for showing
the contents of an archive, and it is used by some user commands of emFileMan.

SUPPORTED FORMATS:

Archive file name suffices    | Required system tools
------------------------------+----------------------
.7z                           | 7z
.ar|.a|.deb|.ipk              | ar
.arc                          | arc
.arj                          | arj
.bz             (unpack only) | bzip2
.bz2                          | bzip2
.gz                           | gzip
.lzh|.lha                     | lha
.lzma                         | xz
.lzo                          | lzop
.rar            (unpack only) | unrar
.tar                          | tar
.tar.bz|.tbz    (unpack only) | tar, bzip2
.tar.bz2|.tbz2|.tgj           | tar, bzip2
.tar.gz|.tgz                  | tar, gzip
.tar.lzma|.tlz                | tar, xz
.tar.lzo|.tzo                 | tar, lzop
.tar.xz|.txz                  | tar, xz
.tar.Z|.taz     (unpack only) | tar, gzip
.xz                           | xz
.Z              (unpack only) | gzip
.zip|.jar                     | zip, unzip
.zoo                          | zoo

SECURITY / SPEED-LOSS:

At least because of the heavy use through emTmpConv, this script program tries
its best in avoiding the creation of unpacked files outside the current
directory, even through nasty archives. Mostly, the archive listing is inspected
and the unpacking is refused if the listing contains an absolute path or an
up-going relative path.

Unfortunately, that listing of an archive can cost a lot of extra time,
especially with tar-based archives. In Addition, the test is mostly a little
bit heuristic and may falsely refuse archives with harmless paths like
\"bla /bla\" and \"bla ../bla\".

This script has a configuration section where the security test can be disabled
on a per-format basis.

USAGE:

  $0 pack|p [<options>] [--] <archive file> <source file|dir>...

  $0 unpack|u [<options>] [--] <archive file>

With the first variant, the archive file is created from the source files and
directories recursively (but some archive formats can pack only a single file).
With the second variant, the archive file is read and the contained files and
directories are unpacked into the current working directory.

OPTIONS:

  -f|--format <format>
      Force archive file format. The default is to detect the format from the
      archive file name suffix. The format can be specified by a sample file
      name or path with correct suffix, or just by the suffix with our without
      the leading dot.

  -g|--trust
      Go on and trust the archive or the unpack tool. This disables the test
      for absolute or up-going paths when unpacking.

  -h|--help
      Print this help and exit.
"


#===============================================================================
#================================ Configuration ================================
#===============================================================================

# If you trust one of your installed archive programs that it never unpacks
# files outside the current directory through nasty or non-nasty archives
# containing absolute paths or up-going paths, then you could disable the
# security test of this script by saying "yes" to the appropriate variable
# below. If in doubt, keep the default of "no" everywhere.
# Hints:
#  - There should be no appreciable speed loss if set to "no", except for
#    Trust_tar.
#  - With some archive formats, absolute paths are something normal (e.g. tar).
#    This increases the danger.
#  - Younger version of GNU tar should be safe concerning absolute paths (they
#    have the option -P|--absolute-paths which is not given here), but what
#    about nasty tar archives containing up-going paths?

Trust_7z=no
Trust_arc=no
Trust_arj=no
Trust_ar=no
Trust_lha=no
Trust_tar=no
Trust_unrar=no
Trust_unzip=no
Trust_zoo=no


#===============================================================================
#========================== Start-up / General Stuff ===========================
#===============================================================================

Error ()
{
	echo "Error: $*" 1>&2
	exit 1
}

ErrorBadArgs ()
{
	Error "Bad arguments. Type: $0 --help"
}

if test $# = 0 ; then
	echo "$Description"
	exit 1
fi

# This is a paranoid test whether 'egrep -v' has the expected behavior, because
# our security checks depend on it. The alternative option --invert-match is not
# supported by every egrep implementation. Besides, I saw an egrep where
# [[:space:]] did not work, and therefore I always say ( |[[:space:]]) instead.
echo "xcbax" | egrep -v "abc" > /dev/null
r1=$?
echo "xabcx" | egrep -v "abc" > /dev/null
r2=$?
if test $r1 != 0 || test $r2 != 1 ; then
	Error "Testing 'egrep -v' failed."
fi

Cmd="$1"
shift
case "$Cmd" in
	unpack|u)
		Cmd=unpack
	;;
	pack|p)
		Cmd=pack
	;;
	*)
		ErrorBadArgs
	;;
esac

TrustByOption=no
Format=""

while test $# != 0 ; do
	case "$1" in
		-g|--trust)
			shift
			TrustByOption=yes
		;;
		-f|--format)
			shift
			Format="$1"
			shift
		;;
		-h|--help)
			echo "$Description"
			exit 0
		;;
		--)
			shift
			break
		;;
		-*)
			Error "Illegal option: $1"
			break
		;;
		*)
			break
		;;
	esac
done

if test $# = 0 ; then
	Error "Missing argument: archive file"
fi
ArchFile="$1"
shift

if test "$Format" = "" ; then
	Format="$ArchFile"
fi

if test "$TMPDIR" = "" ; then
	TMPDIR="/tmp"
fi

ErrorHintFile="$TMPDIR/emArch-error-hint-$$"
if test -f "$ErrorHintFile" ; then
	rm -f "$ErrorHintFile" || exit 1
fi

SetErrorHint()
{
	echo > "$ErrorHintFile"
}

CheckErrorHint ()
{
	if test -f "$ErrorHintFile" ; then
		rm -f "$ErrorHintFile"
		if test $Cmd = pack ; then
			rm -f "$ArchFile"
		fi
		exit 1
	fi
}

CheckErrorHintAnd ()
{
	CheckErrorHint
	if test $1 != 0 ; then
		if test $Cmd = pack ; then
			rm -f "$ArchFile"
		fi
		exit 1
	fi
}

if test $Cmd = pack ; then
#===============================================================================
#=================================== Packing ===================================
#===============================================================================

if test $# = 0 ; then
	Error "Missing argument: file or directory to pack"
fi

case ".$Format" in

#----------------------------------- pack 7z -----------------------------------
*.7z|*.7Z)

basename "$ArchFile" | egrep ".\.." > /dev/null
case $? in
	0)
		break
	;;
	1)
		Error "Archive file name has no suffix (e.g. \".7z\")."
		# Otherwise 7z automatically appends ".7z" (would
		# break the semantics of this script interface).
	;;
	*)
		exit 1
	;;
esac

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec 7z a "$ArchFile" "$@"

;;
#----------------------------------- pack ar -----------------------------------
*.ar|*.AR|*.Ar|\
*.a|*.A|\
*.deb|*.DEB|*.Deb\
*.ipk|*.IPK|*.Ipk)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec ar rc "$ArchFile" "$@"

;;
#---------------------------------- pack arc -----------------------------------
*.arc|*.ARC|*.Arc)

basename "$ArchFile" | egrep "^[^.]?[^.]?[^.]?[^.]?[^.]?[^.]?[^.]?[^.]?\..?.?.?$" > /dev/null
case $? in
	0)
		break
	;;
	1)
		Error "arc archive file name must be a DOS file name (<max 8 non-dot chars><dot><max 3 chars>)."
		# Otherwise arc automatically shortens the file name and
		# possibly appends ".arc" (would break the semantics of this
		# script interface).
	;;
	*)
		exit 1
	;;
esac

for i in "$@" ; do
	if test -d "$i" ; then
		Error "Cannot pack a directory with arc."
	fi
done

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec arc a "$ArchFile" "$@"

;;
#---------------------------------- pack arj -----------------------------------
*.arj|*.ARJ|*.Arj)

basename "$ArchFile" | egrep ".\.." > /dev/null
case $? in
	0)
		break
	;;
	1)
		Error "Archive file name has no suffix (e.g. \".arj\")."
		# Otherwise arj automatically appends ".arj" (would
		# break the semantics of this script interface).
	;;
	*)
		exit 1
	;;
esac

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

# Something like
#   exec arj a -r "$ArchFile" "$@"
# does not work with multiple directories (tested with ARJ32 v 3.10)
for i in "$@" ; do
	arj a -r "$ArchFile" "$i" || exit 1
done

;;
#---------------------------------- pack lzh -----------------------------------
*.lzh|*.LZH|*.Lzh|\
*.lha|*.LHA|*.Lha)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec lha av "$ArchFile" "$@"

;;
#---------------------------------- pack tar -----------------------------------
*.tar|*.TAR|*.Tar)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec tar cvf "$ArchFile" "$@"

;;
#-------------------------------- pack tar.bz2 ---------------------------------
*.tar.bz2|*.TAR.bz2|*.Tar.bz2|\
*.tar.BZ2|*.TAR.BZ2|*.Tar.BZ2|\
*.tar.Bz2|*.TAR.Bz2|*.Tar.Bz2|\
*.tbz2|*.TBZ2|*.Tbz2|\
*.tgj|*.TGJ|*.Tgj)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

{ tar cvf - "$@" || SetErrorHint ; } | bzip2 -c > "$ArchFile"
CheckErrorHintAnd $?

;;
#--------------------------------- pack tar.gz ---------------------------------
*.tar.gz|*.TAR.gz|*.Tar.gz|\
*.tar.GZ|*.TAR.GZ|*.Tar.GZ|\
*.tar.Gz|*.TAR.Gz|*.Tar.Gz|\
*.tgz|*.TGZ|*.Tgz)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

{ tar cvf - "$@" || SetErrorHint ; } | gzip -c > "$ArchFile"
CheckErrorHintAnd $?

;;
#-------------------------------- pack tar.lzma --------------------------------
*.tar.lzma|*.TAR.lzma|*.Tar.lzma|\
*.tar.LZMA|*.TAR.LZMA|*.Tar.LZMA|\
*.tar.Lzma|*.TAR.Lzma|*.Tar.Lzma|\
*.tlz|*.TLZ|*.Tlz)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

{ tar cvf - "$@" || SetErrorHint ; } | xz --stdout --format=lzma > "$ArchFile"
CheckErrorHintAnd $?

;;
#-------------------------------- pack tar.lzo ---------------------------------
*.tar.lzo|*.TAR.lzo|*.Tar.lzo|\
*.tar.LZO|*.TAR.LZO|*.Tar.LZO|\
*.tar.Lzo|*.TAR.Lzo|*.Tar.Lzo|\
*.tzo|*.TZO|*.Tzo)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

{ tar cvf - "$@" || SetErrorHint ; } | lzop -c > "$ArchFile"
CheckErrorHintAnd $?

;;
#--------------------------------- pack tar.xz ---------------------------------
*.tar.xz|*.TAR.xz|*.Tar.xz|\
*.tar.XZ|*.TAR.XZ|*.Tar.XZ|\
*.tar.Xz|*.TAR.Xz|*.Tar.Xz|\
*.txz|*.TXZ|*.Txz)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

{ tar cvf - "$@" || SetErrorHint ; } | xz --stdout > "$ArchFile"
CheckErrorHintAnd $?

;;
#---------------------------------- pack zip -----------------------------------
*.zip|*.ZIP|*.Zip|\
*.jar|*.JAR|*.Jar)

basename "$ArchFile" | egrep ".\.." > /dev/null
case $? in
	0)
		break
	;;
	1)
		Error "Archive file name has no suffix (e.g. \".zip\")."
		# Otherwise zip automatically appends ".zip" (would
		# break the semantics of this script interface).
	;;
	*)
		exit 1
	;;
esac

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec zip -r -9 "$ArchFile" "$@"

;;
#---------------------------------- pack zoo -----------------------------------
*.zoo|*.ZOO|*.Zoo)

basename "$ArchFile" | egrep ".\.." > /dev/null
case $? in
	0)
		break
	;;
	1)
		Error "Archive file name has no suffix (e.g. \".zoo\")."
		# Otherwise zoo automatically appends ".zoo" (would
		# break the semantics of this script interface).
	;;
	*)
		exit 1
	;;
esac

for i in "$@" ; do
	if test -d "$i" ; then
		Error "Packing of directories into zoo archive not supported."
		# We would have to recurse the directories and give zoo the
		# paths of all the files.
	fi
done

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec zoo a "$ArchFile" "$@"

;;
#---------------------------------- pack bz2 -----------------------------------
*.bz2|*.BZ2|*.Bz2)

if test $# != 1 ; then
	Error "Cannot pack multiple files with bzip2."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory with bzip2."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec bzip2 -c "$1" > "$ArchFile"

;;
#----------------------------------- pack gz -----------------------------------
*.gz|*.GZ|*.Gz)

if test $# != 1 ; then
	Error "Cannot pack multiple files with gzip."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory with gzip."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec gzip -c "$1" > "$ArchFile"

;;
#---------------------------------- pack lzma ----------------------------------
*.lzma|*.LZMA|*.Lzma)

if test $# != 1 ; then
	Error "Cannot pack multiple files with lzma."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory with lzma."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec xz --stdout --format=lzma "$1" > "$ArchFile"

;;
#---------------------------------- pack lzo -----------------------------------
*.lzo|*.LZO|*.Lzo)

if test $# != 1 ; then
	Error "Cannot pack multiple files with lzo."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory with lzo."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec lzop -c "$1" > "$ArchFile"

;;
#----------------------------------- pack xz -----------------------------------
*.xz|*.XZ|*.Xz)

if test $# != 1 ; then
	Error "Cannot pack multiple files with xz."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory with xz."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec xz --stdout "$1" > "$ArchFile"

;;
#-------------------------------------------------------------------------------
*)
	Error "Packing of $Format not supported"
;;
esac

else
#===============================================================================
#================================== Unpacking ==================================
#===============================================================================

# No further arguments allowed.
if test $# != 0 ; then
	ErrorBadArgs
fi

case ".$Format" in

#---------------------------------- unpack 7z ----------------------------------
*.7z|*.7Z)

if test $TrustByOption != yes && test $Trust_7z != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		7z l "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | {
		egrep -v "^(Listing archive:|Path =)( |[[:space:]])*`echo "$ArchFile" | tr '\\\\^?*+|\!\$()[]{}' '.'`\$"
		case $? in
			0|1)
				break;
			;;
			*)
				SetErrorHint
			;;
		esac
	} | egrep "((^| |[[:space:]])(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec 7z x "$ArchFile"

;;
#---------------------------------- unpack ar ----------------------------------
*.ar|*.AR|*.Ar|\
*.a|*.A|\
*.deb|*.DEB|*.Deb\
*.ipk|*.IPK|*.Ipk)

if test $TrustByOption != yes && test $Trust_ar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		ar t "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec ar xv "$ArchFile"

;;
#--------------------------------- unpack arc ----------------------------------
*.arc|*.ARC|*.Arc)

if test $TrustByOption != yes && test $Trust_arc != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		arc l "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | egrep "((^| |[[:space:]])(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec arc x "$ArchFile"

;;
#--------------------------------- unpack arj ----------------------------------
*.arj|*.ARJ|*.Arj)

if test $TrustByOption != yes && test $Trust_arj != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		arj v "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | {
		egrep -v "^Processing archive:( |[[:space:]])*`echo "$ArchFile" | tr '\\\\^?*+|\!\$()[]{}' '.'`\$"
		case $? in
			0|1)
				break;
			;;
			*)
				SetErrorHint
			;;
		esac
	} | egrep "((^| |[[:space:]])(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec arj x -y "$ArchFile"

;;
#--------------------------------- unpack lzh ----------------------------------
*.lzh|*.LZH|*.Lzh|\
*.lha|*.LHA|*.Lha)

if test $TrustByOption != yes && test $Trust_lha != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		lha l "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | egrep "((^| |[[:space:]])(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec lha xv "$ArchFile"

;;
#--------------------------------- unpack rar ----------------------------------
*.rar|*.RAR|*.Rar)

if test $TrustByOption != yes && test $Trust_unrar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		unrar vb "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec unrar x -- "$ArchFile"

;;
#--------------------------------- unpack tar ----------------------------------
*.tar|*.TAR|*.Tar)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		tar tf "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec tar xvf "$ArchFile"

;;
#--------------------------- unpack tar.bz2 + tar.bz ---------------------------
*.tar.bz2|*.TAR.bz2|*.Tar.bz2|\
*.tar.BZ2|*.TAR.BZ2|*.Tar.BZ2|\
*.tar.Bz2|*.TAR.Bz2|*.Tar.Bz2|\
*.tbz2|*.TBZ2|*.Tbz2|\
*.tgj|*.TGJ|*.Tgj|\
*.tar.bz|*.TAR.bz|*.Tar.bz|\
*.tar.BZ|*.TAR.BZ|*.Tar.BZ|\
*.tar.Bz|*.TAR.Bz|*.Tar.Bz|\
*.tbz|*.TBZ|*.Tbz)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		bzip2 -d -c < "$ArchFile" || SetErrorHint
	} | {
		tar tf - || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

{ bzip2 -d -c < "$ArchFile" || SetErrorHint ; } | tar xvf -
CheckErrorHintAnd $?

;;
#---------------------------- unpack tar.gz + tar.Z ----------------------------
*.tar.gz|*.TAR.gz|*.Tar.gz|\
*.tar.GZ|*.TAR.GZ|*.Tar.GZ|\
*.tar.Gz|*.TAR.Gz|*.Tar.Gz|\
*.tgz|*.TGZ|*.Tgz|\
*.tar.z|*.TAR.z|*.Tar.z|\
*.tar.Z|*.TAR.Z|*.Tar.Z|\
*.taz|*.taZ|*.TAZ|*.Taz)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		gzip -d -c < "$ArchFile" || SetErrorHint
	} | {
		tar tf - || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

{ gzip -d -c < "$ArchFile" || SetErrorHint ; } | tar xvf -
CheckErrorHintAnd $?

;;
#-------------------------- unpack tar.lzma + tar.xz ---------------------------
*.tar.lzma|*.TAR.lzma|*.Tar.lzma|\
*.tar.LZMA|*.TAR.LZMA|*.Tar.LZMA|\
*.tar.Lzma|*.TAR.Lzma|*.Tar.Lzma|\
*.tlz|*.TLZ|*.Tlz|\
*.tar.xz|*.TAR.xz|*.Tar.xz|\
*.tar.XZ|*.TAR.XZ|*.Tar.XZ|\
*.tar.Xz|*.TAR.Xz|*.Tar.Xz|\
*.txz|*.TXZ|*.Txz)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		xz --decompress --stdout < "$ArchFile" || SetErrorHint
	} | {
		tar tf - || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

{ xz --decompress --stdout < "$ArchFile" || SetErrorHint ; } | tar xvf -
CheckErrorHintAnd $?

;;
#------------------------------- unpack tar.lzo --------------------------------
*.tar.lzo|*.TAR.lzo|*.Tar.lzo|\
*.tar.LZO|*.TAR.LZO|*.Tar.LZO|\
*.tar.Lzo|*.TAR.Lzo|*.Tar.Lzo|\
*.tzo|*.TZO|*.Tzo)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		lzop -d -c < "$ArchFile" || SetErrorHint
	} | {
		tar tf - || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "  /bla"
		# echo "  ../bla"
		# echo "bla/../bla"
	} | egrep "(^( |[[:space:]])*(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

{ lzop -d -c < "$ArchFile" || SetErrorHint ; } | tar xvf -
CheckErrorHintAnd $?

;;
#--------------------------------- unpack zip ----------------------------------
*.zip|*.ZIP|*.Zip|\
*.jar|*.JAR|*.Jar)

if test $TrustByOption != yes && test $Trust_unzip != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		unzip -l "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | {
		egrep -v "^Archive:( |[[:space:]])*`echo "$ArchFile" | tr '\\\\^?*+|\!\$()[]{}' '.'`\$"
		case $? in
			0|1)
				break;
			;;
			*)
				SetErrorHint
			;;
		esac
	} | egrep "((^| |[[:space:]])(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec unzip "$ArchFile"

;;
#--------------------------------- unpack zoo ----------------------------------
*.zoo|*.ZOO|*.Zoo)

if test $TrustByOption != yes && test $Trust_zoo != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		zoo l "$ArchFile" || SetErrorHint
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | {
		egrep -v "^Archive( |[[:space:]])*`echo "$ArchFile" | tr '\\\\^?*+|\!\$()[]{}' '.'`:\$"
		case $? in
			0|1)
				break;
			;;
			*)
				SetErrorHint
			;;
		esac
	} | egrep "((^| |[[:space:]])(/|\.\./))|(/\.\./)" > /dev/null
	ret=$?
	CheckErrorHint
	case $ret in
		0)
			Error "$ArchFile looks like containing an absolute or up-going path."
		;;
		1)
			break
		;;
		*)
			exit 1
		;;
	esac
	echo "okay, unpacking..."
fi

exec zoo x. "$ArchFile"

;;
#------------------------------- unpack bz2 + bz -------------------------------
*.bz2|*.BZ2|*.Bz2|\
*.bz|*.BZ|*.Bz)

n="`basename "$ArchFile"`"

if   b="`basename "$n" .bz2`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .BZ2`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Bz2`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .bZ2`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .bz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .BZ`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Bz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .bZ`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .tbz2`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TBZ2`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tbz2`" && test "$b" != "$n" ; then
	n="$b.Tar"
elif b="`basename "$n" .tbz`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TBZ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tbz`" && test "$b" != "$n" ; then
	n="$b.Tar"
elif b="`basename "$n" .tgj`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TGJ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tgj`" && test "$b" != "$n" ; then
	n="$b.Tar"
else
	n="$n.unpacked"
fi

if test -f "$n" || test -d "$n" ; then
	Error "File already exists: $n"
fi

exec bzip2 -d -c < "$ArchFile" > "$n"

;;
#-------------------------------- unpack gz + Z --------------------------------
*.gz|*.GZ|*.Gz|\
*.z|*.Z)

n="`basename "$ArchFile"`"

if   b="`basename "$n" .gz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .GZ`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Gz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .gZ`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .z`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Z`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .tgz`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TGZ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tgz`" && test "$b" != "$n" ; then
	n="$b.Tar"
elif b="`basename "$n" .taz`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .taZ`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TAZ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Taz`" && test "$b" != "$n" ; then
	n="$b.Tar"
elif b="`basename "$n" z`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" Z`" && test "$b" != "$n" ; then
	n="$b"
else
	n="$n.unpacked"
fi

if test -f "$n" || test -d "$n" ; then
	Error "File already exists: $n"
fi

exec gzip -d -c < "$ArchFile" > "$n"

;;
#------------------------------ unpack lzma + xz -------------------------------
*.lzma|*.LZMA|*.Lzma|\
*.xz|*.XZ|*.Xz)

n="`basename "$ArchFile"`"

if   b="`basename "$n" .lzma`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .LZMA`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Lzma`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .xz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .XZ`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Xz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .tlz`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TLZ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tlz`" && test "$b" != "$n" ; then
	n="$b.Tar"
elif b="`basename "$n" .txz`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TXZ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Txz`" && test "$b" != "$n" ; then
	n="$b.Tar"
else
	n="$n.unpacked"
fi

if test -f "$n" || test -d "$n" ; then
	Error "File already exists: $n"
fi

exec xz --decompress --stdout < "$ArchFile" > "$n"

;;
#--------------------------------- unpack lzo ----------------------------------
*.lzo|*.LZO|*.Lzo)

n="`basename "$ArchFile"`"

if   b="`basename "$n" .lzo`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .LZO`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Lzo`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .tzo`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TZO`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tzo`" && test "$b" != "$n" ; then
	n="$b.Tar"
else
	n="$n.unpacked"
fi

if test -f "$n" || test -d "$n" ; then
	Error "File already exists: $n"
fi

exec lzop -d -c < "$ArchFile" > "$n"

;;
#-------------------------------------------------------------------------------
*)
	Error "Unpacking of $Format not supported"
;;
esac
fi
