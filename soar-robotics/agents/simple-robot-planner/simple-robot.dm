59
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
61
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
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 time 57
3 move 20
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
11 superoperator 51
11 superstate 0
11 top-state 0
11 type 12
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
51 next-action-time 52
57 seconds 58
