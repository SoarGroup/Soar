#!/bin/bash

time out_c/PuddleWorld --episodes 500 --seed $RANDOM --rules PuddleWorld/puddle-world.soar > puddleworld.out

./puddleworld.py
