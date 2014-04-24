#!/bin/bash

set -x

rm *.gv *.png
./fspc -g
dot -Tpng $1 -o draw.png
xv draw.png &
