//==============================================================================
// FILE:
//    TruthTbl.h
//
// DESCRIPTION:
//    Generate a gate using a truth table
//
// License: MIT
//==============================================================================
#ifndef TRUTH_TBL_H
#define TRUTH_TBL_H

#include <string>
#include <vector>

using namespace std;

string load_out(string out, vector<string>& inRegs);
string gen_gate_asm(unsigned inputs, unsigned outputs, unsigned truthTbl);

#endif
