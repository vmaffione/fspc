#!/bin/bash


for i in {1..20}
do
    ./fspc -i tests/input${i}.fsp -o tests/new-output${i}.lts
    diff tests/output${i}.lts tests/new-output${i}.lts > /dev/null
    var=$?
    if [ "$var" != "0" ]; then
	echo ""
	echo "Test FAILED on input${i}.fsp"
	exit
    fi
    rm tests/new-output${i}.lts
done

echo ""
echo "Test OK"
