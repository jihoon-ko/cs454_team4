
# output binary
BIN := run

# source files
SRCS := $(wildcard *.cpp algo/*.cpp)

# files included in the tarball generated by 'make dist' (e.g. add LICENSE file)
DISTFILES := $(BIN)

# filename of the tar archive generated by 'make dist'
DISTOUTPUT := $(BIN).tar.gz

# intermediate directory for generated object files
OBJDIR := .o
# intermediate directory for generated dependency files
DEPDIR := .d

# object files, auto generated from source files
OBJS := $(patsubst %,$(OBJDIR)/%.o,$(basename $(SRCS)))
# dependency files, auto generated from source files
DEPS := $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))

# compilers (at least gcc and clang) don't create the subdirectories automatically
$(shell mkdir -p $(dir $(OBJS)) >/dev/null)
$(shell mkdir -p $(dir $(DEPS)) >/dev/null)

# C compiler
CC := gcc
# C++ compiler
CXX := g++
# linker
LD := g++
# tar
TAR := tar

# C flags
CFLAGS := -std=c11
# C++ flags
CXXFLAGS := -std=c++14
# C/C++ flags
CPPFLAGS := -Wall -Wextra -pedantic -O2
# Debug flags
DEBUGFLAGS := -g -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wlogical-op -Wshift-overflow=2 -Wduplicated-cond -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -D_FORTIFY_SOURCE=2 -fstack-protector
# Release flags
RELEASEFLAGS := -DNDEBUG
# linker flags
LDFLAGS :=
# linker debug flags
LDDEBUGFLAGS :=
# flags required for dependency generation; passed to compilers
DEPFLAGS = -MT $@ -MD -MP -MF $(DEPDIR)/$*.Td

# Following flags broken in Ubuntu 16.04:
# -fsanitize=undefined -fno-sanitize-recover
# -fsanitize=address

# compile C source files
COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) -c -o $@
# compile C++ source files
COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) -c -o $@
# link object files to binary
LINK.o = $(LD) $(LDFLAGS) $(LDLIBS) -o $@
# precompile step
PRECOMPILE =
# postcompile step
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

all: $(BIN)
	mkdir -p res

.PHONY: debug
debug: CPPFLAGS += $(DEBUGFLAGS)
debug: LDFLAGS += $(LDDEBUGFLAGS)
debug: $(BIN)
	mkdir -p res

.PHONY: release
release: CPPFLAGS += $(RELEASEFLAGS)
release: $(BIN)
	mkdir -p res

dist: $(DISTFILES)
	$(TAR) -cvzf $(DISTOUTPUT) $^

.PHONY: clean
clean:
	$(RM) -r $(OBJDIR) $(DEPDIR) $(BIN)

.PHONY: distclean
distclean: clean
	$(RM) $(BIN) $(DISTOUTPUT)

.PHONY: install
install:
	@echo no install tasks configured

.PHONY: uninstall
uninstall:
	@echo no uninstall tasks configured

.PHONY: check
check:
	@echo no tests configured

.PHONY: help
help:
	@echo available targets: all dist clean distclean install uninstall check debug release

$(BIN): $(OBJS)
	$(LINK.o) $^

$(OBJDIR)/%.o: %.c
$(OBJDIR)/%.o: %.c $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.c) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cpp
$(OBJDIR)/%.o: %.cpp $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cc
$(OBJDIR)/%.o: %.cc $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

$(OBJDIR)/%.o: %.cxx
$(OBJDIR)/%.o: %.cxx $(DEPDIR)/%.d
	$(PRECOMPILE)
	$(COMPILE.cc) $<
	$(POSTCOMPILE)

.PRECIOUS = $(DEPDIR)/%.d
$(DEPDIR)/%.d: ;

-include $(DEPS)
