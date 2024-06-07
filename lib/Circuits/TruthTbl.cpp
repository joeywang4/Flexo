//==============================================================================
// FILE:
//    TruthTbl.cpp
//
// DESCRIPTION:
//    Generate a gate using a truth table
//
// License: MIT
//==============================================================================
#include <string>
#include <vector>
#include "llvm/Support/raw_ostream.h"
#include "Circuits/QuineMcCluskey.h"
#include "Circuits/TruthTbl.h"
#include "Utils.h"

using namespace std;

// load out[reg[0] + reg[1] + ... + reg[n]]
string load_out(string out, vector<string>& inRegs) {
  const string offset = to_string(get_int_env("WR_FAKE_OFFSET", 256));
  if (inRegs.size() == 1) {
    return "mov " + offset + "(" + out + "," + inRegs[0] + "), %r10b\n";
  }
  if (inRegs.size() == 2) {
    string asmCode = "lea (" + out + "," + inRegs[0] + "), %r10\n";
    asmCode += "mov " + offset + "(%r10," + inRegs[1] + "), %r10b\n";
    return asmCode;
  }

  string asmCode = "lea (" + out + "," + inRegs[0] + "), %r10\n";
  for (size_t i = 2, end = inRegs.size(); i < end; ++i) {
    asmCode += "add " + inRegs[i] + ", %r10\n";
  }
  asmCode += "mov " + offset + "(%r10," + inRegs[1] + "), %r10b\n";
  return asmCode;
}

// generate a 1 input n outputs gate using its truth table
string gen_gate_asm(unsigned inputs, unsigned outputs, unsigned truthTbl) {
  string gateAsm = (
    "call mod_ret_addr\n"
  );

  vector<vector<unsigned>> terms, invTerms;
  unsigned usedIn;

  terms.resize(outputs);
  invTerms.resize(outputs);

  // compute simplified truth table for each output
  for (unsigned out = 0; out < outputs; ++out) {
    QMCAlg qmc(inputs, truthTbl >> (out * (1 << inputs)));
    QMCAlg qmcInv(inputs, ~(truthTbl >> (out * (1 << inputs))));

    // get prime implicants and used input variables
    if (qmc.getFunctionSize() == 0 || qmcInv.getFunctionSize() == 0) {
      llvm::errs() << "Error: empty function for truth table " << truthTbl << "\n";
      assert(false);
    }
    for (int x : qmc.getFunction(0)) {
      unsigned minTerm = qmc.getPrimeImp(x);
      terms[out].push_back(minTerm);
      usedIn |= minTerm;
    }
    for (int x : qmcInv.getFunction(0)) {
      unsigned minTerm = qmcInv.getPrimeImp(x);
      invTerms[out].push_back(minTerm);
      usedIn |= minTerm;
    }
  }

  // load inputs
  for (unsigned i = 0; i < 2 * inputs; ++i) {
    if (usedIn & (1 << i)) {
      gateAsm += "movzbq ($" + to_string(i) + "), $" + to_string(i) + "\n";
    }
  }

  // load outputs
  for (unsigned out = 0; out < outputs; ++out) {
    unsigned outOffset = 2 * inputs + 2 * out;
    vector<unsigned> allTerms = terms[out];
    allTerms.insert(allTerms.end(), invTerms[out].begin(), invTerms[out].end());
    size_t firstInv = terms[out].size();

    for (size_t i = 0, end = allTerms.size(); i < end; ++i) {
      unsigned term = allTerms[i];
      vector<string> inRegs;
      for (unsigned j = 0; j < 2 * inputs; ++j) {
        if (term & (1 << j)) {
          inRegs.push_back("$" + to_string(j));
        }
      }
      gateAsm += load_out("$" + to_string(outOffset + (i < firstInv)), inRegs);
    }
  }

  return gateAsm;
}
