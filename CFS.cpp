#include <iostream>
#include <sstream>
#include <string>
#include "CFS.h"
using namespace std;

string ItoS(int number){
   ostringstream os;
   os << number;
   return os.str();
 }

int StoI(string s){
  istringstream ss(s);
  int result;
  return ss >> result ? result:0;
}
