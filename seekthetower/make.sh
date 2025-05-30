#!/bin/bash

set -e

echo "Updating build number"

python - graphics.cpp <<EOF
import sys, re
with open(sys.argv[1], 'r') as f:
   content = f.read()
match = re.search(r'tinyfont\.print\(F\("v(\d+)"\)\);', content)
if match:
   number = int(match.group(1))
   new_line = f'tinyfont.print(F("v{number+1}") );'
   new_content = content.replace(match.group(0), new_line)
   with open(sys.argv[1], 'w') as f:
      f.write(new_content)
EOF

echo "Generating fxdata"
ardugotools fxdata generate

DEVICE="${1:-fx}"

echo "Building for $DEVICE"
arduino-cli compile --fqbn "arduboy-homemade:avr:arduboy-$DEVICE" --optimize-for-debug --output-dir dist --build-path build
