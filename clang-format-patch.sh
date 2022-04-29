#!/bin/bash

find . -type d \( -path ./stack/cmake -o -path ./stack/framework/hal \) -prune -o -iname *.h -o -iname *.c | xargs clang-format-10 -style=file -i -fallback-style=none

git diff > clang_format.patch

# Delete if 0 size
if [ ! -s clang_format.patch ]
then
    rm clang_format.patch
fi

exit 0
