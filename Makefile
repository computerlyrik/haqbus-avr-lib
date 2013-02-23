PROJECT=haqbus-lib

MCU = atmega88p
MCU_AVRDUDE = m88p


#full swing, bod 4,7
FUSE_SETTINGS = -U lfuse:w:0xd6:m -U hfuse:w:0xdc:m -U efuse:w:0xf9:m


ifeq ($(OSTYPE),)
OSTYPE      = $(shell uname)
endif
ifneq ($(findstring Darwin,$(OSTYPE)),)
USB_DEVICE = /dev/cu.SLAB_USBtoUART
else
USB_DEVICE = /dev/ttyUSB1
endif


#########################################################################

SRC=$(wildcard *.c)
OBJECTS=$(SRC:.c=.o) 
DFILES=$(SRC:.c=.d) 
HEADERS=$(wildcard *.h)



#  Compiler Options
GCFLAGS = -mmcu=$(MCU) -I. -I simavr/simavr/sim/avr -DF_CPU=20000000 -O2 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wstrict-prototypes  -std=gnu99 -MD -MP -g3 -gdwarf-2
#LDFLAGS =  -Wl,-Map=pwbl.map,--cref    -lm -Wl,--section-start=.text=0x1800
LDFLAGS = -mmcu=$(MCU) -Wl,--undefined=_mmcu,--section-start=.mmcu=0x910000

#  Compiler Paths
GCC = avr-gcc
REMOVE = rm -f
SIZE = avr-size
OBJCOPY = avr-objcopy

#########################################################################

all: $(PROJECT).hex Makefile stats

$(PROJECT).hex: $(PROJECT).elf Makefile
	$(OBJCOPY) -R .eeprom -O ihex $(PROJECT).elf $(PROJECT).hex 

$(PROJECT).elf: $(OBJECTS) Makefile
	$(GCC) $(LDFLAGS) $(OBJECTS) -o $(PROJECT).elf

stats: $(PROJECT).elf Makefile
	$(SIZE) $(PROJECT).elf

clean:
	$(REMOVE) $(OBJECTS)
	$(REMOVE) $(PROJECT).hex
	$(REMOVE) $(DFILES)
	$(REMOVE) $(PROJECT).elf
	$(REMOVE) *.vcd

sim:
	 run_avr -g -mcu $(MCU) -freq 20000000 $(PROJECT).elf

#########################################################################

%.o: %.c Makefile $(HEADERS)
	$(GCC) $(GCFLAGS) -o $@ -c $<

#########################################################################

flash: all
	avrdude -F -p $(MCU_AVRDUDE) -P usb -c dragon_isp    -U flash:w:$(PROJECT).hex
	#avrdude -p $(MCU_AVRDUDE) -c usbtiny    -U flash:w:$(PROJECT).hex

fuse:
	avrdude -F -p $(MCU_AVRDUDE) -P $(USB_DEVICE) -c stk500v2    $(FUSE_SETTINGS)
	#avrdude -p $(MCU_AVRDUDE) -c usbtiny    $(FUSE_SETTINGS)

