#!/bin/bash

pushd . > /dev/null
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
SCRIPT_HOME="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
popd > /dev/null

SOAR_HOME="$SCRIPT_HOME/../out_cd"

CLI="$SOAR_HOME/cli"
AGENT="$SCRIPT_HOME/cliff.soar"

NUM_TRIALS=50
NUM_EPISODES=50

trial() {
  TOTAL_STEPS=0
  SUCCESSES=0

  for t in $(seq 1 $NUM_TRIALS); do
    printf "source \"$AGENT\"\ncommand-to-file rules-out.txt print -f\nexit" | "$CLI" > /dev/null

    for i in $(seq 1 $NUM_EPISODES); do
      OUTPUT=$(printf "source \"$SCRIPT_HOME/cliff/_firstload.soar\"\nrl --set learning-policy $1\nsource rules-out.txt\nrun\ncommand-to-file rules-out.txt print -f\nexit" | "$CLI")
      NUM_STEPS=$(echo "$OUTPUT" | grep -c "(.*)")
#       echo "$OUTPUT"
      SUCCESS=$(echo "$OUTPUT" | grep -c Goal)

      TOTAL_STEPS=$(($TOTAL_STEPS+$NUM_STEPS))
      SUCCESSES=$(($SUCCESSES+$SUCCESS))

#       echo "$NUM_STEPS to $SUCCESS"
    done
  done

  echo "$td : $TOTAL_STEPS total to $SUCCESSES/$(($NUM_EPISODES*$NUM_TRIALS)) over $NUM_TRIALS"
}

for td in sarsa on-policy-gq-lambda q-learning off-policy-gq-lambda; do
  trial $td &
done
wait
