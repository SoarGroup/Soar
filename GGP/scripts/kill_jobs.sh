#!/bin/sh

m="bahamut smaug wyrm grapes auk badboy flamingo winter"

for i in $m; do
  echo $i
  ssh $i "kill -9 \$(ps h -u $USER | grep python | awk '{print \$1}')"
done
