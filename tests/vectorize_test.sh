ITERS=3
#SRC="vectest.cpp ../src/common.cpp"
SRC="vectest2.cpp"
OPTS="-I../src"
OPTS1="$OPTS -O3 -ffast-math -ftree-vectorizer-verbose=5"
OPTS2="$OPTS -O3"
#OPTS2="$OPTS -O3 -ftree-vectorizer-verbose=5 -march=native"

g++ $OPTS1 -o vt1 $SRC || exit 1
g++ $OPTS2 -o vt2 $SRC || exit 1

for i in `seq $ITERS`
do
	/usr/bin/time --quiet -f '%e' vt1 2>&1
	/usr/bin/time --quiet -f '%e' vt2 2>&1
done | awk 'NR % 2 == 1 { printf("v1 %f\n", $1); s1 = s1 + $1 }
            NR % 2 == 0 { printf("v2 %f\n", $1); s2 = s2 + $1 }
            END { print s1 * 2 / NR; print s2 * 2 / NR }'
