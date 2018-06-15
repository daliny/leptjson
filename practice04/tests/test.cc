#include "leptjson.h"
#include "gtest/gtest.h"

using namespace json;
class leptjsontest :public ::testing::Test{
protected:
	   virtual void SetUp(){}
	   virtual void TearDown() {}
       
       void NumberTester(const double expect, const string& s)
       {
          value v = json.parse(s);
	      EXPECT_EQ(LEPT_PARSE_OK, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NUMBER, v.gettype()) << s;
	      EXPECT_DOUBLE_EQ(expect, v.getnumber()) << s;
       }

       void StringTester(const string& expect, const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_OK, json.geterr()) << s;
	      EXPECT_EQ(LEPT_STRING, v.gettype()) << s;
          EXPECT_EQ(expect, v.getstring()) << s;
       }

       void InvalidTester(const string& s)
       {
          value v = json.parse(s);
	      EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.geterr()) << s;
          EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }

       void NotsingularTester(const string& s)
       {
          value v = json.parse(s);
	      EXPECT_EQ(LEPT_PARSE_ROOT_NOT_SINGULAR, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }

       void InvalidStrEscapeTester(const string& s)
       {
          value v = json.parse(s);
	      EXPECT_EQ(LEPT_PARSE_INVALID_STRING_ESCAPE, json.geterr()) << s;
          EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }

       void InvaildStrMissQuotTester(const string& s)
       {
          value v = json.parse(s);
	      EXPECT_EQ(LEPT_PARSE_MISS_QUOTATION_MARK, json.geterr()) << s;
          EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;

       }

       void InvalidStrCharTester(const string& s)
       {
          value v = json.parse(s);
	      EXPECT_EQ(LEPT_PARSE_INVALID_STRING_CHAR, json.geterr()) << s;
          EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }

       void ArrayTester(const string& expect, const string& s)
       {
          value v = json.parse(s);

          EXPECT_EQ(LEPT_PARSE_OK, json.geterr()) << s;
	      EXPECT_EQ(LEPT_ARRAY, v.gettype()) << s;
          EXPECT_EQ(expect, v.stringify()) << s;
       }
       
       void InvalidArrMissRBrktTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_MISS_RBRKT, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       
       void InvalidArrMissCommaTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_MISS_COMMA, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       void InvalidArrMissValTester(const string& s)
       {
          value v = json.parse(s);
          EXPECT_EQ(LEPT_PARSE_INVALID_VALUE, json.geterr()) << s;
	      EXPECT_EQ(LEPT_NULL_PTR, v.gettype()) << s;
       }
       leptjson json; // 被测试的对象
};

TEST_F(leptjsontest, leptnull){
    value v = json.parse("null");
	EXPECT_EQ(LEPT_PARSE_OK, json.geterr());
	EXPECT_EQ(LEPT_NULL, v.gettype());
}

TEST_F(leptjsontest, leptboolean){
    value v = json.parse("true");
	EXPECT_EQ(LEPT_PARSE_OK, json.geterr());
	EXPECT_EQ(LEPT_TRUE, v.gettype());

    v = json.parse("false");
	EXPECT_EQ(LEPT_PARSE_OK, json.geterr());
	EXPECT_EQ(LEPT_FALSE, v.gettype());
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
    json.parse("1e309");
	EXPECT_EQ(LEPT_PARSE_NUMBER_TOO_BIG, json.geterr());
    json.parse("-1e309");
	EXPECT_EQ(LEPT_PARSE_NUMBER_TOO_BIG, json.geterr());
}

TEST_F(leptjsontest, leptrootnotsingular){
	NotsingularTester("false s");
   	NotsingularTester("1.1e1.43"); 
  	NotsingularTester("12INF1");
    NotsingularTester("\"yes\" k");
    NotsingularTester("[[true]]]");
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
  InvaildStrMissQuotTester("\"fdre");
}

TEST_F(leptjsontest, leptarray)
{
  ArrayTester("[]", "[         ]");
  ArrayTester("[true,12.3,\"HELLO\",null]", "[   true  ,12.3, \"HELLO\"  , null ]");
  ArrayTester("[true,[1,\"great\"]]", "[ true, [1,  \"great\"   ]    ]");
  ArrayTester("[true,[2,false,\"\",[]],null]", "[true, [2, false , \"\",[  ]], null ]");
}

TEST_F(leptjsontest, leptarrayinvalid)
{
  InvalidArrMissRBrktTester("[true");
  InvalidArrMissValTester("[true,]");
  InvalidArrMissCommaTester("[\"fantastic\"[123.0]");
}

GTEST_API_ int main(int argc, char **argv)
{
     testing::InitGoogleTest(&argc, argv);
     return RUN_ALL_TESTS();
}
