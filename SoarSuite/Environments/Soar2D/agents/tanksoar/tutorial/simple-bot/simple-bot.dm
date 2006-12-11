96
SOAR_ID 0
ENUMERATION 1 1 nil
SOAR_ID 2
SOAR_ID 3
ENUMERATION 4 1 tanksoar
ENUMERATION 5 1 state
SOAR_ID 6
SOAR_ID 7
SOAR_ID 8
ENUMERATION 9 2 no yes
INTEGER_RANGE 10 -2147483648 2147483647
SOAR_ID 11
INTEGER_RANGE 12 -2147483648 2147483647
INTEGER_RANGE 13 -2147483648 2147483647
INTEGER_RANGE 14 -2147483648 2147483647
INTEGER_RANGE 15 -2147483648 2147483647
INTEGER_RANGE 16 -2147483648 2147483647
INTEGER_RANGE 17 -2147483648 2147483647
INTEGER_RANGE 18 -2147483648 2147483647
INTEGER_RANGE 19 -2147483648 2147483647
INTEGER_RANGE 20 -2147483648 2147483647
INTEGER_RANGE 21 -2147483648 2147483647
INTEGER_RANGE 22 -2147483648 2147483647
INTEGER_RANGE 23 -2147483648 2147483647
INTEGER_RANGE 24 -2147483648 2147483647
ENUMERATION 25 4 east north south west
ENUMERATION 26 2 no yes
ENUMERATION 27 2 no yes
INTEGER_RANGE 28 -2147483648 2147483647
ENUMERATION 29 7 black blue green orange purple red yellow
SOAR_ID 30
ENUMERATION 31 2 off on
FLOAT_RANGE 32 0.0 1.0
ENUMERATION 33 2 no yes
ENUMERATION 34 2 off on
SOAR_ID 35
ENUMERATION 36 5 backward forward left right silent
SOAR_ID 37
INTEGER_RANGE 38 -2147483648 2147483647
ENUMERATION 39 3 center left right
SOAR_ID 40
ENUMERATION 41 4 backward forward left right
SOAR_ID 42
SOAR_ID 43
ENUMERATION 44 1 missile
SOAR_ID 45
SOAR_ID 46
SOAR_ID 47
ENUMERATION 48 2 off on
SOAR_ID 49
ENUMERATION 50 4 backward forward left right
SOAR_ID 51
ENUMERATION 52 2 off on
ENUMERATION 53 2 left right
ENUMERATION 54 1 complete
ENUMERATION 55 2 operator state
ENUMERATION 56 2 multiple none
ENUMERATION 57 2 no-change tie
ENUMERATION 58 1 wander
SOAR_ID 59
ENUMERATION 60 2 move turn
SOAR_ID 61
SOAR_ID 62
SOAR_ID 63
ENUMERATION 64 1 attack
SOAR_ID 65
ENUMERATION 66 2 fire-missile slide
ENUMERATION 67 1 chase
SOAR_ID 68
ENUMERATION 69 2 move turn
ENUMERATION 70 1 retreat
SOAR_ID 71
ENUMERATION 72 2 move wait
ENUMERATION 73 4 backward forward left right
ENUMERATION 74 1 low
INTEGER_RANGE 75 0 1000
INTEGER_RANGE 76 0 1000
INTEGER_RANGE 77 1 14
INTEGER_RANGE 78 1 14
ENUMERATION 79 1 none
INTEGER_RANGE 80 0 28
INTEGER_RANGE 81 1 14
SOAR_ID 82
SOAR_ID 83
SOAR_ID 84
ENUMERATION 85 1 attack
ENUMERATION 86 1 retreat
ENUMERATION 87 1 wander
ENUMERATION 88 1 chase
SOAR_ID 89
ENUMERATION 90 1 wait
ENUMERATION 91 1 elaboration
ENUMERATION 92 1 move
SOAR_ID 93
ENUMERATION 94 1 turn
SOAR_ID 95
137
0 attribute 55
0 choices 56
0 impasse 57
0 io 3
0 item 2
0 missiles-energy 74
0 name 4
0 operator 2
0 operator 82
0 operator 83
0 operator 84
0 operator 89
0 side-direction 40
0 superstate 1
0 top-state 0
0 type 5
2 actions 42
2 name 88
3 input-link 7
3 output-link 42
6 io 3
6 name 58
6 operator 59
6 superstate 0
6 top-state 0
7 blocked 8
7 clock 10
7 constants 11
7 direction 25
7 energy 75
7 energyrecharger 26
7 health 76
7 healthrecharger 27
7 incoming 8
7 missiles 28
7 my-color 29
7 radar 30
7 radar-distance 77
7 radar-setting 78
7 radar-status 31
7 random 32
7 resurrect 33
7 rwaves 8
7 shield-status 34
7 smell 35
7 sound 36
7 x 78
7 y 78
8 backward 9
8 forward 9
8 left 9
8 right 9
11 absorbdamage 12
11 energyincrease 13
11 healthincrease 14
11 mapdim 15
11 maxenergy 16
11 maxhealth 17
11 maxradar 18
11 missilesincrease 19
11 projectiledamage 20
11 shieldcost 21
11 sounddist 22
11 terraindamage 23
11 worldcountlimit 24
30 energy 37
30 flag 37
30 health 37
30 missiles 37
30 obstacle 37
30 open 37
30 tank 37
35 color 29
35 distance 79
35 distance 80
37 distance 38
37 position 39
40 backward 41
40 forward 41
40 left 41
40 right 41
42 fire 43
42 move 49
42 radar 47
42 radar-power 46
42 rotate 45
42 shields 51
43 status 54
43 weapon 44
45 direction 53
45 status 54
46 setting 81
46 status 54
47 status 54
47 switch 48
49 direction 50
49 status 54
51 status 54
51 switch 52
59 actions 42
59 name 60
61 io 3
61 name 64
61 operator 65
61 operator 93
61 operator 95
61 superstate 0
61 top-state 0
62 io 3
62 name 67
62 operator 68
62 sound-direction 36
62 superstate 0
62 top-state 0
63 avoid-direction 73
63 direction 73
63 io 3
63 name 70
63 operator 71
63 superstate 0
63 top-state 0
65 actions 42
65 name 66
68 actions 42
68 name 69
71 actions 42
71 name 72
82 actions 42
82 name 87
83 actions 42
83 name 86
84 actions 42
84 name 85
89 name 90
89 random 91
93 name 92
95 name 94
