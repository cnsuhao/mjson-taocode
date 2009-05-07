#! /bin/bash

node="\"fieldfield\":{\"labellabel\":\"valuevalue\", \"number1\": 12.34e78, \"number2\": 12.34E-56, \"statusstatus\":[true, false, null, [true,false,null]] }";
if [ -n "$1" ]
then
	limit=$1
else
	limit=1
fi

echo "{"

for ((a=1; a < $limit ; a++))
do
	echo "$node,"
done

echo "$node"
echo "}"
exit 0

