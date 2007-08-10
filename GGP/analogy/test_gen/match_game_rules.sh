#!/bin/sh

python convert_graph.py game.gdl game rules.gdl rules > tempin
./klinux graph +f tempin
