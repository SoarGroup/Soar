31
SOAR_ID 0
ENUMERATION 1 1 state
ENUMERATION 2 1 nil
ENUMERATION 3 1 water-jug
SOAR_ID 4
ENUMERATION 5 1 initialize-water-jug
SOAR_ID 6
ENUMERATION 7 1 empty
SOAR_ID 8
SOAR_ID 9
INTEGER_RANGE 10 -2147483648 2147483647
INTEGER_RANGE 11 -2147483648 2147483647
SOAR_ID 12
ENUMERATION 13 1 fill
INTEGER_RANGE 14 -2147483648 2147483647
SOAR_ID 15
ENUMERATION 16 1 record
SOAR_ID 17
ENUMERATION 18 1 state
ENUMERATION 19 1 Impasse__Operator_Tie
ENUMERATION 20 1 evaluate-operator
SOAR_ID 21
ENUMERATION 22 1 pour
SOAR_ID 23
ENUMERATION 24 3 empty fill pour
SOAR_ID 25
ENUMERATION 26 3 best equal worst
ENUMERATION 27 1 tie
ENUMERATION 28 1 create-preferences
SOAR_ID 29
ENUMERATION 30 4 empty fill none pour
40
0 jug 9
0 last-operator 30
0 name 3
0 operator 4
0 operator 12
0 operator 15
0 operator 6
0 operator 8
0 superstate 2
0 type 1
4 name 5
6 empty-jug 9
6 name 7
8 empty-jug 9
8 fill-jug 9
8 name 22
9 contents 10
9 empty 11
9 volume 14
12 fill-jug 9
12 name 13
15 name 16
17 evaluation 25
17 impasse 27
17 item 23
17 last-operator 30
17 name 19
17 operator 21
17 operator 29
17 superstate 0
17 top-state 0
17 type 18
21 evaluation 25
21 name 20
23 empty-jug 9
23 fill-jug 9
23 name 24
25 super-operator 23
25 value 26
29 name 28
