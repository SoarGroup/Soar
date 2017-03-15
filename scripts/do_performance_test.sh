./PerformanceTests arithmetic96 3 0 3
./PerformanceTests arithmetic96_learning 3 0 3
./PerformanceTests count-test-5000 2 0 2
./PerformanceTests count-test-5000_learning 2 0 2
./PerformanceTests Demo_Water_Jug_Look_Ahead96 3 100000 6
./PerformanceTests Demo_Water_Jug_Look_Ahead96_learning 2 102 200
./PerformanceTests fifteen96 2 2000 5
./PerformanceTests fifteen96_learning 2 500 5
./PerformanceTests wait 1 1000000 3
./PerformanceTests wait_learning 1 1000000 3
echo "Chunking Unit Tests"
time ./UnitTests -c ChunkingTests  > /dev/null
echo "\nFunctional Tests"
time ./UnitTests -c FunctionalTests  > /dev/null