
local app = cc.Application:getInstance()
local plat = app:getTargetPlatform()

cclog = function(...)
    print(...) 
end

function __G__TRACKBACK__(msg)
    cclog("-----------------code by lx-----------------------")
    cclog("LUA ERROR: " .. tostring(msg) .. "\n")
    cclog(debug.traceback())
    cclog("------------------code by lx----------------------")
end



OPEN_HOT_UPDATE = true

function setSearchPath()
    local WRITEABLEPATH = cc.FileUtils:getInstance():getWritablePath()
    print("设备可写目录:%s", WRITEABLEPATH);

    CACHEDIR = WRITEABLEPATH .. "cache/";
    _G.__MY_HOT_UPDATE_PATH__ = CACHEDIR .. 'hot/'

    if not (cc.FileUtils:getInstance():isDirectoryExist(CACHEDIR)) then        
        cc.FileUtils:getInstance():createDirectory(CACHEDIR) 
        print("创建游戏图片缓存目录成功:%s", CACHEDIR)      
    end

    if not (cc.FileUtils:getInstance():isDirectoryExist(_G.__MY_HOT_UPDATE_PATH__)) then        
        cc.FileUtils:getInstance():createDirectory(_G.__MY_HOT_UPDATE_PATH__) 
        print("创建热更新缓存目录成功:%s", _G.__MY_HOT_UPDATE_PATH__)      
    end

    print(_G.__MY_HOT_UPDATE_PATH__)
    cc.FileUtils:getInstance():addSearchPath(CACHEDIR);
    cc.FileUtils:getInstance():addSearchPath("src/hall/src");
    cc.FileUtils:getInstance():addSearchPath("src/hall/res");
    cc.FileUtils:getInstance():addSearchPath("src/game/");

    if OPEN_HOT_UPDATE then 
        cc.FileUtils:getInstance():addSearchPath(_G.__MY_HOT_UPDATE_PATH__ .. "src/hall/src", true);
        cc.FileUtils:getInstance():addSearchPath(_G.__MY_HOT_UPDATE_PATH__ .. "src/hall/res", true);
        cc.FileUtils:getInstance():addSearchPath(_G.__MY_HOT_UPDATE_PATH__ .. "src/game/", true);
    end
end

setSearchPath()


-----------------------------------------------------------------
--             基本引入
-----------------------------------------------------------------
require "cocos.init"
require "config"
require "utils.Log"
local EventEmitter = require "utils.EventEmitter"




-----------------------------------------------------------------
--             main 入口
-----------------------------------------------------------------
local function main()
    -- 每5000毫秒回收内存
    collectgarbage("setpause", 100)
    collectgarbage("setstepmul", 5000)

    -- 加载GLView
    local director = cc.Director:getInstance()
    local glView = director:getOpenGLView()
    if nil == glView then
        glView = cc.GLViewImpl:create("myWork")
        director:setOpenGLView(glView)
    end

    director:setOpenGLView(glView)
    glView:setDesignResolutionSize(1024, 768, cc.ResolutionPolicy.NO_BORDER )
    director:setDisplayStats(true)
    director:setAnimationInterval(1.0 / 60)

    -- require "data.BridgeManager"
    -- BridgeManager:ins();
       
    G_EmitIns = EventEmitter:new()
       
    -- 进入游戏场景。
    local scene = cc.Director:getInstance():getRunningScene()
    if not scene then 
        scene = cc.Scene:create()
        cc.Director:getInstance():runWithScene(scene)
    end
    require "update.LogoAndUpdate"

    local layer
    layer = LogoAndUpdate:create(function()
        local Login = require "ui.Login"
        local loginLayer = Login:new()
        scene:addChild(loginLayer);

        layer:removeFromParent()
    end);
    
    scene:addChild(layer);
end

xpcall(main, __G__TRACKBACK__)
