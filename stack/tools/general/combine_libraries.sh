#!/usr/bin/env bash
##
## Copyright (c) 2015-2021 University of Antwerp, Aloxy NV.
##
## This file is part of Sub-IoT.
## See https://github.com/Sub-IoT/Sub-IoT-Stack for further info.
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
##

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