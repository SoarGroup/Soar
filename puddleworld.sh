#!/bin/bash

time out_c/PuddleWorld --episodes -1 --steps 50000 --seed $RANDOM --rules puddle-world/puddle-world-overgeneral.soar > puddleworld.out

./puddleworld.py
