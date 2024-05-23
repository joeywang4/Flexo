//==============================================================================
// FILE:
//    Wire.h
//
// DESCRIPTION:
//    A wire in the circuit
//
// License: MIT
//==============================================================================
#ifndef WIRE_H
#define WIRE_H

#include <vector>

class Gate;

struct Wire {
  unsigned id;
  Gate* input;
  std::vector<Gate*> outputs;
};

#endif
