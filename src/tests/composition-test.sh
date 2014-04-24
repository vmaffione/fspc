#!/bin/bash


get_ms()
{
    echo $(($(date +%s%N)/1000000))
}


if [ -n "$1" ]; then
    # Use a different fspcc command line invokation
    FSPC="$1"
else
    FSPC="./fspcc"
fi

NAME="dining-philosophers.fsp"

if [ ! -f "tests/${NAME}" ]; then
    echo "error: tests/${NAME} not found"
    exit 255
fi

RESULTS=""

for i in 2 3 4 5 6 7 8 9 10
do
    cp tests/${NAME} comp-input.fsp
    sed -i "s/N=3/N=${i}/g" comp-input.fsp

    TSTART=$(get_ms)
    ${FSPC} -i comp-input.fsp -o /dev/null
    TEND=$(get_ms)

    DIFF=$(( $TEND - $TSTART ))
    echo "${i}: ${DIFF} ms"
    RESULTS="${RESULTS} ${i}-${DIFF}"

    rm comp-input.fsp
done

echo $RESULTS | python tests/lsq.py

echo ""
echo "Composition test completed"
