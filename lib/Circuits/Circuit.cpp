//========================================================================
// FILE:
//    Circuit.cpp
//
// DESCRIPTION:
//    Manage a circuit of the weird machine
//
// License: MIT
//========================================================================
#include <vector>
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Constants.h"
#include "Circuits/Circuit.h"
#include "Utils.h"

using namespace llvm;

// get a wire, return nullptr if wire does not exist
Wire* Circuit::getWire(StringRef name) {
  if (namedWires.contains(name)) return namedWires[name];
  else return nullptr;
}

// get a wire, create a new one if it does not exist
Wire* Circuit::getOrCreateWire(StringRef name) {
  if (namedWires.contains(name)) return namedWires[name];
  Wire* newWire = createWire();
  namedWires[name] = newWire;
  return newWire;
}

// add an input wire
Wire* Circuit::addInputWire(StringRef name) {
  // input wire should be a new wire
  assert(!namedWires.contains(name));

  Wire* newWire = createWire();
  namedWires[name] = newWire;
  inputs.push_back(newWire);
  return newWire;
}

// create an unnamed wire
Wire* Circuit::createWire() {
  Wire* newWire = new Wire();
  newWire->input = nullptr;
  newWire->outputs.clear();
  newWire->id = wireCount;
  wireCount += 1;
  wires.push_back(newWire);
  return newWire;
}

// set output wire
void Circuit::addOutput(Wire* wire) {
  outputs.push_back(wire);
}

// create a gate using wire names
void Circuit::addGate(
  GateType type,
  ArrayRef<StringRef> inputs,
  ArrayRef<StringRef> outputs,
  unsigned truthTbl
) {
  std::vector<Wire*> inputWires;
  std::vector<Wire*> outputWires;
  inputWires.reserve(inputs.size());
  outputWires.reserve(outputs.size());

  // get input wires
  for (auto input : inputs) {
    Wire* inWire = getOrCreateWire(input);
    inputWires.push_back(inWire);
  }
  // get or create output wires
  for (auto output : outputs) {
    Wire* outWire = getOrCreateWire(output);
    outputWires.push_back(outWire);
  }
  addGate(type, inputWires, outputWires, truthTbl);
}

// create a gate using wire ptrs
void Circuit::addGate(
  GateType type,
  std::vector<Wire*> inputWires,
  std::vector<Wire*> outputWires,
  unsigned truthTbl
) {
  // create a new gate
  gates.push_back(newGate(type, inputWires, outputWires, truthTbl));

  // DEBUG: print gate generation
  if (verbose) {
    std::string name = toString(type);

    outs() << "Add gate: " << name;
    if (type == GateType::TABLE) {
      outs() << "-" << truthTbl;
    }
    outs() << "(";
    for (size_t i = 0, size = inputWires.size(); i < size; ++i) {
      outs() << inputWires[i]->id;
      if (i == size - 1) outs() << ") -> ";
      else outs() << ", ";      
    }
    for (size_t i = 0, size = outputWires.size(); i < size; ++i) {
      outs() << outputWires[i]->id;
      if (i < size - 1) outs() << ", ";
      else outs() << "\n";
    }
  }
}

// generate a new gate
Gate* Circuit::newGate(
  GateType type,
  std::vector<Wire*> inputWires,
  std::vector<Wire*> outputWires,
  unsigned truthTbl
) {
  // create a new gate
  Gate* newGate = new Gate(type, inputWires, outputWires, truthTbl);

  // assign inWire output to the new gate
  for (Wire* inWire : inputWires) {
    inWire->outputs.push_back(newGate);
  }

  // assign outWire input to the new gate
  for (Wire* outWire : outputWires) {
    assert(outWire->input == nullptr);
    outWire->input = newGate;
  }

  return newGate;
}

GateIterator Circuit::eraseGate(GateIterator it) {
  Gate* gate = *it;
  // remove the gate from input wires
  for (Wire* inWire : gate->inputs) {
    vecRemove(&inWire->outputs, gate);
  }
  // remove the gate from output wires
  for (Wire* outWire : gate->outputs) {
    outWire->input = nullptr;
  }
  // delete the gate
  delete gate;

  return gates.erase(it);
}

// dump the circuit
std::string Circuit::dump() {
  std::string output;

  // print inputs
  output = "Inputs: ";
  for (size_t i = 0, end = inputs.size(); i < end; ++i) {
    output += inputNames[i] + "[" + std::to_string(inputOffsets[i]) + "]";
    output += "(" + std::to_string(inputs[i]->id) + ") ";
  }
  output += "\n";

  // print outputs
  output += "Outputs: ";
  for (size_t i = 0, end = outputs.size(); i < end; ++i) {
    output += outputNames[i] + "[" + std::to_string(outputOffsets[i]) + "]";
    output += "(" + std::to_string(outputs[i]->id) + ") ";
  }
  output += "\n";

  // print gates
  for (Gate* gate : gates) {
    output += toString(gate->getType());
    if (gate->getType() == GateType::TABLE) {
      output += "-" + std::to_string(gate->getTruthTbl());
    }
    output += "(";
    for (size_t i = 0, size = gate->inputs.size(); i < size; ++i) {
      output += std::to_string(gate->inputs[i]->id);
      if (i == size - 1) output += ") -> (";
      else output += ", ";
    }
    for (size_t i = 0, size = gate->outputs.size(); i < size; ++i) {
      output += std::to_string(gate->outputs[i]->id);
      if (i < size - 1) output += ", ";
      else output += ")\n";
    }
  }

  return output;
}

// sort gates in topological order
void Circuit::sortGates() {
  std::vector<bool> wireReady = std::vector<bool>(wires.size(), false);
  std::vector<Gate*> sortedGates;

  // init wire IDs
  for (unsigned i = 0, end = wires.size(); i < end; ++i) {
    wires[i]->id = i;
  }

  // set input wires to ready
  for (Wire* wire : inputs) {
    if (isConstantWire(wire)) continue;
    wireReady[wire->id] = true;
  }

  // sort gates (baseline implementation, n^2)
  while (gates.size()) {
    size_t erased = 0;

    auto it = gates.begin();
    while (it != gates.end()) {
      Gate* gate = *it;
      // check if all input wires are ready
      bool allReady = true;
      for (Wire* wire : gate->inputs) {
        if (!wireReady[wire->id]) {
          allReady = false;
          break;
        }
      }

      // if all input wires are ready, add the gate to sortedGates
      if (allReady) {
        sortedGates.push_back(gate);
        // set output wires to ready
        for (Wire* wire : gate->outputs) {
          wireReady[wire->id] = true;
        }
        it = gates.erase(it);
        ++erased;
      }
      else {
        ++it;
      }

      if (it == gates.end()) break;
    }

    assert(erased > 0 && "Circuit is not acyclic");
  }

  gates = sortedGates;
  // for (Wire* wire : wires) {
  //   wire->id = 0;
  // }
  // getFalseWire()->id = 0;
  // getTrueWire()->id = 0;

  // // update wire IDs with time of use
  // unsigned nextId = 2;
  // for (auto gate: gates) {
  //   for (auto inWire: gate->inputs) {
  //     if (inWire->id == 0) {
  //       inWire->id = nextId;
  //       nextId += 1;
  //     }
  //   }
  //   for (auto outWire: gate->outputs) {
  //     if (outWire->id == 0) {
  //       outWire->id = nextId;
  //       nextId += 1;
  //     }
  //   }
  // }
}

// delete a wire
void Circuit::eraseWire(Wire* wire) {
  for (auto it = wires.begin(), end = wires.end(); it != end; ++it) {
    if (*it == wire) {
      wires.erase(it);
      break;
    }
  }
  delete wire;
}

// move oldWire's IO to newWire and delete oldWire
void Circuit::replaceWire(Wire* oldWire, Wire* newWire) {
  // update oldWire->outputs->inputs
  if (oldWire->outputs.size() > 0) {
    for (Gate* outGate : oldWire->outputs) {
      vecReplace(&outGate->inputs, oldWire, newWire);
    }
    // copy oldWire->outputs to newWire->outputs
    newWire->outputs.insert(
      newWire->outputs.end(), oldWire->outputs.begin(), oldWire->outputs.end()
    );
  }

  // update circuit's input/output wires
  vecReplace(&inputs, oldWire, newWire);
  vecReplace(&outputs, oldWire, newWire);

  // update oldWire->input->outputs
  if (oldWire->input != nullptr) {
    vecReplace(&oldWire->input->outputs, oldWire, newWire);
    assert(newWire->input == nullptr);
    newWire->input = oldWire->input;
  }

  // erase oldWire
  if (!isConstantWire(oldWire)) eraseWire(oldWire);
}

// create assign gates for wires with multiple outputs
void Circuit::insertAssign() {
  const int maxFanout = get_int_env("WM_MAX_FANOUT", 3);

  for (size_t i = 0, end = wires.size(); i < end; ++i) {
    Wire* wire = wires[i];
    bool isOutWire = isOutputWire(wire);
    size_t outSize = wire->outputs.size();
    // create an extra wire as the new output wire
    if (isOutWire) outSize += 1;
    if (
      outSize <= 1 || isInputWire(wire) || isConstantWire(wire)
    ) continue;

    Gate* firstAssign = nullptr;
    size_t currIdx = 0;
    while (true) {
      // create an assign gate
      std::vector<Wire*> outWires;
      unsigned truthTbl = 0;
      outWires.reserve(maxFanout);
      for (int j = 0; j < maxFanout && (currIdx + j) < outSize; ++j) {
        outWires.push_back(createWire());
        truthTbl <<= 2;
        truthTbl |= 2;
      }

      auto assignGate = new Gate(
        useTable ? GateType::TABLE : GateType::ASSIGN,
        {wire}, outWires, truthTbl
      );
      gates.push_back(assignGate);
      if (firstAssign == nullptr) firstAssign = assignGate;
      else wire->outputs.push_back(assignGate);

      // update input/output gates
      for (size_t j = 0, end = outWires.size(); j < end; ++j) {
        outWires[j]->input = assignGate;
        // isOutWire && last new wire: modify circuit's output wires
        if (isOutWire && currIdx + j == outSize - 1) {
          vecReplace(&outputs, wires[i], outWires[j]);
        }
        // last output of this assign && not the last new wire: wire = this wire
        else if (currIdx + j != outSize - 1 && j == end - 1) {
          wire = outWires[j];
        }
        else {
          Gate* outGate = wires[i]->outputs[currIdx + j];
          outWires[j]->outputs.push_back(outGate);
          vecReplace(&outGate->inputs, wires[i], outWires[j]);
        }
      }

      if (currIdx + maxFanout < outSize) {
        currIdx += maxFanout - 1;
      }
      else break;
    }

    // update wires[i]'s output gates
    wires[i]->outputs.clear();
    wires[i]->outputs.push_back(firstAssign);
  }
}

// remove 1 output assign gates
void Circuit::removeRedundantAssign() {
  auto it = gates.begin();
  while (it != gates.end()) {
    auto gate = *it;
    if (
      gate->outputs.size() > 1 ||
      !(
        gate->getType() == GateType::ASSIGN  ||
        (gate->getType() == GateType::TABLE && gate->getTruthTbl() == 2)
      )
    ) {
      ++it;
      continue;
    }

    // remove the gate and the output wire
    gate->outputs[0]->input = nullptr;
    replaceWire(gate->outputs[0], gate->inputs[0]);
    it = eraseGate(it);
  }
}

// remove unused input wires
void Circuit::removeUnusedInputs() {
  for (auto wire : inputs) {
    if (wire->outputs.size() == 0) {
      replaceWire(wire, getFalseWire());
    }
  }
}
