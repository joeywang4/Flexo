#include <algorithm>
#include <cstdlib>
#include <fstream>
#include "llvm/Support/raw_ostream.h"
#include "Utils.h"

using namespace std;

string get_str_env(const char* varname, string default_val) {
  char *PTR_ENV_VAR = getenv(varname);
  return PTR_ENV_VAR ? PTR_ENV_VAR : default_val;
}

int get_int_env(const char* varname, int default_val) {
  char *PTR_ENV_VAR = getenv(varname);
  return PTR_ENV_VAR ? stoi(PTR_ENV_VAR) : default_val;
}

void open_file(ofstream &fileStream, string filename) {
  fileStream.open(filename);
  if (!fileStream.is_open()) {
    llvm::outs() << "Failed to open " << filename << "\n";
    assert(false);
  }
}

void write_file(string filename, string &content) {
  ofstream infile;
  open_file(infile, filename);
  infile << content;
  infile.close();
}
