#include <memory>
#include <iostream>
#include <vector>
#include <string>
using namespace std;
class test {
public:
  test() {
    std::cout << "Test created" << std::endl;
  }
  ~test() {
    std::cout << "Test destroyed" << std::endl;
  }
};

int main() {
  std::cout << "At begin of main.\ncreating std::vector<std::shared_ptr<void>>" 
            << std::endl;
  std::vector<std::shared_ptr<void>> v;
  {
    std::cout << "Creating test" << std::endl;
    v.push_back( std::shared_ptr<string>( new string("fantastic") ) );
    std::cout << "Leaving scope" << std::endl;
  }
  std::cout << "Leaving main" << std::endl;
  return 0;
}

/*
 *
    switch(a[i].gettype())
    {
      case LEPT_NULL :  s += "null"; break;
      case LEPT_TRUE :  s += "true"; break;
      case LEPT_FALSE:  s += "false";break;
      case LEPT_NUMBER: s += a[i].getnumber(); break;
      case LEPT_STRING: s += a[i].stringify_str(); break;
      case LEPT_ARRAY:  s += a[i].stringify_array(); break;
      case LEPT_OBJECT: s += a[i].stringify_obj(); break;
    }
 * */
