SRCDIR := src
OBJDIR := obj
INCDIR := inc
BINDIR := bin
EXAMPLEDIR := examples

SRCS := $(basename $(notdir $(wildcard $(SRCDIR)/*)))
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(SRCS))
EXAMPLES = $(wildcard $(EXAMPLEDIR)/*)

CPPFLAGS += -I$(INCDIR)
CFLAGS := -Wall -Werror -g
ASFLAGS := -g

.PHONY: all clean

all: $(addprefix $(BINDIR)/,$(basename $(notdir $(EXAMPLES))))

clean:
	$(RM) $(BINDIR)/main $(OBJS)

check: $(BINDIR)/tests
	$<

$(BINDIR)/%: $(OBJDIR)/%.o $(OBJS) | $(BINDIR)
	$(LINK.o) $(OUTPUT_OPTION) $^

$(BINDIR)/main: $(OBJS) | $(BINDIR)
	$(LINK.o) $(OUTPUT_OPTION) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $^

$(OBJDIR)/%.o: $(EXAMPLEDIR)/%.c | $(OBJDIR)
	$(COMPILE.c) $(OUTPUT_OPTION) $^

$(OBJDIR)/%.o: $(SRCDIR)/%.S | $(OBJDIR)
	$(COMPILE.S) $(OUTPUT_OPTION) $^

$(BINDIR) $(OBJDIR):
	mkdir -p $@
