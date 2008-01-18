#! /bin/bash

node="\"fieldfield\":{\"labellabel\":\"valuevalue\", \"number\": 123456.456789e789012, \"statusstatus\":[true, false, null] }";
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

