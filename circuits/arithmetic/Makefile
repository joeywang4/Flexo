CXX = clang-17
CXXFLAGS = -fno-discard-value-names -O1 -fno-inline-functions -S -emit-llvm

SOURCES = adder.cpp mul.cpp
TARGETS = $(SOURCES:.cpp=.ll)

all: $(TARGETS)

%.ll: %.cpp macro.h
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(TARGETS)
