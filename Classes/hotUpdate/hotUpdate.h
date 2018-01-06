#ifndef _HOT_UPDATE_
#define _HOT_UPDATE_

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h" 
#ifdef __cplusplus
}
#endif 

#include <iostream>
#include <string> 
#include <mutex>

//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
//#include "external/curl/include/win32/curl/curl.h"
//#include "external/curl/include/win32/curl/easy.h"
//#elif (CC_TARGET_PLATFORM==CC_PLATFORM_ANDROID)
//#include "external/curl/include/android/curl/curl.h"
//#include "external/curl/include/android/curl/easy.h"
//#elif (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//#include "external/curl/include/ios/curl/curl.h"
//#include "external/curl/include/ios/curl/easy.h"
//#endif


#include <curl/curl.h>
#include <curl/easy.h>



#include <stdio.h>
#include <vector>
#include <thread>  


#ifdef __APPLE__
	#include "unzip.h"
#endif

#ifdef __ANDROID__
	#include "unzip.h"
#endif

#ifdef _WINDOWS
	#include "unzip/unzip.h"
#endif


#include "cocos2d.h"
using namespace cocos2d;
using namespace std;

#define BUFFER_SIZE    8192
#define MAX_FILENAME   512 
#define LOW_SPEED_LIMIT 1L
#define LOW_SPEED_TIME 5L   

enum ErrorCode
{
	CREATE_FILE,
	NETWORK,
	NO_NEW_VERSION,
	UNCOMPRESS,
};

class hotUpdate
{
public:
	hotUpdate();
	~hotUpdate();
    
	static hotUpdate* create();
    static void release();

	/*set callback*/
	void setCallback(int success_handle, int progress_handle, int error_handle);
	
	/*set hot package url*/
	void setHotPackageUrl(const char* url, const char* name);

	/*下载路径*/
	void setDownLoadPath(const char* path);

	/*开始请求*/
	void requstHotPackage();

	/*进度信息*/
	int getDownloadProcess(){ return m_downloadProcess; }
	void setDownloadProcess(int downloadProcess){ m_downloadProcess = downloadProcess; }

	/*解压缩*/
	bool uncompress(const string&storagePath, const string&zipname);
	bool createDirectory(const char *path);

	/*处理lua回调*/
	void handleProgressCallback();
	void handleErrorCallback(ErrorCode error);
	void handleSuccessCallback();
private:
	/*线程异步回调*/
	static void* run(void* arg);
	void _run();

    void init();

	/*开始http请求*/
	bool downLoad();
private:
	static hotUpdate* p_mHotUpdate;
    int m_lua_error_handle;
	int m_lua_progress_handle;
	int m_lua_success_handle;
	int m_downloadProcess;


	string mDownLoadWritePath;
	string mHotPackageUrl;
	string mHotPackageName;
	CURL * m_curl;
};

#endif
