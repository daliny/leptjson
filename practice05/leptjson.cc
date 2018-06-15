#include "leptjson.h"
#include <cassert>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <sstream>
#include <vector>
#include <map>

using namespace json;

leptjson::leptjson(): jerrno(0), pos(0){}
leptjson::~leptjson(){}

value::value(){}

value::value(int t, shared_ptr<void> c):type(t), context(c){}

inline void leptjson::init() // 初始化
{
  pos = 0;
  jerrno = 0;
}

value::~value(){}

value leptjson::parse(const string& json) // 解析json文本
{ 
  init();

  parse_whitespace(json);
  if(pos == json.size()){
    jerrno = LEPT_PARSE_EXPECT_VALUE;
    return value(LEPT_NULL_PTR); // 返回空 
  }
  auto v = parse_value(json);
  if(jerrno == LEPT_PARSE_OK)
  {
    parse_whitespace(json);
    if(pos != json.size())
    {
      jerrno = LEPT_PARSE_ROOT_NOT_SINGULAR;
      return value(LEPT_NULL_PTR); // 返回空 
    }
  }
  return v;
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

value leptjson::parse_value(const string& json)
{
  auto ch = json[pos];
  switch(ch)
  {
    case 'n' :  return parse_literal(json, "null", LEPT_NULL);
    case 't' :  return parse_literal(json, "true", LEPT_TRUE);
    case 'f' :  return parse_literal(json, "false", LEPT_FALSE);
    case '\"':  return parse_string(json);
    case '[' :  return parse_array(json);
    case '{' :  return parse_object(json);
    default  :  return parse_number(json);
  }
}

value leptjson::parse_object(const string& json)
{
  assert(json[pos] == '{');
  ++pos;
  parse_whitespace(json);
  if(json[pos] == '}')
  {
    ++pos;
    jerrno = LEPT_PARSE_OK;
    return value(LEPT_OBJECT);
  }

  int stat = KEY_LEFT_QUOTE; // 状态码，表示当前状态位置
  if(json[pos] != '\"')
  {
    jerrno = LEPT_PARSE_MISS_KEY;
    return value(LEPT_NULL_PTR);
  }
  
  stat = KEY_CHAR;
  string key = "";
  value v(LEPT_NULL_PTR);
  map<string, value> obj;
  while(++pos != json.size())
  {
    switch(stat)
    {
      case KEY_LEFT_QUOTE: key = ""; stat = KEY_CHAR;
      case KEY_CHAR: if(json[pos] == '\"')
                     {
                       stat = KEY_RIGHT_QUOTE;
                     }else key += json[pos];
                     break;
      case KEY_RIGHT_QUOTE: parse_whitespace(json);
                            if(pos == json.size() || json[pos] != ':')
                            {
                              jerrno = LEPT_PARSE_MISS_COLON;
                              return value(LEPT_NULL_PTR);
                            }else stat = COLON; break;
      case COLON: parse_whitespace(json);
                  if(pos == json.size())
                  {
                    jerrno = LEPT_PARSE_MISS_VAL;
                    return value(LEPT_NULL_PTR);
                  }else {
                    v = parse_value(json);
                    if(jerrno != LEPT_PARSE_OK)
                      return value(LEPT_NULL_PTR);
                    
                    parse_whitespace(json);
                    if(pos == json.size())
                    {
                      jerrno = LEPT_PARSE_MISS_RBRKT;
                      return value(LEPT_NULL_PTR);
                    }

                    obj[key] = v; // 插入键值对
                    if(json[pos] == '}')
                    {
                      ++pos;
                      jerrno = LEPT_PARSE_OK;
                      return value(LEPT_OBJECT, make_shared<map<string, value>>(obj));
                    }
                    if(json[pos] == ',')
                    {
                      stat = COMMA;
                      break;
                    }else {
                      jerrno = LEPT_PARSE_MISS_COMMA;
                      return value(LEPT_NULL_PTR);
                    }
                  }

      case COMMA: parse_whitespace(json);
                  if(pos == json.size() || json[pos] != '\"'){
                    jerrno = LEPT_PARSE_MISS_PAIR;
                    return value(LEPT_NULL_PTR);
                  }
                  stat = KEY_LEFT_QUOTE;
                  break;
      default:    jerrno = LEPT_PARSE_INVALID_VALUE;
                  return value(LEPT_NULL_PTR);
    }
  }

  switch(stat)
  {
    case KEY_LEFT_QUOTE:
    case KEY_CHAR:        jerrno = LEPT_PARSE_MISS_KEY;   break;
    case KEY_RIGHT_QUOTE: jerrno = LEPT_PARSE_MISS_COLON; break;
    case COLON:           jerrno = LEPT_PARSE_MISS_VAL;   break;
    case COMMA:           jerrno = LEPT_PARSE_MISS_PAIR;   break;
  }
  return value(LEPT_NULL_PTR);
}

value leptjson::parse_array(const string& json)
{
  assert(json[pos] == '[');

  ++pos;
  parse_whitespace(json);
  if(pos == json.size())
  {
    jerrno = LEPT_PARSE_MISS_RBRKT;
    return value(LEPT_NULL_PTR);
  }
  if(json[pos] == ']')
  {
    ++pos;
    jerrno = LEPT_PARSE_OK;
    return value(LEPT_ARRAY);
  }
  vector<value> a;
  while(1)
  {
    parse_whitespace(json);
    auto v = parse_value(json);
    if(jerrno == LEPT_PARSE_OK)
      a.push_back(v); // 压入每个value
    else{
      return value(LEPT_NULL_PTR);
    }
    parse_whitespace(json);
    if(pos == json.size())
    {
      jerrno = LEPT_PARSE_MISS_RBRKT; // 缺右括号
      return value(LEPT_NULL_PTR);
    }

    if(json[pos] == ',')
      ++pos;
    else if(json[pos] == ']'){
      ++pos;
      jerrno = LEPT_PARSE_OK;
      return value(LEPT_ARRAY, make_shared<vector<value>>(a));
    }else {
      jerrno = LEPT_PARSE_MISS_COMMA;
      return value(LEPT_NULL_PTR);
    }
  }
}

int leptjson::parse_hex4(const string& json, unsigned int *u)
{
  assert(json[pos] == 'u');
  unsigned int ul = 0;

  if(json.size() - pos <= 4)
    return LEPT_PARSE_INVALID_UNICODE_HEX;

  for(int i = 0; i < 4; ++i)
  {
    char ch = json[++pos];
    ul <<= 4;
    if(ch >= '0' && ch <= '9')
      ul += ch - '0';
    else if(ch >= 'A' && ch <= 'F')
      ul += ch - 'A' + 10;
    else if(ch >= 'a' && ch <= 'f')
      ul += ch - 'a' + 10;
    else 
      return LEPT_PARSE_INVALID_UNICODE_HEX;
  }

  *u = ul;
  return LEPT_PARSE_OK;
}

void leptjson::encode_utf8(unsigned int u, string& s)
{
  if(u < 0x80)
    s += u;
  else if(u < 0x800)
  {
    s += ((u >> 6)& 0x1F)|0xC0;
    s += (u&0x3F)|0x80;
  }else if(u < 0x10000)
  {
    s += ((u >> 12)&0xF)|0xE0;
    s += ((u >> 6)&0x3F)|0x80;
    s += (u&0x3F)|0x80;
  }else {
    assert(u <= 0x10FFFF);
    s += ((u >> 18)&0x7)|0xF0;
    s += ((u >> 12)&0x3F)|0x80;
    s += ((u >> 6)&0x3F)|0x80;
    s += (u & 0x3F)|0x80;
  }
}

value leptjson::parse_string(const string& json)
{
  assert(json[pos] == '\"');
  string s = "";
  unsigned int u = 0;
  //shared_ptr<string> p = nullptr;
  jerrno = LEPT_PARSE_OK; // 作为没有出错的标志
  while(++pos < json.size())
  { 
    switch(json[pos])
    {
      case '\"':
        ++pos;
        jerrno = LEPT_PARSE_OK;
        
        if(s == "")
          return value(LEPT_STRING);
        return value(LEPT_STRING, make_shared<string>(s));

      case '\\':
        if(pos++ == json.size()-1) // 已经是最后一个元素了，返回
        {
          jerrno = LEPT_PARSE_INVALID_STRING_CHAR;
          return value(LEPT_NULL_PTR); // 返回空 
        }
        
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
          case 'u': 
                     if((jerrno = parse_hex4(json, &u)) == LEPT_PARSE_OK)
                     {
                       if(u >= 0xD800 && u <= 0xDBFF)
                       {
                         if(json.size() - pos > 2 && 
                            json[++pos] == '\\' && json[++pos] == 'u')
                         {
                           unsigned int ul = 0;
                           if((jerrno = parse_hex4(json, &ul)) == LEPT_PARSE_OK)
                           {
                              if(ul >= 0xDC00 && ul <= 0xDFFF)
                              {
                                u = 0x10000 + ((u - 0xD800) << 10) + (ul - 0xDC00);
                              }else{
                                jerrno = LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                              }
                           }
                         }else 
                           jerrno = LEPT_PARSE_INVALID_UNICODE_SURROGATE;
                       }
                       if(jerrno == LEPT_PARSE_OK)
                       {
                         encode_utf8(u, s);
                       }
                     }
                     break;
                     
          default  : jerrno = LEPT_PARSE_INVALID_STRING_ESCAPE;
                     break;

        }
        break;
      default  :
        if(json[pos] < 0x20){
          jerrno = LEPT_PARSE_INVALID_STRING_CHAR;
          break;
        }
        s += json[pos];
        break;
    }
         
    if(jerrno != LEPT_PARSE_OK)
      return value(LEPT_NULL_PTR); // 出错，返回空
  }
  jerrno = LEPT_PARSE_MISS_QUOTATION_MARK;
  return value(LEPT_NULL_PTR);
}

#define NUM0TO9(ch) \
  ((ch) >= '0' && (ch) <= '9')

value leptjson::parse_number(const string& json)
{
  size_t start = pos;
  size_t size = json.size();
  double n = 0.0;

  if(json[start] == '-')
    start += 1;

  if(json[start] == '0')
  {
    if(start+1 != size && json[start+1] != '.' 
        && json[start+1] != 'e' && json[start+1] != 'E'){
      jerrno = LEPT_PARSE_INVALID_VALUE;
      return value(LEPT_NULL_PTR);
    }
  }

  const char *ptrs = json.c_str() + pos;
  char *ptrd = nullptr;
  if(json[size-1] != '.' && (NUM0TO9(json[pos]) || json[pos] == '-'))
  {
    n = strtod(ptrs, &ptrd);
    
    if(errno == ERANGE && (n == HUGE_VAL || n == -HUGE_VAL)) // 数值太大，上溢
      jerrno = LEPT_PARSE_NUMBER_TOO_BIG;
    else if(ptrd == ptrs) // 无效值
      jerrno = LEPT_PARSE_INVALID_VALUE;
    else {
      pos = pos + ptrd - ptrs;
    
      jerrno = LEPT_PARSE_OK;
      //auto p = shared_ptr<double>(new double(n));
      return value(LEPT_NUMBER, make_shared<double>(n));
    }
  }else
    jerrno = LEPT_PARSE_INVALID_VALUE;
  return value(LEPT_NULL_PTR);
}

value  leptjson::parse_literal(const string& json, const string& literal, int type)
{
  assert(json[pos] == literal[0]);
  size_t size = literal.size();
  if(json.size() < size || json.compare(pos, size, literal)){
    jerrno = LEPT_PARSE_INVALID_VALUE;
    return value(LEPT_NULL_PTR);
  }
  pos += size;
  
  jerrno = LEPT_PARSE_OK;
  if(type == LEPT_NULL)
    return value(type);
  bool b = (literal == "true");
    
  //auto p = shared_ptr<bool>(new bool(b));
  return value(type, make_shared<bool>(b));
}

int leptjson::geterr()
{
  return jerrno;
}

int  value::gettype()  // 获取json类型
{
  return type;
}

double value::getnumber()
{
  assert(context != nullptr);
  return *static_cast<double*>(context.get());
}

string value::getstring()
{
  string s = "";
  if(context != nullptr)
    s = *static_cast<string*>(context.get());
  return s;
}

string value::stringify_str()
{
  string s = "";

  string str = getstring();
  s = "\"";
  for(size_t i = 0; i != str.size(); ++i)
  {
    unsigned char ch = str[i];
    switch(ch)
    {
      case '\"': s += "\\\""; break;
      case '\\': s += "\\\\"; break;
      case '\b': s += "\\b";  break;
      case '\f': s += "\\f";  break;
      case '\n': s += "\\n";  break;
      case '\r': s += "\\r";  break;
      case '\t': s += "\\t";  break;
      default:
                 if(ch < 0x20){
                   ostringstream os;
                   os << "\\u" << right << hex << ch;
                   s += os.str();
                 }else
                   s += ch;
    }
  }
  s += "\"";
  return s;
}

string value::stringify_array()
{
  string s = "[]";
  if(context == nullptr)
    return s;

  s = "[";
  auto a = *static_cast<vector<value>*>(context.get());
  for(size_t i = 0; i < a.size(); ++i)
  {
    if(i != 0)
      s+= ",";
    s += a[i].stringify();
  }
  s += "]";
  return s;
}

string value::stringify_obj()
{
  string s = "{}";
  if(context == nullptr)
    return s;
  s = "{";
  auto obj = *static_cast<map<string, value>*>(context.get());
  int i = 0;
  for(auto elem : obj)
  {
    if(i != 0)
      s += ",";
    ++i;
    s += "\"";
    s += elem.first;
    s += "\"";
    s += ":";
    s += elem.second.stringify();
  }
  s += "}";
  return s;
}

string value::stringify()
{
  string s = "";
  ostringstream os;
  switch(type)
  { 
    case LEPT_NULL :  s += "null"; break;
    case LEPT_TRUE :  s += "true"; break;
    case LEPT_FALSE:  s += "false";break;
    case LEPT_NUMBER:
                      os << getnumber(); 
                      s += os.str(); break;
    case LEPT_STRING: s += stringify_str(); break;
    case LEPT_ARRAY:  s += stringify_array(); break;
    case LEPT_OBJECT: s += stringify_obj(); break;
  }
  return s;
}
