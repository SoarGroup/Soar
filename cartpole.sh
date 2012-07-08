#!/bin/bash

time out/CartPole --episodes 1000 --seed $RANDOM --rules CartPole/cartpole-random-SML-var.soar > cartpole.out

## For 10/4, 1/8, pi/8, pi/4
#time out/CartPole --episodes 1000 --seed 10342 --rules CartPole/cartpole-random-SML-var.soar > cartpole.out

cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789] "
cat cartpole.out | grep "STEP [0123456789][0123456789][0123456789][0123456789][0123456789] "

./cartpole.py
