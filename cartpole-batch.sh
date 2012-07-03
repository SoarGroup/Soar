#!/bin/bash

if [ "$CORES" == "" ]; then
  CORES=1
fi

AGGREGATE=5
EPISODES=3000

declare -a VALS=(8 0.5 16 4)

for i in $(seq 1 $AGGREGATE); do
  random[$i]=$RANDOM
done

random=(2698 5643 18028 29364 30519)

for v in $(seq 0 4 $((${#VALS[@]} - 1))); do
  DIR="experiment/${VALS[$((v + 0))]}-${VALS[$((v + 1))]}-${VALS[$((v + 2))]}-${VALS[$((v + 3))]}"
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

experiments () {
  for v in $(seq $1 $2 $((${#VALS[@]} - 1))); do
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
                  ^x (/ 10 ${VALS[$((v + 0))]})
                  ^x-dot (/ 1 ${VALS[$((v + 1))]})
                  ^theta (/ 3.1415926 ${VALS[$((v + 2))]})
                  ^theta-dot (/ 3.141526 ${VALS[$((v + 3))]}))
          }" >> $DIR/in.soar

    for r in ${random[@]}; do
      experiment $r $DIR &
    done
    wait

    ./cartpole.py $DIR/*.out
  done
}

for x in $(seq 0 $(($CORES - 1))); do
#   echo experiments $((4 * x)) $((4 * $CORES))
  experiments $((4 * x)) $((4 * $CORES)) #&
done
wait
