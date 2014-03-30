#!/bin/bash


# This function is used in the blackbox tests (tests on correct input)
# to cover safety/progress violation algorithms. It checks that test
# cases with reported problems corresponds to what stored in the
# "expect" list.
match_expected_problems()
{
    i=$1                        # test index
    FSPC=$2                     # fspc executable
    TESTDIR=$3                  # test directory
    EXPECT_LIST=${TESTDIR}/$4   # list of positive testcase indexes
    PROBLEM=$4                  # the type of problem to check

    ${FSPC} -i ${TESTDIR}/input${i}.fsp -S ${PROBLEM}.fsh > /dev/null
    NUM_PROBLEMS=$?

    grep "^${i}$" ${EXPECT_LIST} > /dev/null
    EXPECT_NO_PROBLEMS=$?
    if [[ $NUM_PROBLEMS == "0" && ${EXPECT_NO_PROBLEMS} == "0" ]]; then
            echo "Test FAILED on ${TESTDIR}/input${i}.fsp: ${PROBLEM} expected but no ${PROBLEM} reported."
            exit 1
    elif [[ $NUM_PROBLEMS != "0" && ${EXPECT_NO_PROBLEMS} != "0" ]]; then
            echo "Test FAILED on ${TESTDIR}/input${i}.fsp: Unexpected ${PROBLEM} reported."
            exit 1
    fi
}


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

cat > progress_violation.fsh << EOF
n = progress
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

    # Are there any deadlocks here? Check that the reported deadlocks
    # are consistent with the file "deadlock"
    match_expected_problems ${i} ${FSPC} ${TESTDIR} deadlock

    # Are there any progress violations here? Check that the reported
    # violations are consistent with the file "progress_violation"
    match_expected_problems ${i} ${FSPC} ${TESTDIR} progress_violation
done

rm deadlock.fsh progress_violation.fsh


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
for i in {1..5}
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


################# test alphabet and label compression #################
TESTDIR="tests/alpha"

cat > alpha.fsh << EOF
alpha P
EOF

for i in {1..2}
do
    if [ ! -f "${TESTDIR}/input${i}.fsp" ]; then
	echo "error: ${TESTDIR}/input${i}.fsp not found"
	exit 255
    fi
    ${FSPC} -i ${TESTDIR}/input${i}.fsp -S alpha.fsh > new-output
    diff ${TESTDIR}/output${i} new-output > /dev/null
    var=$?
    if [ "$var" != "0" ]; then
	echo ""
	echo "Test FAILED on ${TESTDIR}/input${i}.fsp"
	exit 1
    fi
    rm new-output
    echo "${TESTDIR}/input$i ok"
done

rm alpha.fsh


echo ""
echo "Test OK"
