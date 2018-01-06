

require "net.messageProtoId"
require "uiRoot.UI"

local routeTable = require "net.messageRoute"
local msghead_pb = require "msghead_pb"
local OutTimeMaxCount = 2;

local IP = '119.29.180.99';
local PORT = 8022;

-- local IP = '192.168.1.202';
-- local PORT = 8022;
local SERIAL_NUMBER = 0;

local NET_STATUS_DISCONNECT = 0;       --断开连接
local NET_STATUS_CONNECTING = 1;        --与服务器连接中------
local NET_STATUS_HAND = 2;            --与服务器握手------
local NET_STATUS_CONNECTED = 3;          --已经连接成功了 啦啦啦~~~~

comm = class("comm",function()
    local ret = { 
        outTimeCount = 0,
        m_register_list = {}, 
        m_callback_list = {}, 
        connectStatus = NET_STATUS_DISCONNECT
    }
    return ret
end)

comm.__instance = nil

-- 创建通讯实例对象
function comm:create()
    local ret = comm:new()
    comm.__instance = ret
    ret:init()
    return ret
end

-- 获取单例
function comm:ins()
    if not comm.__instance then 
        comm:create()
    end
    return comm.__instance
end

-- 销毁函数
function comm:destroy()
    comm.__instance = nil
end


-- 初始化实例
function comm:init() 
    c_setIP(IP);
    c_setPort(PORT);
    
    local func = function(msgHead, msgBody, boydLen) 
        self:getMessage(msgHead, msgBody, boydLen);
    end

    local statusFunc = function(status)
        self.connectStatus = status;

        if status == NET_STATUS_DISCONNECT then
            self:stopTimer();
        elseif status == NET_STATUS_CONNECTED then
            self:startTimer();
        end
    end

    c_setSockDataCallBack(func, statusFunc);
end


function comm:startTimer()
    if not self.m_scheduleRun then
        local func = function() 
            self:onHeartDuring();
        end
        self.m_scheduleRun = cc.Director:getInstance():getScheduler():scheduleScriptFunc(func, 5, false);
    end
end


function comm:stopTimer()
    if self.m_scheduleRun then
        cc.Director:getInstance():getScheduler():unscheduleScriptEntry(self.m_scheduleRun);
        self.m_scheduleRun = nil;
    end
end


function comm:onHeartDuring()
    if self.connectStatus == NET_STATUS_CONNECTED then 
        self:sendHeartPackage();
    end
end


function comm:getMessage(msgHead, msgBody, boydLen)
    local headJson = json.decode(msgHead);
    local protoId = headJson.protoId;
    local serialNumber = headJson.serialNum;

    if protoId == msg_Login_Heart then   --心跳包
        self.outTimeCount = self.outTimeCount - 1;
    else
    cclog('lua protoId= %d, serialNumber= %d, boydLen= %d', protoId, serialNumber, boydLen);

        if 0 == DEBUG_TEST then 
            UI:getInstance():showWait(false);
        end

        local pRetBody = routeTable[protoId];
        self:decodePBMsgBody(msgBody, pRetBody);

        printProtobuf(pRetBody);
        --监听的消息应答   这边是监听 组ID 并没有使用具体的消息协议ID
        local teamProtoId = math.floor(protoId / 1000) * 1000;     
        local noticeList = self.m_register_list[teamProtoId];
        if noticeList and #noticeList > 0 then 
            for _, obj in ipairs(noticeList) do 
                if obj and obj.onServerNotify then 
                    obj.onServerNotify(obj, protoId, pRetBody);
                end
            end
        end
        --发送消息的应答
        local saveInfo = self.m_callback_list[serialNumber];
        if saveInfo then 
            local protoId = saveInfo.protoId;
            local sendMsg = saveInfo.sendMsg;
            local callback = saveInfo.callback;
            local errCallback = saveInfo.errCallback;
            if callback then 
                callback(protoId, pRetBody, sendMsg);
            end
            self.m_callback_list[serialNumber] = nil;
        end

        self.outTimeCount = 0;                         --正常消息过来 则表示网络正常  直接清零
    end
end



function comm:decodePBMsgBody(pData, stMsgBody)
    if stMsgBody then 
        stMsgBody:Clear();
        stMsgBody:ParseFromString(pData);
    end
end


-- 发送服务器请求
function comm:sendMessage(protoId, msg, callback, errCallback, serialNumber) 
    serialNumber = serialNumber or self:getSerialNunmber();
    local info = self.m_callback_list[serialNumber];
    if not info then 
        info = {
            serialNumber = serialNumber,
            sendMsg = msg,
            protoId = protoId,
            callback = callback,
            errCallback = errCallback,
        };
        self.m_callback_list[serialNumber] = info;
    else
        protoId = info.protoId;
        msg = info.sendMsg;
    end
    local head = {};
    head.protoId = protoId;
    head.serialNum = serialNumber;
    head.time = os.time();
    head.dt = 1;
    head.zt = 1;
    head.code = 1;

    local headString = json.encode(head);
    local bodyString = msg:SerializeToString();
    local ret = c_sendToServerPackage(headString , bodyString);

    if 0 == DEBUG_TEST then 
        UI:getInstance():showWait(true, serialNumber);
    end

   return ret;
end


--发送心跳包
function comm:sendHeartPackage()
    if self.outTimeCount > 2 then 
        cclog('超时啦啦啦拉拉')
        self.connectStatus = NET_STATUS_DISCONNECT;
        return 
    end
    local serialNumber = self:getSerialNunmber();
    local head = {};
    head.protoId = msg_Login_Heart;
    head.serialNum = serialNumber;
    head.time = os.time();
    head.dt = 1;
    head.zt = 1;
    head.code = 1;

    local content = msghead_pb.PBMsgEmpty();
    local bodyString = content:SerializeToString();

    local headString = json.encode(head);
    c_sendToServerPackage(headString , bodyString);
    self.outTimeCount = self.outTimeCount + 1;
end


function comm:closeServer()
    self.outTimeCount = 0;
    self.connectStatus = NET_STATUS_DISCONNECT;
    self:stopTimer();
    c_closeSocket()
end


-- 注册通讯通知(包括数据返回和服务器通知)
function comm:registNotify(protoId, object, isForever)
    local register_info = self.m_register_list[protoId];
    if not register_info then
        register_info = {};
        self.m_register_list[protoId] = register_info;
    end

    for _,obj in ipairs(register_info)do
        if obj == object then
            cclog("comm:registNotify","对象已经注册。")
            return
        end
    end
    object.__custom__isForever__ = isForever;
    table.insert(register_info, object)
end



-- 注销通讯通知(包括数据返回和服务器通知)
function comm:unregistNotify(protoId, object)
    local register_info = self.dic_register_info[protoId] or {}

    for i, obj in ipairs(register_info)do
        if obj == object then
            table.remove(register_info,i)
            break
        end
    end
end


-- 注销对象相关的全部通讯通知(包括数据返回和服务器通知) 永久监听除外
function comm:unregistAllNotify()
    local newRegistList = {};
    for protoId, objList in pairs(self.m_register_list) do 
        for _, obj in ipairs(objList) do 
            if obj.__custom__isForever__ then
                local newObjList = newRegistList[protoId];
                if not newObjList then 
                    newObjList = {};
                end

                table.insert(newObjList, obj);
            end
        end
    end
    self.m_register_list = newRegistList;
end


function comm:getSerialNunmber()
    SERIAL_NUMBER = SERIAL_NUMBER + 1;
    if SERIAL_NUMBER > 5000 then 
        SERIAL_NUMBER = 1
    end
    return SERIAL_NUMBER
end

function comm:setIP(ip, port)
    c_setIP(ip);
    c_setPort(port);
end






 
