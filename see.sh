#!/bin/bash

set -x

dot -Tpng $1.gv -o draw.png
xv draw.png &
