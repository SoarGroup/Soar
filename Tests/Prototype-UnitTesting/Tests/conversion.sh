#! /bin/bash

find . -type f -name "*.soar" -exec sed -i '' 's/(succeeded)/(exec succeeded)/g' {} \;
find . -type f -name "*.soar" -exec sed -i '' 's/(failed)/(exec failed)/g' {} \;
find . -type f -name "*.soar" -exec sed -i '' 's/(halt)/(exec halt)/g' {} \;
