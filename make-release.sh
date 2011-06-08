outdir=`pwd`/out
builddir=`pwd`/build

sconsopts="-Q --prefix=$outdir --no-debug-symbols --optimization=full"

case `uname` in
Linux)
	os=linux
	;;
Darwin)
	os=osx
	;;
WindowsNT)
	os=win
	;;
*)
	echo unsupported
	exit 1
	;;
esac

arch=`uname -m`

reldir="`pwd`/release-$os-$arch"

rm -rf "$outdir" "$builddir" "$reldir"

sed 's|^\(#define USE_PERFORMANCE_FOR_BOTH\)|//\1|g' Core/shared/misc.h >tmp
mv -f tmp Core/shared/misc.h
sed 's|^\(#define NO_TIMING_STUFF\)|//\1|g
     s|^\(#define DETAILED_TIMING_STATS\)|//\1|g' Core/SoarKernel/src/kernel.h >tmp
mv -f tmp Core/SoarKernel/src/kernel.h

case "$os" in
win)
	cmd /C "build clean" && cmd /C build
	;;
*)
	(cd Core; scons --clean; scons $sconsopts)
	;;
esac

if [ $? -ne 0 ]
then
	echo build failed
	exit 1
fi

mv $outdir $reldir

export LD_LIBRARY_PATH="$reldir/lib"
if ! "$reldir"/bin/Tests
then
	echo tests failed
	exit 1
fi

cp Release/*.txt "$reldir"
cp Release/$os/* "$reldir"
rm "$reldir"/share/java/*.src.jar
rm "$reldir"/share/soar/Documentation/*.doc*

case "$os" in
win)
	rm "$reldir"/bin/*.pdb
	# couldn't find a well-behaved stand-alone version of xargs for windows. Also, find uses \ separators 
	# instead of / for some reason
	find "$reldir" -type f | sed 's:\\:/:g' | awk '{system("unix2dos \"" $0 "\"")}'
	;;
*)
	find "$reldir" -type f -print0 | xargs -0 dos2unix
	;;
esac

echo release is in $reldir
echo don\'t forget to copy the manual pdfs into $reldir/share/soar/Documentation
