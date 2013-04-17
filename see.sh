#!/bin/bash

set -x

rm *.gv *.png
./fspc
dot -Tpng $1.gv -o draw.png
xv draw.png
