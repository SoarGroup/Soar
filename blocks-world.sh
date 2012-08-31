#!/bin/bash

SEED=$RANDOM

killall cli

rm -f fromcli tocli

mkfifo tocli
mkfifo fromcli

out/cli <tocli >>fromcli 2>/dev/null &

echo "SEED $SEED"
echo "srand $RAND" >>tocli

echo "source ../blocks-world/blocks-world-overgeneral.soar" >>tocli
echo "watch 0" >>tocli

echo "init" >>tocli
echo "run" >>tocli

COUNT=0

rm blocks-world.out

while read line; do
  LINE="$line"
  STEP=${LINE/*STEP /}

  if [ "$LINE" != "$STEP" ]; then
    echo "$STEP" >> blocks-world.out
    COUNT=$(($COUNT+$STEP))

    if [ $COUNT -lt 1000 ]; then
      echo "init"
      echo "run"
    else
      echo "exit"
      break
    fi
  fi
done <> fromcli >>tocli

rm -f fromcli tocli
