#!/bin/bash

TERMINAL=2000
OUTPUT=blocks-world-out
mkdir -p $OUTPUT

SEED=""
for i in $(seq 0 9); do
  SEED="$SEED $RANDOM"
done

for seed in ${SEED[@]}; do
  killall cli

  rm -f fromcli tocli

  mkfifo tocli
  mkfifo fromcli

  out_c/cli <tocli >>fromcli 2>/dev/null &

  echo "SEED $seed"
  echo "srand $seed" >>tocli

  echo "source blocks-world/blocks-world-overgeneral.soar" >>tocli
  echo "watch 0" >>tocli

  echo "init" >>tocli
  echo "run" >>tocli

  STEP_COUNT=0
  EPISODE_COUNT=0
  FILE=$OUTPUT/blocks-world.$seed.out

  rm $FILE &> /dev/null

  while read line; do
    LINE="$line"
    STEP=${LINE/*STEP /}

    if [ "$LINE" != "$STEP" ]; then
      EPISODE_COUNT=$(($EPISODE_COUNT+1))

      COUNT=0
      while [ $COUNT -ne $STEP ]; do
        COUNT=$(($COUNT+1))
        STEP_COUNT=$(($STEP_COUNT+1))
        if [ $COUNT -ne $STEP ]; then
          REWARD=-1
        else
          REWARD=0
        fi

        echo "$STEP_COUNT $EPISODE_COUNT $COUNT $REWARD" >> $FILE

        if [ $STEP_COUNT -eq $TERMINAL ]; then
          break
        fi
      done

      if [ $STEP_COUNT -eq $TERMINAL ]; then
        echo "exit"
        break
      fi

      echo "init"
      echo "run"
    fi
  done <> fromcli >>tocli
done

rm -f fromcli tocli

killall cli
