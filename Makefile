.PHONY: all clean run debug

DIRECTORY_GUARD=@mkdir -p $(@D)

CWD=${shell pwd}
OUTDIR=$(CWD)/build
SRCDIR=$(CWD)/src

BINARY=$(OUTDIR)/hcc

LDFLAGS:=-O3 -g

CFLAGS:=-g -Werror -Wall -Wextra -Wno-unused-parameter

HCC_SOURCES=$(wildcard $(SRCDIR)/*.c)
HCC_OBJECTS=$(patsubst $(SRCDIR)/%,$(OUTDIR)/%.o,$(HCC_SOURCES))

all: $(BINARY)

run: $(BINARY)
	$(BINARY) $(CWD)/tests/write_a_c_compiler/stage_5/valid/assign.c -vv

test: $(BINARY)
	@cd tests/write_a_c_compiler/; \
		./test_compiler.sh $(BINARY) 5

test-all: $(BINARY)
	@cd tests/write_a_c_compiler/; \
		./test_compiler.sh $(BINARY)

clean:
	@rm -r $(OUTDIR)

$(BINARY): $(HCC_OBJECTS)
	$(DIRECTORY_GUARD)
	@echo [LD] $@
	@gcc $(LDFLAGS) $^ -o $@

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c
	$(DIRECTORY_GUARD)
	@echo [CC] $@
	@gcc -c $(CFLAGS) -o $@ $<

-include $(HCC_OBJECTS:.o=.d)
