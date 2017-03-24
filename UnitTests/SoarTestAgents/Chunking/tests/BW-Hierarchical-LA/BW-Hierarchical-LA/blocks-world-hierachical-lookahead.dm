77
SOAR_ID 0
ENUMERATION 1 1 state
ENUMERATION 2 1 nil
ENUMERATION 3 2 blocks-world evaluate-operator
SOAR_ID 4
ENUMERATION 5 3 move-block pick-up put-down
SOAR_ID 6
SOAR_ID 7
SOAR_ID 8
SOAR_ID 9
ENUMERATION 10 3 A B C
ENUMERATION 11 5 A B C gripper table
ENUMERATION 12 2 block table
SOAR_ID 13
SOAR_ID 14
SOAR_ID 15
ENUMERATION 16 1 initialize-blocks-world
ENUMERATION 17 5 A B C gripper table
ENUMERATION 18 2 block table
SOAR_ID 19
ENUMERATION 20 1 state
ENUMERATION 21 1 move-block
ENUMERATION 22 1 pick-up
SOAR_ID 23
ENUMERATION 24 1 put-down
SOAR_ID 25
SOAR_ID 26
ENUMERATION 27 1 nothing
ENUMERATION 28 2 down up
ENUMERATION 29 2 no yes
SOAR_ID 30
ENUMERATION 31 1 state
ENUMERATION 32 2 pick-up put-down
ENUMERATION 33 1 open-gripper
SOAR_ID 34
ENUMERATION 35 1 close-gripper
SOAR_ID 36
ENUMERATION 37 1 move-gripper-up
SOAR_ID 38
ENUMERATION 39 1 move-gripper-down
SOAR_ID 40
ENUMERATION 41 1 move-gripper-above
SOAR_ID 42
ENUMERATION 43 1 gripper-operators
SOAR_ID 44
SOAR_ID 45
SOAR_ID 46
ENUMERATION 47 5 close down move-gripper-above open up
SOAR_ID 48
ENUMERATION 49 3 gripper move-blocks pick-up-put-down
ENUMERATION 50 1 yes
ENUMERATION 51 5 desired gripper ontop superstate top-state
ENUMERATION 52 1 object
ENUMERATION 53 4 clear inplace-object moveable top-state
SOAR_ID 54
ENUMERATION 55 1 state
ENUMERATION 56 1 Impasse__Operator_Tie
ENUMERATION 57 1 evaluate-operator
SOAR_ID 58
SOAR_ID 59
ENUMERATION 60 1 state
ENUMERATION 61 2 evaluate-operator move-block
ENUMERATION 62 1 move-block
SOAR_ID 63
SOAR_ID 64
ENUMERATION 65 1 state
ENUMERATION 66 1 Impasse__State_No-Change
ENUMERATION 67 1 tie
ENUMERATION 68 1 state
ENUMERATION 69 1 t
ENUMERATION 70 1 no-change
SOAR_ID 71
ENUMERATION 72 1 state
ENUMERATION 73 1 Impasse__State_No-Change
ENUMERATION 74 1 operator
SOAR_ID 75
ENUMERATION 76 2 exhaustion-failure partial-failure
124
0 clear 14
0 desired 7
0 gripper 26
0 inplace-object 14
0 io 44
0 last-moved-block 14
0 moveable 14
0 name 3
0 object 14
0 ontop 6
0 operator 4
0 operator 15
0 problem-space 48
0 success 7
0 superstate 2
0 superstate 0
0 top-state 0
0 type 1
4 destination 14
4 moving-block 14
4 name 5
6 bottom-block 9
6 last-bottom 9
6 top-block 8
7 ontop 13
8 name 10
9 name 11
9 type 12
13 bottom-block 14
13 top-block 14
14 name 17
14 type 18
15 name 16
19 attribute 74
19 desired 4
19 failure 4
19 impasse 70
19 name 21
19 operator 23
19 operator 25
19 problem-space 48
19 quiescence 69
19 success 4
19 superstate 0
19 top-state 0
19 type 20
23 moving-block 14
23 name 22
25 destination 14
25 name 24
26 above 14
26 holding 27
26 holding 14
26 open 29
26 position 28
30 desired 4
30 failure 4
30 name 32
30 operator 34
30 operator 36
30 operator 38
30 operator 40
30 operator 42
30 problem-space 48
30 state-type 43
30 success 4
30 superstate 19
30 top-state 0
30 type 31
34 name 33
36 name 35
38 name 37
40 name 39
42 destination 14
42 name 41
44 output-link 45
45 gripper 46
46 command 47
46 destination 14
48 default-state-copy 50
48 dont-copy 53
48 name 49
48 one-level-attributes 52
48 two-level-attributes 51
54 evaluation 75
54 impasse 67
54 name 56
54 operator 58
54 superstate 0
54 top-state 0
54 type 55
58 evaluation 75
58 name 57
59 attribute 74
59 clear 14
59 desired 4
59 failure 4
59 impasse 70
59 look-ahead-operator 63
59 name 61
59 ontop 6
59 operator 4
59 quiescence 69
59 success 4
59 superstate 54
59 top-state 0
59 tried-tied-operator 4
59 type 60
63 name 62
64 attribute 68
64 impasse 70
64 name 66
64 quiescence 69
64 superstate 59
64 top-state 0
64 type 65
71 attribute 68
71 impasse 70
71 name 73
71 quiescence 69
71 superstate 64
71 top-state 0
71 type 72
75 symbolic-value 76
