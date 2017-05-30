lVersion="9.6"
lTestSuite="full"
lUnitTests=off

while getopts uv:s: opt
do
    case "$opt" in
      u)  lUnitTests=on;;
      v)  lVersion="$OPTARG";;
      s)  lTestSuite="$OPTARG";;
      \?)		# unknown flag
      	  echo >&2 \
	  "usage: $0 [-v [9.4 | 9.6]] [-s [full | fast]] "
	  exit 1;;
    esac
done
shift `expr $OPTIND - 1`

echo "================================================================================="
echo "Running $lTestSuite performance tests using Soar $lVersion agents..."
echo "================================================================================="

if [ $lTestSuite == "full" ] ; then
  if [ $lVersion == "9.6" ] ; then
    nice -n -10 ./PerformanceTests wait 3 1000000
    nice -n -10 ./PerformanceTests wait_learning 1 1000000 2
    nice -n -10 ./PerformanceTests arithmetic96 9
    nice -n -10 ./PerformanceTests arithmetic96_learning 1 0 9
    nice -n -10 ./PerformanceTests Teach_Soar_90_Games 2 10000
    nice -n -10 ./PerformanceTests FactorizationStressTest 2
    nice -n -10 ./PerformanceTests FactorizationStressTest_learning 2
    nice -n -10 ./PerformanceTests fifteen96 3 5000
    nice -n -10 ./PerformanceTests fifteen96_learning 10 500
    nice -n -10 ./PerformanceTests count-test-5000 3
    nice -n -10 ./PerformanceTests count-test-5000_learning 3
    nice -n -10 ./PerformanceTests mac-planning96 1 300 15
    nice -n -10 ./PerformanceTests mac-planning96_learning 4 165 64
    nice -n -10 ./PerformanceTests water-jug-lookahead96 15 10000
    nice -n -10 ./PerformanceTests water-jug-lookahead96_learning 2 102 100
  elif [ $lVersion == "9.4" ] ; then
    nice -n -10 ./PerformanceTests wait 3 1000000
    nice -n -10 ./PerformanceTests wait_learning 1 1000000 2
    nice -n -10 ./PerformanceTests arithmetic94 9
    nice -n -10 ./PerformanceTests arithmetic94_learning 1 0 9
    nice -n -10 ./PerformanceTests FactorizationStressTest 2
    nice -n -10 ./PerformanceTests FactorizationStressTest_learning 2
    nice -n -10 ./PerformanceTests fifteen94 3 5000
    nice -n -10 ./PerformanceTests fifteen94_learning 10 500
    nice -n -10 ./PerformanceTests count-test-5000 3
    nice -n -10 ./PerformanceTests count-test-5000_learning 3
    nice -n -10 ./PerformanceTests mac-planning94 1 300 15
    nice -n -10 ./PerformanceTests mac-planning94_learning 4 165 64
    nice -n -10 ./PerformanceTests water-jug-lookahead94 15 10000
    nice -n -10 ./PerformanceTests water-jug-lookahead94_learning 2 102 100
  fi
elif [ $lTestSuite == "fast" ] ; then
  if [ $lVersion == "9.6" ] ; then
    nice -n -10 ./PerformanceTests wait 1 1000000
    nice -n -10 ./PerformanceTests wait_learning 1 1000000
    nice -n -10 ./PerformanceTests arithmetic96 2
    nice -n -10 ./PerformanceTests arithmetic96_learning 1 0 3
    nice -n -10 ./PerformanceTests Teach_Soar_90_Games 1 10000
    nice -n -10 ./PerformanceTests FactorizationStressTest 1
    nice -n -10 ./PerformanceTests FactorizationStressTest_learning 1
    nice -n -10 ./PerformanceTests fifteen96 2 5000
    nice -n -10 ./PerformanceTests fifteen96_learning 2 500
    nice -n -10 ./PerformanceTests count-test-5000 1
    nice -n -10 ./PerformanceTests count-test-5000_learning 1
    nice -n -10 ./PerformanceTests mac-planning96 1 300 3
    nice -n -10 ./PerformanceTests mac-planning96_learning 2 165 32
    nice -n -10 ./PerformanceTests water-jug-lookahead96 3 10000
    nice -n -10 ./PerformanceTests water-jug-lookahead96_learning 2 102 100

  elif [ $lVersion == "9.4" ] ; then
    nice -n -10 ./PerformanceTests wait 1 1000000
    nice -n -10 ./PerformanceTests wait_learning 1 1000000
    nice -n -10 ./PerformanceTests arithmetic94 2
    nice -n -10 ./PerformanceTests arithmetic94_learning 1 0 3
    nice -n -10 ./PerformanceTests FactorizationStressTest 1
    nice -n -10 ./PerformanceTests FactorizationStressTest_learning 1
    nice -n -10 ./PerformanceTests fifteen94 2 5000
    nice -n -10 ./PerformanceTests fifteen94_learning 2 500
    nice -n -10 ./PerformanceTests count-test-5000 1
    nice -n -10 ./PerformanceTests count-test-5000_learning 1
    nice -n -10 ./PerformanceTests mac-planning94 1 300 3
    nice -n -10 ./PerformanceTests mac-planning94_learning 2 165 32
    nice -n -10 ./PerformanceTests water-jug-lookahead94 3 10000
    nice -n -10 ./PerformanceTests water-jug-lookahead94_learning 2 102 100
  fi
fi

if [ $lUnitTests != off ] ; then
  echo "Chunking Unit Tests"
  time nice -n -10 ./UnitTests -c ChunkingTests  > /dev/null
  echo "\nFunctional Tests"
  time nice -n -10 ./UnitTests -c FunctionalTests  > /dev/null
fi 

