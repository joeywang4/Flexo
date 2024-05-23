//==============================================================================
// FILE:
//    QuineMcCluskey.cpp
//
// DESCRIPTION:
//    Simplify a truth table using Quine-McCluskey algorithm. This
//    implementation is adapted from the mohdomama/Quine-McCluskey repository
//    on GitHub.
//
// License: MIT
//==============================================================================
#include "Circuits/QuineMcCluskey.h"

#define VERBOSE 0
#if VERBOSE
#include <iostream>
#endif

string intToBinString(int size, int val) {
  string bin;
  bin = bitset<100>(val).to_string();
  bin = bin.substr(100-size);
  return bin;
}

// return the number of 1s in a binary string
int get1s(string x) {
  int count = 0;
  for (size_t i = 0, end = x.size(); i < end; ++i) {
    if (x[i] == '1') ++count;
  }
  return count;
}

// checks if two strings differ at exactly one location or not
bool compare(string a, string b) {
  bool diff = false;
  for (size_t i = 0, end = a.size(); i < end; ++i) {
    if (a[i] != b[i]) {
      if (diff) return false;
      diff = true;
    }
  }
  return diff;
}

// retur a string that replaces the differ location of two strings with '-'
string getDiff(string a, string b) {
  for (size_t i = 0, end = a.size(); i < end; ++i) {
    if (a[i] != b[i])
      a[i] = '-';
  }
  return a;
}

// return true if all the vectors in the table are empty
bool checkEmpty(TableTy table) {
  for (auto minTerms : table) {
    if (minTerms.size() != 0) {
      return false;
    }
  }
  return true;
}

// converts binary string to alphabetic variables
string binToString(string x) {
  string out = "";
  for (size_t i = 0, end = x.size(); i < end; ++i) {
    if (x[i]=='1') {
      char x = 97 + i;
      out += x;
    }
    else if (x[i]=='0') {
      char x = 97 + i;
      out += x;
      out += "'";
    }
  }
  return out;
}

// check if a prime implicant satisfies a min term
bool primeIncludes(string imp, string minTerm) {
  for (size_t i = 0, end = imp.size(); i < end; ++i) {
    if (imp[i] != '-' && imp[i] != minTerm[i]) {
      return false;
    }
  }
  return true;
}

// return the number of variables in a petrick method combination
size_t getVar(set<int> comb, vector<string> primeImp) {
  size_t count = 0;
  for (int imp : comb) {
    for (size_t i = 0, end = primeImp[imp].size(); i < end; ++i) {
      if (primeImp[imp][i] != '-') ++count;
    }
  }
  return count;
}

// construct and simplify a truth table
QMCAlg::QMCAlg(int nBits, unsigned truthTbl) : nBits(nBits), nMin(0) {
  assert(nBits <= 5);
  for (int i = 0; i < (1 << nBits); ++i) {
    if (truthTbl & (1 << i)) {
      minBin.push_back(intToBinString(nBits, i));
      ++nMin;
    }
  }

  #if VERBOSE
  cout << "\nBinary representation of provided min terms:" << endl;
  for (int i = 0; i < nMin; ++i) {
    cout << i << ") " << minBin[i] << endl;
  }
  #endif

  table = TableTy(nBits+1);    
  setPrimeImp();
  minimise();
}

// calculate the prime implicants
void QMCAlg::setPrimeImp() {
  set<string> primeImpTemp;
  createTable();
  #if VERBOSE
  cout << "\nGetting Prime Implicants.." << endl;
  #endif

  // Combine consecutive terms in the table until its empty
  while (!checkEmpty(table)) {
    table = combinePairs(table, primeImpTemp);
  }

  for (string itr : primeImpTemp) {
    primeImp.push_back(itr);
  }

  #if VERBOSE
  cout << "\nThe Prime Implicants are:" << endl;
  for (int i = 0; i < primeImp.size(); ++i) {
    cout  << i << ") "<< binToString(primeImp[i]) << endl;
  }
  #endif
}

// run Petrick's method on Quine-McCluskey prime implicants
void QMCAlg::minimise() {
  // prepare primeImp chart
  vector<vector<bool>> primeImpChart(primeImp.size(), vector<bool>(nMin, false));

  for (size_t i = 0, end = primeImp.size(); i < end; ++i) {
    for (int j = 0; j < nMin; ++j) {
      primeImpChart[i][j] = primeIncludes(primeImp[i], minBin[j]);
    }
  }

  #if VERBOSE
  cout << "\nPrime implicants chart (rows: prime implicant no., col: minterm no.):" << endl;
  for (int i = 0; i < primeImp.size(); ++i) {
    for (int j = 0; j < nMin; ++j) {
        if (primeImpChart[i][j] == true) {
          cout << "1\t";
        }
        else {
          cout << "0\t";
        }
      }
      cout << endl;
  }
  #endif

  // petric logic
  vector<set<int>> patLogic;
  // column iteration
  for (int j = 0; j < nMin; ++j) {
    set<int> x;
    // row iteration
    for (size_t i = 0, end = primeImp.size(); i < end; ++i) {
      if (primeImpChart[i][j]) {
        x.insert(i);
      }
    }
    patLogic.push_back(x);
  }

  #if VERBOSE
  cout << "\nPetric logic is (row: minterms no., col: prime implicants no.): " << endl;
  for (int i = 0; i < patLogic.size(); ++i) {
    for (int itr : patLogic[i]) {
      cout << itr << " ";
    }
    cout << endl;
  }
  #endif

  // get all possible combinations
  set<set<int>> posComb;
  set<int> prod;

  // insert essential prime implicants
  for (auto it = patLogic.begin(); it != patLogic.end();) {
    if ((*it).size() == 1) {
      prod.insert(*(*it).begin());
      it = patLogic.erase(it);
    }
    else {
      ++it;
    }
  }
  posComb.insert(prod);

  getPosComb(patLogic, posComb); // recursively multiply set elements
  size_t min = 9999;

  #if VERBOSE
  cout << "\nPossible combinations that satisfy all minterms:" << endl;
  #endif
  for (set<int> comb : posComb) {
    if (comb.size() < min) {
      min = comb.size();
    }

    #if VERBOSE
    for (int itr : comb) {
      cout << itr << " ";
    }
    cout << endl;
    #endif
  }

  #if VERBOSE
  cout << "\nGetting the combinations with min terms and min variables ..." << endl;
  #endif
  //Combinations with minimum terms
  vector<set<int>> minComb;
  for (set<int> comb : posComb) {
    if (comb.size() == min) {
      minComb.push_back(comb);
    }
  }

  //Combinations with minimum variables
  min = 9999;
  for (size_t i = 0, end = minComb.size(); i < end; ++i) {
    if(getVar(minComb[i], primeImp) < min) {
      min = getVar(minComb[i], primeImp);
    }
  }

  for (size_t i = 0, end = minComb.size(); i < end; ++i) {
    if(getVar(minComb[i], primeImp) == min) {
      functions.push_back(minComb[i]);
    }
  }
}

void QMCAlg::displayFunctions() {
  // prints output
  #if VERBOSE
  cout << "\n\nThe possible functions are-\n" << endl;
  for (size_t i = 0, end = functions.size(); i < end; ++i) {
    set<int> function = functions[i];
    cout << "Function " << i + 1 << ":"<< endl;
    for (int x : function) {
      cout << binToString(primeImp[x]) << " + ";
    }
    cout << "\b\b  \n" << endl;
  }
  #endif
}

// a recursive function to multiple elements of set patLogic and store it in set posComb
void QMCAlg::getPosComb(vector<set<int>> &patLogic, set<set<int>> &posComb) {
  for (size_t i = 0, end = patLogic.size(); i < end; ++i) {
    set<set<int>> newLevel;
    // insert each element of patLogic[i] to each set in posComb
    for (int x : patLogic[i]) {
      for (set<int> comb : posComb) {
        comb.insert(x);
        newLevel.insert(comb);
      }
    }
    posComb = newLevel;
  }
}

// combines consecutive terms in the table
TableTy QMCAlg::combinePairs(TableTy table, set<string>& primeImpTemp) {
  vector<vector<bool>> checked(table.size(), vector<bool>(nMin, false));
  TableTy newTable(table.size()-1);

  for (size_t i = 0, end_i = table.size() - 1; i < end_i; ++i) {
    set<string> newTableI;
    for (size_t j = 0, end_j = table[i].size(); j < end_j; ++j) {
      for (size_t k = 0, end_k = table[i+1].size(); k < end_k; k++) {
        if (compare(table[i][j], table[i+1][k])) {
          newTableI.insert(getDiff(table[i][j], table[i + 1][k]));
          checked[i][j] = true;
          checked[i + 1][k] = true;
        }
      }
    }
    newTable[i] = vector<string>(newTableI.begin(), newTableI.end());
  }

  for (size_t i = 0, end_i = table.size(); i < end_i; ++i) {
    for (size_t j = 0, end_j = table[i].size(); j < end_j; ++j) {
      if (!checked[i][j]) {
        primeImpTemp.insert(table[i][j]);
      }
    }
  }

  return newTable;
}

// table[number of 1s] -> vector of min terms (in binary strings)
void QMCAlg::createTable() {
  for (int i = 0; i < nMin; ++i) {
    int num1s = get1s(minBin[i]);
    table[num1s].push_back(minBin[i]);
  }
  
  #if VERBOSE
  cout << "\nTable:" << endl;
  for (int i = 0; i < nBits+1; ++i) {
    cout << i << ")  ";
    for (int j = 0; j < table[i].size(); ++j) {
      cout << table[i][j] << ", ";
    }
    cout << endl;
  }
  #endif
}
