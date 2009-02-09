79
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
INTEGER_RANGE 17 -2147483648 2147483647
SOAR_ID 18
INTEGER_RANGE 19 -2147483648 2147483647
SOAR_ID 20
FLOAT_RANGE 21 -Infinity Infinity
FLOAT_RANGE 22 -Infinity Infinity
FLOAT_RANGE 23 0.0 359.989990234375
STRING 24
SOAR_ID 25
SOAR_ID 26
STRING 27
FLOAT_RANGE 28 -Infinity Infinity
FLOAT_RANGE 29 -Infinity Infinity
FLOAT_RANGE 30 0.0 359.989990234375
FLOAT_RANGE 31 0.0 Infinity
FLOAT_RANGE 32 -180.0 180.0
FLOAT_RANGE 33 0.0 Infinity
FLOAT_RANGE 34 0.0 Infinity
SOAR_ID 35
INTEGER_RANGE 36 -2 2
FLOAT_RANGE 37 0.0 Infinity
FLOAT_RANGE 38 -180.0 180.0
FLOAT_RANGE 39 -180.0 180.0
SOAR_ID 40
SOAR_ID 41
SOAR_ID 42
SOAR_ID 43
SOAR_ID 44
SOAR_ID 45
SOAR_ID 46
SOAR_ID 47
SOAR_ID 48
SOAR_ID 49
STRING 50
FLOAT_RANGE 51 -Infinity Infinity
FLOAT_RANGE 52 -Infinity Infinity
STRING 53
STRING 54
STRING 55
FLOAT_RANGE 56 -1.0 1.0
FLOAT_RANGE 57 -1.0 1.0
ENUMERATION 58 3 backward forward stop
FLOAT_RANGE 59 0.0 1.0
FLOAT_RANGE 60 0.0 1.0
ENUMERATION 61 3 left right stop
FLOAT_RANGE 62 0.0 359.989990234375
FLOAT_RANGE 63 0.0 1.0
ENUMERATION 64 2 complete error
ENUMERATION 65 2 complete error
ENUMERATION 66 2 complete error
ENUMERATION 67 2 complete error
ENUMERATION 68 2 complete error
ENUMERATION 69 2 complete error
ENUMERATION 70 2 complete error
ENUMERATION 71 2 complete error
ENUMERATION 72 2 complete error
ENUMERATION 73 2 complete error
FLOAT_RANGE 74 0.0 Infinity
FLOAT_RANGE 75 0.0 Infinity
INTEGER_RANGE 76 1 2147483647
FLOAT_RANGE 77 0.0 Infinity
FLOAT_RANGE 78 0.0 180.0
79
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
3 add-waypoint 44
3 broadcast-message 49
3 disable-waypoint 47
3 enable-waypoint 46
3 motor 40
3 move 41
3 remove-waypoint 45
3 rotate 42
3 rotate-to 48
3 stop 43
4 name 8
9 range 35
10 geometry 12
10 motor 15
10 name 24
10 pose 20
10 waypoints 25
11 microseconds 34
11 seconds 33
12 baseline-meters 13
12 length 75
12 ranger-slices 76
12 tick-meters 14
12 width 74
15 left 16
15 right 18
16 position 17
18 position 19
20 x 21
20 y 22
20 yaw 23
25 waypoint 26
26 abs-relative-bearing 78
26 distance 31
26 id 27
26 relative-bearing 32
26 x 28
26 y 29
26 yaw 30
35 distance 37
35 end 38
35 id 36
35 start 39
40 left 57
40 right 56
40 status 69
41 direction 58
41 status 68
41 throttle 59
42 direction 61
42 status 66
42 throttle 60
43 status 64
44 id 50
44 status 72
44 x 51
44 y 52
45 id 55
45 status 67
46 id 54
46 status 70
47 id 53
47 status 71
48 status 65
48 throttle 63
48 tolerance 77
48 yaw 62
49 status 73
