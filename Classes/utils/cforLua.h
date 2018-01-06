#ifndef CFORLUA_H
#define CFORLUA_H

#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"

class CFORLUA
{
    static CFORLUA*m_pCForLua;
    CFORLUA(){};
public: 
	static CFORLUA*getInstance(); 
    void releaseInstance();
	int register_lua(lua_State* tolua_S); 

 	static int c_getMd5(lua_State*L);
	static int c_parseUTF8(lua_State* L); 
	static int c_UrlEncode(lua_State* L);
	static int c_UrlDecode(lua_State* L); 
	static int c_serverTimeStr(lua_State* L);
	static int c_checkChar(lua_State* L);
	static int c_GetByteNumOFWord(lua_State* L);
	static int c_getByteLen(lua_State* L);
	static int getTimeInMilliseconds(lua_State* L);
	static int gettime(lua_State* L);
};
#endif //VERSIONMANAGER_H
