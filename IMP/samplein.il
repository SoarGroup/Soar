ILRoot ^holds <ID> holdsID
holdsID ^on <str> -start "A" -update fooTowerName -freq onchange
holdsID ^above <str int> -start "none" -update 4 -freq onchange
-for counter 1 12
holdsID ^disk <int> -start counter
