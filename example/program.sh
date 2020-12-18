#!/bin/sh -e

AVRDUDE="/cygdrive/z/pedro/Projects/avrdude/avrdude.exe"
PORT="COM3"
TARGETMCU="attiny412"

$AVRDUDE -c jtag2updi -P $PORT -p $TARGETMCU -U fl:w:main.hex