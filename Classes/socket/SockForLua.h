 
#ifndef __SockForLua__ 
#define __SockForLua__ 

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h" 
#ifdef __cplusplus
}
#endif 
#include <string> 
#include <mutex> 
#include "cocos2d.h"
using namespace cocos2d;
#include "extensions/ExtensionMacros.h"   
#include "scripting/lua-bindings/manual/tolua_fix.h"
#include "scripting/lua-bindings/manual/LuaBasicConversions.h"
#include "scripting/lua-bindings/manual/CCLuaValue.h"
#include "cocos-ext.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h" 

#include <stdio.h>
#include <vector>
#include <thread>  
#include "scripting/lua-bindings/manual/extension/lua_cocos2dx_extension_manual.h"
#include "../utils/cJSON/cJSON.h"
#ifndef WIN32 
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#endif  
#include "external/unzip/unzip.h"  
using namespace std;

#include "network/HttpClient.h"
using namespace cocos2d::network;

#define BUFFER_SIZE_RECV (1024*64)
#define BUFFER_SIZE_SEND (1024*64) 
#define SINGLE_BUFFER_SIZE 10240
#define SEND_DATA_LIST 100
#define BUFFERSIZE 102400
#define PACKAGE_LENGTH 15
#define ADLER_BASE 65521 
#include "ODSocket.h"

class SockForLua : public Ref
{
public:
	enum NetState
	{
		net_disconnect,       //断开连接
		net_connecting,       //与服务器连接中------
		net_connect_handle,   //握手 1
		net_connected,        //已经连接成功了 啦啦啦~~~~
	};

private:
	ODSocket* m_ODSocket;

	//当前通信状态
	NetState m_netstate;

	char* m_recvBuff;
	char* m_sendBuff;
	int m_sendLength;
	int m_recvLength;

	char m_buffer[BUFFER_SIZE_RECV];  //存储整条消息(服务器分发数据,多次接受)
	char m_single_buffer[BUFFER_SIZE_RECV];  //存储单条消息 

	int m_k;                          //已经使用的字节数  socket拿到数据后 放到m_buffer的后面  收到包后m_k = m_offset + contentLength
	int m_offset;            //数据流  记录上一次从m_buffer拿到的数据后的偏移量 即没有收到数据的时候的长度

public:
	std::string m_ip;
	int m_port;

	static SockForLua*  instance ;
	char *m_pbheaduffer;
	char *m_pbbodyduffer; 

	int m_nDataHandle;
	int m_statusHandle;
	bool m_bSupportIpv6;


public:
	SockForLua();  
	~SockForLua();

	static SockForLua* getInstance(); 
	void releaseInstace();

	void setIP(const char*ip);
	void setPort(int port);

	void startConnect();
	void connect();

	int connectServer();
	bool closeServer(bool removeIndicate = true);
	bool closeSocket();    

	int sendToServerPackage(cJSON* head,int headlen,char*body,int bodylen);

	//打包后发给服务器
	int encodePBMsg(cJSON *stMsgHead,int headlen, char*body,int bodylen, char *stBuff, int stLength, int &usedLength);

	//从服务器拿到数据进行解析
	int decodePBMsgBody(cJSON* headJson, char* pData, char*des, int &datalen);

	void decodeChar(char *buffer);
	void resetData();
	bool hasServerToken();

	unsigned long Adler32(unsigned char *buf, int len);
	string getHttpHeader();
public:
	void processOutput();
	void processInput();
public:
	//监听初始链接服务器是否成功  成功后关闭
	void connecting(float dt);
	//监听收发消息
	void update(float dt);
	//握手
	void handUpdate(float);
public:
	void checkNetType();
	bool domainToIP(const char* pDomain, int port);
public:
	//lua 传入回调
	void setDataHandle(LUA_FUNCTION nHandler);
	void setStatusHandle(LUA_FUNCTION nHandler);
	void removeDataHandle();
	void removeStatusHandle();
public:
	int register_socketforlua(lua_State* L);

	static int c_closeSocket(lua_State* L);
	static int c_sendToServerPackage(lua_State* tolua_S);
	static int c_setSockDataCallBack(lua_State*L);
	static int c_setIP(lua_State*L);
	static int c_setPort(lua_State*L);
};
#endif /* __SockForLua__ */
 