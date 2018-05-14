#include "leptjson.h"
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>

leptjson::leptjson():v(), pos(0){}

leptjson::value::value():n(0.0), type(LEPT_NULL){}

void leptjson::init()
{
  pos = 0;
  //memset(&v, 0, sizeof(value));
  v.n = 0.0;
  v.type = LEPT_NULL;
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
      v.type = LEPT_NULL;
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
    case 'n':   return parse_literal(json, "null", LEPT_NULL);
    case 't':   return parse_literal(json, "true", LEPT_TRUE);
    case 'f':   return parse_literal(json, "false", LEPT_FALSE);
    default :   return parse_number(json);
  }
}

double leptjson::getnumber()
{
  return v.n;
}

#define NUM0TO9(ch) \
  ((ch) >= '0' && (ch) <= '9')

int leptjson::parse_number(const string& json)
{
  /*TODO*/
  size_t start = pos;
  size_t size = json.size();

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
    v.n = strtod(ptrs, &ptrd);
    
    if(errno == ERANGE && (v.n == HUGE_VAL || v.n == -HUGE_VAL)) // 数值太大，上溢
      return LEPT_PARSE_NUMBER_TOO_BIG;

    if(ptrd == ptrs) // 无效值
      return LEPT_PARSE_INVALID_VALUE;

    pos = pos + ptrd - ptrs;
    v.type = LEPT_NUMBER;
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
  v.type = type;

  return LEPT_PARSE_OK;
}

int  leptjson::gettype()  // 获取json类型
{
  return v.type;
}
