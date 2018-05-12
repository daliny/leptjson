#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <string>
using namespace std;

enum{ // 返回状态值
	LEPT_PARSE_OK = 0,  // 解析成功
    LEPT_PARSE_EXPECT_VALUE,  // 只含有空白
    LEPT_PARSE_INVALID_VALUE, // 无效值，非null，boolean字面量
    LEPT_PARSE_ROOT_NOT_SINGULAR, // 若在值和空白之后还有其他字符
    LEPT_PARSE_NUMBER_TOO_BIG   // 数字太大
};
enum{ // JSON类型
	LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER
};

class leptjson{
public:
	leptjson();
	int  parse(const string& json); // 解析json文本，目前只有null，boolean
	int  gettype();  // 获取json类型
    double getnumber(); // 获取数字
private:
    void init();
    void parse_whitespace(const string& json);
    int  parse_value(const string& json);
    int  parse_literal(const string& json, const string& literal, int type);
    int  parse_number(const string& json);

	struct value{ // json值
      double n;
      int type;
      value();
    } v;
    size_t pos; // 解析开始的位置
};
#endif
