#!/bin/bash
set -e

g++ -std=c++23 -g -o emu -Wfatal-errors -Werror -Wall -Wextra -Wpedantic -Wno-unused-parameter src/main.cpp
echo
ls -la --color emu
echo
./emu firmware.bin
