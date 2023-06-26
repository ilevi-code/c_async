CFLAGS := -Wall -Werror -g
ASFLAGS := -g

.PHONY: clean

main: main.o foo.o

clean:
	$(RM) *.o main
