//==============================================================================
// FILE:
//    QuineMcCluskey.h
//
// DESCRIPTION:
//    Simplify a truth table using Quine-McCluskey algorithm
//
// License: MIT
//==============================================================================
#ifndef QUINE_MC_CLUSKEY_H
#define QUINE_MC_CLUSKEY_H

#include <bitset>
#include <cassert>
#include <set>
#include <vector>

using namespace std;
typedef vector<vector<string>> TableTy;

/**
 * @brief      Class for Quine-McCluskey algorithm.
 * @param[in]  nBits    Number of bits/variables
 * @param[in]  truthTbl Truth table
 * 
 * @details    The truth table is a bit string representing the min terms of a
 *             logic function. If min term k is 1, then the k-th bit is 1. For
 *             example, the min term ab'cd is 1011, so the 11th bit is 1. Here,
 *             the inputs are ordered as abcd...
 */
class QMCAlg {
public:
  QMCAlg(int nBits, unsigned truthTbl);

  size_t getFunctionSize() {
    return functions.size();
  }

  set<int> getFunction(size_t i) {
    assert(i < functions.size());
    return functions[i];
  }

  unsigned getPrimeImp(size_t i) {
    assert(i < primeImp.size());
    unsigned output = 0;
    for (size_t j = 0, end = primeImp[i].size(); j < end; ++j) {
      if (primeImp[i][j] == '1') {
        output |= 1 << (2 * j + 1);
      }
      else if (primeImp[i][j] == '0') {
        output |= 1 << (2 * j);
      }
    }
    return output;
  }

private:
  void setPrimeImp();
  void minimise();
  void displayFunctions();
  void getPosComb(vector<set<int>> &patLogic, set<set<int>> &posComb);
  TableTy combinePairs(TableTy table, set<string>& primeImpTemp);
  void createTable();

  vector<string> minBin; // min terms in binary
  int nBits; // number of bits/variables
  int nMin;  // number of initial min terms
  TableTy table; 
  vector<string> primeImp;
  vector<set<int>> functions;
};

#endif
