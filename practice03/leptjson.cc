#include "leptjson.h"
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>

leptjson::leptjson():v(nullptr), pos(0){}

value::value(int _type, double d):type(_type){
    n = d; // 这里不能使用构造函数，只能用赋值函数
}

value::value(int _type, string str):type(_type) {
  new(&s) string;   // 构造string信息结构
  s = str;
}

value::~value(){
  if(type == LEPT_STRING)
    s.~string();
}

void leptjson::init() // 初始化
{
  pos = 0;
  v = nullptr;
}

int  leptjson::parse(const string& json) // 解析json文本，目前只有null，boolean
{
  int ret = 0;
  
  init();

  parse_whitespace(json);
  if(pos == json.size())
    return LEPT_PARSE_EXPECT_VALUE;
  if((ret = parse_value(json)) == LEPT_PARSE_OK)
  {
    parse_whitespace(json);
    if(pos != json.size())
    {
      v = nullptr;
      return LEPT_PARSE_ROOT_NOT_SINGULAR;
    }
  }
  return ret;
}

void  leptjson::parse_whitespace(const string& json)
{
  assert(pos <= json.size());
  char ch = json[pos];
  while(pos != json.size() && (ch == '\n' || ch == '\t' || ch == ' ' || ch == '\r'))
  {
    ch = json[++pos];
  }
}

int  leptjson::parse_value(const string& json)
{
  auto ch = json[pos];
  switch(ch)
  {
    case 'n' :  return parse_literal(json, "null", LEPT_NULL);
    case 't' :  return parse_literal(json, "true", LEPT_TRUE);
    case 'f' :  return parse_literal(json, "false", LEPT_FALSE);
    case '\"':  return parse_string(json);
    default  :  return parse_number(json);
  }
}

int leptjson::parse_string(const string& json)
{/*TODO*/
  assert(json[pos] == '\"');
  string s = "";
  while(++pos < json.size())
  { 
    switch(json[pos])
    {
      case '\"':
        ++pos;
        v = unique_ptr<value>(new value(LEPT_STRING, s));
        return LEPT_PARSE_OK;

      case '\\':
        if(pos++ == json.size()-1) // 已经是最后一个元素了，返回
          return LEPT_PARSE_INVALID_STRING_CHAR;
        
        switch(json[pos])
        {
          case '\"': s += '\"'; break;
          case '\\': s += '\\'; break;
          case '/' : s += '/' ; break;
          case 'b': s += '\b'; break;
          case 'f': s += '\f'; break;
          case 'n': s += '\n'; break;
          case 'r': s += '\r'; break;
          case 't': s += '\t'; break;
          default  : return LEPT_PARSE_INVALID_STRING_ESCAPE;
        }
        break;
      default  :
        if(json[pos] < 0x20)
          return LEPT_PARSE_INVALID_STRING_CHAR;
        s += json[pos];
        break;
    }
  }
  return LEPT_PARSE_MISS_QUOTATION_MARK;
}

#define NUM0TO9(ch) \
  ((ch) >= '0' && (ch) <= '9')

int leptjson::parse_number(const string& json)
{
  size_t start = pos;
  size_t size = json.size();
  double n = 0.0;

  if(json[start] == '-')
    start += 1;

  if(json[start] == '0')
  {
    if(start+1 != size && json[start+1] != '.' 
        && json[start+1] != 'e' && json[start+1] != 'E')
      return LEPT_PARSE_INVALID_VALUE;
  }

  const char *ptrs = json.c_str() + pos;
  char *ptrd = nullptr;
  if(json[size-1] != '.' && (NUM0TO9(json[pos]) || json[pos] == '-'))
  {
    n = strtod(ptrs, &ptrd);
    
    if(errno == ERANGE && (n == HUGE_VAL || n == -HUGE_VAL)) // 数值太大，上溢
      return LEPT_PARSE_NUMBER_TOO_BIG;

    if(ptrd == ptrs) // 无效值
      return LEPT_PARSE_INVALID_VALUE;

    pos = pos + ptrd - ptrs;
    
    v = unique_ptr<value>(new value(LEPT_NUMBER, n));
    return LEPT_PARSE_OK;
  }
  return LEPT_PARSE_INVALID_VALUE;
}

int  leptjson::parse_literal(const string& json, const string& literal, int type)
{
  assert(json[pos] == literal[0]);
  size_t size = literal.size();
  if(json.size() < size || json.compare(0, size, literal))
    return LEPT_PARSE_INVALID_VALUE;
  pos += size;
  
  v = unique_ptr<value>(new value(type, 0.0));

  return LEPT_PARSE_OK;
}

int  leptjson::gettype()  // 获取json类型
{
  if(v == nullptr)
    return LEPT_NULL_PTR;
  return v->type;
}

double leptjson::getnumber()
{
  assert(v != nullptr);
  return v->n;
}

string leptjson::getstring()
{
  assert(v != nullptr);
  return v->s;
}

