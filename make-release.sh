outdir=`pwd`/out
builddir=`pwd`/build

sconsopts="-Q --prefix=$outdir --no-debug-symbols --optimization=full"

# Check for any weird linkage dependencies that may have been accidentally
# included in a shared object. This might happen if the computer that the 
# release is being compiled on has some non-standard libraries installed.
checkdeps() {
	$LDD $1 | awk '
BEGIN {
	file="'$1'"
	n = split( \
		"ElementXML SoarKernelSML TestExternalLibrary " \
		"stdc++ " \
		"System " \
		"linux-vdso linux-gate dl pthread m gcc_s c ld-linux ld-linux-x86-64", x)
	for(i = 1; i <= n; i++) {
		legal[x[i]] = 1
	}
}
{
	n = split($1, p1, "/")
	split(p1[n], p2, "\\.")
	lib = p2[1]
	if (substr(lib, 1, 3) == "lib") {
		lib = substr(lib, 4)
	}
	if (legal[lib] != 1) {
		printf ("Illegal dependency in %s: %s\n", file, lib)
		exit 1
	}
}'
}

case `uname` in
Linux)
	os=linux
	LDD=ldd
	LIB_PATH_VAR=LD_LIBRARY_PATH
	SOEXT=so
	;;
Darwin)
	os=osx
	LDD='otool -L'
	LIB_PATH_VAR=DYLD_LIBRARY_PATH
	SOEXT=dylib
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

if ! (cd Core; scons --clean; scons $sconsopts)
then
	echo build failed
	exit 1
fi

mv $outdir $reldir

eval "export $LIB_PATH_VAR='$reldir/lib'"
for so in "$reldir/lib/*.$SOEXT"
do
	if ! checkdeps $so
	then
		exit 1
	fi
done

if ! "$reldir"/bin/Tests
then
	echo tests failed
	exit 1
fi

cp Release/*.txt "$reldir"
cp Release/$os/* "$reldir"
rm -f "$reldir"/share/java/*.src.jar
rm -f "$reldir"/share/soar/Documentation/*

find "$reldir" -type f -print0 | xargs -0 dos2unix -q
if [ ! $? ]
then
	echo 'Line ending conversion failed'
	exit 1
fi

echo release is in $reldir
echo don\'t forget to copy the manual pdfs into $reldir/share/soar/Documentation
