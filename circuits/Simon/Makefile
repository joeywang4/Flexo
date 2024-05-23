CXX = clang-17
CXXFLAGS = -fno-discard-value-names -fno-inline-functions -O1
SOURCES = simon32.c
TARGETS = $(SOURCES:.c=.ll)

all: $(TARGETS) simon25.ll

%.ll: %.c
	$(CXX) $(CXXFLAGS) -S -emit-llvm $< -o $@

simon25.ll: simon32.c
	$(CXX) $(CXXFLAGS) -D ROUNDS=25 -S -emit-llvm simon32.c -o simon25.ll

clean:
	$(RM) $(TARGETS) simon25.ll
