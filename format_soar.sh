#!/bin/sh 

if [ $# -gt 1 ]; then
  echo "Error:  $# arguments given."
  echo "Syntax is: format_soar.sh [dirname | filename]"
  echo "Examples: ./format_soar.sh ."
  echo "          ./format_soar.sh my_file.cpp"
  exit 1
fi

function contains() {
    local n=$#
    local value=${!n}
    for ((i=1;i < $#;i++)) {
        if [[ "${value}" == *"${!i}"* ]]; then
            echo "y"
            return 0
        fi
    }
    echo "n"
    return 1
}

ignorearray=("./Core/SVS/ccd"
              "./Core/SVS/eigen"
              "./Core/SVS/glfw"
              "./Core/SVS/src/windows"
              "./Core/SVS/viewer"
              "./Core/pcre"
              "./Core/shared/msvc"
              "./Core/shared/pcreposix.h"
              "./Core/shared/portability_windows.h"
              "./Core/SoarKernel/sqlite"
              "./Core/SoarKernel/src/sqlite3.h"
              "./Core/ConnectionSML/src/thread_OSspecific.cpp"
              "./Core/ConnectionSML/src/thread_OSspecific.h"
              "./Core/CLI/src/cli_help.cpp")

filename=""
startdir="."
if [ -d "$1" ]; then
  echo "Dir ${1} exists."
  startdir=$1
fi
if [ -f "$1" ]; then
  filename=$1
fi

echo "Processing directory ${startdir}/${filename}"

if [ "$filename" == "" ]; then
  find $startdir -type f \( -name "*.cpp" -or -name "*.h" \) -print0 |\
  while IFS= read -r -d '' file; do
    if [ $(contains "${ignorearray[@]}" "${file}") == "y" ]; then
        echo "Skipping file $file"
    else
#       echo "Formatting file $file"
      #!/bin/bash
      astyle "$file" --options="/Users/mazzin/git/Soar/SoarSuite/format_soar.astylerc"
    fi
  done
else
  if [ $(contains "${ignorearray[@]}" "${file}") == "y" ]; then
    echo "Skipping file $file"
  else
#     echo "Formatting file $filename"
    #!/bin/bash
    astyle "$filename" --options="/Users/mazzin/git/Soar/SoarSuite/format_soar.astylerc"
  fi
fi
