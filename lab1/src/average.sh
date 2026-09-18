#!/bin/sh

sum=0

for v in $@
do
sum=$(($sum + v))
done

result=$(echo "scale=3;$sum/$#" | bc)

echo "count: $#; avg: $result"