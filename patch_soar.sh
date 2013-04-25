#!/bin/sh

if [ ! -d ../SoarKernel -o ! -d ../../Core ]
then
	echo 'put this directory in SoarSuite/Core first' >&2
	exit 1
fi

for p in soar_patches/*
do
	if patch -p0 -d ../.. --dry-run <$p >/dev/null
	then
		echo -n "Patch $p will succeed. Proceed? "
		read resp
		case $resp in
		y*|Y*)
			patch -p0 -d ../.. <$p
			echo done
			exit 0
			;;
		*)
			echo nothing was changed
			exit 0
			;;
		esac
		exit 0
	else
		echo "Patch $p failed"
	fi
done
echo no patches applied cleanly, nothing was changed
