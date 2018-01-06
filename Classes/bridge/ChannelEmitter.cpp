#include "ChannelEmitter.h"
#include <string> 
ChannelEmitter*ChannelEmitter::m_pCEventEmitter = nullptr;
#include "Channel.h"

ChannelEmitter* ChannelEmitter::getInstance()
{
   if (!m_pCEventEmitter)
   {
      m_pCEventEmitter = new ChannelEmitter();
   }
   return m_pCEventEmitter;
}

ChannelEmitter::ChannelEmitter()
{
}
 
#define MAX_INPUT_ARGS 100
void ChannelEmitter::emit(const char* event_id, const char* format, ...)
{
   unsigned int format_str_len = strlen(format);
   assert(format_str_len <= MAX_INPUT_ARGS);
   LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
   stack->pushString(event_id);
   va_list argp;
   unsigned int ptag = 0;
   va_start(argp, format);
   int argnum = 1;//参数个数已经有event_id了
   while (ptag + 1 < format_str_len)
   {
 
      int para_int;
      long para_long;
      char para_str[MAX_INPUT_ARGS];
      std::string test;
      char target = *(format + ptag + 1);
      switch (target)
      {
 
      case 'l':
         para_long = va_arg(argp, long);
         printf("long参数 %l\n", para_long);
         stack->pushLong(para_long);
         argnum++;
         break;
      case 'd':
         para_int = va_arg(argp, int);
         printf("整形参数 %d\n", para_int);
         stack->pushInt(para_int);
         argnum++;
         break;
      case 's':
         test = va_arg(argp, char*);
         printf("%s\n", test.c_str());
         stack->pushString(test.c_str());
         argnum++;
         break;
      default:
         printf("%c", target);
         printf("字符类型错误!");
         assert(false);
         break;
      }
      ptag += 2;
   }
   va_end(argp);
   //在这里直接将参数传给了lua了
   stack->executeFunctionByHandler(Channel::m_channel_event_handler, argnum);
}