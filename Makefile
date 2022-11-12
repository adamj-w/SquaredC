.PHONY: all clean run debug

DIRECTORY_GUARD=@mkdir -p $(@D)

CWD=${shell pwd}
OUTDIR=$(CWD)/build
SRCDIR=$(CWD)/src

BINARY=$(OUTDIR)/cxc

LDFLAGS:=-g # dev flags
LDFLAGS:=$(LDFLAGS) -O3 # prod flags

CFLAGS:=-g -Werror -Wall -Wpedantic -Wextra -Wno-unused-parameter -fanalyzer # all dev flags
CFLAGS:=$(CFLAGS) -MMD # all prod flags

CXC_SOURCES=$(wildcard $(SRCDIR)/*.c)
CXC_OBJECTS=$(patsubst $(SRCDIR)/%,$(OUTDIR)/%.o,$(CXC_SOURCES))

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

$(BINARY): $(CXC_OBJECTS)
	$(DIRECTORY_GUARD)
	@echo [LD] $@
	@gcc $(LDFLAGS) $^ -o $@

$(OUTDIR)/%.c.o: $(SRCDIR)/%.c
	$(DIRECTORY_GUARD)
	@echo [CC] $@
	@gcc -c $(CFLAGS) -o $@ $<

-include $(CXC_OBJECTS:.o=.d)
