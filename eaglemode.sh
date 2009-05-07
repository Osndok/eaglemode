#!/bin/sh
#-------------------------------------------------------------------------------
# eaglemode.sh
#
# Copyright (C) 2006-2009 Oliver Hamann.
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

# Helper function for extending a search path.
extendPath ()
{
	NEW_PATH="$2"
	OLD_IFS="$IFS"
	IFS=":"
	for i in $1 ; do
		if test "$i" != "$2" ; then
			NEW_PATH="$NEW_PATH:$i"
		fi
	done
	IFS="$OLD_IFS"
	echo "$NEW_PATH"
}

# Have $EM_DIR/lib as the first in the corresponding search path.
case "$OSTYPE" in
	cygwin*)
		PATH="`extendPath "$PATH" "$EM_DIR/lib"`"
		export PATH
	;;
	darwin*)
		DYLD_LIBRARY_PATH="`extendPath "$DYLD_LIBRARY_PATH" "$EM_DIR/lib"`"
		export DYLD_LIBRARY_PATH
	;;
	*)
		LD_LIBRARY_PATH="`extendPath "$LD_LIBRARY_PATH" "$EM_DIR/lib"`"
		export LD_LIBRARY_PATH
	;;
esac

# Execute the binary.
exec "$EM_DIR/bin/eaglemode" "$@"
