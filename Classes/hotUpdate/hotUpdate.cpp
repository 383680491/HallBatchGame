#include "hotUpdate.h"
#include "scripting/lua-bindings/manual/CCLuaEngine.h"
#include<sys/types.h>
#include<sys/stat.h>
using namespace cocos2d;

#define TMPNAME "tmpMyHotUpdate.zip"  
hotUpdate* hotUpdate::p_mHotUpdate = NULL;

static size_t downLoadPackage(void *ptr, size_t size, size_t nmemb, void *userdata)
{
	FILE *fp = (FILE*)userdata;
	size_t written = fwrite(ptr, size, nmemb, fp);
	return written;
}

static int hotUpdateProgressFunc(void *ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	if (totalToDownload <= 0 || nowDownloaded <= 0)
		return 0;

	float temp = (nowDownloaded / totalToDownload * 100);

	int temp_1 = (int)temp;
	if (hotUpdate::create()->getDownloadProcess() != temp_1 && nowDownloaded != totalToDownload)
	{
		hotUpdate::create()->setDownloadProcess(temp_1);
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([=]{
			hotUpdate::create()->handleProgressCallback();
		});
	}
	return 0;
}

hotUpdate::hotUpdate()
{
	m_lua_error_handle = 0;
	m_lua_progress_handle = 0;
	m_lua_success_handle = 0;
	m_downloadProcess = 0;
}

hotUpdate::~hotUpdate()
{

}

hotUpdate* hotUpdate::create()
{
	if (NULL == p_mHotUpdate){
		p_mHotUpdate = new hotUpdate();
		p_mHotUpdate->init();
    }
    
	return p_mHotUpdate;
}

void hotUpdate::release()
{
	if (p_mHotUpdate){
		delete p_mHotUpdate;
		p_mHotUpdate = NULL;
    }
}

void hotUpdate::init()
{
    
}

void hotUpdate::setDownLoadPath(const char* path)
{
	mDownLoadWritePath.append(path);
}

void hotUpdate::setCallback(int success_handle, int progress_handle, int error_handle)
{
	m_lua_success_handle = success_handle;
	m_lua_progress_handle = progress_handle;
	m_lua_error_handle = error_handle;
}

void hotUpdate::setHotPackageUrl(const char* url, const char* name)
{	
	mHotPackageUrl = url;
	mHotPackageName = name;
}

void hotUpdate::requstHotPackage()
{	
	m_downloadProcess = 0;
	thread t(hotUpdate::run, this);
	t.detach();
}

void * hotUpdate::run(void* arg)
{
	hotUpdate* pHotUpdate = (hotUpdate*)arg;
	pHotUpdate->_run();

	return nullptr;
}

void hotUpdate::_run()
{
	if (!this->downLoad())
		return;

	const string oldName = mDownLoadWritePath + TMPNAME;
	string fullZipName = mDownLoadWritePath + mHotPackageName;
	int ret = rename(oldName.c_str(), fullZipName.c_str());
	Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]
		{this->handleSuccessCallback();}
	);
}

//使用的curl_easy库  一个http库
bool hotUpdate::downLoad()
{
	//如果是temp则是下载失败
	const string outFileName = mDownLoadWritePath + TMPNAME;
	FILE *fp = fopen(outFileName.c_str(), "wb");

	if (!fp)
	{
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]{
			this->handleErrorCallback(ErrorCode::CREATE_FILE);
		});
		return false;
	}

	m_curl = curl_easy_init();
	curl_easy_setopt(m_curl, CURLOPT_URL, mHotPackageUrl.c_str());
	curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION, downLoadPackage);
	curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(m_curl, CURLOPT_NOPROGRESS, false);
	curl_easy_setopt(m_curl, CURLOPT_PROGRESSFUNCTION, hotUpdateProgressFunc);
	curl_easy_setopt(m_curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(m_curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_LIMIT, LOW_SPEED_LIMIT);
	curl_easy_setopt(m_curl, CURLOPT_LOW_SPEED_TIME, LOW_SPEED_TIME);

	CURLcode res = curl_easy_perform(m_curl);
	curl_easy_cleanup(m_curl);
	CCLOG("CURL response status   res = %d", res);
	if (res != 0)
	{
		Director::getInstance()->getScheduler()->performFunctionInCocosThread([&, this]{
			this->handleErrorCallback(ErrorCode::NETWORK);
		});

		fclose(fp);
		return false;
	}

	fclose(fp);
	return true;
}

bool hotUpdate::uncompress(const string&storagePath, const string&zipname)
{
	string outFileName = storagePath + zipname;
	unzFile zipFile = unzOpen(outFileName.c_str());

	if (!zipFile)
		return false;

	unz_global_info global_info;
	unzGetGlobalInfo(zipFile, &global_info);
	char readBuffer[BUFFER_SIZE];

	for (uLong i = 0; i < global_info.number_entry; ++i)
	{
		unz_file_info fileInfo;
		char fileName[MAX_FILENAME];
		unzGetCurrentFileInfo(zipFile, &fileInfo, fileName, MAX_FILENAME, NULL, 0, NULL, 0);

		const string fullPath = storagePath + fileName;
		const size_t filenameLength = strlen(fileName);

		if (fileName[filenameLength - 1] == '/')
		{
			createDirectory(fullPath.c_str());
		}else{
			const string fileNameStr(fileName);
			size_t startIndex = 0;
			size_t index = fileNameStr.find("/", startIndex);

			while (index != std::string::npos)
			{
				const string dir = storagePath + fileNameStr.substr(0, index);
				FILE *out = fopen(dir.c_str(), "r");
				if (!out)
				{
					createDirectory(dir.c_str());
				}else{
					fclose(out);
				}
					
				startIndex = index + 1;
				index = fileNameStr.find("/", startIndex);
			}

			unzOpenCurrentFile(zipFile);
			FILE *out = fopen(fullPath.c_str(), "wb");
			int error = UNZ_OK;
			do
			{
				memset(readBuffer, 0, sizeof(readBuffer));
				error = unzReadCurrentFile(zipFile, readBuffer, BUFFER_SIZE);
				if (error > 0)
					fwrite(readBuffer, error, 1, out);
			} while (error > 0);

			fclose(out);
		}

		unzCloseCurrentFile(zipFile);

		if ((i + 1) < global_info.number_entry)
			unzGoToNextFile(zipFile);
	}

	unzClose(zipFile);
	remove(outFileName.c_str());
	return true;
}

bool hotUpdate::createDirectory(const char *path)
{
#if (CC_TARGET_PLATFORM != CC_PLATFORM_WIN32)
	mode_t processMask = umask(0);
	int ret = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO);
	umask(processMask);
	if (ret != 0 && (errno != EEXIST))
		return false;
	return true;
#else
	BOOL ret = CreateDirectoryA(path, NULL);
	if (!ret && ERROR_ALREADY_EXISTS != GetLastError())
		return false;
	return true;
#endif
}

void hotUpdate::handleProgressCallback()
{
	if (m_lua_progress_handle > 0){
        LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
		stack->pushInt(m_downloadProcess);
		stack->executeFunctionByHandler(m_lua_progress_handle, 1);
        stack->clean();
    }
}

void hotUpdate::handleErrorCallback(ErrorCode error)
{
	if (m_lua_error_handle > 0){
		LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
		int err = error;
		stack->pushInt(err);
		stack->executeFunctionByHandler(m_lua_error_handle, 1);
		stack->clean();
	}
}

void hotUpdate::handleSuccessCallback()
{
	if (m_lua_success_handle > 0){
		LuaStack* stack = LuaEngine::getInstance()->getLuaStack();
		stack->executeFunctionByHandler(m_lua_success_handle, 0);
		stack->clean();
	}
}




