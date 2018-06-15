#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <string>
#include <memory>

using namespace std;

namespace json{
  enum{ // 返回状态值
	LEPT_PARSE_OK = 0,                // 解析成功
    LEPT_PARSE_EXPECT_VALUE,          // 只含有空白
    LEPT_PARSE_INVALID_VALUE,         // 无效值，非null，boolean字面量
    LEPT_PARSE_ROOT_NOT_SINGULAR,     // 若在值和空白之后还有其他字符
    LEPT_PARSE_NUMBER_TOO_BIG,        // 数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK,   // 缺右引号
    LEPT_PARSE_INVALID_STRING_ESCAPE, // 无效转义
    LEPT_PARSE_INVALID_STRING_CHAR,    // 非法字符
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE,
    LEPT_PARSE_MISS_VAL,
    LEPT_PARSE_MISS_COMMA,
    LEPT_PARSE_MISS_RBRKT
  };
  enum{ // JSON类型
	LEPT_NULL_PTR, LEPT_NULL, LEPT_FALSE, LEPT_TRUE, 
    LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
  };


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

  class leptjson{
  public:

	  leptjson();
      ~leptjson();
      int   geterr();
      value parse(const string& json); // 解析json文本
  private:
      void init();
      void parse_whitespace(const string& json);

      value  parse_value(const string& json);
      value  parse_literal(const string& json, const string& literal, int type);
      value  parse_number(const string& json);
      value  parse_string(const string& json);
      void encode_utf8(unsigned int u, string& s);
      int  parse_hex4(const string& json, unsigned int *u);
      value  parse_array(const string& json);

      int jerrno;
      size_t pos; // 解析开始的位置
  };
}
#endif
