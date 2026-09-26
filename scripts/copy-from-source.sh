#!/bin/sh
set -e
cd $(dirname "$0")

cd ..
cp -rav ../tang-nano-9k--riscv--cache-psram/emulator/src/* src
