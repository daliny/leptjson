#include "leptjson.h"
#include <iostream>
#include <string>

using namespace json;
int main()
{
  string s = "";
  char c;
  while(cin.get(c))
    s += c;
  cout << s;
  leptjson lpj;

  value v = lpj.parse(s);
  cout << "type " << v.gettype() << endl;
  cout << "errno" << lpj.geterr() << endl;

  cout << v.stringify() << endl;
}

/*
还是需要优化，number解析时，遇到0，有没有更好的处理方式。
*/
