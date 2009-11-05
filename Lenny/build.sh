#/bin/bash

SOAR_PATH=$DYLD_LIBRARY_PATH/../..

gcc -c -o sqlite3.o src/sqlite3.c
g++ -Iinc -o epmem sqlite3.o src/epmem.cpp src/Symbol.cpp src/SymbolFactory.cpp src/SQLiteSymbolFactory.cpp
