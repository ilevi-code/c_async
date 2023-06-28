SRCDIR := src
OBJDIR := obj
INCDIR := inc
BINDIR := bin

SRCS := $(basename $(notdir $(wildcard $(SRCDIR)/*)))
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(SRCS))
$(info $(OBJS))

CPPFLAGS += -I$(INCDIR)
CFLAGS := -Wall -Werror -g
ASFLAGS := -g

.PHONY: all clean

all: $(BINDIR)/main

clean:
	$(RM) $(BINDIR)/main $(OBJS)

$(BINDIR)/main: $(OBJS) | $(BINDIR)
	$(LINK.o) $(OUTPUT_OPTION) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.S | $(OBJDIR)
	$(COMPILE.S) $(OUTPUT_OPTION) $^

$(BINDIR) $(OBJDIR):
	mkdir -p $@
