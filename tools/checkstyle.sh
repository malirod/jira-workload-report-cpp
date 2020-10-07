#!/bin/bash

# SPDX-License-Identifier: MIT

# Copyright (C) 2020 Malinovsky Rodion (rodionmalino@gmail.com)

TEXT_DEFAULT="\\033[0;39m"
TEXT_INFO="\\033[1;32m"
TEXT_ERROR="\\033[1;31m"
TEXT_UNDERLINE="\\0033[4m"
TEXT_BOLD="\\0033[1m"

COMMITS_RANGE=$1

if [ -n "$COMMITS_RANGE" ]; then
  echo "Processing commits range: $COMMITS_RANGE"
else
  echo ${TEXT_ERROR}"No commits provided"${TEXT_DEFAULT}
  exit 1
fi

GIT_DIFF_IGNORE_LIST=":!tools/cpplint/* :!tools/cpplint/* :!tools/conan/*"

##################################################################
### Check for odd whitespace
##################################################################

echo -e "$TEXT_INFO" "Checking odd whitespaces" "$TEXT_DEFAULT"
git diff --check $COMMITS_RANGE --color -- . $GIT_DIFF_IGNORE_LIST
if [ "$?" -ne "0" ]; then
    echo -e "$TEXT_ERROR" "Your changes introduce whitespace errors" "$TEXT_DEFAULT"
    exit 1
fi
echo -e "$TEXT_INFO" "PASSED" "$TEXT_DEFAULT"

##################################################################
### Clang-format check
##################################################################

echo -e "$TEXT_INFO" "Checking cpp style with clang-format" "$TEXT_DEFAULT"

# Auto update c++ files with clang-format. Use default clang-format.
# Use update-alternatives to set default clang.

# set path to clang-format binary
CLANG_FORMAT=$(command -v clang-format)

# remove any older patches from previous commits. Set to true or false.
DELETE_OLD_PATCHES=true

if [ ! -x "$CLANG_FORMAT" ] ; then
    echo -e "$TEXT_ERROR"
    echo -e "Error: clang-format executable not found."
    echo -e "Set the correct path in $(canonicalize_filename "$0")."
    echo -e "$TEXT_DEFAULT"
    exit 1
fi

# create a random filename to store our generated patch
prefix="pre-commit-clang-format"
suffix="$(date +%s)"
patch=$(mktemp "$prefix-$suffix-XXXX.patch" -ut)
tmpdir=$(dirname $(mktemp -u))

# clean up any older clang-format patches
$DELETE_OLD_PATCHES && rm -f $tmpdir/$prefix*.patch

CPP_FILES=$(find jwlrep -type f \( -iname "*.h" -o -iname "*.cc" -o -iname "*.cpp" -o -iname "*.c" -o -iname "*.hpp" \))

# create one patch containing all changes to the files
# git diff-index --cached --diff-filter=ACMR --name-only $against -- | while read file;
for cpp_file in $CPP_FILES
do
    # clang-format our sourcefile, create a patch with diff and append it to our $patch
    # The sed call is necessary to transform the patch from
    #    --- $file timestamp
    #    +++ - timestamp
    # to both lines working on the same file and having a a/ and b/ prefix.
    # Else it can not be applied with 'git apply'.
    "$CLANG_FORMAT" -style=file "$cpp_file" | \
        diff -u "$cpp_file" - | \
        sed -e "1s|--- |--- a/|" -e "2s|+++ -|+++ b/$cpp_file|" >> "$patch"
done

# if no patch has been generated all is ok, clean up the file stub and exit
if [ ! -s "$patch" ] ; then
    rm -f "$patch"
else
    # a patch has been created, notify the user and exit
    echo -e "$TEXT_ERROR"
    echo -e "The following differences were found between the code to commit "
    echo -e "and the clang-format rules:"
    echo -e "$TEXT_INFO"
    cat $patch
    echo -e "$TEXT_ERROR"
    echo -e "You can apply these changes with:"
    echo -e "(Linux)"
    echo -e "git apply $patch"
    echo -e "(Windows)"
    echo -e "git apply %TMP%/$(basename -- $patch)"
    echo -e "(may need to be called from the root directory of your repository)"
    echo -e "Aborting commit. Apply changes and commit again or skip checking with"
    echo -e " --no-verify (not recommended)."
    echo -e "$TEXT_DEFAULT"

    exit 2
fi

echo -e "$TEXT_INFO" "PASSED" "$TEXT_DEFAULT"

exit 0
