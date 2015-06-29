#! /bin/bash

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
TestsFile="$DIR/../Prototype-UnitTesting-XCTest/Tests.mm"

rm -rf "$DIR/../Prototype-UnitTesting-XCTest/kernel"

echo "" > "$TestsFile"

find kernel -mindepth 1 -type f -name '*.hpp' | while read -r FILE; do
	FileRelative=`dirname $FILE`
	NewFile=${FILE%.hpp}.mm
	FullFilePath="$DIR/../Prototype-UnitTesting-XCTest/$NewFile"

	CategoryName=`grep -o 'CATEGORY([a-zA-Z\-\_][a-zA-Z\-\_0-9]\+)' $FILE`
	CategoryName=`echo $CategoryName | cut -d '(' -f2 | cut -d ')' -f1`

	if [ ! -z $CategoryName ]; then
		mkdir -p "$DIR/../Prototype-UnitTesting-XCTest/$FileRelative"
		echo "#import <XCTest/XCTest.h>

#include \"TestHelpers.hpp\"
#include \"${CategoryName}.hpp\"

#import \"XCTestDefines.h\"

TEST_SETUP($CategoryName)
" > "$FullFilePath"

		Tests=`grep -v '^//' $FILE | grep -o 'TEST([a-zA-Z\-\_][a-zA-Z\-\_0-9]*,'`
		IFS=$'\n' read -rd '' -a TestsArray <<<"$Tests"

		for test in "${TestsArray[@]}"
		do
			ProcessedTest=`echo $test | cut -d '(' -f2 | cut -d ',' -f1`
			echo "XC_TEST($ProcessedTest)"  >> "$FullFilePath"
		done

		echo "" >> "$FullFilePath"
		echo "@end" >> "$FullFilePath"

		echo "#import \"$NewFile\"" >> "$TestsFile"
	fi
done
