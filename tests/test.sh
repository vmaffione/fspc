#!/bin/bash


if [ -n "$1" ]; then
    # Use a different fspcc command line invokation
    FSPC="$1"
else
    FSPC="./fspcc"
fi

##################### tests on correct input ##################
TESTDIR="tests/blackbox"

cat > deadlock.fsh << EOF
n = safety
exit n
EOF

for i in {1..29}
do
    if [ ! -f "${TESTDIR}/input${i}.fsp" ]; then
	echo "error: ${TESTDIR}/input${i}.fsp not found"
	exit 255
    fi
    ${FSPC} -i ${TESTDIR}/input${i}.fsp -o ${TESTDIR}/new-output${i}.lts
    diff ${TESTDIR}/output${i}.lts ${TESTDIR}/new-output${i}.lts > /dev/null
    var=$?
    if [ "$var" != "0" ]; then
	echo ""
	echo "Test FAILED on ${TESTDIR}/input${i}.fsp"
	exit 1
    fi
    ${FSPC} -l ${TESTDIR}/new-output${i}.lts -o /dev/null
    var=$?
    if [ "$var" != "0" ]; then
        echo ""
        echo "Test FAILED on ${TESTDIR}/input${i}.fsp, while reopening the compiled output."
        exit 1
    fi
    rm ${TESTDIR}/new-output${i}.lts
    echo "${TESTDIR}/input$i ok"

    # How many deadlock are there here? Check that the reported deadlocks
    # are consistent with the file "deadlocks"
    ${FSPC} -i ${TESTDIR}/input${i}.fsp -S deadlock.fsh > /dev/null
    NUM_DEADLOCKS=$?
    grep "^${i}$" ${TESTDIR}/deadlocks > /dev/null
    EXPECT_NO_DEADLOCKS=$?
    if [[ $NUM_DEADLOCKS == "0" && ${EXPECT_NO_DEADLOCKS} == "0" ]]; then
            echo "Test FAILED on ${TESTDIR}/input${i}.fsp: Deadlock expected but no deadlock reported."
            exit 1
    elif [[ $NUM_DEADLOCKS != "0" && ${EXPECT_NO_DEADLOCKS} != "0" ]]; then
            echo "Test FAILED on ${TESTDIR}/input${i}.fsp: Unexpected deadlock reported."
            exit 1
    fi
done

rm deadlock.fsh


##################### tests on invalid input ##################
TESTDIR="tests/error"
for i in {1..2}
do
    if [ ! -f "${TESTDIR}/err-input${i}.fsp" ]; then
	echo "error: ${TESTDIR}/err-input${i}.fsp not found"
	exit 255
    fi
    ${FSPC} -i ${TESTDIR}/err-input${i}.fsp -o ${TESTDIR}/new-output.lts &> /dev/null
    var=$?
    if [ "$var" == "0" ]; then
	echo ""
	echo "Test FAILED on ${TESTDIR}/err-input${i}.fsp"
	exit 1
    fi
    if [ -f "${TESTDIR}/new-output.lts" ]; then
	rm ${TESTDIR}/new-output.lts
    fi
    echo "${TESTDIR}/err-input$i ok"
done


##################### tests with scripts #####################
TESTDIR="tests/scripts"
for i in {1..4}
do
    if [ ! -f "${TESTDIR}/input${i}.fsp" ]; then
	echo "error: ${TESTDIR}/input${i}.fsp not found"
	exit 255
    fi
    ${FSPC} -i ${TESTDIR}/input${i}.fsp -S ${TESTDIR}/script${i}.fsh -o ${TESTDIR}/new-output${i}.lts
    diff ${TESTDIR}/output${i}.lts ${TESTDIR}/new-output${i}.lts > /dev/null
    var=$?
    if [ "$var" != "0" ]; then
	echo ""
	echo "Test FAILED on ${TESTDIR}/input${i}.fsp"
	exit 1
    fi
    ${FSPC} -l ${TESTDIR}/new-output${i}.lts -o /dev/null
    var=$?
    if [ "$var" != "0" ]; then
        echo ""
        echo "Test FAILED on ${TESTDIR}/input${i}.fsp, while reopening the compiled output."
        exit 1
    fi
    rm ${TESTDIR}/new-output${i}.lts
    echo "${TESTDIR}/input$i ok"
done


echo ""
echo "Test OK"
