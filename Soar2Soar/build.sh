#/bin/bash

SOAR_PATH=$DYLD_LIBRARY_PATH/../..

g++ -L$SOAR_PATH/SoarLibrary/lib/ -B $SOAR_PATH/Core/ClientSML -B $SOAR_PATH/Core/ElementXML -B $SOAR_PATH/Core/ConnectionSML -o soar2soar soar2soar.cpp -lClientSML -lConnectionSML -lElementXML