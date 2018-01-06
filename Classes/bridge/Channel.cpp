#include "Channel.h"   
#include "ChannelEmitter.h"
Channel* Channel::m_pInstance = NULL;
LUA_FUNCTION Channel::m_channel_event_handler;

#if(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include <jni.h>
#include "platform/android/jni/JniHelper.h"
#include <android/log.h>
#endif
 
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
     #include "weixinSDK.h"
#elif(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
extern "C"
{
   /*
    java 调用 C++  java使用的是 public static native void LoginCallBack(int code, long value, String msg, String msgEx, String msgEx2); native关键字
    注意参数的使用
    */
   void Java_org_cocos2dx_lua_AppActivity_LoginCallBack(JNIEnv *env, jobject thiz, jint code, jlong value, jstring msg, jstring msgEx, jstring msgEx2)
   {
      const char * strMsg = env->GetStringUTFChars(msg, NULL);
      const char * strMsgEx = env->GetStringUTFChars(msgEx, NULL);
      const char * strMsgEx2 = env->GetStringUTFChars(msgEx2, NULL);
 
	  Channel::getInstance()->emit(EVENT_bridge_login, code, value, strMsg, strMsgEx, strMsgEx2);
 
      env->ReleaseStringUTFChars(msg, strMsg);
      env->ReleaseStringUTFChars(msgEx, strMsgEx);
   }
}
#endif
 
int Channel::register_lua(lua_State* L)
{
   tolua_open(L);
   tolua_module(L, NULL, 0);
   tolua_beginmodule(L, NULL);
   tolua_function(L, "c_channel_login", c_channel_login);
   tolua_function(L, "c_bind_channel_event", c_bind_channel_event);
   tolua_function(L, "c_channel_pay", c_channel_pay);
   tolua_function(L, "c_invite", c_invite);
   tolua_endmodule(L);
   return 0;
}
 
int Channel::c_channel_login(lua_State*L)
{
	Channel::getInstance()->login(L);
   return 0;
}
 
int Channel::c_channel_pay(lua_State*L)
{
   log("Channel::c_channel_pay");
   Channel::getInstance()->pay(L);
   return 0;
}

int Channel::c_invite(lua_State*L)
{
	log("Channel::c_invite");
	Channel::getInstance()->invite(L);
	return 0;
}
 
int Channel::c_bind_channel_event(lua_State*L)
{
   LUA_FUNCTION nHandler = toluafix_ref_function(L, 1, 0);
   Channel::m_channel_event_handler = nHandler;
   return 0;
}
 
void Channel::releaseInstance()
{
   if (m_pInstance)
      delete m_pInstance;
   m_pInstance = NULL;
}
 
Channel* Channel::getInstance()
{
   if (!m_pInstance)
   {
	   m_pInstance = new Channel();
   }
   return m_pInstance;
}
 
/*C++调用lua 函数*/
void Channel::emit(const char* eventName, int code, long value, const char* msg, const char* msgEx, const char* msgEx2)
{
   ChannelEmitter::getInstance()->emit(eventName, "%d%l%s%s%s", code, value, msg, msgEx, msgEx2);
   return;
}

int Channel::login(lua_State*L)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	weixinSDK::create()->sendAuthRequest();
#elif(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
   JniMethodInfo t;
 
   if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/lua/AppActivity",
      "login", "()V")) {
 
      t.env->CallStaticVoidMethod(t.classID, t.methodID);
      t.env->DeleteLocalRef(t.classID);
   }
#endif
 
   return 0;
}
 
//add logout code here
int Channel::pay(lua_State*L)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
 //audioRecord::create()->pay();
#elif(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
   const char* partern = "(Ljava/lang/String;ILjava/lang/String;)V";
 
   std::string orderId = tolua_tostring(L,1, 0);
   int amount = tolua_tonumber(L,2, 0);
   std::string productName = tolua_tostring(L,3, 0);
 
   JniMethodInfo t;
   if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/lua/AppActivity", "pay" , partern))
   {
      jstring jorderId = t.env->NewStringUTF(orderId.c_str());
      jstring jproductName = t.env->NewStringUTF(productName.c_str());
      t.env->CallStaticVoidMethod(t.classID, t.methodID, jorderId, amount, jproductName);
      t.env->DeleteLocalRef(jorderId);
      t.env->DeleteLocalRef(jproductName);
      t.env->DeleteLocalRef(t.classID);
   }
#endif
 
   return 0;
}

int Channel::invite(lua_State*L)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
	//audioRecord::create()->pay();
#elif(CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
	JniMethodInfo t;

	if (JniHelper::getStaticMethodInfo(t, "org/cocos2dx/lua/AppActivity",
		"invite", "()V")) {

		t.env->CallStaticVoidMethod(t.classID, t.methodID);
		t.env->DeleteLocalRef(t.classID);
	}
#endif

	return 0;
}
