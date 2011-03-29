BEGIN {
	n = split("&,<,<< >>,<=,<=>,<>,>,>=,~", x, ",")
	for(i = 1; i <= n; i++) {
		weird[x[i]] = 1
	}
}
$1 == "\\item" {
	item = $2
	sub(",$", "", item)
	if (weird[item]) {
		$2 = "\\verb+" item "+,"
	} else if (item == "^") {
		$2 = "\\carat,"
	}
}
{ print }

