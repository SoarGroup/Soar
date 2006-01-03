ILRoot ^holds <ID> holdsID
holdsID ^on <str> -start "A" -update fooTowerName -freq onchange
holdsID ^above <str int> -start "none" -update 4 -freq onchange
-for counter 1 12
holdsID ^disk <int> -start counter
-for apples 1 6
-class apple
ILRoot ^apple <ID> appleID
appleID ^color <str> -start "red"
appleID ^radius <flt> -start 4.25
-end apple
