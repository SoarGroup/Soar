87
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 simple-robot
ENUMERATION 8 1 initialize-simple-robot
SOAR_ID 9
ENUMERATION 10 1 wander
SOAR_ID 11
ENUMERATION 12 1 state
ENUMERATION 13 1 wander
ENUMERATION 14 1 move
SOAR_ID 15
ENUMERATION 16 1 rotate
SOAR_ID 17
ENUMERATION 18 1 stop
SOAR_ID 19
SOAR_ID 20
SOAR_ID 21
SOAR_ID 22
ENUMERATION 23 3 backward forward stop
FLOAT_RANGE 24 0.0 1.0
ENUMERATION 25 2 complete error
ENUMERATION 26 3 left right stop
FLOAT_RANGE 27 0.0 1.0
ENUMERATION 28 2 complete error
ENUMERATION 29 2 complete error
SOAR_ID 30
ENUMERATION 31 1 wait
ENUMERATION 32 2 operator state
ENUMERATION 33 2 multiple none
ENUMERATION 34 1 elaboration
ENUMERATION 35 2 no-change tie
SOAR_ID 36
STRING 37
SOAR_ID 38
SOAR_ID 39
FLOAT_RANGE 40 0.0 Infinity
FLOAT_RANGE 41 0.0 Infinity
SOAR_ID 42
SOAR_ID 43
ENUMERATION 44 3 backward forward stop
SOAR_ID 45
SOAR_ID 46
ENUMERATION 47 3 left right stop
SOAR_ID 48
SOAR_ID 49
STRING 50
SOAR_ID 51
FLOAT_RANGE 52 0.0 Infinity
FLOAT_RANGE 53 0.0 1.0
FLOAT_RANGE 54 -Infinity Infinity
FLOAT_RANGE 55 0.0 1.0
FLOAT_RANGE 56 0.0 1.0
SOAR_ID 57
INTEGER_RANGE 58 -2147483648 2147483647
SOAR_ID 59
SOAR_ID 60
FLOAT_RANGE 61 -Infinity Infinity
FLOAT_RANGE 62 -Infinity Infinity
SOAR_ID 63
ENUMERATION 64 1 wander-waypoint
SOAR_ID 65
ENUMERATION 66 1 choose-next-waypoint
ENUMERATION 67 1 go-to-waypoint
SOAR_ID 68
ENUMERATION 69 1 clean-up-destination
SOAR_ID 70
SOAR_ID 71
SOAR_ID 72
FLOAT_RANGE 73 -Infinity Infinity
FLOAT_RANGE 74 -Infinity Infinity
SOAR_ID 75
FLOAT_RANGE 76 -Infinity Infinity
SOAR_ID 77
ENUMERATION 78 1 choose-next-waypoint
SOAR_ID 79
FLOAT_RANGE 80 -Infinity Infinity
FLOAT_RANGE 81 -Infinity Infinity
FLOAT_RANGE 82 0.0 1.0
SOAR_ID 83
ENUMERATION 84 1 go-to-waypoint
SOAR_ID 85
ENUMERATION 86 1 choose-destination
94
0 action-interval-time 54
0 attribute 32
0 choices 33
0 impasse 35
0 io 1
0 item 36
0 name 7
0 operator 4
0 operator 9
0 operator 30
0 operator 63
0 operator 65
0 operator 83
0 operator 85
0 superstate 6
0 top-state 0
0 type 5
0 waypoints 71
1 input-link 2
1 output-link 3
2 self 59
2 time 57
3 move 20
3 move-to 79
3 rotate 21
3 stop 22
4 name 8
9 actions 38
9 name 10
11 future-time 40
11 io 1
11 name 13
11 next-action-time 41
11 operator 15
11 operator 17
11 operator 19
11 operator 68
11 operator 70
11 operator 77
11 superoperator 51
11 superstate 0
11 top-state 0
11 type 12
11 waypoints 71
15 actions 42
15 name 14
17 actions 45
17 name 16
19 actions 48
19 name 18
20 direction 23
20 status 25
20 throttle 24
21 direction 26
21 status 28
21 throttle 27
22 status 29
30 name 31
30 random 34
36 name 37
38 dummy 39
39 dummy 50
39 throttle 55
42 move 43
43 direction 44
43 throttle 53
45 rotate 46
46 direction 47
46 throttle 56
48 stop 49
51 destination 72
51 next-action-time 52
57 seconds 58
59 current-location 60
60 x 61
60 y 62
63 name 64
65 name 66
68 destination 72
68 name 67
70 name 69
71 at 72
71 waypoint 72
72 distance 76
72 next 75
72 x 73
72 y 74
77 destination 72
77 name 78
79 throttle 82
79 x 80
79 y 81
83 name 84
85 name 86
