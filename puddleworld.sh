#!/bin/bash

time out/PuddleWorld --episodes 200 --seed $RANDOM --rules out/test_agents/puddle-world.soar > puddleworld.out

./puddleworld.py
