#!/bin/sh

#===============================================================================
#================================= Description =================================
#===============================================================================

Description="\
emArch.sh

Copyright (C) 2007-2008,2010-2011,2019-2021 Oliver Hamann.

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
.bz             (unpack only) | bzip2
.bz2                          | bzip2
.gz                           | gzip
.lzma                         | xz
.lzo                          | lzop
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
Trust_ar=no
Trust_tar=no
Trust_unzip=no

# Whether to use pigz or 7z instead of gzip for gz archives.
# (At most one of these may be set to yes.)
Use_pigz=no
Use_7z_for_gz=no

# Whether to use pbzip2, lbzip2 or 7z instead of bzip2 for bz2 archives.
# (At most one of these may be set to yes.)
Use_pbzip2=no
Use_lbzip2=no
Use_7z_for_bz2=no

# Whether to use pixz, pxz or 7z instead of xz for xz archives.
# (At most one of these may be set to yes.)
Use_pixz=no
Use_pxz=no
Use_7z_for_xz=no

# Whether to use 7z instead of zip/unzip for zip archives.
Use_7z_for_zip=no


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
	-h|--help)
		echo "$Description"
		exit 0
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

SetErrorHintIfNotBrokenPipe()
{
	if test "$?" != "141" ; then
		SetErrorHint
	fi
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

exec 7z a -- "$ArchFile" "$@"

;;
#----------------------------------- pack ar -----------------------------------
*.ar|*.AR|*.Ar|\
*.a|*.A|\
*.deb|*.DEB|*.Deb\
*.ipk|*.IPK|*.Ipk)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec ar rc -- "$ArchFile" "$@"

;;
#---------------------------------- pack tar -----------------------------------
*.tar|*.TAR|*.Tar)

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec tar cvf "$ArchFile" -- "$@"

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

{
	tar cvf - -- "$@" || SetErrorHint
} | {
	if test $Use_pbzip2 = yes ; then
		pbzip2 -c
	elif test $Use_lbzip2 = yes ; then
		lbzip2 -c
	elif test $Use_7z_for_bz2 = yes ; then
		7z a -si -so .bz2
	else
		bzip2 -c
	fi
} > "$ArchFile"
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

{
	tar cvf - -- "$@" || SetErrorHint
} | {
	if test $Use_pigz = yes ; then
		pigz -c
	elif test $Use_7z_for_gz = yes ; then
		7z a -si -so .gz
	else
		gzip -c
	fi
} > "$ArchFile"
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

{ tar cvf - -- "$@" || SetErrorHint ; } | xz --stdout --format=lzma > "$ArchFile"
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

{ tar cvf - -- "$@" || SetErrorHint ; } | lzop -c > "$ArchFile"
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

{
	tar cvf - -- "$@" || SetErrorHint
} | {
	if test $Use_pixz = yes ; then
		pixz
	elif test $Use_pxz = yes ; then
		pxz --stdout
	elif test $Use_7z_for_xz = yes ; then
		7z a -si -so .xz
	else
		xz --stdout
	fi
} > "$ArchFile"
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

if test $Use_7z_for_zip = yes ; then
	exec 7z a -tzip -- "$ArchFile" "$@"
else
	exec zip -r -9 "$ArchFile" -- "$@"
fi

;;
#---------------------------------- pack bz2 -----------------------------------
*.bz2|*.BZ2|*.Bz2)

if test $# != 1 ; then
	Error "Cannot pack multiple files into bz2 archive."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory into bz2 archive."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

if test $Use_pbzip2 = yes ; then
	exec pbzip2 -c -- "$1" > "$ArchFile"
elif test $Use_lbzip2 = yes ; then
	exec lbzip2 -c -- "$1" > "$ArchFile"
elif test $Use_7z_for_bz2 = yes ; then
	exec 7z a -so -- .bz2 "$1" > "$ArchFile"
else
	exec bzip2 -c -- "$1" > "$ArchFile"
fi

;;
#----------------------------------- pack gz -----------------------------------
*.gz|*.GZ|*.Gz)

if test $# != 1 ; then
	Error "Cannot pack multiple files into gz archive."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory into gz archive."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

if test $Use_pigz = yes ; then
	exec pigz -c -- "$1" > "$ArchFile"
elif test $Use_7z_for_gz = yes ; then
	exec 7z a -so -- .gz "$1" > "$ArchFile"
else
	exec gzip -c -- "$1" > "$ArchFile"
fi

;;
#---------------------------------- pack lzma ----------------------------------
*.lzma|*.LZMA|*.Lzma)

if test $# != 1 ; then
	Error "Cannot pack multiple files into lzma archive."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory into lzma archive."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec xz --stdout --format=lzma -- "$1" > "$ArchFile"

;;
#---------------------------------- pack lzo -----------------------------------
*.lzo|*.LZO|*.Lzo)

if test $# != 1 ; then
	Error "Cannot pack multiple files into lzo archive."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory into lzo archive."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

exec lzop -c -- "$1" > "$ArchFile"

;;
#----------------------------------- pack xz -----------------------------------
*.xz|*.XZ|*.Xz)

if test $# != 1 ; then
	Error "Cannot pack multiple files into xz archive."
fi
if test -d "$1" ; then
	Error "Cannot pack a directory into xz archive."
fi

if test -f "$ArchFile" ; then
	rm -f "$ArchFile" || exit 1
fi

if test $Use_pixz = yes ; then
	exec pixz -t < "$1" > "$ArchFile"
elif test $Use_pxz = yes ; then
	exec pxz --stdout -- "$1" > "$ArchFile"
elif test $Use_7z_for_xz = yes ; then
	exec 7z a -so -- .xz "$1" > "$ArchFile"
else
	exec xz --stdout -- "$1" > "$ArchFile"
fi

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
		{
			if test $Use_pbzip2 = yes ; then
				pbzip2 -d -c
			elif test $Use_lbzip2 = yes ; then
				lbzip2 -d -c
			elif test $Use_7z_for_bz2 = yes ; then
				7z x -tbzip2 -si -so
			else
				bzip2 -d -c
			fi
		} < "$ArchFile" || SetErrorHintIfNotBrokenPipe
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

{
	{
		if test $Use_pbzip2 = yes ; then
			pbzip2 -d -c
		elif test $Use_lbzip2 = yes ; then
			lbzip2 -d -c
		elif test $Use_7z_for_bz2 = yes ; then
			7z x -tbzip2 -si -so
		else
			bzip2 -d -c
		fi
	} < "$ArchFile" || SetErrorHintIfNotBrokenPipe
} | tar xvf -
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
		{
			if test $Use_pigz = yes ; then
				pigz -d -c
			elif test $Use_7z_for_gz = yes ; then
				7z x -tgzip -si -so
			else
				gzip -d -c
			fi
		} < "$ArchFile" || SetErrorHintIfNotBrokenPipe
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

{
	{
		if test $Use_pigz = yes ; then
			pigz -d -c
		elif test $Use_7z_for_gz = yes ; then
			7z x -tgzip -si -so
		else
			gzip -d -c
		fi
	} < "$ArchFile" || SetErrorHintIfNotBrokenPipe
} | tar xvf -
CheckErrorHintAnd $?

;;
#------------------------------- unpack tar.lzma -------------------------------
*.tar.lzma|*.TAR.lzma|*.Tar.lzma|\
*.tar.LZMA|*.TAR.LZMA|*.Tar.LZMA|\
*.tar.Lzma|*.TAR.Lzma|*.Tar.Lzma|\
*.tlz|*.TLZ|*.Tlz)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		xz --decompress --stdout < "$ArchFile" || SetErrorHintIfNotBrokenPipe
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

{
	xz --decompress --stdout < "$ArchFile" || SetErrorHintIfNotBrokenPipe
} | tar xvf -
CheckErrorHintAnd $?

;;
#-------------------------------- unpack tar.xz --------------------------------
*.tar.xz|*.TAR.xz|*.Tar.xz|\
*.tar.XZ|*.TAR.XZ|*.Tar.XZ|\
*.tar.Xz|*.TAR.Xz|*.Tar.Xz|\
*.txz|*.TXZ|*.Txz)

if test $TrustByOption != yes && test $Trust_tar != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		{
			if test $Use_pixz = yes ; then
				pixz -d
			elif test $Use_7z_for_xz = yes ; then
				7z x -txz -si -so
			else
				xz --decompress --stdout
			fi
		} < "$ArchFile" || SetErrorHintIfNotBrokenPipe
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

{
	{
		if test $Use_pixz = yes ; then
			pixz -d
		elif test $Use_7z_for_xz = yes ; then
			7z x -txz -si -so
		else
			xz --decompress --stdout
		fi
	} < "$ArchFile" || SetErrorHintIfNotBrokenPipe
} | tar xvf -
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
		lzop -d -c < "$ArchFile" || SetErrorHintIfNotBrokenPipe
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

{
	lzop -d -c < "$ArchFile" || SetErrorHintIfNotBrokenPipe
} | tar xvf -
CheckErrorHintAnd $?

;;
#--------------------------------- unpack zip ----------------------------------
*.zip|*.ZIP|*.Zip|\
*.jar|*.JAR|*.Jar)

if test $TrustByOption != yes && test $Trust_unzip != yes ; then
	echo "Scanning archive listing for dangerous paths..."
	{
		if test $Use_7z_for_zip = yes ; then
			7z l -tzip -- "$ArchFile" || SetErrorHint
		else
			unzip -l "$ArchFile" || SetErrorHint
		fi
		# for testing the test:
		# echo "/bla"
		# echo "../bla"
		# echo "1234 /bla"
		# echo "1234 ../bla"
		# echo "1234 bla/../bla"
	} | {
		if test $Use_7z_for_zip = yes ; then
			egrep -v "^(Listing archive:|Path =)( |[[:space:]])*`echo "$ArchFile" | tr '\\\\^?*+|\!\$()[]{}' '.'`\$"
		else
			egrep -v "^Archive:( |[[:space:]])*`echo "$ArchFile" | tr '\\\\^?*+|\!\$()[]{}' '.'`\$"
		fi
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

if test $Use_7z_for_zip = yes ; then
	exec 7z x -tzip -- "$ArchFile"
else
	exec unzip "$ArchFile"
fi

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

if test $Use_pbzip2 = yes ; then
	exec pbzip2 -d -c < "$ArchFile" > "$n"
elif test $Use_lbzip2 = yes ; then
	exec lbzip2 -d -c < "$ArchFile" > "$n"
elif test $Use_7z_for_bz2 = yes ; then
	exec 7z x -tbzip2 -si -so < "$ArchFile" > "$n"
else
	exec bzip2 -d -c < "$ArchFile" > "$n"
fi

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

if test $Use_pigz = yes ; then
	exec pigz -d -c < "$ArchFile" > "$n"
elif test $Use_7z_for_gz = yes ; then
	exec 7z x -tgzip -si -so < "$ArchFile" > "$n"
else
	exec gzip -d -c < "$ArchFile" > "$n"
fi

;;
#--------------------------------- unpack lzma ---------------------------------
*.lzma|*.LZMA|*.Lzma)

n="`basename "$ArchFile"`"

if   b="`basename "$n" .lzma`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .LZMA`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Lzma`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .tlz`" && test "$b" != "$n" ; then
	n="$b.tar"
elif b="`basename "$n" .TLZ`" && test "$b" != "$n" ; then
	n="$b.TAR"
elif b="`basename "$n" .Tlz`" && test "$b" != "$n" ; then
	n="$b.Tar"
else
	n="$n.unpacked"
fi

if test -f "$n" || test -d "$n" ; then
	Error "File already exists: $n"
fi

exec xz --decompress --stdout < "$ArchFile" > "$n"

;;
#---------------------------------- unpack xz ----------------------------------
*.xz|*.XZ|*.Xz)

n="`basename "$ArchFile"`"

if   b="`basename "$n" .xz`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .XZ`" && test "$b" != "$n" ; then
	n="$b"
elif b="`basename "$n" .Xz`" && test "$b" != "$n" ; then
	n="$b"
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

if test $Use_pixz = yes ; then
	exec pixz -d < "$ArchFile" > "$n"
elif test $Use_7z_for_xz = yes ; then
	exec 7z x -txz -si -so < "$ArchFile" > "$n"
else
	exec xz --decompress --stdout < "$ArchFile" > "$n"
fi

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
