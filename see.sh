#!/bin/bash

set -x

dot -Tpng $1 -o draw.png
xv draw.png &
