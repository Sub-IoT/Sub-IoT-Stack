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
SOURCE=${1:?Usage $0 <SRC_FILE> <DST_FILE>}
DEST=${2:?Usage $0 <SRC_FILE> <DST_FILE>}

if [ ! -e "$SOURCE" ]
then
    echo "Missing source file '$SOURCE'" 1>&2
    exit 1
fi

cut_no=`cat "$SOURCE" | nl -b a | grep "Allocating common symbols" | awk '{print $1}'`
if [ -z "$cut_no" ]
then
    echo "Malformatted symbol map '$SOURCE'" 1>&2
    exit 1
fi

cp "$SOURCE" "$DEST"
head -n $cut_no "$SOURCE" | grep -v "Allocating common symbols" | tail -n +2 | sed 's/^[     ]*//' | sed 's/[    ]*$//' | sed 's/\([^(]*\)(.*/\1/' | grep -v '^$' | sort | uniq | while read path
do
    abs_path="`cd $(dirname "$path" ) && pwd`/`basename $path`"
    esc_path=$(echo $path | sed 's/\//\\\//g' | sed 's/\./\\./g')
    esc_abs_path=$(echo $abs_path | sed 's/\//\\\//g')
    if [ "`uname`" = "Darwin" ]
    then
	sed -i '' "s/$esc_path/$esc_abs_path/" "$DEST"
    else
	sed -i "s/$esc_path/$esc_abs_path/" "$DEST"
    fi
done