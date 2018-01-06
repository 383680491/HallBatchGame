#ifndef _C_CHANNEL_H_
#define _C_CHANNEL_H_

#include "cocos2d.h"
using namespace cocos2d;

#include "extensions/ExtensionMacros.h"   
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/CCLuaValue.h"
#include "cocos-ext.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h" 
 
class Channel
{
   Channel(){};
public:
	static Channel* getInstance();
	void releaseInstance();

	int register_lua(lua_State* tolua_S);

	/*lua����C++�ӿ� c_channel_login*/
	static int c_channel_login(lua_State*L);             
	static int c_channel_pay(lua_State*L);
	static int c_invite(lua_State*L);

	/*c_bind_channel_event ����ص�  ֮��C++����luaȫ��ʹ��������*/
	static int c_bind_channel_event(lua_State*L);       

public:
	int login(lua_State*L);
	int pay(lua_State*L);
	int invite(lua_State*L);
	void emit(const char* eventName, int code, long value, const char* msg, const char* msgEx, const char* msgEx2);
public:
	static Channel* m_pInstance;
	static LUA_FUNCTION m_channel_event_handler;
};
#endif //Channel_H