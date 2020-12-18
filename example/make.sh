#!/bin/sh -e

# avr-gcc++ path
# Replace by location of your avr-gcc compiler if different
BINPATH="/cygdrive/b/AVR_Toolchain/bin"
TARGETMCU=attiny412


echo Linking...
$BINPATH/avr-gcc  -Wall -mmcu=$TARGETMCU -o main.elf main.c 

echo Creating HEX file...
$BINPATH/avr-objcopy -O ihex "main.elf" "main.hex"

echo Creating LSS file...
$BINPATH/avr-objdump -h -S "main.elf" > "main.lss"