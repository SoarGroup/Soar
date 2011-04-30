BEGIN {
	n = split("&,<,<<,>>,<=,<=>,<>,>,>=,~", x, ",")
	for(i = 1; i <= n; i++) {
		weird[x[i]] = 1
	}
}
$1 == "\\item" {
	for (i = 2; i <= NF; ++i) {
		item = $i
		sub(",$", "", item)
		if (weird[item]) {
			$i = "\\verb+" item "+,"
		} else if (item == "^") {
			$i = "\\carat,"
		}
	}
}
{ print }

