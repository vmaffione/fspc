#!/bin/bash

set -x

rm *.gv *.png
./fspc
dot -Tpng *.gv -o draw.png
xv draw.png
