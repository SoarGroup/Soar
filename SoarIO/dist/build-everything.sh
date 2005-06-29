#!/bin/sh
make && make install && cd SoarIO/examples/TestJavaSML && echo "TestJavaSML..." && ./buildJava.sh && cd ../../../SoarJavaDebugger && echo "SoarJavaDebugger..." && ./buildDebugger.sh && cd ../JavaMissionaries && echo "JavaMissionaries..." && ./buildmac.sh && cd ../JavaTOH && echo "JavaTOH..." && ./buildtoh.sh && cd ..
