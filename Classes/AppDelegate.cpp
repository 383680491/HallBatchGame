#include "AppDelegate.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include "audio/include/SimpleAudioEngine.h"
#include "cocos2d.h"
#include "scripting/lua-bindings/manual/lua_module_register.h"
#include "socket/SockForLua.h"
#include "hotUpdate/lua_hot_update_sdk.h"
#include "pb.c"
#include "utils/cforLua.h"


using namespace CocosDenshion;

USING_NS_CC;
using namespace std;

AppDelegate::AppDelegate()
{
}

AppDelegate::~AppDelegate()
{
    SimpleAudioEngine::end();

#if (COCOS2D_DEBUG > 0) && (CC_CODE_IDE_DEBUG_SUPPORT > 0)
    // NOTE:Please don't remove this call if you want to debug with Cocos Code IDE
    RuntimeEngine::getInstance()->end();
#endif

}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages, 
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching()
{
    // set default FPS
    Director::getInstance()->setAnimationInterval(1.0 / 60.0f);

    // register lua module
    auto engine = LuaEngine::getInstance();
    ScriptEngineManager::getInstance()->setScriptEngine(engine);
    lua_State* L = engine->getLuaStack()->getLuaState();
    lua_module_register(L);

    register_all_packages();
	SockForLua::instance->register_socketforlua(L);

	luaopen_pb(L);
	register_all_hot_update(L);
	CFORLUA::getInstance()->register_lua(L);

   //LuaStack* stack = engine->getLuaStack();
	//stack->setXXTEAKeyAndSign(CODE_KEY, strlen(CODE_KEY), CODE_SIGN, strlen(CODE_SIGN));

    //register custom function
    //LuaStack* stack = engine->getLuaStack();
    //register_custom_function(stack->getLuaState());

    /*if (engine->executeScriptFile("src/main.lua"))
    {
        return false;
    }*/

	doSearchPath();

    return true;
}

void AppDelegate::doSearchPath()
{
	auto engine = LuaEngine::getInstance();
	string wirtePath = FileUtils::getInstance()->getWritablePath();
	string hotPath = wirtePath + "cache/hot/";

	if (FileUtils::getInstance()->isDirectoryExist(hotPath))
	{
		CCLOG("cache/hot/ exist");
		if (FileUtils::getInstance()->isFileExist(wirtePath + "cache/hot/versioninfo.json"))
		{
			CCLOG("versioninfo exist");
			string cacheData = FileUtils::getInstance()->getStringFromFile(wirtePath + "cache/hot/versioninfo.json");
			string resData = FileUtils::getInstance()->getStringFromFile("res/versioninfo.json");
			//CCLOG("data = %s", cacheData.c_str());

			cJSON *cacheDataJson = cJSON_Parse(cacheData.c_str());
			cJSON *resDataJson = cJSON_Parse(resData.c_str());

			cJSON* cacheBuildNode = cJSON_GetObjectItem(cacheDataJson, "build");
			cJSON* resBuildNode = cJSON_GetObjectItem(resDataJson, "build");

			CCLOG("cacheBuildNode build = %d", cacheBuildNode->valueint);
			CCLOG("resBuildNode build = %d", resBuildNode->valueint);

			if (resBuildNode->valueint > cacheBuildNode->valueint){ //如果本地版本比cache版本大 则意味着有以前的版本被新版本覆盖安装了，这种情况直接删掉cache/hot
				if (FileUtils::getInstance()->removeDirectory(wirtePath + "cache/hot/"))
				{
					CCLOG("cache/hot delete success");
				}
				else{
					CCLOG("WARNING : \n\n cache/hot delete fail \n\n");
				}
			}

			cJSON_Delete(cacheDataJson);
			cJSON_Delete(resDataJson);
		}

		if (FileUtils::getInstance()->isFileExist(wirtePath + "cache/hot/src/main.lua"))
		{
			CCLOG("start_with cache/hot/src/main.lua");
			engine->executeScriptFile((wirtePath + "cache/hot/src/main.lua").c_str());
			return;
		}
	}

	engine->executeScriptFile("src/hall/src/main.lua");
}


// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground()
{
    Director::getInstance()->stopAnimation();

    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground()
{
    Director::getInstance()->startAnimation();

    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
}
