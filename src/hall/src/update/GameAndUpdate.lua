-----------------------------------------------------------
-- 是否需要每个游戏文件里放一个uploadVersionList.json 
-- 是不需要的 既然有按钮就保证游戏已经上了，没上按钮就不要显示了
-----------------------------------------------------------
local device = require 'cocos.framework.device'
local display = require 'cocos.framework.display'
require "update.HotUpdate"

local SHOW_TIP_SWITCH = 70
local localDownInfo = "资源下载中(%d%%)"

GameAndUpdate = class("GameAndUpdate",  function()
    local ret = cc.Layer:create()
    return ret
end )


--消息传过来准备 点击确定后重连 
function GameAndUpdate:create(gameType, func_updateFinised)
    local ret = GameAndUpdate:new()
    ret.finisnedFunc = func_updateFinised
    ret:initUI(gameType)
    return ret
end

function GameAndUpdate:initUI(gameType) 
    self.tipStatus = 'start'         --无法保证是 logo动画先执行完还是获得热更新配置文件先执行  所以用状态
    self.curProgress = 0
    self.gameType = gameType

    local func = function(status, downDownIndex, packageTotal, percent) 
        self:updateUI(status, downDownIndex, packageTotal, percent);
    end

    self.hotInstance = HotUpdate:create(self.gameType, func, self)

   local node = cc.Node:create()
   self:addChild(node) 
    node:runAction(cc.RepeatForever:create(cc.Sequence:create(
        cc.DelayTime:create(0.1),
        cc.CallFunc:create(function(sender) 
            self:update(sender)
        end)
    )))
end

function GameAndUpdate:updateUI(status, downDownIndex, percent)
    if STATUS_NO_UPDATE == status then 
        print("不需要热更新")
        self.tipStatus = 'ok'

    elseif STATUS_NEED_UPDATE == status then 
        self.needUpdateFlag = true

        if self.downSize <= SHOW_TIP_SWITCH then  
            print("需要热更新, 隐藏提示  同时自动开始热更新")
            self.hotInstance:needUpdate()
        else
            print("需要热更新, 提示玩家是否更新")
            self.tipStatus = 'show'    
        end
    elseif STATUS_UPDATEING == status then 
        cclog('percent ==============' .. percent)

        if self.downSize > SHOW_TIP_SWITCH then   --大于阈值才有界面表现   判断是否是wifi kCCNetworkStatusReachableViaWiFi == network.getInternetConnectionStatus()
            self:udateLoading(downDownIndex, percent)
            self.curPkgIndex = downDownIndex
        end
    elseif STATUS_UPDATE_END == status then 
        if self.downSize > SHOW_TIP_SWITCH  then   --大于阈值才有界面表现    or kCCNetworkStatusReachableViaWiFi == network.getInternetConnectionStatus()
            self.pro_bar:setPercent(100)
            self.label_pro:setString(string.format(localDownInfo, 100));
        end

        self.tipStatus = 'ok'
        print('更新完成')
    elseif STATUS_NET_FAIL == status then 
        print("网络异常，请检查网络")
        self.tipStatus = "tip_net"
    elseif STATUS_DOWNLOAD_AGAIN == status then 
        print("请重新下载最新版本")
    end
end


--例如 5个包 那么每个占20的进度条   
function GameAndUpdate:udateLoading(index, percent)
    if index == self.curPkgIndex then 
        local curBeginPos = (index - 1) * self.percentWeight
        self.curProgress = curBeginPos + percent / 100 * self.percentWeight;
    end

    if self.pro_bar then 
        self.pro_bar:setPercent(self.curProgress)
        self.label_pro:setString(string.format(localDownInfo, self.curProgress));
    end
end

function GameAndUpdate:setDownloadInfo(downCount, downSize)
    self.downCount = downCount;
    self.downSize = downSize;
    self.percentWeight = 100 / self.downCount;    --多个任务共同用一个 进度条  每一个平均在比例
    self.curPkgIndex = 1
print('下载的大小  下载的数量', downCount, downSize)
end

function GameAndUpdate:addUITip()
    local layer = cc.CSLoader:createNode("csb/login/hotUpdate.csb")
    self:addChild(layer)
    self.tipLayer = layer

    local image_bg = layer:getChildByName('image_bg')
    local image_tip_frame = layer:getChildByName('image_tip_frame')
    local label_desc = image_tip_frame:getChildByName('label_desc')
    local button_ok = image_tip_frame:getChildByName('button_ok')
    local button_cancle = image_tip_frame:getChildByName('button_cancle')
    local image_bar_bg = layer:getChildByName('image_bar_bg')
    local image_game = layer:getChildByName('image_game');
    self.pro_bar = image_bar_bg:getChildByName('pro_bar')
    self.label_pro = image_bar_bg:getChildByName('label_pro')
    self.label_version = layer:getChildByName('label_version')

    if APP_FOR=="BiYin" then    
        image_game:loadTexture('img/common/bg/logo_beyond_big.png')
    end

    local directUpdate = function()  --直接更新
        self.pro_bar:setPercent(0);
        self.label_pro:setString(string.format(localDownInfo, 0));
        image_tip_frame:setVisible(false)
        self.hotInstance:needUpdate()
    end

    local tipUpdate = function()   --有tip 
        image_bar_bg:setVisible(false)
        self.pro_bar:setPercent(0);
        self.label_pro:setString(string.format(localDownInfo, 0));

        label_desc:setString(string.format("当前需要下载%0.3fM资源，是否下载？", self.downSize / 1024))

        button_ok:addTouchEventListener(function(widget, EventType)
            if EventType == ccui.TouchEventType.ended then 
            cclook("2")
                self.hotInstance:needUpdate()
                image_bar_bg:setVisible(true)
                image_tip_frame:setVisible(false)
            end
        end)

        button_cancle:addTouchEventListener(function(widget, EventType)
            if EventType == ccui.TouchEventType.ended then 
            cclook("3")
                cc.Director:getInstance():endToLua()
            end
        end)
    end

    if device.platform == "windows" then
        print('platform window 直接更新')
        directUpdate()
    elseif device.platform == "android" then
        print('platform android')
        local luaj = require "cocos.cocos2d.luaj";
        local className = "org.cocos2dx.lua/AppActivity";
        local args = {}
        local sig = "()I"
        local ok, ret = luaj.callStaticMethod(className, "getNetType", args, sig)
        if not ok then
            print('热更新网络状态 没有找到jni')
            tipUpdate()
        else
            if ret == 1 then   --1 是 wifi 
                print('wifi')
                directUpdate()
            else
                print('移动网络')
                tipUpdate()
            end
        end
    elseif device.platform == "ios" then
        print('platform ios')
        local luaoc = require "cocos.cocos2d.luaoc";
        local ok, ret = luaoc.callStaticMethod("DZNetworkTool", "getCurrentNetworkStatus")

        if not ok then
            print('热更新网络状态 没有找到jni')
            tipUpdate()
        else
            if ret == 1 then   --1 是 wifi 
                print('wifi')
                directUpdate()
            else
                print('移动网络')
                tipUpdate()
            end
        end
    end 
end

function GameAndUpdate:addFailUI(desc) 
    local layer = self.failUILayer
    if not layer then 
        layer = cc.CSLoader:createNode("csb/login/hotUpdate.csb")
        self:addChild(layer)
    end

    local image_bg = layer:getChildByName('image_bg')
    local image_tip_frame = layer:getChildByName('image_tip_frame')
    local label_desc = image_tip_frame:getChildByName('label_desc')
    local button_ok = image_tip_frame:getChildByName('button_ok')
    local button_cancle = image_tip_frame:getChildByName('button_cancle')
    local image_bar_bg = layer:getChildByName('image_bar_bg')
    local image_game = layer:getChildByName('image_game');

    if APP_FOR=="BiYin" then
        image_game:loadTexture('img/common/bg/logo_beyond_big.png')
    end

    image_tip_frame:setVisible(true)
    local size = image_tip_frame:getContentSize();

    image_bg:setVisible(true)
    button_cancle:setVisible(false)
    image_bar_bg:setVisible(false)

    if desc == "tip_net" then 
        label_desc:setString("无法连接服务器，请检查网络");
        button_ok:setTitleText('重连')
    elseif desc == 'tip_version' then
        label_desc:setString("检测到有最新的游戏版本，请到下载页面更新");
    elseif desc == 'tip_restart' then
        label_desc:setString("更新完毕，请重启游戏。");
    end

    button_ok:setPositionX(size.width / 2)
    button_ok:addTouchEventListener(function(widget, EventType)
        if EventType == ccui.TouchEventType.ended then 
            if desc == "tip_net" then 
                if self.hotInstance then
                    self.hotInstance:checkServerVersionInfo()
                end

                image_tip_frame:setVisible(false)
            elseif desc == 'tip_version' then
                if device.platform == "android" then
                    local luaj = require "cocos.cocos2d.luaj";
                    local className = "org.cocos2dx.lua/AppActivity";
                    local args = {}
                    local sig = "()Ljava/lang/String;"
                    local ok, channel = luaj.callStaticMethod(className, "getChannel", args, sig)
                    if ok then
                        local config = self:readJson("channel")
                        for _, info in ipairs(config) do 
                            if info.desc == channel then 
                                table.insert(args, info.url)
                                sig = "(Ljava/lang/String;)V"
                                luaj.callStaticMethod(className, "skipDownloadLink", args, sig)
                                break
                            end
                        end
                    end
                elseif device.platform == "ios" then
                    local config = self:readJson("channel")
                    cclook(" ios , config:%s ", config )
                    for _, info in ipairs(config) do 
                        if info.desc==APP_FOR then 
                            local luaoc = require "cocos.cocos2d.luaoc";
                            luaoc.callStaticMethod("DZNetworkTool", "linkToDownloadPage", {link = info.url})
                            return
                        end
                    end
                    -- ios 暂时没在 channel里配置，直接调用
                    local urlTab = {    BiYin="http://fir.im/beyondTexas", 
                                        TaiTan = "http://fir.im/sewx"  }
                    local url = urlTab[ APP_FOR ]
                    local luaoc = require "cocos.cocos2d.luaoc";
                    luaoc.callStaticMethod("DZNetworkTool", "linkToDownloadPage", {link = url })
                end

                cc.Director:getInstance():endToLua()
            elseif desc == 'tip_restart' then
                if device.platform == "ios" then
                    
                else
                    cc.Director:getInstance():endToLua()
                end
            end
        end
    end)
end


function GameAndUpdate:readJson( fileName )
    local path = JSON_DIR.."/"..fileName..".json"
    local tab = {}
    if cc.FileUtils:getInstance():isFileExist( path ) then
        local data = cc.FileUtils:getInstance():getDataFromFile(path)
        tab = json.decode( data )
    end
    return tab
end 


function GameAndUpdate:update(sender)
    if self.tipStatus == 'show' then 
        self.tipStatus = 'start'     --恢复默认值
        self:addUITip()
    elseif self.tipStatus == 'ok'  then
        print('可以进入正常的 逻辑了 ')
        sender:stopAllActions()

        if self.logoLayer then 
            self.logoLayer:setVisible(false)
        end
        if self.tipLayer then 
            self.tipLayer:setVisible(false)
        end
        
    elseif self.tipStatus == 'tip_net' then
        self:addFailUI("tip_net")
        self.tipStatus = 'start'     --恢复默认值

    elseif self.tipStatus == 'tip_version' then
        self:addFailUI("tip_version")
        self.tipStatus = 'start'     --恢复默认值
    end

    if self.tipStatus=="ok" then 
        if self.finisnedFunc then 
            self.finisnedFunc()
        end
    end
end








   


















