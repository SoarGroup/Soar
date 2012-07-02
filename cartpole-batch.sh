#!/bin/bash

if [ "$CORES" == "" ]; then
  CORES=1
fi

AGGREGATE=5
EPISODES=3000

declare -a VALS=(8 0.5 16 2 \
                \
                 8 0.5 16 4 \
                 8 0.5 32 2 \
                 8 1 16 2 \
                 16 0.5 16 2 \
                \
                 8 0.5 32 4
                 8 1 16 4
                 16 0.5 16 4
                 8 1 32 2
                 16 0.5 32 2
                 16 1 16 2
                \
                 8 1 32 4 \
                 16 0.5 32 4 \
                 16 1 16 4 \
                 16 1 32 2 \
                \
                 16 1 32 4 \
                )

for i in $(seq 1 $AGGREGATE); do
  random[$i]=$RANDOM
done

for v in $(seq 0 4 $((${#VALS[@]} - 1))); do
  DIR="experiment/${VALS[$((v + 0))]}-${VALS[$((v + 1))]}-${VALS[$((v + 2))]}-${VALS[$((v + 3))]}"
  mkdir -p $DIR
  rm -i $DIR/*
  DIRS[v]=$DIR
done

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
          ^div-x (/ 10 ${VALS[$((v + 0))]})
          ^div-x-dot (/ 1 ${VALS[$((v + 1))]})
          ^div-theta (/ 3.1415926 ${VALS[$((v + 2))]})
          ^div-theta-dot (/ 3.141526 ${VALS[$((v + 3))]}))
  }" >> $DIR/in.soar

    RUN=1
    for r in ${random[@]}; do
      echo Run $RUN of $AGGREGATE
      echo time $(pwd)/out/CartPole --episodes $EPISODES --seed $r --rules $DIR/in.soar --rl-rules-out $DIR/out-$r.soar \> $DIR/cartpole-$r.out
      time $(pwd)/out/CartPole --episodes $EPISODES --seed $r --rules $DIR/in.soar --rl-rules-out $DIR/out-$r.soar > $DIR/cartpole-$r.out
      echo ""

      #cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789] "
      #cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789][0123456789] "

      RUN=$(($RUN + 1))
    done
  done
}

for x in $(seq 0 $(($CORES - 1))); do
#   echo experiments $((4 * x)) $((4 * $CORES))
  experiments $((4 * x)) $((4 * $CORES)) &
done
wait
