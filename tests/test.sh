#!/bin/bash


if [ -n "$1" ]; then
    # Use a different fspcc command line invokation
    FSPC="$1"
else
    FSPC="./fspcc"
fi

##################### tests on correct input ##################
for i in {1..25}
do
    if [ ! -f "tests/input${i}.fsp" ]; then
	echo "error: tests/input${i}.fsp not found"
	exit 255
    fi
    ${FSPC} -i tests/input${i}.fsp -o tests/new-output${i}.lts
    diff tests/output${i}.lts tests/new-output${i}.lts > /dev/null
    var=$?
    if [ "$var" != "0" ]; then
	echo ""
	echo "Test FAILED on input${i}.fsp"
	exit 1
    fi
    ${FSPC} -l tests/new-output${i}.lts -o /dev/null
    var=$?
    if [ "$var" != "0" ]; then
        echo ""
        echo "Test FAILED on input${i}.fsp, while reopening the compiled output."
        exit 1
    fi
    rm tests/new-output${i}.lts
    echo "input$i ok"
done


##################### tests on invalid input ##################
for i in {1..2}
do
    if [ ! -f "tests/err-input${i}.fsp" ]; then
	echo "error: tests/err-input${i}.fsp not found"
	exit 255
    fi
    ${FSPC} -i tests/err-input${i}.fsp -o tests/new-output.lts &> /dev/null
    var=$?
    if [ "$var" == "0" ]; then
	echo ""
	echo "Test FAILED on err-input${i}.fsp"
	exit 1
    fi
    if [ -f "tests/new-output.lts" ]; then
	rm tests/new-output.lts
    fi
    echo "err-input$i ok"
done


echo ""
echo "Test OK"
