#!/bin/bash

time out/PuddleWorld --episodes 200 --seed $RANDOM --rules PuddleWorld/puddle-world.soar > puddleworld.out

./puddleworld.py
