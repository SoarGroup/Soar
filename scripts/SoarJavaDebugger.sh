#!/usr/bin/env bash

set -o errexit
set -o nounset
set -o pipefail
if [[ "${TRACE-0}" == "1" ]]; then
    set -o xtrace
fi

THISDIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
export SOAR_HOME="$THISDIR"
export DYLD_LIBRARY_PATH="$SOAR_HOME"

FLAG=""
# SWT requirement: display must be created on main thread due to Cocoa restrictions
if [[ $(uname) == 'Darwin' ]]; then
  FLAG="-XstartOnFirstThread"
fi

java $FLAG -Djava.library.path="$SOAR_HOME" -jar "$SOAR_HOME/SoarJavaDebugger.jar" "$@" &
