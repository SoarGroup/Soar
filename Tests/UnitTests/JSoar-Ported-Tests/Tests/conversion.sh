#! /bin/bash
for f in *.soar
do
	sed -i '' 's/(succeeded)/(exec succeeded)/g' $f
	sed -i '' 's/(failed)/(exec failed)/g' $f
	sed -i '' 's/(halt)/(exec halt)/g' $f
done
