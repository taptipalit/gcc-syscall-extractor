GCCDIR = /morespace/gcc/install/bin

CXX = g++-7  #$(GCCDIR)/g++-5-debug
CC = gcc-7 #$(GCCDIR)/gcc-5-debug

# Flags for the C++ compiler: enable C++11 and all the warnings, -fno-rtti is required for GCC plugins
CXXFLAGS = -std=c++11 -Wall -fno-rtti  -g -O0
# Workaround for an issue of -std=c++11 and the current GCC headers
CXXFLAGS += -Wno-literal-suffix -g -O0

# Determine the plugin-dir and add it to the flags
PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

# top level goal: build our plugin as a shared library
all: syscall_extractor.so

syscall_extractor.so: syscall_extractor.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

syscall_extractor.o : syscall_extractor.cc
	$(CXX) $(CXXFLAGS) -fPIC -c -o $@ $<

clean:
	rm -f syscall_extractor.o syscall_extractor.so

check: syscall_extractor.so test.c
	rm -f *.confine
	$(CC) -O0 -fplugin=./syscall_extractor.so -c -fdump-rtl-expand test.c -o /dev/null # 2> test.dot

.PHONY: all clean check

