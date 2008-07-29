152
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
FLOAT_RANGE 34 -Infinity Infinity
SOAR_ID 35
FLOAT_RANGE 36 -Infinity Infinity
FLOAT_RANGE 37 -Infinity Infinity
FLOAT_RANGE 38 0.0 1.0
SOAR_ID 39
ENUMERATION 40 1 go-to-waypoint
SOAR_ID 41
ENUMERATION 42 1 give-up-on-move-to
SOAR_ID 43
ENUMERATION 44 1 state
ENUMERATION 45 1 give-up-on-move-to
ENUMERATION 46 1 update-map
SOAR_ID 47
ENUMERATION 48 1 go-to-waypoint
SOAR_ID 49
SOAR_ID 50
INTEGER_RANGE 51 0 2147483647
INTEGER_RANGE 52 0 2147483647
ENUMERATION 53 1 true
ENUMERATION 54 1 true
ENUMERATION 55 1 true
FLOAT_RANGE 56 0.0 Infinity
SOAR_ID 57
ENUMERATION 58 1 state
ENUMERATION 59 2 evaluate-operator go-to-location
ENUMERATION 60 1 go-to-waypoint
SOAR_ID 61
SOAR_ID 62
ENUMERATION 63 1 go-to-location
ENUMERATION 64 1 yes
ENUMERATION 65 1 waypoints
SOAR_ID 66
ENUMERATION 67 1 state
ENUMERATION 68 1 selection
SOAR_ID 69
ENUMERATION 70 1 state
ENUMERATION 71 1 Impasse__Operator_Tie
SOAR_ID 72
FLOAT_RANGE 73 0.0 Infinity
SOAR_ID 74
ENUMERATION 75 1 evaluate-operator
SOAR_ID 76
ENUMERATION 77 1 execute-mission
SOAR_ID 78
ENUMERATION 79 1 state
ENUMERATION 80 1 execute-mission
ENUMERATION 81 1 start
SOAR_ID 82
ENUMERATION 83 1 go-to-location
SOAR_ID 84
SOAR_ID 85
ENUMERATION 86 1 state
ENUMERATION 87 1 go-to-location
SOAR_ID 88
SOAR_ID 89
SOAR_ID 90
ENUMERATION 91 1 complete
ENUMERATION 92 1 start
INTEGER_RANGE 93 0 2147483647
ENUMERATION 94 1 go-to-location
FLOAT_RANGE 95 -Infinity Infinity
FLOAT_RANGE 96 -Infinity Infinity
SOAR_ID 97
ENUMERATION 98 1 scout
ENUMERATION 99 1 nil
ENUMERATION 100 1 mission-operator
FLOAT_RANGE 101 -Infinity Infinity
FLOAT_RANGE 102 -Infinity Infinity
INTEGER_RANGE 103 0 2147483647
INTEGER_RANGE 104 -2147483648 2147483647
SOAR_ID 105
ENUMERATION 106 1 give-up-on-move-to
ENUMERATION 107 1 true
SOAR_ID 108
ENUMERATION 109 1 go-to-waypoint
ENUMERATION 110 1 true
SOAR_ID 111
ENUMERATION 112 1 update-progress
INTEGER_RANGE 113 0 2147483647
FLOAT_RANGE 114 0.0 Infinity
ENUMERATION 115 3 go-to-location selection simple-robot
ENUMERATION 116 1 none
ENUMERATION 117 1 state
SOAR_ID 118
SOAR_ID 119
STRING 120
INTEGER_RANGE 121 -2147483648 2147483647
SOAR_ID 122
INTEGER_RANGE 123 0 2147483647
INTEGER_RANGE 124 0 2147483647
SOAR_ID 125
SOAR_ID 126
STRING 127
STRING 128
SOAR_ID 129
STRING 130
SOAR_ID 131
STRING 132
SOAR_ID 133
STRING 134
SOAR_ID 135
STRING 136
SOAR_ID 137
INTEGER_RANGE 138 -2147483648 2147483647
STRING 139
SOAR_ID 140
ENUMERATION 141 1 state
ENUMERATION 142 1 process-message
ENUMERATION 143 1 ignore-unknown-message
SOAR_ID 144
SOAR_ID 145
ENUMERATION 146 1 process-message
INTEGER_RANGE 147 -2147483648 2147483647
ENUMERATION 148 1 true
ENUMERATION 149 1 nil
ENUMERATION 150 1 nil
STRING 151
193
0 attribute 117
0 choices 116
0 desired 31
0 io 1
0 item 21
0 mission 88
0 move-to 50
0 name 115
0 operator 4
0 operator 18
0 operator 76
0 operator 145
0 replan-time 23
0 superstate 6
0 top-state 0
0 type 5
0 waypoints 30
1 input-link 2
1 output-link 3
2 self 26
2 time 24
3 broadcast-message 131
3 move 8
3 move-to 35
3 remove-message 137
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
26 name 139
26 received-messages 118
27 x 28
27 y 29
30 at 31
30 waypoint 31
31 distance 34
31 id 147
31 id 151
31 next 31
31 x 32
31 y 33
35 throttle 38
35 x 36
35 y 37
39 destination 31
39 distance 73
39 name 40
39 track-progress 55
41 map-updated 54
41 move-to 50
41 name 42
43 io 1
43 name 45
43 operator 47
43 operator 49
43 superoperator 41
43 superstate 85
43 top-state 0
43 type 44
47 destination 31
47 name 46
47 source 31
49 destination 31
49 name 48
50 closest 56
50 cutoff 52
50 destination 31
50 give-up 53
50 map-updated 148
50 source 31
50 time 51
57 failure 31
57 name 59
57 operator 61
57 superstate 66
57 superstate-set 0
57 top-state 0
57 tried-tied-operator 72
57 type 58
57 waypoints 30
61 destination 31
61 name 60
62 default-state-copy 64
62 name 63
62 two-level-attributes 65
66 item 39
66 name 68
66 operator 74
66 superstate 85
66 superstate-set 0
66 top-state 0
66 type 67
69 name 71
69 superstate 57
69 superstate-set 0
69 top-state 0
69 type 70
74 name 75
74 superoperator 39
76 mission 88
76 name 77
78 io 1
78 mission 88
78 name 80
78 operator 82
78 operator 84
78 superstate 0
78 top-state 0
78 type 79
78 waypoints 30
82 name 81
82 type 100
84 desired 31
84 name 83
84 x 101
84 y 102
85 desired 31
85 io 1
85 name 87
85 operator 105
85 operator 108
85 operator 111
85 problem-space 62
85 success 31
85 superstate 78
85 top-state 0
85 type 86
85 waypoints 30
88 current 89
88 first 89
89 delay-time 93
89 end-time 104
89 name 92
89 next 90
89 start-time 103
89 status 91
90 name 94
90 next 97
90 x 95
90 y 96
97 name 98
97 next 99
105 map-updated 107
105 move-to 50
105 name 106
108 destination 31
108 name 109
108 track-progress 110
111 distance 114
111 name 112
111 time 113
118 message 119
119 first 125
119 from 120
119 id 121
119 time 122
122 microseconds 124
122 seconds 123
125 next 126
125 word 127
126 next 129
126 word 128
129 next 126
129 word 130
131 next 133
131 next 149
131 word 132
133 next 135
133 next 150
133 word 134
135 next 133
135 word 136
137 id 138
140 io 1
140 name 142
140 operator 144
140 superstate 0
140 top-state 0
140 type 141
144 name 143
145 message 119
145 name 146
