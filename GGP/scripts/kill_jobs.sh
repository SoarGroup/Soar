#!/bin/sh
m="bahamut smaug grapes auk badboy flamingo winter wyrm 141.212.109.250"

for i in $m; do
  echo $i
  ssh $i "kill -9 \$(ps -o comm,pid -u $USER | grep ^python | awk '{print \$2}' | tr '\\n' ' ')"
done
