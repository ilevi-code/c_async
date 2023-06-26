CFLAGS := -Wall -Werror -g
ASFLAGS := -g

.PHONY: clean

main: main.o generator.o generator_bits.o tests.o

clean:
	$(RM) *.o main
