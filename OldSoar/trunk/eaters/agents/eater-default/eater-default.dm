18
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
ENUMERATION 4 1 state
ENUMERATION 5 1 nil
ENUMERATION 6 1 eater-default
SOAR_ID 7
ENUMERATION 8 1 complete
ENUMERATION 9 4 east north south west
SOAR_ID 10
ENUMERATION 11 5 bonusfood eater empty normalfood wall
SOAR_ID 12
ENUMERATION 13 4 east north south west
ENUMERATION 14 5 black blue purple red yellow
INTEGER_RANGE 15 -2147483648 2147483647
INTEGER_RANGE 16 -2147483648 2147483647
INTEGER_RANGE 17 -2147483648 2147483647
23
0 io 1
0 name 6
0 superstate 5
0 top-state 0
0 type 4
1 input-link 2
1 output-link 3
2 eater 12
2 my-location 10
3 jump 7
3 move 7
7 direction 9
7 status 8
10 content 11
10 east 10
10 north 10
10 south 10
10 west 10
12 direction 13
12 name 14
12 score 15
12 x 16
12 y 17
