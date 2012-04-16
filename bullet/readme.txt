This directory contains the headers and precompiled libraries for the parts
of the Bullet physics engine SVS requires. The current build is from
2.80-rev2531.

The libraries in lib were compiled running the following commands in the
bullet source directory:

cmake . -DCMAKE_CXX_FLAGS='-fPIC' -DCMAKE_C_FLAGS='-fPIC' -DBUILD_DEMOS=off -DBUILD_EXTRAS=off -DINSTALL_LIBS=on
make install

The objects in the resulting archives are pulled into libSoar.so using the
linker flag --whole-archive, hence the -fPIC flag.
