#!/bin/bash

if [ "$CORES" == "" ]; then
  CORES=1
fi

AGGREGATE=5
EPISODES=3000

# declare -a VALS=(8 0.5 16 2 \
#                 \
#                  8 0.5 16 4 \
#                  8 0.5 32 2 \
#                  8 1 16 2 \
#                  16 0.5 16 2 \
#                 \
#                  8 0.5 32 4
#                  8 1 16 4
#                  16 0.5 16 4
#                  8 1 32 2
#                  16 0.5 32 2
#                  16 1 16 2
#                 \
#                  8 1 32 4 \
#                  16 0.5 32 4 \
#                  16 1 16 4 \
#                  16 1 32 2 \
#                 \
#                  16 1 32 4 \
#                 )
declare -a VALS=(8 0.5 16 4 \
                 8 0.5 16 4 \
                 8 0.5 16 4 \
                 8 0.5 16 4 \
                 8 0.5 16 4 \
                 8 0.5 16 4 \
                 8 0.5 16 4 \
                 8 0.5 16 4)
declare -a VALSP=(  -1 16 0.5 16 4 \
                     0 16 0.5 16 4 \
                   500 16 0.5 16 4 \
                  1000 16 0.5 16 4 \
                  1500 16 0.5 16 4 \
                  2000 16 0.5 16 4 \
                  2500 16 0.5 16 4 \
                  3000 16 0.5 16 4)

for i in $(seq 1 $AGGREGATE); do
  random[$i]=$RANDOM
done

random=(2698 5643 18028 29364 30519)

for v in $(seq 0 $((${#VALS[@]} / 4 - 1))); do
  BASE="experiment"
  START="${VALS[$((4 * v + 0))]}-${VALS[$((4 * v + 1))]}-${VALS[$((4 * v + 2))]}-${VALS[$((4 * v + 3))]}"
  SP="${VALSP[$((5 * v + 0))]}_${VALSP[$((5 * v + 1))]}-${VALSP[$((5 * v + 2))]}-${VALSP[$((5 * v + 3))]}-${VALSP[$((5 * v + 4))]}"
  DIR="$BASE/${START}_$SP"
  mkdir -p $DIR
  rm -i $DIR/*
  DIRS[v]=$DIR
done

experiment () {
  echo time out/CartPole --episodes $EPISODES --seed $1 --rules $2/in.soar --rl-rules-out $2/out-$1.soar \> $2/cartpole-$1.out
       time out/CartPole --episodes $EPISODES --seed $1 --rules $2/in.soar --rl-rules-out $2/out-$1.soar > $2/cartpole-$1.out
  echo ""

  #cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789] "
  #cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789][0123456789] "
}

experiment_sp () {
  echo time out/CartPole --episodes $EPISODES --seed $1 --rules $2/in.soar --rl-rules-out $2/out-$1.soar --sp-special ${VALSP[$((5 * $3 + 0))]} ${VALSP[$((5 * $3 + 1))]} ${VALSP[$((5 * $3 + 2))]} ${VALSP[$((5 * $3 + 3))]} ${VALSP[$((5 * $3 + 4))]} \> $2/cartpole-$1.out
       time out/CartPole --episodes $EPISODES --seed $1 --rules $2/in.soar --rl-rules-out $2/out-$1.soar --sp-special ${VALSP[$((5 * $3 + 0))]} ${VALSP[$((5 * $3 + 1))]} ${VALSP[$((5 * $3 + 2))]} ${VALSP[$((5 * $3 + 3))]} ${VALSP[$((5 * $3 + 4))]}  > $2/cartpole-$1.out
  echo ""

  #cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789] "
  #cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789][0123456789] "
}

experiments () {
  for v in $(seq $1 $2 $((${#VALS[@]} / 4 - 1))); do
    DIR=${DIRS[v]}

#     echo $DIR
#     continue

    cp ./CartPole/cartpole-random-SML-var.soar $DIR/in.soar
    echo "sp {apply*initialize*cartpole
              (state <s> ^operator.name cartpole)
          -->
              (<s> ^name cartpole
                  ^div <d>)
              (<d> ^name default
                  ^x (/ 10 ${VALS[$(($2 * v + 0))]})
                  ^x-dot (/ 1 ${VALS[$(($2 * 4 * v + 1))]})
                  ^theta (/ 3.1415926 ${VALS[$(($2 * 4 * v + 2))]})
                  ^theta-dot (/ 3.141526 ${VALS[$(($2 * 4 * v + 3))]}))
          }" >> $DIR/in.soar

#     for r in ${random[@]}; do
#       experiment $r $DIR &
#     done
#     wait

    for r in ${random[@]}; do
      experiment_sp $r $DIR $v &
    done
    wait 

    ./cartpole.py $DIR/*.out
  done
}

for x in $(seq 0 $(($CORES - 1))); do
#   echo experiments $((4 * x)) $((4 * $CORES))
  experiments $x $CORES #&
done
wait
