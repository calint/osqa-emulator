#!/bin/sh
set -e
cd $(dirname "$0")

echo " * build emulator"
./make.sh
echo " * test emulator"
qa-emulator/test.sh
echo " * test firmware"
qa-firmware/test.sh
