CFLAGS = -Wall -mmcu=attiny85 -DF_CPU=1000000 -Os
OBJECTS = lamppwm.o
CC = avr-gcc
P = lamppwm.hex

$(P): $(OBJECTS)
	avr-objcopy -Oihex $(OBJECTS) $(P)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $<
