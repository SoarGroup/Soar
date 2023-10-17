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

echo "================================================================================="
echo "Running $lTestSuite performance tests using Soar $lVersion agents..."
echo "================================================================================="

if [ "$lTestSuite" == "full" ] ; then
    nice -n -10 ./PerformanceTests wait 3 1000000
    nice -n -10 ./PerformanceTests wait_learning 1 1000000 2
    nice -n -10 ./PerformanceTests arithmetic 9
    nice -n -10 ./PerformanceTests arithmetic_learning 1 0 9
    nice -n -10 ./PerformanceTests Teach_Soar_90_Games 2 10000
    nice -n -10 ./PerformanceTests FactorizationStressTest 2
    nice -n -10 ./PerformanceTests FactorizationStressTest_learning 2
    nice -n -10 ./PerformanceTests fifteen 3 5000
    nice -n -10 ./PerformanceTests fifteen_learning 10 500
    nice -n -10 ./PerformanceTests count-test-5000 3
    nice -n -10 ./PerformanceTests count-test-5000_learning 3
    nice -n -10 ./PerformanceTests mac-planning 1 300 15
    nice -n -10 ./PerformanceTests mac-planning_learning 4 165 64
    nice -n -10 ./PerformanceTests water-jug-lookahead 15 10000
    nice -n -10 ./PerformanceTests water-jug-lookahead_learning 2 102 100
elif [ "$lTestSuite" == "fast" ] ; then
    nice -n -10 ./PerformanceTests wait 1 1000000
    nice -n -10 ./PerformanceTests wait_learning 1 1000000
    nice -n -10 ./PerformanceTests arithmetic 2
    nice -n -10 ./PerformanceTests arithmetic_learning 1 0 3
    nice -n -10 ./PerformanceTests Teach_Soar_90_Games 1 10000
    nice -n -10 ./PerformanceTests FactorizationStressTest 1
    nice -n -10 ./PerformanceTests FactorizationStressTest_learning 1
    nice -n -10 ./PerformanceTests fifteen 2 5000
    nice -n -10 ./PerformanceTests fifteen_learning 2 500
    nice -n -10 ./PerformanceTests count-test-5000 1
    nice -n -10 ./PerformanceTests count-test-5000_learning 1
    nice -n -10 ./PerformanceTests mac-planning 1 300 3
    nice -n -10 ./PerformanceTests mac-planning_learning 2 165 32
    nice -n -10 ./PerformanceTests water-jug-lookahead 3 10000
    nice -n -10 ./PerformanceTests water-jug-lookahead_learning 2 102 100
fi

if [ $lUnitTests != off ] ; then
  echo "Chunking Unit Tests"
  time nice -n -10 ./UnitTests -c ChunkingTests  > /dev/null
  printf "\nFunctional Tests\n"
  time nice -n -10 ./UnitTests -c FunctionalTests  > /dev/null
fi

