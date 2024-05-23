//========================================================================
// FILE:
//    BlifParser.cpp
//
// DESCRIPTION:
//    Parse a Blif file to a circuit.
//
// License: MIT
//========================================================================
#include <algorithm>
#include <locale>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "Circuits/Circuit.h"

using namespace std;

// https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
// trim from start (in place)
static inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(string &s) {
  s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
      return !isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(string &s) {
  rtrim(s);
  ltrim(s);
}

string removeInlineComment(const string& line) {
    // Find the position of '//' within the line
    size_t commentPos = line.find("#");
    if (commentPos != string::npos) {
        // Remove the comment portion of the line
        return line.substr(0, commentPos);
    }
    // If no comment found, return the original line
    return line;
}

void Circuit::parseBlif(StringRef filename) {
  useTable = true;
  ifstream infile(filename.str() + ".blif");
  if (!infile.is_open()) {
    infile.open(filename.str());
  }
  if (!infile.is_open()) {
    llvm::outs() << "Failed to open Blif file: " << filename << "\n";
    assert(false);
  }

  vector<vector<string>> tokens;
  string line;
  while (getline(infile, line)) {
    trim(line);
    line = removeInlineComment(line);
    if (line.empty()) continue;

    istringstream iss(line);
    vector<string> tokenizedLine;
    copy(istream_iterator<string>(iss),
         istream_iterator<string>(),
         back_inserter(tokenizedLine));
    tokens.push_back(tokenizedLine);
  }

  infile.close();

  auto it = tokens.begin(), end = tokens.end();
  while (it != end) {
    vector<string> tokenizedLine = *it;
    if (tokenizedLine[0] == ".inputs" || tokenizedLine[0] == ".outputs") {
      bool isInput = tokenizedLine[0] == ".inputs";
      for (size_t i = 1, size = tokenizedLine.size(); i < size; ++i) {
        string token = tokenizedLine[i];
        // remove leading '_' from token
        if (token[0] == '_') {
          token = token.substr(1);
        }
        if (isInput) {
          addInputWire(token);
        }
        else {
          addOutput(getOrCreateWire(token));
        }

        auto names = isInput ? &inputNames : &outputNames;
        auto offsets = isInput ? &inputOffsets : &outputOffsets;

        // split token into name and offset
        size_t offsetPos = token.find('[');
        if (offsetPos != string::npos) {
          names->push_back(token.substr(0, offsetPos));
          offsets->push_back(stoi(token.substr(offsetPos + 1, token.size() - offsetPos - 2)));
        }
        else {
          names->push_back(token);
          offsets->push_back(0);
        }
      }
    }
    else if (tokenizedLine[0] == ".names") {
      vector<StringRef> wireNames;
      if (tokenizedLine.size() >= 3) {
        for (size_t i = 1, size = tokenizedLine.size(); i < size; ++i) {
          if (tokenizedLine[i][0] == '_') {
            tokenizedLine[i] = tokenizedLine[i].substr(1);
          }
          if (i != size - 1) wireNames.push_back(tokenizedLine[i]);
        }
      }
      else {
        // ignore $false, $true, and $undef
        if (tokenizedLine.size() != 2 || (
          tokenizedLine[1] != "$false" &&
          tokenizedLine[1] != "$true" &&
          tokenizedLine[1] != "$undef"
        )) {
          llvm::outs() << "Invalid inputs/outputs: ";
          for (auto& token : tokenizedLine) {
            llvm::outs() << token << " ";
          }
          llvm::outs() << "\n";
        }
        // skip this gate
        while ((*++it)[0][0] != '.') {}
        continue;
      }

      unsigned truthTbl = 0;
      vector<string> entry = *++it;
      while (entry[0][0] != '.') {
        assert(entry.size() == 2 && entry[1] == "1");
        int minTerm = stoi(entry[0], nullptr, 2);
        truthTbl |= (1 << minTerm);
        entry = *++it;
      }

      addGate(
        GateType::TABLE,
        wireNames,
        {tokenizedLine[tokenizedLine.size() - 1]},
        truthTbl
      );
      continue;
    }
    else if (tokenizedLine[0] == ".end") {
      break;
    }
    else if (tokenizedLine[0] != ".model") {
      llvm::outs() << "Unknown Blif command: " << tokenizedLine[0] << "\n";
      assert(false);
    }
    ++it;
  }

  insertAssign();
  removeRedundantAssign();
  sortGates();
}
