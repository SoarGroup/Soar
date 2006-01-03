#!/bin/sh
# Author: Jonathan Voigt
# Date: 2005
# Simply calls ifnames and directs output to a file
ifnames src/*.cpp src/*.h include/*.h lib_src/pcre-4.5/*.c lib_src/pcre-4.5/*.h > ifnames.txt
