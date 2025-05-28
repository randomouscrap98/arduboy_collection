#!/bin/bash

./make.sh mini
ardugotools sketch write --infile dist/seekthetower.ino.hex
ardugotools flashcart writedev --infile fxdata/fxdata_dev.bin
