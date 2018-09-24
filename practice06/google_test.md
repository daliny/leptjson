#介绍：为什么使用Google C++测试框架？

Google C++测试框架帮助你更好的写C++测试。

无论你是在Linux，Windows，或者Mac平台工作，只要你写C++代码，google测试就能帮到你。

因此，怎么写一个好的测试，并且如何让Google C++测试框架适用呢？我们相信：

1. 测试应该独立并且重复。debug一个因其他测试成功或失败的测试就是一种痛点。
Google C++测试框架将这些测试分离到每一个不同的对象。当一个测试失败，Google
C++测试框架允许你单独运行它来快速debugging。

2. 测试应该能够更好的被组织和反映到测试代码的结构上。google C++测试框架组织
那些相关的测试到一个测试用例（test cases）中，它们可以共享数据和子程序。这种常见的模式很
容易识别，并使测试易于维护。当人们交换projects和开始在一个新的代码基础工作时，这种一致性（consistency）非常有用。
3. 测试应该有可移植性和重用性。

#断言（assertion）

google test断言是一些类似于函数的宏定义。你通过用断言来测试一个类或函数的行为。当一个
断言失败时，google test会打印该断言的源码，行数定位，和一个失败信息。你也可以提供一个
自定义的失败信息，将会在google test的信息中出现。

断言成对出现，它们在一个函数中测试同一个事件，但是有不同的效果。`ASSERT_*`版本会在它们失败时，
生成致命错误，并停止(abort)当前函数。`EXPECT_*`版本生成非致命失败，它不会停止
当前函数。通常在测试中允许超过一个错误被报告时，`EXPECT_*`被作为首选。而如果当断言发生时，它意味着不能继续时，
你应该使用`ASSERT_*`。

因为一个`ASSERT_*`断言失败会马上返回，会跳过它后面的代码，它可能造成内存泄露。这依赖于泄露的性质，它可能或不可能
值得被修复--所以，请记住，如果你获得一个堆检查错误，要排除这种可能。

为了提供自定义(custom)的失败信息，简单地将它用`<<`操作符重定向给宏，或者是这个操作符的序列，例如：
``C++
ASSERT_EQ(x.size(), y.size()) << "Vectors x and y are of unequal length";

for(int i = 0; i < x.size(); ++i)
{
  EXPECT_EQ(x[i], y[i]) << "vectors x and y differ at index" << i;
}
``
任何能够被重定向给`ostream`都可以重定向给一个断言宏。特别是，C风格的字符串和`string`。如果一个宽字符串
（在windows上的wchar_t*, TCHAR*以UNICODE编码的，或者std::wstring）被重定向给一个断言，它将被翻译为UTF-8
然后打印。

##基础断言（Basic Assertions）
这些宏处理基本的true/false条件测试：
``
ASSERT_TRUE(condition); ASSERT_FALSE(condition):
EXPECT_TRUE(condition); EXPECT_FALSE(condition);
``

###二元比较

``
ASSERT_*(val1, val2);
EXPECT_*(val1, val2);
``
参数必须通过断言比较函数比较。
这些断言可以在用户自定义的类型下使用，但是仅当你为该类型定义了相应的比较操作符。
如果已定义了相应的比较操作符，选择使用`ASSERT_*`宏会更好，因为它们将打印出不仅是比较结果，
还有两个操作数。

参数总是只被计算一次。因此，在有副作用的参数（如i++）它也是可以的，然而，对于一般的C/C++函数，
这种参数的评估顺序是未定义的（如编译器是可以选择任意顺序的），所以你的代码应该不要依赖于任何
特定的顺序评估。

`ASSERT_EQ()`可以比较指针是否相等。如果使用在两个C风格字符串上，它测试的是它们是否在同一个地址上，
而不是拥有相同的值。因此，如果你要比较C风格的字符串的值，请使用`ASSERT_STREQ()`，它们将在后面说到。
特别的，断言一个C字符串是否为`NULL`，使用`ASSERT_STREQ(NULL, c_string)`，然而如果比较两个`string`对象，
你应该使用`ASSERT_EQ`.

这一节的宏可以使用于原生字符串和宽字符串对象（string和wstring）。

历史原因note: 在2016年2月前，`*_EQ`有一个方便调用它的形式`ASSERT_EQ(expected, actual)`，所以很多存在的代码
都使用了这个顺序。现在`*_EQ`也是这么使用的。

###C风格字符串比较（string comparison）

`NULL`指针和空字符串是不同的。

##简单测试（Simple Tests）

为了创建一个测试：
1. 使用`TEST()`宏来定义和命名一个测试函数。这些普通C++函数不返回一个值。
2. 在这个函数内，包含任何你想要的有效C++语句，使用各种google test断言去检查值。
3. 测试结果将被这些断言决定。如果在测试中有任何断言失败（致命的或非致命的），
或者是测试出错，整个测试就会失败。否则，就测试成功。
``C++
TEST(testCaseName, testName){
...test body...
}
``
`TEST()`参数从一般到特定。第一个参数是test case的名字，第二个参数是在这个
test case中一个test项的名字。两个名字必须是有效的C++标识符，并且它们不能包括
下划线(_). 这个测试的全名由它的test case和它自身的名字组成。来自不同test case的
测试可以有相同的自身名字。

##测试夹具：为多个测试使用相同的数据配置

如果你发现你自己写了两个或多个测试都操作相同的数据，你应该使用一个测试夹具（test fixture）。
它允许你为多个不同的测试重用对象的相同的配置。

为了创建一个fixture，只要：
1. 从`::testing::Test`继承一个class。以`protected:`或`public:`开始它的body作为我们将要从子类访问
的fixture成员。
2. 在类内，声明任何你将要使用的对象。
3. 如有必要，写一个默认构造函数或`SetUp()`函数来为每个测试准备对象。一个常见的错误是将`SetUp()`写成`Setup()`
--请不要让它发生在你身上。
4. 如有必要，写一个析构函数或`TearDown()`函数来释放任何资源你在`SetUp()`中分配的。
5. 如有需要，为你的测试定义用来共享的子程序。

当使用fixture时，使用`TEST_F()`代替`TEST()`，它允许你在test fixture中访问对象和子程序：
``C++
TEST_F(test_case_name, test_name){
...test body...
}
``

就像`TEST()`, 第一个参数是test case的名字，但是，`TEST_F()`的这个参数必须是test fixture类的名字。
你可能已经猜到: `_F`是对于fixture的。

不幸的是，C++宏系统不允许我们创建一个宏来处理test的类型。使用错误的宏会造成编译期错误。

所以，你必须先定义一个test fixture类在`TEST_F()`使用它之前。或者你将得到一个编译期错误
"virtual outside class declaration"
