#从0开始学习JSON库（四）

* LINDA
* 2018/6/12

### 结构体设计问题
隔了好久没有写了，主要是之前写JSON数组时，发现原来的数据结构不太适合。之前的结构体如下：
```
struct value{ // json值
    int type;
    union{
      string s;  // 存字符串
      double n;  // 存数字
    };
    value(int _type, double d);
    value(int _type, string str);
    ~value();
};
```
其中，字符串`s`和数字`n`在一个union中，这是模仿叶老师的写法的，但是，要加入数组和对象类型，这个union会很难使用，各种构造函数和析构函数要考虑，我就在思考能不能用[varient](http://en.cppreference.com/w/cpp/utility/variant)来代替union。嗯，`varient`在C++17已经加入std了，`使用相关操作在最后`。尝试用`variant`如下：
```
struct value{ // json值
        int type;
        variant<double, shared_ptr<string>, shared_ptr<vector<value>>> u;
    
        value(const int _type, shared_ptr<vector<value>> a); // 构建json数组
        value(const value& v);
        value(const int _type, const double d);
        value(const int _type, const string str);
      };
```
这样写的话，是可以用了，但是构造函数要写多个，太繁琐了，抛弃！
我又想既然每个类型互斥，是否可以用模板来写：
```
template <typename T>
  class value{ // json值
  public:
    value(int t, shared_ptr<T> c=nullptr);
    int gettype(); // 获取类型
    double getnumber(); // 获取数字
    string getstring(); // 获取字符串
  private:
    int type;
    shared_ptr<T> context; // 内容
  };
```
这样，看似问题解决了，其实，并没有。使用模板类型要先传入，也就是说，在**编译期**就要先知道对象的类型，不能到运行时再获取。而每次我们要解析一个json文本都是要在运行时才知道类型的。所以，模板也不能使用。。。此外，因为类成员变量不能用模板，所以，值要传回，不能保存在类内部。
[来自这里](https://stackoverflow.com/questions/27838286/c-function-with-variable-return-type-using-auto)
最后，我采用了最简单的方法，效果还不错：
```
class value{ // json值
  public:
    value();
    ~value();
    value(int t, shared_ptr<void> c=nullptr);
    int gettype(); // 获取类型
    double getnumber(); // 获取数字
    string getstring(); // 获取字符串
    string stringify(); // 序列化
  private:

    string stringify_str();
    string stringify_array();
    string stringify_obj();
    int type;
    shared_ptr<void> context; // 内容
  };
```
把上面的模板改为固定的，**智能指针始终为void类型**，而当我们要获取相应JSON值时，再进行类型转换。这样，也不用额外的栈来存储了，直接存入即可。
相应做了一些调整：
```
int gettype(); // 获取类型
double getnumber(); // 获取数字
string getstring(); // 获取字符串
string stringify(); // 序列化
```
这些函数都是与json值操作相关的放在value中较为合理。

之前是在leptjson类中使用成员变量`v`来保存解析后的值，现在直接将解析后的值传回，**错误码**保存在leptjson的成员变量`jerrno`中。

此外，也顺便把**json生成器**写了——`string stringify()`。

### 编码问题
 `shared_ptr<>()`vs`make_shared<>()`
使用时，要注意一下两个传入的值不同：
```
auto p = shared_ptr<double>(new double(12.3)); // OK
auto p = make_shared<double>(12.3); // OK
auto p = make_shared<double>(new double(12.3)); //error
```
详见[make_shared](http://en.cppreference.com/w/cpp/memory/shared_ptr/make_shared#Example)
此外，`shared_ptr<>()`使用一次，分配两次（一次分配存储内容空间，另一次分配控制块空间）；`make_shared<>()`使用一次，只分配一次。所以，后者效率比前者高，尽量使用后者。

要将double转换为string，可以使用string流：
```
#include <sstream>
double d = 12.3;
ostringstream os;
os << d;
string s = os.str();
```
同样，将数值以16进制转为string：
```
#include <sstream>
double d = 13;
ostringstream os;
os << hex << d;
string s = os.str();
```
详见[how-do-i-convert-a-double-into-a-string-in-c](https://stackoverflow.com/questions/332111/how-do-i-convert-a-double-into-a-string-in-c)

实现就没什么好说的了。。就酱。

叶老师的知乎专栏也更新啦。哈哈，有点小激动，跟上老师的步伐。

