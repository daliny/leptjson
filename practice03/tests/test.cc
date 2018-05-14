#include "leptjson.h"
#include "gtest/gtest.h"

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

       void StringTester(const string& expect, const string& s)
       {
          EXPECT_EQ(LEPT_PARSE_OK, json.parse(s)) << s;
	      EXPECT_EQ(LEPT_STRING, json.gettype()) << s;
          EXPECT_EQ(expect, json.getstring()) << s;
       }

       void InvalidTester(const string& s)
       {
	      EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.parse(s)) << s;
          EXPECT_EQ(LEPT_NULL_PTR, json.gettype()) << s;
       }

       void NotsingularTester(const string& s)
       {
	      EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, json.parse(s)) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, json.gettype()) << s;
       }

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
   	InvalidTester("\"onef");

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
    NotsingularTester("\"yes\" k");
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

int main(int argc, char **argv)
{
     ::testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
}
