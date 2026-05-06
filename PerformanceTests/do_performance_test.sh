#!/usr/bin/env bash

set -o errexit
set -o nounset
set -o pipefail
if [[ "${TRACE-0}" == "1" ]]; then
    set -o xtrace
fi

usage="Usage: $0 [-s [full | fast]] "

if [[ "${1-}" =~ ^-*h(elp)?$ ]]; then
    echo "$usage

Score Soar's performance on a variety of tasks.

"
    exit
fi

lVersion="9.6"
lTestSuite="full"
lUnitTests=off

while getopts u:s: opt
do
    case "$opt" in
      u)  lUnitTests=on;;
      s)  lTestSuite="$OPTARG";;
      \?)		# unknown flag
      	  echo >&2 "$usage"
	  exit 1;;
    esac
done
shift "$((OPTIND - 1))"

# Allow caller (ctest) to point at the binaries elsewhere; default to CWD.
PERF_BIN="${PERF_BIN:-./PerformanceTests}"
UNIT_BIN="${UNIT_BIN:-./UnitTests}"

# Negative niceness requires CAP_SYS_NICE (root). If not available — e.g. on
# CI runners — fall back to no renicing rather than aborting the whole test.
if nice -n -10 true >/dev/null 2>&1; then
    NICE_CMD="nice -n -10"
else
    NICE_CMD=""
fi

echo "================================================================================="
echo "Running $lTestSuite performance tests using Soar $lVersion agents..."
echo "================================================================================="

if [ "$lTestSuite" == "full" ] ; then
    $NICE_CMD "$PERF_BIN" wait 3 1000000
    $NICE_CMD "$PERF_BIN" wait_learning 1 1000000 2
    $NICE_CMD "$PERF_BIN" arithmetic 9
    $NICE_CMD "$PERF_BIN" arithmetic_learning 1 0 9
    $NICE_CMD "$PERF_BIN" Teach_Soar_90_Games 2 10000
    $NICE_CMD "$PERF_BIN" FactorizationStressTest 2
    $NICE_CMD "$PERF_BIN" FactorizationStressTest_learning 2
    $NICE_CMD "$PERF_BIN" fifteen 3 5000
    $NICE_CMD "$PERF_BIN" fifteen_learning 10 500
    $NICE_CMD "$PERF_BIN" count-test-5000 3
    $NICE_CMD "$PERF_BIN" count-test-5000_learning 3
    $NICE_CMD "$PERF_BIN" mac-planning 1 300 15
    $NICE_CMD "$PERF_BIN" mac-planning_learning 4 165 64
    $NICE_CMD "$PERF_BIN" water-jug-lookahead 15 10000
    $NICE_CMD "$PERF_BIN" water-jug-lookahead_learning 2 102 100
elif [ "$lTestSuite" == "fast" ] ; then
    $NICE_CMD "$PERF_BIN" wait 1 1000000
    $NICE_CMD "$PERF_BIN" wait_learning 1 1000000
    $NICE_CMD "$PERF_BIN" arithmetic 2
    $NICE_CMD "$PERF_BIN" arithmetic_learning 1 0 3
    $NICE_CMD "$PERF_BIN" Teach_Soar_90_Games 1 10000
    $NICE_CMD "$PERF_BIN" FactorizationStressTest 1
    $NICE_CMD "$PERF_BIN" FactorizationStressTest_learning 1
    $NICE_CMD "$PERF_BIN" fifteen 2 5000
    $NICE_CMD "$PERF_BIN" fifteen_learning 2 500
    $NICE_CMD "$PERF_BIN" count-test-5000 1
    $NICE_CMD "$PERF_BIN" count-test-5000_learning 1
    $NICE_CMD "$PERF_BIN" mac-planning 1 300 3
    $NICE_CMD "$PERF_BIN" mac-planning_learning 2 165 32
    $NICE_CMD "$PERF_BIN" water-jug-lookahead 3 10000
    $NICE_CMD "$PERF_BIN" water-jug-lookahead_learning 2 102 100
fi

if [ $lUnitTests != off ] ; then
  echo "Chunking Unit Tests"
  time $NICE_CMD "$UNIT_BIN" -c ChunkingTests  > /dev/null
  printf "\nFunctional Tests\n"
  time $NICE_CMD "$UNIT_BIN" -c FunctionalTests  > /dev/null
fi

