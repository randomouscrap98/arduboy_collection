#!/bin/bash

BASEDIR=$(pwd)
FOLDER=runtime
mkdir -p $FOLDER
cd $FOLDER

ardens file=$BASEDIR/dist/seekthetower.ino.elf file=$BASEDIR/fxdata/fxdata_dev.bin >>ardens.log 2>&1 &

echo "Running in $FOLDER"
