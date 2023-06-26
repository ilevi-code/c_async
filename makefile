CFLAGS := -Wall -Werror -g
ASFLAGS := -g

.PHONY: clean

main: main.o generator.o generator_bits.o

clean:
	$(RM) *.o main
