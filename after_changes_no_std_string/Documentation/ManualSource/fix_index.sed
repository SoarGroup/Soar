# sed script to put \verb+*+ around all characters in the manual.ind file that
# latex doesn't like, and also replace ^ with \carat
# jzxu 8/22/2008

s/\\item \(&\|<\|<< >>\|<=\|<=>\|<>\|>\|>=\|~\)/\\item \\verb\+\1\+/
s/\\item \^/\\item \\carat/
