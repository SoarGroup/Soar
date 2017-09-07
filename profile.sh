./build_lsb.sh --no-svs --cflags=-Og --lnflags=-Og all

SOARSUITE=$(pwd)
cp out/java/sml.jar ../VisualSoar/lib/
pushd ../VisualSoar
ant
cp VisualSoar.jar $SOARSUITE/out/
popd
