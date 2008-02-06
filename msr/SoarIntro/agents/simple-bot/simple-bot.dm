73
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
FLOAT_RANGE 16 0.0 Infinity
FLOAT_RANGE 17 0.0 Infinity
ENUMERATION 18 2 false true
SOAR_ID 19
SOAR_ID 20
FLOAT_RANGE 21 -1.0 1.0
FLOAT_RANGE 22 -1.0 1.0
ENUMERATION 23 2 false true
FLOAT_RANGE 24 0.0 Infinity
FLOAT_RANGE 25 0.0 1.0
SOAR_ID 26
SOAR_ID 27
FLOAT_RANGE 28 0.0 1.0
FLOAT_RANGE 29 0.0 1.0
ENUMERATION 30 2 false true
SOAR_ID 31
ENUMERATION 32 1 wander
SOAR_ID 33
ENUMERATION 34 1 state
ENUMERATION 35 1 wander
ENUMERATION 36 1 drive
SOAR_ID 37
ENUMERATION 38 1 stop
SOAR_ID 39
ENUMERATION 40 1 reverse
SOAR_ID 41
ENUMERATION 42 1 turn
SOAR_ID 43
ENUMERATION 44 1 reset
SOAR_ID 45
SOAR_ID 46
ENUMERATION 47 1 wait
ENUMERATION 48 1 elaboration
SOAR_ID 49
SOAR_ID 50
FLOAT_RANGE 51 -1.0 1.0
FLOAT_RANGE 52 -1.0 1.0
ENUMERATION 53 2 false true
ENUMERATION 54 2 complete error
SOAR_ID 55
SOAR_ID 56
SOAR_ID 57
SOAR_ID 58
ENUMERATION 59 1 state
ENUMERATION 60 1 none
SOAR_ID 61
SOAR_ID 62
SOAR_ID 63
FLOAT_RANGE 64 0.0 Infinity
FLOAT_RANGE 65 0.0 Infinity
FLOAT_RANGE 66 0.0 Infinity
ENUMERATION 67 2 backward forward
ENUMERATION 68 2 left right
SOAR_ID 69
FLOAT_RANGE 70 0.0 Infinity
FLOAT_RANGE 71 0.0 Infinity
FLOAT_RANGE 72 0.0 Infinity
85
0 attribute 59
0 choices 60
0 io 1
0 name 7
0 operator 4
0 operator 9
0 operator 31
0 operator 46
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 config 26
2 override 19
2 random 25
2 sensors 11
2 time 24
3 drive-power 50
4 name 8
9 actions 49
9 name 10
11 bumper 12
12 front 13
12 rear 14
13 pressed 15
13 time 16
14 pressed 18
14 time 17
19 active 23
19 drive-power 20
20 left 51
20 right 22
20 stop 30
26 delay 69
26 power 27
27 drive 28
27 reverse 29
31 name 32
33 io 1
33 name 35
33 operator 37
33 operator 39
33 operator 41
33 operator 43
33 operator 45
33 reverse 62
33 stop 61
33 superstate 0
33 top-state 0
33 turn 63
33 type 34
37 actions 55
37 name 36
39 actions 56
39 name 38
39 reverse 62
39 stop 61
39 turn 63
41 actions 57
41 name 40
41 reverse 62
43 actions 58
43 name 42
43 turn 63
45 name 44
46 name 47
46 random 48
49 drive-power 50
50 left 21
50 right 52
50 status 54
50 stop 53
55 drive-power 50
56 drive-power 50
57 drive-power 50
58 drive-power 50
61 timeout 64
62 direction 67
62 timeout 65
63 direction 68
63 timeout 66
69 reverse 71
69 stop 70
69 turn 72
