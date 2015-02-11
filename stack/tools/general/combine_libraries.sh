#!/usr/bin/env bash

ROOT_DIR=${1:?USAGE <root_dir> '<library>;<library>;...' <destination>}
LIBRARIES=${2:?USAGE <root_dir> '<library>;<library>;...' <destination>}
DESTINATION=${3:?USAGE <root_dir> '<library>;<library>;...' <destination>}

LIBRARY_FILES=""

for libname in `echo $LIBRARIES | sed 's/;/ /g'`
do
    #for now assume only a single file wil be found
    libfile=`find "$ROOT_DIR" -iname "lib${libname}.a"`
    if [ -z "${libfile}" ]
    then
	echo "Could not find library file for library $libname" 1>&2
	exit 1
    fi
    LIBRARY_FILES="$LIBRARY_FILES $libfile"
done

if [ "`uname`" == "Darwin" ]
then
    #we're running on OS X, libtool makes this easy
    libtool -static -o "${DESTINATION}" ${LIBRARY_FILES}
else
    #General linux/unix. This is more complicated
    
    tmpdir="/tmp/__cl_${RANDOM}"
    while [ -d "$tmpdir" ]
    do
	tmpdir="/tmp/__cl_${RANDOM}"
    done
    
    mkdir "$tmpdir"
    
    for libfile in $LIBRARY_FILES
    do
	libname=`basename $libfile .a`
	mkdir $tmpdir/$libname
	abspath="`cd $(dirname "$libfile" ) && pwd`/`basename $libfile`"
	bash -c "cd $tmpdir/$libname;ar -xo $abspath"	    
    done
    ar -ru ${DESTINATION} `find $tmpdir -type f`
    rm -rf "$tmpdir"
fi
    exit 0

