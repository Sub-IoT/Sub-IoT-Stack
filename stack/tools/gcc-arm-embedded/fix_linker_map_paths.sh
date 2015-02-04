#!/usr/bin/env bash
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
else
    ((cut_no=$cut_no-1))
fi

cp "$SOURCE" "$DEST"
head -n $cut_no "$SOURCE" | tail -n +2 | sed 's/^[     ]*//' | sed 's/[    ]*$//' | sed 's/\([^(]*\)(.*/\1/' | grep -v '^$' | sort | uniq | while read path
do
    abs_path="`cd $(dirname "$path" ) && pwd`/`basename $path`"
    esc_path=$(echo $path | sed 's/\//\\\//g' | sed 's/\./\\./g')
    esc_abs_path=$(echo $abs_path | sed 's/\//\\\//g')
    if [ "`uname`" == "Darwin" ]
    then
	sed -i '' "s/$esc_path/$esc_abs_path/" "$DEST"
    else
	sed -i "s/$esc_path/$esc_abs_path/" "$DEST"
    fi
done
