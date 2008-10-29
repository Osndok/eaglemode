#!/bin/sh
#-------------------------------------------------------------------------------
# eaglemode.sh
#
# Copyright (C) 2006-2008 Oliver Hamann.
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

# Set EM_DIR to the directory where this script is in.
OLD_DIR="`pwd`"
cd "`dirname "$0"`"
n="`basename "$0"`"
while test -h "$n" ; do
	n="`readlink "$n"`"
	cd "`dirname "$n"`"
	n="`basename "$n"`"
done
EM_DIR="`pwd`"
cd "$OLD_DIR"
export EM_DIR

# Have $EM_DIR/lib as the first in LD_LIBRARY_PATH.
OLD_IFS="$IFS"
IFS=":"
NEW_LD_LIBRARY_PATH="$EM_DIR/lib"
for i in $LD_LIBRARY_PATH ; do
	if test "$i" != "$EM_DIR/lib" ; then
		NEW_LD_LIBRARY_PATH="$NEW_LD_LIBRARY_PATH:$i"
	fi
done
IFS="$OLD_IFS"
LD_LIBRARY_PATH="$NEW_LD_LIBRARY_PATH"
export LD_LIBRARY_PATH

# Special case for cygwin.
if test "$OSTYPE" = "cygwin" ; then
	OLD_IFS="$IFS"
	IFS=":"
	NEW_PATH="$EM_DIR/lib"
	for i in $PATH ; do
		if test "$i" != "$EM_DIR/lib" ; then
			NEW_PATH="$NEW_PATH:$i"
		fi
	done
	IFS="$OLD_IFS"
	PATH="$NEW_PATH"
	export PATH
fi

# Execute the binary.
exec "$EM_DIR/bin/eaglemode" "$@"
