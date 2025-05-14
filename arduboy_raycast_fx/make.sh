#!/bin/sh

arduino-cli compile --fqbn "arduboy-homemade:avr:arduboy-fx" --optimize-for-debug --output-dir dist --build-path build
