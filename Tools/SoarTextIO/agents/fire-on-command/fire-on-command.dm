111
SOAR_ID 0
ENUMERATION 1 1 nil
SOAR_ID 2
SOAR_ID 3
ENUMERATION 4 1 tanksoar
ENUMERATION 5 1 state
SOAR_ID 6
SOAR_ID 7
ENUMERATION 8 2 no yes
INTEGER_RANGE 9 -2147483648 2147483647
SOAR_ID 10
INTEGER_RANGE 11 -2147483648 2147483647
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
ENUMERATION 24 4 east north south west
ENUMERATION 25 2 no yes
ENUMERATION 26 2 no yes
INTEGER_RANGE 27 -2147483648 2147483647
ENUMERATION 28 7 black blue green orange purple red yellow
SOAR_ID 29
ENUMERATION 30 2 off on
FLOAT_RANGE 31 0.0 1.0
ENUMERATION 32 2 no yes
ENUMERATION 33 2 off on
SOAR_ID 34
ENUMERATION 35 5 backward forward left right silent
SOAR_ID 36
INTEGER_RANGE 37 -2147483648 2147483647
ENUMERATION 38 3 center left right
SOAR_ID 39
ENUMERATION 40 4 backward forward left right
SOAR_ID 41
SOAR_ID 42
ENUMERATION 43 1 missile
SOAR_ID 44
SOAR_ID 45
SOAR_ID 46
ENUMERATION 47 2 off on
SOAR_ID 48
ENUMERATION 49 4 backward forward left right
SOAR_ID 50
ENUMERATION 51 2 off on
ENUMERATION 52 2 left right
ENUMERATION 53 1 complete
ENUMERATION 54 2 operator state
ENUMERATION 55 2 multiple none
ENUMERATION 56 2 no-change tie
ENUMERATION 57 1 low
INTEGER_RANGE 58 0 1000
INTEGER_RANGE 59 0 1000
INTEGER_RANGE 60 1 14
INTEGER_RANGE 61 1 14
ENUMERATION 62 1 none
INTEGER_RANGE 63 0 28
INTEGER_RANGE 64 1 14
SOAR_ID 65
SOAR_ID 66
SOAR_ID 67
ENUMERATION 68 1 attack
ENUMERATION 69 1 retreat
ENUMERATION 70 1 wander
ENUMERATION 71 1 chase
SOAR_ID 72
ENUMERATION 73 1 wait
ENUMERATION 74 1 elaboration
SOAR_ID 75
ENUMERATION 76 4 east north south west
INTEGER_RANGE 77 0 2147483647
SOAR_ID 78
SOAR_ID 79
ENUMERATION 80 4 east north south west
ENUMERATION 81 4 attack chase retreat wander
SOAR_ID 82
ENUMERATION 83 1 remove-sound
SOAR_ID 84
SOAR_ID 85
INTEGER_RANGE 86 0 15
SOAR_ID 87
ENUMERATION 88 1 init-map
SOAR_ID 89
SOAR_ID 90
SOAR_ID 91
ENUMERATION 92 3 -1 0 1
SOAR_ID 93
ENUMERATION 94 4 backward forward left right
ENUMERATION 95 1 14
SOAR_ID 96
ENUMERATION 97 1 *yes*
ENUMERATION 98 1 *yes*
ENUMERATION 99 1 *yes*
ENUMERATION 100 1 *yes*
ENUMERATION 101 1 *yes*
ENUMERATION 102 1 *yes*
SOAR_ID 103
ENUMERATION 104 1 fire-on-command
SOAR_ID 105
ENUMERATION 106 1 I-Dont-Know
SOAR_ID 107
ENUMERATION 108 1 Turn
SOAR_ID 109
ENUMERATION 110 1 move
171
0 attribute 54
0 choices 55
0 direction-map 78
0 impasse 56
0 io 3
0 item 2
0 map 84
0 maze-size 95
0 missiles-energy 57
0 name 4
0 name 81
0 operator 2
0 operator 65
0 operator 66
0 operator 67
0 operator 72
0 operator 82
0 operator 87
0 operator 103
0 operator 105
0 operator 107
0 operator 109
0 opposite-direction 93
0 radar-map 89
0 side-direction 39
0 sound 75
0 square 85
0 superstate 1
0 superstate 0
0 top-state 0
0 type 5
2 actions 41
2 name 71
3 input-link 6
3 output-link 41
6 blocked 7
6 clock 9
6 constants 10
6 direction 24
6 energy 58
6 energyrecharger 25
6 health 59
6 healthrecharger 26
6 incoming 7
6 missiles 27
6 my-color 28
6 radar 29
6 radar-distance 60
6 radar-setting 61
6 radar-status 30
6 random 31
6 resurrect 32
6 rwaves 7
6 shield-status 33
6 smell 34
6 sound 35
6 x 61
6 y 61
7 backward 8
7 forward 8
7 left 8
7 right 8
10 absorbdamage 11
10 energyincrease 12
10 healthincrease 13
10 mapdim 14
10 maxenergy 15
10 maxhealth 16
10 maxradar 17
10 missilesincrease 18
10 projectiledamage 19
10 shieldcost 20
10 sounddist 21
10 terraindamage 22
10 worldcountlimit 23
29 energy 36
29 flag 36
29 health 36
29 missiles 36
29 obstacle 36
29 open 36
29 tank 36
34 color 28
34 distance 62
34 distance 63
36 distance 37
36 position 38
39 backward 40
39 forward 40
39 left 40
39 right 40
41 fire 42
41 move 48
41 radar 46
41 radar-power 45
41 rotate 44
41 shields 50
42 status 53
42 weapon 43
44 direction 52
44 status 53
45 setting 64
45 status 53
46 status 53
46 switch 47
48 direction 49
48 status 53
50 status 53
50 switch 51
65 actions 41
65 name 70
66 actions 41
66 name 69
67 actions 41
67 name 68
72 name 73
72 random 74
75 absolute-direction 76
75 expire-time 77
78 east 79
78 north 79
78 south 79
78 west 79
79 backward 80
79 forward 80
79 left 80
79 right 80
82 name 83
84 energy 96
84 health 96
84 missiles 96
84 obstacle 96
84 open 96
84 square 85
84 tank 96
85 east 85
85 east-x 86
85 energy 98
85 health 97
85 missiles 100
85 north 85
85 obstacle 99
85 open 102
85 south 85
85 south-y 86
85 tank 101
85 west 85
85 x 86
85 y 86
87 name 88
89 east 90
89 north 90
89 south 90
89 west 90
90 center 91
90 left 91
90 right 91
90 sx 92
90 sy 92
91 x 92
91 y 92
93 backward 94
93 forward 94
93 left 94
93 right 94
96 x 86
96 y 86
103 name 104
105 name 106
107 name 108
109 name 110
