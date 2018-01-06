STATUS_DOWNLOAD_AGAIN = 1
STATUS_NO_UPDATE = 2
STATUS_NEED_UPDATE = 3
STATUS_UPDATEING = 4
STATUS_NET_FAIL = 5
STATUS_UPDATE_END = 6

HotUpdate = class("HotUpdate", function()
    return {};
end)

HotUpdate.__instance = nil  

function HotUpdate:create(gameType, callback, parent)   
    local ret = HotUpdate.new()
    ret:init(gameType, callback, parent)
    return ret
end

function HotUpdate:ctor()
    self.downloadIndex = 1;
    self.updateList = {}               --把需要更新的列表全都放到这个列表里面
end



function HotUpdate:init(gameType, callback, parent)

    local url = "http://dj-dpu.oss-cn-beijing.aliyuncs.com"
    self.serverVersionListUrl = string.format("%s%s%s%s", url, '/myTest/', gameType, '/uploadVersionList.json')
    self.hotUpdateServerDir = string.format("%s%s%s%s", url, '/myTest/', gameType, '/')

    self.gameType = gameType
    self.callback = callback;
    self.parent = parent;
    self.myWritePath = _G.__MY_HOT_UPDATE_PATH__;
    self.localInfo = self:readVersion();
    self:checkServerVersionInfo();
end



--保存本地版本信息
function HotUpdate:saveLocalVersion(versionInfo) 
    local info = json.encode(versionInfo)
    local writePath = string.format("%s%s%s%s", self.myWritePath, self.gameType, '_', "versioninfo.json")
    io.writefile(writePath, info)
end

--读取本地版本信息
function HotUpdate:readVersion()
    local writePath = string.format("%s%s%s%s", self.myWritePath, self.gameType, '_', "versioninfo.json")
    if not cc.FileUtils:getInstance():isFileExist(writePath) then 
        print('热更新目录的配置文件不存在')
        if self.gameType == 'hall' then 
            local fullPath = cc.FileUtils:getInstance():fullPathForFilename("src/hall/res/versioninfo.json");
            local info = cc.FileUtils:getInstance():getStringFromFile(fullPath);
            info = json.decode(info);

            self:saveLocalVersion(info);
            return info
        else
            local fullPath = cc.FileUtils:getInstance():fullPathForFilename("src/hall/res/versioninfo.json");
            local info = cc.FileUtils:getInstance():getStringFromFile(fullPath);
            info = json.decode(info);
            info.version = 0

            self:saveLocalVersion(info);
            return info
        end
    else
        print('热更新目录的配置文件已存在')
        local info = cc.FileUtils:getInstance():getStringFromFile(writePath);    
        info = json.decode(info);

        if self.gameType == 'hall' then 
            local fullPath = cc.FileUtils:getInstance():fullPathForFilename("src/hall/res/versioninfo.json");
            local localinfo = cc.FileUtils:getInstance():getStringFromFile(fullPath);
            print('localinfo=========', localinfo)

            localinfo = json.decode(localinfo);
            info.build = localinfo.build           --永远使用res/version.json的build版号   可能是后面重新打包后需要强制下载新包
            if localinfo.version > info.version then    --如果是本地大于缓存的版本 可能是后面打的新包的提高了版本 不想更新之前的包
                info.version = localinfo.version
            end
        end

        return info
    end
end

--接下来就是判断是否有需要热更新的版本 下载服务器的versionlist.json 
function HotUpdate:checkServerVersionInfo()
    local xhr = cc.XMLHttpRequest:new()
print('HotUpdate:checkServerVersionInfo')
look(self.serverVersionListUrl, 'self.serverVersionListUrl')
    xhr.responseType = cc.XMLHTTPREQUEST_RESPONSE_STRING;
    xhr:open("GET", self.serverVersionListUrl)  

    local function onReadyStateChanged()
        if xhr.readyState == 4 and (xhr.status >= 200 and xhr.status < 207) then
            print('get server config == ' .. xhr.response)
            local versionList = json.decode(xhr.response)             --做了一个排序  不要紧张

            table.sort(versionList,function(versionA, versionB) 
                return versionA.version < versionB.version
            end);   

            --如果判断热更新包已经存在则解压缩删除
            for _, updateInfo in ipairs(versionList) do 
                local zipName = "zeus_" .. updateInfo.build .. '_' .. updateInfo.version  .. '.zip';
                if io.exists(self.myWritePath .. zipName) then    --文件是否存在
                    local success = hotUpdate:create():uncompress(self.myWritePath, zipName);   --解压缩删除
                    if not success then 
                        os.remove(self.myWritePath .. zipName);
                    end
                end
            end 

            os.remove(self.myWritePath .. 'versionlist.dat');

            print('self.localInfo.build===========', self.localInfo.build)
            local curBuild = self.localInfo.build
            if #versionList > 0 then   --判断最后一个热更新包的build版本
                local updateInfo = versionList[#versionList]
                if updateInfo.build ~= curBuild then  --如果是build 版本小于本地 则直接下载最新版本 不用热更新
                    cclog('请重新下载最新版本')
                    self.callback(STATUS_DOWNLOAD_AGAIN);
                    return;
                end
            end

            local curVerison = self.localInfo.version;
            for _, versionInfo in ipairs(versionList) do    --版本比当前版本大的话就更新
                print('versionInfo.version ====  curVerison ==', versionInfo.version, curVerison)
                if versionInfo.version > curVerison then 
                    table.insert(self.updateList, versionInfo);
                end
            end

            if #self.updateList > 0 then     --需要更新   则把需要更新的配置数据写入到 热更新目录versionlist.dat
                local writeContent = json.encode(self.updateList);
                local temp = io.writefile(self.myWritePath .. 'versionlist.dat', writeContent);

                if temp then 
                    local size = 0;
                    for _, updateInfo in ipairs(self.updateList) do 
                        size = size + updateInfo.size;
                    end

                    cclog('需要更新')
                    self.parent:setDownloadInfo(#self.updateList, size);
                    self.callback(STATUS_NEED_UPDATE);
                else
                    cclog('不需要更新 执行正常的逻辑')
                    self.callback(STATUS_NO_UPDATE);
                end
            else
                cclog('不需要更新 执行正常的逻辑')
                self.callback(STATUS_NO_UPDATE);
            end
        else
            print("xhr.readyState is:", xhr.readyState, "xhr.status is: ",xhr.status)
            cclog('网络异常 执行正常的逻辑')
            self.callback(STATUS_NET_FAIL);
        end

        xhr:unregisterScriptHandler()
    end

    xhr:registerScriptHandler(onReadyStateChanged)
    xhr:send()
end



--在这里要注意一点  可能有多个更新包 看怎么在界面显示
function HotUpdate:needUpdate() 
    local function onError(errorCode)
        if errorCode then
            cclog('下载出错了')
            self.callback(STATUS_NO_UPDATE);
        end
    end

    --下载进度  由于可能是多个包 把当前包的索引传过去
    local function onProgress(percent)
        cclog('下载中====' .. percent)
        self.callback(STATUS_UPDATEING, self.downloadIndex, percent);
    end

    --下载成功
    local function onSuccess()
        cclog('下载完成')
        --self.callback(STATUS_UPDATEING, self.downloadIndex, #self.updateList, 101);
        self.downloadIndex = self.downloadIndex + 1;      --一直把列表里面的全部下载下来

        if self.downloadIndex > #self.updateList then      --成功完成了下载
            cc.FileUtils:getInstance():purgeCachedEntries();

            local configPath = self.myWritePath .. 'versionlist.dat';
            local data = io.readfile(configPath);
            data = json.decode(data);

            if data and #data > 0 then 
                for _, info in ipairs(data) do 
                    local zipName = "zeus_" .. info.build .. '_' .. info.version  .. '.zip';
                    if io.exists(self.myWritePath .. zipName) then    --文件是否存在
                        local success = hotUpdate:create():uncompress(self.myWritePath, zipName);   --解压缩
                        if not success then 
                            os.remove(self.myWritePath .. zipName);
                        end
                    end

                    self.localInfo.build = info.build
                    self.localInfo.version = info.version
                    self:saveLocalVersion(info);                               --保证每次更新本地版本号
                end

                os.remove(configPath)                   --移除配置文件versionlist.dat
            end

            self.callback(STATUS_UPDATE_END);
            --改变版本号
        else
            self:startOneTask();                   --继续下一个更新包的下载
        end
    end

    --这里就是在C++创建热更新对象 注册监听事件 
    hotUpdate:create():setCallback(onSuccess, onProgress, onError);
    hotUpdate:create():setDownLoadPath(self.myWritePath);

    self:startOneTask();
end

--开始执行一个下载任务
function HotUpdate:startOneTask() 
    local updateInfo = self.updateList[self.downloadIndex]
    local zipName = "zeus_" .. updateInfo.build .. '_' .. updateInfo.version  .. '.zip';
    local url = self.hotUpdateServerDir .. zipName;

    hotUpdate:create():setHotPackageUrl(url, zipName);
    hotUpdate:create():requstHotPackage();
end