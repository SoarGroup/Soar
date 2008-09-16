97
SOAR_ID 0
SOAR_ID 1
SOAR_ID 2
SOAR_ID 3
SOAR_ID 4
ENUMERATION 5 1 state
ENUMERATION 6 1 nil
ENUMERATION 7 1 arithmetic
ENUMERATION 8 1 initialize-arithmetic
SOAR_ID 9
ENUMERATION 10 1 process-column
SOAR_ID 11
ENUMERATION 12 1 next-column
SOAR_ID 13
SOAR_ID 14
SOAR_ID 15
ENUMERATION 16 1 finish-problem
INTEGER_RANGE 17 -2147483648 2147483647
INTEGER_RANGE 18 -2147483648 2147483647
INTEGER_RANGE 19 -2147483648 2147483647
SOAR_ID 20
INTEGER_RANGE 21 0 18
INTEGER_RANGE 22 0 1
INTEGER_RANGE 23 0 1
SOAR_ID 24
ENUMERATION 25 1 state
ENUMERATION 26 2 get-digit1 process-column
ENUMERATION 27 1 compute-result
SOAR_ID 28
ENUMERATION 29 1 carry-borrow
SOAR_ID 30
ENUMERATION 31 1 get-digit1
SOAR_ID 32
ENUMERATION 33 1 get-digit2
SOAR_ID 34
INTEGER_RANGE 35 0 9
INTEGER_RANGE 36 0 1
ENUMERATION 37 1 write-result
SOAR_ID 38
SOAR_ID 39
SOAR_ID 40
ENUMERATION 41 1 state
ENUMERATION 42 1 get-digit1
ENUMERATION 43 1 write-digit1
SOAR_ID 44
SOAR_ID 45
ENUMERATION 46 1 state
ENUMERATION 47 1 carry-borrow
ENUMERATION 48 1 new-column
SOAR_ID 49
SOAR_ID 50
ENUMERATION 51 1 generate-facts
ENUMERATION 52 2 addition subtraction
INTEGER_RANGE 53 0 9
ENUMERATION 54 1 compute-result
SOAR_ID 55
ENUMERATION 56 2 |+| |-|
SOAR_ID 57
ENUMERATION 58 1 generate-problem
SOAR_ID 59
ENUMERATION 60 1 state
ENUMERATION 61 1 generate-problem
ENUMERATION 62 1 generate-operation
SOAR_ID 63
ENUMERATION 64 1 finish-problem-generation
SOAR_ID 65
ENUMERATION 66 1 next-column
SOAR_ID 67
ENUMERATION 68 1 generate-digit1
SOAR_ID 69
ENUMERATION 70 1 generate-digit2
SOAR_ID 71
INTEGER_RANGE 72 0 9
INTEGER_RANGE 73 0 9
ENUMERATION 74 1 nil
ENUMERATION 75 1 t
ENUMERATION 76 1 nil
ENUMERATION 77 1 t
INTEGER_RANGE 78 0 2147483647
SOAR_ID 79
ENUMERATION 80 1 stop-arithmetic
INTEGER_RANGE 81 -2147483648 2147483647
INTEGER_RANGE 82 -2147483648 2147483647
INTEGER_RANGE 83 -2147483648 2147483647
INTEGER_RANGE 84 -2147483648 2147483647
SOAR_ID 85
ENUMERATION 86 1 state
ENUMERATION 87 1 compute-result
ENUMERATION 88 1 borrow
SOAR_ID 89
ENUMERATION 90 1 add-ten
SOAR_ID 91
ENUMERATION 92 1 compute-result
SOAR_ID 93
SOAR_ID 94
INTEGER_RANGE 95 -2147483648 2147483647
INTEGER_RANGE 96 -2147483648 2147483647
136
0 arithmetic 39
0 arithmetic-problem 13
0 count 78
0 io 1
0 name 7
0 operator 4
0 operator 9
0 operator 11
0 operator 15
0 operator 50
0 operator 57
0 operator 79
0 superstate 6
0 top-state 0
0 type 5
1 input-link 2
1 output-link 3
4 name 8
9 name 10
11 name 12
13 bottom-number 82
13 computed-result 84
13 current-column 14
13 one-column 14
13 operation 52
13 operation-symbol 56
13 result-number 83
13 top-number 81
14 carry-borrow 23
14 column 77
14 digit1 17
14 digit2 18
14 new-digit1 17
14 next-column 14
14 next-column 76
14 result 19
15 count 78
15 name 16
20 carry-borrow 22
20 digit1 21
20 digit2 21
20 operation 52
20 result 53
20 sum 21
24 arithmetic 39
24 arithmetic-problem 13
24 carry-borrow 36
24 current-column 14
24 digit1 35
24 digit2 35
24 name 26
24 operator 28
24 operator 30
24 operator 32
24 operator 34
24 operator 38
24 query 20
24 result 35
24 superstate 0
24 top-state 0
24 type 25
28 name 27
30 name 29
32 name 31
34 name 33
38 name 37
39 add10-facts 94
39 facts 20
39 subtraction-facts 20
40 arithmetic 39
40 arithmetic-problem 13
40 carry-borrow 36
40 current-column 14
40 digit1 35
40 digit2 35
40 name 42
40 operator 44
40 operator 55
40 result 35
40 superstate 24
40 top-state 0
40 type 41
44 name 43
45 current-column 14
45 name 47
45 operator 49
45 superstate 24
45 top-state 0
45 type 46
49 name 48
50 name 51
55 name 54
57 name 58
59 current-column 14
59 current-column 74
59 digits 73
59 name 61
59 operator 63
59 operator 65
59 operator 67
59 operator 69
59 operator 71
59 problem 13
59 quiescence 75
59 superstate 0
59 top-state 0
59 type 60
63 name 62
63 operation 52
63 operation-symbol 56
65 name 64
67 name 66
69 digit1 72
69 name 68
71 digit2 72
71 name 70
79 name 80
85 arithmetic 39
85 arithmetic-problem 13
85 current-column 14
85 digit1 35
85 digit2 35
85 name 87
85 operator 89
85 operator 91
85 operator 93
85 prior-column 14
85 result 35
85 superstate 24
85 top-state 0
85 type 86
89 name 88
91 name 90
93 name 92
94 digit-10 96
94 digit1 95
