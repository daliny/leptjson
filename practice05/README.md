# 从零开始学习JSON库（三）

* LINDA
* 2018/5/13


## JSON字符串
在这一节里，加入对字符串的处理。JSON字符串有转义字符，如下：

```
\" represents the quotation mark character (U+0022).
\\ represents the reverse solidus character (U+005C).
\/ represents the solidus character (U+002F).
\b represents the backspace character (U+0008).
\f represents the form feed character (U+000C).
\n represents the line feed character (U+000A).
\r represents the carriage return character (U+000D).
\t represents the character tabulation character (U+0009).
```
除此还有Unicode编码`\uxxxx`， 其中`xxxx`为16进制数。

## 字符串表示
```
enum{..., LEPT_STRING};

struct value{
/********************
	union{
		string s; // string不能在union中使用，
		          // 因为有非平凡构造函数的类在union中没有意义。
		double n;
	};
********************/
	int type;
}v;
```
[Why compiler doesn't allow std::string inside union?
](https://stackoverflow.com/questions/3521914/why-compiler-doesnt-allow-stdstring-inside-union)
翻译如下：
1.  有非平凡（copy等）构造函数（non-trivial (copy/)constructor）的类在union中没有意义。
2. 
```
		union U {
			 string x;
			 vector<int> y;
		};
		U u;  // <--	
```
如果U是一个结构体，`u.x`和`u.y`将会被初始化为空string和空vector。但是，union的成员共享相同的地址。所以，如果`u.x`被初始化了，`u.y`将包含无效数据，反之亦然。如果两者都没有被初始化，它们都不能被使用。在任何时候，在union有这样的数据都不能很容易的被处理。所以，C++98选择禁用：
>An object of a class with a non-trivial constructor (12.1), a non-trivial copy constructor (12.8), a non-trivial destructor (12.4), or a non-trivial copy assignment operator (13.5.3, 12.8) cannot be a member of a union, nor can an array of such objects.

而C++0x选择解禁：
>At most one non-static data member of a union may have a brace-or-equal-initializer. [Note: if any non-static data member of a union has a non-trivial default constructor (12.1), copy constructor (12.8), move constructor (12.8), copy assignment operator (12.8), move assignment operator (12.8), or destructor (12.4), the corresponding member function of the union must be user-provided or it will be implicitly deleted (8.4.3) for the union. — end note ]

但是，为union创建构造函数和析构函数是不可能的。例如，如果没有额外信息，您或编译器如何为上述union编写copy构造函数？为了确保union中的成员有效，你需要使用tagged union，并且需要手动处理构造函数和析构函数：
```
struct TU {
   int type;
   union {
     int i;
     float f;
     std::string s;
   } u;

   TU(const TU& tu) : type(tu.type) {
     switch (tu.type) {
       case TU_STRING: new(&u.s)(tu.u.s); break;
       case TU_INT:    u.i = tu.u.i;      break;
       case TU_FLOAT:  u.f = tu.u.f;      break;
     }
   }
   ~TU() {
     if (tu.type == TU_STRING)
       u.s.~string();
   }
   ...
};
```
此外，这个已经实现为 [boost::variant](https://www.boost.org/doc/libs/1_67_0/doc/html/variant.html) 或 [boost::any](https://www.boost.org/doc/libs/1_67_0/doc/html/any.html)了。

**不过**，对于上面那段代码，我觉得是不正确的，疑惑如下：
1.  我们就是为了构造结构体TU才写的构造函数，但是上面的copy构造函数却使用一个结构体`tu`作为传入参数，那么这个`tu`从何而来。。。
2. 答者是想使用`new(ptr) T`这个函数来分配空间给union中s。但是`new(&u.s)(tu.u.s)`这样写我编译不同过。。。提示：`‘tu.u.s’ does not name a type`。正确的方式应该是`new(&u.s) string;`这是给`u.s`空间放置string信息的操作，然后内存多少由使用者操作。[看这里](https://zhuanlan.zhihu.com/p/20029820) 和[这里](http://en.cppreference.com/w/cpp/language/new)。
3. 析构函数中的`tu.type`又是哪里来的？直接用`type`就可以了。

因此，我的结构体声明和定义如下：
```c++
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

class leptjson{
	...
	unique_ptr<value> v; // 指向value结构体的智能指针
};
value::value(int _type, double d):type(_type){
	n = d;
}

value::value(int _type, string str){
	new(s) string;
	s = str;
}

value::~value(){
	if(type == LEPT_STRING)
		s.~string();
}
```

因为改用智能指针，所以value只有在解析成功了才分配空间存储，其他情况为`nullptr`。

## 字符串解析
```
int leptjson::parse_value(string& s)
{
	...
	case '\"': parse_string(s);
	...
}
int leptjson::parse_string(string& s);
```

## 单元测试

```c++

...
void InvalidStrEscapeTester(const string& s)
{
	 EXPECT_EQ(LEPT_PARSE_INVALID_STRING_ESCAPE, json.parse(s)) << s;
     EXPECT_EQ(LEPT_NULL_PTR, json.gettype()) << s;
}

void InvalidStrCharTester(const string& s)
{
	 EXPECT_EQ(LEPT_PARSE_INVALID_STRING_CHAR, json.parse(s)) << s;
     EXPECT_EQ(LEPT_NULL_PTR, json.gettype()) << s;
}
...

TEST_F(leptjsontest, leptstring)
{
  StringTester("", "\"\"");
  StringTester("hello", "\"hello\"");
  StringTester("hello \n world", "\"hello \\n world\"");
  StringTester("\" \\ / \b \f \n \r \t", "\"\\\" \\\\ \\/ \\b \\f \\n \\r \\t\"");
  StringTester("\"hello \t\n/\b\f\r\\\"", "\"\\\"hello \\t\\n\\/\\b\\f\\r\\\\\\\"\"");
}

TEST_F(leptjsontest, leptstringinvalid)
{
  InvalidStrEscapeTester("\"one \\k \"");
  InvalidStrEscapeTester("\"one \\r \\x\"");
  InvalidStrCharTester("\"fde \\");
  InvalidStrCharTester("\"fde \t");
}
```
##编码
[在这](https://github.com/daliny/leptjson/tree/master/practice03)
