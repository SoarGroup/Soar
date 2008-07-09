58
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
FLOAT_RANGE 20 -Infinity Infinity
SOAR_ID 21
SOAR_ID 22
SOAR_ID 23
ENUMERATION 24 3 backward forward stop
FLOAT_RANGE 25 0.0 1.0
ENUMERATION 26 2 complete error
ENUMERATION 27 3 left right stop
FLOAT_RANGE 28 0.0 1.0
ENUMERATION 29 2 complete error
ENUMERATION 30 2 complete error
SOAR_ID 31
ENUMERATION 32 1 wait
ENUMERATION 33 2 operator state
ENUMERATION 34 2 multiple none
ENUMERATION 35 1 elaboration
ENUMERATION 36 2 no-change tie
SOAR_ID 37
STRING 38
SOAR_ID 39
SOAR_ID 40
FLOAT_RANGE 41 0.0 Infinity
FLOAT_RANGE 42 0.0 Infinity
SOAR_ID 43
SOAR_ID 44
ENUMERATION 45 3 backward forward stop
SOAR_ID 46
SOAR_ID 47
ENUMERATION 48 3 left right stop
SOAR_ID 49
SOAR_ID 50
STRING 51
SOAR_ID 52
FLOAT_RANGE 53 0.0 Infinity
FLOAT_RANGE 54 0.0 1.0
FLOAT_RANGE 55 -Infinity Infinity
FLOAT_RANGE 56 0.0 1.0
FLOAT_RANGE 57 0.0 1.0
60
0 action-interval-time 55
0 attribute 33
0 choices 34
0 impasse 36
0 io 1
0 item 37
0 name 7
0 operator 4
0 operator 9
0 operator 31
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
2 time 20
3 move 21
3 rotate 22
3 stop 23
4 name 8
9 actions 39
9 name 10
11 future-time 41
11 io 1
11 name 13
11 next-action-time 42
11 operator 15
11 operator 17
11 operator 19
11 superoperator 52
11 superstate 0
11 top-state 0
11 type 12
15 actions 43
15 name 14
17 actions 46
17 name 16
19 actions 49
19 name 18
21 direction 24
21 status 26
21 throttle 25
22 direction 27
22 status 29
22 throttle 28
23 status 30
31 name 32
31 random 35
37 name 38
39 dummy 40
40 dummy 51
40 throttle 56
43 move 44
44 direction 45
44 throttle 54
46 rotate 47
47 direction 48
47 throttle 57
49 stop 50
52 next-action-time 53
