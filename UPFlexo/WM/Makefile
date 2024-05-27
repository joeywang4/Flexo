CXX = clang-17
CXXFLAGS = -fno-discard-value-names -O1 -S -emit-llvm -I.

AES_SRCS := $(wildcard AES/*.c)
AES_TARGETS = $(patsubst AES/%.c, build/%.ll, $(AES_SRCS))

SIMON_SRCS := $(wildcard Simon/*.c)
SIMON_TARGETS = $(patsubst Simon/%.c, build/%.ll, $(SIMON_SRCS))

all: build $(AES_TARGETS) $(SIMON_TARGETS) build/simon25_CTR.ll

build:
	mkdir build

# build AES
build/%.ll: AES/%.c AES/aes.h chksum.h
	$(CXX) $(CXXFLAGS) $< -o $@

# build Simon32
build/%.ll: Simon/%.c Simon/simon32.h chksum.h
	$(CXX) $(CXXFLAGS) $< -o $@

# build Simon25
build/simon25_CTR.ll: Simon/simon32_CTR.c Simon/simon32.h chksum.h
	$(CXX) $(CXXFLAGS) -D ROUNDS=25 $< -o build/simon25_CTR.ll

clean:
	$(RM) build/*.ll
