# 从零开始学习JSON库（六）

* LINDA
* 2018/9/23



## TODO

补充了对 array、object 中各元素的取值操作和赋值操作，并完成相关测试。

### 原因

为什么突然想写它们呢？

一是之前在做 leetcode 时，有时候无法通过测试用例，想在本地调试，又苦于它的测试用例是 json 数组，无法直接用 vector 来存储，有了对 json array 对象的各元素的取值操作，就可以很方便的存储到 vector 中了O(∩_∩)O~

二是最近有网友问起，就顺便回顾一下。

## 实现

首先，我们要实现的功能如下：

```c++
leptjson json;
value v = json.parse("[[1,2,3], [4,5,6], [7,8,9]]"); // 解析json 数组
vector<vector<int>> vv(3, vector<int>(3));
for(int i = 0; i < (int)v.size(); ++i)
    for(int j = 0; j < (int)v[0].size(); ++j)
        vv[i][j] = v[i][j].getnumber(); // 取值赋给vector
```

需要知道对象的大小，所以，在 value 类中增加一个 `sz` 成员变量，相应的构造函数也要添加：

```c++
class value {
    //...
    size_t size();
private:
    size_t sz; // 注意，sz只对 json array 和 json object 有意义！
};
```

因为json array 的底层存储就是 `vector<value>`，所以我们可以很容易的实现索引操作符 `operator[]`：

```c++
value& value::operator[](size_t i) {
    assert(type == LEPT_ARRAY); // 只对 json array 可用
    auto v& = *static_cast<vector<value>>(context.get()); // 注意这里是引用类型！相关问题下面 
                                                          // 再说
    return v[i];
}

value value::operator[](size_t i) const { // const函数，我们返回它的一个拷贝
    assert(type == LEPT_ARRAY); // 只对 json array 可用
    auto v& = *static_cast<vector<value>>(context.get());
    return v[i];
}
```

因为有拷贝操作，所以需要一个 copy 构造函数：

```c++
explicit value(const value& v) :type(v.type), sz(v.sz), context(v.context){ }
```

json object 的操作类似就不写了。

接着，说一下赋值操作。

```c++
value& value::operator=(const value& v) {
    type    = v.type;
    sz      = v.sz;
    context = v.context;
    return *this;
}
```

因为在这样的设计下，赋值必然是对每个 value 对象赋值，而 value 对象又有多种类型，这就让赋值操作变得很麻烦：

```c++
value v = json.parse("123"); // json number
v = value(LEPT_NUMBER, 0, make_shared<double>(456));  // 太麻烦！
```

为了解决这种情况，我们应该有一个json number构造函数：

```c++
value::value(double d) :type(LEPT_NUMBER), sz(0), context(make_shared<double>(d)) {}
```

相应的也应该有json string 的构造函数:

```c++
value::value(const string& s) :type(LEPT_STRING), sz(0), 
                          context(make_shared<string>(s)) { }
// bool 类型的函数会与double 类型的构造函数冲突，就不采用了
// value::value(bool b) : type(b|LEPT_FALSE), sz(0), 
//                       context(make_shared<bool>(b)) {}
```

那么，我们就可以使用数字和字符串直接赋值了：

```c++
value v = json.parse("123");
v = 456;
v = string("hello"); // 注意：不能直接使用 const char* 赋值
```

另外，object 赋值要小心，因为底层是智能指针，所以，相当于拷贝了被赋值对象的指针，两个对象指向相同的内容，一个修改内容会影响另一个！为浅层复制，深层复制暂时没有实现：

```c++
value o = json.parse("{\"1\":{\"name\":\"LINDA\", \"age\":23},\"2\":{\"name\":\"DALIN\", \"age\":22}}");
value o1(o["1"]); // 浅层复制
o["1"] = o["2"];  // 浅层复制，o["1"]对象的指针指向与o["2"]相同
```



## 测试

测试 array 取值和赋值

```c++
TEST_F(leptjsontest, getleptarray){
  value v = json.parse("[[1,2,3],[4,5,6],[7,8,9]]");
  EXPECT_EQ(LEPT_PARSE_OK, json.geterr());
  EXPECT_EQ(LEPT_ARRAY, v.gettype());
  EXPECT_EQ((int)v.size(), 3);
  EXPECT_EQ((int)v[0].size(), 3);
  int exp[3][3]  = {{1,2,3}, {4,5,6}, {7,8,9}};
  int exp2[3][3] = {{7,8,9}, {4,5,6}, {1,2,3}};
  for(int i = 0; i < (int)v.size(); ++i) {
     for(int j = 0; j < (int)v[i].size(); ++j) {
        EXPECT_EQ(v[i][j].getnumber(), exp[i][j]);
        //v[i][j] = value(LEPT_NUMBER, 0, make_shared<double>(exp2[i][j])); // 太复杂！！
        v[i][j] = exp2[i][j];
        EXPECT_EQ(v[i][j].getnumber(), exp2[i][j]);
     }
  }
}
```



测试 object 取值和赋值

```c++
TEST_F(leptjsontest, getleptobject) {
  value v = json.parse("{\"1\":{\"name\":\"LINDA\", \"age\":23}, \"2\":{\"name\":\"DALIN\", \"age\":22}}");
  EXPECT_EQ(LEPT_PARSE_OK, json.geterr());
  EXPECT_EQ(LEPT_OBJECT, v.gettype());
  EXPECT_EQ((int)v.size(), 2);
  value v1(v["1"]);  // 浅复制
  EXPECT_EQ(v1["name"].getstring(), "LINDA");
  EXPECT_EQ(v1["age"].getnumber(), 23);
  value v2(v["2"]);  // 这些是浅复制
  EXPECT_EQ(v2["name"].getstring(), "DALIN");
  EXPECT_EQ(v2["age"].getnumber(), 22);

  string name = v["1"]["name"].getstring();
  int age = v["1"]["age"].getnumber();
  v["1"]["name"] = v["2"]["name"]; // 不可以是 v[1] = v[2]
  v["1"]["age"]  = v["2"]["age"];
  cout << v["1"]["name"].getstring() << endl;
  cout << v["1"]["age"].getnumber() << endl;
  v["2"]["name"] = name;
  v["2"]["age"]  = age;
  
  cout << v["1"]["name"].getstring() << endl;
  cout << v["1"]["age"].getnumber() << endl;
  EXPECT_EQ(v["1"]["name"].getstring(), "DALIN");
  EXPECT_EQ(v["1"]["age"].getnumber(), 22);
  EXPECT_EQ(v["2"]["name"].getstring(), "LINDA");
  EXPECT_EQ(v["2"]["age"].getnumber(), 23);
}
```




## 测试出现的bug

1.  `operator[]()`函数在一般返回对象的元素的引用，所以，**一定不要返回临时变量**！有时，可能是不小心就会出错：

   ```c++
   value& operator[](size_t i) {
       assert(type == LEPT_ARRAY); // 很重要！！
       auto v = static_cast<vector<value>*>(context.get()); // 一不小心，就使用了临时变量v，
                                                            // 它是源对象的拷贝！！
       return v[i];
   }
   ```

   因为是临时变量，出来作用域就被析构了，就会导致莫名的错误，编译器不会报错，而且有时运行还感觉不出来有错，比如，我遇到的如果没有断言，可能还不知道有错：

   第一元素的type值总为0，而其他内容正确！

2. 如果不想`operator=()`赋值操作符将**某些类型值隐式转型进行赋值**，一定要明确地 explicit 它的相关构造函数，可以将错误提前到编译期。

   

   