#!/bin/sh

cat game.gdl | dot -Tps > game.ps
cat rules.gdl | dot -Tps > rules.ps
