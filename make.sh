#!/bin/bash
set -e

g++ -std=c++23 -o osqa -fno-rtti -fno-exceptions -Wfatal-errors -Werror -Wall -Wextra -Wpedantic -Wno-unused-parameter src/main.cpp
echo
ls -la --color osqa
echo
