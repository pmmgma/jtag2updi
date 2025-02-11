
#PLATFORM=ATTINY1624
PLATFORM=ARDUINO

SRCSMAIN=jtag2updi.cpp
SRCSTEST=
SRCS=JICE_io.cpp  JTAG2.cpp  UPDI_hi_lvl.cpp  UPDI_lo_lvl.cpp  crc16.cpp  dbg.cpp  updi_io.cpp  updi_io_soft.cpp  sys.cpp
#SRCS+=serial.c
#SRCS+=serial.c pcint.c eeprom.c display.c 

TARGET=jtag2updi.hex

CFLAGS=-mmcu=$(MCU) -Wall -W -Os $(PLAT) -Werror -fmax-errors=5



PYTHONUNBUFFERED="1"

OBJMAIN=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCSMAIN)))
OBJTEST=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCSTEST)))
OBJS=$(patsubst %.c,%.o,$(patsubst %.cpp,%.o,$(SRCS)))
TARGET_ELF=$(patsubst %.hex,%.elf,$(TARGET))



ifeq ($(PLATFORM),ARDUINO) 
	AVRPORT = COM4
	MCU=atmega168p
	MCUP=ATMEGA168 
	PROGRAMMER = -c arduino -P $(AVRPORT) -b 19200
	PLAT  = -DARDUINO
	FUSES = -U hfuse:w:0x99:m -U lfuse:w:0xef:m
    F_CPU = 16000000U
    CFLAGS+=-DF_CPU=$(F_CPU)
endif




export PATH:=/cygdrive/b/avr-gcc-14.1.0-x64-windows/bin:/bin:/usr/bin

#export AVRDUDE = /cygdrive/z/pedro/Projects/avrdude/avrdude.exe
export AVRDUDE = /cygdrive/b/avr-gcc-14.1.0-x64-windows/bin/avrdude.exe
export CHECKINSERTER = /cygdrive/z/pedro/vc/CheckInserter/CheckInserter.exe
export MODULEPROGRAMMER = /cygdrive/z/pedro/Projects/Bat8051/Tests/Sw/ProtocolTestPC/Debug/ModuleProgrammer.exe


CC=avr-gcc.exe
CPP=avr-g++.exe
CC_LD=avr-g++.exe
OBJCOPY=avr-objcopy.exe
OBJDUMP=avr-objdump.exe
AVRSIZE=avr-size.exe
# optimize for size:





ifdef TESTING_SUPPORT
CPPFLAGS+=-DTESTING_SUPPORT
endif

ifdef TESTING_SUPPORT
all: .runallways $(TARGET) testing 
else
all: .runallways $(TARGET)
endif

.runallways:
	@printf '\033[32m%-*s\n' `tput cols` '' | tr ' ' - 
	@printf '%-*s\n' `tput cols` 'Compilation start here: ' | tr ' ' - 
	@printf '%-*s\033[0m\n' `tput cols` '' | tr ' ' - 



compile: $(TARGET)

download: $(TARGET)
	$(AVRDUDE) $(PROGRAMMER) -p $(MCUP) -v -U fl:w:$(TARGET)
	



ifneq ($(PLATFORM),ARDUINO) 
fuses:
	$(AVRDUDE) $(PROGRAMMER) -p $(MCUP) -v $(FUSES)
endif
	
reset:
	#echo $(PATH)
	$(AVRDUDE) $(PROGRAMMER) -p $(MCUP)
	
	
$(TARGET): $(TARGET_ELF) 
	$(OBJCOPY) -R .eeprom -R .debug -O ihex $(TARGET_ELF) $(TARGET) 
	$(AVRSIZE) $(TARGET_ELF)
	@echo " "
	@echo "Expl.: data=initialized data, bss=uninitialized data, text=code"
	@echo " "
	

$(TARGET_ELF) : $(OBJS) $(OBJMAIN)
	$(CC_LD) $(CFLAGS) -Wl,--gc-sections -o $@ -Wl,-Map,main.map $^
	$(OBJDUMP) -S -d $@ > main.S
	 
	
	
%.o: %.c 
	$(CC) $(CFLAGS) -ffunction-sections -I. -Os -o $@ -c $< 
	
    
%.o: %.cpp
	$(CPP) $(CFLAGS) -ffunction-sections -I. -Os -o $@ -c $< 
	

clean:
	echo "$(PATH)"
	rm *.hex *.o .depend
	
readback:
	$(AVRDUDE) $(PROGRAMMER) -p $(MCUP) -U fl:r:readback.hex:i
	

depend: .depend 

.depend: $(SRCS) $(SRCSMAIN) $(SRCSTEST) Makefile
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $(SRCS) $(SRCS) $(SRCSMAIN) $(SRCSTEST) >>./.depend
    
ifneq ($(MAKECMDGOALS),clean)
include .depend
endif