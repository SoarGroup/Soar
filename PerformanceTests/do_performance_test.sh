./PerformanceTests mac-planning96 15 300 0
./PerformanceTests mac-planning96_learning 4 165 64
./PerformanceTests water-jug-lookahead96 15 100000
./PerformanceTests water-jug-lookahead96_learning 2 102 100
./PerformanceTests arithmetic96 9
./PerformanceTests arithmetic96_learning 0 0 9
./PerformanceTests FactorizationStressTest 2
./PerformanceTests FactorizationStressTest_learning 2
./PerformanceTests fifteen96 10 2000
./PerformanceTests fifteen96_learning 0 500 10
./PerformanceTests count-test-5000 4
./PerformanceTests count-test-5000_learning 1 0 4
./PerformanceTests wait 3 1000000
./PerformanceTests wait_learning 1 1000000 2
echo "Chunking Unit Tests"
time ./UnitTests -c ChunkingTests  > /dev/null
echo "\nFunctional Tests"
time ./UnitTests -c FunctionalTests  > /dev/null