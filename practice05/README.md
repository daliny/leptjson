# 从零开始学习JSON库（五）

* LINDA
* 2018/6/27



## TODO

补充了object类型解析，和object文本生成，并完成相关测试。



## 实现

object类型使用STL的map类型存储，由于上次已经把`class value`改为如下存储结构：

```c++
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

    string stringify_str(); // 字符串序列化
    string stringify_array(); // 数组序列化
    string stringify_obj(); // 对象序列化
    int type; // 相应类型
    shared_ptr<void> context; // 内容
  };
```

仅使用一个智能指针`shared_ptr<void> context`来存放对象, 所以，无需改动结构，简洁方便～

`parse_object()`函数使用状态机的方式。

状态如下：

```c++
enum{ // object的状态码
    KEY_LEFT_QUOTE, // 遇到'"'（左引号）
    KEY_CHAR,  // 解析到"KEY"中的KEY
    KEY_RIGHT_QUOTE, // 遇到'"'（右引号）
    COLON,     // 遇到':'
    COMMA      // 遇到','
  };
```

object序列化很简单，看源码就行了。

## 测试

### 测试代码

```c++
       void ObjectTester(const string& expect, const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_OK, json.geterr()) << s;
          EXPECT_EQ(LEPT_OBJECT, v.gettype()) << s;
          EXPECT_EQ(expect, v.stringify()) << s;
       }
       void InvalidObjMissRBrktTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_MISS_RBRKT, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       void InvalidObjMissColonTester(const string& s)
       {
          value v = json.parse(s); 
          EXPECT_EQ(LEPT_PARSE_MISS_COLON, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }       
       void InvalidObjMissCommaTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_MISS_COMMA, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       void InvalidObjMissKeyTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_MISS_KEY, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       void InvalidObjNoValueTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       void InvalidObjMissPairTester(const string& s)
       {
          value v= json.parse(s);
          EXPECT_EQ(LEPT_PARSE_MISS_PAIR, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
```

### 测试用例

```c++
TEST_F(leptjsontest, leptobject)
{
  ObjectTester("{}", "{       }");
  ObjectTester("{\"1\":null}", "{   \"1\": null  }");
  ObjectTester("{\"\":[{},{}]}", "{\"\" : [{  } ,{}]       }");
  ObjectTester("{\"\":{\"\":{\"1\":{}}}}", "{\"\" : {\"\": { \"1\":{     } }}       }");
  ObjectTester("{\"one\":true,\"three\":12.3,\"two\":[]}", "{\"one\": true   , \"two\"  : [], \"three\": 12.3  }");
  ObjectTester("{\"1\":{\"1.1\":1.1,\"1.2\":[false,\"\"]},\"2\":[{\"2.1\":{\"2.1.1\":\"\"}}]}", 
               "{  \"1\"  :{\"1.1\" : 1.1, \"1.2\"  :[ false, \"\" ]} , \"2\" : [ {\"2.1\": {\"2.1.1\" :\"\"}} ]       }");
}
TEST_F(leptjsontest, leptobjectinvalid)
{
  InvalidObjMissRBrktTester("{   \"1\":3   ");
  InvalidObjMissColonTester("{\"1\"            123}");
  InvalidObjMissCommaTester("{\"1\":123\"3\":321}");
  InvalidObjMissKeyTester("{  ");
  InvalidObjMissKeyTester("{123:456}");
  InvalidObjNoValueTester("{\"123\":,\"2\":5}");
  InvalidObjMissPairTester("{\"1\":123,          }");
}
```



## 集成测试出现的bug

使用这个数据[a Github User repos json](https://api.github.com/users/mralexgray/repos)测试，发现了一个bug：

```c++
"stargazers_count": 0 // parse fail
```

发现是数字类型解析函数有bug：

```c++
  if(json[start] == '0') // 这里有错，0后为空格或其他字符时，
  {                      // 直接返回错误了
    if(start+1 != size && json[start+1] != '.' 
        && json[start+1] != 'e' && json[start+1] != 'E'){
      jerrno = LEPT_PARSE_INVALID_VALUE;
      return value(LEPT_NULL_PTR);
    }
  }
```



修改为：

```c++
if(json[start] == '0')
  {
    if(start+1 != size && (NUM0TO9(json[start+1]) ||                     json[start+1] == 'x' || json[start+1] == 'X'))
    {
      jerrno = LEPT_PARSE_INVALID_VALUE;
      return value(LEPT_NULL_PTR);
    }
  }
```

因为number类型解析内部调用了`strtod()`函数，JSON数据不支持`x or X`十六进制前缀，所以过滤掉它们就可以了。



到这里，json的解析器和生成器就算写完了，但还是存在一些小瑕疵：

1. object类型是用map类型实现的，map内部是RB tree实现的，导致解析后的数据再序列化可能会出现键值对的顺序与之前不同的情况。
2. 测试数据有限，可能存在很多没有注意到的bug。