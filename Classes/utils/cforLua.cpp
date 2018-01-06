#include "cforlua.h"    
#include "UrlCode.h" 
#include "md5.h"
CFORLUA*CFORLUA::m_pCForLua=NULL;
 

std::vector<std::string>  parseUTF8(const std::string &str) 
{
	int l = str.length();
	std::vector<std::string> ret;
	ret.clear();
	for(int p = 0; p < l; ) {
		int size;
		unsigned char c = str[p];
		if(c < 0x80) {
			size = 1;
		} else if(c < 0xc2) {
		} else if(c < 0xe0) {
			size = 2;
		} else if(c < 0xf0) {
			size = 3;
		} else if(c < 0xf8) {
			size = 4;
		} else if (c < 0xfc) {
			size = 5;
		} else if (c < 0xfe) {
			size = 6;
		}
		std::string temp = "";
		temp = str.substr(p, size);
		ret.push_back(temp);
		p += size;
	}
	return ret;
}
   
CFORLUA* CFORLUA::getInstance()
{
	if (!m_pCForLua)
	{
		m_pCForLua = new CFORLUA();
	}
	return m_pCForLua;
}


void CFORLUA::releaseInstance()
{
	if (m_pCForLua)
		delete m_pCForLua;
	m_pCForLua = NULL;
}

int CFORLUA::register_lua(lua_State* L)
{
	tolua_open(L);
	tolua_module(L, NULL, 0);
	tolua_beginmodule(L, NULL);
	tolua_function(L, "c_getMd5", c_getMd5);
	tolua_function(L, "c_parseUTF8", c_parseUTF8);
	tolua_function(L, "c_UrlEncode", c_UrlEncode);
	tolua_function(L, "c_UrlDecode", c_UrlDecode);
	tolua_function(L, "c_serverTimeStr", c_getMd5);
	tolua_function(L, "c_checkChar", c_parseUTF8);
	tolua_function(L, "c_GetByteNumOFWord", c_UrlEncode);
	tolua_function(L, "c_getByteLen", c_UrlDecode);
	tolua_endmodule(L);
	return 0;
}

  
 
int CFORLUA::c_getMd5(lua_State*L)
{
	std::string message = tolua_tostring(L, 1, "");  
	MD5 md5(message);
	string result = md5.md5(); 
	tolua_pushstring(L,result.c_str()); 
	return 1;
}

int CFORLUA::c_parseUTF8(lua_State* L) 
{
	std::string str = tolua_tostring(L, 1, "");       
	std::vector<std::string> parseret = parseUTF8(str);
	vector<std::string>::iterator iter;
	lua_newtable(L);    
	int index = 0; 
	for (iter=parseret.begin();iter!=parseret.end();iter++)
	{
		lua_pushstring(L,(*iter).c_str());
		lua_rawseti(L, -2, ++index);  
	}
	return 1;
}
 
int CFORLUA::c_UrlEncode(lua_State* L) 
{
	std::string str = tolua_tostring(L, 1, "");     
	std::string ret=UrlEncode(str);
	const char*rstr=ret.c_str();
	lua_pushlstring(L,rstr,strlen(rstr));
	return 1;
} 
int CFORLUA::c_UrlDecode(lua_State* L) 
{
	std::string str = tolua_tostring(L, 1, "");     
	std::string ret=UrlDecode(str);
	const char*rstr=ret.c_str();
	lua_pushlstring(L,rstr,strlen(rstr));
	return 1;
}      
  
int CFORLUA::c_serverTimeStr(lua_State* tolua_S)
{
	int itime = tolua_tonumber(tolua_S, 1, 0);
	int itype = tolua_tonumber(tolua_S, 2, 0); 
	time_t time1;
	time1 = (time_t)itime;
	struct tm* tminfo;

	tminfo = localtime(&time1);
	__String* timeFormatStr = NULL;
	switch (itype) {
	case 0://年-月-日 时:分:秒
		timeFormatStr = __String::createWithFormat("%04d-%02d-%02d %02d:%02d:%02d", tminfo->tm_year + 1900, tminfo->tm_mon + 1, tminfo->tm_mday, tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec);
		break;
	case 1://年-月-日
		timeFormatStr = __String::createWithFormat("%04d-%02d-%02d", tminfo->tm_year + 1900, tminfo->tm_mon + 1, tminfo->tm_mday);
		break;
	case 2://时:分:秒
		timeFormatStr = __String::createWithFormat("%02d:%02d:%02d", tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec);
		break;
	default://默认显示:年-月-日 时:分:秒
		timeFormatStr = __String::createWithFormat("%04d-%02d-%02d %02d:%02d:%02d", tminfo->tm_year + 1900, tminfo->tm_mon + 1, tminfo->tm_mday, tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec);
		break;
	} 
	tolua_pushstring(tolua_S,timeFormatStr->getCString());
	return 1;
}

int CFORLUA::c_checkChar(lua_State*tolua_S)
{ 
	char ch = tolua_tonumber(tolua_S, 1, 0);
	int ret = (0x80 & ch);
	tolua_pushnumber(tolua_S, ret);
	return 1;
}

int CFORLUA::c_GetByteNumOFWord(lua_State*tolua_S)
{
	unsigned char ucChar = tolua_tonumber(tolua_S, 1, 0);
	int ret = -1;
	if ((ucChar & 0x80) != 0)//ascii code.(0-127) 非英文
	{
		if (((ucChar & 0xF0) ^ 0xF0) == 0)
		{
			ret = 4;
		}

		if (((ucChar & 0xE0) ^ 0xE0) == 0)   //汉字
		{
			ret = 3;
		}

		if (((ucChar & 0xC0) ^ 0xC0) == 0)
		{
			ret = 2;
		}

		ret = -1;
	}
	else
	{
		ret =1;
	}
	tolua_pushnumber(tolua_S, ret);
	return 1;
}

int CFORLUA::c_getByteLen(lua_State*tolua_S)
{
	std::string str = tolua_tostring(tolua_S, 1, "");
	tolua_pushnumber(tolua_S, str.size());
	return 1;
}

int CFORLUA::gettime(lua_State*tolua_S)
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);

	double time =  (double)tv.tv_sec + (double)tv.tv_usec / 1000000;
	tolua_pushnumber(tolua_S, time);
	return 1;
}

int CFORLUA::getTimeInMilliseconds(lua_State*tolua_S)
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	long long time =  (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
	tolua_pushnumber(tolua_S, time);
	return 1;
}


