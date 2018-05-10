#include "leptjson.h"
#include <cassert>

leptjson::leptjson():type(LEPT_NULL), pos(0){}
int  leptjson::parse(const string& json) // 解析json文本，目前只有null，boolean
{
  /*TODO*/
  int ret = 0;
  
  pos = 0;
  type = LEPT_NULL;

  parse_whitespace(json);
  if((ret = parse_value(json)) == LEPT_PARSE_OK)
  {
    parse_whitespace(json);
    if(pos != json.size())
    {
      type = LEPT_NULL;
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
    case 'n':   return parse_null(json);
    case 't':   return parse_true(json);
    case 'f':   return parse_false(json);
    default :   return LEPT_PARSE_INVALID_VALUE;
  }
}

int  leptjson::parse_null(const string& json)
{
  assert(json[pos] == 'n');
  if(json.size() < 4 || json.compare(0, 4, "null"))
    return LEPT_PARSE_INVALID_VALUE;
  pos += 4;
  type = LEPT_NULL;
  return LEPT_PARSE_OK;
}

int  leptjson::parse_true(const string& json)
{
  assert(json[pos] == 't');
  if(json.size() < 4 || json.compare(0, 4, "true"))
    return LEPT_PARSE_INVALID_VALUE;
  pos += 4;
  type = LEPT_TRUE;
  return LEPT_PARSE_OK;
}

int  leptjson::parse_false(const string& json)
{
  assert(json[pos] == 'f');
  if(json.size() < 5 || json.compare(0, 5, "false"))
    return LEPT_PARSE_INVALID_VALUE;
  pos += 5;
  type = LEPT_FALSE;
  return LEPT_PARSE_OK;
}

int  leptjson::gettype()  // 获取json类型
{
  /*TODO*/
  return type;
}
