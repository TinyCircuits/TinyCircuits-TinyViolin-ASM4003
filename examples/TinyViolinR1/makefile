DEVICE     = attiny841
CLOCK      = 8000000
PROGRAMMER = -c usbasp
OBJECTS    = main.o
FUSES      = -U efuse:w:0xED:m
 
AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE) -B 5
COMPILE = avr-gcc -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)
 
all:	main.hex
 
.c.o:
	$(COMPILE) -c $< -o $@
 
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@
 
.c.s:
	$(COMPILE) -S $< -o $@
 
flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i
 
fuse:
	$(AVRDUDE) $(FUSES)
 
install: flash fuse
 
load: all
	bootloadHID main.hex
 
clean:
	rm -f main.hex main.elf $(OBJECTS)
 
main.elf: $(OBJECTS)
	$(COMPILE) -o main.elf $(OBJECTS)
 
main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
 
disasm:	main.elf
	avr-objdump -d main.elf
 
cpp:
	$(COMPILE) -E main.c