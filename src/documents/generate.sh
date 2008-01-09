#! /bin/bash

node="\"field\":{\"label\":\"value\", \"number\": 123, \"status\": true }";
if [ -n "$1" ]
then
	limit=$1
else
	limit=1
fi

echo "\"root\":{"

for ((a=1; a < $limit ; a++))
do
	echo "$node,"
done

echo "$node"
echo "}"
exit 0

