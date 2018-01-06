#include "lua_hot_update_sdk.h"
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include "hotUpdate.h"

int hot_update_create(lua_State* tolua_S)
{
    int argc = 0;
    bool ok  = true;
    
    argc = lua_gettop(tolua_S) - 1;
    
    if (argc == 0)
    {
        if(!ok)
            return 0;
		hotUpdate* ret = hotUpdate::create();
        if (NULL == ret){
            CCLOG("hotUpdate ret is null");
        }
		object_to_luaval<hotUpdate>(tolua_S, "hotUpdate", (hotUpdate*)ret);
        return 1;
    }
    
    CCLOG("%s has wrong number of arguments: %d, was expecting %d\n ", "create",argc, 0);
    return 0;
}



int hot_update_release(lua_State* tolua_S)
{
    int argc = 0;
	hotUpdate* cobj = nullptr;
    bool ok  = true;
    
	cobj = (hotUpdate*)tolua_tousertype(tolua_S, 1, 0);
    
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'hot_update_release'", nullptr);
        return 0;
    }
    
    argc = lua_gettop(tolua_S)-1;
    if (argc == 0)
    {
        if(!ok)
            return 0;
        
        cobj->release();
        return 1;
    }
    
    CCLOG("has wrong number of arguments: %d, was expecting %d \n",argc, 0);
    return 0;
}


int hot_update_setHotPackageUrl(lua_State* tolua_S)
{
    int argc = 0;
	hotUpdate* cobj = nullptr;
    bool ok  = true;
    
	cobj = (hotUpdate*)tolua_tousertype(tolua_S, 1, 0);
    
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'hot_update_setHotPackageUrl'", nullptr);
        return 0;
    }
    
    argc = lua_gettop(tolua_S)-1;
    if (argc == 2)
    {
        if(!ok)
            return 0;

		std::string url = tolua_tostring(tolua_S, 2, 0);
		std::string name = tolua_tostring(tolua_S, 3, 0);
		cobj->setHotPackageUrl(url.c_str(), name.c_str());
        return 1;
    }
    
    CCLOG("has wrong number of arguments: %d, was expecting %d \n",argc, 0);
    return 0;
}

int hot_update_setDownLoadPath(lua_State* tolua_S)
{
	int argc = 0;
	hotUpdate* cobj = nullptr;
	bool ok = true;

	cobj = (hotUpdate*)tolua_tousertype(tolua_S, 1, 0);

	if (!cobj)
	{
		tolua_error(tolua_S, "invalid 'cobj' in function 'hot_update_setDownLoadPath'", nullptr);
		return 0;
	}

	argc = lua_gettop(tolua_S) - 1;
	if (argc == 1)
	{
		if (!ok)
			return 0;

		std::string path = tolua_tostring(tolua_S, 2, 0);
		cobj->setDownLoadPath(path.c_str());
		return 1;
	}

	CCLOG("has wrong number of arguments: %d, was expecting %d \n", argc, 0);
	return 0;
}

int hot_update_set_callback(lua_State* tolua_S)
{
    int argc = 0;
    hotUpdate* cobj = nullptr;
    bool ok  = true;
    
	cobj = (hotUpdate*)tolua_tousertype(tolua_S, 1, 0);
    
    if (!cobj)
    {
        tolua_error(tolua_S,"invalid 'cobj' in function 'share_sdk_showShareUI'", nullptr);
        return 0;
    }
    
    argc = lua_gettop(tolua_S)-1;
    if (argc == 3)
    {
        if(!ok)
            return 0;

        LUA_FUNCTION handler_success = toluafix_ref_function(tolua_S, 2, 0);
		LUA_FUNCTION handler_progress = toluafix_ref_function(tolua_S, 3, 0);
		LUA_FUNCTION handler_error = toluafix_ref_function(tolua_S, 4, 0);
		cobj->setCallback(handler_success, handler_progress, handler_error);
        
        return 1;
    }
    
    CCLOG("has wrong number of arguments: %d, was expecting %d \n",argc, 0);
    return 0;
}

int hot_update_requstHotPackage(lua_State* tolua_S)
{
	int argc = 0;
	hotUpdate* cobj = nullptr;
	bool ok = true;

	cobj = (hotUpdate*)tolua_tousertype(tolua_S, 1, 0);

	if (!cobj)
	{
		tolua_error(tolua_S, "invalid 'cobj' in function 'hot_update_requstHotPackage'", nullptr);
		return 0;
	}

	argc = lua_gettop(tolua_S) - 1;
	if (argc == 0)
	{
		if (!ok)
			return 0;

		cobj->requstHotPackage();
		return 1;
	}

	CCLOG("has wrong number of arguments: %d, was expecting %d \n", argc, 0);
	return 0;
}

int hot_update_uncompress(lua_State* tolua_S)
{
	int argc = 0;
	hotUpdate* cobj = nullptr;
	bool ok = true;

	cobj = (hotUpdate*)tolua_tousertype(tolua_S, 1, 0);

	if (!cobj)
	{
		tolua_error(tolua_S, "invalid 'cobj' in function 'hot_update_uncompress'", nullptr);
		return 0;
	}

	argc = lua_gettop(tolua_S) - 1;
	if (argc == 2)
	{
		if (!ok)
			return 0;

		std::string storagePath = tolua_tostring(tolua_S, 2, 0);
		std::string zipname = tolua_tostring(tolua_S, 3, 0);
		bool flag = cobj->uncompress(storagePath, zipname);
		tolua_pushboolean(tolua_S, (bool)flag);
		return 1;
	}

	CCLOG("has wrong number of arguments: %d, was expecting %d \n", argc, 0);
	return 0;
}


int register_hot_update(lua_State* tolua_S)
{
    tolua_usertype(tolua_S, "hotUpdate");
    tolua_cclass(tolua_S, "hotUpdate", "hotUpdate", "", nullptr);

    tolua_beginmodule(tolua_S, "hotUpdate");
    tolua_function(tolua_S,"create", hot_update_create);
	tolua_function(tolua_S, "release", hot_update_release);
	tolua_function(tolua_S, "setCallback", hot_update_set_callback);
	tolua_function(tolua_S, "setHotPackageUrl", hot_update_setHotPackageUrl);
	tolua_function(tolua_S, "setDownLoadPath", hot_update_setDownLoadPath);
	tolua_function(tolua_S, "requstHotPackage", hot_update_requstHotPackage); 
	tolua_function(tolua_S, "uncompress", hot_update_uncompress); 

    tolua_endmodule(tolua_S);

	std::string typeName = typeid(hotUpdate).name();
    g_luaType[typeName] = "hotUpdate";
    g_typeCast["hotUpdate"] = "hotUpdate";
    return 1;
}

TOLUA_API int register_all_hot_update(lua_State* L)
{
    tolua_open(L);

    tolua_module(L, nullptr, 0);
    tolua_beginmodule(L, nullptr);

	register_hot_update(L);

    tolua_endmodule(L);

    return 1;
}