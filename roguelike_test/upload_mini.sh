#!/bin/bash

./make.sh mini
ardugotools sketch write --infile dist/roguelike_test.ino.hex
ardugotools flashcart writedev --infile fxdata/fxdata_dev.bin
