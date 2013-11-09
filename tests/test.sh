#!/bin/bash


##################### tests on correct input ##################
for i in {1..25}
do
    if [ ! -f "tests/input${i}.fsp" ]; then
	echo "error: tests/input${i}.fsp not found"
	exit 255
    fi
    ./fspcc -i tests/input${i}.fsp -o tests/new-output${i}.lts
    diff tests/output${i}.lts tests/new-output${i}.lts > /dev/null
    var=$?
    if [ "$var" != "0" ]; then
	echo ""
	echo "Test FAILED on input${i}.fsp"
	exit 1
    fi
    ./fspcc -l tests/new-output${i}.lts
    var=$?
    if [ "$var" != "0" ]; then
        echo ""
        echo "Test FAILED on input${i}.fsp, while reopening the compiled output."
        exit 1
    fi
    rm tests/new-output${i}.lts
done


##################### tests on invalid input ##################
for i in {1..2}
do
    if [ ! -f "tests/err-input${i}.fsp" ]; then
	echo "error: tests/err-input${i}.fsp not found"
	exit 255
    fi
    ./fspcc -i tests/err-input${i}.fsp -o tests/new-output.lts &> /dev/null
    var=$?
    if [ "$var" == "0" ]; then
	echo ""
	echo "Test FAILED on err-input${i}.fsp"
	exit 1
    fi
    if [ -f "tests/new-output.lts" ]; then
	rm tests/new-output.lts
    fi
done


echo ""
echo "Test OK"
