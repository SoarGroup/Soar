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
FLOAT_RANGE 13 0.0 Infinity
FLOAT_RANGE 14 0.0 Infinity
SOAR_ID 15
SOAR_ID 16
INTEGER_RANGE 17 0 2147483647
FLOAT_RANGE 18 -Infinity Infinity
INTEGER_RANGE 19 -2147483648 2147483647
SOAR_ID 20
INTEGER_RANGE 21 0 2147483647
INTEGER_RANGE 22 -2147483648 2147483647
FLOAT_RANGE 23 -Infinity Infinity
SOAR_ID 24
FLOAT_RANGE 25 -Infinity Infinity
FLOAT_RANGE 26 -Infinity Infinity
FLOAT_RANGE 27 0.0 359.989990234375
STRING 28
SOAR_ID 29
SOAR_ID 30
STRING 31
FLOAT_RANGE 32 -Infinity Infinity
FLOAT_RANGE 33 -Infinity Infinity
FLOAT_RANGE 34 0.0 359.989990234375
FLOAT_RANGE 35 0.0 Infinity
FLOAT_RANGE 36 -180.0 180.0
FLOAT_RANGE 37 0.0 Infinity
FLOAT_RANGE 38 0.0 Infinity
SOAR_ID 39
INTEGER_RANGE 40 -2 2
FLOAT_RANGE 41 0.0 Infinity
FLOAT_RANGE 42 -180.0 180.0
FLOAT_RANGE 43 -180.0 180.0
SOAR_ID 44
SOAR_ID 45
SOAR_ID 46
SOAR_ID 47
SOAR_ID 48
SOAR_ID 49
SOAR_ID 50
SOAR_ID 51
SOAR_ID 52
SOAR_ID 53
STRING 54
FLOAT_RANGE 55 -Infinity Infinity
FLOAT_RANGE 56 -Infinity Infinity
STRING 57
STRING 58
STRING 59
FLOAT_RANGE 60 -1.0 1.0
FLOAT_RANGE 61 -1.0 1.0
ENUMERATION 62 3 backward forward stop
FLOAT_RANGE 63 0.0 1.0
FLOAT_RANGE 64 0.0 1.0
ENUMERATION 65 3 left right stop
FLOAT_RANGE 66 0.0 359.989990234375
FLOAT_RANGE 67 0.0 1.0
ENUMERATION 68 2 complete error
ENUMERATION 69 2 complete error
ENUMERATION 70 2 complete error
ENUMERATION 71 2 complete error
ENUMERATION 72 2 complete error
ENUMERATION 73 2 complete error
ENUMERATION 74 2 complete error
ENUMERATION 75 2 complete error
ENUMERATION 76 2 complete error
ENUMERATION 77 2 complete error
FLOAT_RANGE 78 0.0 Infinity
FLOAT_RANGE 79 0.0 Infinity
INTEGER_RANGE 80 1 2147483647
FLOAT_RANGE 81 0.0 Infinity
FLOAT_RANGE 82 0.0 180.0
83
0 io 1
0 name 7
0 operator 4
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 ranges 9
2 self 10
2 time 11
3 add-waypoint 48
3 broadcast-message 53
3 disable-waypoint 51
3 enable-waypoint 50
3 motor 44
3 move 45
3 remove-waypoint 49
3 rotate 46
3 rotate-to 52
3 stop 47
4 name 8
9 range 39
10 geometry 12
10 motor 15
10 name 28
10 pose 24
10 waypoints 29
11 microseconds 38
11 seconds 37
12 baseline-meters 13
12 length 79
12 ranger-slices 80
12 tick-meters 14
12 width 78
15 left 16
15 right 20
16 current 17
16 position 19
16 velocity 18
20 current 21
20 position 22
20 velocity 23
24 x 25
24 y 26
24 yaw 27
29 waypoint 30
30 abs-relative-bearing 82
30 distance 35
30 id 31
30 relative-bearing 36
30 x 32
30 y 33
30 yaw 34
39 distance 41
39 end 42
39 id 40
39 start 43
44 left 61
44 right 60
44 status 73
45 direction 62
45 status 72
45 throttle 63
46 direction 65
46 status 70
46 throttle 64
47 status 68
48 id 54
48 status 76
48 x 55
48 y 56
49 id 59
49 status 71
50 id 58
50 status 74
51 id 57
51 status 75
52 status 69
52 throttle 67
52 tolerance 81
52 yaw 66
53 status 77
