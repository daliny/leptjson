# 从零开始学习JSON库（一）
* LINDA
* 2018/5/8

前几天看了叶神的知乎专栏[从零开始的JSON库教程](https://zhuanlan.zhihu.com/json-tutorial)，叶老师很详尽地跟我们讲解一个项目的正确打开方式：
1.  要做一个项目首先要知道做什么，即项目要实现的协议是怎么样的，可以先看相关的RFC文档。
2. 知道大概要做什么了，我们先从简单的入手，构思相应的数据结构，函数接口，罗列出来。
3. 先写对应接口的单元测试，当然，我们不可能一下子就把所有测试都想到，先写一部分能想到的。
4. 把相应接口实现，看测试能否通过，测试通过了，思考是否实现过于繁琐，能不能通过重构简化接口。在写接口时，对于一些正确情况下不可能出现的条件要设断言，这样在重构时才能避免一些错误。
5. 测试通过后，进行下一个接口的实现，重复2-4.

我跟着老师的步骤把过程走了一遍，还是觉得应该自己从0开始实现一个JSON库，这样才能发现自己有哪些没掌握，并学学老师的写作排版。好，我要开动来\(^o^)/~
**注：引用为叶老师教程里的部分原文。**

##JSON是什么

>JSON（JavaScript Object Notation）是一个用于数据交换的文本格式，的标准为[ECMA-404](http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf)。虽然 JSON 源至于 JavaScript 语言，但它只是一种数据格式，可用于任何编程语言。现时具类似功能的格式有 XML、YAML，当中以 JSON 的语法最为简单。
>
例如，一个动态网页想从服务器获得数据时，服务器从数据库查找数据，然后把数据转换成 JSON 文本格式：
>
```js
{
    "title": "Design Patterns",
    "subtitle": "Elements of Reusable Object-Oriented Software",
    "author": [
        "Erich Gamma",
        "Richard Helm",
        "Ralph Johnson",
        "John Vlissides"
    ],
    "year": 2009,
    "weight": 1.8,
    "hardcover": true,
    "publisher": {
        "Company": "Pearson Education",
        "Country": "India"
    },
    "website": null
}
```
>网页的脚本代码就可以把此 JSON 文本解析为内部的数据结构去使用。
>从此例子可看出，JSON 是树状结构，而 JSON 只包含 6 种数据类型：

>* null: 表示为 null
* boolean: 表示为 true 或 false
* number: 一般的浮点数表示方式，在下一单元详细说明
* string: 表示为 "..."
* array: 表示为 [ ... ]
* object: 表示为 { ... }

> 我们要实现的 JSON 库，主要是完成 3 个需求：

>1. 把 JSON 文本解析为一个树状数据结构（parse）。
>2. 提供接口访问该数据结构（access）。
>3. 把数据结构转换成 JSON 文本（stringify）。

![requirement.png](https://img-blog.csdn.net/20180508153837220?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpbmRhX2Rz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)

##JSON解析

###1. null和boolean的解析
JSON文本表示（详见老师课程1）
```
JSON-text = ws value ws
ws = *(%x20 / %x09 / %x0A / %x0D)
value = null / false / true 
null  = "null"
false = "false"
true  = "true"
```
```c++
enum{ // 返回状态值
	LEPT_PARSE_OK = 0,  // 解析成功
    LEPT_PARSE_EXPECT_VALUE,  // 只含有空白
    LEPT_PARSE_INVALID_VALUE, // 无效值，非null，boolean字面量
    LEPT_PARSE_ROOT_NOT_SINGULAR // 若在值和空白之后还有其他字符
};
enum{ // JSON类型
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE
};

class leptjson{
public:
	leptjson():type(LEPT_NULL){}
	int  parse(const string& json); // 解析json文本，目前只有null，boolean
	int  gettype();  // 获取json类型
private:
	int parse_whitespace(); // 清除空格
	int parse_value(const string& json); // 解析分支
	int parse_null(const string& json);  // 解析null
	int parse_true(const string& json); // 解析true
	int parse_false(const string& json); // 解析false
	
	int type; // 类型
	size_t pos;  // 开始的位置
};
```
####单元测试(使用google C++ test)[入门文档](https://github.com/google/googletest/blob/master/googletest/docs/Primer.md)
```c++
class leptjsontest :public ::testing::Test{
protected:
	   virtual void SetUp(){}
	   virtual void TearDown() {}
leptjson json; // 被测试的对象
};

TEST_F(leptjsontest, leptnull){
	EXPECT_EQ(LEPT_PARSE_OK, json.parse("null"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
}

TEST_F(leptjsontest, leptboolean){
	EXPECT_EQ(LEPT_PARSE_OK, json.parse("true"));
	EXPECT_EQ(LEPT_TRUE,json.gettype());
	EXPECT_EQ(LEPT_PARSE_OK, json.parse("false"));
	EXPECT_EQ(LEPT_TRUE,json.gettype());
}

TEST_F(leptjsontest, leptinvalid){
	EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse("tru"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
	EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse("n"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
	EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse("fase"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
}

TEST_F(leptjsontest, leptrootnotsingular){
	EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, json.parse("false s"));
	EXPECT_EQ(LEPT_NULL, json.gettype());
}
```

#####使用google test遇到的一些问题：
1. 编译源码
按照文档中所述，要先编译google test源码：
	```sh
	~/googletest$: cd googletest/make  #我使用make所以要用make文件夹的makefile来编译
	~/googletest/googletest/make$: make
	#运行测试
	~/googletest/googletest/make$: ./sample1_unittest
	```

2. 写单元测试的makefile。下面是一个模板，只要改路径就可以了。
```makefile
# A sample Makefile for building Google Test and using it in user
# tests.  Please tweak it to suit your environment and project.  You
# may want to move it to your project's root directory.
#
# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.

# Please tweak the following variable definitions as needed by your
# project, except GTEST_HEADERS, which you can use in your own targets
# but shouldn't modify.

# Points to the root of Google Test, relative to where this file is.
# Remember to tweak this if you move this file.
#gtest文件夹所在路径
GTEST_DIR = /home/googletest/googletest

# Where to find user code.
#我的测试源码的路径
USER_DIR=.

# Flags passed to the preprocessor.
#预编译的头文件
CPPFLAGS+=-I $(GTEST_DIR)/include

# Flags passed to the C++ compiler.
#g++参数
CXXFLAGS+=-g -Wall -Wextra -std=c++11

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
#你的单元测试可执行文件的名字
TESTS=leptjson_test

# All Google Test headers.  Usually you shouldn't change this
# definition.
#注意'\'表示下一行接着上一行，和#define的'\'同理
GTEST_HEADERS=$(GTEST_DIR)/include/gtest/gtest.h \
				$(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

all : $(TESTS)

clean :
		rm -f $(TESTS) gtest.a gtest_main.a *.o

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_=$(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
gtest-all.o : $(GTEST_SRCS_)
		$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
		$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
		$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
		$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

leptjson.o : $(USER_DIR)/leptjson.cc $(USER_DIR)/leptjson.h
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/leptjson.cc

test.o : $(USER_DIR)/test.cc $(USER_DIR)/leptjson.h $(GTEST_HEADERS)
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/test.cc

#$@指leptjson_test   $^指冒号后面的
leptjson_test : leptjson.o  test.o gtest_main.a
#注意：-lpthread要放在gtest_main.a 之后，否则出错
#		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -lpthread -o $@
```
**遇到的错误的解答**

[模板](https://stackoverflow.com/questions/37382804/project-makefile-problems-with-google-test)
[-lpthread](https://stackoverflow.com/questions/10315016/error-during-making-gtest)
每一行的开头不能为空格，而是tab键。
[No rule to make target 'vertex.cpp', needed by 'vertex.o'.  Stop.](https://stackoverflow.com/questions/834748/gcc-makefile-error-no-rule-to-make-target)
3. 只测试函数可以使用`TEST(testCaseName, testName)`，不同的test case测试相同的对象时，可以使用`test fixture`，即我上面的单元测试使用的，
当使用`test fixture`时，我们要使用`TEST_F()`代替`TEST()`。

