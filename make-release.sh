case `uname` in
Linux)
	os=linux
	;;
Darwin)
	os=osx
	;;
*)
	echo unsupported
	exit 1
esac

arch=`uname -m`

reldir="`pwd`/release-$os-$arch"
export LD_LIBRARY_PATH="$reldir/lib"
sconsopts="--prefix=$reldir --no-debug-symbols --optimization=full"

rm -rf "$reldir"
mkdir "$reldir"

sed 's|^\(#define USE_PERFORMANCE_FOR_BOTH\)|//\1|g' Core/shared/misc.h >tmp
mv tmp Core/shared/misc.h
sed 's|^\(#define NO_TIMING_STUFF\)|//\1|g
     s|^\(#define DETAILED_TIMING_STATS\)|//\1|g' Core/SoarKernel/src/kernel.h >tmp
mv tmp Core/SoarKernel/src/kernel.h

if !(cd Core; scons --clean; scons $sconsopts)
then
	echo build failed
	exit 1
fi

if ! "$reldir"/bin/Tests
then
	echo tests failed
	exit 1
fi

cp Release/*.txt "$reldir"
cp Release/$os/* "$reldir"
rm "$reldir"/share/java/*.src.jar
rm "$reldir"/share/soar/Documentation/*.doc*
find "$reldir" -type f -print0 | xargs -0 dos2unix

echo release is in $reldir
echo don\'t forget to copy the manual pdfs into $reldir/share/soar/Documentation
