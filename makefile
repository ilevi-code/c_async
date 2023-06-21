CFLAGS := -Wall -Werror -g

.PHONY: clean

main: main.o foo.o

clean:
	$(RM) *.o main
