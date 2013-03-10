PROJECT		= lamppwm
F_CPU 		= 1000000
PART		= attiny85
CFLAGS		= -Wall -mmcu=$(PART) -DF_CPU=$(F_CPU) -Os
OBJECTS		= $(PROJECT).o
CC		= avr-gcc
P 		= $(PROJECT).hex
OBJCOPY 	= avr-objcopy

all: $(P) pcintonly.hex sleepworks.hex

%.hex: %.o
	$(OBJCOPY) -Oihex $< $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<

install:
	avrdude -c buspirate -P $(CU) -p $(PART) -U flash:w:$(P)

binary: $(PROJECT).bin

$(PROJECT).bin: $(OBJECTS)
	$(OBJCOPY) -Obinary $(OBJECTS) $(PROJECT).bin

all: $(P) binary

size: $(OBJECTS)
	avr-size $(OBJECTS)
