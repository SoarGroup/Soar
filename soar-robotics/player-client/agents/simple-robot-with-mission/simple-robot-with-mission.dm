141
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 initialize-simple-robot
SOAR_ID 8
SOAR_ID 9
SOAR_ID 10
ENUMERATION 11 3 backward forward stop
FLOAT_RANGE 12 0.0 1.0
ENUMERATION 13 2 complete error
ENUMERATION 14 3 left right stop
FLOAT_RANGE 15 0.0 1.0
ENUMERATION 16 2 complete error
ENUMERATION 17 2 complete error
SOAR_ID 18
ENUMERATION 19 1 wait
ENUMERATION 20 1 elaboration
SOAR_ID 21
STRING 22
FLOAT_RANGE 23 -Infinity Infinity
SOAR_ID 24
INTEGER_RANGE 25 -2147483648 2147483647
SOAR_ID 26
SOAR_ID 27
FLOAT_RANGE 28 -Infinity Infinity
FLOAT_RANGE 29 -Infinity Infinity
SOAR_ID 30
SOAR_ID 31
FLOAT_RANGE 32 -Infinity Infinity
FLOAT_RANGE 33 -Infinity Infinity
SOAR_ID 34
FLOAT_RANGE 35 -Infinity Infinity
SOAR_ID 36
FLOAT_RANGE 37 -Infinity Infinity
FLOAT_RANGE 38 -Infinity Infinity
FLOAT_RANGE 39 0.0 1.0
SOAR_ID 40
ENUMERATION 41 1 go-to-waypoint
SOAR_ID 42
ENUMERATION 43 1 give-up-on-move-to
SOAR_ID 44
ENUMERATION 45 1 state
ENUMERATION 46 1 give-up-on-move-to
ENUMERATION 47 1 update-map
SOAR_ID 48
ENUMERATION 49 1 go-to-waypoint
SOAR_ID 50
SOAR_ID 51
INTEGER_RANGE 52 0 2147483647
INTEGER_RANGE 53 0 2147483647
ENUMERATION 54 1 true
ENUMERATION 55 1 true
ENUMERATION 56 1 true
FLOAT_RANGE 57 0.0 Infinity
SOAR_ID 58
ENUMERATION 59 1 state
ENUMERATION 60 2 evaluate-operator go-to-location
ENUMERATION 61 1 go-to-waypoint
SOAR_ID 62
SOAR_ID 63
ENUMERATION 64 1 go-to-location
ENUMERATION 65 1 yes
ENUMERATION 66 1 waypoints
SOAR_ID 67
ENUMERATION 68 1 state
ENUMERATION 69 1 selection
SOAR_ID 70
ENUMERATION 71 1 state
ENUMERATION 72 1 Impasse__Operator_Tie
SOAR_ID 73
FLOAT_RANGE 74 0.0 Infinity
SOAR_ID 75
ENUMERATION 76 1 evaluate-operator
SOAR_ID 77
ENUMERATION 78 1 execute-mission
SOAR_ID 79
ENUMERATION 80 1 state
ENUMERATION 81 1 execute-mission
ENUMERATION 82 1 start
SOAR_ID 83
ENUMERATION 84 1 go-to-location
SOAR_ID 85
SOAR_ID 86
ENUMERATION 87 1 state
ENUMERATION 88 1 go-to-location
SOAR_ID 89
SOAR_ID 90
SOAR_ID 91
ENUMERATION 92 1 complete
ENUMERATION 93 1 start
INTEGER_RANGE 94 0 2147483647
ENUMERATION 95 1 go-to-location
FLOAT_RANGE 96 -Infinity Infinity
FLOAT_RANGE 97 -Infinity Infinity
SOAR_ID 98
ENUMERATION 99 1 scout
ENUMERATION 100 1 nil
ENUMERATION 101 1 mission-operator
FLOAT_RANGE 102 -Infinity Infinity
FLOAT_RANGE 103 -Infinity Infinity
INTEGER_RANGE 104 0 2147483647
INTEGER_RANGE 105 -2147483648 2147483647
SOAR_ID 106
ENUMERATION 107 1 give-up-on-move-to
ENUMERATION 108 1 true
SOAR_ID 109
ENUMERATION 110 1 go-to-waypoint
ENUMERATION 111 1 true
SOAR_ID 112
ENUMERATION 113 1 update-progress
INTEGER_RANGE 114 0 2147483647
FLOAT_RANGE 115 0.0 Infinity
ENUMERATION 116 3 go-to-location selection simple-robot
ENUMERATION 117 1 none
ENUMERATION 118 1 state
SOAR_ID 119
SOAR_ID 120
STRING 121
INTEGER_RANGE 122 -2147483648 2147483647
SOAR_ID 123
INTEGER_RANGE 124 0 2147483647
INTEGER_RANGE 125 0 2147483647
SOAR_ID 126
SOAR_ID 127
STRING 128
STRING 129
SOAR_ID 130
STRING 131
SOAR_ID 132
STRING 133
SOAR_ID 134
STRING 135
SOAR_ID 136
STRING 137
SOAR_ID 138
INTEGER_RANGE 139 -2147483648 2147483647
STRING 140
177
0 attribute 118
0 choices 117
0 desired 31
0 io 1
0 item 21
0 mission 89
0 move-to 51
0 name 116
0 operator 4
0 operator 18
0 operator 77
0 replan-time 23
0 superstate 6
0 top-state 0
0 type 5
0 waypoints 30
1 input-link 2
1 output-link 3
2 self 26
2 time 24
3 broadcast-message 132
3 move 8
3 move-to 36
3 remove-message 138
3 rotate 9
3 stop 10
4 name 7
8 direction 11
8 status 13
8 throttle 12
9 direction 14
9 status 16
9 throttle 15
10 status 17
18 name 19
18 random 20
21 name 22
24 seconds 25
26 current-location 27
26 name 140
26 received-messages 119
27 x 28
27 y 29
30 at 31
30 waypoint 31
31 distance 35
31 next 34
31 x 32
31 y 33
36 throttle 39
36 x 37
36 y 38
40 destination 31
40 distance 74
40 name 41
40 track-progress 56
42 map-updated 55
42 move-to 51
42 name 43
44 name 46
44 operator 48
44 operator 50
44 superoperator 42
44 superstate 86
44 top-state 0
44 type 45
48 destination 31
48 name 47
48 source 31
50 destination 31
50 name 49
51 closest 57
51 cutoff 53
51 destination 31
51 give-up 54
51 source 31
51 time 52
58 failure 31
58 name 60
58 operator 62
58 superstate 67
58 superstate-set 0
58 top-state 0
58 tried-tied-operator 73
58 type 59
58 waypoints 30
62 destination 31
62 name 61
63 default-state-copy 65
63 name 64
63 two-level-attributes 66
67 item 40
67 name 69
67 operator 75
67 superstate 86
67 superstate-set 0
67 top-state 0
67 type 68
70 name 72
70 superstate 58
70 superstate-set 0
70 top-state 0
70 type 71
75 name 76
75 superoperator 40
77 mission 89
77 name 78
79 io 1
79 mission 89
79 name 81
79 operator 83
79 operator 85
79 superstate 0
79 top-state 0
79 type 80
79 waypoints 30
83 name 82
83 type 101
85 desired 31
85 name 84
85 x 102
85 y 103
86 desired 31
86 io 1
86 name 88
86 operator 106
86 operator 109
86 operator 112
86 problem-space 63
86 success 31
86 superstate 79
86 top-state 0
86 type 87
86 waypoints 30
89 current 90
89 first 90
90 delay-time 94
90 end-time 105
90 name 93
90 next 91
90 start-time 104
90 status 92
91 name 95
91 next 98
91 x 96
91 y 97
98 name 99
98 next 100
106 map-updated 108
106 move-to 51
106 name 107
109 destination 31
109 name 110
109 track-progress 111
112 distance 115
112 name 113
112 time 114
119 message 120
120 first 126
120 from 121
120 id 122
120 time 123
123 microseconds 125
123 seconds 124
126 next 127
126 word 128
127 next 130
127 word 129
130 next 127
130 word 131
132 next 134
132 word 133
134 next 136
134 word 135
136 next 134
136 word 137
138 id 139
