83
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 default-robot
ENUMERATION 8 1 initialize-default-robot
SOAR_ID 9
SOAR_ID 10
SOAR_ID 11
SOAR_ID 12
ENUMERATION 13 2 false true
SOAR_ID 14
FLOAT_RANGE 15 -1.0 1.0
FLOAT_RANGE 16 -1.0 1.0
SOAR_ID 17
FLOAT_RANGE 18 0.0 Infinity
FLOAT_RANGE 19 0.0 Infinity
SOAR_ID 20
SOAR_ID 21
INTEGER_RANGE 22 0 2147483647
FLOAT_RANGE 23 -Infinity Infinity
INTEGER_RANGE 24 -2147483648 2147483647
SOAR_ID 25
INTEGER_RANGE 26 0 2147483647
INTEGER_RANGE 27 -2147483648 2147483647
FLOAT_RANGE 28 -Infinity Infinity
SOAR_ID 29
FLOAT_RANGE 30 -Infinity Infinity
FLOAT_RANGE 31 -Infinity Infinity
FLOAT_RANGE 32 0.0 359.989990234375
STRING 33
SOAR_ID 34
SOAR_ID 35
STRING 36
FLOAT_RANGE 37 -Infinity Infinity
FLOAT_RANGE 38 -Infinity Infinity
FLOAT_RANGE 39 0.0 359.989990234375
FLOAT_RANGE 40 0.0 Infinity
FLOAT_RANGE 41 -180.0 180.0
FLOAT_RANGE 42 0.0 Infinity
FLOAT_RANGE 43 0.0 Infinity
SOAR_ID 44
INTEGER_RANGE 45 -2 2
FLOAT_RANGE 46 0.0 Infinity
FLOAT_RANGE 47 -180.0 180.0
FLOAT_RANGE 48 -180.0 180.0
SOAR_ID 49
SOAR_ID 50
SOAR_ID 51
SOAR_ID 52
SOAR_ID 53
SOAR_ID 54
SOAR_ID 55
SOAR_ID 56
SOAR_ID 57
SOAR_ID 58
STRING 59
FLOAT_RANGE 60 -Infinity Infinity
FLOAT_RANGE 61 -Infinity Infinity
STRING 62
STRING 63
STRING 64
FLOAT_RANGE 65 -1.0 1.0
FLOAT_RANGE 66 -1.0 1.0
ENUMERATION 67 3 backward forward stop
FLOAT_RANGE 68 0.0 1.0
FLOAT_RANGE 69 0.0 1.0
ENUMERATION 70 3 left right stop
FLOAT_RANGE 71 0.0 359.989990234375
FLOAT_RANGE 72 0.0 1.0
ENUMERATION 73 2 complete error
ENUMERATION 74 2 complete error
ENUMERATION 75 2 complete error
ENUMERATION 76 2 complete error
ENUMERATION 77 2 complete error
ENUMERATION 78 2 complete error
ENUMERATION 79 2 complete error
ENUMERATION 80 2 complete error
ENUMERATION 81 2 complete error
ENUMERATION 82 2 complete error
83
0 io 1
0 name 7
0 operator 4
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 override 12
2 ranges 9
2 self 10
2 time 11
3 add-waypoint 53
3 broadcast-message 58
3 disable-waypoint 56
3 enable-waypoint 55
3 motor 49
3 move 50
3 remove-waypoint 54
3 rotate 51
3 rotate-to 57
3 stop 52
4 name 8
9 range 44
10 geometry 17
10 motor 20
10 name 33
10 pose 29
10 waypoints 34
11 microseconds 43
11 seconds 42
12 active 13
12 motor 14
14 left 16
14 right 15
17 baseline-meters 18
17 tick-meters 19
20 left 21
20 right 25
21 current 22
21 position 24
21 velocity 23
25 current 26
25 position 27
25 velocity 28
29 x 30
29 y 31
29 yaw 32
34 waypoint 35
35 distance 40
35 id 36
35 relative-bearing 41
35 x 37
35 y 38
35 yaw 39
44 distance 46
44 end 47
44 id 45
44 start 48
49 left 66
49 right 65
49 status 78
50 direction 67
50 status 77
50 throttle 68
51 direction 70
51 status 75
51 throttle 69
52 status 73
53 name 59
53 status 81
53 x 60
53 y 61
54 name 64
54 status 76
55 name 63
55 status 79
56 name 62
56 status 80
57 status 74
57 throttle 72
57 yaw 71
58 status 82
