104
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 simple-robot
ENUMERATION 8 1 initialize
SOAR_ID 9
SOAR_ID 10
SOAR_ID 11
SOAR_ID 12
ENUMERATION 13 2 false true
SOAR_ID 14
FLOAT_RANGE 15 0.0 Infinity
FLOAT_RANGE 16 0.0 Infinity
SOAR_ID 17
SOAR_ID 18
INTEGER_RANGE 19 0 2147483647
FLOAT_RANGE 20 -Infinity Infinity
INTEGER_RANGE 21 -2147483648 2147483647
SOAR_ID 22
INTEGER_RANGE 23 0 2147483647
INTEGER_RANGE 24 -2147483648 2147483647
FLOAT_RANGE 25 -Infinity Infinity
SOAR_ID 26
FLOAT_RANGE 27 -Infinity Infinity
FLOAT_RANGE 28 -Infinity Infinity
FLOAT_RANGE 29 0.0 359.989990234375
STRING 30
SOAR_ID 31
SOAR_ID 32
STRING 33
FLOAT_RANGE 34 -Infinity Infinity
FLOAT_RANGE 35 -Infinity Infinity
FLOAT_RANGE 36 0.0 359.989990234375
FLOAT_RANGE 37 0.0 Infinity
FLOAT_RANGE 38 -180.0 180.0
FLOAT_RANGE 39 0.0 Infinity
FLOAT_RANGE 40 0.0 Infinity
SOAR_ID 41
INTEGER_RANGE 42 -2 2
FLOAT_RANGE 43 0.0 Infinity
FLOAT_RANGE 44 -180.0 180.0
FLOAT_RANGE 45 -180.0 180.0
SOAR_ID 46
SOAR_ID 47
SOAR_ID 48
SOAR_ID 49
SOAR_ID 50
SOAR_ID 51
SOAR_ID 52
SOAR_ID 53
SOAR_ID 54
SOAR_ID 55
STRING 56
FLOAT_RANGE 57 -Infinity Infinity
FLOAT_RANGE 58 -Infinity Infinity
STRING 59
STRING 60
STRING 61
FLOAT_RANGE 62 -1.0 1.0
FLOAT_RANGE 63 -1.0 1.0
ENUMERATION 64 3 backward forward stop
FLOAT_RANGE 65 0.0 1.0
FLOAT_RANGE 66 0.0 1.0
ENUMERATION 67 3 left right stop
FLOAT_RANGE 68 0.0 359.989990234375
FLOAT_RANGE 69 0.0 1.0
ENUMERATION 70 4 accepted complete error interrupted
ENUMERATION 71 2 complete error
ENUMERATION 72 2 complete error
ENUMERATION 73 2 complete error
ENUMERATION 74 2 complete error
ENUMERATION 75 2 complete error
ENUMERATION 76 2 complete error
ENUMERATION 77 2 complete error
ENUMERATION 78 2 complete error
ENUMERATION 79 2 complete error
FLOAT_RANGE 80 0.0 Infinity
FLOAT_RANGE 81 0.0 Infinity
INTEGER_RANGE 82 1 2147483647
FLOAT_RANGE 83 0.0 Infinity
FLOAT_RANGE 84 0.0 180.0
SOAR_ID 85
FLOAT_RANGE 86 0.0 Infinity
ENUMERATION 87 2 false true
SOAR_ID 88
SOAR_ID 89
ENUMERATION 90 1 action
FLOAT_RANGE 91 -180.0 180.0
SOAR_ID 92
ENUMERATION 93 3 accepted complete error
FLOAT_RANGE 94 -0.5 0.5
FLOAT_RANGE 95 -180.0 180.0
FLOAT_RANGE 96 -180.0 180.0
FLOAT_RANGE 97 -0.5 0.5
INTEGER_RANGE 98 -2 2
SOAR_ID 99
FLOAT_RANGE 100 -180.0 180.0
FLOAT_RANGE 101 -0.5 0.5
SOAR_ID 102
ENUMERATION 103 1 announce-self
107
0 io 1
0 name 7
0 operator 4
0 operator 89
0 operator 102
0 parameters 85
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 override 12
2 ranges 9
2 self 10
2 time 11
3 set-velocity 92
3 stop 49
4 name 8
9 range 41
10 geometry 14
10 motor 17
10 name 30
10 pose 26
10 waypoints 31
11 microseconds 40
11 seconds 39
12 active 13
12 add-waypoint 50
12 broadcast-message 55
12 disable-waypoint 53
12 enable-waypoint 52
12 motor 46
12 move 47
12 remove-waypoint 51
12 remove-waypoint 51
12 rotate 48
12 rotate-to 54
12 stop 49
14 baseline-meters 15
14 length 81
14 ranger-slices 82
14 tick-meters 16
14 width 80
17 left 18
17 right 22
18 current 19
18 position 21
18 velocity 20
22 current 23
22 position 24
22 velocity 25
26 x 27
26 y 28
26 yaw 29
31 waypoint 32
32 abs-relative-bearing 84
32 distance 37
32 id 33
32 relative-bearing 38
32 x 34
32 y 35
32 yaw 36
41 action 88
41 ang-distance 98
41 blocked 87
41 distance 43
41 end 44
41 id 42
41 start 45
46 left 63
46 right 62
46 status 75
47 direction 64
47 status 74
47 throttle 65
48 direction 67
48 status 72
48 throttle 66
49 status 70
50 name 56
50 status 78
50 x 57
50 y 58
51 name 61
51 status 73
52 name 60
52 status 76
53 name 59
53 status 77
54 status 71
54 throttle 69
54 tolerance 83
54 yaw 68
55 status 79
85 angular-velocity 96
85 linear-velocity 97
85 range-tolerance 86
88 set-velocity 99
89 action 88
89 distance 91
89 name 90
92 angular-velocity 95
92 linear-velocity 94
92 status 93
99 angular-velocity 100
99 linear-velocity 101
102 name 103
