CXX = clang-17
CXXFLAGS = -fno-discard-value-names -O1 -S -emit-llvm

SOURCES = sbox.c aes_sep_round.c aes_round.c aes_block.c
TARGETS = $(SOURCES:.c=.ll)

all: $(TARGETS)

%.ll: %.c aes.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(TARGETS)
