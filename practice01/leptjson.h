#ifndef LEPTJSON_H__
#define LEPTJSON_H__
#include <string>
using namespace std;

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
	leptjson();
	int  parse(const string& json); // 解析json文本，目前只有null，boolean
	int  gettype();  // 获取json类型
private:
    void  parse_whitespace(const string& json);
    int  parse_value(const string& json);
    int  parse_null(const string& json);
    int  parse_true(const string& json);
    int  parse_false(const string& json);

	int   type;
    size_t pos; // 解析开始的位置
};
#endif
