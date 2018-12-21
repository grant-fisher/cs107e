NAME = main
# Add any modules for which you want to use your own code for assign7, rest will be drawn from library
MY_MODULES = draw.o axis.o ringbufferProject.o graphics.o pen.o mouse.o keyboard.o gprof.o timer.o gpio.o strings.o printf.o backtrace.o malloc.o shell.o fb.o gl.o console.o

# This is the list of modules for building libmypi.a
LIMYBPI_MODULES = draw.o axis.o ringbufferProject.o graphics.o pen.o mouse.o timer.o gpio.o strings.o printf.o backtrace.o malloc.o keyboard.o shell.o fb.o gl.o console.o

CS107E = ../cs107e.github.io/cs107e

CFLAGS  = -I$(CS107E)/include -g -Wall -Wpointer-arith 
CFLAGS += -Og -std=c99 -ffreestanding 
CFLAGS += -mapcs-frame -fno-omit-frame-pointer -mpoke-function-name
LDFLAGS = -nostdlib -T memmap -L. -L$(CS107E)/lib 
LDLIBS  = -lc_pi -lpiextra -lpi -lc_pi -lm -lgcc

all : $(NAME).bin $(MY_MODULES)

%.bin: %.elf
	arm-none-eabi-objcopy $< -O binary $@

%.elf: %.o start.o cstart.o $(MY_MODULES)
	arm-none-eabi-gcc $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	arm-none-eabi-gcc $(CFLAGS) -c $< -o $@

%.o: %.s
	arm-none-eabi-as $(ASFLAGS) $< -o $@

libmypi.a: $(LIMYBPI_MODULES) Makefile
	rm -f $@
	arm-none-eabi-ar cDr $@ $(filter %.o,$^)

%.list: %.o
	arm-none-eabi-objdump  --no-show-raw-insn -d $< > $@

install: $(NAME).bin
	rpi-install.py -p $<

test: test.bin
	rpi-install.py -p $<

bonus: $(NAME)-bonus.bin
	rpi-install.py -p $<

# Note: link is now against local libmypi first
%-bonus.elf: %.o start.o cstart.o libmypi.a 
	arm-none-eabi-gcc $(LDFLAGS) $(filter %.o,$^) -lmypi $(LDLIBS) -o $@

clean:
	rm -f *.o *.bin *.elf *.list *~ libmypi.a

.PHONY: all clean install test bonus

.PRECIOUS: %.o %.elf %.a

%:%.o
%:%.c
