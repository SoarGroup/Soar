85
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 simple-bot
ENUMERATION 8 1 initialize-simple-bot
SOAR_ID 9
ENUMERATION 10 1 override
SOAR_ID 11
SOAR_ID 12
SOAR_ID 13
SOAR_ID 14
ENUMERATION 15 2 false true
ENUMERATION 16 2 false true
SOAR_ID 17
SOAR_ID 18
FLOAT_RANGE 19 -1.0 1.0
FLOAT_RANGE 20 -1.0 1.0
ENUMERATION 21 2 false true
FLOAT_RANGE 22 0.0 Infinity
FLOAT_RANGE 23 0.0 1.0
SOAR_ID 24
SOAR_ID 25
FLOAT_RANGE 26 0.0 1.0
FLOAT_RANGE 27 0.0 1.0
ENUMERATION 28 2 false true
SOAR_ID 29
ENUMERATION 30 1 wander
SOAR_ID 31
ENUMERATION 32 1 state
ENUMERATION 33 1 wander
ENUMERATION 34 1 drive
SOAR_ID 35
ENUMERATION 36 1 stop
SOAR_ID 37
ENUMERATION 38 1 reverse
SOAR_ID 39
ENUMERATION 40 1 turn
SOAR_ID 41
ENUMERATION 42 1 reset
SOAR_ID 43
SOAR_ID 44
ENUMERATION 45 1 wait
ENUMERATION 46 1 elaboration
SOAR_ID 47
SOAR_ID 48
FLOAT_RANGE 49 -1.0 1.0
FLOAT_RANGE 50 -1.0 1.0
ENUMERATION 51 2 false true
ENUMERATION 52 2 complete error
SOAR_ID 53
SOAR_ID 54
SOAR_ID 55
SOAR_ID 56
ENUMERATION 57 1 state
ENUMERATION 58 1 none
SOAR_ID 59
SOAR_ID 60
SOAR_ID 61
FLOAT_RANGE 62 0.0 Infinity
FLOAT_RANGE 63 0.0 Infinity
ENUMERATION 64 2 backward forward
ENUMERATION 65 2 left right
SOAR_ID 66
FLOAT_RANGE 67 0.0 Infinity
FLOAT_RANGE 68 0.0 Infinity
FLOAT_RANGE 69 0.0 Infinity
ENUMERATION 70 2 false true
ENUMERATION 71 2 false true
ENUMERATION 72 5 driving idle reversing stopping turning
INTEGER_RANGE 73 0 600
FLOAT_RANGE 74 0.0 1.0
FLOAT_RANGE 75 0.0 1.0
FLOAT_RANGE 76 0.0 Infinity
SOAR_ID 77
ENUMERATION 78 2 false true
SOAR_ID 79
SOAR_ID 80
SOAR_ID 81
STRING 82
SOAR_ID 83
SOAR_ID 84
95
0 attribute 57
0 choices 58
0 io 1
0 motion-state 59
0 name 7
0 operator 4
0 operator 9
0 operator 29
0 operator 44
0 reverse 64
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 config 24
2 override 17
2 random 23
2 sensors 11
2 time 22
3 drive-power 48
3 stop-sim 84
4 name 8
9 actions 47
9 name 10
11 bumper 12
11 com 79
11 sicklrf 77
12 front 13
12 rear 14
13 pressed 15
13 was-pressed 70
14 pressed 16
14 was-pressed 71
17 active 21
17 drive-power 18
18 left 49
18 right 20
18 stop 28
24 delay 66
24 power 25
25 drive 26
25 reverse 27
29 name 30
31 io 1
31 name 33
31 noise 73
31 operator 35
31 operator 37
31 operator 39
31 operator 41
31 operator 43
31 superstate 0
31 top-state 0
31 turn-left-power 75
31 turn-right-power 74
31 type 32
35 actions 53
35 name 34
37 actions 54
37 name 36
37 timeout 62
39 actions 55
39 name 38
39 reverse 60
41 actions 56
41 name 40
41 turn 61
43 name 42
44 name 45
44 random 46
47 drive-power 48
48 left 19
48 right 50
48 status 52
48 stop 51
53 drive-power 48
54 drive-power 48
55 drive-power 48
56 drive-power 48
59 current 72
59 timeout 62
61 direction 65
61 timeout 63
66 reverse 68
66 stop 67
66 turn 76
66 variance 69
77 obstacle 78
79 broadcast 80
80 next 81
81 next 83
81 value 82
83 next 81
83 value 82
