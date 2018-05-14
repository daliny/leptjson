#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <string>
#include <memory>

using namespace std;

enum{ // 返回状态值
	LEPT_PARSE_OK = 0,                // 解析成功
    LEPT_PARSE_EXPECT_VALUE,          // 只含有空白
    LEPT_PARSE_INVALID_VALUE,         // 无效值，非null，boolean字面量
    LEPT_PARSE_ROOT_NOT_SINGULAR,     // 若在值和空白之后还有其他字符
    LEPT_PARSE_NUMBER_TOO_BIG,        // 数字太大
    LEPT_PARSE_MISS_QUOTATION_MARK,   // 缺右引号
    LEPT_PARSE_INVALID_STRING_ESCAPE, // 无效转义
    LEPT_PARSE_INVALID_STRING_CHAR    // 非法字符
};
enum{ // JSON类型
	LEPT_NULL_PTR, LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING
};


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
public:
	leptjson();
	int  parse(const string& json); // 解析json文本，目前只有null，boolean
	int  gettype();  // 获取json类型
    double getnumber(); // 获取数字
    string getstring(); // 获取字符串
private:
    void init();
    void parse_whitespace(const string& json);
    int  parse_value(const string& json);
    int  parse_literal(const string& json, const string& literal, int type);
    int  parse_number(const string& json);
    int  parse_string(const string& json);

    unique_ptr<value> v; // 指向value结构体的指针
    size_t pos; // 解析开始的位置
};
#endif
