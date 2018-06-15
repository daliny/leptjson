#include <string>
#include <iostream>
#include <memory>
using namespace std;
class value {
  shared_ptr<void> c;
  int type;
  public:
  value(int t, shared_ptr<void> cp): c(cp), type(t){}
  ~value(){
    cout << "value destory" << endl;
  }
};


int main()
{
  auto p = make_shared<string>(string("fantastic"));
  //auto p = make_shared<double>(12.3);

  value(0, p);
}

/*
 * auto p = make_shared<double>(new double(12.3)); // 这样会错，为什么？
 * value(0, make_shared<void>(p)); // 这样也错，为什么？
 * 为什么make_shared不能用
 *
 * ****解决****************
 * make_shared<T>(value)中的T和value类型要相同，即
 * make_shared<double>(12.3); // OK，不需要先new
 * make_shared<string>(new string("great")); // error, 不需要new
 * auto p = shared_ptr<string>(new string("great")); // OK, 需要先new
 * */

/*
 * 用googletest时，为什么只能在每个case中定义变量v，不能作为类的成员变量
 * */
