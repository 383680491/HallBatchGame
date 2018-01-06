#include "SockForLua.h"  

SockForLua* SockForLua::instance = NULL;
#define YM_KEY "ym="  

SockForLua* SockForLua::getInstance()
{
 	if (!instance)
	{
		instance = new SockForLua(); 
	} 
	return instance;
} 

void SockForLua::releaseInstace()
{
	if (instance)
	{
		delete instance;
	}
	instance = NULL;
}

SockForLua::SockForLua()
{
	m_recvBuff = new char[BUFFERSIZE];
	m_sendBuff = new char[BUFFERSIZE];
	m_pbheaduffer = new char[BUFFERSIZE];
	m_pbbodyduffer = new char[BUFFERSIZE];
	m_ODSocket = NULL;
	resetData();
}

SockForLua::~SockForLua()
{
	removeDataHandle();
	delete m_ODSocket;
	delete m_recvBuff;
	delete m_sendBuff;
	delete m_pbheaduffer;
	delete m_pbbodyduffer;
}

bool SockForLua::closeServer(bool removeIndicate)
{
	closeSocket();
	return true;
}


bool SockForLua::closeSocket()
{
	delete m_ODSocket;
	m_ODSocket = NULL;
	resetData();
	Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
	return true;
}

void SockForLua::setIP(const char*ip)
{
	m_ip = ip;
}
void SockForLua::setPort(int port)
{
	m_port = port;
}

void SockForLua::setDataHandle(LUA_FUNCTION nHandler)
{
	removeDataHandle();
	m_nDataHandle = nHandler;
}

void SockForLua::setStatusHandle(LUA_FUNCTION nHandler)
{
	removeStatusHandle();
	m_statusHandle = nHandler;
}



void SockForLua::removeDataHandle()
{
	if (m_nDataHandle != 0)
	{
		ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(m_nDataHandle);
		m_nDataHandle = 0;
	}
}

void SockForLua::removeStatusHandle()
{
	if (m_statusHandle != 0)
	{
		ScriptEngineManager::getInstance()->getScriptEngine()->removeScriptHandler(m_statusHandle);
		m_statusHandle = 0;
	}
}



//开始连接
void SockForLua::startConnect()
{
	//在这边判断这个ip是否为域名，为域名后才进行判断是否为v6网络 
	std::string key = YM_KEY;
	int flag = m_ip.find(key);
	m_bSupportIpv6 = (flag != -1);
	//每次都要检测是否未ipv6
	//在线程中去检测网络版本
	if (m_bSupportIpv6)
	{
		auto t = std::thread(&SockForLua::checkNetType, this);   //由于IPV6解析需要耗费一定时间，故放到子线程里面处理 
		t.detach();                //创建的线程为分离式线程  即资源由系统释放
	}
	else
	{
		CCLOG("not need IPV6");
		connect();                //如果不需要IPV6则直接连接
	}
}

//子线程执行该回调
void SockForLua::checkNetType()
{
	std::string ip = m_ip;
	std::string key = YM_KEY;
	ip = ip.substr(key.length(), ip.length() - key.length());
	bool isSuccess = domainToIP(ip.c_str(), m_port);
	if (!isSuccess)
	{
		log("domainToIP failed");
		return;
	}

	//执行成功则执行连接
	connect();
}

//域名转换为IP
bool SockForLua::domainToIP(const char* pDomain, int port)
{
	char strIP[100];
	sprintf(strIP, "%s", pDomain);
	char strPort[100];
	sprintf(strPort, "%d", port);
	struct addrinfo *ailist, *aip;
	struct addrinfo hint;
	struct sockaddr_in *sinp;
	int sockfd;
	int err;
	char seraddr[INET_ADDRSTRLEN];
	short serport;

	hint.ai_family = 0;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_CANONNAME;
	hint.ai_protocol = 0;
	hint.ai_addrlen = 0;
	hint.ai_addr = NULL;
	hint.ai_canonname = NULL;
	hint.ai_next = NULL;
	if ((err = getaddrinfo(strIP, strPort, &hint, &ailist)) != 0) {
		log("getaddrinfo error: %s\n", gai_strerror(err));
		return false;
	}
	log("getaddrinfo ok\n");
	for (aip = ailist; aip != NULL; aip = aip->ai_next) {

		sinp = (struct sockaddr_in *)aip->ai_addr;
		if (inet_ntop(sinp->sin_family, &sinp->sin_addr, seraddr, INET_ADDRSTRLEN) != NULL)
		{
			log("server address is %s\n", seraddr);
		}
		serport = ntohs(sinp->sin_port);
		m_ODSocket = new ODSocket();
		m_ODSocket->Init();
		m_ODSocket->Create(aip->ai_family, SOCK_STREAM, 0);
		m_ODSocket->Connect(aip->ai_addr, aip->ai_addrlen);

		break;
	}
	freeaddrinfo(ailist);
	return true;
}

//开始连接
void SockForLua::connect()
{
	if (!m_bSupportIpv6)
	{
		CCLOG("prepare connect server");
		m_ODSocket = new ODSocket();
		m_ODSocket->Init();
		m_ODSocket->Create(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		m_ODSocket->Connect(m_ip.c_str(), m_port);
	}
	//启动select定时器  主要是监听是否与服务器连接成功
	Director::getInstance()->getScheduler()->schedule(schedule_selector(SockForLua::connecting), this, 0, false);
}

//异步socket 开启定时器监听socket状态 
void SockForLua::connecting(float dt)
{
	CCLOG("connecting  ");
	if (m_ODSocket->isConnected())
	{
		//切换状态到连接成功
		m_netstate = net_connect_handle;

		//关闭连接服务器状态监听的定时器
		Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);     
		//开启收发定时器
		Director::getInstance()->getScheduler()->schedule(schedule_selector(SockForLua::handUpdate), this, 0, false);
	}
}

/*void SockForLua::handUpdate(float)
{
	if (net_connect_handle_1 == m_netstate)
	{
		string httpHeader = getHttpHeader();
		int sendlen = m_ODSocket->Send((char*)httpHeader.c_str(), httpHeader.length());
		if (sendlen < 0)
			return;

		CCLOG("send hand pachage hand:%s", httpHeader.c_str());
		m_netstate = net_connect_handle_2;
	}
	else if (net_connect_handle_2 == m_netstate){
		char handShakeBuffer[5];
		memset(handShakeBuffer, 0, 5);
		int recv_length = m_ODSocket->Recv(handShakeBuffer, 4, 0);
		if (recv_length <= 0)
			return;
			
		CCLOG("get hand response data:%s", handShakeBuffer);
		m_netstate = net_connect_handle_3;
	}
	else if (net_connect_handle_3 == m_netstate) {
		int sendlen = m_ODSocket->Send("gate", 4);
		if (sendlen < 0)
			return;

		CCLOG("hand success");
		m_netstate = net_connected;

		if (m_statusHandle != 0)    //数据放入lua虚拟机  lua进行解析
		{
			LuaEngine* pEngine = LuaEngine::getInstance();
			LuaStack* pStack = pEngine->getLuaStack();
			pStack->pushInt(net_connected);
			pStack->executeFunctionByHandler(m_statusHandle, 1);
			pStack->clean();
		}

		//关闭连接服务器状态监听的定时器
		Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
		CCLOG("connect  successs ");
		//开启收发定时器
		Director::getInstance()->getScheduler()->schedule(schedule_selector(SockForLua::update), this, 0, false);
	}
}*/

void SockForLua::handUpdate(float)
{
	if (net_connect_handle == m_netstate)
	{
		int sendlen = m_ODSocket->Send("gate", 4);
		if (sendlen < 0)
			return;

		CCLOG("hand success");
		m_netstate = net_connected;

		if (m_statusHandle != 0)    //数据放入lua虚拟机  lua进行解析
		{
			LuaEngine* pEngine = LuaEngine::getInstance();
			LuaStack* pStack = pEngine->getLuaStack();
			pStack->pushInt(net_connected);
			pStack->executeFunctionByHandler(m_statusHandle, 1);
			pStack->clean();
		}

		//关闭连接服务器状态监听的定时器
		Director::getInstance()->getScheduler()->unscheduleAllForTarget(this);
		CCLOG("connect  successs ");
		//开启收发定时器
		Director::getInstance()->getScheduler()->schedule(schedule_selector(SockForLua::update), this, 0, false);
	}
}

//监听socket 收发消息
void SockForLua::update(float)
{
	if (net_connect_handle == m_netstate)
	{
		string httpHeader = getHttpHeader();
	}

	processInput();
	processOutput();
}

void SockForLua::processInput()
{
	//接收数据 
	int recv_length = m_ODSocket->Recv(m_recvBuff, BUFFERSIZE, 0);
	if (recv_length > 0)
	{
		CCLOG("get message from server  recv_length = %d", recv_length);
		for (int i = 0; i<recv_length; i++)
		{
			m_buffer[m_k] = m_recvBuff[i];
			m_k++;
		}

		int bodyLen = ntohl(*(unsigned int*)(m_buffer + m_offset)); //ntohl 网络字节转换为主机字节顺序   这里是m_buffer + m_offset 四个字节就是包体大小
		int packageLen = bodyLen + PACKAGE_LENGTH + 4;
		if ((packageLen > BUFFER_SIZE_RECV) || (packageLen <= 0))
		{
			m_offset = 0;
			m_k = 0;
			memset(m_buffer, 0x00, sizeof(m_buffer));
			return;
		}
	}
	else
	{
		return;
	}

	//解析字符串，多包的情况

	int rt = (m_k - m_offset) > 0;
	while (rt)
	{
		int bodyLen = ntohl(*(unsigned int*)(m_buffer + m_offset));
		int packageLen = bodyLen + PACKAGE_LENGTH + 4;
		//CCLOG("duobao11111111111111111111 bodyLen ===== %d", bodyLen);
		//CCLOG("packageLen ===== %d", packageLen);
		//CCLOG("duobao2222222222 m_k ===== %d        m_offset === %d  ", m_k, m_offset);

		if ((m_k - m_offset) == packageLen)
		{
			memset(m_single_buffer, 0x00, sizeof(m_single_buffer));
			for (int i = 0; i < packageLen; i++)
			{
				m_single_buffer[i] = m_buffer[m_offset + i];      //单独拿出来放到m_single_buffer里面进行解析
			}
			m_offset = 0;
			m_k = 0;
			memset(m_buffer, 0x00, sizeof(m_buffer));
			//给luabuffer
			decodeChar(m_single_buffer);
			//退出解析
			rt = 0;
		}
		else if ((m_k - m_offset) > packageLen)   //解决粘包
		{
			memset(m_single_buffer, 0x00, sizeof(m_single_buffer));
			for (int i = 0; i < packageLen; i++)
			{
				m_single_buffer[i] = m_buffer[m_offset + i];
			}
			m_offset += packageLen;
			//重新确认长度
			packageLen = ntohl(*(unsigned int*)(m_buffer + m_offset));
			//CCLOG("m_offset ===================== %d", m_offset);
			decodeChar(m_single_buffer);
		}
		else
		{
			rt = 0;
		}
	}
}

void SockForLua::decodeChar(char *buffer)
{
	int bodylen;
	cJSON *headJson = cJSON_CreateObject();
	int ret = decodePBMsgBody(headJson, buffer, m_pbbodyduffer, bodylen);

	if (m_nDataHandle != 0 && ret > 0)    //数据放入lua虚拟机  lua进行解析
	{
		char* headJsonStr = cJSON_Print(headJson);

		LuaEngine* pEngine = LuaEngine::getInstance(); 
		LuaStack* pStack = pEngine->getLuaStack();
		lua_pushlstring(pStack->getLuaState(), headJsonStr, strlen(headJsonStr));
		lua_pushlstring(pStack->getLuaState(), m_pbbodyduffer, bodylen);
		pStack->pushInt(bodylen);
		pStack->executeFunctionByHandler(m_nDataHandle, 3);
		pStack->clean();

		free(headJsonStr);
	}

	cJSON_Delete(headJson);
}

//解析包
int SockForLua::decodePBMsgBody(cJSON* headJson, char* pData, char*dest, int& datalen)
{
	// 先清空旧的消息体 
	datalen = 0;

	char* tempHead = pData;
	int headSize = PACKAGE_LENGTH;
	int bodySize = ntohl(*(unsigned int*)(pData));
	int totalSize = bodySize + headSize + 4;

	pData += (int)sizeof(unsigned int);

	if (totalSize < headSize)
	{
		return 0;
	}

	short protoId = ntohs(*(short*)(pData));
	pData += (short)sizeof(short);

	short serialNum = ntohs(*(short*)(pData));
	pData += (short)sizeof(short);

	int time = ntohl(*(unsigned int*)(pData));
	pData += (int)sizeof(unsigned int);

	int dt = (int)*pData;
	pData += (char)sizeof(char);

	int zt = (int)*pData;
	pData += (char)sizeof(char);

	int code = (int)*pData;
	pData += (char)sizeof(char);

	cJSON_AddNumberToObject(headJson, "protoId", protoId);
	cJSON_AddNumberToObject(headJson, "serialNum", serialNum);
	cJSON_AddNumberToObject(headJson, "time", time);
	cJSON_AddNumberToObject(headJson, "dt", dt);
	cJSON_AddNumberToObject(headJson, "zt", zt);
	cJSON_AddNumberToObject(headJson, "code", code);

	if (bodySize > 0)
	{
		datalen = bodySize;
		memcpy(dest, pData, bodySize);
	}
		


	/*int tempoffset = 0;
	int iMsgDataLen = ntohl(*(unsigned int*)(pData));    //从这里可以看出  数据结构是  前4个字节是包体大小  紧接着之后的两个字节是包头大小
	tempoffset += sizeof(unsigned int);
	int iMsgHeadLen = ntohs(*(unsigned short*)(pData + tempoffset));
	tempoffset += sizeof(unsigned short);
	if (iMsgHeadLen <= 0)
	{
		return 0;
	}
	if (iMsgHeadLen > iMsgDataLen - (int)sizeof(unsigned int)-(int)sizeof(unsigned short)-(int)sizeof(char))
	{
		CCLOG("msgHeadLen Longer than total error");
		return 0;
	}
	//消息头
	if (0 == type)
	{
		datalen = iMsgHeadLen;
		memset(dest, 0, iMsgHeadLen + 1);
		memcpy(dest, pData + tempoffset, iMsgHeadLen);
		dest[iMsgHeadLen] = '\0';
	}
	//消息体
	else
	{
		tempoffset += iMsgHeadLen;
		int iMsgBodyLen = iMsgDataLen - sizeof(unsigned int)-sizeof(unsigned short)-iMsgHeadLen - sizeof(char);
		if (iMsgBodyLen > 0)
		{
			datalen = iMsgBodyLen;
			memset(dest, 0, iMsgBodyLen + 1);
			memcpy(dest, pData + tempoffset, iMsgBodyLen);
		}
	}*/
	return 1;
}


void SockForLua::processOutput()
{
	int alreadysend = 0;
	while (m_sendLength>0)
	{
		int sendlen = m_ODSocket->Send(m_sendBuff + alreadysend, m_sendLength);
		if (sendlen < 0)
			return;
		alreadysend += sendlen;
		m_sendLength -= sendlen;
	}
}






int SockForLua::encodePBMsg(cJSON * stMsgHead, int headSize, char*stMsgBody, int bodySize, char *stBuff, int stMaxLength, int &usedLength)
{ 
	cJSON* protoIdNode = cJSON_GetObjectItem(stMsgHead, "protoId");
	cJSON* serialNumNode = cJSON_GetObjectItem(stMsgHead, "serialNum");
	cJSON* timeNode = cJSON_GetObjectItem(stMsgHead, "time");
	cJSON* dtNode = cJSON_GetObjectItem(stMsgHead, "dt");
	cJSON* ztNode = cJSON_GetObjectItem(stMsgHead, "zt");
	cJSON* codeNode = cJSON_GetObjectItem(stMsgHead, "code");

	int protoId = protoIdNode->valueint;
	int serialNum = serialNumNode->valueint;
	int time = timeNode->valueint;
	int dt = dtNode->valueint;
	int zt = ztNode->valueint;
	int code = codeNode->valueint;
	char* tempHead = stMsgBody;

	//CCLOG("C++ protoID= %d, serialNum= %d, time= %d, dt= %d, zt= %d, code= %d", protoId, serialNum, time, dt, zt, code);

	usedLength = headSize + bodySize + 4;

	if (stMaxLength < usedLength)
	{
		CCLOG("Message is too large");
		return 0;
	}

	*((int *)stBuff) = (int)htonl(bodySize);
	stBuff += (int)sizeof(unsigned int);

	*((short *)stBuff) = (short)htons(protoId);
	stBuff += (short)sizeof(short);

	*((short *)stBuff) = (short)htons(serialNum);
	stBuff += (short)sizeof(short);

	*((int *)stBuff) = (int)htonl(time);
	stBuff += (int)sizeof(unsigned int);

	*(stBuff) = dt;
	stBuff += (char)sizeof(char);

	*(stBuff) = zt;
	stBuff += (char)sizeof(char);

	*(stBuff) = code;
	stBuff += (char)sizeof(char);

	memcpy(stBuff, stMsgBody, bodySize);
	stBuff += bodySize;

	unsigned long checkCode = Adler32((unsigned char*)tempHead, headSize + bodySize);

	*((int *)stBuff) = (int)htonl(checkCode);
	stBuff += (int)sizeof(unsigned int);
	/*usedLength = (int)sizeof(unsigned int)+(int)sizeof(unsigned short)+headSize + bodySize + (int)sizeof(char);
	if (stLength < usedLength)
	{
		CCLOG("Message is too large");
		return 0;

	} 
	int offset = 0;
	*((int *)stBuff) = (int)htonl(usedLength);
	offset += (int)sizeof(unsigned int); 
	*((unsigned short *)(stBuff + offset)) = (short)htons(headSize);
	offset += (int)sizeof(unsigned short);
	memcpy(stBuff + offset, stMsgHead , headSize);
	offset += headSize; 
	memcpy(stBuff + offset, stMsgBody , bodySize); 
	offset += bodySize; 
	*(stBuff + offset) = '\t'; */

	return 1;
} 
 
int SockForLua::sendToServerPackage(cJSON* head, int headlen, char* body,int bodylen)
{   
	int send_data_length = 0; 
	//CCLOG("headlen ============ %d", headlen);
	//CCLOG("bodylen ============ %d", bodylen);
	if (!encodePBMsg(head, headlen, body, bodylen, m_sendBuff + m_sendLength, BUFFER_SIZE_SEND, send_data_length))
	{
		CCLOG("sendToServerPackage  encodePBMsg 失败");
		return 13000;
	} 
	m_sendLength += send_data_length;

	//同时在这里判断网络情况
	switch (m_netstate)
	{
	case SockForLua::net_disconnect:
		//去连接网络并马上启动定时器检测是否连接成功
		CCLOG("Try to connect server  ing");
		m_netstate = net_connecting;
		startConnect();
		break;
	case SockForLua::net_connecting:
		break;
	case SockForLua::net_connected:
		break;
	default:
		break;
	}
	return 0;
} 

void SockForLua::resetData()
{
	m_offset = 0;
	m_k = 0;
	m_sendLength = 0;
	m_netstate = net_disconnect;
}


unsigned long SockForLua::Adler32(unsigned char *buf, int len)
{
	unsigned long adler = 1;
	unsigned long s1 = adler & 0xffff;
	unsigned long s2 = (adler >> 16) & 0xffff;

	int i;
	for (i = 0; i < len; i++)
	{
		s1 = (s1 + buf[i]) % ADLER_BASE;
		s2 = (s2 + s1) % ADLER_BASE;
	}

	return (s2 << 16) + s1;
}

string SockForLua::getHttpHeader()
{
	string str;
	char port[32];
	memset(port, 0, 32);
	sprintf(port, "%d", m_port);

	str.append("GET /gate HTTP/1.1\r\nHost: ");
	str.append(m_ip);
	str.append(":");
	str.append(port);
	str.append("\r\nContent-Length: 0\r\n\r\n");
	return str;
}


























































int SockForLua::c_closeSocket(lua_State* L)
 {
 	SockForLua::getInstance()->closeSocket();
 	return 0;
 }
int SockForLua::c_sendToServerPackage(lua_State* tolua_S)
{
	size_t headlen = 0;
	char *head = (char*)luaL_checklstring(tolua_S, 1, &headlen);  

	size_t bodylen = 0;
	char *body = (char*)luaL_checklstring(tolua_S, 2, &bodylen);  

	cJSON *headJson = cJSON_Parse(head);

	int ret = SockForLua::getInstance()->sendToServerPackage(headJson, PACKAGE_LENGTH, body, bodylen);

	cJSON_Delete(headJson);

	tolua_pushnumber(tolua_S, ret);
	return  ret;
}
 
int SockForLua::c_setSockDataCallBack(lua_State*tolua_S)
{
	LUA_FUNCTION nHandler = toluafix_ref_function(tolua_S, 1, 0);
	SockForLua::getInstance()->setDataHandle(nHandler); 

	nHandler = toluafix_ref_function(tolua_S, 2, 0);
	SockForLua::getInstance()->setStatusHandle(nHandler);

	return 0;
}
int SockForLua::c_setIP(lua_State*L)
{
	std::string ip = tolua_tostring(L, 1, "");
	SockForLua::getInstance()->setIP(ip.c_str());
	return 0;
}
int SockForLua::c_setPort(lua_State*L)
{
	int port = (int)tolua_tonumber(L, 1, 0); 
	SockForLua::getInstance()->setPort(port);
	return 0;
}
int SockForLua::register_socketforlua(lua_State* L)
{
	tolua_open(L);
	tolua_module(L, NULL, 0);
	tolua_beginmodule(L, NULL);  
	tolua_function(L, "c_closeSocket", c_closeSocket);
	tolua_function(L, "c_sendToServerPackage", c_sendToServerPackage);
	tolua_function(L, "c_setPort", c_setPort);
	tolua_function(L, "c_setIP", c_setIP);
	tolua_function(L, "c_setSockDataCallBack", c_setSockDataCallBack); 
	tolua_endmodule(L);
	return 0;
}
 