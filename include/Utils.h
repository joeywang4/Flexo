#ifndef UTILS_H
#define UTILS_H

#include <fstream>
#include <string>
#include <vector>

using namespace std;

string get_str_env(const char* varname, string default_val);
int get_int_env(const char* varname, int default_val);
void open_file(ofstream &infile, string filename);
void write_file(string filename, string &content);

template <class T, class V>
void vecRemove(vector<T>* vec, V val) {
  vec->erase(remove(vec->begin(), vec->end(), val), vec->end());
}

template <class T, class V>
void vecReplace(vector<T>* vec, V oldVal, V newVal) {
  replace(vec->begin(), vec->end(), oldVal, newVal);
}

template <class T, class V>
int findIdx(vector<T>* vec, V val) {
  auto it = find(vec->begin(), vec->end(), val);
  return it == vec->end() ? -1 : distance(vec->begin(), it);
}

#endif
