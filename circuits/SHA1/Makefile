CXX = clang-17
CXXFLAGS = -fno-discard-value-names -fno-inline-functions -O1 -S -emit-llvm

SOURCES = sha1_round.c sha1_2blocks.c
TARGETS = $(SOURCES:.c=.ll)

all: $(TARGETS)

%.ll: %.c sha1.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(TARGETS)
