# 从零开始学习JSON库（二）

* LINDA

* 2018/5/10

## 代码重构
practice01中的`parse_true(), parse_null, parse_false()`有重复代码，我们可以将它们合并为一个`parse_literal()`，减少代码量。
```c++
class leptjson{
...
private:
    // json为json文本，literal为要判断的字面量
	int parse_literal(string& json, string& literal);
	...
};
```
相应地，要修改`parse_value()`
```c++
int leptjson::parse_value(string& json)
{
	auto ch = json[0];
	switch(ch)
	{
		case 'n': return parse_literal(json, "null");
		case 't': return parse_literal(json, "true");
		case 'f': return parse_literal(json, "false");
		default : return LEPT_PARSE_INVALID_VALUE;
	}
}
```
## 添加数字解析

1. **json数字的语法：** 
	
	```
	number = [ "-" ] int [ frac ] [ exp ]
	int = "0" / digit1-9 *digit
	frac = "." 1*digit
	exp = ("e" / "E") ["-" / "+"] 1*digit
	```

	 >number 是以十进制表示，它主要由 4 部分顺序组成：负号、整数、小数、指数。只有整数是必需部分。注意和直觉可能不同的是，正号是不合法的。

	>整数部分如果是 0 开始，只能是单个 0；而由 1-9 开始的话，可以加任意数量的数字（0-9）。也就是说，0123 不是一个合法的 JSON 数字。

	>小数部分比较直观，就是小数点后是一或多个数字（0-9）。

    >JSON 可使用科学记数法，指数部分由大写 E 或小写 e 开始，然后可有正负号，之后是一或多个数字（0-9）。 
   
   ![number](https://img-blog.csdn.net/20180512152429651?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpbmRhX2Rz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
   图来自[ECMA-404](http://www.ecma-international.org/publications/files/ECMA-ST/ECMA-404.pdf)
按叶老师教程的说法，小数点之后必须有一位或多位数字才合法，但是，从这个图来看，个人感觉小数点之后也可以没有数字：
![小数点之后](https://img-blog.csdn.net/20180512152914301?watermark/2/text/aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L2xpbmRhX2Rz/font/5a6L5L2T/fontsize/400/fill/I0JBQkFCMA==/dissolve/70)
digit下面的线不是允许跳过的意思么？暂时不清楚，先放着，下面还是按叶老师的说法实现的。

2. **数据结构**
	```
	enum{..., LEPT_PARSE_NUMBER_TOO_BIG}; 
	enum{..., LEPT_NUMBER};// 增加一个number类型
	struct lept_value{
		int type; // 类型
		double n; // 存数字
	}; 
	```
	我把原来的type放到结构体中，这样class的成员变量就不会太多太乱。
	
3. 还是按教程2中的方法使用`strtod()`来将字符串转为数字。下次再自己实现一个。**函数声明**为
	```
	int parse_number(string& json);
	int getnumber();
	```
4.  **单元测试**：（借鉴[google test的例子](https://github.com/daliny/googletest/blob/master/googletest/samples/sample3_unittest.cc)）
因为测试数字需要重复用到三个宏，所以我将它们写在`leptjsontest`类中，这样就可以避免多余代码了，其中的`InvalidTester`函数和`NotsingularTester`函数同理。

	```c++
	class leptjsontest :public ::testing::Test{
	protected:
		   virtual void SetUp(){}
		   virtual void TearDown() {}
       
	       void NumberTester(const double expect, const string& s)
       {
		      EXPECT_EQ(LEPT_PARSE_OK, json.parse(s)) << s;
		      EXPECT_EQ(LEPT_NUMBER, json.gettype()) << s;
		      EXPECT_DOUBLE_EQ(expect, json.getnumber()) << s;
	       }

	       void InvalidTester(const string& s)
	       {
		      EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse(s)) << s;
	          EXPECT_EQ(LEPT_NULL, json.gettype()) << s;
	       }

	       void NotsingularTester(const string& s)
	       {
		      EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, json.parse(s)) << s;
		      EXPECT_EQ(LEPT_NULL, json.gettype()) << s;
	       }

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
		EXPECT_EQ(LEPT_FALSE,json.gettype());
	}

	TEST_F(leptjsontest, leptinvalid){
	    InvalidTester("tru");
		InvalidTester("tru");
		InvalidTester("n");
		InvalidTester("fase");
		InvalidTester("+0");
	    InvalidTester("+1");
	    InvalidTester(".123"); /* at least one digit before '.' */
		InvalidTester("1.");   /* at least one digit after '.' */
	    InvalidTester("0123");
	    InvalidTester("-00123");
		InvalidTester("0x123");
	  	InvalidTester("INF");
	  	InvalidTester("inf");
	  	InvalidTester("NAN");
	   	InvalidTester("nan");
	}

	TEST_F(leptjsontest, leptparsenumbertoobig)
	{
		EXPECT_EQ(LEPT_PARSE_NUMBER_TOO_BIG, json.parse("1e309"));
		EXPECT_EQ(LEPT_PARSE_NUMBER_TOO_BIG, json.parse("-1e309"));
	}	

	TEST_F(leptjsontest, leptrootnotsingular){
		NotsingularTester("false s");
	   	NotsingularTester("1.1e1.43"); 
	  	NotsingularTester("12INF1");
	}


	TEST_F(leptjsontest, leptnumber)	
	{
		NumberTester(1314.0, "1314");
		NumberTester(0.0, "0");
	    NumberTester(0.0, "-0");
	    NumberTester(0.0, "-0.0");
	    NumberTester(1.0, "1");
	    NumberTester(-1.0, "-1");
	    NumberTester(1.5, "1.5");
		NumberTester(-1.5, "-1.5");
		NumberTester(3.1416, "3.1416");
		NumberTester(1E10, "1E10");
		NumberTester(1e10, "1e10");
		NumberTester(1E+10, "1E+10");
		NumberTester(1E-10, "1E-10");
		NumberTester(-1E10, "-1E10");
	    NumberTester(-1e10, "-1e10");
		NumberTester(-1E+10, "-1E+10");
	    NumberTester(-1E-10, "-1E-10");
	    NumberTester(1.234E+10, "1.234E+10");
	    NumberTester(1.234E-10, "1.234E-10");
	    NumberTester(0.0, "1e-10000"); /* must underflow */
	    NumberTester(2.2250738585072009E-308, "2.2250738585072009E-308");
	    NumberTester(-2.2250738585072009E-308, "-2.2250738585072009E-308");
	    NumberTester(4.9e-324, "4.9e-324");
	    NumberTester(-4.9e-324, "-4.9e-324");
	    NumberTester(2.2250738585072014e-308, "2.2250738585072014e-308");
	    NumberTester(-2.2250738585072014e-308, "-2.2250738585072014e-308");
	    NumberTester(1.7976931348623157E308, "1.7976931348623157E308");
		NumberTester(-1.7976931348623157E308, "-1.7976931348623157E308");
		NumberTester( 4.9406564584124654e-324, "4.9406564584124654e-324" );
	    NumberTester(-4.9406564584124654e-324, "-4.9406564584124654e-324");
	}
	```
5.  **实现函数**
本来使用c++11提供的`double stod( const std::string& str, std::size_t* pos = 0 );`。但是，很多测试都通过不了，因为stod产生的数值上限和下限都比较小，如`"-4.9406564584124654e-324"`这样的数在`strtod`可以通过，但是在`stod`会出现下溢，并抛出`out_of_range`异常，同理，上溢也会抛出`out_of_range`异常，并且没有设置`errno`和`HUGE_VAL`。
**注意：pos**的**初值**并不会被函数使用。**ptr**也同理。
所以为了能够通过测试，我还是选用了`double std::strtod(str.c_str(), &ptr)`。

6. **makefile改写**
上次的makefile并没有将`test.c`文件和`*.obj`文件分开到不同的文件夹中。在改写过程中也遇到一点小问题，写一下：
<1> 增加变量`TESTS_DIR`和`OBJ_DIR`分别表示存放`test.c`文件和`*.obj`文件的文件夹路径。
<2> 在每个target前加上相应路径变量，make执行到g++命令时，会交给shell执行，每个g++ （-c选项的）命令后面都不能直接加生成文件的路径，所以只能在它执行完，将文件从当前文件夹`mv`到相应的文件夹中。而（-o选项的）g++命令后面可以加生成文件的路径，可以加上去或者直接使用`$@`, `$^`代替。命令执行完，文件已经在相应路径下了，不要再使用`mv`了。。。
```
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
GTEST_DIR = /home/lin/Downloads/C++/googletest/googletest

# Where to find user code.
USER_DIR = .

# Where to find test code.
TESTS_DIR = tests 

# where to find object code.
OBJ_DIR = obj

#BIN_DIR = bin

# Flags passed to the preprocessor.-I 选项是添加头文件所在路径
CPPFLAGS += -I $(GTEST_DIR)/include \
			-I .

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -std=c++11

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TESTS = leptjson_test

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/gtest.h \
				$(GTEST_DIR)/include/gtest/internal/*.h

# House-keeping build targets.

all : $(TESTS)

clean :
		rm -rf $(OBJ_DIR)
		rm -f  $(TESTS)
		mkdir $(OBJ_DIR)

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
$(OBJ_DIR)/gtest-all.o : $(GTEST_SRCS_)
		$(CXX) $(CPPFLAGS) -I $(GTEST_DIR) $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest-all.cc
		mv gtest-all.o $(OBJ_DIR)/gtest-all.o

$(OBJ_DIR)/gtest_main.o : $(GTEST_SRCS_)
#gtest_main.o : $(GTEST_SRCS_)
		$(CXX) $(CPPFLAGS) -I $(GTEST_DIR) $(CXXFLAGS) -c \
			$(GTEST_DIR)/src/gtest_main.cc
		mv gtest_main.o $(OBJ_DIR)/gtest_main.o

#$(OBJ_DIR)/gtest.a : $(OBJ_DIR)/gtest-all.o
$(OBJ_DIR)/gtest.a : $(OBJ_DIR)/gtest-all.o
		$(AR) $(ARFLAGS) $@ $^
		#mv gtest.a $(OBJ_DIR)/gtest.a 

#$(OBJ_DIR)/gtest_main.a : $(OBJ_DIR)/gtest-all.o $(OBJ_DIR)/gtest_main.o
$(OBJ_DIR)/gtest_main.a : $(OBJ_DIR)/gtest-all.o $(OBJ_DIR)/gtest_main.o
		$(AR) $(ARFLAGS) $@ $^

#因为$^已经指向$(OBJ_DIR)/gtest_main.a了，所以不用再mv；而上面的前两个使用-c
#选项，在命令最后不能加输出目标文件，所以要让它生成目标文件后，再mv
		#mv gtest-all.o $(OBJ_DIR)/gtest-all.o
		#mv gtest_main.o $(OBJ_DIR)/gtest_main.o
		#mv gtest.a $(OBJ_DIR)/gtest.a
		#mv gtest_main.a $(OBJ_DIR)/gtest_main.a
	
# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

$(OBJ_DIR)/leptjson.o : $(USER_DIR)/leptjson.cc $(USER_DIR)/leptjson.h
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $(USER_DIR)/leptjson.cc
		mv leptjson.o $(OBJ_DIR)/leptjson.o

$(OBJ_DIR)/test.o : tests/test.cc leptjson.h $(GTEST_HEADERS)
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c tests/test.cc 
		mv test.o $(OBJ_DIR)/test.o

#$@指leptjson_test   $^指冒号后面的
leptjson_test : $(OBJ_DIR)/leptjson.o  $(OBJ_DIR)/test.o $(OBJ_DIR)/gtest_main.a
#		$(CXX) $(CPPFLAGS) $(CXXFLAGS) -lpthread $^ -o $@
		$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -lpthread -o $@
```


----------
嗯......就这样 :-)
