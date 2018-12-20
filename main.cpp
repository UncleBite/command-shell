#include <iostream>
#include <sstream>
using namespace std;

std::string parse(const std::string & s) {
  std::stringstream ss{""};
  cout << "in parse, string is" << s << endl;
  for (size_t i = 0; i < s.length(); i++) {
    if (s.at(i) == '\\') {
      switch (s.at(i + 1)) {
        case 'n':
          ss << "\n";
          i++;
          break;
        case '"':
          ss << "\"";
          i++;
          break;
        default:
          ss << "\\";
          break;
      }
    }
    else {
      ss << s.at(i);
    }
  }

  return ss.str();
}

int main(int argc, char ** argv) {
  cout << "argc == >  " << argc << endl;
  for (int i = 0; i < argc; i++) {
    cout << "argv[" << i << "] ==>" << argv[i] << endl;
  }

  return 0;
}
